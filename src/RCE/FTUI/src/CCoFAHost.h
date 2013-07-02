// CoFAHost.h : Declaration of the CCoFAHost

#pragma once
#include "resource.h"       // main symbols

#include <FTUI/FileTransfer/CFileAccessServer.h>
#include <boost/shared_ptr.hpp>

#include <NWL/Streaming/CSocketSystem.h>
#include <NWL/TLS/CTLSSystem.h>
#include <RCEngine/sitelock/RCSiteLockImpl.h>
#include <RCEngine/SafeEvents/CSafeEvents.h>
#include <AidLib/Logging/CInstanceTracker.h>
#include <AidLib/CThread/CThread.h>
#include "..\..\Brokers\Shared\IBrokerClient.h"
#include "..\..\Brokers\Shared\CAvailableServices.h"
#include "..\..\Brokers\Shared\BrokersTypes.h"

// _IFileAccsesHostEvents
[
	uuid("814C9237-35BB-462c-9B58-77BD541412E3"),
	dispinterface,
	helpstring("_IFileAccsesHostEvents Interface")
]

///  _IFileAccsesHostEvents events
__interface _IFileAccsesHostEvents
{
};

// ICoFAHost
[
	object,
	uuid("BE949E59-21FC-48F3-9456-0942957A0716"),
	dual,	helpstring("ICoFAHost Interface"),
	pointer_default(unique)
]
__interface ICoFAHost
{
};


// _ICoFAHostEvents
[
	dispinterface,
	uuid("A714EB2F-560A-49D7-BBF4-8E12755570AB"),
	helpstring("_ICoFAHostEvents Interface")
]
__interface _ICoFAHostEvents
{
};


[
	coclass,
	//threading(apartment),
	threading(neutral),
	support_error_info("ICoFAHost"),
	vi_progid("FTUI.ICoFAHost"),
	progid("FTUI.ICoFAHost.1"),
	version(1.0),
	event_source(com),
	uuid("BFC3A266-A3A0-4F25-901C-730CF6DC3554"),
	helpstring("CoFAHost Class"),
	registration_script("control.rgs")
]
///ActiveX  wrapper for File Access Server
///@remark
///threading("apartment")\n
///support_error_info("ICoFAHost")\n
///vi_progid("FTUI.ICoFAHost")\n
///progid("FTUI.ICoFAHost.1")\n
///version(1.0)\n
///event_source("com")\n
///uuid("BFC3A266-A3A0-4F25-901C-730CF6DC3554")\n
///helpstring("CoFAHost Class")\n
///registration_script("control.rgs")\n
class ATL_NO_VTABLE CCoFAHost :
	protected CThread,
	public IBrokerClient,
	public CInstanceTracker,
	public IObjectWithSiteImpl<CCoFAHost>,
	public IPersistStreamInitImpl<CCoFAHost>,
	public IOleControlImpl<CCoFAHost>,
	public IOleObjectImpl<CCoFAHost>,
	public IOleInPlaceActiveObjectImpl<CCoFAHost>,
	public IViewObjectExImpl<CCoFAHost>,
	public IOleInPlaceObjectWindowlessImpl<CCoFAHost>,
	public IPersistStorageImpl<CCoFAHost>,
	public ISpecifyPropertyPagesImpl<CCoFAHost>,
	public IQuickActivateImpl<CCoFAHost>,
	public IDataObjectImpl<CCoFAHost>,
	public CComCompositeControl<CCoFAHost>,
	public IDispatchImpl<ICoFAHost, &__uuidof(ICoFAHost), &LIB_GUID>,
	public CRCSiteLockImpl<CCoFAHost>,
	public CSafeEvents<CCoFAHost,_IFileAccsesHostEvents>
{
private:
	/// pointer to the CFileAccessServer class
	boost::shared_ptr<CFileAccessServer> m_server; 
	/// pointer to the network stream
	boost::shared_ptr<CAbstractStream> m_stream;
	/// local store for permission
	unsigned char m_permission[8];
public:
///DOXYS_OFF	

	/// This method is called when IE is closed
	HRESULT IOleObject_Close(DWORD dwSaveOption);
	void OnDisconnect( void* );


	HRESULT OnDraw(ATL_DRAWINFO& di)
	{
		RECT& rc = *(RECT*)di.prcBounds;
		// Set Clip region to the rectangle specified by di.prcBounds
		HRGN hRgnOld = NULL;
		if(GetClipRgn(di.hdcDraw,hRgnOld)!=1)hRgnOld=NULL;
		bool bSelectOldRgn=false;
		HRGN hRgnNew=CreateRectRgn(rc.left,rc.top,rc.right,rc.bottom);
		if (hRgnNew != NULL){ bSelectOldRgn = (SelectClipRgn(di.hdcDraw, hRgnNew) != ERROR);}
		Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
		SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
		LPCTSTR pszText = _T("ATL 7.0 : RCHostAXCtrl");
#ifdef _DEBUG			
		TextOut(di.hdcDraw,(rc.left + rc.right)/2,(rc.top+rc.bottom)/2,_T("CoFAHost"),_tcslen(_T("CoFAHost")));
#else
		TextOut(di.hdcDraw,(rc.left + rc.right)/2,(rc.top + rc.bottom)/2,pszText,lstrlen(pszText));
#endif
		if (bSelectOldRgn)SelectClipRgn(di.hdcDraw, hRgnOld);
		return S_OK;
	}
	

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	enum { IDD = IDD_PANE };

	DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE | 
	OLEMISC_CANTLINKINSIDE | 
		OLEMISC_INSIDEOUT | 
		OLEMISC_ACTIVATEWHENVISIBLE | 
		OLEMISC_SETCLIENTSITEFIRST
		)

	BEGIN_COM_MAP(CCoFAHost)
		COM_INTERFACE_ENTRY(IObjectSafety)
		COM_INTERFACE_ENTRY(IObjectSafetySiteLock)
		COM_INTERFACE_ENTRY(ICoFAHost)
		COM_INTERFACE_ENTRY(IBrokerClient)
	END_COM_MAP( )

	BEGIN_PROP_MAP(CCoFAHost)
		PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
		PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	END_PROP_MAP()


	BEGIN_MSG_MAP(CCoFAHost)
		CHAIN_MSG_MAP(CComCompositeControl<CCoFAHost>)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()


	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)
	DECLARE_PROTECT_FINAL_CONSTRUCT()
///DOXYS_ON
	
	/// ActiveX friendly name, use by registration
	static const TCHAR* GetObjectFriendlyName() 
	{
		return _T("Support Platform File Acces Host");
	}

	/// /ctor
	CCoFAHost();

	/// .dtor
	~CCoFAHost();

protected:
	///  ActiveX events
	__event __interface _IFileAccsesHostEvents;	

	/// Launches host object
	void LaunchHost();

	/// Infrastracture
	
	/// borker intercommunication
	CComGITPtr<_IBrokerClientEvents> m_brokerEvents;

	/// The method initialize viewer by _IBrokerClientEvents interface
	STDMETHOD(Init)(IUnknown *events);
	
	/// The method handle request sent by Broker
	STDMETHOD(HandleRequest)(BSTR dstUserId,ULONG dstSvcId,BSTR srcUserId,ULONG srcSvcId,ULONG rId,ULONG rType,ULONG param,BSTR params);

	/// The method call when asked sub stream connected
	STDMETHOD(SetSubStream)(BSTR dstUserId, ULONG dstSvcId, ULONG streamId, ULONG pointer_shared_ptr_stream);

	/// File server activity notification handler
	/// @param description activity description
	void OnActivityNotification(const tstring description);

	/// Activity notifications thread entry point
	virtual void Execute(void*);
};

