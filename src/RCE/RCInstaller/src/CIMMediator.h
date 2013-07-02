/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CIMMediator.h
///
///  Declares CIMMediator class, responsible for IMMediator ActiveX
///
///  @author Dmitry Netrebenko @date 26.12.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once
#include "resource.h"       // main symbols
#include <atlctl.h>
#include <map>
#include <AidLib/CException/CException.h>

#include <RCEngine/sitelock/RCSiteLockImpl.h>

///  Map entry
typedef std::pair<tstring, tstring> MsgMapEntry;
///  Messages map
typedef std::map<tstring, tstring> MsgMap;


// IIMMediator
[
	object,
	uuid(11B0E69C-A4D3-4142-926F-EED8660AB5D9),
	dual,
	helpstring("IIMMediator Interface"),
	pointer_default(unique)
]
__interface IIMMediator : public IDispatch
{
	[id(1), helpstring("method HandleMsgInternal")] HRESULT HandleMsgInternal([in] BSTR peerId, [in] BSTR msg);
	[id(2), helpstring("method SendMsg")] HRESULT SendMsg([in] BSTR peerId, [in] BSTR msg);
	[id(3), helpstring("method HandleMsg")] HRESULT HandleMsg([in] BSTR peerId, [out,retval] BSTR* msg);
	[id(4), helpstring("method ResetMap")] HRESULT ResetMap();
	[id(5), helpstring("method GetEvent")] HRESULT GetEvent([out,retval] /*HANDLE*/LONGLONG* hEvent);
};


// _IIMMediatorEvents
[
	uuid("FC5F5FAE-D3C0-434F-9030-53DE9081EC82"),
	dispinterface,
	helpstring("_IIMMediatorEvents Interface")
]
__interface _IIMMediatorEvents
{
	[id(1), helpstring("method OnSendMessage")] HRESULT OnSendMessage([in] BSTR peerId, [in] BSTR msg);
};

// CIMMediator
[
	coclass,
	threading(apartment),
	vi_progid("RCInstaller.IMMediator"),
	progid("RCInstaller.IMMediator.1"),
	version(2.0),
	uuid("3FE73D88-72F5-4526-A106-FAA12DE9A619"),
	helpstring("IMMediator Class"),
	event_source(com),
	support_error_info(IIMMediator),
	registration_script("control.rgs")
]
class ATL_NO_VTABLE CIMMediator : 
	public CInstanceTracker,
	public IIMMediator,
	public IPersistStreamInitImpl<CIMMediator>,
	public IOleControlImpl<CIMMediator>,
	public IOleObjectImpl<CIMMediator>,
	public IOleInPlaceActiveObjectImpl<CIMMediator>,
	public IViewObjectExImpl<CIMMediator>,
	public IOleInPlaceObjectWindowlessImpl<CIMMediator>,
	public IPersistStorageImpl<CIMMediator>,
	public ISpecifyPropertyPagesImpl<CIMMediator>,
	public IQuickActivateImpl<CIMMediator>,
	public IDataObjectImpl<CIMMediator>,
	public CComControl<CIMMediator>,
//	public IObjectSafetyImpl<CIMMediator,INTERFACESAFE_FOR_UNTRUSTED_CALLER|INTERFACESAFE_FOR_UNTRUSTED_DATA>
	public CRCSiteLockImpl<CIMMediator>

{
private:
///  Messages map
	MsgMap m_MessagesMap;
///  Critical section
	CRITICAL_SECTION m_MapSection;
/// message for send message event
	const UINT m_msgFireEventOtherThreadsSendMsg;
///  Event
	HANDLE m_hEvent;

public:
///  Constructor
	CIMMediator();
///  Destructor
	~CIMMediator();

private:

///  Adds message to internal map
///  @param peer
///  @param message text
///  @remarks
	void AddMsgToMap( const tstring&, const tstring& );

///  Extracts message from internal map by peer
///  @param peer
///  @remarks
	tstring GetMsgFromMapByPeerId( const tstring& );

///  Clears internal map
///  @remarks
	void ClearMap();

public:

BEGIN_COM_MAP(CIMMediator)
	COM_INTERFACE_ENTRY(IObjectSafety)
	COM_INTERFACE_ENTRY(IObjectSafetySiteLock)
END_COM_MAP( )

DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE | 
	OLEMISC_CANTLINKINSIDE | 
	OLEMISC_INSIDEOUT | 
	OLEMISC_ACTIVATEWHENVISIBLE | 
	OLEMISC_SETCLIENTSITEFIRST
)


BEGIN_PROP_MAP(CIMMediator)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()


BEGIN_MSG_MAP(CIMMediator)
	MESSAGE_HANDLER(m_msgFireEventOtherThreadsSendMsg,FireEventOtherThreadsSendMsg)
	CHAIN_MSG_MAP(CComControl<CIMMediator>)
	DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()

	LRESULT FireEventOtherThreadsSendMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	__event __interface _IIMMediatorEvents;
// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// IIMMediator
public:
		HRESULT OnDraw(ATL_DRAWINFO& di)
		{
		RECT& rc = *(RECT*)di.prcBounds;
		// Set Clip region to the rectangle specified by di.prcBounds
		HRGN hRgnOld = NULL;
		if (GetClipRgn(di.hdcDraw, hRgnOld) != 1)
			hRgnOld = NULL;
		bool bSelectOldRgn = false;

		HRGN hRgnNew = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);

		if (hRgnNew != NULL)
		{
			bSelectOldRgn = (SelectClipRgn(di.hdcDraw, hRgnNew) != ERROR);
		}
		// region leack in ATL realization
		if(hRgnNew!=NULL)
			DeleteObject(hRgnNew);

		Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
		SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
		LPCTSTR pszText = _T("ATL 7.0 : IMMediator");
		TextOut(di.hdcDraw, 
			(rc.left + rc.right) / 2, 
			(rc.top + rc.bottom) / 2, 
			pszText, 
			lstrlen(pszText));

		if (bSelectOldRgn)
			SelectClipRgn(di.hdcDraw, hRgnOld);

		return S_OK;
	}


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}
	///  Adds message to internal map
	STDMETHOD(HandleMsgInternal)(BSTR peerId, BSTR msg);
	///  Raises OnSendMessage event
	STDMETHOD(SendMsg)(BSTR peerId, BSTR msg);
	///  Returns message for peer
	STDMETHOD(HandleMsg)(BSTR peerId, BSTR* msg);
	///  Resets event, clears internal map
	STDMETHOD(ResetMap)();
	///  Returns handle of event
	STDMETHOD(GetEvent)(/*HANDLE*/LONGLONG* hEvent);
};

