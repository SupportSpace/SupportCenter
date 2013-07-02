/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCoVBrokerProxy.h
///
///  CCoVBrokerProxy ActiveX object declaration 
///
///  @author Kirill Solovyov @date 10.10.2007
///
////////////////////////////////////////////////////////////////////////
// CCoBrokerProxy.h : Declaration of the CCoBrokerProxy
#pragma once
#include "resource.h"       // main symbols
#include <atlsync.h>
//#include <atlctl.h>
#include <RCEngine/sitelock/RCSiteLockImpl.h>
#include <queue>

class CCoBrokerProxy;
#include "C_ICoBrokerEvents.h"
#include "..\..\Shared\SRequest.h"


//#include "BPLightInstaller/CCoBPLightInstaller.h"
#include <AidLib/Logging/CInstanceTracker.h>
#include <AidLib/CThread/CThread.h>
#include <atlsync.h>
//#include <atlfile.h>

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif


// ICoBrokerProxy
[
	object,
	uuid(51035546-F6BB-4E5E-8434-2AC02F44BE8F),
	dual,
	helpstring("ICoBrokerProxy Interface"),
	pointer_default(unique)
]
__interface ICoBrokerProxy : public IDispatch
{
	[id(1), helpstring("method Init")] HRESULT Init([in] BSTR msiPath, [in] BSTR version, [in] BSTR productCode);
	[id(2), helpstring("method InitSession")] HRESULT InitSession([in] BSTR relaySrv, [in] BSTR sId, [in] BSTR userId, [in] BSTR passwd, [in] BSTR remoteUserId);
	[id(3), helpstring("method HandleRequest")] HRESULT HandleRequest([in] BSTR dstUserId, [in] ULONG dstSvcId, [in] BSTR srcUserId, [in] ULONG srcSvcId, [in] ULONG rId, [in] ULONG rType, [in] ULONG param, [in] BSTR params);
};


// _ICoBrokerProxyEvents
[
	uuid("83D9D9B4-C6CC-4574-A7D6-F5490AA25E81"),
	dispinterface,
	helpstring("_ICoBrokerProxyEvents Interface")
]
__interface _ICoBrokerProxyEvents
{
	/// The event notifies that log message become.
	/// @param message is a text log message
	/// @param severity type of log message _EXCEPTION=0,_ERROR,_WARNING,_MESSAGE,_SUCCESS
	[id(1), helpstring("method NotifyLogMessage")] HRESULT NotifyLogMessage([in] BSTR message, [in] LONG severity);
	[id(2), helpstring("method RequestSent")] HRESULT RequestSent([in] BSTR dstUserId, [in] ULONG dstSvcId, [in] BSTR srcUserId, [in] ULONG srcSvcId, [in] ULONG rId, [in] ULONG rType, [in] ULONG param, [in] BSTR params);
};

// CCoBrokerProxy
[
	coclass,
	control,
	default(ICoBrokerProxy, _ICoBrokerProxyEvents),
	threading(apartment),
	vi_progid("BrokerProxy.CoBrokerProxy"),
	progid("BrokerProxy.CoBrokerProxy.1"),
	version(1.0),
	//uuid("D96BEC5D-3046-48D8-9FA4-24F9CDD8EFE3"),
	//uuid("08653405-44A9-4E99-9C09-DD00770AAA08"),
	uuid("28FFA157-376A-49d8-A2EE-50D989A736DD"),
	helpstring("CoBrokerProxy Class"),
	event_source(com),
	support_error_info(ICoBrokerProxy),
	registration_script("control.rgs")
]
class ATL_NO_VTABLE CCoBrokerProxy :
	public cLog,
	public ICoBrokerProxy,
	public IPersistStreamInitImpl<CCoBrokerProxy>,
	public IOleControlImpl<CCoBrokerProxy>,
	public IOleObjectImpl<CCoBrokerProxy>,
	//public IObjectWithSiteImpl<CCoBrokerProxy>,//for sitelock for ActiveXObject() creation http://www.tech-archive.net/Archive/VC/microsoft.public.vc.atl/2007-10/msg00159.html
	public IOleInPlaceActiveObjectImpl<CCoBrokerProxy>,
	public IViewObjectExImpl<CCoBrokerProxy>,
	public IOleInPlaceObjectWindowlessImpl<CCoBrokerProxy>,
	public IPersistStorageImpl<CCoBrokerProxy>,
	public ISpecifyPropertyPagesImpl<CCoBrokerProxy>,
	public IQuickActivateImpl<CCoBrokerProxy>,
#ifndef _WIN32_WCE
	public IDataObjectImpl<CCoBrokerProxy>,
#endif
#ifdef _WIN32_WCE // IObjectSafety is required on Windows CE for the control to be loaded correctly
	public IObjectSafetyImpl<CCoBrokerProxy, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
#endif
	public CComControl<CCoBrokerProxy>,
	public CRCSiteLockImpl<CCoBrokerProxy>,
	public CInstanceTracker,
	protected CThread
{
public:
	CCoBrokerProxy();
	~CCoBrokerProxy();
DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_INSIDEOUT |
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST)

BEGIN_PROP_MAP(CCoBrokerProxy)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_MSG_MAP(CCoBrokerProxy)
	MESSAGE_HANDLER(m_msgFireNotifyLogMessage,FireNotifyLogMessage)
	MESSAGE_HANDLER(m_msgFireRequestSent,FireRequestSent)
	MESSAGE_HANDLER(m_msgCreateBroker,CreateBroker)
	CHAIN_MSG_MAP(CComControl<CCoBrokerProxy>)
	DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	__event __interface _ICoBrokerProxyEvents;
// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// ICoBrokerProxy
public:
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

	/// ActiveX friendly name, use by registration
	static const TCHAR* GetObjectFriendlyName() 
	{
		return _T("Support Platform Strapper");
	}
	// The method overload global the same name function for multy thread event call. It is called by event firing function which generate by attributed ATL
	inline HRESULT __ComInvokeEventHandler(IDispatch* pDispatch, DISPID id, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult);
	STDMETHOD(Advise)(IUnknown* pUnkSink,DWORD* pdwCookie);
	STDMETHOD(Unadvise)(DWORD dwCookie);

protected:
		/// A virtual method that prepare message string and fire event+ NotifyLogMessage
	virtual void AddList(const cEventDesc &EventDesc, const TCHAR *Item, ...)throw( );
	virtual void AddString(const TCHAR* LogString, const eSeverity Severity)throw(){}
	/// message code of event fire in other thread
	const UINT m_msgFireNotifyLogMessage;
	/// message handler. function fire NotifyLogMessage event 
	LRESULT FireNotifyLogMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	/// message code of event fire in other thread
	const UINT m_msgFireRequestSent;
	/// message handler. function fire NotifyLogMessage event 
	LRESULT FireRequestSent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	/// message code of Broker creation after installation
	const UINT m_msgCreateBroker;
	/// message handler. function fire Broker creation
	LRESULT CreateBroker(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	// Broker's parameters
	struct
	{
		CComBSTR m_msiPath;     /// relative path and name of installation package file
		CComBSTR m_version;     /// version available version of tools
		CComBSTR m_productCode; /// current poduct code
	} m_brokerParams;

	/// session's parameters
	struct CSessionParams
	{
		CComBSTR m_relaySrv;      /// relaySrv relay server address
		CComBSTR m_sId;          /// unique session identifier
		CComBSTR m_userId;       /// unique user identifier
		CComBSTR m_passwd;       /// password for user identifier
		CComBSTR m_remoteUserId; /// remote user identifier

		CSessionParams(const CComBSTR& relaySrv, const CComBSTR& sId, const CComBSTR& userId, const CComBSTR& passwd, const CComBSTR& remoteUserId):
			m_relaySrv(relaySrv),
			m_sId(sId),
			m_userId(userId),
			m_passwd(passwd),
			m_remoteUserId(remoteUserId)
		{}

		CSessionParams(const CSessionParams& params)
		{
			m_relaySrv=params.m_relaySrv;
			m_sId=params.m_sId;
			m_userId=params.m_userId;
			m_passwd=params.m_passwd;
			m_remoteUserId=params.m_remoteUserId;
		}
		
		CSessionParams& operator=(const CSessionParams& params)
		{
			m_relaySrv=params.m_relaySrv;
			m_sId=params.m_sId;
			m_userId=params.m_userId;
			m_passwd=params.m_passwd;
			m_remoteUserId=params.m_remoteUserId;
			return *this;
		}
	};

	/// map of session parameters. use when broker has not yet been instantiated
	std::queue<CSessionParams> m_sessionsParams;

	/// out-proc singleton broker instance 
	CComPtr<IDispatch> m_broker;
	
	// Broker events receiver (_ICoBrokerEvents)
	CComPtr<CComObject<C_ICoBrokerEvents> > m_brokerEvents;

	/// installer object
	//CComPtr<ICoBPLightInstaller> m_installer;

	/// request identifier for progress of installation
	unsigned long m_installerRId;
	/// installation in progress
	bool m_installing;

	/// The mothod install or update Broker. 
	void InstallBroker(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params);

	//
	//CMutex m_firstInstance;
	
	/// The method retrieve creating page URL
	/// @return url string in case successful, otherwise throw std::exception
	tstring GetPageUrl();

	/// The child process parameters file map object
	//CAtlFileMapping<SParentProcParams> m_newProc;
	
	/// message window create for no window ActiveX case
	CWindow m_wnd;
	
	/// message window window procedure
	static LRESULT CALLBACK WndWindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

	/// message window old window procedure
	WNDPROC oldWndWindowProc;

	/// overloaded ATL method call ActiveX's PostMessage or m_wnd PostMessage if ActiveX's window not exist
	BOOL PostMessage(UINT message,WPARAM wParam = 0,LPARAM lParam = 0 ) throw();


public:
	/// The method initialize parameters of BrokerProxy
	/// @param msiPath relative path and name of installation package file
	/// @param version available version of tools
	/// @param current poduct code
	STDMETHOD(Init)(BSTR msiPath, BSTR version, BSTR productCode);
	
	/// The method initialize new session
	/// @param relaySrv relay server address
	/// @param sId unique session identifier
	/// @param userId user identifier
	/// @param passwd password for user identifier
	/// @param remoteUserId remote user identifier
	STDMETHOD(InitSession)(BSTR relaySrv, BSTR sId, BSTR userId, BSTR passwd, BSTR remoteUserId);


	/// The method handle request
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionsMgr.h file).
	STDMETHOD(HandleRequest)(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params);

	/// requests which sent when broker was not created
	std::queue<SRequest> m_requests;
	CCriticalSection m_csRequests;

	/// The method create Broker object
	void CreateBroker();

	/// The entery point of service installation and creation thread
	void Execute(void *Params);
	
	/// fire RequestSent event through post message
	void FireRequestSent(BSTR dstUserId,ULONG dstSvcId,BSTR srcUserId,ULONG srcSvcId,ULONG rId,ULONG rType,ULONG param,BSTR params);


};
