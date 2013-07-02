
#pragma once
#include "resource.h"
#include <atlctl.h>
#include <RCEngine/Mediator/CMediatorRefImpl.h>
#include <RCEngine/sitelock/RCSiteLockImpl.h>
#include <RCEngine/SafeEvents/CSafeEvents.h>
#include <AidLib/Logging/CInstanceTracker.h>
#include "..\..\Brokers\Shared\IBrokerClient.h"
#include "..\..\Brokers\Shared\CAvailableServices.h"
#include "..\..\Brokers\Shared\BrokersTypes.h"
#include "..\..\Brokers\Shared\CRIdGen.h"
#include "CCommandManager.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

#define TRACENAME		ATLTRACE(__FUNCTION__);

/// Dialog based CComControl analogue
template <class T>
class ATL_NO_VTABLE CComDlgCtrl : public CComControlBase,
	public CDialogImpl<T>
{
public:
	CComDlgCtrl() : CComControlBase(m_hWnd) {}
	HRESULT FireOnRequestEdit(DISPID dispID)
	{
		//TRACENAME
		//
		//T* pT = static_cast<T*>(this);
		//return T::__ATL_PROP_NOTIFY_EVENT_CLASS::FireOnRequestEdit
		//	(pT->GetUnknown(), dispID);
		
		return S_OK;
	}
	HRESULT FireOnChanged(DISPID dispID)
	{
		//TRACENAME
		//
		//T* pT = static_cast<T*>(this);
		//return T::__ATL_PROP_NOTIFY_EVENT_CLASS::FireOnChanged
		//	(pT->GetUnknown(), dispID);
		
		return S_OK;
	}
	virtual HRESULT ControlQueryInterface(const IID& iid, void** ppv)
	{
		TRACENAME
		
		T* pT = static_cast<T*>(this);
		return pT->_InternalQueryInterface(iid, ppv);
	}
	virtual HWND CreateControlWindow(HWND hWndParent, RECT& rcPos)
	{
		TRACENAME
		
		T* pT = static_cast<T*>(this);
		return pT->Create(hWndParent);
		// CDialogImpl::Create differs from CWindowImpl
	}
};

// ICoFAViewer
[
	object,
	uuid(1CBC5773-1373-4C93-BDEA-7F378D536B34),
	dual,
	helpstring("ICoFAViewer Interface"),
	pointer_default(unique)
]
__interface ICoFAViewer
{
};


// _ICoFAViewerEvents
[
	uuid("ABC07A58-D935-4B35-8D21-450AB34D53ED"),
	dispinterface,
	helpstring("_ICoFAViewerEvents Interface")
]
__interface _ICoFAViewerEvents
{
	/// Called to notify session started
	[id(1), helpstring("method NotifySessionStart")] HRESULT NotifySessionStart(void);
	
	/// Called to notify session stopped
	/// @param reasonCode stop reason (Stop() called; Remote Stop() called; Stream signaled end-of-file; etc.) LOCAL_STOP=0, REMOTE_STOP=1, STREAM_ERROR=2, PROTOCOL_ERROR=3, CHANGE_DISPLAY_MODE=4,CONNECTING_ERROR=5, OPENFILE_ERROR=6. See _ESessionStopReason definition for last version of reason code.
	[id(2), helpstring("method NotifySessionStop")] HRESULT NotifySessionStop([in] LONG reasonCode);
};


// Light weight implementation of file transfer UI
[
	coclass,
	threading(apartment),
	vi_progid("FTUI.CoFAViewer"),
	progid("FTUI.CoFAViewer.1"),
	version(1.0),
	uuid("A64B54A3-3F5B-493F-879F-41A71B09F098"),
	helpstring("CoFAViewer Class"),
	event_source(com),
	support_error_info(ICoFAViewer),
	registration_script("control.rgs")
]
class ATL_NO_VTABLE CCoFAViewer :
	public CInstanceTracker,
	public IBrokerClient,
	public CSafeEvents<CCoFAViewer,_ICoFAViewerEvents>,
	public IPersistStreamInitImpl<CCoFAViewer>,
	public IOleControlImpl<CCoFAViewer>,
	public IOleObjectImpl<CCoFAViewer>,
	public IOleInPlaceActiveObjectImpl<CCoFAViewer>,
	public IViewObjectExImpl<CCoFAViewer>,
	public IOleInPlaceObjectWindowlessImpl<CCoFAViewer>,
	public IPersistStorageImpl<CCoFAViewer>,
	public ISpecifyPropertyPagesImpl<CCoFAViewer>,
	public IQuickActivateImpl<CCoFAViewer>,
	public IDispatchImpl<ICoFAViewer, &__uuidof(ICoFAViewer), &LIB_GUID>,
	public CRCSiteLockImpl<CCoFAViewer>,
	public IDataObjectImpl<CCoFAViewer>,
	public CComCompositeControl/*CComDlgCtrl*/<CCoFAViewer>,
	protected CRIdGen
{
	boost::shared_ptr<CCommandManager> m_commandManager;
public:

	/// ActiveX friendly name, use by registration
	static const TCHAR* GetObjectFriendlyName() 
	{
		return _T("Support Platform File Access Viewer");
	}
	/// Initializes object instance
	CCoFAViewer();
	~CCoFAViewer();

	enum { IDD = IDD_LIGHTVIEWERDLG };

///DOXYS_OFF
BEGIN_COM_MAP(CCoFAViewer)
	COM_INTERFACE_ENTRY(IObjectSafety)
	COM_INTERFACE_ENTRY(IObjectSafetySiteLock)
	COM_INTERFACE_ENTRY(ICoFAViewer)
	COM_INTERFACE_ENTRY(IBrokerClient)
END_COM_MAP( )

DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_INSIDEOUT |
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST
)

BEGIN_PROP_MAP(CCoFAViewer)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries	   
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_MSG_MAP(CCoFAViewer)
TRY_CATCH
	MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
	MESSAGE_HANDLER(m_msgSessionStarted, FireEventSessionStarted)
	MESSAGE_HANDLER(m_msgSessionStopped, FireEventSessionStopped)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroyDialog)
	MESSAGE_HANDLER(WM_SIZE, OnSizeDialog)
	REFLECT_NOTIFICATIONS()
	DEFAULT_REFLECTION_HANDLER()
CATCH_LOG()
END_MSG_MAP()

	__event __interface _ICoFAViewerEvents;
// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

///DOXYS_ON

// ICoFAViewer
public:

	/// Requiret to handle dialog based com control
	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam, BOOL&bHandled);
	LRESULT OnDestroyDialog(UINT uMsg,WPARAM wParam,LPARAM lParam, BOOL&bHandled);
	LRESULT OnSizeDialog(UINT uMsg,WPARAM wParam,LPARAM lParam, BOOL&bHandled);

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		TRACENAME
		
		return S_OK;
	}

	void FinalRelease()
	{
		TRACENAME
	}

private:
	/// Internal messages
	/// Session started message
	const UINT m_msgSessionStarted;
	/// Session stopped message
	const UINT m_msgSessionStopped;

	/// Controls mouse activate messages in order to select viewer's vidget by click
	LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/// Session started message handler. Raises NotifySessionStart ActiveX event
	LRESULT FireEventSessionStarted(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/// Session stopped message handler. Raises NotifySessionStop ActiveX event
	LRESULT FireEventSessionStopped(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

protected:
	/// Internal stream
	boost::shared_ptr<CAbstractStream> m_stream;
	
	/// Send accept/deny request button click event handler
	void OnSendRequestBtn();

	/// critical section
	CRITICAL_SECTION	m_cs;

	/// Infrastracture
	
	/// borker intercommunication
	CComGITPtr<_IBrokerClientEvents> m_brokerEvents;

	/// The method initialize viewer by _IBrokerClientEvents interface
	STDMETHOD(Init)(IUnknown *events);
	
	/// The method handle request sent by Broker
	STDMETHOD(HandleRequest)(BSTR dstUserId,ULONG dstSvcId,BSTR srcUserId,ULONG srcSvcId,ULONG rId,ULONG rType,ULONG param,BSTR params);

	/// The method call when asked sub stream connected
	STDMETHOD(SetSubStream)(BSTR dstUserId, ULONG dstSvcId, ULONG streamId, ULONG pointer_shared_ptr_stream);

public:
	virtual HWND CreateControlWindow(HWND hWndParent, RECT& rcPos)
	{
		// Taken from the deep inside of ATL
		CComControl< CCoFAViewer, CAxDialogImpl< CCoFAViewer > >::Create(hWndParent);
		SetBackgroundColorFromAmbient();

		if (m_hWnd != NULL)
			AdviseSinkMap(true);

		return m_hWnd;
	}
};

