/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SSInitialInstaller.h
///
///  CSSInitialInstaller, initial installation object 
///
///  @author "Archer Software" Kirill Solovyov. @date 22.03.2007
///
////////////////////////////////////////////////////////////////////////
// SSInitialInstaller.h : Declaration of the CSSInitialInstaller
#pragma once
#include "cLogLight.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// ISSInitialInstaller
[
	object,
	uuid(28B0285F-9552-4CD1-A575-1BEB074BF4E6),
	dual,
	helpstring("ISSInitialInstaller Interface"),
	pointer_default(unique)
]
__interface ISSInitialInstaller : public IDispatch
{
	[id(1), helpstring("method Install")] HRESULT Install([in] ULONG base, [in] BSTR url, [in] BSTR params, [in] ULONG flags, [out,retval] ULONG* msiResult);
	[id(2), helpstring("method GetComponentVersion")] HRESULT GetComponentVersion([in] BSTR componentGUID, [in] BSTR productKey, [in] SHORT keyType, [out,retval] BSTR* version);
};


// _ISSInitialInstallerEvents
[
	uuid("C498C47E-804A-4BF5-B0EE-5099BA9BFC25"),
	dispinterface,
	helpstring("_ISSInitialInstallerEvents Interface")
]
__interface _ISSInitialInstallerEvents
{
};

/// Install flags
#define IF_SYNC					0x01 // synchronous call; 
#define IF_FREE_LIB			0x02 // call CoFreeUnusedLibraries() function before; 
#define IF_RELAUNCH			0x04 // relaunch browser
#define IF_NOINSTALL		0x08 // no installation process
#define IF_SHOW_CMD			0x0F000000 // Show command line (for debug purposes)

// CSSInitialInstaller
[
	coclass,
	control,
	default(ISSInitialInstaller, _ISSInitialInstallerEvents),
	threading(apartment),
	vi_progid("SSInitial.SSInitialInstaller"),
	progid("SSInitial.SSInitialInstaller.1"),
	version(1.0),
	uuid("08653405-44A9-4E99-9C09-DD00770AAA08"),
	helpstring("SSInitialInstaller Class"),
	event_source(com),
	support_error_info(ISSInitialInstaller),
	registration_script("control.rgs")
]
class ATL_NO_VTABLE CSSInitialInstaller :
	public cLogLight,
	public ISSInitialInstaller,
	public IPersistStreamInitImpl<CSSInitialInstaller>,
	public IOleControlImpl<CSSInitialInstaller>,
	public IOleObjectImpl<CSSInitialInstaller>,
	public IOleInPlaceActiveObjectImpl<CSSInitialInstaller>,
	public IViewObjectExImpl<CSSInitialInstaller>,
	public IOleInPlaceObjectWindowlessImpl<CSSInitialInstaller>,
	public IPersistStorageImpl<CSSInitialInstaller>,
	public ISpecifyPropertyPagesImpl<CSSInitialInstaller>,
	public IQuickActivateImpl<CSSInitialInstaller>,
#ifndef _WIN32_WCE
	public IDataObjectImpl<CSSInitialInstaller>,
	public CRCSiteLockImpl<CSSInitialInstaller>,
#endif
#ifdef _WIN32_WCE // IObjectSafety is required on Windows CE for the control to be loaded correctly
	public IObjectSafetyImpl<CSSInitialInstaller, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
#endif
	public CComControl<CSSInitialInstaller>
{
public:

	///Initializes object instance
	CSSInitialInstaller();

	/// Destroys object instance
	virtual ~CSSInitialInstaller();

	public:
	HRESULT OnDraw(ATL_DRAWINFO& di);
	DECLARE_PROTECT_FINAL_CONSTRUCT()
	HRESULT FinalConstruct();
	void FinalRelease();

	/// The method retrieve creating page URL
	/// @return url string in case successful, otherwise throw std::exception
	tstring GetPageUrl();
	
	/// The method retrieve installation (codebase) URL
	/// @return url string in case successful, otherwise throw std::exception
	tstring GetCodeBaseUrl();

protected:
	/// The method output logging message
	/// @param tstring logging message
	virtual void AddList(const tstring)throw();

public:

	/// The method execute installation due to Msiexec utility.
	/// @param base Base url may be equal 0 - this component codebase installation url, 1 - creating page url
	/// @param url Relative part url, it is appended to base parametr
	/// @param flags Method behaviour. Any combination of permitted flags is acceptable for the this parameter. 0x01 - synchronous call; 0x02 - call CoFreeUnusedLibraries() function before; 0x04 - rerun browser
	/// @param msiResult 
	STDMETHOD(Install)(ULONG base, BSTR url, BSTR params, ULONG flags,ULONG* msiResult);

	/// Returns version string for the product component
	/// @param componentGUID GUID of component, which version should be retrived
	/// @param productKey key, used to identify product, it can be one of: product dode; upgrade code; GUID of some product compoents
	/// @see keyType
	/// @param keyType type of productKey, i.e. one of product dode; upgrade code; GUID of some product compoents
	/// following integers should be used:
	/// 0 - product code
	/// 1 - upgrade code
	/// 2 - component GUID
	/// @param version in case of successfull version retriving - version is stored here
	STDMETHOD(GetComponentVersion)(BSTR componentGUID, BSTR productKey, SHORT keyType, BSTR* version);
	

	///DOXYS_OFF
DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_INSIDEOUT |
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST)

BEGIN_PROP_MAP(CSSInitialInstaller)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
END_PROP_MAP()

BEGIN_MSG_MAP(CSSInitialInstaller)
	CHAIN_MSG_MAP(CComControl<CSSInitialInstaller>)
	DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
///DOXYS_ON

	__event __interface _ISSInitialInstallerEvents;
// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)
};

