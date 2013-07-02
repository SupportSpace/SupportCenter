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
#ifndef __CUSTOMSITEH__
#define __CUSTOMSITEH__

#include <mshtmhst.h>
#include <occimpl.h>

class CCustomControlSite: public CBrowserControlSite
{
public:
	CCustomControlSite(COleControlContainer *pCnt, CDHtmlDialog* pHandler):CBrowserControlSite(pCnt, pHandler)
	{
		 m_sWebBaseUrl = GetWebBaseUrl(); 
		 m_sLocalBaseUlr = GetLocalBaseUrl(); 
	}

	CString	GetWebBaseUrl();
	CString	GetLocalBaseUrl();

	static CString m_sWebBaseUrl;
	static CString m_sLocalBaseUlr;

protected:

	DECLARE_INTERFACE_MAP();


///////////////////////////////////////
//// Implement IInternetSecurityManager
BEGIN_INTERFACE_PART(InternetSecurityManager, IInternetSecurityManager)
		STDMETHOD(SetSecuritySite)(IInternetSecurityMgrSite*);
		STDMETHOD(GetSecuritySite)(IInternetSecurityMgrSite**);
		STDMETHOD(MapUrlToZone)(LPCWSTR,DWORD*,DWORD);
		STDMETHOD(GetSecurityId)(LPCWSTR,BYTE*,DWORD*,DWORD);
		STDMETHOD(ProcessUrlAction)(
            /* [in] */ LPCWSTR pwszUrl,
            /* [in] */ DWORD dwAction,
            /* [size_is][out] */ BYTE __RPC_FAR *pPolicy,
            /* [in] */ DWORD cbPolicy,
            /* [in] */ BYTE __RPC_FAR *pContext,
            /* [in] */ DWORD cbContext,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwReserved = 0);
		STDMETHOD(QueryCustomPolicy)(LPCWSTR,REFGUID,BYTE**,DWORD*,BYTE*,DWORD,DWORD);
		STDMETHOD(SetZoneMapping)(DWORD,LPCWSTR,DWORD);
		STDMETHOD(GetZoneMappings)(DWORD,IEnumString**,DWORD);
END_INTERFACE_PART(InternetSecurityManager)


///////////////////////////////////////
//// Implement IServiceProvider
BEGIN_INTERFACE_PART(ServiceProvider, IServiceProvider)
		STDMETHOD(QueryService)(REFGUID,REFIID,void**);
END_INTERFACE_PART(ServiceProvider)


};

#endif