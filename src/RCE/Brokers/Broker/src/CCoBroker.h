/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCoBroker.h
///
///  CCoBroker ActiveX object declaration 
///
///  @author Kirill Solovyov @date 11.10.2007
///
////////////////////////////////////////////////////////////////////////

// CCoBroker.h : Declaration of the CCoBroker

#pragma once
#include "resource.h"       // main symbols
#include <AidLib/logging/clog.h>
#include "..\..\Shared\CRequestsMgr.h"

#include <HelperService/CSrvComm.h>
#include <boost/shared_ptr.hpp>
#include <atlfile.h>
//TODO this to all object
//#include <AidLib/Logging/CInstanceTracker.h>
//CInstanceTracker m_moduleInsanceTracker(_T("VBroker module"));

//#include "..\..\BrokerProxy\src\BPInstaller\ICoBPInstaller.h"
#include "CInstaller.h"

//class CCoBroker;
//
////installer events receiver
//[
//	coclass,
//	noncreatable,
//	uuid("07E028F1-8059-4e9d-9E07-200AF2511378"),
//	event_receiver(com,true)
//]
//class C_ICoBPInstallerEvents:
//	public _ICoBPInstallerEvents
//{
//protected:
//	// rcinstaller have neutral thread model, if it will have other model use CComGITPtr
//	CComGITPtr<IUnknown> m_unkn;
//public:
//	C_ICoBPInstallerEvents();
//	~C_ICoBPInstallerEvents();
//	HRESULT EventAdvise(IUnknown* unkn);
//	HRESULT EventUnadvise(void);
//	CCoBroker* m_owner;
//
//	// _C_ICoBPInstallerEvents Methods
//public:
//	/// The event notifies that feature installation has completed.
//	/// @param result Equal 0 if feature installation is success, nonzero value otherwise.
//	STDMETHOD(NotifyFeatureInstalled)(LONG result);
//	
//	/// The event notifies progress of installing feature
//	/// @param percentCompleted percent completed of installing process
//	/// @param status Status is a text message that describes the current step.
//	STDMETHOD(NotifyInstalling)(LONG percentCompleted,BSTR status);
//};
//

// Broker logging registrar
class CBrokerLogReg
{
public: 
	CBrokerLogReg();
	virtual ~CBrokerLogReg();
};


/// Broker start types enum
enum EBrokerStartTypes
{
	/// start on same process by defult
	BST_SAME=0,
	/// start in new IE process
	BST_IE,
	/// start in new native (SupportSpace_tools.exe) process
	BST_NATIVE,
	/// start in new instance of browser
	BST_MAXELEMENT
	//BST_NEW,//not implemented now
	/// maximum element of the enum

};


// SParentProcParams structer of parent process parameters, using by Broker new process creation
struct SParentProcParams
{
	/// parent process id (BrokerProxy process)
	DWORD m_id;
	/// child process id (BrokerProxy process)
	DWORD m_childId;
	/// notify window of parent process
	HWND m_wnd;
};




#ifdef _WIN32_WCE
#error "Neutral-threaded COM objects are not supported on Windows CE."
#endif

// ICoBroker
[
	object,
	uuid("98CFDD1C-F190-4225-9CE2-5AF4A49ADBD5"),
	dual,	helpstring("ICoBroker Interface"),
	pointer_default(unique)
]
__interface ICoBroker : IDispatch
{
	[id(1), helpstring("method Init")] HRESULT Init([in] BSTR msiPath, [in] BSTR version, [in] BSTR productCode);
	[id(2), helpstring("method InitSession")] HRESULT InitSession([in] BSTR relaySrv, [in] BSTR sId, [in] BSTR userId, [in] BSTR passwd, [in] BSTR remoteUserId);
	[id(3), helpstring("method HandleRequest")] HRESULT HandleRequest([in] BSTR dstUserId, [in] ULONG dstSvcId, [in] BSTR srcUserId, [in] ULONG srcSvcId, [in] ULONG rId, [in] ULONG rType, [in] ULONG param, [in] BSTR params);
};


// _ICoBrokerEvents
[
	dispinterface,
	uuid("213FABA5-C360-476E-93B8-8286045820AE"),
	helpstring("_ICoBrokerEvents Interface")
]
__interface _ICoBrokerEvents
{
	/// The event notifies that log message become.
	/// @param message is a text log message
	/// @param severity type of log message _EXCEPTION=0,_ERROR,_WARNING,_MESSAGE,_SUCCESS
	[id(1), helpstring("method NotifyLogMessage")] HRESULT NotifyLogMessage([in] BSTR message, [in] LONG severity);
	[id(2), helpstring("method RequestSent")] HRESULT RequestSent([in] BSTR dstUserId, [in] ULONG dstSvcId, [in] BSTR srcUserId, [in] ULONG srcSvcId, [in] ULONG rId, [in] ULONG rType, [in] ULONG param, [in] BSTR params);
};


// CCoBroker

[
	coclass,
	default(ICoBroker, _ICoBrokerEvents),
	//threading(neutral),
	//threading(apartment),
	threading(single),
	event_source(com),
	vi_progid("Broker.CoBroker"),
	progid("Broker.CoBroker.1"),
	version(1.0),
	uuid("2FF5923D-5B0C-4EAB-8CF7-7CC79F1A627E"),
	helpstring("CoBroker Class")
]
class ATL_NO_VTABLE CCoBroker :
	public ICoBroker,
	public cLog,
	public CWindowImpl<CCoBroker>,
	public CRequestsMgr,
	public CThread,
	public CInstaller
{
	friend class C_ICoBPInstallerEvents;
private:
	/// Communicator with service
	boost::shared_ptr<CSrvSTDQueueComm> m_communicator;
public:
	
	/// ActiveX friendly name, use by registration
	static const TCHAR* GetObjectFriendlyName() 
	{
		return _T("Support Platform Broker");
	}
	
	CCoBroker();
	~CCoBroker();
	__event __interface _ICoBrokerEvents;

	BEGIN_MSG_MAP( CCoBroker )
		MESSAGE_HANDLER(m_msgFireNotifyLogMessage,FireNotifyLogMessage)
		MESSAGE_HANDLER(m_msgChildBrokerCreated,OnChildBrokerCreated)
	END_MSG_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	///  Singleton object
	//DECLARE_CLASSFACTORY_SINGLETON(CCoBroker)

	HRESULT FinalConstruct();
	void FinalRelease();

	STDMETHOD(Advise)(IUnknown* pUnkSink,DWORD* pdwCookie);
	STDMETHOD(Unadvise)(DWORD dwCookie);

protected:
	/// Broker's parameters
	struct
	{
		/// msi package relative path
		tstring m_msiPath;
		/// available version
		tstring m_version;
		/// current product code of customer package
		tstring m_productCode;
	} m_brokerParams;

	/// BRT_SERVICE_Handler protection (installation)
	CCriticalSection m_csBRT_SERVICE_Handler;

	/// installer
	//CComGITPtr<ICoBPInstaller> m_installer;
	//DWORD m_installerEventCookie;
	
	/// installer events receiver object
	//CComPtr<CComObject<C_ICoBPInstallerEvents> > m_installerEvents;
	
	/// request which follow to installation
	SRequest m_installerRequest;
	/// installer action ADDLOCAL or REINSTALL
	tstring m_installerAction;
	/// request identifier for progress of installation
	unsigned long m_installerRId;

	//DWORD m_installerRef;

	/// A virtual method that prepare message string and fire event+ NotifyLogMessage
	virtual void AddList(const cEventDesc &EventDesc, const TCHAR *Item, ...)throw( );
	virtual void AddString(const TCHAR* LogString, const eSeverity Severity)throw(){}
	/// message code of event fire in other thread
	const UINT m_msgFireNotifyLogMessage;
	/// message handler. function fire NotifyLogMessage event 
	LRESULT FireNotifyLogMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		// The method overload global the same name function for multy thread event call. It is called by event firing function which generate by attributed ATL
	inline HRESULT __ComInvokeEventHandler(IDispatch* pDispatch, DISPID id, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult);

	/// The method handles service requests BRT_SERVICE
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionsMgr.h file).
	virtual void BRT_SERVICE_Handler(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params);

	/// The method create service and send respond on service request (BRT_SERVICE)
	void innerCreateToolService(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params);
	
	/// The entery point of service installation and creation thread
	void Execute(void *Params);

	virtual void SendRequestJS(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params);

	/// loggin registrar (only one loggin registration per process)
	static CBrokerLogReg m_logReg;

	/// owner BrokerProxy parameters
	struct 
	{
		/// process id
		DWORD m_pId;
		/// creation thread id
		DWORD m_tId;
		/// ActiveX or Message window handle
		HWND m_wnd;
	} m_brokerProxyParams;
	
	/// message code of child process Broker creation notification
	const UINT m_msgChildBrokerCreated;
	/// m_msgChildBrokerCreated message handler
	LRESULT OnChildBrokerCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/// The method hadled js service request (initial), start new process of IE and others. see EBrokerStartTypes for possible cases
	/// @return true if reqeust handled and not need to pass to next handler or false if it was not handled and must be pass to next handler
	bool HandleRequestService(const tstring& dstUserId, ULONG dstSvcId, const tstring& srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, const tstring& params);

	/// The method use for changing CWindowImplBaseT::WindowProc() to CCoBroker::safeWindowProc()
	virtual WNDPROC GetWindowProc()
	{
		return safeWindowProc;
	}
	
	/// The method lock COM object via AddRef() and pass hanling to CWindowImpl<CCoBroker>::GetWindowProc(). For prevent destroy Broker during message handling.
	static LRESULT CALLBACK safeWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
	
	/// The event notifies that installation has completed.
	/// @param result Equal 0 if feature installation is success, nonzero value otherwise.
	virtual void OnInstalled(LONG result);

	/// The event notifies progress of installation
	/// @param percentCompleted percent completed of installing process
	/// @param status Status is a text message that describes the current step.
	virtual void OnInstalling(LONG percentCompleted, const tstring& status);

};

