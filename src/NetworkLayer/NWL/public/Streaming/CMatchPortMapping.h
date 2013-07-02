//////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CMatchPortMapping.h
///
///  Declares CMatchPortMapping class
///  Search UPnP device in the network and match free ports to do port mapping
///  Do test for the port availability
///
///  @author Alexander Novak @date 20.07.2007
///
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/UPnP/CPortMapping.h>
#include <NWL/Streaming/COpenPortRequest.h>

#define UNSIGNED_SHORT_MAX			( ( 1<<(sizeof(unsigned short)*8) )-1 )
//////////////////////////////////////////////////////////////////////////////////////////

class NWL_API CMatchPortMapping
{
	CPortMapping			m_port_mapping;
	COpenPortRequest		m_open_port_request;
	int						m_search_delay;
	int						m_count_try;
	static volatile LONG	m_search_in_process;

	CMatchPortMapping(const COpenPortRequest&);
	CMatchPortMapping& operator=(const COpenPortRequest&);
public:
	/// @param serverAddress	Server address
	/// @param serverPort		Server port
	/// @param searchDelay		Search deleay for UPnP device finding
	/// @param countTry			Count of ports binding if they are busy
	/// @remarks	If ports are busy they are selected in random order
	CMatchPortMapping(	const tstring& serverAddress,
						unsigned short serverPort,
						int searchDelay,
						int countTry);
	~CMatchPortMapping();

	/// Try to do the port mapping and check ports for availability
	/// Return in a preferenceExPort and in a preferenceInPort ports which are real mapped
	/// @param userID				User identifier
	/// @param password				User password
	/// @param preferenceExPort		The preference external port
	/// @param inAddress			LAN address
	/// @param preferenceInPort		The preference internal port
	/// @param protocol				Name of protocol
	/// @return		Return true if operation was successful
	/// @return		Return false if discovering used by other thread
	/// @remarks	If an inAddress parameter is zero, when the local address
	/// @remarks	directly connected to an InternetGatewayDevice is used
	bool CreateMappedPorts(	const tstring& userID,
							const tstring& password,
							unsigned short* preferenceExPort,
							unsigned long inAddress,
							unsigned short* preferenceInPort,
							const char* protocol = "TCP");

	/// Delete the port mapping and check if that has done
	/// @param userID				User identifier
	/// @param password				User password
	/// @param exPort				External port
	/// @param protocol				Name of protocol
	void DeleteMappedPorts(	const tstring& userID,
							const tstring& password,
							unsigned short exPort,
							unsigned short inPort,
							const char* protocol = "TCP");
};
//////////////////////////////////////////////////////////////////////////////////////////
