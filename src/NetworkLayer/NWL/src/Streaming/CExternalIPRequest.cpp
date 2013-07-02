/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CExternalIPRequest.h
///
///  cass for requesting external peer IP
///
///  @author "Archer Software" Dmitry Netrebenko @date 20.11.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CExternalIPRequest.h>
#include <AidLib/Logging/cLog.h>
#include <NWL/Streaming/CStreamException.h>
#include <NWL/Streaming/CSocketStream.h>
#include <NWL/Streaming/CNetworkLayer.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <AidLib/CSingleton/CSingleton.h>
#include <AidLib/CCrypto/CCrypto.h>

CExternalIPRequest::CExternalIPRequest( const tstring& ServerAddr, const unsigned int& ServerPort )
: m_sock( stTCP ), m_strServerAddr( ServerAddr ), m_nServerPort( ServerPort )
{
TRY_CATCH
	InitializeCriticalSection(&m_cs);
	// Create event object
	m_hConnectEvent = CreateEvent(
									NULL,
									TRUE,
									FALSE,
									NULL );
 
	if ( !m_hConnectEvent )
		throw MCStreamException( _T("Failed to CreateEvent") );

CATCH_THROW("CExternalIPRequest::CExternalIPRequest")
}

CExternalIPRequest::~CExternalIPRequest()
{
TRY_CATCH
	DeleteCriticalSection(&m_cs);
	CloseHandle( m_hConnectEvent );
	m_sock.Close();
CATCH_LOG("CExternalIPRequest::~CExternalIPRequest")
}

SPeerAddr CExternalIPRequest::GetExternalAddress(const tstring& userId, const tstring& passwd)
{
TRY_CATCH
	m_sock.Create();
	if ( m_sock.Connect( m_strServerAddr, m_nServerPort) )
	{

		SRelayMessage msg;
		/// Sending auth request
		msg.type = rmtAuthRequest;
		strcpy_s(msg.data, MAX_MSG_SIZE, userId.c_str());
		msg.size = static_cast<int>(RELAY_MSG_HEAD_SIZE + userId.length() + 1);
		Log.Add(_MESSAGE_,"Sending auth request...");
		if(!m_sock.Send(reinterpret_cast<char*>(&msg),msg.size))
			throw MCStreamException("Sending auth message failed");
		Log.Add(_MESSAGE_,"Receiving challenge request...");
		/// Retriving challenge request
		if(!m_sock.Receive(reinterpret_cast<char*>(&msg), RELAY_MSG_HEAD_SIZE))
			throw MCStreamException("Receiving challenge message header failed");
		if(!msg.size || msg.size != RELAY_MSG_HEAD_SIZE + RELAY_CHALLENGE_SIZE)
			throw MCStreamException("Invalid message size");
		if(msg.type != rmtChallengeRequest)
			throw MCStreamException("Invalid message type");
		if(!m_sock.Receive(msg.data, msg.size - RELAY_MSG_HEAD_SIZE))
			throw MCStreamException("Receiving challenge message body failed");
		Log.Add(_MESSAGE_,"Challenge request received");
		/// Calculating hash
		std::auto_ptr<char> buf;
		buf.reset(new char[RELAY_CHALLENGE_SIZE + passwd.length()]);
		memcpy(buf.get(), passwd.c_str(), passwd.length());
		memcpy(buf.get() + passwd.length(), msg.data, RELAY_CHALLENGE_SIZE);
		char hash[RELAY_HASH_SIZE];
		CRYPTO_INSTANCE.MakeHash(buf.get(),RELAY_CHALLENGE_SIZE + (unsigned int)passwd.length(), hash, RELAY_HASH_SIZE);
		/// Sending challenge response
		msg.type = rmtChallengeResponceNoSession;
		msg.size = RELAY_MSG_HEAD_SIZE + sizeof(hash);
		memcpy(msg.data, hash, sizeof(hash));
		Log.Add(_MESSAGE_,"Sending challenge response...");
		if(!m_sock.Send(reinterpret_cast<char*>(&msg), msg.size))
			throw MCStreamException("Sending challenge response message failed");
		Log.Add(_MESSAGE_,"Shallenge response sent");
		/// Is auth succeeded?
		msg.size = RELAY_MSG_HEAD_SIZE;
		try
		{
			Log.Add(_MESSAGE_,"Receiving auth reply...");
			if(!m_sock.Receive(reinterpret_cast<char*>(&msg), msg.size))
				throw MCStreamException("Receiving auth reply message failed");
			Log.Add(_MESSAGE_,"Auth reply received");
		}
		catch(CStreamException&)
		{
			throw MCStreamException("Auth failed");
		}
		if (msg.type != rmtAuthSuccessfull)
			throw MCStreamException("Relay auth failed");
		/// Auth succeeded, sending request
		msg.type = rmtExternalIPRequest;
		msg.size = RELAY_MSG_HEAD_SIZE;
		if(!m_sock.Send( reinterpret_cast<char*>(&msg), msg.size ))
			throw MCStreamException("Sending ip request message failed");

		/// Receiving response
		SPeerAddr addr;
		if(!m_sock.Receive( reinterpret_cast<char*>(&addr), sizeof(SPeerAddr) ))
			throw MCStreamException("Receiving external address failed");

		return addr;
	}
	throw MCStreamException("Failed to connect relay server");
CATCH_THROW("CExternalIPRequest::GetExternalAddress")
}

SPeerAddr CExternalIPRequest::GetExternalAddressThroughProxy(const tstring& userId, const tstring& passwd, const int timeOut)
{
TRY_CATCH
	
	CCritSection cs(&m_cs); //Only one concurent call to this method
	CSocketStream socketStream;
	socketStream.SetProxySettings(NWL_INSTANCE.GetProxySettings());
	socketStream.SetConnectThroughProxy(true);
	ResetEvent( m_hConnectEvent );
	m_lastConnectError = cerNoError;
	socketStream.SetConnectedEvent( boost::bind( &CExternalIPRequest::OnConnected, this, _1 ) );
	socketStream.SetConnectErrorEvent( boost::bind( &CExternalIPRequest::OnConnectError, this, _1, _2 ) );

	socketStream.Connect( m_strServerAddr, m_nServerPort );
	if ( WAIT_TIMEOUT == WaitForSingleObject( m_hConnectEvent, timeOut ) )
	{
		/// Waiting for connected or error occuered
		throw MCStreamException("Connect wait failed");
	}
	switch(m_lastConnectError)
	{
		case cerUnknown:
			throw MCStreamException( _T("Unknown connection error") );

		case cerWinError:
			throw MCStreamException( _T("WinError occured") );

		case cerAuthFailed:
			throw MCStreamException( _T("Authentication failed") );

		case cerTimeout:
			throw MCStreamException( _T("Connection timeout expired") );

		case cerCancelled:
			throw MCStreamException( _T("Connect cancelled") );
	}
	SRelayMessage msg;
	/// Sending auth request
	msg.type = rmtAuthRequest;
	strcpy_s(msg.data, MAX_MSG_SIZE, userId.c_str());
	msg.size = static_cast<int>(RELAY_MSG_HEAD_SIZE + userId.length() + 1);
	Log.Add(_MESSAGE_,"Sending auth request...");
	socketStream.Send(reinterpret_cast<char*>(&msg),msg.size);
	Log.Add(_MESSAGE_,"Receiving challenge request...");
	/// Retriving challenge request
	socketStream.Receive(reinterpret_cast<char*>(&msg), RELAY_MSG_HEAD_SIZE);
	if(!msg.size || msg.size != RELAY_MSG_HEAD_SIZE + RELAY_CHALLENGE_SIZE)
		throw MCStreamException("Invalid message size");
	if(msg.type != rmtChallengeRequest)
		throw MCStreamException("Invalid message type");
	socketStream.Receive(msg.data, msg.size - RELAY_MSG_HEAD_SIZE);
	Log.Add(_MESSAGE_,"Challenge request received");
	/// Calculating hash
	std::auto_ptr<char> buf;
	buf.reset(new char[RELAY_CHALLENGE_SIZE + passwd.length()]);
	memcpy(buf.get(), passwd.c_str(), passwd.length());
	memcpy(buf.get() + passwd.length(), msg.data, RELAY_CHALLENGE_SIZE);
	char hash[RELAY_HASH_SIZE];
	CRYPTO_INSTANCE.MakeHash(buf.get(),RELAY_CHALLENGE_SIZE + (unsigned int)passwd.length(), hash, RELAY_HASH_SIZE);
	/// Sending challenge response
	msg.type = rmtChallengeResopnce;
	msg.size = RELAY_MSG_HEAD_SIZE + sizeof(hash);
	memcpy(msg.data, hash, sizeof(hash));
	Log.Add(_MESSAGE_,"Sending challenge response...");
	socketStream.Send(reinterpret_cast<char*>(&msg), msg.size);
	Log.Add(_MESSAGE_,"Shallenge response sent");
	/// Is auth succeeded?
	msg.size = RELAY_MSG_HEAD_SIZE;
	try
	{
		Log.Add(_MESSAGE_,"Receiving auth reply...");
		socketStream.Receive(reinterpret_cast<char*>(&msg), msg.size);
		Log.Add(_MESSAGE_,"Auth reply received");
	}
	catch(CStreamException&)
	{
		throw MCStreamException("Auth failed");
	}
	if (msg.type != rmtAuthSuccessfull)
		throw MCStreamException("Relay auth failed");
	/// Auth succeeded, sending request
	msg.type = rmtExternalIPRequest;
	msg.size = RELAY_MSG_HEAD_SIZE;
	socketStream.Send( reinterpret_cast<char*>(&msg), msg.size );

	/// Receiving response
	SPeerAddr addr;
	socketStream.Receive( reinterpret_cast<char*>(&addr), sizeof(SPeerAddr) );

	return addr;

CATCH_THROW("CExternalIPRequest::GetExternalAddressThroughProxy")
}

void CExternalIPRequest::OnConnected( void* param )
{
TRY_CATCH
	SetEvent( m_hConnectEvent );
CATCH_LOG("CExternalIPRequest::OnConnected")
}

void CExternalIPRequest::OnConnectError( void* param, EConnectErrorReason reason )
{
TRY_CATCH
	SetEvent( m_hConnectEvent );
	m_lastConnectError = reason;
CATCH_LOG("CExternalIPRequest::OnConnectError")
}