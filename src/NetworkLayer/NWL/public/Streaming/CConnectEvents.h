/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CConnectEvents.h
///
///  Declares CConnectEvents class, responsible for connection events
///
///  @author Dmitry Netrebenko @date 10.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Events/Events.h>
#include <NWL/NetworkLayer.h>

///  CConnectEvents class, responsible for connection events
///  @remarks
class NWL_API CConnectEvents
{
private:
/// Prevents making copies of CConnectEvents objects.
	CConnectEvents( const CConnectEvents& );
	CConnectEvents& operator=( const CConnectEvents& );

public:
///  Constructor
	CConnectEvents();

///  Destructor
	~CConnectEvents();

protected:
/// Connected event
	NotifyEvent			m_ConnectedEvent;
/// Disconnected event
	NotifyEvent			m_DisconnectedEvent;
/// Connection Error event
	ConnectErrorEvent	m_ConnectErrorEvent;

protected:
///  Raises "Connected" event
///  @remarks
	virtual void RaiseConnectedEvent() = NULL;

///  Raises "Disconnected" event
///  @remarks
	virtual void RaiseDisconnectedEvent() = NULL;

///  Raises "Connection Error" event
///  @param Error reason
///  @remarks
	virtual void RaiseConnectErrorEvent( EConnectErrorReason ) = NULL;

public:
///  Returns "Connected" event
///  @return event
///  @remarks
	NotifyEvent GetConnectedEvent() const;

///  Sets new "Connected" event
///  @param   new event
///  @remarks
	void SetConnectedEvent( NotifyEvent );

///  Returns "Disconnected" event
///  @return event
///  @remarks
	NotifyEvent GetDisconnectedEvent() const;

///  Sets new "Disconnected" event
///  @param   new event
///  @remarks
	void SetDisconnectedEvent( NotifyEvent );

///  Returns "Connection Error" event
///  @return event
///  @remarks
	ConnectErrorEvent GetConnectErrorEvent() const;

///  Sets new "Connection Error" event
///  @param   new event
///  @remarks
	void SetConnectErrorEvent( ConnectErrorEvent );

};
