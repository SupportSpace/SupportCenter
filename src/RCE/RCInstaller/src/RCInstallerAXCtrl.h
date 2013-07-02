/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRCInstallerAXCtrl.h
///
///  IRCInstallerAXCtrl,  ActiveX installer
///
///  @author "Archer Software" Solovyov K. @date 20.12.2006
///
////////////////////////////////////////////////////////////////////////
// RCInstallerAXCtrl.h : Declaration of the CRCInstallerAXCtrl
#pragma once
#include "resource.h"       // main symbols
#include <atlctl.h>
#include <RCEngine/RCEngine.h>
#include <RCEngine/sitelock/RCSiteLockImpl.h>
#include <AidLib/CThread/CThread.h>
#include <AidLib/WatchDog/CProcessWatchDog.h>
//#include "hardcode.h"

#include <Msi.h>//TODO 

#define GUID_LENGTH			39		/// Including trailing NULL
#define UNICODE_MAX_PATH	32768	/// maximum length for a unicode path 
#define ACTION_COUNT_GUESS	23		/// assumed actions count

#define DEF_THREAD_WAIT_TIME 30000	// Default timeout for waiting of async thread

#define ALIVE_MSG (WM_USER + 123)
#define WATCHDOG_TIMEOUT 1000

/// hay buck
class CWatchDogThread : public CThread
{
private:
	std::set<HWND> m_clientWindows;
	CCritSectionSimpleObject m_criticalSection;
public:
	void AddWindow(const HWND window)
	{
	TRY_CATCH
		CCritSection cs(&m_criticalSection);
		m_clientWindows.insert(window);
		Log.Add(_MESSAGE_,_T("Added window %d to WatchDog"),window);

		// Starting if wasn't started before
		if (Terminated())
		{
			Start();
		}

	CATCH_THROW()
	}

	virtual void Execute(void*)
	{
	TRY_CATCH
		Log.Add(_MESSAGE_,_T("WatchDog thread started"));
		while(!Terminated())
		{
			CCritSection cs(&m_criticalSection);
			/// Refreshing list
			for(std::set<HWND>::iterator window = m_clientWindows.begin();
				window != m_clientWindows.end();)
			{
				if (FALSE == SendMessage(*window, ALIVE_MSG,0,0))
				{
					Log.WinError(_MESSAGE_,_T("Dead client found for HWND(%d): "),*window);
					window = m_clientWindows.erase(window);
				} 
				else
					++window;
			}
		
			if (m_clientWindows.empty())
			{
				Log.Add(_MESSAGE_,_T("RCInstaller watch dog: no more alive clients, terminating process"));
				TerminateProcess(GetCurrentProcess(),0);
				Log.WinError(_ERROR_,_T("Failed to terminate process "));
			}
			
			cs.Unlock();
			
			Sleep(WATCHDOG_TIMEOUT);
	
		}
	CATCH_LOG()
	}
};


//------------------------------------------------------------------------

/// Adds domain to trusted zone. 
/// @param domainName domain name
/// @remark if add to trusted zone failed - exeption thrown
void AddDomain2Trusted(const tstring& domainName);

/// Returns true if process is isolated
/// for example if we run activeX on vista from untrusted site
bool IsProcessIsolated();

//------------------------------------------------------------------------
// IRCInstallerAXCtrl
[
	object,
	uuid(953A47A7-143C-4EE6-8C23-0363EBDBC0E8),
	dual,
	helpstring("IRCInstallerAXCtrl Interface"),
	pointer_default(unique)
]
__interface IRCInstallerAXCtrl : public IDispatch
{
	[id(1), helpstring("method IsProcessUnderUIPI")] HRESULT IsProcessUnderUIPI([out,retval] BOOL* result);
	[id(2), helpstring("method GetServiceID")] HRESULT GetServiceID([in]BSTR featureName,[in] BSTR serviceName, [out,retval] BSTR* serviceGUID);
	[id(3), helpstring("method SetInternalUI")] HRESULT SetInternalUI([in] ULONG dwUILevel, [out,retval] ULONG* dwOldUILevel);
	[id(4), helpstring("method ConfigureProductEx")] HRESULT ConfigureProductEx([in] BSTR comandLine);
	[id(5), helpstring("method CancelInstalling")] HRESULT CancelInstalling(void);
	
	[id(6), helpstring("method EnableLog")] HRESULT EnableLog([in] ULONG logMode,[in] BSTR logFile, [in] ULONG logAttributes);
	[id(7), helpstring("method DirectConfigureProductEx")]	HRESULT DirectConfigureProductEx([in] BSTR comandLine);
	[id(8), helpstring("method SelfTerminate")]	HRESULT StartWhatching([in] ULONG pid);
};

///DOXYS_OFF
/*
///DOXYS_ON
#define __interface struct
#define STDMETHOD(method)       virtual HRESULT STDMETHODCALLTYPE method
///DOXYS_OFF
*/
///DOXYS_ON

// _IRCInstallerAXCtrlEvents
[
	uuid("6ED4958A-D8DB-466F-A290-D52F5AB39F29"),
	dispinterface,
	helpstring("_IRCInstallerAXCtrlEvents Interface")
]
__interface _IRCInstallerAXCtrlEvents
{	/// The event notifies that log message become.
	/// @param message is a text log message
	/// @param severity type of log message _EXCEPTION=0,_ERROR,_WARNING,_MESSAGE,_SUCCESS
	[id(1), helpstring("method NotifyLogMessage")] HRESULT NotifyLogMessage([in] BSTR message, [in] LONG severity);
	/// The event notifies that feature installation has completed.
	/// @param result Equal 0 if feature installation is success, nonzero value otherwise.
	[id(2), helpstring("method NotifyFeatureInstalled")] HRESULT NotifyFeatureInstalled(LONG result);
	/// The event notifies progress of installing feature
	/// @param percentCompleted percent completed of installing process
	/// @param status Status is a text message that describes the current step.
	[id(3), helpstring("method NotifyInstalling")] HRESULT NotifyInstalling([in] LONG percentCompleted, [in] BSTR status);
};

class CNetworkLog;

// CRCInstallerAXCtrl
[
	coclass,
	threading(apartment),
	vi_progid("RCInstaller.RCInstallerAXCtrl"),
	progid("RCInstaller.RCInstallerAXCtrl.1"),
	version(1.0),
	uuid("7B3BBD75-A77C-40D9-BD0E-943055093249"),
	helpstring("RCInstallerAXCtrl Class"),
	event_source(com),
	support_error_info(IRCInstallerAXCtrl),
	registration_script("control.rgs")
]
class ATL_NO_VTABLE CRCInstallerAXCtrl : 
	public cLog,
	public IRCInstallerAXCtrl,
	// invoke this object events. use /Fx flag for see inject code and ConfigureProductEx() comment
	public _IRCInstallerAXCtrlEvents,
	public IPersistStreamInitImpl<CRCInstallerAXCtrl>,
	public IOleControlImpl<CRCInstallerAXCtrl>,
	public IOleObjectImpl<CRCInstallerAXCtrl>,
	public IOleInPlaceActiveObjectImpl<CRCInstallerAXCtrl>,
	public IViewObjectExImpl<CRCInstallerAXCtrl>,
	public IOleInPlaceObjectWindowlessImpl<CRCInstallerAXCtrl>,
	public IPersistStorageImpl<CRCInstallerAXCtrl>,
	public ISpecifyPropertyPagesImpl<CRCInstallerAXCtrl>,
	public IQuickActivateImpl<CRCInstallerAXCtrl>,
	public IDataObjectImpl<CRCInstallerAXCtrl>,
	public CComControl<CRCInstallerAXCtrl>,
	public CRCSiteLockImpl<CRCInstallerAXCtrl>
{
private:
	CSuicideProcessWatchDog m_watchDog;
	std::auto_ptr<CInstanceTracker> m_instanceTracker;
	/// Handle to internal thread
	HANDLE m_thread;
public:
	CRCInstallerAXCtrl();
	virtual ~CRCInstallerAXCtrl();
///DOXYS_OFF	
BEGIN_COM_MAP(CRCInstallerAXCtrl)
	COM_INTERFACE_ENTRY(IObjectSafety)
	COM_INTERFACE_ENTRY(IObjectSafetySiteLock)
END_COM_MAP( )

DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE | 
	OLEMISC_CANTLINKINSIDE | 
	OLEMISC_INSIDEOUT | 
	OLEMISC_ACTIVATEWHENVISIBLE | 
	OLEMISC_SETCLIENTSITEFIRST
)


BEGIN_PROP_MAP(CRCInstallerAXCtrl)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
END_PROP_MAP()


BEGIN_MSG_MAP(CRCInstallerAXCtrl)
	/*if(ALIVE_MSG == uMsg)
	{
		bHandled = TRUE;
		return TRUE;
	}*/
	if (WM_DESTROY == uMsg)
	{
		ReleaseOutProcInstance();
	}
	MESSAGE_HANDLER(m_msgFireEventOtherThreadsLog,FireEventOtherThreadsLog)
	MESSAGE_HANDLER(m_msgFireEventOtherThreadsFeatureInstalled,FireEventOtherThreadsFeatureInstalled)
	MESSAGE_HANDLER(m_msgFireEventOtherThreadsInstalling,FireEventOtherThreadsInstalling)
	CHAIN_MSG_MAP(CComControl<CRCInstallerAXCtrl>)
	DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
///DOXYS_ON	
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

protected:
	/// message code of event fire in other thread
	const UINT m_msgFireEventOtherThreadsLog;
	const UINT m_msgFireEventOtherThreadsFeatureInstalled;
	const UINT m_msgFireEventOtherThreadsInstalling;
	/// message handler. function fire NotifyLogMessage event 
	LRESULT FireEventOtherThreadsLog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	/// message handler. function fire NotifyFeatureInstalled event
	LRESULT FireEventOtherThreadsFeatureInstalled(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	/// message handler. function fire NotifyInstalling event
	LRESULT FireEventOtherThreadsInstalling(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	/// function fire event NotifyInstalling from any thread
	void FireNotifyInstalling(int percentCompleted,const TCHAR* status);


	/// A virtual method that prepare message string and fire event+ NotifyLogMessage
	virtual void AddList(const cEventDesc &EventDesc, const TCHAR *Item, ...)throw( );
	virtual void AddString(const TCHAR* LogString, const eSeverity Severity)throw(){}
	


public:

	__event __interface _IRCInstallerAXCtrlEvents;
// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// IRCInstallerAXCtrl
	//TODO viewer in scale mode white window 
	HRESULT OnDraw(ATL_DRAWINFO& di);

	DECLARE_PROTECT_FINAL_CONSTRUCT()
	HRESULT FinalConstruct();
	void FinalRelease();

protected:	
	/// old UI message handler
	INSTALLUI_HANDLER	m_oldUI;
	/// internal (for this class, external for WI) function UI message handler
	static INT CALLBACK InstallUIHandler(LPVOID pvContext,UINT iMessageType,LPCTSTR szMessage);
	///	internal current Total tick value for current state. used in InstallUIHandler function
	int m_progressTotal;
	///	internal current tick value for current state. used in InstallUIHandler function
	int m_progressCurrent;
	///	internal InstallUIHandler return value, used for canceling WI operations
	DWORD m_cancelInstalling;
	/// internal current progress message
	tstring m_progressStatus;
	
	/// Product id in msi package. Generate by GUIDGEN in RCComponents (WiX) project.
	const TCHAR*const m_GUIDProduct;
	/// CRCInstaller component id in msi package. Generate by GUIDGEN in RCComponents (WiX) project. 
	const TCHAR*const m_GUIDCRCInstaller;

	/// The function retrieve out of process companion instance, in first call create its. 
	/// The ActiveX class must allow out of process creation (DllSurrogate registry value) and Integrity level elevation for Vista.
	/// (Elevation registry key and LocalizedString registry value).
	///	@return pointer on IRCInstallerAXCtrl interface
	CComPtr<IRCInstallerAXCtrl> GetOutProcInstance(void);
	/// The method release out of process companion instance, then object can create new instance by call GetOutProcInstance() method.
	void ReleaseOutProcInstance(void);
	/// out of process companion
	struct
	{
		/// pointer to IConnectionPoint for _IRCInstallerAXCtrlEvents event interface
		CComPtr<IConnectionPoint> connectionPoint;
		/// Advise cookie for event
		DWORD	cookieEvent;
		/// pointer to IRCInstallerAXCtrl interface of out of process object
		CComPtr<IRCInstallerAXCtrl> instance;
		/// pointer to IStream is used by interthread marshaling.
		IStream *stream;
		/// commandLine parameter is used by asynchronous call ConfigureProductEx
		CComBSTR commandLine;
	} m_outProcInstance;
	/// thread fuction for asynchronous call ConfigureProductEx
	friend DWORD WINAPI OutProcConfigureProductThreadProc(LPVOID lpParameter);

	///
	DWORD m_logMode;
	///
	CComBSTR m_logFile;
	///
	DWORD m_logAttributes;

	/// The method retrieve creating page URL
	/// @param url Buffer for url string
	///	@param urlLength Length of buffer for url string in TCHAR
	void GetPageUrl(wchar_t *url, size_t urlLength);

	/// Return description from action string
	/// The format of this string is Action [1]: [2]. [3] where:
	/// [1] is the TIME the action started
	/// [2] is the name of the action (from the particular Sequence table)
	/// [3] is the description of the action (as described in the Description column of the ActionText table or established via a MsiProcessMessage call)
	/// For example:
	/// Action 16:31:56: RemoveRegistryValues. Removing system registry 
	/// @see http://msdn2.microsoft.com/En-US/library/aa370573.aspx
	/// @if there's no action description, then action name is returned
	static tstring GetActionDescription(const tstring& actionString);

public:
	//TODO correct comment!!!!
	/// The method finds and retrieves GUID. 
	/// @param serviceName service name.
	/// @param serviceGUID [out,retval] parameter return service GUID string if feature has found.
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(GetServiceID)(BSTR featureName,BSTR serviceName,BSTR* serviceGUID);

	/// The method is canceled installation process
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(CancelInstalling)(void);

	/// Use to check if process is restricted by UIPI (windows Vista "super" feature)
	/// @param result [out,retval] parameter return true if current process is restricted by UIPI
	/// @return S_OK
	STDMETHOD(IsProcessUnderUIPI)(BOOL* result);

	/// The DirectConfigureProductEx method installs or uninstalls a product. A product command line may be specified.
	/// @param commandLine Specifies the command line property settings. This should be a list of the format Property=Setting Property=Setting. The command line passed in as szCommandLine can contain any of the Feature Installation Options Properties (MSDN > Windows Installer > Properties).
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(DirectConfigureProductEx)(BSTR commandLine);
	
	/// The method installs or uninstalls a product. A product command line may be specified. The method is asynchronous.
	/// The method create out-of-process CRCInstallerAXCtrl activeX object and invoke ConfigureProductEx of this object in separate thread.
	/// Vista: Out-of-process ActiveX object is created due to COM Elevation moniker whith Administrator level. Therefore, 
	/// the ActiveX class must allow out of process creation (DllSurrogate registry value) and Integrity level elevation (Elevation 
	/// registry key and LocalizedString registry value).\n For this implementation, _IRCInstallerAXCtrlEvents intarface is added
	/// to inherit. This interface methods call corresponding event method (attributed coclass, use /Fx for to see injected code).
	/// If this COM object event is connected to other CRCInstallerAXCtrl, there are passing event from other to this object event recipient.
	/// @param commandLine Specifies the command line property settings. This should be a list of the format Property=Setting Property=Setting. The command line passed in as szCommandLine can contain any of the Feature Installation Options Properties (MSDN > Windows Installer > Properties).
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(ConfigureProductEx)(BSTR comandLine);
	
	/// The SetInternalUI method enables the installer's internal user interface. Then this user interface is used for all subsequent calls to user-interface-generating installer functions in this process.
	/// @param dwUILevel Specifies the level of complexity of the user interface. This parameter can be one of the following values: \n INSTALLUILEVEL_NOCHANGE = 0,		// UI level is unchanged	\n INSTALLUILEVEL_DEFAULT  = 1,    // default UI is used	\n INSTALLUILEVEL_NONE     = 2,    // completely silent installation	\n INSTALLUILEVEL_BASIC    = 3,    // simple progress and error handling	\n INSTALLUILEVEL_REDUCED  = 4,    // authored UI, wizard dialogs suppressed	\n INSTALLUILEVEL_FULL     = 5,    // authored UI with wizards, progress, errors	\n INSTALLUILEVEL_ENDDIALOG    = 0x80, // display success/failure dialog at end of install	\n INSTALLUILEVEL_PROGRESSONLY = 0x40, // display only progress dialog	\n INSTALLUILEVEL_HIDECANCEL   = 0x20, // do not display the cancel button in basic UI	\n INSTALLUILEVEL_SOURCERESONLY = 0x100, // force display of source resolution even if quiet
	/// @param dwOldUILevel	[out,retval] The previous user interface level is returned. If an invalid dwUILevel is passed, then INSTALLUILEVEL_NOCHANGE = 0 is returned.
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(SetInternalUI)(ULONG dwUILevel, ULONG* dwOldUILevel);
	/// The EnableLog method sets the log mode for all subsequent installations that are initiated in the calling process.
	/// Create file name due to change extension Component Path full file name on a ".log".
	/// @param logMode Specifies the log mode. This parameter can be one or more of the following values:\n0x00000001 INSTALLLOGMODE_FATALEXIT Logs out of memory or fatal exit information.\n0x00000002 INSTALLLOGMODE_ERROR Logs the error messages.\n0x00002000 INSTALLLOGMODE_EXTRADEBUG Sends extra debugging information, such as handle creation information, to the log file.	Windows XP/2000 and Windows 98/95:  This feature is not supported.\n0x00000004 INSTALLLOGMODE_WARNING Logs the warning messages.\n0x00000008 INSTALLLOGMODE_USER Logs the user requests.\n0x00000010 INSTALLLOGMODE_INFO Logs the status messages that are not displayed.\n0x00000040 INSTALLLOGMODE_RESOLVESOURCE Request to determine a valid source location.\n0x00000080 INSTALLLOGMODE_OUTOFDISKSPACE Indicates insufficient disk space.\n0x00000100 INSTALLLOGMODE_ACTIONSTART Logs the start of new installation actions.\n0x00000200 INSTALLLOGMODE_ACTIONDATA Logs the data record with the installation action.\n0x00000800 INSTALLLOGMODE_COMMONDATA Logs the parameters for user-interface initialization.\n0x00000400 INSTALLLOGMODE_PROPERTYDUMP Logs the property values at termination.\n0x00001000 INSTALLLOGMODE_VERBOSE Sends large amounts of information to a log file not generally useful to users. May be used for technical support.\n0x00004000 INSTALLLOGMODE_SHOWDIALOG \n Set to zero disable logging.
	/// @param logAttributes Specifies how frequently the log buffer is to be flushed. \n0x00000001 INSTALLLOGATTRIBUTES_APPEND If this value is set, the installer appends the existing log specified by szLogFile. If not set, any existing log specified by szLogFile is overwritten.\n0x00000002 INSTALLLOGATTRIBUTES_FLUSHEACHLINE Forces the log buffer to be flushed after each line. If this value is not set, the installer flushes the log buffer after 20 lines by calling FlushFileBuffers. 
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(EnableLog)(ULONG logMode,BSTR logFile,ULONG logAttributes);

	/// Starts watch-dog timer for process with pid
	STDMETHOD(StartWhatching)(ULONG pid);
};

