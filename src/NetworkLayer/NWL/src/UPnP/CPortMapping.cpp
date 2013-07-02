
#include <AidLib/CException/CException.h>
#include <NWL/UPnP/CPortMapping.h>
#include <MiniUPnP/miniupnpc.h>
#include <MiniUPnP/upnpcommands.h>

// CPortMapping [BEGIN] //////////////////////////////////////////////////////////////////

CPortMapping::CPortMapping()
	:	m_ctrlUrl(""),
		m_srvcType(""),
		m_deviceFound(E_PM_NOUPNPDEVICES)
{
TRY_CATCH
CATCH_LOG()
}
//----------------------------------------------------------------------------------------

CPortMapping::~CPortMapping()
{
TRY_CATCH
CATCH_LOG()
}
//----------------------------------------------------------------------------------------

EErrPortMapping CPortMapping::Search(int delay)
{
TRY_CATCH
	//Each search operation resets object state
	m_deviceFound	= E_PM_NOUPNPDEVICES;
	m_ctrlUrl		= ("");
	m_srvcType		= ("");

	if (UPNPDev* devList=upnpDiscover(delay))
	{
		//Some UPnP devices have been found
		UPNPUrls igdUrls;
		IGDdatas igdData;
		
		if (UPNP_GetValidIGD(devList,&igdUrls,&igdData,m_localAddress,sizeof(m_localAddress))==1)
		{
			//A valid connected IGD has been found
			m_ctrlUrl		= igdUrls.controlURL;
			m_srvcType		= igdData.servicetype;
			m_deviceFound	= E_PM_SUCCESS;
			
			FreeUPNPUrls (&igdUrls);
		}
		else
			m_deviceFound=E_PM_NOVALIDDEVICES;

		freeUPNPDevlist(devList);
	}
	return m_deviceFound;
CATCH_THROW()
}
//----------------------------------------------------------------------------------------

EErrPortMapping CPortMapping::AddMapping(	const char* exPort,
											const char* inAddress,
											const char* inPort,
											const char* protocol)
{
TRY_CATCH
	if (m_deviceFound!=E_PM_SUCCESS)
		return m_deviceFound;
	
	if (UPNP_AddPortMapping(	m_ctrlUrl.c_str(),
								m_srvcType.c_str(),
								exPort,
								inPort,
								(inAddress)?inAddress:m_localAddress,
								"",
								protocol))
		return E_PM_SUCCESS;
	
	//TODO: Test return values and check why error was been
	return 	E_PM_PORTISBUSY;
CATCH_THROW()
}
//----------------------------------------------------------------------------------------

EErrPortMapping CPortMapping::AddMapping(	unsigned short exPort,
											unsigned long inAddress,
											unsigned short inPort,
											const char* protocol)
{
TRY_CATCH
	char strExPort[7];
	char strInPort[7];
	_itoa_s(exPort,strExPort,sizeof(strExPort),10);
	_itoa_s(inPort,strInPort,sizeof(strInPort),10);
	
	in_addr netFormatInAddress;
	netFormatInAddress.s_addr=inAddress;
	
	return AddMapping(strExPort,(inAddress)?inet_ntoa(netFormatInAddress):0,strInPort,protocol);
CATCH_THROW()
}
//----------------------------------------------------------------------------------------

void CPortMapping::DeleteMapping(const char* exPort, const char* protocol)
{
TRY_CATCH
	if (m_deviceFound!=E_PM_SUCCESS)
		return;
	
	UPNP_DeletePortMapping(m_ctrlUrl.c_str(),m_srvcType.c_str(),exPort, protocol);
CATCH_THROW()
}
//----------------------------------------------------------------------------------------

void CPortMapping::DeleteMapping(unsigned short exPort, const char* protocol)
{
TRY_CATCH
	char strExPort[7];
	_itoa_s(exPort,strExPort,sizeof(strExPort),10);
	
	DeleteMapping(strExPort,protocol);	
CATCH_LOG()
}
// CPortMapping [END] ////////////////////////////////////////////////////////////////////
