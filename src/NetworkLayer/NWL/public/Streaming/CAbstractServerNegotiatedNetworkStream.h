/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAbstractServerNegotiatedNetworkStream.h
///
///  Declares CAbstractServerNegotiatedNetworkStream class - abstract class
///    for streams which negotiates using server
///
///  @author Dmitry Netrebenko @date 24.11.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CAbstractNetworkStream.h"
#include "CRelayConnectSettings.h"

///  CAbstractServerNegotiatedNetworkStream class - abstract class
///    for streams which negotiates using server
///  Base class CAbstractNetworkStream - abstract network stream with
///  Base class CRelayConnectSettings - settings for connection to relay server
///  @remarks
class CAbstractServerNegotiatedNetworkStream :
	public CAbstractNetworkStream,
	public CRelayConnectSettings
{
private:
/// Prevents making copies of CAbstractServerNegotiatedNetworkStream objects.
	CAbstractServerNegotiatedNetworkStream( const CAbstractServerNegotiatedNetworkStream& );
	CAbstractServerNegotiatedNetworkStream& operator=( const CAbstractServerNegotiatedNetworkStream& );

public:
///  Constructor
	CAbstractServerNegotiatedNetworkStream();

///  Destructor
	virtual ~CAbstractServerNegotiatedNetworkStream();

};
