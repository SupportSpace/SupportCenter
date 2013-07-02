/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CConnectEvents.cpp
///
///  Implements CConnectEvents class, responsible for connection events
///
///  @author Dmitry Netrebenko @date 10.10.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CConnectEvents.h>
#include <AidLib/CException/CException.h>
#include <AidLib/Logging/cLog.h>

CConnectEvents::CConnectEvents()
: m_ConnectedEvent( NULL ), m_DisconnectedEvent( NULL ),
m_ConnectErrorEvent( NULL )
{
TRY_CATCH

CATCH_THROW("CConnectEvents::CConnectEvents")
}

CConnectEvents::~CConnectEvents()
{
TRY_CATCH

CATCH_LOG("CConnectEvents::~CConnectEvents")
}

NotifyEvent CConnectEvents::GetConnectedEvent() const
{
TRY_CATCH

	return m_ConnectedEvent;

CATCH_THROW("CConnectEvents::GetConnectedEvent")
}

void CConnectEvents::SetConnectedEvent( NotifyEvent event )
{
TRY_CATCH

	m_ConnectedEvent = event;

CATCH_THROW("CConnectEvents::SetConnectedEvent")
}

NotifyEvent CConnectEvents::GetDisconnectedEvent() const
{
TRY_CATCH

	return m_DisconnectedEvent;

CATCH_THROW("CConnectEvents::GetDisconnectedEvent")
}

void CConnectEvents::SetDisconnectedEvent( NotifyEvent event )
{
TRY_CATCH

	m_DisconnectedEvent = event;

CATCH_THROW("CConnectEvents::SetDisconnectedEvent")
}

ConnectErrorEvent CConnectEvents::GetConnectErrorEvent() const
{
TRY_CATCH

	return m_ConnectErrorEvent;

CATCH_THROW("CConnectEvents::GetConnectErrorEvent")
}

void CConnectEvents::SetConnectErrorEvent( ConnectErrorEvent event )
{
TRY_CATCH

	m_ConnectErrorEvent = event;

CATCH_THROW("CConnectEvents::SetConnectErrorEvent")
}
