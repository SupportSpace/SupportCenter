// CCustomControlSite.cpp implementation
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CCustomControlSite
//
//-------------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CCustomControlSite : implementation InternetSecurityManager interface 
//-------------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 10/29/2006 05:21:10 PM
// Comments: First Issue
//===========================================================================

#include "stdafx.h"
#include "AidLib/CException/CException.h"
#include <AidLib/Logging/cLog.h> 
#include "UnicodeConvert.h"
#include "Settings.h"
#include "SupportMessenger.h"

extern CSupportMessengerApp theApp;

#undef AFX_DATA
#define AFX_DATA AFX_DATA_IMPORT

#include "custsite.h"
#include <urlmon.h>

static BOOL		m_bWebBaseUrlMaped = FALSE;
static BOOL		m_bLocalBaseUlrMaped = FALSE;
static BOOL		m_bLocalJSMaped  = FALSE;

CString CCustomControlSite::m_sWebBaseUrl;
CString CCustomControlSite::m_sLocalBaseUlr;

#define DO_NOT_MAP_URL_TO_ZONE  URLZONE_USER_MAX  + 1  //Just not map url to zone. Config.ini may be 10001
#define DEFAULT_SECURITY_ZONE   URLZONE_TRUSTED

BEGIN_INTERFACE_MAP(CCustomControlSite, COleControlSite)
	INTERFACE_PART(CCustomControlSite, IID_IInternetSecurityManager, InternetSecurityManager)
	INTERFACE_PART(CCustomControlSite, IID_IServiceProvider, ServiceProvider)
END_INTERFACE_MAP()


//////////////////////////////////////////////////////////////////////////////
// InternetSecurityManager Methods
HRESULT FAR EXPORT CCustomControlSite::XInternetSecurityManager
										::QueryInterface(REFIID riid, void** ppvObj)
{
	HRESULT hr;
TRY_CATCH 
	METHOD_PROLOGUE(CCustomControlSite, InternetSecurityManager)
    hr = (HRESULT)pThis->ExternalQueryInterface(&riid, ppvObj);
CATCH_LOG(_T("CCustomControlSite::XInternetSecurityManager::QueryInterface"))
	return hr;
}

ULONG FAR EXPORT CCustomControlSite::XInternetSecurityManager::AddRef()
{
TRY_CATCH 
CATCH_LOG(_T("CCustomControlSite::XInternetSecurityManager::AddRef"))
	METHOD_PROLOGUE(CCustomControlSite, InternetSecurityManager)
	return pThis->ExternalAddRef();
}

ULONG FAR EXPORT CCustomControlSite::XInternetSecurityManager::Release()
{  
TRY_CATCH 
CATCH_LOG(_T("CCustomControlSite::XInternetSecurityManager::Release"))
 
	METHOD_PROLOGUE(CCustomControlSite, InternetSecurityManager)
	return pThis->ExternalRelease();
}

HRESULT CCustomControlSite::XInternetSecurityManager
							::SetSecuritySite (IInternetSecurityMgrSite *pSite)
{
TRY_CATCH 
CATCH_LOG(_T("CCustomControlSite::XInternetSecurityManager::SetSecuritySite"))
 
	METHOD_PROLOGUE(CCustomControlSite, InternetSecurityManager)
	return INET_E_DEFAULT_ACTION;
}

HRESULT CCustomControlSite::XInternetSecurityManager
							::GetSecuritySite(IInternetSecurityMgrSite **ppSite)
{
TRY_CATCH 
CATCH_LOG(_T("CCustomControlSite::XInternetSecurityManager::GetSecuritySite"))
 
	METHOD_PROLOGUE(CCustomControlSite, InternetSecurityManager)
	return INET_E_DEFAULT_ACTION;
}

HRESULT  FAR EXPORT CCustomControlSite::XInternetSecurityManager
										::GetSecurityId(LPCWSTR pwszUrl,
										BYTE *pbSecurityId,
										DWORD *pcbSecurityId, 
										DWORD dwReserved)
{
TRY_CATCH 
CATCH_LOG(_T("CCustomControlSite::XInternetSecurityManager::GetSecurityId"))
 
	METHOD_PROLOGUE(CCustomControlSite, InternetSecurityManager)
	return INET_E_DEFAULT_ACTION;
}

HRESULT CCustomControlSite::XInternetSecurityManager
							::MapUrlToZone(LPCWSTR pwszUrl,DWORD *pdwZone,DWORD dwFlags)
{
TRY_CATCH 
 
	METHOD_PROLOGUE(CCustomControlSite, InternetSecurityManager)

	if(theApp.m_cSettings.m_dwMapUrlToZone==DO_NOT_MAP_URL_TO_ZONE || theApp.m_dwIEMajorVersion >= 7)
	{
		//Log.Add(_MESSAGE_,_T("MapUrlToZone is not Done. IE7 not needed")); 
		return S_OK;
	}

	CString sUrl(pwszUrl);

	BOOL bNeeMap = FALSE;

	//	this is needed only for IE6 and you have to do it each time MapUrlToZone is called
	//	tried todo it once and widget is not opened
	//  http://www.codeproject.com/KB/shell/IEMenuButton.aspx?print=true
	if( sUrl.Find(m_sWebBaseUrl)!= -1) 
	{
		m_bWebBaseUrlMaped = TRUE;
		bNeeMap = TRUE;
	}

	if(bNeeMap==FALSE)
		return S_OK;

	if(theApp.m_cSettings.m_dwMapUrlToZone!=DO_NOT_MAP_URL_TO_ZONE && m_bWebBaseUrlMaped==FALSE)
		Log.Add(_MESSAGE_,_T("CCustomControlSite::XInternetSecurityManager::MapUrlToZone '%s', MapUrlToZone: '%d', SupportSpace BaseUrl: '%s', LocalBaseUlr: '%s'"), 
			sUrl, theApp.m_cSettings.m_dwMapUrlToZone, m_sWebBaseUrl, m_sLocalBaseUlr);

	switch(theApp.m_cSettings.m_dwMapUrlToZone)
	{
	case SET_DEFAULT_SECURITY_ZONE:
		*pdwZone = DEFAULT_SECURITY_ZONE ; //default zone defined as trusted
		break;
	case DO_NOT_MAP_URL_TO_ZONE:
		break;
	default:
		*pdwZone = theApp.m_cSettings.m_dwMapUrlToZone;
		break;
	}
CATCH_LOG(_T("CCustomControlSite::XInternetSecurityManager::MapUrlToZone"))

	return S_OK;
}
 
STDMETHODIMP CCustomControlSite::XInternetSecurityManager
				::ProcessUrlAction(	/* [in] */ LPCWSTR pwszUrl,
									/* [in] */ DWORD dwAction,
									/* [size_is][out] */ BYTE __RPC_FAR *pPolicy,
									/* [in] */ DWORD cbPolicy,
									/* [in] */ BYTE __RPC_FAR *pContext,
									/* [in] */ DWORD cbContext,
									/* [in] */ DWORD dwFlags,
									/* [in] */ DWORD dwReserved)
{
TRY_CATCH 
	METHOD_PROLOGUE(CCustomControlSite, InternetSecurityManager)

	CString sUrl(pwszUrl);

	//Log.Add(_MESSAGE_,_T("CCustomControlSite::XInternetSecurityManager::ProcessUrlAction from URL '%s', MapUrlToZone: '%d', Actual URL: '%s', LocalBaseUlr: '%s'"), 
	//	sUrl, theApp.m_cSettings.m_dwMapUrlToZone, m_sWebBaseUrl, m_sLocalBaseUlr);

/*
	// 	
	// TODO this code will be releavnt in the future when experts will wtite their own widgets
	// It seems that safest way is to add use MapUrlToZone 
	// and return INET_E_DEFAULT_ACTION in the ProcessUrlAction function
	//
	if( sUrl.Find(m_sWebBaseUrl)== -1 && sUrl.Find(m_sLocalBaseUlr)==-1 && sUrl.Find("javascript:false")==-1 )
	{
		//  script engine currrently need following permiossions to perform following actions:
		//  'URLACTION_SCRIPT_RUN' and 'URLACTION_SCRIPT_OVERRIDE_SAFETY'
		//  from URLs:
		//  file:///C:/Program%20Files/SupportSpace/Support%20Platform/Scripts/Diagnostics/Local/index.html;
		//  http://www.supportspace.com
		//  javascript:false
		if(dwAction==URLACTION_SCRIPT_RUN || dwAction==URLACTION_SCRIPT_OVERRIDE_SAFETY)
		{
			Log.Add(_MESSAGE_,_T("ProcessUrlAction. Action: '%d' is Done. Url is not match supportspace site url %s. Local url: '%s'"), 
				dwAction, sUrl, m_sLocalBaseUlr); 		
		}
		else
		{
			Log.Add(_MESSAGE_,_T("ProcessUrlAction. Action: '%d' not Done. Action will use default zone security Url is not match supportspace site url %s. Local url: '%s'"), 
				dwAction, sUrl, m_sLocalBaseUlr); 
			return INET_E_DEFAULT_ACTION;
		}
	}
*/

	if ( cbPolicy >= sizeof (DWORD))
	{
		Log.Add(_CALL_,_T("ProcessUrlAction for Action: '%d' is Done. Url: '%s'. See urlmon.h for details"), dwAction, sUrl); 
		*(DWORD*) pPolicy = URLPOLICY_ALLOW;
		return S_OK;
	}

CATCH_LOG(_T("CCustomControlSite::XInternetSecurityManager::ProcessUrlAction"))
	return S_FALSE;
}

HRESULT CCustomControlSite::XInternetSecurityManager
							::QueryCustomPolicy(LPCWSTR pwszUrl,
												REFGUID guidKey,
												BYTE **ppPolicy,
												DWORD *pcbPolicy,
												BYTE *pContext,
												DWORD cbContext,
												DWORD dwReserved)
{
TRY_CATCH
CATCH_LOG(_T("CCustomControlSite::XInternetSecurityManager::QueryCustomPolicy"))
	METHOD_PROLOGUE(CCustomControlSite, InternetSecurityManager)
	return INET_E_DEFAULT_ACTION;
}

HRESULT CCustomControlSite::XInternetSecurityManager
							::SetZoneMapping(DWORD dwZone,
											LPCWSTR lpszPattern,
											DWORD dwFlags)
{
TRY_CATCH
CATCH_LOG(_T("CCustomControlSite::XInternetSecurityManager::SetZoneMapping"))
 
	METHOD_PROLOGUE(CCustomControlSite, InternetSecurityManager)
	return INET_E_DEFAULT_ACTION;
}

HRESULT CCustomControlSite::XInternetSecurityManager
							::GetZoneMappings(DWORD dwZone,
											IEnumString **ppenumString, 
											DWORD dwFlags)
{
TRY_CATCH
CATCH_LOG(_T("CCustomControlSite::XInternetSecurityManager::GetZoneMappings"))
 
	METHOD_PROLOGUE(CCustomControlSite, InternetSecurityManager)
	return INET_E_DEFAULT_ACTION;
}



/////////////////////////////////////////////////////////////
// ServiceProvider Methods
ULONG FAR EXPORT CCustomControlSite::XServiceProvider::AddRef()
{
TRY_CATCH
CATCH_LOG(_T("CCustomControlSite::XServiceProvider::AddRef"))
 
	METHOD_PROLOGUE(CCustomControlSite, ServiceProvider)
	return pThis->ExternalAddRef();
}

ULONG FAR EXPORT CCustomControlSite::XServiceProvider::Release()
{     
 
	METHOD_PROLOGUE(CCustomControlSite, ServiceProvider)
	return pThis->ExternalRelease();
}

HRESULT FAR EXPORT CCustomControlSite::XServiceProvider
										::QueryInterface(REFIID riid, 
														void** ppvObj)
{
    HRESULT hr;
TRY_CATCH
	METHOD_PROLOGUE(CCustomControlSite, ServiceProvider)
    hr = (HRESULT)pThis->ExternalQueryInterface(&riid, ppvObj);
CATCH_LOG(_T("CCustomControlSite::XServiceProvider::QueryInterface"))
	return hr;
}
STDMETHODIMP CCustomControlSite::XServiceProvider
								::QueryService(REFGUID guidService,  
												REFIID riid,
												void** ppvObject)
{
TRY_CATCH
 
	if (guidService == SID_SInternetSecurityManager && 
					riid == IID_IInternetSecurityManager)
	{
		METHOD_PROLOGUE(CCustomControlSite, ServiceProvider)
		HRESULT hr = (HRESULT)pThis->ExternalQueryInterface(&riid, ppvObject);
		return hr;
	} 
	else 
	{
		*ppvObject = NULL;

	}

CATCH_LOG(_T("CCustomControlSite::XServiceProvider::QueryService"))

	return E_NOINTERFACE;
}

/////////////////////////////////////////////////////////////
// static private Methods


//	this call usually will return http://www.supportspace.com - 
//  the base url of the workbench
CString	CCustomControlSite::GetWebBaseUrl()
{
	CString sWebUlr;
TRY_CATCH
	CString sUrl = theApp.m_cSettings.m_sBaseUrlPickUp;
	int	ind = 0;
	ind = sUrl.Find("//");		//	first appearence of "//"
	//ind = sUrl.Find(".");			//	first appearence of "." to get supportspace.com 
	ind = sUrl.Find('/', ind + 2);  //	the end of the base url
	sWebUlr =  sUrl.Left(ind);
CATCH_LOG(_T("CCustomControlSite::GetLocalBaseUrl"))	
	return sWebUlr;
}

//	this call usually will return file:///C:/Program%20Files/SupportSpace/Support%20Platform -
//  the base url of the local scripts like diagnostic
CString	CCustomControlSite::GetLocalBaseUrl()
{
	CString sLocalUlr;
TRY_CATCH
	sLocalUlr = theApp.m_sApplicationPath;
	sLocalUlr.Replace(" ","%20"); // Replace all occurrences of " " with "%20"
	sLocalUlr.Replace('\\','/'); // Replace all occurrences of " " with "%20"
	sLocalUlr = "file:///" + sLocalUlr;
CATCH_LOG(_T("CCustomControlSite::GetLocalBaseUrl"))
	return sLocalUlr;
}