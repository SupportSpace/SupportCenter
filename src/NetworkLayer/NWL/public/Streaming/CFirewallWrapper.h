/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFirewallWrapper.h
///
///  Declares CFirewallWrapper class, responsible for firewall management
///
///  @author Dmitry Netrebenko @date 27.12.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <AidLib/Strings/tstring.h>
#include <NWL/NetworkLayer.h>
#include <atlbase.h>
#include <netfw.h>
#include <NWL/Streaming/SAuthAppSettings.h>
#include <AidLib/Com/CInterfaceMarshaler.h>

/// CFirewallWrapper class, responsible for firewall management
class NWL_API CFirewallWrapper
{
private:
/// Prevents making copies of CFirewallWrapper objects.
	CFirewallWrapper( const CFirewallWrapper& );
	CFirewallWrapper& operator=( const CFirewallWrapper& );
public:
/// Constructor
	CFirewallWrapper();
/// Destructor
	~CFirewallWrapper();
/// Authorizes application in firewall
/// @param fileName - full application file name
/// @return shared pointer to structure with previous settings of application
	SPAuthAppSettings AuthorizeApplication(const tstring& fileName);
/// Removes application from firewall exceptions
/// @param fileName - full application file name
	void RemoveApplication(const tstring& fileName);
/// Enables/disables firewall exceptions
/// @param enable - exceptions are allowed
/// @return previuos state
	bool EnableExceptions(const bool enable);
/// Extracts process name from file name
/// @param fileName - full application file name
/// @return process name
	static tstring GetProcessName(const tstring& fileName);
/// Sets application settings
/// @param fileName - full application file name
/// @param settings - new application settings. if settings == NULL application will be removed from firewall
/// @return shared pointer to structure with previous settings of application
	SPAuthAppSettings SetApplicationSettings(const tstring& fileName, SPAuthAppSettings settings);
private:
/// Interface of manager
	CInterfaceMarshaler<INetFwMgr>		m_manager;
/// Interface of the profile
	CInterfaceMarshaler<INetFwProfile>	m_profile;
private:
/// Returns structure of application settings
/// @param app - interface of authorized application
	SPAuthAppSettings GetApplicationSettings(CComPtr<INetFwAuthorizedApplication> app);
/// Updates settings of application
/// @param app - interface of authorized application
/// @param settings - new application settings
	void SetApplicationSettings(CComPtr<INetFwAuthorizedApplication> app, SPAuthAppSettings settings);
};
