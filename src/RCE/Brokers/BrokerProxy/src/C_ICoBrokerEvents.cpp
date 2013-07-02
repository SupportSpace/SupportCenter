/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  C_IRCViewerAXCtrlEvents.cpp
///
///  C_IRCViewerAXCtrlEvents object implementation. The object is events receiever of RCViewerAXCtrl events
///
///  @author Kirill Solovyov @date 05.10.2007
///
////////////////////////////////////////////////////////////////////////
// C_ICoBrokerEvents.cpp : Implementation of C_ICoBrokerEvents

#include "stdafx.h"
#include "C_ICoBrokerEvents.h"
#include <AidLib/CException/CException.h>
#include <RCEngine/AXstuff/AXstuff.h>
#include <AidLib/Com/ComException.h>


// C_ICoBrokerEvents

C_ICoBrokerEvents::C_ICoBrokerEvents():
	m_owner(NULL),
	CInstanceTracker(_T("C_ICoBrokerEvents"))
{
TRY_CATCH
CATCH_LOG()
}

C_ICoBrokerEvents::~C_ICoBrokerEvents()
{
TRY_CATCH
	//stupidity
	if(m_unkn.p)
		EventUnadvise();
	Log.Add(_MESSAGE_,_T("C_ICoBrokerEvents::~C_ICoBrokerEvents"));
CATCH_LOG()
}

HRESULT C_ICoBrokerEvents::EventAdvise(IUnknown *unkn)
{
TRY_CATCH
	m_unkn=unkn;
	return __hook(_ICoBrokerEvents,m_unkn);
CATCH_THROW()
}

HRESULT C_ICoBrokerEvents::EventUnadvise(void)
{
TRY_CATCH
	CComPtr<IUnknown> unkn=m_unkn;
	m_unkn.Release();
	return __unhook(_ICoBrokerEvents,unkn);
CATCH_THROW()
}

STDMETHODIMP C_ICoBrokerEvents::NotifyLogMessage(BSTR message, long severity)
{
TRY_CATCH
	if(!m_owner)
		throw MCException("Owner don't set");
	m_owner->NotifyLogMessage(message,severity);
	return S_OK;
CATCH_LOG()
	return E_FAIL;
}

STDMETHODIMP C_ICoBrokerEvents::RequestSent(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params)
{
TRY_CATCH
	if(!m_owner)
		throw MCException("Owner don't set");
	USES_CONVERSION;
	m_owner->FireRequestSent(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
	return S_OK;
CATCH_LOG()
	return E_FAIL;
}

//C_ICoBrokerEvents1::C_ICoBrokerEvents1(IUnknown* unkn)//:
//	//m_owner(NULL)
//{
//TRY_CATCH
//	m_unkn=unkn;
//	__hook(&_ICoBrokerEvents::NotifyLogMessage,unkn,&C_ICoBrokerEvents1::NotifyLogMessage);
//	__hook(&_ICoBrokerEvents::RequestSent,unkn,&C_ICoBrokerEvents1::RequestSent);
//CATCH_LOG()
//}
//
//
//
//C_ICoBrokerEvents1::~C_ICoBrokerEvents1()
//{
//TRY_CATCH
//	__unhook(&_ICoBrokerEvents::NotifyLogMessage,m_unkn,&C_ICoBrokerEvents1::NotifyLogMessage);
//	__unhook(&_ICoBrokerEvents::RequestSent,m_unkn,&C_ICoBrokerEvents1::RequestSent);
//CATCH_LOG()
//}
