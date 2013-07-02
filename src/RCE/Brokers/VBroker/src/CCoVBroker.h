/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCoVBroker.h
///
///  CCoVBroker ActiveX object declaration 
///
///  @author Kirill Solovyov @date 04.10.2007
///
////////////////////////////////////////////////////////////////////////
// CCoVBroker.h : Declaration of the CCoVBroker
#pragma once

#include "resource.h"       // main symbols
#include <atlctl.h>
//#define BOOST_THREAD_NO_LIB
#include <boost/shared_ptr.hpp>
//#include <boost/thread.hpp>
//#include <boost/bind.hpp>
//#include <RCEngine/RCEngine.h>
#include <RCEngine/sitelock/RCSiteLockImpl.h>
#include <AidLib/CException/CException.h>


#include "..\..\Shared\BrokersTypes.h"
#include "..\..\Shared\CRequestsMgr.h"


#include <HelperService/CSrvComm.h>

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif


// ICoVBroker
[
	object,
	uuid(0442A1C0-96C3-461C-A5EA-2D0FDE8B8FA0),
	dual,
	helpstring("ICoVBroker Interface"),
	pointer_default(unique)
]
__interface ICoVBroker : public IDispatch
{
	[id(1), helpstring("method StartToolService")] HRESULT StartToolService([in] BSTR relaySrv, [in] BSTR sId, [in] BSTR userId, [in] BSTR passwd, [in] BSTR remoteUserId, [in] ULONG ftype, [in] IDispatch *host, [out,retval] ULONG* ret_svcId);
	[id(2), helpstring("method StopToolService")] HRESULT StopToolService([in] ULONG svcId);
	[id(3), helpstring("method HandleRequest")] HRESULT HandleRequest([in] BSTR dstUserId, [in] ULONG dstSvcId, [in] BSTR srcUserId, [in] ULONG srcSvcId, [in] ULONG rId, [in] ULONG rType, [in] ULONG param, [in] BSTR params);
	//[id(3), helpstring("method Send")] HRESULT Send([in] BSTR sId, [in] ULONG svcId, [in] ULONG rId, [in] ULONG rType, [in] ULONG param, [in] BSTR params);
	[id(4), helpstring("method GetAvailableServices")] HRESULT GetAvailableServices([out,retval,satype(VARIANT)] SAFEARRAY **params);
};


// _ICoVBrokerEvents
[
	uuid("D2716FA1-2C28-4AF3-9E15-EA53216EAB2B"),
	dispinterface,
	helpstring("_ICoVBrokerEvents Interface")
]
__interface _ICoVBrokerEvents
{
	/// The event notifies that log message become.
	/// @param message is a text log message
	/// @param severity type of log message _EXCEPTION=0,_ERROR,_WARNING,_MESSAGE,_SUCCESS
	[id(1), helpstring("method NotifyLogMessage")] HRESULT NotifyLogMessage([in] BSTR message, [in] LONG severity);
	/// The event notifies that request sent to JS
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionsMgr.h file).
	[id(2), helpstring("method RequestSent")] HRESULT RequestSent([in] BSTR dstUserId, [in] ULONG dstSvcId, [in] BSTR srcUserId, [in] ULONG srcSvcId, [in] ULONG rId, [in] ULONG rType, [in] ULONG param, [in] BSTR params);
	/// Reserved. The event notifies that request received from remote side through c++ connect (no jabber)
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionsMgr.h file).
	//[id(3), helpstring("method Received")] HRESULT Received([in] BSTR sId, [in] ULONG svcId, [in] ULONG rId, [in] ULONG rType, [in] ULONG param, [in] BSTR params);
};

//// IBrokerClient1
//[
//	export,
//	uuid("D8B1B541-E523-424E-BA9B-5078FFDC2741"),
//	dual,
//	oleautomation,
//	helpstring("IBrokerClient Interface"),
//	pointer_default(unique)
//]
//__interface IBrokerClient1 : public IDispatch
////__interface IBrokerClient1 : public IUnknown
//{
//	//[id(1), helpstring("method Init")] HRESULT Init([in] IUnknown* events);
//	[id(1), helpstring("method Init")] HRESULT Init([in] ULONG events);
//	[id(2), helpstring("method HandleRequest")] HRESULT HandleRequest([in] BSTR dstUserId, [in] ULONG dstSvcId, [in] BSTR srcUserId, [in] ULONG srcSvcId, [in] ULONG rId, [in] ULONG rType, [in] ULONG param, [in] BSTR params);
//
//	/// The method set sub stream to the service. The method is answer on call of GetSubStream() method of IBrokerClient
//	/// @param dstUserId unique destination user identifier
//	/// @param dstSvcId unique per UserId destination service identifier
//	/// @param streamId unique for this service stream identifier. it is not real sub stream identifier of Stream Multimplexer
//	/// @param pointer_shared_ptr_stream raw pointer on shared_ptr object which is pointer on substream. Service must assign with refrence count increase (boost::shared_ptr<CAbstractStream> m_stream;\n m_stream=*reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream);
//	[id(3), helpstring("method SetSubStream")] HRESULT SetSubStream([in] BSTR dstUserId, [in] ULONG dstSvcId, [in] ULONG streamId, [in] ULONG pointer_shared_ptr_stream);
//};

// CCoVBroker
[
	coclass,
	control,
	default(ICoVBroker, _ICoVBrokerEvents),
	threading(apartment),
	vi_progid("VBroker.CoVBroker"),
	progid("VBroker.CoVBroker.1"),
	version(1.0),
	uuid("ED2B90B2-9124-4D63-8FEB-E9203FCA1BCE"),
	helpstring("CoVBroker Class"),
	event_source(com),
	support_error_info(ICoVBroker),
	registration_script("control.rgs")
]
class ATL_NO_VTABLE CCoVBroker :
	public cLog,
	public CRequestsMgr,
	public ICoVBroker,
	public IPersistStreamInitImpl<CCoVBroker>,
	public IOleControlImpl<CCoVBroker>,
	public IOleObjectImpl<CCoVBroker>,
	public IOleInPlaceActiveObjectImpl<CCoVBroker>,
	public IViewObjectExImpl<CCoVBroker>,
	public IOleInPlaceObjectWindowlessImpl<CCoVBroker>,
	public IPersistStorageImpl<CCoVBroker>,
	public ISpecifyPropertyPagesImpl<CCoVBroker>,
	public IQuickActivateImpl<CCoVBroker>,
#ifndef _WIN32_WCE
	public IDataObjectImpl<CCoVBroker>,
#endif
#ifdef _WIN32_WCE // IObjectSafety is required on Windows CE for the control to be loaded correctly
	public IObjectSafetyImpl<CCoVBroker, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
#endif
	public CComControl<CCoVBroker>,
	//,public IConnectionPointImpl<CCoVBroker, &__uuidof(::_ICoVBrokerEvents), CComDynamicUnkArray>
	public CRCSiteLockImpl<CCoVBroker>
{
private:
	/// Communicator with service
	boost::shared_ptr<CSrvSTDQueueComm> m_communicator;
public:
	CCoVBroker();
	~CCoVBroker();
	DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE |
	OLEMISC_CANTLINKINSIDE |
		OLEMISC_INSIDEOUT |
		OLEMISC_ACTIVATEWHENVISIBLE |
		OLEMISC_SETCLIENTSITEFIRST
		)

		BEGIN_PROP_MAP(CCoVBroker)
			PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
			PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
			// Example entries
			// PROP_ENTRY("Property Description", dispid, clsid)
			// PROP_PAGE(CLSID_StockColorPage)
		END_PROP_MAP()

		BEGIN_MSG_MAP(CCoVBroker)
			TRY_CATCH
//			MESSAGE_HANDLER(WM_SIZE, OnSize)
			MESSAGE_HANDLER(WM_CREATE, OnCreateVBrokerWindow)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroyVBrokerWindow)
			MESSAGE_HANDLER(m_msgFireNotifyLogMessage,FireNotifyLogMessage)
			CHAIN_MSG_MAP(CComControl<CCoVBroker>)
			DEFAULT_REFLECTION_HANDLER()
			CATCH_LOG()
		END_MSG_MAP()
		
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

		__event __interface _ICoVBrokerEvents;

	// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)
	DECLARE_PROTECT_FINAL_CONSTRUCT()
	
	/// ActiveX friendly name, use by registration
	static const TCHAR* GetObjectFriendlyName() 
	{
		return _T("Support Platform VBroker");
	}

protected:
	// The method overload global the same name function for multy thread event call. It is called by event firing function which generate by attributed ATL
	inline HRESULT __ComInvokeEventHandler(IDispatch* pDispatch, DISPID id, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult);

	/// A virtual method that prepare message string and fire event+ NotifyLogMessage
	virtual void AddList(const cEventDesc &EventDesc, const TCHAR *Item, ...)throw( );
	virtual void AddString(const TCHAR* LogString, const eSeverity Severity)throw(){}
	/// message code of event fire in other thread
	const UINT m_msgFireNotifyLogMessage;
	/// message handler. function fire NotifyLogMessage event 
	LRESULT FireNotifyLogMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	/// send request through JS
	virtual void SendRequestJS(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params);

public:
	HRESULT FinalConstruct();
	void FinalRelease();
	STDMETHOD(Advise)(IUnknown* pUnkSink,DWORD* pdwCookie);
	STDMETHOD(Unadvise)(DWORD dwCookie);

public:
	/// The method creates service ActiveX COM object which host on DHTML object.
	/// @param sId session identifier
	/// @param svcId service identifier
	/// @param svcType service type
	/// @host DHTML host object
	/// @return [retval] return service identifier
	STDMETHOD(StartToolService)(BSTR relaySrv, BSTR sId, BSTR userId, BSTR passwd, BSTR remoteUserId, ULONG svcType, IDispatch *host, ULONG* ret_svcId);

	/// The method stop service
	/// @param sId session identifier
	/// @param svcId service identifier returned previous call of StartService() method
	STDMETHOD(StopToolService)(ULONG svcId);

	/// The method handle request
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionsMgr.h file).
	/// @param destination destination of request. see EBrokerJSDestination for detail
	STDMETHOD(HandleRequest)(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params);
	
	/// The method return list of available services
	/// @param params - list of available services
	STDMETHOD(GetAvailableServices)(SAFEARRAY **params);

private:
	// An original browser window procedure
	static WNDPROC m_parentWndProc;
	
	// The browser window procedure for subclassing
	static LRESULT CALLBACK BrowserWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Used for subclassing a browser window, notify then VBroker window is created
	LRESULT OnCreateVBrokerWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	// Used for unsubclassing a browser window, notify then VBroker window is destroyed
	LRESULT OnDestroyVBrokerWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

