/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAbstractServerNegotiatedNetworkStream.cpp
///
///  Implements CAbstractServerNegotiatedNetworkStream class - abstract class
///    for streams which negotiates using server
///
///  @author Dmitry Netrebenko @date 24.11.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CAbstractServerNegotiatedNetworkStream.h>
#include <AidLib/Logging/cLog.h>

CAbstractServerNegotiatedNetworkStream::CAbstractServerNegotiatedNetworkStream()
	:	CAbstractNetworkStream()
	,	CRelayConnectSettings()
{
TRY_CATCH
CATCH_THROW()
}

CAbstractServerNegotiatedNetworkStream::~CAbstractServerNegotiatedNetworkStream()
{
TRY_CATCH
CATCH_LOG()
}
