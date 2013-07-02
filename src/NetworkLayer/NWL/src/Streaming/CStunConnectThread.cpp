/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStunConnectThread.cpp
///
///  Implements CStunConnectThread class, responsible for connection to
///    remote peer through STUN server
///
///  @author Dmitry Netrebenko @date 27.11.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CStunConnectThread.h>
#include <AidLib/Logging/cLog.h>
#include <NWL/Streaming/ENATTraversalStreamState.h>
#include <AidLib/CCrypto/CCrypto.h>
#include <NWL/Streaming/CStreamException.h>
#include <AidLib/CSingleton/CSingleton.h>
#include <udt.h>
#include <AidLib/CThread/CThreadLS.h>

CStunConnectThread::CStunConnectThread(SStunConnectRuntime& statistic)
	:	CThread()
	,	CRelayConnectSettings()
	,	CStunConnectSettings()
	,	m_strLocalAddr(_T(""))
	,	m_nLocalPort(0)
	,	m_strRemoteAddr(_T(""))
	,	m_nRemotePort(0)
	,	m_strPeerAddr(_T(""))
	,	m_nPeerPort(0)
	,	m_State(ntssNone)
	,	m_Sock(NULL)
	,	m_UDTSock(NULL)
	,	m_AuthTries(0)
	,	m_NextAuthTime(cDate().GetNever())
	,	m_BindTries(0)
	,	m_NextBindTime(cDate().GetNever())
	,	m_ProbeTries(0)
	,	m_NextProbeTime(cDate().GetNever())
	,	m_LastConnectError(cerNoError)
	,	m_bConnectedToPeer(false)
	,	m_statistic(statistic)
{
TRY_CATCH
	// Clear internal buffer
	ClearBuffer();
	// Init CCrypto singleton
	CRYPTO_INSTANCE;
CATCH_THROW()
}

CStunConnectThread::~CStunConnectThread()
{
TRY_CATCH
CATCH_LOG()
}

void CStunConnectThread::Execute(void*)
{
	SET_THREAD_LS;
TRY_CATCH
	if( !m_Sock || !m_UDTSock )
		throw MCStreamException(_T("Sockets not created"));
	// Prepare addresses and socket
	Prepare();
	while(!Terminated())
	{
		if(m_Sock->ReadSelect(100))
		{
			// Clear buffer
			ClearBuffer();
			// Read data from socket
			m_nBytesInBuffer = m_Sock->ReceiveFrom(m_strRemoteAddr, m_nRemotePort, m_Buffer, STUN_CLNT_BUFFER_SIZE);
			if(m_nBytesInBuffer && (SOCKET_ERROR != m_nBytesInBuffer))
			{
				// Construct message
				SPStunMessage msg = GetMessage();
				// Process incoming message
				if(msg.get())
					ProcessStunMessage(msg.get());
			}
		}
		// Check state
		if(CheckState())
			break;
	}
	// Create UDT connection
	if(m_bConnectedToPeer)
	{
		// Store time for starting UDT connection
		m_statistic.m_udt.m_startTime = timeGetTime();
		ConnectThroughUDT();
		// Store time for end of UDT connection
		m_statistic.m_udt.m_endTime = timeGetTime();
	}
	else
	{
		if(m_ConnectErrorEvent && !Terminated())
			m_ConnectErrorEvent(NULL, cerTriesPassed);
	}
	return;
CATCH_LOG()
	if (m_ConnectErrorEvent && !Terminated())
		m_ConnectErrorEvent(NULL, m_LastConnectError);
}

void CStunConnectThread::SetSockets(CSSocket* win_sock, CSUDTSocket* udt_sock)
{
TRY_CATCH
	// Set up sockets
	m_Sock = win_sock;
	m_UDTSock = udt_sock;
CATCH_THROW()
}

void CStunConnectThread::BindEvents(NotifyEvent connect, ConnectErrorEvent connect_error)
{
TRY_CATCH
	// Set up events
	m_ConnectedEvent = connect;
	m_ConnectErrorEvent = connect_error;
CATCH_THROW()
}

void CStunConnectThread::ClearBuffer()
{
TRY_CATCH
	memset(m_Buffer, 0, STUN_CLNT_BUFFER_SIZE);
	m_nBytesInBuffer = 0;
CATCH_THROW()
}

void CStunConnectThread::Prepare()
{
TRY_CATCH
	// Set variables
	m_bConnectedToPeer = false;
	// Init state
	m_State = ntssNone;
	
	m_AuthTries = 0;
	m_NextAuthTime.GetNow();
	m_BindTries = 0;
	m_NextBindTime.GetNever();
	m_ProbeTries = 0;
	m_NextProbeTime.GetNever();

	// Get local address and port
	CSSocket sock(stUDP);
	sock.Create();
	sock.Connect(m_strServerAddr, m_nServerPort);
	m_strLocalAddr = sock.GetSockName();
	sock.Close();

	m_Sock->Bind();
	m_nLocalPort = m_Sock->GetSockPort();

	// Clear remote addresses list
	m_nRemoteAddresses.clear();

	//Create local addresses list
	m_LocalAddresses.clear();
	SPeerAddr addr;

	char name_buf[MAX_PATH];
	memset(name_buf, 0, MAX_PATH);

	// Get host name
	if(!gethostname(name_buf, MAX_PATH))
	{
		// Get host info
		hostent* host = gethostbyname(name_buf);

		if(host)
		{
			// Copy addresses
			char** addrs = host->h_addr_list;
			while(*addrs)
			{
				// Clear address
				memset(addr.address, 0, PEER_ADDR_SIZE);
				// Get address
				char* str_addr = inet_ntoa(*((struct in_addr *)*addrs));
				// Fill address
				memcpy(addr.address, str_addr, min(strlen(str_addr), PEER_ADDR_SIZE - 1));
				addr.port = m_nLocalPort;
				// Add address
				m_LocalAddresses.push_back(addr);
				addrs++;
			}
		}
	}

	// Add local address if list is empty
	if(m_LocalAddresses.empty())
	{
		memset(addr.address, 0, PEER_ADDR_SIZE);
		memcpy(addr.address, m_strLocalAddr.c_str(), min(m_strLocalAddr.length(), PEER_ADDR_SIZE - 1));
		addr.port = m_nLocalPort;
		m_LocalAddresses.push_back(addr);
	}
CATCH_THROW()
}

SPStunMessage CStunConnectThread::GetMessage()
{
TRY_CATCH
	if(m_nBytesInBuffer < sizeof(SStunMessage))
	{
		ClearBuffer();
		return SPStunMessage();
	}
	// Allocate memory for message
	SPStunMessage msg((SStunMessage*) new char[m_nBytesInBuffer]);
	// Copy data from buffer to message
	memcpy(msg.get(), m_Buffer, m_nBytesInBuffer);
	// Clear internal buffer
	ClearBuffer();
	return msg;
CATCH_THROW()
}

void CStunConnectThread::SendAuthRequest()
{
TRY_CATCH
	// Message
	SStunMessage msg;
	// Set sizes
	msg.data_size = 0;
	msg.size = sizeof(SStunMessage);
	// Set message type
	msg.msg_type = smtAuthRequest;
	// Set identificator
	msg.ident = m_AuthTries + 1;
	// Set connection id
	memset(msg.connect_id, 0, STUN_CONNID_SIZE);
	memcpy(msg.connect_id, m_strConnectId.c_str(), min(m_strConnectId.length(), STUN_CONNID_SIZE - 1));
	// Set peer id
	memset(msg.peer_id, 0, STUN_PEERID_SIZE);
	memcpy(msg.peer_id, m_strLocalPeerId.c_str(), min(m_strLocalPeerId.length(), STUN_PEERID_SIZE - 1));
	// Set user id
	memset(msg.auth.user_id, 0, STUN_USERID_SIZE);
	memcpy(msg.auth.user_id, m_strSrvUserId.c_str(), min( m_strSrvUserId.length(), STUN_USERID_SIZE - 1));
	// Set local address
	memset(msg.src_addr.address, 0, PEER_ADDR_SIZE);
	memcpy(msg.src_addr.address, m_strLocalAddr.c_str(), min(m_strLocalAddr.length(), PEER_ADDR_SIZE - 1));	
	msg.src_addr.port = m_nLocalPort;
	// Set remote address
	memset(msg.dest_addr.address, 0, PEER_ADDR_SIZE);
	memcpy(msg.dest_addr.address, m_strServerAddr.c_str(), min(m_strServerAddr.length(), PEER_ADDR_SIZE - 1));	
	msg.dest_addr.port = m_nServerPort;

	// Store statistic data
	if(!m_AuthTries)
		m_statistic.m_stages[srtAuth].m_total.m_startTime = timeGetTime();
	SStunStageTime time;
	time.m_startTime = timeGetTime();
	m_statistic.m_stages[srtAuth].m_requests.push_back(time);

	// Send authentication request message
	m_Sock->SendTo(msg.dest_addr.address, msg.dest_addr.port, (char*)&msg, msg.size);

	Log.Add(_MESSAGE_,_T("Sending auth request to %s:%d"),msg.dest_addr.address, msg.dest_addr.port);

	// Set new state
	m_State |= ntssAuthRequested;
CATCH_THROW()
}

void CStunConnectThread::SendBindRequest(bool add_local_addresses)
{
TRY_CATCH
	unsigned int size;
	unsigned int data_size;
	// Calculate sizes
	if(add_local_addresses)
	{
		data_size = (unsigned int)m_LocalAddresses.size();
		size = STUN_MSG_HEAD_SIZE + sizeof(SPeerAddr) * data_size;
	}
	else
	{
		data_size = 0;
		size = sizeof(SStunMessage);
	}
	// Allocate memory for message
	SCPStunMessage msg((SStunMessage*) new char[size]);
	// Set up sizes
	msg->size = size;
	msg->data_size = data_size;
	// Set up message type
	msg->msg_type = smtBindRequest;
	// Set identificator
	msg->ident = m_BindTries + 1;
	// Set up connection id 
	memset(msg->connect_id, 0, STUN_CONNID_SIZE);
	memcpy(msg->connect_id, m_strConnectId.c_str(), min(m_strConnectId.length(), STUN_CONNID_SIZE - 1));
	// Set up user id
	memset(msg->auth.user_id, 0, STUN_USERID_SIZE);
	memcpy(msg->auth.user_id, m_strSrvUserId.c_str(), min(m_strSrvUserId.length(), STUN_USERID_SIZE - 1));
	// Set up hash
	memcpy(msg->auth.hash, m_Hash, STUN_HASH_SIZE);
	// Set up peer id
	memset(msg->peer_id, 0, STUN_PEERID_SIZE);
	memcpy(msg->peer_id, m_strLocalPeerId.c_str(), min(m_strLocalPeerId.length(), STUN_PEERID_SIZE - 1));
	// Set up source address
	memset(msg->src_addr.address, 0, PEER_ADDR_SIZE);
	memcpy(msg->src_addr.address, m_strLocalAddr.c_str(), min(m_strLocalAddr.length(), PEER_ADDR_SIZE - 1));	
	msg->src_addr.port = m_nLocalPort;
	// Set up destination address
	memset(msg->dest_addr.address, 0, PEER_ADDR_SIZE);
	memcpy(msg->dest_addr.address, m_strServerAddr.c_str(), min(m_strServerAddr.length(), PEER_ADDR_SIZE - 1));	
	msg->dest_addr.port = m_nServerPort;
	if(add_local_addresses)
	{
		// Add local addresses from list
		SPeerAddr* addr = (SPeerAddr*)msg->data;
		Addresses::iterator index;
		for(index = m_LocalAddresses.begin(); index != m_LocalAddresses.end(); ++index)
		{
			memcpy(addr, &(*index), sizeof(SPeerAddr));	
			Log.Add(_MESSAGE_,_T("Sending addr(%s:%d)"),addr->address,addr->port);
			addr++;
		}
	}

	// Store statistic data
	if(!m_BindTries)
		m_statistic.m_stages[srtBind].m_total.m_startTime = timeGetTime();
	SStunStageTime time;
	time.m_startTime = timeGetTime();
	m_statistic.m_stages[srtBind].m_requests.push_back(time);

	// Send message
	m_Sock->SendTo(msg->dest_addr.address, msg->dest_addr.port, (char*)msg.get(), msg->size);

	Log.Add(_MESSAGE_,_T("Sending bind request to %s:%d"),msg->dest_addr.address, msg->dest_addr.port);
	// Set new state
	m_State |= ntssBindRequested;
CATCH_THROW()
}

void CStunConnectThread::SendProbeRequests()
{
TRY_CATCH
	// Get number of addresses
	size_t count = m_nRemoteAddresses.size();
	// Check up number of remote addresses
	if(!count)
		return;

	// Store statistic data
	if(!m_ProbeTries)
		m_statistic.m_stages[srtProbe].m_total.m_startTime = timeGetTime();
	SStunStageTime time;
	time.m_startTime = timeGetTime();
	m_statistic.m_stages[srtProbe].m_requests.push_back(time);

	// Addresses iterator
	Addresses::iterator index;
	// Send probe request messages
	for(index = m_nRemoteAddresses.begin(); index != m_nRemoteAddresses.end(); ++index)
	{
		for(unsigned int idx2 = index->port; idx2 < index->port + m_nProbePortRange; idx2++)
			SendProbeRequest(index->address, idx2);
	}
CATCH_THROW()
}

void CStunConnectThread::SendProbeRequest(const tstring& peer_addr, const unsigned int& peer_port)
{
TRY_CATCH
	// Create message
	SStunMessage msg;
	// Set up sizes
	msg.data_size = 0;
	msg.size = sizeof(SStunMessage);
	// Set up message type
	msg.msg_type = smtProbeRequest;
	// Set identificator
	msg.ident = m_ProbeTries + 1;
	// Set up connection id
	memset(msg.connect_id, 0, STUN_CONNID_SIZE);
	memcpy(msg.connect_id, m_strConnectId.c_str(), min(m_strConnectId.length(), STUN_CONNID_SIZE - 1));
	// Set up user id
	memset(msg.auth.user_id, 0, STUN_USERID_SIZE);
	memcpy(msg.auth.user_id, m_strSrvUserId.c_str(), min(m_strSrvUserId.length(), STUN_USERID_SIZE - 1));
	// Set up peer id
	memset(msg.peer_id, 0, STUN_PEERID_SIZE);
	memcpy(msg.peer_id, m_strLocalPeerId.c_str(), min(m_strLocalPeerId.length(), STUN_PEERID_SIZE - 1));
	// Set up source address
	memset(msg.src_addr.address, 0, PEER_ADDR_SIZE);
	memcpy(msg.src_addr.address, m_strLocalAddr.c_str(), min(m_strLocalAddr.length(), PEER_ADDR_SIZE - 1));	
	msg.src_addr.port = m_nLocalPort;
	// Set up destination address
	memset(msg.dest_addr.address, 0, PEER_ADDR_SIZE);
	memcpy(msg.dest_addr.address, peer_addr.c_str(), min( peer_addr.length(), PEER_ADDR_SIZE - 1));	
	msg.dest_addr.port = peer_port;
	// Send message
	m_Sock->SendTo(peer_addr, peer_port, (char*)&msg, msg.size);

	Log.Add(_MESSAGE_,_T("Sending probe request to %s:%d"), peer_addr.c_str(), peer_port);
CATCH_THROW()
}

void CStunConnectThread::SendProbeResponse(SStunMessage* probe_request_msg)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("Sending probe response"));
	// Create message
	SStunMessage msg;
	// Set up sizes
	msg.data_size = 0;
	msg.size = sizeof(SStunMessage);
	// Set up message type
	msg.msg_type = smtProbeResponse;
	// Set identificator
	msg.ident = probe_request_msg->ident;
	// Set up connection id
	memset(msg.connect_id, 0, STUN_CONNID_SIZE);
	memcpy(msg.connect_id, m_strConnectId.c_str(), min(m_strConnectId.length(), STUN_CONNID_SIZE - 1));
	// Set up user id
	memset(msg.auth.user_id, 0, STUN_USERID_SIZE);
	memcpy(msg.auth.user_id, m_strSrvUserId.c_str(), min(m_strSrvUserId.length(), STUN_USERID_SIZE - 1));
	// Set up peer id
	memset(msg.peer_id, 0, STUN_PEERID_SIZE);
	memcpy(msg.peer_id, m_strLocalPeerId.c_str(), min(m_strLocalPeerId.length(), STUN_PEERID_SIZE - 1));
	// Set up source address
	memset(msg.src_addr.address, 0, PEER_ADDR_SIZE);
	memcpy(msg.src_addr.address, m_strLocalAddr.c_str(), min(m_strLocalAddr.length(), PEER_ADDR_SIZE - 1));	
	msg.src_addr.port = m_nLocalPort;
	// Set up destination address
	memset(msg.dest_addr.address, 0, PEER_ADDR_SIZE);
	memcpy(msg.dest_addr.address, m_strPeerAddr.c_str(), min(m_strPeerAddr.length(), PEER_ADDR_SIZE - 1));	
	msg.dest_addr.port = m_nPeerPort;
	// Send probe response message
	m_Sock->SendTo(msg.dest_addr.address, msg.dest_addr.port, (char*)&msg, msg.size);
	
	Log.Add(_MESSAGE_,_T("Sending probe response to %s:%d"), msg.dest_addr.address, msg.dest_addr.port);
CATCH_THROW()
}

void CStunConnectThread::ProcessStunMessage(SStunMessage* msg)
{
TRY_CATCH
	// Process incoming message
	switch(msg->msg_type)
	{
	case smtAuthRequest:
		// Invalid message, will be ignored
		break;
	case smtAuthResponse:
		ProcessAuthResponse(msg);
		break;
	case smtAuthFailed:
		ProcessAuthFailedResponse(msg);
		break;
	case smtServerBusy:
		ProcessServerBusyResponse(msg);
		break;
	case smtBindRequest:
		// Invalid message, will be ignored
		break;
	case smtBindResponse:
		ProcessBindResponse(msg);
		break;
	case smtProbeRequest:
		ProcessProbeRequest(msg);
		break;
	case smtProbeResponse:
		ProcessProbeResponse(msg);
		break;
	case smtData:
		// Invalid message, will be ignored
		break;
	default:
		break;
	}
CATCH_THROW()
}

void CStunConnectThread::ProcessAuthResponse(SStunMessage* auth_response_msg)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("Processing auth response"));
	unsigned int id = auth_response_msg->ident - 1;
	// Store statistic data
	if((id >= 0) && (id < m_statistic.m_stages[srtAuth].m_requests.size()))
		m_statistic.m_stages[srtAuth].m_requests[id].m_endTime = timeGetTime();
	if(auth_response_msg->ident != m_AuthTries)
	{
		Log.Add(_MESSAGE_,_T("Invalid message identificator"));
		return;
	}
	// Copy challenge from message
	memcpy(m_Challenge, auth_response_msg->data, STUN_CHALLENGE_SIZE);
	// Calculate buffer length
	unsigned int buf_len = max((unsigned int)STUN_HASH_SIZE, (unsigned int)(m_strSrvPassword.length() + STUN_CHALLENGE_SIZE));
	// Allocate buffer
	boost::scoped_ptr<char> buf(new char[buf_len]);
	// Fill buffer by challenge and password
	memset(buf.get(), 0, buf_len);
	memcpy(buf.get(), m_Challenge, STUN_CHALLENGE_SIZE);
	memcpy(buf.get() + STUN_CHALLENGE_SIZE, m_strSrvPassword.c_str(), m_strSrvPassword.length());
	// Create sha1 hash from buffer and copy to m_Hash
	CRYPTO_INSTANCE.MakeHash(buf.get(), (unsigned int)(m_strSrvPassword.length() + STUN_CHALLENGE_SIZE), buf.get());
	memcpy(m_Hash, buf.get(), STUN_HASH_SIZE);
	// Set new state
	m_State |= ntssAuthResponsed;
	// Set current time as time to send bind request
	m_NextBindTime.GetNow();
	// Store statistic data
	m_statistic.m_stages[srtAuth].m_total.m_endTime = timeGetTime();
CATCH_THROW()
}

void CStunConnectThread::ProcessAuthFailedResponse(SStunMessage* auth_failed_msg)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("Processing auth failed response"));
	unsigned int id = auth_failed_msg->ident - 1;
	// Store statistic data
	if((id >= 0) && (id < m_statistic.m_stages[srtAuth].m_requests.size()))
		m_statistic.m_stages[srtAuth].m_requests[id].m_endTime = timeGetTime();
	if(auth_failed_msg->ident != m_AuthTries)
	{
		Log.Add(_MESSAGE_,_T("Invalid message identificator"));
		return;
	}
	m_LastConnectError = cerAuthFailed;
	// Store statistic data
	m_statistic.m_stages[srtAuth].m_total.m_endTime = timeGetTime();
	throw MCStreamException(_T("Server authentication failed"));
CATCH_THROW()
}

void CStunConnectThread::ProcessServerBusyResponse(SStunMessage* srv_busy_msg)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("Processing server busy response"));
	unsigned int id = srv_busy_msg->ident - 1;
	// Store statistic data
	if((id >= 0) && (id < m_statistic.m_stages[srtAuth].m_requests.size()))
		m_statistic.m_stages[srtAuth].m_requests[id].m_endTime = timeGetTime();

//TODO: ??????? will be skipped now

CATCH_THROW()
}

void CStunConnectThread::ProcessBindResponse(SStunMessage* bind_response_msg)
{
TRY_CATCH
	unsigned int id = bind_response_msg->ident - 1;
	// Store statistic data
	if((id >= 0) && (id < m_statistic.m_stages[srtBind].m_requests.size()))
		m_statistic.m_stages[srtBind].m_requests[id].m_endTime = timeGetTime();
	if(bind_response_msg->ident != m_BindTries)
	{
		Log.Add(_MESSAGE_,_T("Invalid message identificator"));
		return;
	}
	if(bind_response_msg->data_size)
	{
		Log.Add(_MESSAGE_,_T("Processing bind response"));
		// Set new state
		m_State |= ntssBindResponsed;
		// Copy addresses list from message
		SPeerAddr addr;
		SPeerAddr* data = (SPeerAddr*)bind_response_msg->data;
		for(unsigned int index = 0; index < bind_response_msg->data_size; index++)
		{
			memcpy(&addr, data, sizeof(SPeerAddr));
			m_nRemoteAddresses.push_back(addr);
			data++;
		}
		// Store statistic data
		m_statistic.m_stages[srtBind].m_total.m_endTime = timeGetTime();
		// Set current time as time for next probe request
		m_NextProbeTime.GetNow();
	}
	else
	{
		Log.Add(_MESSAGE_,_T("Processing null bind response"));
		// Set new state
		m_State |= ntssBindNullResponsed;
		// Clear remote addresses
		m_nRemoteAddresses.clear();
	}
CATCH_THROW()
}

void CStunConnectThread::ProcessProbeRequest(SStunMessage* probe_request_msg)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("Trying processing probe request from %s:%d"),m_strRemoteAddr.c_str(),m_nRemotePort);
	tstring peer(probe_request_msg->peer_id);
	tstring connect(probe_request_msg->connect_id);
	if((peer != m_strRemotePeerId) || (connect != m_strConnectId))
	{
		Log.Add(_MESSAGE_,_T("PROBE REQUEST FROM UNKNOWN PEER IS RECEIVED"));
		return;
	}
	if(m_State & ntssBindResponsed)
	{
		Log.Add(_MESSAGE_,_T("Processing probe request"));
		// Store peer's address and port
		m_strPeerAddr = m_strRemoteAddr;
		m_nPeerPort = m_nRemotePort;
		// Set new state
		m_State |= ntssProbeRequested;
		// Send probe response
		SendProbeResponse(probe_request_msg);
		// Store statistic data
		unsigned int id = static_cast<unsigned int>(m_statistic.m_stages[srtProbe].m_requests.size());
		if(id)
			m_statistic.m_stages[srtProbe].m_requests[id - 1].m_endTime = timeGetTime();
		m_statistic.m_stages[srtProbe].m_total.m_endTime = timeGetTime();
	}
CATCH_THROW()
}

void CStunConnectThread::ProcessProbeResponse(SStunMessage* probe_response_msg)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("Processing probe response from %s:%d"),m_strRemoteAddr.c_str(),m_nRemotePort);
	unsigned int id = probe_response_msg->ident - 1;
	// Store statistic data
	if((id >= 0) && (id < m_statistic.m_stages[srtProbe].m_requests.size()))
		m_statistic.m_stages[srtProbe].m_requests[id].m_endTime = timeGetTime();
	// Store peer's address and port
	m_strPeerAddr = m_strRemoteAddr;
	m_nPeerPort = m_nRemotePort;
	// Set new state
	m_State |= ntssProbeResponsed;
	// Store statistic data
	m_statistic.m_stages[srtProbe].m_total.m_endTime = timeGetTime();
CATCH_THROW()
}

void CStunConnectThread::ConnectThroughUDT()
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("Connection established. Local port: %d, remote addr: %s, remote port: %d"),m_nLocalPort,m_strPeerAddr.c_str(),m_nPeerPort);
	m_Sock->Close();
	// Bind socket
	if(!m_UDTSock->Bind(m_nLocalPort))
	{
		m_LastConnectError = cerWinError;
		throw MCStreamException(Format(_T("Bind method failed. %s"), UDT::getlasterror().getErrorMessage()));
	}
	while(!Terminated())
	{
		if(m_UDTSock->Connect(m_strPeerAddr, m_nPeerPort))
		{
			Log.Add(_MESSAGE_,_T("Connection through UDT established."));
			// Raise "Connected" event
			if (m_ConnectedEvent)
				m_ConnectedEvent(NULL);
			break;
		}
	}
CATCH_THROW()
}

bool CStunConnectThread::CheckState()
{
TRY_CATCH
	// Get current time
	cDate tmCurrent = cDate().GetNow();

	if((m_State & ntssProbeResponsed) || (m_State & ntssProbeRequested))
	{
		m_bConnectedToPeer = true;
		return true;
	}

	// Is time for probe request
	if((m_State & ntssBindResponsed) && (m_NextProbeTime <= tmCurrent))
	{
		if(m_ProbeTries >= m_nProbeMaxRetryCount)
		{
			// Store statistic data
			m_statistic.m_stages[srtProbe].m_total.m_endTime = timeGetTime();
			return true;
		}
		SendProbeRequests();
		m_ProbeTries++;
		m_NextProbeTime = tmCurrent;
		m_NextProbeTime.AddMilliSecs(m_nProbeRetryDelay);
	}

	// Is time for bind request
	if(!(m_State & ntssBindResponsed) && (m_State & ntssAuthResponsed) && (m_NextBindTime <= tmCurrent))
	{
		if(m_BindTries >= m_nBindMaxRetryCount)
		{
			// Store statistic data
			m_statistic.m_stages[srtBind].m_total.m_endTime = timeGetTime();
			return true;
		}
		SendBindRequest(!m_BindTries || !(m_State & ntssBindNullResponsed));
		m_BindTries++;
		m_NextBindTime = tmCurrent;
		m_NextBindTime.AddMilliSecs(m_nBindRetryDelay);
	}

	// Is time for auth request
	if(!(m_State & ntssAuthResponsed) && (m_NextAuthTime <= tmCurrent))
	{
		if(m_AuthTries >= m_nAuthMaxRetryCount)
		{
			// Store statistic data
			m_statistic.m_stages[srtAuth].m_total.m_endTime = timeGetTime();
			return true;
		}
		SendAuthRequest();
		m_AuthTries++;
		m_NextAuthTime = tmCurrent;
		m_NextAuthTime.AddMilliSecs(m_nAuthRetryDelay);
	}
	return false;
CATCH_THROW()
}
