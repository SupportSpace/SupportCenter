/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  C_IRCViewerAXCtrlEvents.h
///
///  C_IRCViewerAXCtrlEvents object declaration. The object is events receiever of CoBroker events
///
///  @author Kirill Solovyov @date 12.10.2007
///
////////////////////////////////////////////////////////////////////////
// C_ICoBrokerEvents.h : Declaration of the C_ICoBrokerEvents

#pragma once
#include "resource.h"       // main symbols
#include <AidLib/Logging/CInstanceTracker.h>

class C_ICoBrokerEvents;
#include "CCoBrokerProxy.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif


#import "Broker.dll" raw_interfaces_only, raw_native_types, no_namespace, named_guids, auto_search, embedded_idl, exclude("_ICoBPInstallerEvents")

//// C_ICoBrokerEvents
//class ATL_NO_VTABLE C_ICoBrokerEvents :
//	public CComObjectRootEx<CComSingleThreadModel>,
//	public CComCoClass<C_ICoBrokerEvents>,
//	public IDispEventImpl<0, C_ICoBrokerEvents, &__uuidof(_ICoBrokerEvents), &LIBID_Broker, 1, 0>
//
//	//public CComObjectRootEx<CComSingleThreadModel>,
//	//public CComCoClass<C_ICoBrokerEvents>,
//	//public IDispatchImpl<_ICoBrokerEvents, &__uuidof(_ICoBrokerEvents), &LIBID_Broker, /* wMajor = */ 1>
//{
//public:
//	C_ICoBrokerEvents();
//
//	DECLARE_PROTECT_FINAL_CONSTRUCT()
//
//	HRESULT FinalConstruct();
//	void FinalRelease();
//	
//	BEGIN_COM_MAP(C_ICoBrokerEvents)
//		COM_INTERFACE_ENTRY_IID(__uuidof(_ICoBrokerEvents), C_ICoBrokerEvents)
//	END_COM_MAP()
//
//	BEGIN_SINK_MAP(C_ICoBrokerEvents)
//		SINK_ENTRY_EX(0, __uuidof(_ICoBrokerEvents), 1, NotifyLogMessage)
//		SINK_ENTRY_EX(0, __uuidof(_ICoBrokerEvents), 2, RequestSent)
//	END_SINK_MAP()
//
//	/// owner of this events receiver
//	CCoBrokerProxy* m_owner;
//
//	// _ICoBrokerEvents Methods
//public:
//	STDMETHOD(NotifyLogMessage)(BSTR message, long severity);
//	STDMETHOD(RequestSent)(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params);
//};

[
	coclass,
	noncreatable,
	uuid("A22D0752-647F-4674-83CD-8892D7438A2C"),
	event_receiver(com,true)
]
class C_ICoBrokerEvents:
	public _ICoBrokerEvents,
	public CInstanceTracker
{
protected:
	CComPtr<IUnknown> m_unkn;
public:
	C_ICoBrokerEvents();
	~C_ICoBrokerEvents();
	HRESULT EventAdvise(IUnknown* unkn);
	HRESULT EventUnadvise(void);
	CCoBrokerProxy* m_owner;

	// _ICoBrokerEvents Methods
public:
	STDMETHOD(NotifyLogMessage)(BSTR message, long severity);
	STDMETHOD(RequestSent)(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params);
};