/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  C_IRCHostAXCtrlEvents.h
///
///  C_IRCHostAXCtrlEvents object declaration. The object is events receiever of RCHostAXCtrl events
///
///  @author Kirill Solovyov @date 24.10.2007
///
////////////////////////////////////////////////////////////////////////
// C_IRCHostAXCtrlEvents.h : Declaration of the C_IRCHostAXCtrlEvents

#pragma once
#include "resource.h"       // main symbols

#include "CCoBroker.h"
class CCoBroker;


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

#import "RCUI.dll" raw_interfaces_only, raw_native_types, no_namespace, named_guids, auto_search

// C_IRCHostAXCtrlEvents
class ATL_NO_VTABLE C_IRCHostAXCtrlEvents :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<C_IRCHostAXCtrlEvents>,
	public IDispEventImpl<0, C_IRCHostAXCtrlEvents, &__uuidof(_IRCHostAXCtrlEvents), &LIBID_RCUI, 1, 0>
{
public:
	C_IRCHostAXCtrlEvents();

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

	BEGIN_COM_MAP(C_IRCHostAXCtrlEvents)
		COM_INTERFACE_ENTRY_IID(__uuidof(_IRCHostAXCtrlEvents), C_IRCHostAXCtrlEvents)
	END_COM_MAP()

	BEGIN_SINK_MAP(C_IRCHostAXCtrlEvents)
		SINK_ENTRY_EX(0, __uuidof(_IRCHostAXCtrlEvents), 1, NotifySessionStart)
		SINK_ENTRY_EX(0, __uuidof(_IRCHostAXCtrlEvents), 2, NotifySessionStop)
		SINK_ENTRY_EX(0, __uuidof(_IRCHostAXCtrlEvents), 3, NotifyConnecting)
	END_SINK_MAP()

	/// owner of this events receiver
	CCoBroker* m_owner;


	// _IRCHostAXCtrlEvents Methods
public:
	STDMETHOD(NotifySessionStart)(long clientId);
	STDMETHOD(NotifySessionStop)(long clientId, long reasonCode);
	STDMETHOD(NotifyConnecting)(long percentCompleted, BSTR status);
};

