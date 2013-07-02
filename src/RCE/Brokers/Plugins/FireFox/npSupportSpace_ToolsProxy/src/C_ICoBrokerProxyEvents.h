/////////////////////////////////////////////////////////////////////////
///
///  SupportSpace Ltd.
///
///  C_ICoBrokerProxyEvents.h.h
///
///  C_ICoBrokerProxyEvents object declaration. The object is events receiever of CoBrokerProxy events
///
///  @author Anatoly Gutnick @date 16.04.2008
///
////////////////////////////////////////////////////////////////////////
//   C_ICoBrokerEvents.h : Declaration of the C_ICoBrokerEvents
#pragma once

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif
		 
#import "SupportSpace_tools.dll" raw_interfaces_only, raw_native_types, no_namespace, named_guids, auto_search, embedded_idl

#include "C_ICoBrokerProxyEvents.h"

//   helper interface IEventListener to be implemented by event listener
class IEventListener
{
public:
	IEventListener(){};
	~IEventListener(){};

  virtual long DoNotifyLogMessage(const char* message, ULONG severity) = 0;
  virtual long DoRequestSent(const char* dstUserId, ULONG dstSvcId, const char* srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, const char* params) = 0;
};

class C_ICoBrokerProxyEvents:
	public _ICoBrokerProxyEvents
{
public:
	
	C_ICoBrokerProxyEvents(void);
	~C_ICoBrokerProxyEvents(void);

	IEventListener* m_owner;

	STDMETHOD(NotifyLogMessage)(BSTR message, long severity);
	STDMETHOD(RequestSent)(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params);

	//+++ Start Injected Code For Attribute 'coclass'
    virtual HRESULT STDMETHODCALLTYPE _ICoBrokerProxyEvents::Invoke(
                /* [in] */ DISPID dispIdMember,
                /* [in] */ REFIID riid,
                /* [in] */ LCID lcid,
                /* [in] */ WORD wFlags,
                /* [out][in] */ DISPPARAMS *pDispParams,
                /* [out] */ VARIANT *pVarResult,
                /* [out] */ EXCEPINFO *pExcepInfo,
                /* [out] */ UINT *puArgErr) 
    {
        (void) riid;
        (void) dispIdMember;
        (void) lcid;
        (void) wFlags;
        (void) pExcepInfo;
        (void) puArgErr;
        HRESULT hr = S_OK;
        if (pDispParams == 0) {
            return DISP_E_BADVARTYPE;
        }
        if (pDispParams->cArgs > 8) {
            return DISP_E_BADPARAMCOUNT;
        }
        if (pVarResult != 0) {
            ::VariantInit(pVarResult);
        }
        ATL::CComVariant rgVars[8];
        VARIANT* rgpVars[8];
        UINT index = 0;
        if (pDispParams->cNamedArgs > 0) {
            if (pDispParams->rgdispidNamedArgs[0] == DISPID_PROPERTYPUT) {
                rgpVars[0] = &pDispParams->rgvarg[0];
                index = 1;
            }
            for (; index < pDispParams->cNamedArgs; ++index) {
                if (pDispParams->rgdispidNamedArgs[index] >= (int) pDispParams->cArgs || pDispParams->rgdispidNamedArgs[index] < 0) {
                    if (puArgErr != 0) {
                        *puArgErr = index;
                    }
                    return DISP_E_PARAMNOTFOUND;
                }
                rgpVars[pDispParams->cArgs - pDispParams->rgdispidNamedArgs[index] - 1] = &pDispParams->rgvarg[index];
            }
        }
        for (; index < pDispParams->cArgs; ++index) {
            rgpVars[index] = &pDispParams->rgvarg[index];
        }
        VARIANT v0;
        VARIANT* v;
        switch (dispIdMember) {
        case 1:
            {
                if (pDispParams->cArgs != 2) {
                    return DISP_E_BADPARAMCOUNT;
                }
                v = rgpVars[1];
                if (v->vt != VT_BSTR)
            {
                    rgVars[1] = *rgpVars[1];
                    v = &rgVars[1];
                    if (FAILED(__VariantChangeType(v, &v0, VT_BSTR))) {
                        if (puArgErr != 0) {
                            *puArgErr = 1;
                        }
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                BSTR i1 = (BSTR) V_BSTR(v);
                v = rgpVars[0];
                if (v->vt != VT_I4)
            {
                    rgVars[0] = *rgpVars[0];
                    v = &rgVars[0];
                    if (FAILED(__VariantChangeType(v, &v0, VT_I4))) {
                        if (puArgErr != 0) {
                            *puArgErr = 0;
                        }
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                long i2 = V_I4(v);
                hr = ((::_ICoBrokerProxyEvents*) this)->NotifyLogMessage(i1, i2);
                break;
            }
        case 2:
            {
                if (pDispParams->cArgs != 8) {
                    return DISP_E_BADPARAMCOUNT;
                }
                v = rgpVars[7];
                if (v->vt != VT_BSTR)
            {
                    rgVars[7] = *rgpVars[7];
                    v = &rgVars[7];
                    if (FAILED(__VariantChangeType(v, &v0, VT_BSTR))) {
                        if (puArgErr != 0) {
                            *puArgErr = 7;
                        }
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                BSTR i1 = (BSTR) V_BSTR(v);
                v = rgpVars[6];
                if (v->vt != VT_I4)
            {
                    rgVars[6] = *rgpVars[6];
                    v = &rgVars[6];
                    if (FAILED(__VariantChangeType(v, &v0, VT_I4))) {
                        if (puArgErr != 0) {
                            *puArgErr = 6;
                        }
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                unsigned long i2 = V_I4(v);
                v = rgpVars[5];
                if (v->vt != VT_BSTR)
            {
                    rgVars[5] = *rgpVars[5];
                    v = &rgVars[5];
                    if (FAILED(__VariantChangeType(v, &v0, VT_BSTR))) {
                        if (puArgErr != 0) {
                            *puArgErr = 5;
                        }
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                BSTR i3 = (BSTR) V_BSTR(v);
                v = rgpVars[4];
                if (v->vt != VT_I4)
            {
                    rgVars[4] = *rgpVars[4];
                    v = &rgVars[4];
                    if (FAILED(__VariantChangeType(v, &v0, VT_I4))) {
                        if (puArgErr != 0) {
                            *puArgErr = 4;
                        }
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                unsigned long i4 = V_I4(v);
                v = rgpVars[3];
                if (v->vt != VT_I4)
            {
                    rgVars[3] = *rgpVars[3];
                    v = &rgVars[3];
                    if (FAILED(__VariantChangeType(v, &v0, VT_I4))) {
                        if (puArgErr != 0) {
                            *puArgErr = 3;
                        }
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                unsigned long i5 = V_I4(v);
                v = rgpVars[2];
                if (v->vt != VT_I4)
            {
                    rgVars[2] = *rgpVars[2];
                    v = &rgVars[2];
                    if (FAILED(__VariantChangeType(v, &v0, VT_I4))) {
                        if (puArgErr != 0) {
                            *puArgErr = 2;
                        }
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                unsigned long i6 = V_I4(v);
                v = rgpVars[1];
                if (v->vt != VT_I4)
            {
                    rgVars[1] = *rgpVars[1];
                    v = &rgVars[1];
                    if (FAILED(__VariantChangeType(v, &v0, VT_I4))) {
                        if (puArgErr != 0) {
                            *puArgErr = 1;
                        }
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                unsigned long i7 = V_I4(v);
                v = rgpVars[0];
                if (v->vt != VT_BSTR)
            {
                    rgVars[0] = *rgpVars[0];
                    v = &rgVars[0];
                    if (FAILED(__VariantChangeType(v, &v0, VT_BSTR))) {
                        if (puArgErr != 0) {
                            *puArgErr = 0;
                        }
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                BSTR i8 = (BSTR) V_BSTR(v);
                hr = ((::_ICoBrokerProxyEvents*) this)->RequestSent(i1, i2, i3, i4, i5, i6, i7, i8);
                break;
            }
        default:
            return DISP_E_MEMBERNOTFOUND;
        }
        if (FAILED(hr) && pExcepInfo != NULL)
        {
            AtlExcepInfoFromErrorInfo(hr, pExcepInfo);
        }
        return hr;
    }
    virtual HRESULT STDMETHODCALLTYPE _ICoBrokerProxyEvents::GetIDsOfNames(
                /* [in] */ REFIID riid,
                /* [size_is][in] */ LPOLESTR *rgszNames,
                /* [in] */ UINT cNames,
                /* [in] */ LCID lcid,
                /* [size_is][out] */ DISPID *rgDispId) 
    {
        (void) riid;
        (void) rgszNames;
        (void) cNames;
        (void) lcid;
        (void) rgDispId;
        static LPOLESTR names[] = { L"message", L"severity", L"NotifyLogMessage", L"dstUserId", L"dstSvcId", L"srcUserId", L"srcSvcId", L"rId", L"rType", L"param", L"params", L"RequestSent" };
        static DISPID dids[] = { 0, 1, 1, 0, 1, 2, 3, 4, 5, 6, 7, 2 };
        for (unsigned int i = 0; i < cNames; ++i) {
            int fFoundIt = 0;
            for (unsigned int j = 0; j < sizeof(names)/sizeof(LPOLESTR); ++j) {
                if (_wcsicmp(rgszNames[i], names[j]) == 0) {
                    fFoundIt = 1;
                    rgDispId[i] = dids[j];
                    break;
                }
            }
            if (fFoundIt == 0) {
                return DISP_E_UNKNOWNNAME;
            }
        }
        return S_OK;
    }
    HRESULT TypeInfoHelper(REFIID iidDisp, LCID /*lcid*/, ITypeInfo** ppTypeInfo) 
    {
        if (ppTypeInfo == NULL) {
            return E_POINTER;
        }
        *ppTypeInfo = NULL;
        TCHAR szModule1[_MAX_PATH];
        int nLen = ::GetModuleFileName(_AtlBaseModule.GetModuleInstance(), szModule1, _MAX_PATH);
        if (nLen == 0 || nLen == _MAX_PATH) {
            return E_FAIL;
        }
        USES_CONVERSION_EX;
        LPOLESTR pszModule = T2OLE_EX(szModule1, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
        if (pszModule == NULL) {
            return E_OUTOFMEMORY;
        }
#endif
        CComPtr<ITypeLib> spTypeLib;
        HRESULT hr = LoadTypeLib(pszModule, &spTypeLib);
        if (SUCCEEDED(hr)) {
            CComPtr<ITypeInfo> spTypeInfo;
            hr = spTypeLib->GetTypeInfoOfGuid(iidDisp, &spTypeInfo);
            if (SUCCEEDED(hr)) {
                *ppTypeInfo = spTypeInfo.Detach();
            }
        }
        return hr;
    }
    virtual HRESULT STDMETHODCALLTYPE _ICoBrokerProxyEvents::GetTypeInfoCount(unsigned int*  pctinfo) 
    {
        if (pctinfo == NULL) {
            return E_POINTER;
        }
        CComPtr<ITypeInfo> spTypeInfo;
        *pctinfo = 
                       (SUCCEEDED(TypeInfoHelper(__uuidof(_ICoBrokerProxyEvents), 0, &spTypeInfo))) ? 1 : 0;
        return S_OK;
    }
    virtual HRESULT STDMETHODCALLTYPE _ICoBrokerProxyEvents::GetTypeInfo(unsigned int iTInfo, LCID lcid, ITypeInfo** ppTInfo) 
    {
        if (iTInfo != 0) {
            return DISP_E_BADINDEX;
        }
        return TypeInfoHelper(__uuidof(_ICoBrokerProxyEvents), lcid, ppTInfo);
    }

	LONG m_cRef;
	STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

};
