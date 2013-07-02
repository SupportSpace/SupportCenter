/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  RCViewerAXCtrl.h
///
///  IRCViewerAXCtrl,  ActiveX wrapper of CRCViewer
///
///  @author "Archer Software" Solovyov K. @date 23.11.2006
///
////////////////////////////////////////////////////////////////////////
// RCViewerAXCtrl.h : Declaration of the CCoRCViewer
#pragma once
#include "resource.h"       // main symbols
#include <atlctl.h>
#include <RCEngine/RCEngine.h>
#include "RCViewerImpl.h"
#include <RCEngine/Streaming/cinstreamgzipped.h>
#include <RCEngine/sitelock/RCSiteLockImpl.h>
#include <RCEngine/SafeEvents/CSafeEvents.h>
#include "AXmisc.h"
#include "CRCViewerUIMediator.h"

#pragma comment(lib,"Ws2_32.lib")

#include "..\..\Brokers\Shared\IBrokerClient.h"
#include "..\..\Brokers\Shared\CAvailableServices.h"
#include "..\..\Brokers\Shared\BrokersTypes.h"
#include "..\..\Brokers\Shared\CRIdGen.h"
#include <AidLib/CThread/CThread.h>

#define RECONNECT_TIMEOUT 15000 /*15 seconds timeout before starting reconnect procedure*/


//class CRCViewerUIMediator;
// IRCViewerAXCtrl
[
	object,
	uuid(328007B2-63AE-4DCF-9CC4-C3AB6052B5A1),
	dual,
	helpstring("IRCViewerAXCtrl Interface"),
	pointer_default(unique)
]
__interface IRCViewerAXCtrl
{
	[id(2), helpstring("method SetDisplayMode")] HRESULT SetDisplayMode([in] LONG displayMode);
	[id(5), helpstring("method SetSessionOpts")] HRESULT SetSessionOpts([in] LONG colorDepth, [in] LONG encoding, [in] VARIANT_BOOL useCompressLevel, [in] LONG compressLevel, [in] VARIANT_BOOL jpegCompress, [in] LONG jpegQualityLevel);
	[id(6), helpstring("method SetSessionMode")] HRESULT SetSessionMode([in] LONG mode, [in] VARIANT_BOOL state);
	[id(7), helpstring("method SetSessionRecording")] HRESULT SetSessionRecording([in] BSTR fileName, [in] VARIANT_BOOL mode);
	[id(8), helpstring("method StartPlayback")] HRESULT StartPlayback([in] BSTR fileName);
	[id(9), helpstring("method SetDelayFactor")] HRESULT SetDelayFactor([in] DOUBLE delayFactor);
	[id(10), helpstring("method SetPlaybackMode")] HRESULT SetPlaybackMode([in] LONG mode);
	[id(11), helpstring("method SetCaptureAlphaBlend")] HRESULT SetCaptureAlphaBlend([in] VARIANT_BOOL captureAlphaBlend);
	[id(12), helpstring("method SetUIStatus")] HRESULT SetUIStatus([in] LONG status, [in] BSTR message);
	[id(13), helpstring("method Init")] HRESULT Init([in] BSTR peerId);
};


// _IRCViewerAXCtrlEvents
[
	uuid("B0FC3CF7-BEA8-4162-BF7A-8A056701E038"),
	dispinterface,
	helpstring("_IRCViewerAXCtrlEvents Interface")
]
///DOXYS_OFF
/*
///DOXYS_ON
#define __interface struct
#define STDMETHOD(method)       virtual HRESULT STDMETHODCALLTYPE method
///DOXYS_OFF
*/
///DOXYS_ON
/// ActiveX events for CCoRCViewer
__interface _IRCViewerAXCtrlEvents
{
};


// CCoRCViewer
[
	coclass,
	threading(apartment),
	vi_progid("RCUI.RCViewerAXCtrl"),
	progid("RCUI.RCViewerAXCtrl.1"),
	version(2.0),
	uuid("DA4679AA-3239-43EF-8D59-6E82AEF6F081"),
	helpstring("RCViewerAXCtrl Class"),
	event_source(com),
	support_error_info(IRCViewerAXCtrl),
	registration_script("control.rgs")
]
/// ActiveX wrapper of CRCViewer
/// @remark threading("apartment"),vi_progid("RCUI.RCViewerAXCtrl"),progid("RCUI.RCViewerAXCtrl.1"),version(1.0),uuid("DA4679AA-3239-43EF-8D59-6E82AEF6F081"),helpstring("RCViewerAXCtrl Class"),	event_source("com"),support_error_info(IRCViewerAXCtrl),registration_script("control.rgs")
class ATL_NO_VTABLE CCoRCViewer : 
	public CSafeEvents<CCoRCViewer,_IRCViewerAXCtrlEvents>,
	public IPersistStreamInitImpl<CCoRCViewer>,
	public IOleControlImpl<CCoRCViewer>,
	public IOleObjectImpl<CCoRCViewer>,
	public IOleInPlaceActiveObjectImpl<CCoRCViewer>,
	public IViewObjectExImpl<CCoRCViewer>,
	public IOleInPlaceObjectWindowlessImpl<CCoRCViewer>,
	public IPersistStorageImpl<CCoRCViewer>,
	public ISpecifyPropertyPagesImpl<CCoRCViewer>,
	public IQuickActivateImpl<CCoRCViewer>,
	public IDataObjectImpl<CCoRCViewer>,
	//public CComControl<CCoRCViewer>,
	public CComCompositeControl<CCoRCViewer>,
	public IDispatchImpl<IRCViewerAXCtrl, &__uuidof(IRCViewerAXCtrl), &LIB_GUID>,
	//	public IObjectSafetyImpl<CCoRCViewer,INTERFACESAFE_FOR_UNTRUSTED_CALLER|INTERFACESAFE_FOR_UNTRUSTED_DATA>
	public CRCSiteLockImpl<CCoRCViewer>,
	public IBrokerClient,
	protected CThread,
	protected CRIdGen
{
	friend class CRCViewerImpl;
	friend class CRCViewerUIMediator;
private:
	CInstanceTracker m_instanceTracker;
public:
		/// ActiveX friendly name, use by registration
	static const TCHAR* GetObjectFriendlyName() 
	{
		return _T("Support Platform Remote Control Viewer");
	}
	// / initializes object instance
	CCoRCViewer();

	// / destroys object instance
	~CCoRCViewer();
	enum { IDD = IDD_RCVIEWERDLG };
	///DOXYS_OFF	
	BEGIN_COM_MAP(CCoRCViewer)
		COM_INTERFACE_ENTRY(IObjectSafety)
		COM_INTERFACE_ENTRY(IObjectSafetySiteLock)
		COM_INTERFACE_ENTRY(IRCViewerAXCtrl)
		COM_INTERFACE_ENTRY(IBrokerClient)
	END_COM_MAP( )

	DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE | 
	OLEMISC_CANTLINKINSIDE | 
		OLEMISC_INSIDEOUT | 
		OLEMISC_ACTIVATEWHENVISIBLE | 
		OLEMISC_SETCLIENTSITEFIRST
		)


		BEGIN_PROP_MAP(CCoRCViewer)
			PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
			PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
		END_PROP_MAP()


		BEGIN_MSG_MAP(CCoRCViewer)
			TRY_CATCH
			MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
			MESSAGE_HANDLER(m_msgFireEventOtherThreadsStarted,FireEventOtherThreadsStarted)
			MESSAGE_HANDLER(m_msgFireEventOtherThreadsStopped,FireEventOtherThreadsStopped)
			MESSAGE_HANDLER(m_msgFireEventOtherThreadsStartService,FireEventOtherThreadsStartService)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			//CHAIN_MSG_MAP(CComControl<CCoRCViewer>)
			MESSAGE_HANDLER(WM_SIZE, OnSize)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			CHAIN_MSG_MAP(CComCompositeControl<CCoRCViewer>)
			DEFAULT_REFLECTION_HANDLER()
			CATCH_LOG("BEGIN_MSG_MAP(CCoRCViewer)")
		END_MSG_MAP()
		///DOXYS_ON
protected:
	/// message code of event fire in other thread
	const UINT m_msgFireEventOtherThreadsStarted;
	const UINT m_msgFireEventOtherThreadsStopped;
	const UINT m_msgFireEventOtherThreadsStartService;

	/// Controls mouse activate messages in order to select viewer's vidget by click
	LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	/// function fire event from other threads. Starts service
	LRESULT FireEventOtherThreadsStartService(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	/// function fire event from other threads. call NotifySessionStart
	LRESULT FireEventOtherThreadsStarted(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	/// function fire event from other threads. call NotifySessionStop
	LRESULT FireEventOtherThreadsStopped(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
	// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

	// IRCViewerAXCtrl
	/// WM_PAINT message handler
	//HRESULT OnDraw(ATL_DRAWINFO& di);

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	//HRESULT FinalConstruct(){m_ui.CreateUI(m_hWnd);return S_OK;}
	//void FinalRelease(){}

	/// Sends Ctrl + alt + del to host
	void SendCtrlAltDel();

	/// Applys current options set
	void ApplyOptions();

protected:

	/// MessageBox request identifiers
	typedef enum _EMessageBoxRequests
	{
		RID_PERMISSION,
	} EMessageBoxRequests;

	/// True if user has allowed full control
	ERCAccessMode m_maximumAllowedPermission;
	/// Requested permission 
	ERCAccessMode m_requestedPermission;

	/// internal RCViewerImpl object
	boost::shared_ptr<CRCViewerImpl> m_spViewer;
	/// internal	 CAbstractStream for playback moda
	boost::shared_ptr<CInStreamGZipped> m_spstream;//boost::shared_ptr<CAbstractStream> m_spstream;
	/// internal options set value. It are used jast befor CRCViewer::Start() in Start() method.
	SViewerOptions m_opts;
	/// Remote desktop width
	int m_cx;
	/// Remoted desktop height
	int m_cy;
	/// Critical section for thread safety
	CRITICAL_SECTION m_cs;
	/// types CCoRCViewer and CWindowImplBaseT<> offset
	static int m_wndClassOffset;
	/// true if alpha blend capturing is turned on
	bool m_captureAlphaBlend;
	/// true to hide  wallpaper on host
	bool m_hideWallpaper;
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual WNDPROC GetWindowProc(){return CCoRCViewer::WindowProc;}

	/// A virtual method that called after session is started, 
	/// or notifies remote desktop size is changed. Size is real remote desktop size.
	/// @param width new remote desktop width
	/// @param height new remote desktop height
	virtual void SetRemoteDesktopSize(const int width, const int height);

	/// UI mediator
	CRCViewerUIMediator m_ui;
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

public:
	///AcitiveX events
	__event __interface _IRCViewerAXCtrlEvents;
	//__event __interface _IBrokerClientEvents;

	/// Toggle Display mode between Window Scaling, Window Scrolling and Full Screen.
	/// Can be invoked before Start() or during an active session.
	/// Window Scale Mode applies image scaling to the image of the remote desktop, so that it will fit the current window size (by stretching or shrinking it).
	/// Window Scrolling Mode displays the remote desktop as-is (1:1 pixel size).  If the remote desktop is larger than the window it is displayed in, horizontal and/or vertical scroll bars are added.
	/// Full Screen mode is identical to Window Scaling mode, but uses a full screen
	/// @param displayMode new display mode. Where SCALE_MODE=0, SCROLL_MODE=1, FULLSCREEN_MODE=2. See EDisplayMode definition for last version.
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(SetDisplayMode)(LONG displayMode);

	/// Set custom options set for viewer
	/// @param colorDepth	Colors count. 0: full colors; 1:256 colors; 2:64 colors; 3:8 colors; 4:8 grey colors; 5:4 colors; 6:2 grey colors
	/// @param encoding	Preferred encoding.	0:ZRLE; 1:Zlib(+xor); 2:ZlibHex; 3:Hextile; 4:RRE; 5:Raw; 6:Tight;
	/// @param useCompressLevel Use custom zip/tight compression level.
	///	@param compressLevel Custom zip/tight compression level.
	/// @param jpegCompress	Use custom jpeg compression.
	/// @param jpegQualityLevel	Custom jpeg compression.
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	/// @remark Can by invoke only before connection estableshed.
	STDMETHOD(SetSessionOpts)(LONG colorDepth, LONG encoding, VARIANT_BOOL useCompressLevel, LONG compressLevel, VARIANT_BOOL jpegCompress, LONG jpegQualityLevel);

	/// Toggle the boolean State of a specified session Mode.
	/// Can be invoked before or after Start() has been invoked.
	/// @param mode mode to toggle state. VIEW_ONLY=0, VISUAL_POINTER=1. See ESessionMode definition for last version velues.
	/// @param state new state for mode, assign true to set specify mode, assign false to reset specify mode.
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(SetSessionMode)(LONG mode, VARIANT_BOOL state);

	/// Toggles session recording mode.
	/// When enabled, recording takes place into the specified file name.
	/// The specified file create in a hard-coded directory(My Documents), and the file name is cleaned up to prevent creation of files outside this directory (e.g. using relative paths, such as “..\..\filename.ext”). There append extension ".rce" to specified file name.
	/// @param fileName Recording file name only, don't includ path and extension.
	/// @param mode Recording mode, true : start recording, false :	stop recording
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(SetSessionRecording)(BSTR fileName, VARIANT_BOOL mode);

	/// Begin or ending playback of a recorded session. 
	/// The specified file create in a hard-coded directory(My Documents), and the file name is cleaned up to prevent creation of files outside this directory (e.g. using relative paths, such as “..\..\filename.ext”). There append extension ".rce" to specified file name.
	/// @param fileName Recording file name only, don't includ path and extension.
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(StartPlayback)(BSTR fileName);

	///	Set Delay Factor when playback is in progress. 
	/// @param delayFactor if delayFactor=1.0 playback with real speed, if delayFacotr>1.0 slows playback down if delayFactor<1.0 playback faster then original. For example, delayFactor=0.5 mean playback in 2 time highest speed, and delayFactor=4.0 mean playback in 4 time slowest speed.
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(SetDelayFactor)(DOUBLE delayFactor);

	/// Set playback mode during playback session. Used for pause, stop and restart playback mode. May be invoke in plaback progress only. 
	/// @param mode Playback mode. May be assing value as folow: 0 : Starts playback if it was stopped or paused; 1 : Stops stream if it was running or paused; 2 : Pauses stream if it was running
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(SetPlaybackMode)(LONG mode);

	/// Turn on or off alpha blended and layered windows capturing
	/// @param captureAlphaBlend set this to true if to turn alpha blend capturing on, false otherway
	/// @remark alphablended and layered windows capturing can decrease performance dramatically
	/// so by default it's turned off, and be started only manually as option
	STDMETHOD(SetCaptureAlphaBlend)(VARIANT_BOOL captureAlphaBlend);

	/// Set UI status
	/// @param UI status for see enum ERCViewerUIStatuses for details
	/// @param message
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD (SetUIStatus)(LONG status, BSTR message);

	/// Init Viewer ActiveX object
	/// @param peerId peer id host side
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(Init)(BSTR peerId);



	// new infrastructure
protected:
	/// borker intercommunication
	CComGITPtr<_IBrokerClientEvents> m_brokerEvents;
	
	/// service sub stream, it's used for peer communication
	boost::shared_ptr<CAbstractStream> m_svcStream;

	/// the service stream connected event. set when service stream is connected
	CEvent m_svcStreamConnectedEvent;

	/// Object is shuting down event. Set when service is going to shutdown
	CEvent m_terminateEvent;

	/// The method is entry point of service sub stream creation and recieve handling
	virtual void Execute(void *Params);

	/// The method is handler of UI events (Send button click and others)
	void HandleUIMediatorCommand(ERCViewerUIEvents eventType, LONG param);

	/// The method start RC session
	void InitiateRCConnectAndStart(void);

	/// The method is called when message come through service sub stream
	void HandleCoHostCommand(ULONG buf);

	/// Minimized button pressed event handler
	void OnViewerMinimize();

public:
	
	/// The method initialize viewer by _IBrokerClientEvents interface
	STDMETHOD(Init)(IUnknown *events);
	
	/// The method handle request sent by Broker
	STDMETHOD(HandleRequest)(BSTR dstUserId,ULONG dstSvcId,BSTR srcUserId,ULONG srcSvcId,ULONG rId,ULONG rType,ULONG param,BSTR params);

	/// The method call when asked sub stream connected
	STDMETHOD(SetSubStream)(BSTR dstUserId, ULONG dstSvcId, ULONG streamId, ULONG pointer_shared_ptr_stream);

	virtual HWND CreateControlWindow(HWND hWndParent, RECT& rcPos)
	{
		// Taken from the deep inside of ATL
		CComControl< CCoRCViewer, CAxDialogImpl< CCoRCViewer > >::Create(hWndParent);
		SetBackgroundColorFromAmbient();

		if (m_hWnd != NULL)
			AdviseSinkMap(true);

		return m_hWnd;
	}
};

