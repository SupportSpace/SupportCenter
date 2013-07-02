/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CScriptEngineClient.h
///
///  Declares CScriptEngineClient class, responsible for ScriptEngineClient ActiveX
///
///  @author Dmitry Netrebenko @date 15.11.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "resource.h"       // main symbols
#include <atlctl.h>

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif


#include <AidLib/Com/CSafeEventsRaiser.h>
#include <boost/shared_array.hpp>
#include <AidLib/Strings/tstring.h>
#include <NWL/Streaming/CAbstractStream.h>
#include <boost/shared_ptr.hpp>
#include "CProtocolMessageReceiver.h"
#include <map>
#include "SExecEnvironment.h"
#include <AidLib/CCritSection/CCritSectionObject.h>
#include <AidLib/Com/CInterfaceMarshaler.h>
#include "CScriptObjectWrapper.h"
#include "CCleaner.h"
#include "CUnZip32.h"
#include "CScriptDirectories.h"
#include "CEventThreadsManager.h"
#include "SScriptEngineRequest.h"
#include <AidLib/Com/CComVariantEx.h>

#include <AidLib/Logging/CInstanceTracker.h>
#include "..\..\Brokers\Shared\IBrokerClient.h"
#include "..\..\Brokers\Shared\CAvailableServices.h"
#include "..\..\Brokers\Shared\BrokersTypes.h"
#include "..\..\Brokers\Shared\CRIdGen.h"
#include <RCEngine/sitelock/RCSiteLockImpl.h>

#include "CNotificationThread.h"
#include <wtl/atlcrack.h>

#define FRAME_TAG					_T("<iframe id=\"%s\" width=\"100%%\" height=\"100%%\" src=\"file:///%s%s\"></iframe>")
#define DEFAULT_HTML_PAGE			_T("index.html")
#define ENGINE_VARIABLE				_T("ScriptEngineClient")
#define SCRIPT_FILES_MASK			_T("*.*")
#define SCRIPT_FILE_EXT				_T(".zip")
#define UNZIP_PASSWD				_T("ScriptEngine")
#define DEFAULT_SYNC_TIMEOUT		60000
#define EMPTY_ACTIVITY				_T("")
#define DEPLOY_ACTIVITY				_T("Deploying scripts...")
#define INVOKE_FILE_ACTIVITY		_T("Executing script...")
#define INVOKE_CODE_ACTIVITY		_T("Executing script...")
#define GET_FILES_ACTIVITY			_T("Transferring results to expert...")

/// GUID of library with web browser
struct __declspec(uuid("eab22ac0-30c1-11cf-a7eb-0000c05bae0b")) __WEBBROWSER_LIB;

/// OnDocumentComplete event Id of WebBrowser
#define WEBBROWSER_ONDOCUMENTCOMPLETE_ID 0x00000103

[export]
typedef enum _EScriptEngineError
{
	SE_ERROR_UNKNOWN		= 0, 
	SE_ERROR_STOP_SERVICE	= 1
} EScriptEngineError;

// IScriptEngineClient
[
	object,
	uuid(FEEE3154-35F9-4501-9DF1-922FC5D716AF),
	dual,
	helpstring("IScriptEngineClient Interface"),
	pointer_default(unique)
]
__interface IScriptEngineClient : public IDispatch
{
	[id(1), helpstring("method InitHost. Asynchronously initializes "
		"IScriptEngineHost on remote side.")]											HRESULT InitHost(void);
	[id(2), helpstring("method HostInited. Returns true if remote side "
		"is ready for invocations.")]													HRESULT HostInited([out,retval] LONG* inited);
	[id(3), helpstring("method Deploy. Asynchronously transfers all script files from "
		"predefined folder to remote (customer side).")]								HRESULT Deploy(void);
	[id(4), helpstring("method SetTimeOut. Set timeout for remote script execution.")]	HRESULT SetTimeOut([in] LONG timeOut);
	[id(5), helpstring("method InvokeFile. Asynchronously run procedure "
		"in remote script with specified set of parameters.")]							HRESULT InvokeFile([in] BSTR language, [in] BSTR fileName, [in] BSTR procedureName, [in] VARIANT param1, [in] VARIANT param2, [in] BSTR completionName);
	[id(6), helpstring("method InvokeCode. The same as previous, "
		"but code for procedure is taken from code parameter.")]						HRESULT InvokeCode([in] BSTR language, [in] BSTR code, [in] BSTR procedureName, [in] VARIANT param1, [in] VARIANT param2, [in] BSTR completionName);
	[id(7), helpstring("method GetRemoteFiles. Asynchronously transfers set of files "
		"from remote to local machine. files - array of file names. ")]					HRESULT GetRemoteFiles([in] VARIANT files);
	[id(8), helpstring("method WriteError. Adds error text to log.")]					HRESULT WriteLogError([in] BSTR errorText);
	[id(9), helpstring("method WriteWarning. Adds warning text to log.")]				HRESULT WriteLogWarning([in] BSTR warnText);
	[id(10), helpstring("method WriteMessage. Adds message text to log.")]				HRESULT WriteLogMessage([in] BSTR msgText);
	[id(11), helpstring("method InitHostSync. Synchronously initializes "
		"IScriptEngineHost on remote side.")]											HRESULT InitHostSync(void);
	[id(12), helpstring("method DeploySync. Synchronously transfers all script "
		"files from predefined folder to remote (customer side).")]						HRESULT DeploySync(void);
	[id(13), helpstring("method InvokeFileSync. Synchronously run procedure "
		"in remote script with specified set of parameters.")]							HRESULT InvokeFileSync([in] BSTR language, [in] BSTR fileName, [in] BSTR procedureName, [in] VARIANT param1, [in] VARIANT param2, [out,retval] VARIANT* result);
	[id(14), helpstring("method InvokeCodeSync. The same as previous, "
		"but code for procedure is taken from code parameter.")]						HRESULT InvokeCodeSync([in] BSTR language, [in] BSTR code, [in] BSTR procedureName, [in] VARIANT param1, [in] VARIANT param2, [out,retval] VARIANT* result);
	[id(15), helpstring("method GetRemoteFilesSync. Asynchronously transfers "
		"set of files from remote to local machine. files - array of file names.")]		HRESULT GetRemoteFilesSync(VARIANT files);
	[id(16), helpstring("method NotifyActivity. Sends activity string to host side")]	HRESULT NotifyActivity([in] BSTR activity);
	[id(17), helpstring("method BringOnTop. Brings script engine widget on top of other widgets")]	HRESULT BringOnTop();
};


// _IScriptEngineClientEvents
[
	uuid("985A6754-D4BE-43AA-AA21-398EAD41FA8B"),
	dispinterface,
	helpstring("_IScriptEngineClientEvents Interface")
]
__interface _IScriptEngineClientEvents
{
	[id(1), helpstring("method OnHostInited. Occurred, when remote part initialized "
		"successfully or remote initialization failed.")]								HRESULT OnHostInited([in] BOOL success, [in] BSTR errorString);
	[id(2), helpstring("method OnDeployComplete. Occurred, when deploy completed "
		"successfully or deploy failed.")]												HRESULT OnDeployComplete([in] BOOL success, [in] BSTR errorString);
	[id(3), helpstring("method OnFilesTransferred. Occurred, when remote files "
		"are transferred to local machine.")]											HRESULT OnFilesTransferred([in] BOOL success, [in] BSTR errorString);
	[id(4), helpstring("method OnProgress. Occurred, when state of some action "
		"is changed.")]																	HRESULT OnProgress([in] BSTR msg);
	[id(5), helpstring("method OnUnexpectedError. Occured, when unexpected error "
		"is raised.")]																	HRESULT OnUnexpectedError([in] EScriptEngineError errorCode, [in] BSTR errorMessage);
};

///  CScriptEngineClient class, responsible for ScriptEngineClient ActiveX
[
	coclass,
	control,
	default(IScriptEngineClient, _IScriptEngineClientEvents),
	threading(apartment),
	vi_progid("ScriptEngine.ScriptEngineClient"),
	progid("ScriptEngine.ScriptEngineClient.1"),
	version(1.0),
	uuid("D50A842C-B6C3-4912-8714-807F249B9519"),
	helpstring("ScriptEngineClient Class"),
	event_source(com),
	support_error_info(IScriptEngineClient),
	registration_script("control.rgs")
]
class ATL_NO_VTABLE CScriptEngineClient :
	public IScriptEngineClient,
	public IBrokerClient,
	public CInstanceTracker,
	public CSafeEventsRaiser<CScriptEngineClient,_IScriptEngineClientEvents>,
	public CProtocolMessageReceiver,
	public IPersistStreamInitImpl<CScriptEngineClient>,
	public IOleControlImpl<CScriptEngineClient>,
	public IOleObjectImpl<CScriptEngineClient>,
	public IOleInPlaceActiveObjectImpl<CScriptEngineClient>,
	public IViewObjectExImpl<CScriptEngineClient>,
	public IOleInPlaceObjectWindowlessImpl<CScriptEngineClient>,
	public IPersistStorageImpl<CScriptEngineClient>,
	public ISpecifyPropertyPagesImpl<CScriptEngineClient>,
	public IQuickActivateImpl<CScriptEngineClient>,
#ifndef _WIN32_WCE
	public IDataObjectImpl<CScriptEngineClient>,
#endif
#ifdef _WIN32_WCE // IObjectSafety is required on Windows CE for the control to be loaded correctly
	public IObjectSafetyImpl<CScriptEngineClient, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
#endif
	public CComControl<CScriptEngineClient>,
	public CRCSiteLockImpl<CScriptEngineClient>,
	public IDispEventImpl<0, CScriptEngineClient, &__uuidof(DWebBrowserEvents2), &__uuidof(__WEBBROWSER_LIB), 1, 0>,
	protected CRIdGen
{
public:
/// ActiveX friendly name, use by registration
	static const TCHAR* GetObjectFriendlyName() 
	{
		return _T("Support Platform Script Engine Client");
	}
/// Constructor
	CScriptEngineClient();
/// Destructor
	~CScriptEngineClient();
/// Enum for Ids of ScriptEngineClient events
	enum ESEClientEvent
	{
		seceCompletion			= 0xFF,	/// Special identificator of completion routine 
		seceHostInited			= 1,
		seceDeployComplete		= 2,
		seceFilesTransferred	= 3,
		seceProgress			= 4,
		seceUnexpectedError		= 5
	};

/// Enum for type of activity message
	enum ESEActivityType
	{
		seatEmpty				= 0,
		seatDeploy				= 1,
		seatInvokeFile			= 2,
		seatInvokeCode			= 3,
		seatGetRemoteFiles		= 4
	};

DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_INSIDEOUT |
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST
)


BEGIN_PROP_MAP(CScriptEngineClient)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()


BEGIN_MSG_MAP(CScriptEngineClient)
	CHAIN_MSG_MAP(CComControl<CScriptEngineClient>)
	DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	__event __interface _IScriptEngineClientEvents;
// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// IScriptEngineClient
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

		Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
		SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
		LPCTSTR pszText = _T("ATL 8.0 : ScriptEngineClient");
#ifndef _WIN32_WCE
		TextOut(di.hdcDraw,
			(rc.left + rc.right) / 2,
			(rc.top + rc.bottom) / 2,
			pszText,
			lstrlen(pszText));
#else
		ExtTextOut(di.hdcDraw,
			(rc.left + rc.right) / 2,
			(rc.top + rc.bottom) / 2,
			ETO_OPAQUE,
			NULL,
			pszText,
			ATL::lstrlen(pszText),
			NULL);
#endif

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
/// Map of handlers for DWebBrowserEvents2 events
	BEGIN_SINK_MAP(CScriptEngineClient)
		SINK_ENTRY_EX(0, __uuidof(DWebBrowserEvents2), WEBBROWSER_ONDOCUMENTCOMPLETE_ID, OnDocumentComplete)
	END_SINK_MAP()

private:
/// Directories
	CScriptDirectories								m_dirs;
/// Current script name
	tstring											m_scriptName;
/// Script object
	CScriptObjectWrapper							m_script;
/// Map of execution environments
	std::map<unsigned int,SPExecEnvironment>		m_execEnvironments;
/// Critical section to access env map
	CCritSectionSimpleObject						m_envSection;
/// Index of current request
	unsigned int									m_requestIndex;
/// Critical section to access request index
	CCritSectionSimpleObject						m_indexSection;
/// Transport stream
	boost::shared_ptr<CAbstractStream>				m_stream;
/// IDispatch of <object> element
	CInterfaceMarshaler<IDispatch>					m_objectDisp;
/// Cleaner
	CCleaner										m_cleaner;
/// Allows decompress directories
	CUnZip32										m_decompressor;
/// Map of variablis to be copied from host document to frame document
	std::map<CComBSTR, CComVariant>					m_copiedVariables;
/// URL of the frame
	tstring											m_frameURL;
/// Manager of event threads
	CEventThreadsManager							m_eventThreadsManager;
/// Timeout for sync calls
	DWORD											m_syncTimeout;
/// Map of requests
	std::map<unsigned int,SPScriptEngineRequest>	m_requests;
/// Critical section to protect requests map
	CCritSectionSimpleObject						m_requestSection;
/// Thread for sending notifications
	CNotificationThread								m_notificationThread;
/// Activity string
	tstring											m_activityString;
private:
/// Checks up transport stream
	void CheckStream();
/// Obtains self IDispatch
	CComPtr<IDispatch> GetSelfDispatch();
/// Returns next index of request
	unsigned int GetRequestIndex();
/// Executes completion procedure
/// @param requestId - id of request(id of execution environment)
/// @param success - indicates success execution
/// @param errorString - description of error
	void ExecCompletionRoutine(unsigned int requestId, bool success, const tstring& errorString);
/// Raises OnDeployComplete event
/// @param success - deploy is completed
/// @param errorString - error description if failed
	void RaiseOnDeployComplete(bool success, const tstring& errorString);
/// Raises OnFilesTransferred event
/// @param success - indicates success transfer
/// @param errorString - error description if failed
	void RaiseOnFilesTransferred(bool success, const tstring& errorString);
/// Raises OnHostInited event
/// @param success - indicates success host initialization
/// @param errorString - error description if failed
	void RaiseOnHostInited(bool success, const tstring& errorString);
/// Raises OnProgress event
/// @param progressMessage - progress message
	void RaiseOnProgress(const tstring& progressMessage);
/// Raises OnUnexpectedError event
/// @param errorCode - internal error code
/// @param errorString - error description
	void RaiseOnUnexpectedError(int errorCode, const tstring& errorString);
/// Raises custom client's event
/// @param eventId - id of event
/// @param requestId - id of request
/// @param success - indicates success
/// @param errorString - error description if failed
	void RaiseCustomEvent(int eventId, unsigned int requestId, bool success, const tstring& stringParam);
/// Callback function for password request at decompression directory
/// @param buf - buffer for password
/// @param size - buffer size
	int OnUnZipPasswd(char* buf, int size);
/// Sends request to initialize host
/// @param async - async request
/// @return id of request
	unsigned int InitHostInternal(bool async);
/// Sends request to deploy
/// @param async - async request
/// @return id of request
	unsigned int DeployInternal(bool async);
/// Send request to invoke file
	void InvokeFileInternal(bool async, SPExecEnvironment env);
/// Send request to invoke code
	void InvokeCodeInternal(bool async, SPExecEnvironment env);
/// Send request to get remote files
/// @return id of request
	unsigned int GetRemoteFilesInternal(bool async, VARIANT files);
/// Creates new request
/// @param requestId - id of new request
/// @param asyc - is async request
/// @return created request
	SPScriptEngineRequest CreateRequest(unsigned int requestId, bool async);
/// Creates execution environment
	SPExecEnvironment CreateEnvironment(CComBSTR lang, CComBSTR fileName, CComBSTR code, CComBSTR procedureName, CComBSTR completionName, CComVariant param1, CComVariant param2);
/// Waiting for response
/// @param requestId - id of request
	void WaitForResponse(unsigned int requestId);
/// Prepares result of execution for sync methods
/// @param requestId - id of request
/// @param result - pointer to VARIANT whick will be returned as result
	void PrepareExecutionResult(unsigned int requestId, VARIANT* result);
/// Handles "OnError" message
/// @param requestId - id of request
/// @param data - variant with message data
	void HandleOnErrorMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg);
/// Handles "OnTimeout" message
/// @param requestId - id of request
/// @param data - variant with message data
	void HandleOnTimeoutMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg);
/// Handles "OnSuccess" message
/// @param requestId - id of request
/// @param data - variant with message data
	void HandleOnSuccessMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg);
/// Handles "OnDeploy" message
/// @param requestId - id of request
/// @param data - variant with message data
	void HandleOnDeployMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg);
/// Handles "SetParams" message
/// @param requestId - id of request
/// @param data - variant with message data
	void HandleSetParamsMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg);
/// Handles "OnGetFiles" message
/// @param requestId - id of request
/// @param data - variant with message data
	void HandleOnGetFilesMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg);
/// Returns activity string by type
	tstring GetActivity(ESEActivityType activityType);
protected:
/// Broker intercommunication
	CComGITPtr<_IBrokerClientEvents> m_brokerEvents;

/// Start local script execution
	void StartScript(BSTR scriptName);

/// Handles received message
/// @param requestId - id of request
/// @param msgType - type of message
/// @param msgData - buffer with message's data
/// @param size - buffer size
	virtual void HandleProtocolMessage(unsigned int requestId, EScriptEngineMsgType msgType, boost::shared_array<char> msgData, const unsigned int size);

/// Connection point implementation
	SAFEEVENTS_CONNECTIONPOINT_IMPL(CScriptEngineClient,_IScriptEngineClientEvents);

public:
/// IScriptEngineClient interface realization
/// Asynchronously initializes host
	STDMETHOD(InitHost)(void);
/// Returns TRUE if host inited
	STDMETHOD(HostInited)(LONG* inited);
/// Asynchronously deploys script
	STDMETHOD(Deploy)(void);
/// Sets script execution timeout
	STDMETHOD(SetTimeOut)(LONG timeOut);
/// Asynchronously runs remote procedure
	STDMETHOD(InvokeFile)(BSTR language, BSTR fileName, BSTR procedureName, VARIANT param1, VARIANT param2, BSTR completionName);
/// Asynchronously runs remote procedure
	STDMETHOD(InvokeCode)(BSTR language, BSTR code, BSTR procedureName, VARIANT param1, VARIANT param2, BSTR completionName);
/// Asynchronously transffers remote file to local machine
	STDMETHOD(GetRemoteFiles)(VARIANT files);
/// Writes error to log file
	STDMETHOD(WriteLogError)(BSTR errorText);
/// Writes warning to log file
	STDMETHOD(WriteLogWarning)(BSTR warnText);
/// Writes message to log file
	STDMETHOD(WriteLogMessage)(BSTR msgText);
/// Synchronously initializes host
	STDMETHOD(InitHostSync)(void);
/// Synchronously deploys script
	STDMETHOD(DeploySync)(void);
/// Synchronously runs remote procedure
	STDMETHOD(InvokeFileSync)(BSTR language, BSTR fileName, BSTR procedureName, VARIANT param1, VARIANT param2, VARIANT* result);
/// Synchronously runs remote procedure
	STDMETHOD(InvokeCodeSync)(BSTR language, BSTR code, BSTR procedureName, VARIANT param1, VARIANT param2, VARIANT* result);
/// Synchronously transffers remote file to local machine
	STDMETHOD(GetRemoteFilesSync)(VARIANT files);
/// Sets activity string
	STDMETHOD(NotifyActivity)(BSTR activity);
/// Brings script endine widget on top over other widgets
	STDMETHOD(BringOnTop)();
/// OnDocumentComplete event handler (event of web browser)
	virtual HRESULT __stdcall OnDocumentComplete(IDispatch *pDisp, VARIANT *URL);
public:
/// Infrastracture methods
/// The method initialize viewer by _IBrokerClientEvents interface
	STDMETHOD(Init)(IUnknown *events);
/// The method handle request sent by Broker
	STDMETHOD(HandleRequest)(BSTR dstUserId,ULONG dstSvcId,BSTR srcUserId,ULONG srcSvcId,ULONG rId,ULONG rType,ULONG param,BSTR params);
/// The method call when asked sub stream connected
	STDMETHOD(SetSubStream)(BSTR dstUserId, ULONG dstSvcId, ULONG streamId, ULONG pointer_shared_ptr_stream);

};

