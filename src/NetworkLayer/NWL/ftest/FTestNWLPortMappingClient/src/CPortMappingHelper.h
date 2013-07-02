//////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CPortMappingHelper.h
///
///  Declares CPortMappingHelper class
///  Includes configuration parameters and provide methods for communication with GUI 
///
///  @author Alexander Novak @date 23.07.2007
///
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Streaming/CNetworkLayer.h>
#include <AidLib/CException/CException.h>
#include <vector>
#include "ini.h"
//////////////////////////////////////////////////////////////////////////////////////////

/// Collect data for connection to server and preferred port numbers
struct SPortMappingInfo
{
	tstring			m_server_address;
	unsigned short	m_server_port;
	tstring			m_user_id;
	tstring			m_user_password;
	unsigned short	m_user_external_port;
	unsigned short	m_user_internal_port;
	tstring			m_protocol;
	int				m_search_delay;
	int				m_count_to_try;
	bool			m_delete_mapping;
};
//////////////////////////////////////////////////////////////////////////////////////////

class CPortMappingHelper
{
	CIniFile m_ini_file;

	int m_currentInfoItemIndex;
	volatile LONG m_increment;
	volatile LONG m_count_active_thread;
	volatile LONG m_count_fail_thread;
	volatile LONG m_count_success_thread;
	
	CRITICAL_SECTION cs;

	CPortMappingHelper(const CPortMappingHelper&);
	CPortMappingHelper& operator=(const CPortMappingHelper&);
public:
	/*
		The ini-file structure

		[CONFIG]
		 PortMapping=1						//Test with port mapping configuration
		 CountToStart=1						//Count threads to start
		 SearchDelay=2000					//Time for searching UPnP device
		 CountToTry=3						//Count for change port numbers if it need
		 DeleteMapping=1					//Delete port mapping after all operations

		[SERVER]
		 Address=010.010.010.010			//Relay server address
		 Port=123							//Relay server port

		[AUTOGENUSER]
		 Name=NamePrefix
		 Password=UserLoginPassword
		 ExternalPort=
		 InternalPort=
		 Protocol=
		 
		[USERS]								
		 Section1=User1						//Section name with user data
		 Section2=User2						//If this section is empty then uses [AUTOGENUSER] section
		 ...

		[User1]
		 Name=UserLoginName					//User name for connect to relay server
		 Password=UserLoginPassword			//User password for connect to relay server
		 ExternalPort=80					//Prefer external port
		 InternalPort=81					//Prefer internal port
		 Protocol=TCP						//Protocol (TCP/UDP)
		
		[User2]
		 Name=UserLoginName
		 Password=UserLoginPassword		
		 ExternalPort=80
		 InternalPort=81
		 Protocol=TCP

		...
	*/
	CPortMappingHelper(const TCHAR* ansiIniFile);
	~CPortMappingHelper();

	/// Return data for thread which connect and check port availability
	/// @remarks	In cycles pick up one user info from the internal list
	SPortMappingInfo GetDataForPortMapping();

	/// Increment the m_count_active_thread member
	/// @remarks	Used for GUI
	void ThreadStartedNotify();

	/// Decrement the m_count_active_thread member, increment the m_count_fail_thread member
	/// @remarks	Used for GUI
	void ThreadFailedNotify();

	/// Decrement the m_count_active_thread member, increment the m_count_success_thread member
	/// @remarks	Used for GUI
	void ThreadSuccessedNotify();

	/// Gets count of active threads
	/// @return		Count of active threads
	/// @remarks	Used for GUI
	int GetCountActiveThread() const;

	/// Gets count of failed threads
	/// @return		Count of failed threads
	/// @remarks	Used for GUI
	int GetCountFailThread() const;

	/// Gets count of successed threads
	/// @return		Count of successed threads
	/// @remarks	Used for GUI
	int GetCountSuccessThread() const;
};
//----------------------------------------------------------------------------------------

inline void CPortMappingHelper::ThreadStartedNotify()
{
	InterlockedIncrement(&m_count_active_thread);
}
//----------------------------------------------------------------------------------------

inline int CPortMappingHelper::GetCountActiveThread() const
{
	return m_count_active_thread;
}
//----------------------------------------------------------------------------------------

inline int CPortMappingHelper::GetCountFailThread() const
{
	return m_count_fail_thread;
}
//----------------------------------------------------------------------------------------

inline int CPortMappingHelper::GetCountSuccessThread() const
{
	return m_count_success_thread;
}
//////////////////////////////////////////////////////////////////////////////////////////
