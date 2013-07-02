/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  C_IRCViewerAXCtrlEvents.h
///
///  C_IRCViewerAXCtrlEvents object declaration. The object is events receiever of RCViewerAXCtrl events
///
///  @author Kirill Solovyov @date 05.10.2007
///
////////////////////////////////////////////////////////////////////////
// C_IRCViewerAXCtrlEvents.h : Declaration of the C_IRCViewerAXCtrlEvents

#pragma once
#include "resource.h"       // main symbols
#include "CCoVBroker.h"
class CCoVBroker;

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

//embedded_idl("emitidl"),
#import "RCUI.dll" raw_interfaces_only, raw_native_types, no_namespace, named_guids, auto_search

// C_IRCViewerAXCtrlEvents
class ATL_NO_VTABLE C_IRCViewerAXCtrlEvents :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<C_IRCViewerAXCtrlEvents>,
	public IDispEventImpl<0, C_IRCViewerAXCtrlEvents, &DIID__IRCViewerAXCtrlEvents, &LIBID_RCUI, 1, 0>
{
public:
	C_IRCViewerAXCtrlEvents();
	DECLARE_PROTECT_FINAL_CONSTRUCT()
	HRESULT FinalConstruct();
	void FinalRelease();
	BEGIN_COM_MAP(C_IRCViewerAXCtrlEvents)
		COM_INTERFACE_ENTRY_IID(DIID__IRCViewerAXCtrlEvents, C_IRCViewerAXCtrlEvents)
	END_COM_MAP()

	BEGIN_SINK_MAP(C_IRCViewerAXCtrlEvents)
		SINK_ENTRY_EX(0, DIID__IRCViewerAXCtrlEvents, 1, NotifySessionStop)
		SINK_ENTRY_EX(0, DIID__IRCViewerAXCtrlEvents, 2, NotifySessionStart)
		SINK_ENTRY_EX(0, DIID__IRCViewerAXCtrlEvents, 3, NotifyConnecting)
		SINK_ENTRY_EX(0, DIID__IRCViewerAXCtrlEvents, 4, NotifyUIEvent)
	END_SINK_MAP()
	/// owner of this event receiver
	CCoVBroker *m_owner;
	// _IRCViewerAXCtrlEvents Methods
public:
	STDMETHOD(NotifySessionStop)(long reasonCode);
	STDMETHOD(NotifySessionStart)(long connectType);
	STDMETHOD(NotifyConnecting)(long percentCompleted, BSTR status);
	STDMETHOD(NotifyUIEvent)(long eventType, long param);
	
};


// C_IRCViewerAXCtrlEvents1
// don't work with #export
//[event_receiver(com,false)]
//class ATL_NO_VTABLE C_IRCViewerAXCtrlEvents1 
//{
//public:
//	C_IRCViewerAXCtrlEvents1();
//	void EventAdvise(IUnknown* source);
//	void EventUnadvise(IUnknown* source);
//
//	// _IRCViewerAXCtrlEvents Methods
//public:
//	STDMETHOD(NotifySessionStop)(long reasonCode);
//	STDMETHOD(NotifySessionStart)(long connectType);
//	STDMETHOD(NotifyConnecting)(long percentCompleted, BSTR status);
//	STDMETHOD(NotifyUIEvent)(long eventType, long param);
//
//};