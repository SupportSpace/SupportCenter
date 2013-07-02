/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  RCHostAXCtrl.h
///
///  IRCHostAXCtrl,  ActiveX wrapper of CRCHost
///
///  @author "Archer Software" Solovyov K. @date 24.11.2006
///
////////////////////////////////////////////////////////////////////////
// RCHostAXCtrl.h : Declaration of the CRCHostAXCtrl
#pragma once
#include "resource.h"       // main symbols
#include <atlctl.h>
#include <RCEngine/CRCHost.h>
#include <RCEngine/Mediator/CMediatorRefImpl.h>
#include <RCEngine/sitelock/RCSiteLockImpl.h>
#include "AXmisc.h"

#include "..\..\Brokers\Shared\IBrokerClient.h"
#include "..\..\Brokers\Shared\CAvailableServices.h"
#include "..\..\Brokers\Shared\BrokersTypes.h"


//TODO: move to cpp
//#include  <Logging/cLog.h>
//#include  <CSingleton/CSingleton.h>
//																		
//class CGlobalInit
//{
//public:
//	CGlobalInit()
//	{
//		Log.RegisterLog(new cMsgBoxLog(),true,cLog::_EXCEPTION);
//		Log.RegisterLog(new cMsgBoxLog(),true,cLog::_ERROR);
//		Log.RegisterLog(new cMsgBoxLog(),true,cLog::_WARNING);
//	}
//
//	virtual ~CGlobalInit()
//	{
//	}
//};


// IRCHostAXCtrl
[
	object,
	uuid(FEEDFDDD-2851-44AD-A9AF-0C3541B93683),
	dual,
	helpstring("IRCHostAXCtrl Interface"),
	pointer_default(unique)
]
__interface IRCHostAXCtrl : public IMediatorRef
{
	[id(2), helpstring("method StartClient")] HRESULT StartClient([in] BSTR userId, [in] BSTR password, [in] BSTR peerId, [in] BSTR relayServer,[in] ULONG timeOut,[out,retval] LONG* id);
	[id(3), helpstring("method StopClient")] HRESULT StopClient([in] LONG id);
	[id(4), helpstring("method SetSessionMode")] HRESULT SetSessionMode([in] LONG clientId,[in] LONG mode,[in] VARIANT_BOOL state);
	[id(5), helpstring("method SetSessionRecording")] HRESULT SetSessionRecording([in] BSTR fileName, [in] VARIANT_BOOL mode);
	[id(6), helpstring("method ProtectWindow")] HRESULT ProtectWindow([in] VARIANT_BOOL mode);
	[id(7), helpstring("method SetCaptureAlphaBlend")] HRESULT SetCaptureAlphaBlend([in] VARIANT_BOOL captureAlphaBlend);
};


// _IRCHostAXCtrlEvents
[
	uuid("D81E3296-70BC-402A-823F-F30CFF1C997D"),
	dispinterface,
	helpstring("_IRCHostAXCtrlEvents Interface")
]
///DOXYS_OFF
/*
///DOXYS_ON
#define __interface struct
#define STDMETHOD(method)       virtual HRESULT STDMETHODCALLTYPE method
///DOXYS_OFF
*/
///DOXYS_ON
/// ActiveX events for CRCHostAXCtrl
__interface _IRCHostAXCtrlEvents
{	
	/// The event notifies session has started 
	/// @param clientId unique client session identifier of new session
	/// @remark for detail see StartClient() method remark.
	[id(1), helpstring("method NotifySessionStart")] HRESULT NotifySessionStart([in] LONG clientId);
	/// The event notifies session has stopped and why 
	/// @param clientId unique client session identifier of stopped session
	/// @param reasonCode stop reason (Stop() called; Remote Stop() called; Stream signaled end-of-file; etc.)LOCAL_STOP=0, REMOTE_STOP=1, STREAM_ERROR=2, PROTOCOL_ERROR=3, CHANGE_DISPLAY_MODE=4, CONNECTING_ERROR=5, OPENFILE_ERROR=6. See _ESessionStopReason definition for last version of reason code.
	[id(2), helpstring("method NotifySessionStoped")] HRESULT NotifySessionStop([in] LONG clientId, [in] LONG reasonCode);
	/// The event notifies progress of connection	establishment
	/// @param percentCompleted percent completed of connection establishment process
	/// @param status Status is a text message that describes the current step.
	[id(3), helpstring("method NotifyConnecting")] HRESULT NotifyConnecting([in] LONG percentCompleted, [in] BSTR status);
};

// CRCHostAXCtrl
[
	coclass,
	threading(apartment),
	vi_progid("RCUI.RCHostAXCtrl"),
	progid("RCUI.RCHostAXCtrl.1"),
	version(2.0),
	uuid("82AA4446-50DD-443B-9AD7-919B0438E145"),
	helpstring("RCHostAXCtrl Class"),
	event_source(com),
	support_error_info(IRCHostAXCtrl),
	registration_script("control.rgs")
]
//ActiveX wrapper of CRCHost. Multiple inheritance from CRCHost.
/// ActiveX wrapper of CRCHost inherits from CRCHost.
/// @remark coclass, threading("apartment"), vi_progid("RCUI.RCHostAXCtrl"), progid("RCUI.RCHostAXCtrl.1"), version(1.0), uuid("82AA4446-50DD-443B-9AD7-919B0438E145"), helpstring("RCHostAXCtrl Class"), event_source("com"), support_error_info(IRCHostAXCtrl), registration_script("control.rgs")
class ATL_NO_VTABLE CRCHostAXCtrl : 
	public CRCHost,
	public IPersistStreamInitImpl<CRCHostAXCtrl>,
	public IOleControlImpl<CRCHostAXCtrl>,
	public IOleObjectImpl<CRCHostAXCtrl>,
	public IOleInPlaceActiveObjectImpl<CRCHostAXCtrl>,
	public IViewObjectExImpl<CRCHostAXCtrl>,
	public IOleInPlaceObjectWindowlessImpl<CRCHostAXCtrl>,
	public IPersistStorageImpl<CRCHostAXCtrl>,
	public ISpecifyPropertyPagesImpl<CRCHostAXCtrl>,
	public IQuickActivateImpl<CRCHostAXCtrl>,
	public IDataObjectImpl<CRCHostAXCtrl>,
	public CComControl<CRCHostAXCtrl>,
	public IDispatchImpl<CMediatorRefImpl<IRCHostAXCtrl>, &__uuidof(IRCHostAXCtrl), &LIB_GUID>,
//	public IObjectSafetyImpl<CRCHostAXCtrl,INTERFACESAFE_FOR_UNTRUSTED_CALLER|INTERFACESAFE_FOR_UNTRUSTED_DATA>
	public CRCSiteLockImpl<CRCHostAXCtrl>

{
private:
	CInstanceTracker m_instanceTracker;
public:
	CRCHostAXCtrl();
	virtual ~CRCHostAXCtrl();

///DOXYS_OFF
BEGIN_COM_MAP(CRCHostAXCtrl)
	COM_INTERFACE_ENTRY(IObjectSafety)
	COM_INTERFACE_ENTRY(IObjectSafetySiteLock)
	COM_INTERFACE_ENTRY(IRCHostAXCtrl)
	COM_INTERFACE_ENTRY(IMediatorRef)
END_COM_MAP()

DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE | 
	OLEMISC_CANTLINKINSIDE | 
	OLEMISC_INSIDEOUT | 
	OLEMISC_ACTIVATEWHENVISIBLE | 
	OLEMISC_SETCLIENTSITEFIRST
)


BEGIN_PROP_MAP(CRCHostAXCtrl)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()


BEGIN_MSG_MAP(CRCHostAXCtrl)
	TRY_CATCH
	MESSAGE_HANDLER(m_msgFireEventOtherThreadsStarted,FireEventOtherThreadsStarted)
	MESSAGE_HANDLER(m_msgFireEventOtherThreadsStopped,FireEventOtherThreadsStopped)
	MESSAGE_HANDLER(m_msgFireEventOtherThreadsConnecting,FireEventOtherThreadsConnecting)
	CHAIN_MSG_MAP(CComControl<CRCHostAXCtrl>)
	DEFAULT_REFLECTION_HANDLER()
	CATCH_LOG("BEGIN_MSG_MAP(CRCHostAXCtrl)")
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
///DOXYS_ON

protected:
	/// message code of event fire in other thread
	const UINT m_msgFireEventOtherThreadsStarted;
	const UINT m_msgFireEventOtherThreadsStopped;
	const UINT m_msgFireEventOtherThreadsConnecting;
	/// function fire event from other threads. call NotifySessionStart
	LRESULT FireEventOtherThreadsStarted(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	/// function fire event from other threads. call NotifySessionStop
	LRESULT FireEventOtherThreadsStopped(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	/// function fire NotifyConnecting event from other threads
	LRESULT FireEventOtherThreadsConnecting(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	/// A virtual method that notifies session has started.
	/// @param clientId corresponding cliend identifier
	virtual void NotifySessionStarted(const int clientId);
	/// A virtual method that notifies session has stopped and why 
	/// @param ReasonCode stop reason (Stop() called; Remote Stop() called; Stream signaled end-of-file; etc.)
	/// @param clientId corresponding cliend identifier
	virtual void NotifySessionStopped(const int clientId,ESessionStopReason ReasonCode);
	
	

	//TODO what is throw() in method definition
public:	
// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// IRCHostAXCtrl
	//HRESULT OnDraw(ATL_DRAWINFO& di);

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	//HRESULT FinalConstruct(){return S_OK;}
	//void FinalRelease(){}
	
protected:

	/// A callback method that is invoked by the factory to notify the progress 
	/// and status of connection attempt.
	/// @param percentCompleted Percent Completed is an integer value in the range of 0-100 
	/// that represents the connection progress (in terms of steps/actions done).
	/// @param status Status is a text message that describes the current step.
	virtual void NotifyProgress(const int& percentCompleted, const tstring& status);

	///	connecting establish process flag. true:connecting, file: other wise.
	bool m_fConnecting;
	/// Virtual metody, that notifyes connect is complete
	/// @param stream connected stream, null if not connected (NULL -> stream.get() == NULL)
	virtual void ConnectCompletion(boost::shared_ptr<CAbstractNetworkStream> stream);


public:
	/// Create new Remote Control session.
	/// The caller may use this method to create multiple session to a single CRCHostAXCtrl object.
	/// @param userId Identifies the local user.
	/// @param password Used for authenticating the local user with the relay server 
	/// @param peerId Identifies the remote peer.
	/// @param relayServer Specifies the IP address or hostname of the relay server.
	/// @param timeOut Specifies connection timeout in millisecond.
	/// @param id [out,retval] return  unique client ID parameter of new session. The method ara invoked in JavaScript as follow: ... var id; id=RCHostAXCtrl.StartClient("1","bla","2",60000);...
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	/// @remark The method is asynchronous. It establishes connection process and returns control immediately. In connection process the object firing NotifyConnecting() events which notify connect progress. After connection process complete, are fired NotifySessionStart() event if connection established, and NotifySessionStop() otherwise. When StartClient() method are called it return new client session identifier equal -2, after connection success established real client session identifier pass in NotifySessionStart() event. Calling StartClient() method when connection process already started return E_FAIL value.
	STDMETHOD(StartClient)(BSTR userId, BSTR password, BSTR peerId,BSTR relayServer,ULONG timeOut,LONG* id);
	
	/// End Remote Control session with unique client session identifier.
	/// @param id unique client session identifier required ending session. 
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(StopClient)(LONG id);
	
	/// ActiveX events
	__event __interface _IRCHostAXCtrlEvents;
	/// Toggle the boolean State of a specified session Mode.
	/// @param mode mode to toggle state. VIEW_ONLY=0, VISUAL_POINTER=1. See ESessionMode definition for last version velues.
	/// @param state new state for mode, assign true to set specify mode, assign false to reset specify mode.
	/// @param clientId unique client session identifier for which set mode
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(SetSessionMode)(LONG clientId,LONG mode,VARIANT_BOOL state);

	/// Toggles session recording mode.
	/// When enabled, recording takes place into the specified file name.
	/// The specified file create in a hard-coded directory(My Documents), and the file name is cleaned up to prevent creation of files outside this directory (e.g. using relative paths, such as “..\..\filename.ext”). There append extension ".rce" to specified file name.
	/// @param fileName Recording file name only, don't includ path and extension.
	/// @param mode Recording mode, true : start recording, false :	stop recording
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(SetSessionRecording)(BSTR fileName, VARIANT_BOOL mode);

	/// Protect browser window
	/// @param mode If assign true window protected, otherwise the protection are tacked off.
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	STDMETHOD(ProtectWindow)(VARIANT_BOOL mode);

	/// Turn on or off alpha blended and layered windows capturing
	/// @param captureAlphaBlend set this to true if to turn alpha blend capturing on, false otherway
	/// @remark alphablended and layered windows capturing can decrease performance dramatically
	/// so by default it's turned off, and be started only manually as option
	STDMETHOD(SetCaptureAlphaBlend)(VARIANT_BOOL captureAlphaBlend);
};

