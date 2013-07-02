/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CGatewayInfo.cpp
///
///  Implements CGatewayInfo class for obtaining gateway device information
///
///  @author Dmitry Netrebenko @date 08.10.2007
///
////////////////////////////////////////////////////////////////////////

#include <NWL/UPnP/CGatewayInfo.h>
#include <NWL/Streaming/CStreamException.h>
#include <MiniUPnP/miniupnpc.h>
#include <boost/shared_ptr.hpp>

CGatewayInfo::CGatewayInfo()
{
TRY_CATCH
CATCH_THROW()
}

CGatewayInfo::~CGatewayInfo()
{
TRY_CATCH
CATCH_LOG()
}

void CGatewayInfo::GetGatewayDeviceInfo(SGatewayInfo* info, int delay)
{
TRY_CATCH
	if(!info)
		throw MCStreamException(_T("The structure for information is not allocated."));
	boost::shared_ptr<UPNPDev> devList(upnpDiscover(delay), freeUPNPDevlist);
	if(devList.get())
	{
		//Some UPnP devices have been found
		UPNPUrls igdUrls;
		IGDdatas igdData;
		char localAddress[16];
		if (UPNP_GetValidIGD(devList.get(), &igdUrls, &igdData, localAddress, sizeof(localAddress))==1)
		{
			//A valid connected IGD has been found
			FreeUPNPUrls (&igdUrls);
			memset(info, 0, sizeof(SGatewayInfo));
			size_t sz = min(MINIUPNPC_URL_MAXSIZE, GATEWAY_INFO_FIELD_SIZE);

			memcpy(info->m_manufacturer, igdData.deviceInfo.m_manufacturer, sz);
			memcpy(info->m_manURL, igdData.deviceInfo.m_manURL, sz);
			memcpy(info->m_model, igdData.deviceInfo.m_model, sz);
			memcpy(info->m_modelDesc, igdData.deviceInfo.m_modelDesc, sz);
			memcpy(info->m_modelURL, igdData.deviceInfo.m_modelURL, sz);
			memcpy(info->m_name, igdData.deviceInfo.m_name, sz);
		}
		else
			throw MCStreamException(_T("No valid gateway devices were found."));
	}
	else
		throw MCStreamException(_T("No gateway devices were found."));
CATCH_THROW()
}

