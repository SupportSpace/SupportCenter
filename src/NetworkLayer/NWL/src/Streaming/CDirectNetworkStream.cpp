/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CDirectNetworkStream.cpp
///
///  Implements CDirectNetworkStream class, responsible for direct end-to-end stream
///
///  @author Dmitry Netrebenko @date 10.10.2006
///
////////////////////////////////////////////////////////////////////////


#include <NWL/Streaming/CDirectNetworkStream.h>
#include <NWL/Streaming/CStreamException.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <boost/scoped_ptr.hpp>

CDirectNetworkStream::CDirectNetworkStream()
	:	CAbstractNetworkStream()
	,	CHTTPProxy()
	,	CTLSAuthSettings()
	,	m_DirectStream( dsNone )
	,	m_hConnectEvent( NULL )
	,	m_LastConnectError( cerNoError )
	,	m_nRemotePort( 0 )
	,	m_strRemoteAddr( _T("") )
	,	m_nLocalPort( 0 )
{
TRY_CATCH

	// Initialize critical section
	InitializeCriticalSection(
		&m_csConnectSection);

	// Create event object
	m_hConnectEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL );
 
	if ( !m_hConnectEvent )
		throw MCStreamException( _T("Event creation failed") );
/*
	m_pClientStream.reset( new CTLSSocketStream() );
	m_pServerStream.reset( new CTLSSocketStream() );
	m_pActiveStream.reset();
*/
CATCH_THROW("CDirectNetworkStream::CDirectNetworkStream")
}

CDirectNetworkStream::~CDirectNetworkStream()
{
TRY_CATCH

	DestroyStreams();

	// Close critical section
	DeleteCriticalSection(
		&m_csConnectSection);

	// Destroy event
	CloseHandle( m_hConnectEvent );

CATCH_LOG("CDirectNetworkStream::~CDirectNetworkStream")
}

void CDirectNetworkStream::Connect( const bool AsyncConnect )
{
TRY_CATCH

	// Check state
	if ( dsNone != m_DirectStream )
		throw MCStreamException( _T("Connection already established") );

	// Checking for loopback connections
	if (m_nLocalPort == m_nRemotePort)
	{
		/// Checking RemoteAddr to be really remote address, not one of local
	}
/*	
	m_pClientStream->Close();
	m_pServerStream->Close();
*/
	m_pClientStream.reset( new CTLSSocketStream() );
	m_pServerStream.reset( new CTLSSocketStream() );
	m_pActiveStream.reset();

	// Initialize streams
	m_nErrorReports = 0;
	m_bAuthFailed = false;

	InitStreams();

	m_LastConnectError = cerNoError;

	m_pServerStream->Accept( m_nLocalPort );
	m_pClientStream->Connect( m_strRemoteAddr, m_nRemotePort );

	ResetEvent( m_hConnectEvent );

	if (AsyncConnect)
	{
		// Do nothing
	} else
	if ( WAIT_TIMEOUT == WaitForSingleObject( m_hConnectEvent, m_nConnectTimeout ) )
	{

/// This block of code creates deadlock at TLS handshake if other side skips TLS authentication
		//// Enter critical section
		//boost::scoped_ptr<CCritSection> section( new CCritSection( &m_csConnectSection ) );

		//if ( dsNone == m_DirectStream )
		//{

		//	RaiseConnectErrorEvent( cerTimeout );

		//	m_DirectStream = dsClient;

		//	m_pClientStream->Close();
		//	m_pServerStream->Close();

		//	m_DirectStream = dsNone;
		//}

		RaiseConnectErrorEvent( cerTimeout );
		m_pClientStream->Close();
		m_pServerStream->Close();
		m_DirectStream = dsNone;
	}

	switch ( m_LastConnectError )
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

CATCH_THROW("CDirectNetworkStream::Connect")
}

void CDirectNetworkStream::Disconnect()
{
TRY_CATCH

	// Check connection
	CheckStream();

	// Disconnect
	m_pActiveStream->Disconnect();

CATCH_THROW("CDirectNetworkStream::Disconnect")
}

unsigned int CDirectNetworkStream::ReceiveInternal( char* buf, const unsigned int& len )
{
TRY_CATCH

	// Check connection
	CheckStream();

	// Receive data
	m_pActiveStream->Receive( buf, len );

	return len;

CATCH_THROW("CDirectNetworkStream::ReceiveInternal")
}

unsigned int CDirectNetworkStream::SendInternal( const char* buf, const unsigned int& len )
{
TRY_CATCH

	// Check connection
	CheckStream();

	// Send data
	m_pActiveStream->Send( buf, len );

	return len;

CATCH_THROW("CDirectNetworkStream::SendInternal")
}

unsigned int CDirectNetworkStream::GetInBuffer( char* buf, const unsigned int& len)
{
TRY_CATCH
	
	// Check connection
	CheckStream();

	// Send data
	return m_pActiveStream->GetInBuffer(buf,len);

CATCH_THROW("CDirectNetworkStream::GetInBuffer")
}

void CDirectNetworkStream::CancelReceiveOperation()
{
TRY_CATCH

	// Cancel read from the stream
	if ( m_pActiveStream.get() )
		m_pActiveStream->CancelReceiveOperation();

CATCH_THROW("CDirectNetworkStream::CancelReceiveOperation")
}

bool CDirectNetworkStream::HasInData()
{
TRY_CATCH

	// Check connection
	CheckStream();

	// Check data in the stream
	return m_pActiveStream->HasInData();

CATCH_THROW("CDirectNetworkStream::HasInData")
}

void CDirectNetworkStream::InitStreams()
{
TRY_CATCH

	m_pActiveStream.reset();

	m_pClientStream->SetProxySettings( m_ProxySettings );
	m_pClientStream->SetConnectThroughProxy( m_bConnectThroughProxy );
	m_pClientStream->SetSuite( m_Suite );
	m_pClientStream->SetCredentials(m_Credentials);
	m_pClientStream->SetConnectedEvent( boost::bind( &CDirectNetworkStream::OnConnected, this, _1 ) );
	m_pClientStream->SetConnectErrorEvent( boost::bind( &CDirectNetworkStream::OnConnectError, this, _1, _2 ) );

	m_pServerStream->SetSuite( m_Suite );
	m_pServerStream->SetCredentials(m_Credentials);
	m_pServerStream->SetConnectedEvent( boost::bind( &CDirectNetworkStream::OnConnected, this, _1 ) );
	m_pServerStream->SetConnectErrorEvent( boost::bind( &CDirectNetworkStream::OnConnectError, this, _1, _2 ) );

CATCH_THROW("CDirectNetworkStream::InitStreamSettings")
}

void CDirectNetworkStream::P2PAuthenticate()
{
TRY_CATCH

	// Check connection
	CheckStream();

	switch ( m_DirectStream )
	{
	case dsClient:
		m_pActiveStream->InitSecureConnection(false);
		break;

	case dsServer:
		m_pActiveStream->InitSecureConnection(true);
		break;

	default:
		break;

	}

CATCH_THROW("CDirectNetworkStream::P2PAuthenticate")
}

void CDirectNetworkStream::CheckStream()
{
TRY_CATCH

	if ( ( dsNone == m_DirectStream ) || !m_pActiveStream.get() )
		throw MCStreamException( _T("Connection not established") );

CATCH_THROW("CDirectNetworkStream::CheckStream")
}

void CDirectNetworkStream::OnConnected( void* param )
{
TRY_CATCH

	CTLSSocketStream*	pInactiveStream = NULL;
	CTLSSocketStream*	pConnectedStream = dynamic_cast<CTLSSocketStream*>(reinterpret_cast<CSocketStream*>(param));
	if(!pConnectedStream)
		throw MCStreamException( _T("Invalid event parameter") );

	{
		// Enter critical section
		CCritSection cs(&m_csConnectSection );

		if ( dsNone != m_DirectStream )
			return;

		if ( pConnectedStream == m_pServerStream.get() )
		{
			pInactiveStream = m_pClientStream.get();
			m_pActiveStream = m_pServerStream;
			m_DirectStream = dsServer;
		}
		else if ( pConnectedStream == m_pClientStream.get() )
		{
			pInactiveStream = m_pServerStream.get();
			m_pActiveStream = m_pClientStream;
			m_DirectStream = dsClient;
		}
		else
			throw MCStreamException( _T("Invalid event parameter") );
	}

	if ( pInactiveStream )
		pInactiveStream->Close();

	m_nErrorReports = 1;

	P2PAuthenticate();

	m_pActiveStream->SetDisconnectedEvent( boost::bind( &CDirectNetworkStream::OnDisconnected, this, _1 ) );

	RaiseConnectedEvent();

	SetEvent( m_hConnectEvent );

CATCH_THROW("CDirectNetworkStream::OnConnected")
}

void CDirectNetworkStream::OnConnectError( void* param, EConnectErrorReason reason )
{
TRY_CATCH

	switch( reason )
	{
	case cerAuthFailed:
		RaiseConnectErrorEvent( reason );
		m_DirectStream = dsNone;
		m_bAuthFailed = true;
		break;

	default:
		CTLSSocketStream*	pErrorStream = (CTLSSocketStream*)param;

		// Enter critical section
		CCritSection cs(&m_csConnectSection);
	
		if ( dsNone != m_DirectStream )
			return;

		m_nErrorReports++;

		if ( 2 == m_nErrorReports )
		{
			SetEvent( m_hConnectEvent );
			if ( !m_bAuthFailed )
				RaiseConnectErrorEvent( reason );

			m_pClientStream->Close();
			m_pServerStream->Close();
		}
	}

CATCH_THROW("CDirectNetworkStream::OnConnectError")
}

void CDirectNetworkStream::OnDisconnected( void* )
{
TRY_CATCH

//	CCritSection section( &m_csSendSection );
	if (dsNone == m_DirectStream)
	{
		Log.Add(_WARNING_,_T("CDirectNetworkStream::OnDisconnected from not connected state"));
		return;
	}

	if (m_pClientStream.get())
		m_pClientStream->Close();
	if (m_pServerStream.get())
		m_pServerStream->Close();

	RaiseDisconnectedEvent();

	m_DirectStream = dsNone;

CATCH_THROW("CDirectNetworkStream::OnDisconnected")
}

void CDirectNetworkStream::DestroyStreams()
{
TRY_CATCH

	// Cancelling connect wait if we have such
	SetEvent(m_hConnectEvent);
	m_LastConnectError = cerCancelled;

	// Destroy streams
	if(m_pServerStream.get())
		m_pServerStream->Close();
	if(m_pClientStream.get())
		m_pClientStream->Close();

	m_pServerStream.reset();
	m_pClientStream.reset();
	m_pActiveStream.reset();

CATCH_THROW("CDirectNetworkStream::DestroyStreams")
}

void CDirectNetworkStream::RaiseConnectErrorEvent( EConnectErrorReason reason )
{
TRY_CATCH

	m_LastConnectError = reason;

	CAbstractNetworkStream::RaiseConnectErrorEvent( reason );

CATCH_THROW("CDirectNetworkStream::RaiseConnectErrorEvent")
}

void CDirectNetworkStream::SetLocalAddr( const unsigned int& LocalPort )
{
TRY_CATCH

	m_nLocalPort = LocalPort;

CATCH_THROW("CDirectNetworkStream::SetLocalAddr")
}

void CDirectNetworkStream::SetRemoteAddr( const tstring& RemoteAddr, const unsigned int& RemotePort )
{
TRY_CATCH

	m_strRemoteAddr = RemoteAddr;
	m_nRemotePort = RemotePort;

CATCH_THROW("CDirectNetworkStream::SetRemoteAddr")
}

unsigned int CDirectNetworkStream::GetRemotePort() const
{
TRY_CATCH

	return m_nRemotePort;

CATCH_THROW("CDirectNetworkStream::GetRemotePort")
}

tstring CDirectNetworkStream::GetRemoteAddr() const
{
TRY_CATCH

	return m_strRemoteAddr;

CATCH_THROW("CDirectNetworkStream::GetRemoteAddr")
}

unsigned int CDirectNetworkStream::GetLocalPort() const
{
TRY_CATCH

	return m_nLocalPort;

CATCH_THROW("CDirectNetworkStream::GetLocalPort")
}

bool CDirectNetworkStream::Connected() const
{
TRY_CATCH
	return !( ( dsNone == m_DirectStream ) || !m_pActiveStream.get() );
CATCH_THROW("CDirectNetworkStream::Connected")
}

EDirectStreamType CDirectNetworkStream::GetStreamType() const
{
TRY_CATCH

	return m_DirectStream;

CATCH_THROW("CDirectNetworkStream::GetStreamType")
}

