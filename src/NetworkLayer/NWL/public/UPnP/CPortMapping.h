//////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CPortMapping.h
///
///  Declares CPortMapping class.
///  Allow port mapping on devices with UPnP support
///
///  @author Alexander Novak @date 25.06.2007
///
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <NWL/NetworkLayer.h>

#pragma comment(lib,"miniUPnP.lib")

//========================================================================================

//Error codes for object's methods
enum EErrPortMapping
{
	E_PM_SUCCESS=0,					//Operation was successful
	E_PM_NOUPNPDEVICES,				//UPnP device wasn't discovered
	E_PM_NOVALIDDEVICES,			//InternetGatewayDevice wasn't found
	E_PM_PORTISBUSY					//Port is busy for mapping, try to use another one
};
//////////////////////////////////////////////////////////////////////////////////////////

class NWL_API CPortMapping
{
	std::string m_ctrlUrl;
	std::string m_srvcType;
	char m_localAddress[16];
	EErrPortMapping m_deviceFound;
	
	CPortMapping(const CPortMapping&);
	CPortMapping& operator=(const CPortMapping&);
public:
	CPortMapping();
	virtual ~CPortMapping();

	/// Search upnp device in network
	/// @param delay	Time for discovering wait
	/// @return		Return error codes are listed above
	EErrPortMapping Search(int delay = 2000);

	/// Add new port mapping rule
	/// @param exPort		WAN external port
	/// @param inAddress	LAN address
	/// @param inPort		LAN port
	/// @param protocol		Name of protocol
	/// @return		Return error codes are listed above
	/// @remarks	If an inAddress parameter is zero, when the local address
	/// @remarks	directly connected to an InternetGatewayDevice is used
	EErrPortMapping AddMapping(	const char* exPort,
								const char* inAddress,
								const char* inPort,
								const char* protocol = "TCP");
	EErrPortMapping AddMapping(	unsigned short exPort,
								unsigned long inAddress,
								unsigned short inPort,
								const char* protocol = "TCP");

	/// Delete existing port mapping rule
	/// @param exPort		WAN external port
	/// @param protocol		Name of protocol
	void DeleteMapping(const char* exPort, const char* protocol = "TCP");
	void DeleteMapping(unsigned short exPort, const char* protocol = "TCP");
};
//////////////////////////////////////////////////////////////////////////////////////////
