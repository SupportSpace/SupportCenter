/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFirewallConfigurator.cpp
///
///  Implements CFirewallConfigurator class, responsible for allowing incoming
///    TCP connections for application on XP, 2003
///
///  @author Dmitry Netrebenko @date 18.10.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CFirewallConfigurator.h>
#include <NWL/Streaming/CStreamException.h>
#include <AidLib/Logging/cLog.h>
#include <AidLib/CCritSection/CCritSection.h>

CFirewallConfigurator::CFirewallConfigurator()
	:	m_curProcessFileName(_T(""))
{
TRY_CATCH
	/// Get full file name of current process
	TCHAR buf[MAX_PATH];
	if(!GetModuleFileName(NULL, buf, MAX_PATH))
		throw MCStreamException("Can not obtain module name.");
	m_curProcessFileName = buf;
CATCH_THROW()
}

CFirewallConfigurator::~CFirewallConfigurator()
{
TRY_CATCH
CATCH_LOG()
}

void CFirewallConfigurator::AllowIncomingForApplication(const tstring& filename)
{
TRY_CATCH
	/// Windows version less or equals 2000
	if(!OSVersionHigherThan2000())
		return;
	tstring lowFileName = LowerCase(filename);
	/// Enter critical section
	CCritSection section(&m_allowSection);
	/// Search file name in history
	if(m_history.find(lowFileName) != m_history.end())
	{
		Log.Add(_MESSAGE_, _T("Firewall already configured for this application"));
		return;
	}
	if(!m_history.size())
		m_exceptionsAllowed = m_firewall.EnableExceptions(true);
	m_history[lowFileName] = m_firewall.AuthorizeApplication(filename);
CATCH_THROW()
}

bool CFirewallConfigurator::OSVersionHigherThan2000() const
{
TRY_CATCH
	OSVERSIONINFO verInfo;
	verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(TRUE == GetVersionEx(&verInfo))
		return ((verInfo.dwMajorVersion == 5) && (verInfo.dwMinorVersion > 0)) || (verInfo.dwMajorVersion > 5);
	else
		return false;
CATCH_THROW()
}

void CFirewallConfigurator::AllowIncoming()
{
TRY_CATCH
	AllowIncomingForApplication(m_curProcessFileName);	
CATCH_THROW()
}

void CFirewallConfigurator::RestoreSettingsForApplication(const tstring& filename)
{
TRY_CATCH
	tstring lowFileName = LowerCase(filename);
	/// Enter critical section
	CCritSection section(&m_allowSection);
	/// Search for application in history
	std::map<tstring,SPAuthAppSettings>::iterator index = m_history.find(lowFileName);
	if(index == m_history.end())
	{
		Log.Add(_MESSAGE_, _T("Firewall is not configured for this application"));
		return;
	}
	/// Get previous settings from history
	SPAuthAppSettings settings = index->second;
	m_history.erase(index);
	m_firewall.SetApplicationSettings(filename, settings);
	if(!m_history.size())
		m_firewall.EnableExceptions(m_exceptionsAllowed);
CATCH_THROW()
}

void CFirewallConfigurator::RestoreSettings()
{
TRY_CATCH
	RestoreSettingsForApplication(m_curProcessFileName);
CATCH_THROW()
}

void CFirewallConfigurator::RestoreAll()
{
TRY_CATCH
	/// Enter critical section
	CCritSection section(&m_allowSection);
	/// Restore settings for all applications
	while(m_history.size())
	{
		std::map<tstring,SPAuthAppSettings>::iterator index = m_history.begin();
		tstring appName = index->first;
		RestoreSettingsForApplication(appName);
	}
CATCH_THROW()
}

