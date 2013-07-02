/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRelayConnector.cpp
///
///  Implements CRelayConnector class, responsible for testing of connection
///    with relay server
///
///  @author Dmitry Netrebenko @date 04.10.2007
///
////////////////////////////////////////////////////////////////////////

#include <winsock2.h>
#include "CRelayConnector.h"
#include <AidLib/CException/CException.h>
#include <NWL/Streaming/CSocketStream.h>
#include <NWL/Streaming/relay_messages.h>
#include <AidLib/CCrypto/CCrypto.h>
#include <AidLib/CSingleton/CSingleton.h>
#include <NWL/Streaming/CSSocket.h>
#include "CSettings.h"

CRelayConnector::CRelayConnector(const size_t poolSize)
{
TRY_CATCH
	if(poolSize <= 0)
		throw MCException(_T("Invalid pool size"));
	CSingleton<CCrypto>::instance();
	m_pool.reset(new boost::threadpool::pool(poolSize));
CATCH_THROW()
}

CRelayConnector::~CRelayConnector()
{
TRY_CATCH
CATCH_LOG()
}

void CRelayConnector::ThreadEntryPoint(const tstring& userId, const tstring& password, const EStopConnect stop, const unsigned int exPort)
{
TRY_CATCH
	boost::scoped_array<char> buf(new char[BUFFER_SIZE]);
	boost::scoped_ptr<CSocketStream> stream(new CSocketStream());
	SRelayMessage msg;

	Log.Add(_MESSAGE_,_T("Starting new thread. User = %s. Stopping at step %d"), userId.c_str(), stop);

	Log.Add(_MESSAGE_,_T("Connecting to server..."));
	stream->Connect(
		CSingleton<CSettings>::instance().m_host, 
		CSingleton<CSettings>::instance().m_port, 
		true);
	Log.Add(_MESSAGE_,_T("Connected"));

	if(SC_AFTER_CONNECT == stop)
		return;

	if(SC_BEFORE_AUTH_REQUEST == stop)
	{
		Log.Add(_MESSAGE_,_T("Sending garbage instead of authentication request..."));
		stream->Send(buf.get(), BUFFER_SIZE);
	}
	else
	{
		Log.Add(_MESSAGE_,_T("Sending authentication request..."));
		msg.type = rmtAuthRequest;
		msg.size = static_cast<int>(RELAY_MSG_HEAD_SIZE + (userId.length() + 1)*sizeof(TCHAR));
		_tcscpy_s(msg.data,MAX_MSG_SIZE,userId.c_str()); 
		stream->Send(reinterpret_cast<char*>(&msg),msg.size);
	}

	if(SC_AFTER_AUTH_REQUEST == stop)
		return;

	Log.Add(_MESSAGE_,_T("Receiving challenge request..."));
	stream->Receive(reinterpret_cast<char*>(&msg), RELAY_MSG_HEAD_SIZE);
	if (!msg.size || msg.size != RELAY_MSG_HEAD_SIZE + RELAY_CHALLENGE_SIZE)
		throw MCStreamException(_T("Invalid message size"));
	if (msg.type != rmtChallengeRequest)
		throw MCStreamException(_T("Invalid message type"));
	stream->Receive(msg.data,msg.size - RELAY_MSG_HEAD_SIZE);

	// Calculating hash
	std::auto_ptr<TCHAR> buffer;
	buffer.reset(new TCHAR[RELAY_CHALLENGE_SIZE + password.length()]);
	memcpy(buffer.get(),password.c_str(),password.length()*sizeof(TCHAR));
	memcpy(buffer.get()+password.length(),msg.data,RELAY_CHALLENGE_SIZE);
	char hash[RELAY_HASH_SIZE];
	CSingleton<CCrypto>::instance().MakeHash(	buffer.get(),
												(int)(RELAY_CHALLENGE_SIZE + password.length())*sizeof(TCHAR),
												hash,
												RELAY_HASH_SIZE);

	if(SC_BEFORE_CHALLENGE == stop)
	{
		Log.Add(_MESSAGE_,_T("Sending garbage instead of challenge response..."));
		stream->Send(buf.get(), BUFFER_SIZE);
	}
	else
	{
		Log.Add(_MESSAGE_,_T("Sending challenge response..."));
		msg.type = rmtChallengeResponceNoSession;
		msg.size = RELAY_MSG_HEAD_SIZE + RELAY_HASH_SIZE;
		memcpy(msg.data,hash,RELAY_HASH_SIZE);
		stream->Send(reinterpret_cast<char*>(&msg),msg.size);
	}

	if(SC_AFTER_CHALLENGE == stop)
		return;

	// Validate authentication
	msg.size = RELAY_MSG_HEAD_SIZE;
	try
	{
		Log.Add(_MESSAGE_,_T("Receiving authentication reply..."));
		stream->Receive(reinterpret_cast<char*>(&msg),msg.size);
		Log.Add(_MESSAGE_,_T("Authentication reply received"));
	}
	catch(CStreamException&)
	{
		throw MCStreamException(_T("Authentication failed"));
	}
	if (msg.type != rmtAuthSuccessfull)
		throw MCStreamException(_T("Relay authentication failed"));
	Log.Add(_MESSAGE_,"Authentication was successful");


	if(GetTickCount() % 2)
	{
		if(SC_BEFORE_CHECK_PORT == stop)
		{
			Log.Add(_MESSAGE_,_T("Sending garbage instead of port request..."));
			stream->Send(buf.get(), BUFFER_SIZE);
		}
		else
		{
			Log.Add(_MESSAGE_,"Send port number (%u) for checking by server...",exPort);
			msg.type = rmtCheckPort;
			msg.size = RELAY_MSG_HEAD_SIZE + RELAY_PORTCHECK_SIZE;
			*(reinterpret_cast<int*>(msg.data)) = exPort;
			stream->Send(reinterpret_cast<char*>(&msg),msg.size);
		}

		if(SC_AFTER_CHECK_PORT == stop)
			return;

		Log.Add(_MESSAGE_,"Receiving confirmation data...");
		stream->Receive(reinterpret_cast<char*>(&msg), RELAY_MSG_HEAD_SIZE);
		if (!msg.size || msg.size != RELAY_MSG_HEAD_SIZE + RELAY_PORTCHECK_SIZE)
			throw MCStreamException("Invalid message size");
		if (msg.type != rmtPortOpened)
			throw MCStreamException("Invalid message type");
		stream->Receive(msg.data,msg.size - RELAY_MSG_HEAD_SIZE);
		//Check received and sent data on an equality
		if ( static_cast<int>(exPort) != *(reinterpret_cast<int*>(msg.data)) )
			throw MCStreamException("Wrong answer from server side");
		Log.Add(_MESSAGE_,"All operation have done. The port is available...");
	}
	else
	{
		if(SC_BEFORE_CHECK_PORT == stop)
		{
			Log.Add(_MESSAGE_,_T("Sending garbage instead of external port request..."));
			stream->Send(buf.get(), BUFFER_SIZE);
		}
		else
		{
			Log.Add(_MESSAGE_,_T("Sending external port request..."));
			msg.type = rmtExternalIPRequest;
			msg.size = RELAY_MSG_HEAD_SIZE;
			stream->Send( reinterpret_cast<char*>(&msg), msg.size );
		}

		if(SC_AFTER_CHECK_PORT == stop)
			return;

		/// Receiving response
		SPeerAddr addr;
		stream->Receive(reinterpret_cast<char*>(&addr), sizeof(SPeerAddr));
	}

	if(SC_BEFORE_EXIT == stop)
	{
		Log.Add(_MESSAGE_,_T("Sending garbage at exit..."));
		stream->Send(buf.get(), BUFFER_SIZE);
	}

	Log.Add(_MESSAGE_,_T("SUCCESS"));

CATCH_LOG()
}

void CRelayConnector::StubEntryPoint()
{
TRY_CATCH
	boost::shared_ptr<CSSocket> socket(new CSSocket());
	if(!socket->Create())
		throw MCException_Win("Socket create failed");
	if(!socket->Bind(CSingleton<CSettings>::instance().m_extPort))
		throw MCException_Win("Bind failed");
	if(!socket->Listen())
		throw MCException_Win("Listen failed");
	while(!m_terminated)
	{
		if(socket->ReadSelect(100))
		{
			boost::scoped_ptr<CSSocket> sock(socket->Accept());
			if(sock.get())
			{
				Log.Add(_MESSAGE_,"Socket accepted");
				SRelayMessage msg;
				try
				{
					sock->Receive(reinterpret_cast<char*>(&msg), RELAY_MSG_HEAD_SIZE);
					if (	msg.size &&
						msg.size == RELAY_MSG_HEAD_SIZE + RELAY_PORTCHECK_SIZE &&
						msg.type == rmtCheckPort )
					{
						sock->Receive(msg.data,msg.size - RELAY_MSG_HEAD_SIZE);
						msg.type=rmtPortOpened;
						sock->Send(reinterpret_cast<char*>(&msg),msg.size);
						Log.Add(_MESSAGE_,"PortRequest confirmation sent");
					}
				}
				catch(...)
				{
				}
			}
		}
	}
CATCH_LOG()
}

void CRelayConnector::Start(const int count)
{
TRY_CATCH
	m_terminated = false;
	m_thread.reset(new boost::thread(boost::bind(&CRelayConnector::StubEntryPoint, this)));
	for(int i = 0; i < count; ++i)
	{
		tstring userId;
		unsigned int port;
		if(i%2)
			userId = Format(_T("%s%d"), CSingleton<CSettings>::instance().m_user.c_str(), i);
		else
			userId = CSingleton<CSettings>::instance().m_user;
		if(i%3)
			port = CSingleton<CSettings>::instance().m_extPort;
		else
			port = CSingleton<CSettings>::instance().m_extPort + 1;
		int stop = i % MAX_ESTOPCONNECT;
		m_pool->schedule(
			boost::bind(
				&CRelayConnector::ThreadEntryPoint, 
				this, 
				userId, 
				CSingleton<CSettings>::instance().m_passwd, 
				/*SC_NONE_STOP*/static_cast<EStopConnect>(stop), 
				port
			)
		);
	}
	m_pool->wait();
	m_terminated = true;
	m_thread->join();
CATCH_THROW()
}

