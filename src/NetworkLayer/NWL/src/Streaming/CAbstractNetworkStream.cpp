/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAbstractNetworkStream.cpp
///
///  Implements CAbstractNetworkStream class, abstract class for network streams
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CAbstractNetworkStream.h>
#include <AidLib/CException/CException.h>
#include <AidLib/Logging/cLog.h>

CAbstractNetworkStream::CAbstractNetworkStream()
: CAbstractStream(), CConnectEvents(),
m_nConnectTimeout( DEFAULT_CONNECT_TIMEOUT )
{
TRY_CATCH

CATCH_THROW("CAbstractNetworkStream::CAbstractNetworkStream")
}

CAbstractNetworkStream::~CAbstractNetworkStream()
{
TRY_CATCH

CATCH_LOG("CAbstractNetworkStream::~CAbstractNetworkStream")
}

void CAbstractNetworkStream::RaiseConnectedEvent()
{
TRY_CATCH

	if ( m_ConnectedEvent )
		m_ConnectedEvent( this );

CATCH_THROW("CAbstractNetworkStream::RaiseConnectedEvent")
}

void CAbstractNetworkStream::RaiseDisconnectedEvent()
{
TRY_CATCH

	if ( m_DisconnectedEvent )
		m_DisconnectedEvent( this );

CATCH_THROW("CAbstractNetworkStream::RaiseDisconnectedEvent")
}

void CAbstractNetworkStream::RaiseConnectErrorEvent( EConnectErrorReason reason )
{
TRY_CATCH

	if ( m_ConnectErrorEvent )
		m_ConnectErrorEvent( this, reason );

CATCH_THROW("CAbstractNetworkStream::RaiseConnectErrorEvent")
}

unsigned int CAbstractNetworkStream::GetConnectTimeout() const
{
TRY_CATCH

	return m_nConnectTimeout;

CATCH_THROW("CAbstractNetworkStream::GetConnectTimeout")
}

void CAbstractNetworkStream::SetConnectTimeout( const unsigned int& timeout )
{
TRY_CATCH

	if ( !timeout )
		m_nConnectTimeout = INFINITE;
	else
		m_nConnectTimeout = timeout;

CATCH_THROW("CAbstractNetworkStream::SetConnectTimeout")
}

