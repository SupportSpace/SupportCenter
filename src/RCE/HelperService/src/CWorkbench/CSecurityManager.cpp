/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSecurityManager.cpp
///
///  IInternetSecurityManager Interface implementation for Workbench application
///
///  @author "Archer Software" Sogin M. @date 11.04.2008
///
////////////////////////////////////////////////////////////////////////
#include "CWorkbench.h"

HRESULT STDMETHODCALLTYPE CSecurityManager::QueryService(	/* [in] */ REFGUID guidService,
															/* [in] */ REFIID riid,
															/* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
TRY_CATCH

	if (guidService == SID_SInternetSecurityManager && 
		riid == IID_IInternetSecurityManager)
	{
		HRESULT hr = QueryInterface(riid, ppvObject);
		return hr;
	} 
	else 
	{
		*ppvObject = NULL;
	}
CATCH_LOG()
	return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE CSecurityManager::SetSecuritySite( /* [unique][in] */ IInternetSecurityMgrSite *pSite) 
{ 
TRY_CATCH
CATCH_LOG()
	return INET_E_DEFAULT_ACTION;
}

HRESULT STDMETHODCALLTYPE CSecurityManager::GetSecuritySite( /* [out] */ IInternetSecurityMgrSite **ppSite) 
{ 
TRY_CATCH
CATCH_LOG()
	return INET_E_DEFAULT_ACTION;
}

HRESULT STDMETHODCALLTYPE CSecurityManager::MapUrlToZone(	/* [in] */ LPCWSTR pwszUrl,
												/* [out] */ DWORD *pdwZone,
												/* [in] */ DWORD dwFlags)
{
TRY_CATCH

	*pdwZone = URLZONE_TRUSTED ; //default zone defined as trusted
	Log.Add(_CALL_,_T("MapUrlToZone is Done")); 
	return S_OK;
	/*if(theApp.m_cSettings.m_dwMapUrlToZone==DO_NOT_MAP_URL_TO_ZONE || theApp.m_dwIEMajorVersion >= 7)
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
		break;*/

CATCH_LOG()
	return INET_E_DEFAULT_ACTION;
}
    
HRESULT STDMETHODCALLTYPE CSecurityManager::GetSecurityId(	/* [in] */ LPCWSTR pwszUrl,
															/* [size_is][out] */ BYTE *pbSecurityId,
															/* [out][in] */ DWORD *pcbSecurityId,
															/* [in] */ DWORD_PTR dwReserved )

{
TRY_CATCH
CATCH_LOG()
	return INET_E_DEFAULT_ACTION;
}

HRESULT STDMETHODCALLTYPE CSecurityManager::ProcessUrlAction(	/* [in] */ LPCWSTR pwszUrl,
																/* [in] */ DWORD dwAction,
																/* [size_is][out] */ BYTE *pPolicy,
																/* [in] */ DWORD cbPolicy,
																/* [in] */ BYTE *pContext,
																/* [in] */ DWORD cbContext,
																/* [in] */ DWORD dwFlags,
																/* [in] */ DWORD dwReserved)
{
TRY_CATCH

	if ( cbPolicy >= sizeof (DWORD))
	{
		Log.Add(_CALL_,_T("ProcessUrlAction for Action: '%d' is Done. See urlmon.h for details"), dwAction); 
		*(DWORD*) pPolicy = URLPOLICY_ALLOW;
		return S_OK;
	}

CATCH_LOG()
	return INET_E_DEFAULT_ACTION;
}

HRESULT STDMETHODCALLTYPE CSecurityManager::QueryCustomPolicy(	/* [in] */ LPCWSTR pwszUrl,
																/* [in] */ REFGUID guidKey,
																/* [size_is][size_is][out] */ BYTE **ppPolicy,
																/* [out] */ DWORD *pcbPolicy,
																/* [in] */ BYTE *pContext,
																/* [in] */ DWORD cbContext,
																/* [in] */ DWORD dwReserved )
{
TRY_CATCH
CATCH_LOG()
	return INET_E_DEFAULT_ACTION;
}

HRESULT STDMETHODCALLTYPE CSecurityManager::SetZoneMapping(	/* [in] */ DWORD dwZone,
															/* [in] */ LPCWSTR lpszPattern,
															/* [in] */ DWORD dwFlags)
{
TRY_CATCH
CATCH_LOG()
	return INET_E_DEFAULT_ACTION;
}

HRESULT STDMETHODCALLTYPE GetZoneMappings(	/* [in] */ DWORD dwZone,
											/* [out] */ IEnumString **ppenumString,
											/* [in] */ DWORD dwFlags )
{
TRY_CATCH
CATCH_LOG()
	return INET_E_DEFAULT_ACTION;
}
 