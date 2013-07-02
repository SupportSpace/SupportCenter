/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  C_IRCInstallerAXCtrlEvents.h
///
///  C_IRCInstallerAXCtrlEvents object declaration. The object is events receiever of RCInstallerAXCtrl events
///
///  @author Kirill Solovyov @date 24.10.2007
///
////////////////////////////////////////////////////////////////////////
// C_IRCInstallerAXCtrlEvents.h : Declaration of the C_IRCInstallerAXCtrlEvents
#pragma once

class C_IRCInstallerAXCtrlEvents;
#include "CCoBroker.h"




#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

#import "RCInstaller.dll" raw_interfaces_only, raw_native_types, no_namespace, named_guids, auto_search

class ATL_NO_VTABLE C_IRCInstallerAXCtrlEvents :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<C_IRCInstallerAXCtrlEvents>,
	public IDispEventImpl<0, C_IRCInstallerAXCtrlEvents, &__uuidof(_IRCInstallerAXCtrlEvents), &LIBID_RCInstaller, 1, 0>
{
public:
	C_IRCInstallerAXCtrlEvents();
	DECLARE_PROTECT_FINAL_CONSTRUCT()
	HRESULT FinalConstruct();
	void FinalRelease();

	BEGIN_COM_MAP(C_IRCInstallerAXCtrlEvents)
		COM_INTERFACE_ENTRY_IID(__uuidof(_IRCInstallerAXCtrlEvents), C_IRCInstallerAXCtrlEvents)
	END_COM_MAP()

	BEGIN_SINK_MAP(C_IRCInstallerAXCtrlEvents)
		SINK_ENTRY_EX(0, __uuidof(_IRCInstallerAXCtrlEvents), 1, NotifyLogMessage)
		SINK_ENTRY_EX(0, __uuidof(_IRCInstallerAXCtrlEvents), 2, NotifyFeatureInstalled)
		SINK_ENTRY_EX(0, __uuidof(_IRCInstallerAXCtrlEvents), 3, NotifyInstalling)
	END_SINK_MAP()
	/// owner of this event receiver
	CCoBroker *m_owner;


	// _IRCInstallerAXCtrlEvents Methods
public:
	
	STDMETHOD(NotifyLogMessage)(BSTR message, long severity);
	
	STDMETHOD(NotifyFeatureInstalled)(long result);
	
	STDMETHOD(NotifyInstalling)(long percentCompleted, BSTR status);
};

