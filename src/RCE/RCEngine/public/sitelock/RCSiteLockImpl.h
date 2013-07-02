/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  RCSiteLockImpl.h
///
///  Declares CRCSiteLockImpl template
///
///  @author Dmitry Netrebenko @date 26.12.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlctl.h>
#include "sitelock.h"
#include "hardcode.h"


#ifdef ALLOW_INTRANET_ZONE
	#define ALLOWED_ARRAY_DECL static const SiteList rgslTrustedSites[ALLOWED_DOMAINS_COUNT+2];
#else
	#define ALLOWED_ARRAY_DECL static const SiteList rgslTrustedSites[ALLOWED_DOMAINS_COUNT];
#endif

#ifdef ALLOW_INTRANET_ZONE
	#define ALLOWED_ARRAY_IMPL template<typename T>\
		const IObjectSafetySiteLock::SiteList CRCSiteLockImpl<T>::rgslTrustedSites[ALLOWED_DOMAINS_COUNT+2] =\
		{\
			ALLOWED_DOMAINS,\
			ALLOWED_ENTRY(SITELOCK_INTRANET_ZONE),\
			ALLOWED_ENTRY_S(SITELOCK_INTRANET_ZONE)\
		};
#else
	#define ALLOWED_ARRAY_IMPL template<typename T>\
		const IObjectSafetySiteLock::SiteList CRCSiteLockImpl<T>::rgslTrustedSites[ALLOWED_DOMAINS_COUNT] =\
		{\
			ALLOWED_DOMAINS\
		};
#endif


template <typename T>
class ATL_NO_VTABLE CRCSiteLockImpl
	:		public IObjectSafetySiteLockImpl<T, INTERFACESAFE_FOR_UNTRUSTED_CALLER|INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:

	ALLOWED_ARRAY_DECL;

	CRCSiteLockImpl() {};

};
// ZONE ABILITY TORN OFF see sitelock.h look for "ZONE ABILITY TORN OFF"
ALLOWED_ARRAY_IMPL;

/// module names declaration
#define RCUI_NAME _T("RCUI")
#define FTUI_NAME _T("FTUI")
#define RCINSTALLER_NAME _T("RCInstaller")
#define CoFAViewerNAME _T("CoFAViewer")

#include <set>
#include <AidLib/Strings/tstring.h>
#include <AidLib/Logging/cLog.h>
/// Class for reporting module name
class CReportModuleName
{
private:
	/// Already reported modules
	std::set<tstring> m_reportedModules;
public:
	void ReportModule(tstring module)
	{
		if (m_reportedModules.find(module) == m_reportedModules.end())
		{
#ifdef _DEBUG
	#if defined(_DYNAMIC_NWL) || defined(_DYNAMIC_AID_)
			PTCHAR config=_T("dynamic_debug");
	#else
			PTCHAR config=_T("static_debug");
	#endif
#else
	#if defined(_DYNAMIC_NWL) || defined(_DYNAMIC_AID_)
			PTCHAR config=_T("dynamic_release");
	#else
			PTCHAR config=_T("static_release");
	#endif
#endif
			m_reportedModules.insert(module);
			Log.Add(_MESSAGE_,_T("%s version(%s) compile date(%s) configuration(%s) initialized_______________________________________"),module.c_str(),BUILD_TAG,__DATE__,config);
		}
	}	
};

/// macro for reporting module started
#define REPORT_MODULE(module) CSingleton<CReportModuleName>::instance().ReportModule(module);