#include "StdAfx.h"
#include "C_ICoBrokerProxyEvents.h"
#include <AidLib/Logging/CLogFolder.h>

C_ICoBrokerProxyEvents::C_ICoBrokerProxyEvents(void)
{
TRY_CATCH
	m_cRef=0;
CATCH_LOG(_T("C_ICoBrokerProxyEvents::C_ICoBrokerProxyEvents"))
}

C_ICoBrokerProxyEvents::~C_ICoBrokerProxyEvents(void)
{
TRY_CATCH
CATCH_LOG(_T("C_ICoBrokerProxyEvents::~C_ICoBrokerProxyEvents"))
}

STDMETHODIMP C_ICoBrokerProxyEvents::NotifyLogMessage(BSTR message, long severity)
{
TRY_CATCH
	USES_CONVERSION;
	
	if(m_owner!=NULL)
	{
		AtlTrace("C_ICoBrokerProxyEvents::NotifyLogMessage is called(%s,0x%x)",OLE2A(message),severity);
		m_owner->DoNotifyLogMessage(OLE2A(message), severity);
	}
	else
		AtlTrace("C_ICoBrokerProxyEvents::NotifyLogMessage. m_owner==NULL");

CATCH_LOG(_T("C_ICoBrokerProxyEvents::NotifyLogMessage"))
	return S_OK;
}

STDMETHODIMP C_ICoBrokerProxyEvents::RequestSent(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params)
{
TRY_CATCH
	USES_CONVERSION;

	if(m_owner!=NULL)
		m_owner->DoRequestSent(OLE2A(dstUserId), dstSvcId, OLE2A(srcUserId), srcSvcId, rId, rType, param, OLE2A(params));
	else
		Log.Add(_WARNING_, _T("C_ICoBrokerProxyEvents::RequestSent. m_owner==NULL"));

CATCH_LOG(_T("C_ICoBrokerProxyEvents::RequestSent"))
	return S_OK;
}

STDMETHODIMP C_ICoBrokerProxyEvents::QueryInterface(REFIID riid, void **ppv)
{
	if(!ppv)
		return E_NOINTERFACE;
	if(riid==__uuidof(IUnknown))
		*ppv=static_cast<IUnknown*>(this);
	else if(riid==__uuidof(IDispatch))
		*ppv=static_cast<IDispatch*>(this);
	else if(riid==__uuidof(_ICoBrokerProxyEvents))
		*ppv=static_cast<_ICoBrokerProxyEvents*>(this);
	else
	{
		*ppv=NULL;
		return E_NOINTERFACE;
	}
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) C_ICoBrokerProxyEvents::AddRef(void)
{
	return ++m_cRef;
}

STDMETHODIMP_(ULONG) C_ICoBrokerProxyEvents::Release(void)
{
	return --m_cRef;
}

