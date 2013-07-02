/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFirewallConfigurator.h
///
///  Declares CFirewallConfigurator class, responsible for allowing incoming
///    TCP connections for application on XP, 2003
///
///  @author Dmitry Netrebenko @date 18.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <AidLib/Strings/tstring.h>
#include <NWL/NetworkLayer.h>
#include <AidLib/Loki/Singleton.h>
#include <AidLib/CCritSection/CCritSectionObject.h>
#include <map>
#include <boost/shared_ptr.hpp>
#include <NWL/Streaming/CFirewallWrapper.h>
#include <NWL/Streaming/SAuthAppSettings.h>

/// CFirewallConfigurator class, responsible for allowing incoming
///   TCP connections for application on XP, 2003
class NWL_API CFirewallConfigurator
{
private:
/// Prevents making copies of CFirewallConfigurator objects.
	CFirewallConfigurator( const CFirewallConfigurator& );
	CFirewallConfigurator& operator=( const CFirewallConfigurator& );
public:
/// Constructor
	CFirewallConfigurator();
/// Destructor
	~CFirewallConfigurator();
private:
/// Checks windows version
/// @return true if windows version is higher than 2000
	bool OSVersionHigherThan2000() const;
public:
/// Allows incoming connections for application
/// @param  full application file name
	void AllowIncomingForApplication(const tstring&);
/// Allows incoming connections for current application
	void AllowIncoming();
/// Restore previous registry settings for application
/// @param  full application file name
	void RestoreSettingsForApplication(const tstring&);
/// Restore previous registry settings for current application
	void RestoreSettings();
/// Restore previous registry settings for all applications from history
	void RestoreAll();
private:
/// Stores current process file name
	tstring									m_curProcessFileName;
/// Critical section for registry modifications
	CCritSectionSimpleObject				m_allowSection;
/// History of changes
	std::map<tstring,SPAuthAppSettings>		m_history;
/// Firewall object
	CFirewallWrapper						m_firewall;
/// Is exceptions are allowed in firewall's rules	
	bool									m_exceptionsAllowed;
};

/// Should be used to access firewall configurator as single instance
#define FIREWALL_CONFIGURATOR_INSTANCE Loki::SingletonHolder<CFirewallConfigurator, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
