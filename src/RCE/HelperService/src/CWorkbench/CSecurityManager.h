#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSecurityManager.h
///
///  IInternetSecurityManager Interface implementation for Workbench application
///
///  @author "Archer Software" Sogin M. @date 11.04.2008
///
////////////////////////////////////////////////////////////////////////
#include <atlbase.h>
#include <atlwin.h>
#include <AidLib/CException/CException.h>
#include <Urlmon.h>

class CSecurityManager 
	:	public IInternetSecurityManager,
		public IServiceProvider,
		public CComObjectRoot
{
public:

	BEGIN_COM_MAP(CSecurityManager)
		COM_INTERFACE_ENTRY(IInternetSecurityManager)
		COM_INTERFACE_ENTRY(IServiceProvider)
	END_COM_MAP()

// IServiceProvider methods----------------------------------

    virtual /* [local] */ HRESULT STDMETHODCALLTYPE QueryService(	/* [in] */ REFGUID guidService,
																	/* [in] */ REFIID riid,
																	/* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

// IInternetSecurityManager methods--------------------------

    virtual HRESULT STDMETHODCALLTYPE SetSecuritySite( 
		/* [unique][in] */ IInternetSecurityMgrSite *pSite);
    
    virtual HRESULT STDMETHODCALLTYPE GetSecuritySite( 
        /* [out] */ IInternetSecurityMgrSite **ppSite);
    
    virtual HRESULT STDMETHODCALLTYPE MapUrlToZone( 
        /* [in] */ LPCWSTR pwszUrl,
        /* [out] */ DWORD *pdwZone,
        /* [in] */ DWORD dwFlags);
    
    virtual HRESULT STDMETHODCALLTYPE GetSecurityId( 
        /* [in] */ LPCWSTR pwszUrl,
        /* [size_is][out] */ BYTE *pbSecurityId,
        /* [out][in] */ DWORD *pcbSecurityId,
        /* [in] */ DWORD_PTR dwReserved);
    
    virtual HRESULT STDMETHODCALLTYPE ProcessUrlAction( 
        /* [in] */ LPCWSTR pwszUrl,
        /* [in] */ DWORD dwAction,
        /* [size_is][out] */ BYTE *pPolicy,
        /* [in] */ DWORD cbPolicy,
        /* [in] */ BYTE *pContext,
        /* [in] */ DWORD cbContext,
        /* [in] */ DWORD dwFlags,
        /* [in] */ DWORD dwReserved);
    
    virtual HRESULT STDMETHODCALLTYPE QueryCustomPolicy( 
        /* [in] */ LPCWSTR pwszUrl,
        /* [in] */ REFGUID guidKey,
        /* [size_is][size_is][out] */ BYTE **ppPolicy,
        /* [out] */ DWORD *pcbPolicy,
        /* [in] */ BYTE *pContext,
        /* [in] */ DWORD cbContext,
        /* [in] */ DWORD dwReserved);
    
    virtual HRESULT STDMETHODCALLTYPE SetZoneMapping( 
        /* [in] */ DWORD dwZone,
        /* [in] */ LPCWSTR lpszPattern,
        /* [in] */ DWORD dwFlags);
    
    virtual HRESULT STDMETHODCALLTYPE GetZoneMappings( 
        /* [in] */ DWORD dwZone,
        /* [out] */ IEnumString **ppenumString,
        /* [in] */ DWORD dwFlags) { return S_OK;};
};