/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAbstractNetworkStream.h
///
///  Declares CAbstractNetworkStream class, abstract class for network streams
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CAbstractStream.h"
#include "CConnectEvents.h"
#include <AidLib/Strings/tstring.h>
#include <NWL/NetworkLayer.h>

#define DEFAULT_CONNECT_TIMEOUT 1000

///  Abstract class for network streams
///  Base class CAbstractStream - abstract stream class
///  @remarks
class NWL_API CAbstractNetworkStream 
	:	public virtual CAbstractStream
	,	public virtual CConnectEvents
{
private:
/// Prevents making copies of CAbstractNetworkStream objects.
	CAbstractNetworkStream( const CAbstractNetworkStream& );
	CAbstractNetworkStream& operator=( const CAbstractNetworkStream& );

public:
///  Constructor
	CAbstractNetworkStream();
	
///  Destructor
	virtual ~CAbstractNetworkStream();

///  Connect to remote host
///  @param   do asynchronous connect
///  @remarks
	virtual void Connect( const bool = false ) = NULL;

///  Disconnect from remote host
///  @remarks
	virtual void Disconnect() = NULL;

protected:

/// Connection timeout (msec)
	unsigned int			m_nConnectTimeout;

public:

///  Returns connection timeout
///  @return msec
///  @remarks
	unsigned int GetConnectTimeout() const;

///  Sets new connection timeout
///  @param   new timeout in msecs
///  @remarks
	void SetConnectTimeout( const unsigned int& );


/// Returns true if connected, false othervay
/// @return true if connected, false othervay
	virtual bool Connected() const = NULL;

protected:
///  Raises "Connected" event
///  @remarks
	virtual void RaiseConnectedEvent();

///  Raises "Disconnected" event
///  @remarks
	virtual void RaiseDisconnectedEvent();

///  Raises "Connect Error" event
///  @param Error reason
///  @remarks
	virtual void RaiseConnectErrorEvent( EConnectErrorReason );

};
