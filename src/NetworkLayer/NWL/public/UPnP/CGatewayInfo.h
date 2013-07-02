/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CGatewayInfo.h
///
///  Declares CGatewayInfo class for obtaining gateway device information
///
///  @author Dmitry Netrebenko @date 08.10.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>
#include <NWL/UPnP/SGatewayInfo.h>
#include <AidLib/Loki/Singleton.h>
#include <NWL/NetworkLayer.h>

///  CGatewayInfo class for obtaining gateway device information
class NWL_API CGatewayInfo
{
private:
/// Prevents making copies of CGatewayInfo objects
	CGatewayInfo(const CGatewayInfo&);
	CGatewayInfo& operator=(const CGatewayInfo&);
public:
/// Constructor
	CGatewayInfo();
/// Destructor
	~CGatewayInfo();
/// Searches gateway device and obtains information
/// @param info - pointer to gateway's information structure
/// @param delay - discovery delay in msecs
	void GetGatewayDeviceInfo(SGatewayInfo* info, int delay = 2000);
};

/// Should be used to CStatisticClient as single instance
#define GATEWAY_INFO_INSTANCE Loki::SingletonHolder<CGatewayInfo, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
