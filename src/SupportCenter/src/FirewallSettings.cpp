#include "StdAfx.h"
#include "FirewallSettings.h"
#include <AidLib/Logging/cLog.h> 
#include <AidLib/CException/CException.h>
#include <NWL/Streaming/COpenPortRequest.h>
#include <NWL/UPnP/CPortMapping.h>

#define DEFAULT_POTR_MAPPING_SEARCH_TIMEOUT 5000

CFirewallSettings::CFirewallSettings(void)
{
}

CFirewallSettings::~CFirewallSettings(void)
{
}

std::set<tstring> CFirewallSettings::GetLocalAddresses()
{
TRY_CATCH
	
	std::set<tstring> localAddresses;
	char name_buf[MAX_PATH];
	memset( name_buf, 0, MAX_PATH );
	// Get host name
	if ( !gethostname( name_buf, MAX_PATH ) )
	{
		// Get host info
		hostent* host = gethostbyname( name_buf );
		if ( host )
		{
			// Copy addresses
			char** addrs = host->h_addr_list;
			while ( *addrs )
			{
				// Get address
				char* str_addr = inet_ntoa( *((struct in_addr *)*addrs) );
				// Add address
				localAddresses.insert( str_addr );
				addrs++;
			}
		}
	}
	return localAddresses;	
CATCH_THROW("CStreamFactory::GetLocalAddresses")
}

void CFirewallSettings::ConfiguringPortOnRuterThroughUPnP(const int internalPort)
{
	CPortMapping portMapping;
	int externalPort = internalPort;
	unsigned long iLocalAddress = 0;
	EErrPortMapping ePMStatus = E_PM_SUCCESS;	

	//
	//	PortMapping Search
	//
	if( E_PM_SUCCESS != portMapping.Search(DEFAULT_POTR_MAPPING_SEARCH_TIMEOUT)) //timeout in ms TODO
	{
		throw MCException("Failed to find UPnP router"); //TODO
	}

	//
	//  Retriving local addresses
	//
	std::set<tstring> localAddresses = GetLocalAddresses();
	tstring addressStr;

	//	
	//	Loop for all local localAddresses till SUCCESS or last in list
	//
	for(std::set<tstring>::const_iterator addr = localAddresses.begin(); addr != localAddresses.end(); addr++)
	{
		iLocalAddress = inet_addr((*addr).c_str());
		if(E_PM_SUCCESS == portMapping.AddMapping(externalPort, iLocalAddress, internalPort))
			break;	
	}

	if(ePMStatus!=E_PM_SUCCESS)
		throw MCException("Failed to add port mapping"); //TODO
}

void CFirewallSettings::TestPortAvailabilityWithRelayD(
	tstring jabberUserName,
	tstring relayServerAddress,
	unsigned short relayServerPort,
	const int internalPort)
{
	int externalPort = internalPort;

	COpenPortRequest request(relayServerAddress.c_str(), relayServerPort);

	request.CheckPortAvailability(
		jabberUserName, 
		NWL_INSTANCE.GetRelayPasswd(), 
		internalPort,  //internal port 
		externalPort); //external port
}

int  CFirewallSettings::GetCustomDirectStreamPort() const
{
	return NWL_INSTANCE.GetCustomDirectStreamPort();
}

int CFirewallSettings::GetDefaultDirectStreamPort() const 
{
	return NWL_INSTANCE.GetDefaultDirectStreamPort();
}

EPortSelectionMode CFirewallSettings::GetDirectStreamPortMode() const 
{
	return NWL_INSTANCE.GetDirectStreamPortMode();
}

void  CFirewallSettings::SetCustomDirectStreamPort(const int directStreamPort) 
{
	NWL_INSTANCE.SetCustomDirectStreamPort(directStreamPort, true);
}

void CFirewallSettings::SetDirectStreamPortMode(EPortSelectionMode portMode) 
{
	NWL_INSTANCE.SetDirectStreamPortMode(portMode, true);
}









