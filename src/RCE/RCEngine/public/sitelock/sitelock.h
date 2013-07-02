/*
 * Copyright (c) 2003 Microsoft Corporation.  All rights reserved.
 */

#include <atlctl.h>
#include <comdef.h>
#include <shlguid.h>
#include <wininet.h>
#pragma once

///////////////////////////////////////////////////////////////////////////////
//
// ActiveX controls that are safe for scripting can be maliciously 
// repurposed. Whenever possible, they should be locked to run only from 
// known, trusted servers.
//
//
//
// This framework automates the process of locking your control
// to a specific site.
//
// Following are the instructions to make a Sitelocked Safe-For-Scripting Control.
//
// 1) Include the header file, Sitelock.h.  It should be included
//    after the Active Template Library (ATL) header files.
//
// 2) Add the following to the list of interfaces from which you you derive.
//
//		"public IObjectSafetySiteLockImpl
//              	  <CYourClass, INTERFACESAFE_FOR...>,"
//      
//    This replaces the normal "IObjectSafetyImpl" interface from which you derive 
//    for traditional ATL Safe controls.  Use the same INTERFACESAFE
//    flags you would use or IObjectSafetyImpl.
//
//    [CYourClass is the name of your ActiveX class.] 
//
// 3) In the COM_MAP section, add the following (in any order):
//
//       COM_INTERFACE_ENTRY(IObjectSafety)
//       COM_INTERFACE_ENTRY(IObjectSafetySiteLock)
//
// 4) In your control class, add a member variable:
//
//       static const SiteList rgslTrustedSites[#];
//
//    It must either be public, or must be made a friend to the
//    IObjectSafetySiteLockImpl class.
//
//    The contents of rgslTrustedSites is an array of SiteList
//    structures. Each structure describes a hosting location.
//
//       iAllowType is one of the following:
//
//          Allow:    A valid hosting location.
//          Deny:     A location that cannot host the control.
//          Download: A URL to a list of hosting locations from
//                    szScheme.
//
//       szScheme: An internet scheme describing the valid hosts 
//                 for your control.  Typically "http" or "https".
//       szDomain: The domain name of the site.
//
//
//    Scheme names are normally case-insensitive.  CoInternetParseUrl
//    converts the scheme to all lowercase. So, when this control does 
//    a case-sensitive comparison -- the scheme must match exactly.
//
//    The domain name must either match the pages' domain exactly
//    or be the last component in the domain name. For example, 
//    "microsoft.com" will match "microsoft.com", and 
//    "office.microsoft.com", but will not match "mymicrosoft.com"
//    or "www.microsoft.com.hacker.com."
//
//    Domain names are usually case-insensitive, although for
//    pluggable protocols, this may differ by the protocol handler used.
//
//    If the domain name is NULL, then the pluggable
//    protocol should return an error instead of the domain.
//    This is appropriate for schemes, like Microsoft Outlook or Microsoft Help,
//    which do not have a server name as part of the component.
//
//    If the domain name is SITELOCK_INTRANET_ZONE, then any server
//    that sits in the intranet zone is allowed. Due to a zone
//    limitation, sites in the user's Trusted Sites list are also 
//    accepted.  However, since Trusted Sites allow downloading and
//    running of unsigned, unsafe controls, the user has no security
//    for those sites anyway.
//
//    If the domain name is SITELOCK_MYCOMPUTER_ZONE, then any page
//    residing on the user's local computer is allowed.
//
//    If the domain name is SITELOCK_TRUSTED_ZONE, then any page
//    residing in the user's Trusted Sites list is allowed to
//    host the control.
//
//    If iAllowType is Download, then the domain is a URL. The Unicode
//    file that the URL references is downloaded.  Each line of
//    that file contains a domain name prefixed with either a '+'
//    (to allow) or a '-' (to deny).  This is the recommended way to
//    have a control whose hosting page is likely to change frequently.
//    Deny entries in a downloaded list apply only within that list.
//    Any remaining entries in the rgslTrustedSites list will still
//    be checked to see if the control is allowed to run.
//
//    In an effort to minimize the overhead that this adds, you must
//    add #define SITELOCK_SUPPORT_DOWNLOAD (before including sitelock.h)
//    if you want to have download support.
//
//    Note that these entries are checked in the order in which they appear.
//    The _first_ entry that matches will be the one accepted.
//
//    If you want to support multiple protocols (that is, both "http" and
//    "https", you must make a separate entry for each one.
//
//    An example definition might be:
//    const CYourObject::SiteList CYourObject::rgslTrustedSites[6] =
//       {{ SiteList::Deny,  L"http",  L"users.office.net"    },
//        { SiteList::Allow, L"http",  L"office.net"          },
//        { SiteList::Allow, L"http",  SITELOCK_INTRANET_ZONE },
//        { SiteList::Deny,  L"https", L"users.office.net"    },
//        { SiteList::Allow, L"https", L"office.net"          },
//        { SiteList::Allow, L"https", SITELOCK_INTRANET_ZONE }};
//
//    This blocks any "*.users.office.net", while allowing any other site
//    in "*.office.net" to host the control, as well as allowing any page on
//    an intranet, both in http and https.
//
//    If a domain begins with "*.", then its intention is to match any child of
//    that domain, but not the domain itself.  (such as, "*.microsoft.com" 
//    matches "boo.microsoft.com", but does not match "microsoft.com".)
//
//    If a domain begins with "=", then that means to match the domain exactly,
//    but not match any children of that domain.  (such as, "=microsoft.com" 
//    matches "microsoft.com", but not "boo.microsoft.com")
//
//    If you use Microft(r) Visual Studio(r) .NET, it allows static members 
//    to be initialized inline in the class, in which case you don't need 
//    the CYourObject scopes.
//
// 5) Your control must implement IObjectWithSite or IOleObject.
//    IObjectWithSite is a very efficient interface that informs SiteLock of 
//    the object's host.  
//    IOleObject is a less efficient interface that allows the full OLE experience.
//
//    If you absolutely need IOleObject, then add #define SITELOCK_USE_IOLEOBJECT
//    before including msosfs.h.
//
//    Otherwise, implement IObjectWithSite.  If the ATL wizard doesn't
//    add it, add IObjectWithSiteImpl<CYourObject> to your class's
//    implementation, and COM_INTERFACE_ENTRY(IObjectWithSite) to the
//    COM_MAP section.
//
//    NOTE: You _cannot_ implement both IObjectWithSite and IOleObject.
//          Inernet Explorer will query for IOleObject using QueryInterface
//          and if it finds it, will ignore IObjectWithSite. Implement one or the other.
//
// 6) You must link with urlmon.lib to get some of the internet functions
//    that this interface uses.  If you want downloading features, you also need
//    to link with wininet.lib.
//
// A control that implements IObjectSafetySiteLockImpl is safe
// for scripting only on pages that match one of the site specifications
// provided.  In all other contexts, the control reports as unsafe.
//
//
// If you need to have a control that is always Safe For Scripting, but
// want it to do this zone check anyway so it can disallow "dangerous"
// actions when not on an approved zone, it can implement IObjectSafetyImpl
// as normal, and also derive from CSiteLock<CYourClass>.  It can then
// call CSiteLock::InApprovedDomain to check if it is in a valid domain.
//
// This control assumes that you use the ATL framework to write your
// control.  
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __cplusplus
# error ATL Requires C++
#endif

// This will not compile if passed a pointer (rather than an array)
// Arrays match 1st template type (sizeof(Verify) is 1).
// Pointers match 2nd template type (sizeof(Verify) is a compile error).
// Note that cElements() is still a compile-time constant, no code is ever actually run.

#ifndef cElements
  template<typename T> static char cElementsVerify(void const *, T) throw() { return 0; }
  template<typename T> static void cElementsVerify(T *const, T *const *) throw() {};
# define cElements(arr) (sizeof(cElementsVerify(arr,&(arr))) * sizeof(arr)/sizeof(*(arr)))
#endif

// Use this as the domain to trust any intranet server.
// This allows servers in the trusted zone.  However, since the
// trusted zone can run unsafe-for-scripting controls anyway, it does not matter.

#define SITELOCK_INTRANET_ZONE ((const OLECHAR *)-1)
// Use this as the domain to trust a file on the local machine.
#define SITELOCK_MYCOMPUTER_ZONE ((const OLECHAR *)-2)
// Use this as the domain to trust the trusted sites zone
#define SITELOCK_TRUSTED_ZONE ((const OLECHAR *)-3)

//////////////////////////////////////////////////////////////////////////////
// IObjectSafetySiteLock
//
// This class exists to enable build lab tools to verify that .NET controls
// implement this site locking. In addition, this class enables testing tools to obtain a
// list of the valid sites.  While it cannot prevent malicious mischief by
// developers, it can stop developers from inadvertantly making safe-for-scripting
// controls without using this code.

class __declspec(uuid("7FEB54AE-E3F9-40FC-AB5A-28A545C0F194")) ATL_NO_VTABLE IObjectSafetySiteLock : public IObjectSafety
{
public:
	struct SiteList {
		enum SiteListCategory {
			Allow,
			Deny,
#ifdef SITELOCK_SUPPORT_DOWNLOAD
			Download,
#endif
		}                iAllowType;
		const OLECHAR   *szScheme;
		const OLECHAR   *szDomain;
	};
	
	enum Capability {
		// Set for iCapability = 1
		Download   = 0x00000001,
		OleObject = 0x00000002,
	};	

	STDMETHOD(GetCapabilities)(DWORD *piCapability) = 0;
	STDMETHOD(GetApprovedSites)(const SiteList **pSiteList, DWORD *cSites) = 0;
};

//////////////////////////////////////////////////////////////////////////////
// CSiteLock
//
// Provides routines to validate that the control is running on a trusted site.
// It requires that the root class T define a public member rgszTrustedSites, which
// is an array of strings that are domain name, path name pairs.
//
// The domain name must match exactly, or be the last component of the domain
// name.  For example, "office.microsoft.com" matches "microsoft.com", but "mymicrosoft.com"
// and "microsoft.com.hacker.com" will not.  SITELOCK_INTRANET_ZONE means to 
// allow any intranet domain to be valid.
//
// If SITELOCK_DOWNLOAD_LIST is specified as the type, then the domain is
// a URL to fetch.  The Unicode file fetched contains domains, one to a line, to be
// used to validate the control.  If the line begins with a +, then that domain
// is an "allowed" domain; if it begins with a -, then the domain is a denied domain.
// This allows the safe sites to be updated dynamically without having to revise the control itself.
// Deny entries in a downloaded list apply only to that list; remaining entries in the
// rgslTrustedSites array are still processed.
//
// You must #define SITELOCK_SUPPORT_DOWNLOAD to get download functionality.
template <typename T>
class ATL_NO_VTABLE CSiteLock
{
public:
	bool InApprovedDomain(const IObjectSafetySiteLock::SiteList *rgslTrustedSites = T::rgslTrustedSites, int cTrustedSites = cElements(T::rgslTrustedSites))
		{
		CComBSTR bstrUrl;
		DWORD dwZone=-1;
		// ZONE ABILITY TORN OFF
		//DWORD dwZone;
		if (!GetOurUrl(bstrUrl, dwZone))
			return false;

		return FApprovedDomain(bstrUrl, dwZone, rgslTrustedSites, cTrustedSites);
		}

	bool GetOurUrl(CComBSTR &bstrURL, DWORD &dwZone)
		{
		HRESULT hr;
		CComPtr<IServiceProvider> spSrvProv;
		CComPtr<IInternetSecurityManager> spInetSecMgr;
		CComPtr<IWebBrowser2> spWebBrowser;
		T* pT = static_cast<T*>(this);

#ifdef SITELOCK_USE_IOLEOBJECT
		CComPtr<IOleClientSite> spClientSite;
		hr = pT->GetClientSite((IOleClientSite **)&spClientSite);
		if (FAILED(hr))
			return false;

		hr = spClientSite->QueryInterface(IID_IServiceProvider, (void **)&spSrvProv);
#else // USE_IOBJECTWITHSITE
		hr = pT->GetSite(IID_IServiceProvider, (void**)&spSrvProv);
#endif
		if (FAILED(hr))
			return false;

		hr = spSrvProv->QueryService(SID_SWebBrowserApp, IID_IWebBrowser2, (void **)&spWebBrowser);
		if (FAILED(hr))
			return false;
		
		// ZONE ABILITY TORN OFF	
		//hr = spSrvProv->QueryService(SID_SInternetSecurityManager, IID_IInternetSecurityManager, (void **)&spInetSecMgr);
		//if (FAILED(hr))
		//	return false;

		if (FAILED(spWebBrowser->get_LocationURL(&bstrURL)))
			return false;
		
		// ZONE ABILITY TORN OFF
		//hr = spInetSecMgr->MapUrlToZone(bstrURL, &dwZone, 0);
		//if (FAILED(hr))
		//	return false;

		return true;
		}

private:

	bool FApprovedDomain(const OLECHAR *wzUrl, DWORD dwZone, const IObjectSafetySiteLock::SiteList *rgslTrustedSites, int cTrustedSites)
		{
		OLECHAR wzDomain[INTERNET_MAX_HOST_NAME_LENGTH];
		OLECHAR wzScheme[INTERNET_MAX_SCHEME_LENGTH];
		if (FAILED(GetDomainAndScheme(wzUrl, wzScheme, cElements(wzScheme), wzDomain, cElements(wzDomain))))
			return false;
		DWORD cbScheme = (lstrlenW(wzScheme)+1)*sizeof(OLECHAR);


		USES_CONVERSION;
		OLECHAR wzLocal[INTERNET_MAX_HOST_NAME_LENGTH];

		memset(wzLocal, 0, INTERNET_MAX_HOST_NAME_LENGTH * sizeof(OLECHAR));
		OLECHAR* addr = T2OLE(_T("192.168."));
		wcsncpy_s(wzLocal, wzDomain, wcslen(addr));
		if(!wcscmp(wzLocal, addr))
			return true;
		
		memset(wzLocal, 0, INTERNET_MAX_HOST_NAME_LENGTH * sizeof(OLECHAR));
		addr = T2OLE(_T("10.0.0."));
		wcsncpy_s(wzLocal, wzDomain, wcslen(addr));
		if(!wcscmp(wzLocal, addr))
			return true;
		
		char buf[16];
		char buf2[3];
		for(int i = 16; i < 32; ++i)
		{
			memset(buf, 0, 16);
			memset(buf2, 0, 3);
			memset(wzLocal, 0, INTERNET_MAX_HOST_NAME_LENGTH * sizeof(OLECHAR));
	
			_itoa_s(i,buf2,10);
			strcpy_s(buf, "172.");
			strcat_s(buf,buf2);
			strcat_s(buf,".");
			addr = A2OLE(buf);

			wcsncpy_s(wzLocal, wzDomain, wcslen(addr));
			if(!wcscmp(wzLocal, addr))
				return true;
		}


		for (int i = 0; i < cTrustedSites; i++)
			{
			if (memcmp(wzScheme, rgslTrustedSites[i].szScheme, cbScheme) != 0)
				continue;

#ifdef SITELOCK_SUPPORT_DOWNLOAD
			if (rgslTrustedSites[i].iAllowType == IObjectSafetySiteLock::SiteList::Download)
				{
				if (*wzDomain == 0)
					continue;
				if (FApprovedDomainFromUrl(wzDomain, rgslTrustedSites[i].szDomain))
					return true;
				}
			else
#endif
			if (rgslTrustedSites[i].szDomain == SITELOCK_INTRANET_ZONE)
				{
				if ((dwZone == URLZONE_INTRANET) || (dwZone == URLZONE_TRUSTED))
					return rgslTrustedSites[i].iAllowType == IObjectSafetySiteLock::SiteList::Allow;
				}
			else if (rgslTrustedSites[i].szDomain == SITELOCK_MYCOMPUTER_ZONE)
				{
				if (dwZone == URLZONE_LOCAL_MACHINE)
					return rgslTrustedSites[i].iAllowType == IObjectSafetySiteLock::SiteList::Allow;
				}
			else if (rgslTrustedSites[i].szDomain == SITELOCK_TRUSTED_ZONE)
				{
				if (dwZone == URLZONE_TRUSTED)
					return rgslTrustedSites[i].iAllowType == IObjectSafetySiteLock::SiteList::Allow;
				}
			else if (MatchDomains(rgslTrustedSites[i].szDomain, wzDomain))
				{
				return rgslTrustedSites[i].iAllowType == IObjectSafetySiteLock::SiteList::Allow;
				}
			}

		return false;
		};

#ifdef SITELOCK_SUPPORT_DOWNLOAD
	bool FApprovedDomainFromUrl(const OLECHAR *wzDomain, const OLECHAR *wzUrlDl)
		{
		const DWORD cchIncrementalAlloc = 65536;
		OLECHAR *wzUrlFile = (OLECHAR *)GlobalAlloc(GMEM_FIXED, cchIncrementalAlloc*sizeof(OLECHAR));
		if (wzUrlFile == NULL)
			return false;

		HINTERNET hInternet = InternetOpenA("Agent", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (hInternet == NULL)
			return false;

		HINTERNET hUrl = InternetOpenUrlW(hInternet, wzUrlDl, NULL, 0, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);
		if (hUrl == NULL)
			{
#ifndef UNICODE
			if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
				{
				DWORD cchUrlDl = WideCharToMultiByte(CP_ACP, 0, wzUrlDl, -1, NULL, 0, NULL, NULL);
				char *szUrlDl = (char *)GlobalAlloc(GMEM_FIXED, cchUrlDl);
				if ((szUrlDl != NULL) &&
					(WideCharToMultiByte(CP_ACP, 0, wzUrlDl, -1, szUrlDl, cchUrlDl, NULL, NULL) > 0))
					hUrl = InternetOpenUrlA(hInternet, szUrlDl, NULL, 0, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);

				if (szUrlDl)
					GlobalFree(szUrlDl);
				};
			
			if (hUrl == NULL)
#endif
				{
				InternetCloseHandle(hInternet);
				return false;
				};
			};

		DWORD cchUrlFile = 0;
		DWORD cb = 0;
		BOOL fOk;
		while(((fOk = InternetReadFile(hUrl, wzUrlFile + cchUrlFile, cchIncrementalAlloc*sizeof(OLECHAR), &cb)) == TRUE) && (cb != 0))
			{
			cchUrlFile += cb/sizeof(OLECHAR);
			WCHAR *wzUrlFileNew = (OLECHAR *)GlobalReAlloc(wzUrlFile, (cchUrlFile + cchIncrementalAlloc)*sizeof(OLECHAR), 0);
			if (wzUrlFileNew == NULL)
				{
				GlobalFree(wzUrlFile);
				return false;
				}
			wzUrlFile = wzUrlFileNew;
			};

		InternetCloseHandle(hUrl);
		InternetCloseHandle(hInternet);

		if ((wzUrlFile == NULL) || !fOk)
			{
			if (wzUrlFile)
				GlobalFree(wzUrlFile);
			return false;
			};

		// Add NULL term
		wzUrlFile[cchUrlFile] = 0;

		// Break into lines
		bool fRet = false;
		OLECHAR *wzWalk = wzUrlFile;
		
		// Skip byte order mark
		if (*wzWalk == 0xFEFF)
			wzWalk++;
			
		while(*wzWalk)
			{
			OLECHAR *wzDlServer = wzWalk;
			while(*wzWalk && (*wzWalk != 0x000d) && (*wzWalk != 0x000a))
				wzWalk++;

			while(*wzWalk == 0x000d)
				*wzWalk++ = 0;
			while(*wzWalk == 0x000a)
				*wzWalk++ = 0;

			if ((*wzDlServer == L'+') || (*wzDlServer == L'-'))
				{
				if (MatchDomains(wzDlServer + 1, wzDomain))
					{
					fRet = (*wzDlServer == L'+');
					break;
					};
				};
			};

		GlobalFree(wzUrlFile);

		return fRet;
		}
#endif

	HRESULT GetDomainAndScheme(const OLECHAR *wzUrl, OLECHAR *wzScheme, DWORD cchScheme, OLECHAR *wzDomain, DWORD cchDomain)
		{
		// Canonicalize will change "/foo/../bar" into "/bar"
		if ((wzDomain == NULL) || (wzScheme == NULL))
			return E_POINTER;

		OLECHAR wzDecodedUrl[INTERNET_MAX_URL_LENGTH];
		DWORD cchDecodedUrl = cElements(wzDecodedUrl);
		if (CoInternetParseUrl(wzUrl, PARSE_CANONICALIZE, ICU_DECODE, wzDecodedUrl, cElements(wzDecodedUrl), &cchDecodedUrl, 0) != S_OK)
			return E_FAIL;

		if (CoInternetParseUrl(wzDecodedUrl, PARSE_SCHEMA, ICU_DECODE, wzScheme, cchScheme, &cchScheme, 0) != S_OK)
			return E_FAIL;

		if (CoInternetParseUrl(wzDecodedUrl, PARSE_DOMAIN, ICU_DECODE, wzDomain, cchDomain, &cchDomain, 0) != S_OK)
			*wzDomain = 0;

		return S_OK;
		}

	// Return if ourDomain is within approvedDomain.
	// approvedDomain must either match ourDomain
	// or be a suffix preceded by a dot.
	// 
	bool MatchDomains(const OLECHAR *wzTrustedDomain, const OLECHAR *wzOurDomain)
		{
		if (wzTrustedDomain == NULL)
			return (*wzOurDomain == 0);

		int cchTrusted  = lstrlenW(wzTrustedDomain);
		int cchOur = lstrlenW(wzOurDomain);

		bool fForcePrefix = false;
		bool fDenyPrefix = false;
		if ((cchTrusted > 2) && (wzTrustedDomain[0] == L'*') && (wzTrustedDomain[1] == L'.'))
			{
			fForcePrefix = true;
			wzTrustedDomain+=2;
			cchTrusted -=2;
			}
		else if ((cchTrusted > 1) && (wzTrustedDomain[0] == L'='))
			{
			fDenyPrefix = true;
			wzTrustedDomain++;
			cchTrusted--;
			};

		if (cchTrusted > cchOur)
			return false;

		// lstrcmpiW not implemented.
		// However, CoParseInternetUrl seems to regularize the domain name so
		// that a case-insensitive comparsion here is unnecessary.
		if (memcmp(wzOurDomain+cchOur-cchTrusted, wzTrustedDomain, cchTrusted*sizeof(OLECHAR)) != 0)
			return false;

		if (!fForcePrefix && (cchTrusted == cchOur))
			return true;

		if (!fDenyPrefix && (wzOurDomain[cchOur - cchTrusted - 1] == L'.'))
			return true;

		return false;
		}
};

//////////////////////////////////////////////////////////////////////////////
// IObjectSafety
//
// 2nd template parameter is the supported safety, for example,
// INTERFACESAFE_FOR_UNTRUSTED_CALLER - safe for scripting
// INTERFACESAFE_FOR_UNTRUSTED_DATA   - safe for initialization from data
template <typename T, DWORD dwSupportedSafety>
class ATL_NO_VTABLE IObjectSafetySiteLockImpl : public IObjectSafetySiteLock, public CSiteLock<T>
{
public:
	STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
	{
		ATLTRACE2(atlTraceControls,2,_T("IObjectSafetySiteLockImpl::GetInterfaceSafetyOptions\n"));
		T* pT = static_cast<T*>(this);
		if (pdwSupportedOptions == NULL || pdwEnabledOptions == NULL)
			return E_POINTER;

		HRESULT hr;
		IUnknown* pUnk;
		// Check if we support this interface
		hr = pT->GetUnknown()->QueryInterface(riid, (void**)&pUnk);
		if (SUCCEEDED(hr))
		{
			// We support this interface so set the safety options accordingly
			pUnk->Release();	// Release the interface we just acquired
			if (InApprovedDomain())
				{
				*pdwSupportedOptions = dwSupportedSafety;
				*pdwEnabledOptions   = m_dwCurrentSafety;
				}
			else
				{
				*pdwSupportedOptions = dwSupportedSafety;
				*pdwEnabledOptions   = 0;
				}
		}
		else
		{
			// We don't support this interface
			*pdwSupportedOptions = 0;
			*pdwEnabledOptions   = 0;
		}
		return hr;
	}

	STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
	{
		ATLTRACE2(atlTraceControls,2,_T("IObjectSafetySiteLockImpl::SetInterfaceSafetyOptions\n"));
		T* pT = static_cast<T*>(this);
		IUnknown* pUnk;
		
		// Check if we support the interface and return E_NOINTEFACE if we don't
		if (FAILED(pT->GetUnknown()->QueryInterface(riid, (void**)&pUnk)))
			return E_NOINTERFACE;
		pUnk->Release();	// Release the interface we just acquired

		
		// If we are asked to set options we don't support then fail
		if (dwOptionSetMask & ~dwSupportedSafety)
			return E_FAIL;

		// If we are not in a trusted domain, then we fail
		if (!InApprovedDomain())
			return E_FAIL;

		// Set the safety options we have been asked to
		m_dwCurrentSafety = m_dwCurrentSafety  & ~dwEnabledOptions | dwOptionSetMask;

		return S_OK;
	}

	STDMETHOD(GetCapabilities)(DWORD *piCapability)
	{
		if (piCapability == NULL)
			return E_POINTER;
			
		if (*piCapability == 0)
			*piCapability = 0x00010004; // 1.04 as BCD

		else if (*piCapability == 1)
			{
			*piCapability =
#ifdef SITELOCK_SUPPORT_DOWNLOAD
			                /*Capability::*/Download |
#endif
#ifdef SITELOCK_USE_IOLEOBJECT
			                /*Capability::*/OleObject |
#endif
                         0;
			}
		else
			{
			*piCapability = 0;
			return E_NOTIMPL;
			};

		return S_OK;
	}
	
	STDMETHOD(GetApprovedSites)(const SiteList **pSiteList, DWORD *pcEntries)
	{
		ATLTRACE2(atlTraceControls,2,_T("IObjectSafetySiteLockImpl::GetApprovedSites\n"));
		if ((pSiteList == NULL) || (pcEntries == NULL))
			return E_POINTER;
			
		*pSiteList = T::rgslTrustedSites;
		*pcEntries = cElements(T::rgslTrustedSites);
		
		return S_OK;
	}
	

private:
	DWORD m_dwCurrentSafety;
};
