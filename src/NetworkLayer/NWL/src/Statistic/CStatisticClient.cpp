/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStatisticClient.cpp
///
///  Implements CStatisticClient class, responsible for client of statistic service
///
///  @author Dmitry Netrebenko @date 15.10.2007
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Statistic/CStatisticClient.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <NWL/Streaming/CNetworkLayer.h>
#include <AidLib/CThread/CThreadLs.h>
#include <AidLib/CCrypto/CCrypto.h>
#include <NWL/UPnP/CGatewayInfo.h>

CStatisticClient::CStatisticClient()
	:	CThread()
	,	m_socket(stUDP)
	,	m_ident(0)
	,	m_section()
{
TRY_CATCH

	/// Create socket
	if(!m_socket.Create())
		throw MCStreamException(_T("Create socket failed."));

	/// Create socket
	if(!m_socket.Bind())
		throw MCStreamException(_T("Bind socket failed."));

	/// Create event
	m_event.reset(CreateEvent(NULL, FALSE, FALSE, NULL), CloseHandle);

	/// Start thread
	Start();

CATCH_THROW()
}

CStatisticClient::~CStatisticClient()
{
TRY_CATCH

	/// Terminate thread
	Terminate();
	SetEvent(m_event.get());

	// Waiting thread termination
	if(WAIT_OBJECT_0 != WaitForSingleObject(hTerminatedEvent.get(), STAT_THREAD_TERM_TIMEOUT))
	{
		Log.Add(_WARNING_, _T("Manual thread termination..."));
		State=_RUNNING;
		Stop(true);
	}

	/// Close socket
	m_socket.Close();

CATCH_LOG()
}

void CStatisticClient::Execute(void*)
{
TRY_CATCH

	SET_THREAD_LS;

	while(!Terminated())
	{
		/// Waiting for event
		WaitForSingleObject(m_event.get(), INFINITE);

		/// Check termination
		while(!Terminated())
		{
			/// Extract message from queue
			SPStunMessage msg;
			{
				CCritSection section(&m_section);
				if(m_queue.empty())
					break;
				msg = m_queue.front();
				m_queue.pop();
				msg->ident = m_ident;
				++m_ident;
			}

			/// Get router information if current message is statistic message with sttRouterInfo type
			if(smtPeerStatistic == msg->msg_type)
			{
				/// Get pointer to statistic message
				SStatisticMessage* stat = reinterpret_cast<SStatisticMessage*>(msg->data);
				/// Check message type
				if(sttRouterInfo == stat->m_type)
				{
					/// Get pointer to router info structure
					SGatewayInfo* router = &stat->m_data.m_router;
					bool infoReceived = false;
					TRY_CATCH
						/// Get router info
						GATEWAY_INFO_INSTANCE.GetGatewayDeviceInfo(router);
						infoReceived = true;
					CATCH_LOG()
					if(!infoReceived)
						continue;
				}
			}

			/// Send authentication request
			SendAuthRequest(msg);

			bool answerReceived = false;
			bool invalidIdent = false;

			/// Waiting for auth response
			do
			{
				if(m_socket.ReadSelect(NWL_INSTANCE.GetAuthRetryDelay()))
				{
					/// Auth response received
					tstring addr;
					unsigned int port;
					memset(m_buffer, 0, STAT_CLIENT_BUF_SIZE);

					/// Receive data
					int recvd = m_socket.ReceiveFrom(addr, port, m_buffer, STAT_CLIENT_BUF_SIZE);

					/// Check message
					if(recvd != STUN_MSG_HEAD_SIZE + STUN_CHALLENGE_SIZE)
					{
						Log.Add(_WARNING_, _T("Message with invalid size is received from server."));
						break;
					}
					SStunMessage* auth = reinterpret_cast<SStunMessage*>(m_buffer);
					if(smtAuthResponse != auth->msg_type)
					{
						Log.Add(_WARNING_, _T("Message with invalid type is received from server."));
						break;
					}
					if(msg->ident != auth->ident)
					{
						Log.Add(_WARNING_, _T("Message with invalid identificator is received from server."));
						invalidIdent = true;
						continue;
					}

					answerReceived = true;
					invalidIdent = false;

					/// Copy challenge from message
					memcpy(m_challenge, auth->data, STUN_CHALLENGE_SIZE);

					tstring passwd = NWL_INSTANCE.GetRelayPasswd();
					/// Calculate buffer length
					unsigned int buf_len = max((unsigned int)STUN_HASH_SIZE, (unsigned int)(passwd.length() + STUN_CHALLENGE_SIZE));

					/// Allocate buffer
					boost::scoped_ptr<char> buf(new char[buf_len]);

					/// Fill buffer by challenge and password
					memset(buf.get(), 0, buf_len);
					memcpy(buf.get(), m_challenge, STUN_CHALLENGE_SIZE);
					memcpy(buf.get() + STUN_CHALLENGE_SIZE, passwd.c_str(), passwd.length());

					/// Create sha1 hash from buffer and copy to m_Hash
					CRYPTO_INSTANCE.MakeHash(buf.get(), (unsigned int)(passwd.length() + STUN_CHALLENGE_SIZE), buf.get());
					memcpy(msg->auth.hash, buf.get(), STUN_HASH_SIZE);

					/// Send statistic message
					m_socket.SendTo(msg->dest_addr.address, msg->dest_addr.port, (char*)msg.get(), msg->size);
				}
				else
				{
					answerReceived = false;
					invalidIdent = false;
				}
			} while(invalidIdent && !Terminated());

			if(!answerReceived)
				Log.Add(_WARNING_, _T("Authentication response not received."));
		}
	}

CATCH_LOG()
}

void CStatisticClient::AddMsg(int code, const char* buf, unsigned int len, const tstring& connectId, const tstring& peerId)
{
TRY_CATCH

	if(!buf)
		return;

	/// Create STUN message
	unsigned int size = len + STUN_MSG_HEAD_SIZE;
	SPStunMessage msg((SStunMessage*) new char[size]);
	memset(msg.get(), 0, size);

	/// Copy statistic
	memcpy(msg->data, buf, len);

	/// Set sizes
	msg->size = size;
	msg->data_size = len;

	/// Set message type
	switch(code)
	{
	case 0:
		msg->msg_type = smtPeerStatistic;
		break;
	default:
		msg->msg_type = smtPeerLog;
	}
	/// Set destination address
	msg->dest_addr.port = NWL_INSTANCE.GetRelayUDPPort();
	tstring srvAddr = NWL_INSTANCE.GetRelayHost();
	memcpy(msg->dest_addr.address, srvAddr.c_str(), min(PEER_ADDR_SIZE, srvAddr.length()));

	memcpy(msg->connect_id, connectId.c_str(), min(STUN_CONNID_SIZE, connectId.length()));
	memcpy(msg->peer_id, peerId.c_str(), min(STUN_PEERID_SIZE, peerId.length()));
	memcpy(msg->auth.user_id, m_userId.c_str(), min(STUN_USERID_SIZE, m_userId.length()));


	/// Add message to queue
	{
		CCritSection section(&m_section);
		m_queue.push(msg);
	}

	SetEvent(m_event.get());

CATCH_THROW()
}

void CStatisticClient::SendAuthRequest(SPStunMessage message)
{
TRY_CATCH

	// Message
	SStunMessage msg;

	// Copy data
	memcpy(&msg, message.get(), sizeof(SStunMessage));

	// Set sizes
	msg.data_size = 0;
	msg.size = sizeof(SStunMessage);

	// Set message type
	msg.msg_type = smtStatAuthRequest;

	// Send authentication request message
	m_socket.SendTo(msg.dest_addr.address, msg.dest_addr.port, (char*)&msg, msg.size);

CATCH_THROW()
}

tstring CStatisticClient::GetUserId() const
{
TRY_CATCH
	return m_userId;
CATCH_THROW()
}

void CStatisticClient::SetUserId(const tstring& userId)
{
TRY_CATCH
	m_userId = userId;
CATCH_THROW()
}

