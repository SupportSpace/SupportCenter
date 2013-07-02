/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSocketStream.cpp
///
///  Implements CSocketStream class, responsible for CSSocket streams.
///
///  @author Dmitry Netrebenko @date 11.10.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CSSocket.h>
#include <NWL/Streaming/CSocketStream.h>
#include <NWL/Streaming/CStreamException.h>
#include <NWL/Streaming/CSocketClientConnectThread.h>
#include <NWL/Streaming/CSocketServerConnectThread.h>
#include <NWL/Streaming/CSocketProxyClientConnectThread.h>
#include <NWL/Streaming/CFirewallConfigurator.h>
#include <NWL/Streaming/CNetworkLayer.h>
#include <AidLib/CSingleton/CSingleton.h>

CSocketStream::CSocketStream()
	:	CAbstractStream(),
		CConnectEvents(),
		m_strHost( _T("") ),
		m_nPort( 0 ),
		m_ReceiveCancelled( false ),
		m_sSocket(),
		m_sDataSocket(),
		m_StreamState( ssDisconnected ),
		m_ConnectThread(),
		m_ProxySettings(),
		m_bConnectThroughProxy( false ),
		m_bBlockTransportError( false ),
		m_sendTimeOutIsSet( false ),
		m_useSendTimeout( NWL_INSTANCE.GetUseSendTimeout() )
{
TRY_CATCH

	// Create new CSSocket object
	m_sSocket.reset(new CSSocket());

	if ( !m_sSocket->Create() )
		throw MCSocketStreamException( _T("Failed to m_sSocket->Create") );

	// Set read timeout
	if (!m_sSocket->SetTimeout( 1000, sstReceive ))
		throw MCSocketStreamException( _T("Failed to m_sSocket->SetTimeout"));

	TRY_CATCH
		// Configure firewall for incoming connections
		FIREWALL_CONFIGURATOR_INSTANCE.AllowIncoming();
	CATCH_LOG("CSocketStream::CSocketStream. Failed to allow incomming connects for windows firewall")

CATCH_THROW("CSocketStream::CSocketStream")
}

CSocketStream::CSocketStream( SPSocket sock )
	:	CAbstractStream(), 
		CConnectEvents(),
		m_strHost( _T("") ), 
		m_nPort( 0 ), 
		m_ReceiveCancelled( false ),
		m_sSocket(sock), 
		m_sDataSocket(sock), 
		m_StreamState( ssConnected ),
		m_ConnectThread(), 
		m_ProxySettings(), 
		m_bConnectThroughProxy( false ),
		m_bBlockTransportError( false ),
		m_useSendTimeout( NWL_INSTANCE.GetUseSendTimeout() )
{
TRY_CATCH

	// Check up CSSocket object
	if ( !m_sSocket.get() )
		throw MCStreamException( _T("Socket wasn't created") );
	if ( !m_sDataSocket.get() )
		throw MCStreamException( _T("Socket wasn't created") );

	m_sSocket->SetTimeout( 1000, sstReceive );

	TRY_CATCH
		// Configure firewall for incoming connections
		FIREWALL_CONFIGURATOR_INSTANCE.AllowIncoming();
	CATCH_LOG("CSocketStream::CSocketStream. Failed to allow incomming connects for windows firewall")

CATCH_THROW("CSocketStream::CSocketStream( CSSocket* )")
}

CSocketStream::~CSocketStream()
{
TRY_CATCH

	// Terminate 

	DWORD ThreadId = 0;

	if ( m_ConnectThread.get() )
	{
		ThreadId = m_ConnectThread->GetTid();
		m_ConnectThread->Terminate();
	}

	// Close socket if exists
	if(m_sDataSocket.get())
		m_sDataSocket->Close();

	// Destroy CSSocket
	m_sSocket->Close();

	if ( m_ConnectThread.get() )
	{
		if ( ( GetCurrentThreadId() != ThreadId ) && ThreadId )
			WaitForSingleObject(m_ConnectThread->hTerminatedEvent.get(),INFINITE);
	}

	// Destroy thread
	m_ConnectThread.reset();

CATCH_LOG("CSocketStream::~CSocketStream")
}

void CSocketStream::Connect( const tstring& strHost, const unsigned int& nPort, bool sync )
{
TRY_CATCH

	// Check state
	if ( ssDisconnected !=  m_StreamState )
		throw MCStreamException( Format(_T("Socket already connected. Current state is %d"), m_StreamState) );
	
	// Store parameters
	m_strHost = strHost;
	m_nPort = nPort;

	// Destroy connect thread and additional socket
	FreeObjects();

	// Create new thread for client connection
	if ( m_bConnectThroughProxy )
	{
		CSocketProxyClientConnectThread* thread = new CSocketProxyClientConnectThread();
		thread->SetProxySettings( m_ProxySettings );
		m_ConnectThread.reset(thread);
	}
	else
		m_ConnectThread.reset(new CSocketClientConnectThread());

	if (!sync)
	{

		// Change state
		m_StreamState = ssConnecting;

		// Start thread
		StartConnectThread();
	} else
	{
		// Initialize connect thread
		m_ConnectThread->Init( 
			m_sSocket, 
			boost::bind( &CSocketStream::OnSocketConnected, this, _1 ),
			boost::bind( &CSocketStream::OnSocketConnectError, this, _1, _2 ), 
			m_strHost, 
			m_nPort );

		// Start thread
		m_ConnectThread->Execute(reinterpret_cast<void*>(true));
		m_StreamState = ssConnected;
	}

CATCH_THROW("CSocketStream::Connect")
}

void CSocketStream::Disconnect()
{
TRY_CATCH

	// Check state
	if ( ssDisconnected == m_StreamState )
		throw MCStreamException( _T("Socket already disconnected") );

	// Raise "Disconnect" event
	RaiseDisconnectedEvent();

CATCH_THROW("CSocketStream::Disconnect")
}

void CSocketStream::Accept( const unsigned int& nPort )
{
TRY_CATCH

	// Check state
	if ( ssDisconnected !=  m_StreamState )
		throw MCStreamException( _T("Socket already connected") );

	// Store Port parameter
	m_strHost = _T("");
	m_nPort = nPort;

	// Destroy connect thread and additional socket
	FreeObjects();

	// Create new thread for server connection
	m_ConnectThread.reset(new CSocketServerConnectThread());

	// Start thread
	StartConnectThread();

	// Change state
	m_StreamState = ssConnecting;

CATCH_THROW("CSocketStream::Accept")
}

void CSocketStream::CancelReceiveOperation()
{
TRY_CATCH

	if (m_sDataSocket)
		m_sDataSocket->ReadSelect(100);

	// Set "Cancelled" flag
	m_ReceiveCancelled = true;

CATCH_THROW("CSocketStream::CancelReceiveOperation)")
}

bool CSocketStream::HasInData()
{
TRY_CATCH

	// Check stream state and socket
	if ( m_sDataSocket.get() && ( ssConnected ==  m_StreamState ) )
		return m_sDataSocket->ReadSelect(0);
	else
		return false;

CATCH_THROW("CSocketStream::HasInData")
}

unsigned int CSocketStream::ReceiveInternal( char* pbuffer, const unsigned int& bufsize )
{
TRY_CATCH

	if ( m_ReceiveCancelled )
	{
		m_ReceiveCancelled = false;
		throw MCStreamException( _T("Cancelled") );
	}

	// Check buffer
	CheckBuffer( pbuffer );

	// Check state
	if ( ssConnected != m_StreamState )
		throw MCStreamException( Format(_T("Socket not connected. Current state is %d"), m_StreamState) );

	int result;
	unsigned int i = 0;
	for( i = 0; i < bufsize; )
	{
		result = m_sDataSocket->Receive( pbuffer + i, bufsize - i );
		if ( result > 0 )
		{
			i += result;
		}
		else 
		{
			int WSAError = WSAGetLastError();
			if ( ( SOCKET_ERROR == result ) && ( WSAETIMEDOUT == WSAError ) )
			{
				if ( m_ReceiveCancelled )
				{
					m_ReceiveCancelled = false;
					throw MCStreamException( _T("Cancelled") );
				}
			} 
			else
			{
				if ( !m_bBlockTransportError )
				{
					RaiseDisconnectedEvent();
					if (0 != result)
						throw MCStreamException_Win( _T("ReadError"), WSAError);
					else
						throw MCStreamException(_T("ReadError: the connection has been gracefully closed"));
				}
				else
					break;
			}
		}
	}

	if ( m_ReceiveCancelled )
	{
		m_ReceiveCancelled = false;
		throw MCStreamException( _T("Cancelled") );
	}

	return i;

CATCH_THROW("CSocketStream::ReceiveInternal")
}

unsigned int CSocketStream::SendInternal( const char* pbuffer, const unsigned int& bufsize )
{
TRY_CATCH

	// Check buffer
	CheckBuffer( pbuffer );

	// Check state
	if ( ssConnected !=  m_StreamState )
		throw MCStreamException( Format(_T("Socket not connected. Current state is %d"), m_StreamState) );

	if ( !bufsize ) 
		return 0;

	// Setting timeout if necessary
	TriggerSendTimeout(bufsize);

	unsigned int ret = m_sDataSocket->Send( pbuffer, bufsize );

	if ( (ret != bufsize) && !m_bBlockTransportError )
	{
		int err = WSAGetLastError();
		if (m_useSendTimeout && m_sendTimeOutIsSet)
			Log.Add(_WARNING_,_T("Send timeout is turned on"));
		RaiseDisconnectedEvent();
		throw MCStreamException_Win( _T("Failed to send"), err );
	}

	return ret;

CATCH_THROW("CSocketStream::SendInternal")
}

void CSocketStream::StartConnectThread()
{
TRY_CATCH

	// Initialize connect thread
	m_ConnectThread->Init( 
		m_sSocket, 
		boost::bind( &CSocketStream::OnSocketConnected, this, _1 ),
		boost::bind( &CSocketStream::OnSocketConnectError, this, _1, _2 ), 
		m_strHost, 
		m_nPort );

	// Start thread
	m_ConnectThread->Start();

CATCH_THROW("CSocketStream::StartConnectThread")
}

void CSocketStream::CheckBuffer( const char* buf )
{
TRY_CATCH

	// Check buffer
	if ( !buf ) 
		throw MCStreamException( _T("buf == NULL") );

CATCH_THROW("CSocketStream::CheckBuffer")
}

void CSocketStream::FreeObjects()
{
TRY_CATCH

	// Destroy thread
	m_ConnectThread.reset();

	// Destroy socket if exists
	m_sDataSocket.reset();

CATCH_THROW("CSocketStream::FreeObjects")
}

SHTTPProxySettings& CSocketStream::GetProxySettings()
{
TRY_CATCH

	return m_ProxySettings;

CATCH_THROW("CSocketStream::GetProxySettings")
}
	
void CSocketStream::SetProxySettings( const SHTTPProxySettings& proxy )
{
TRY_CATCH

	m_ProxySettings = proxy;

CATCH_THROW("CSocketStream::SetProxySettings")
}

bool CSocketStream::GetConnectThroughProxy() const
{
TRY_CATCH

	return m_bConnectThroughProxy;

CATCH_THROW("CSocketStream::GetConnectThroughProxy")
}
	
void CSocketStream::SetConnectThroughProxy( const bool useproxy )
{
TRY_CATCH

	m_bConnectThroughProxy = useproxy;

CATCH_THROW("CSocketStream::SetConnectThroughProxy")
}

void CSocketStream::Send2Queue( const char* pbuffer, const unsigned int& bufsize )
{
TRY_CATCH

	// Check buffer
	CheckBuffer( pbuffer );

	// Check state
	if ( ssConnected !=  m_StreamState )
		throw MCStreamException( Format(_T("Socket not connected. Current state is %d"), m_StreamState) );

	CAbstractStream::Send2Queue( pbuffer, bufsize );

CATCH_THROW("CSocketStream::Send2Queue")
}

void CSocketStream::OnSocketConnected( void* )
{
TRY_CATCH

	// Get socket to read/write data
	m_sDataSocket = m_ConnectThread->GetConnectedSocket();

	// Change state
	m_StreamState = ssConnected;

	tstring peerAddr = m_sDataSocket->GetPeerName();
	unsigned int peerPort = m_sDataSocket->GetPeerPort();
	tstring sockAddr = m_sDataSocket->GetSockName();
	unsigned int sockPort = m_sDataSocket->GetSockPort();
	Log.Add(_MESSAGE_, _T("Socket connected local(%s:%d) remote(%s:%d)"), sockAddr.c_str(), sockPort, peerAddr.c_str(), peerPort);

	//Raise "Connected" event
	RaiseConnectedEvent();

	m_sDataSocket->SetTimeout( 1000, sstReceive );

CATCH_THROW("CSocketStream::OnSocketConnected")
}

void CSocketStream::OnSocketConnectError( void*, EConnectErrorReason reason )
{
TRY_CATCH

	// Change state
	m_StreamState = ssDisconnected;

	//Raise "Connect Error" event
	RaiseConnectErrorEvent( reason );

CATCH_THROW("CSocketStream::OnSocketConnectError")
}

void CSocketStream::RaiseConnectedEvent()
{
TRY_CATCH

	if ( m_ConnectedEvent )
		m_ConnectedEvent( this );

CATCH_THROW("CSocketStream::RaiseConnectedEvent")
}

void CSocketStream::RaiseDisconnectedEvent()
{
TRY_CATCH

	// Terminate connection thread
	if ( m_ConnectThread.get() && ( ssConnecting ==  m_StreamState ) )
	{
		m_ConnectThread->Terminate();
		m_ConnectThread->Stop(false);
	}

	// Close socket
	if ( ( m_sDataSocket.get() != m_sSocket.get() ) && m_sDataSocket.get() )
		m_sDataSocket->Close();

	m_sSocket->Create(); 

	if ( m_DisconnectedEvent )
		m_DisconnectedEvent( this );

	// Change state
	m_StreamState = ssDisconnected;

CATCH_THROW("CSocketStream::RaiseDesconnectedEvent")
}

void CSocketStream::RaiseConnectErrorEvent( EConnectErrorReason reason )
{
TRY_CATCH

	if ( m_ConnectErrorEvent )
		m_ConnectErrorEvent( this, reason );

CATCH_THROW("CSocketStream::RaiseConnectErrorEvent")
}

void CSocketStream::Close()
{
TRY_CATCH

	DWORD threadId = 0;
	if ( m_ConnectThread.get() )
	{
		threadId = m_ConnectThread->GetTid();
		m_ConnectThread->Terminate();
	}

	// Destroy socket if exists
//	m_sDataSocket.reset();
	if(m_sDataSocket.get())
		m_sDataSocket->Close();

	m_sSocket->Create();

	if ( m_ConnectThread.get() )
	{
		if ( threadId && (GetCurrentThreadId() != threadId) )
			WaitForSingleObject(m_ConnectThread->hTerminatedEvent.get(),INFINITE);
	}

	m_StreamState = ssDisconnected;

CATCH_THROW("CSocketStream::Close")
}
