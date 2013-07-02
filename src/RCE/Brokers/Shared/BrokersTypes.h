/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  BrokersTypes.h
///
///  Brokers types and enumarations declaration.
///
///  @author Kirill Solovyov @date 20.10.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once

/// destinations types for request sent by JS
enum EBrokerJSRequestDestinations
{
	BJSRD_LOCAL=0,    /// request send to the local side.
	BJSRD_REMOTE      /// request send to the remote side.
};

/// Brokers requests results
enum EBrokerReqeustsResults
{
	BRR_ERROR=0xffffff00,      /// any errors params="[ErrorCode];;[_dstUserId];;[_dstSvcId];;[_params|others]"
	BRR_UNKNOWN=0xffffff01,    /// request type is not unknown
	BRR_BUSY=0xffffff02,       /// request handler busy
	BRR_DECLINED=0,            /// request is declined
	BRR_APPROVED,              /// request is approved
	BRR_RESERVED11,            /// use this at first
	BRR_BPFAILED,              /// BrokerProxy ActiveX object creation failed (for example in case when user push "don't install" button)
	BRR_RESERVED2
};

/// Brokers Requests types
enum EBrokersRequestsTypes
{
	BRT_RESPONSE=0x80000000, /// respons on any request, it mark original request as response via bitwise OR operation (for exapmle: BRT_SERVICE|BRT_RESPONSE - response on service request
	BRT_PING = 0,           /// ping request - blank request. it is used for connection establishment (for example).
	BRT_RESERVED1,          /// 
	BRT_CHAT_MESSAGE,       /// chat message; params="[Message]";
	BRT_SERVICE,            /// service creation request param=srvType, params="[Caption text];;[Message text];;[approve button text];;[decline button text];;[Access Mode]"
	BRT_PROGRESS,           /// progress reqeust param=percent completion params="message"
	BRT_MESSAGEBOX,         /// message box show request param=count buttons params="[Caption text];;[Message text];;[button1 text];;[button2 text];;...;;[buttonN text]"
	BRT_SUBSTREAM,          /// inner substream request, it is used by brokers in substream management
	BRT_INSTALLATION,       /// installation progress reqeuest param percent params="[Message text]"
	BRT_SET_SCRIPTNAME,     /// set script name for script engine params="[Script name]"
	BRT_START_WATCHDOG,     /// starts watchdog timer (now handled by broker) param=pid of process to watch
	BRT_NWL_DISCONNECTED,   /// network layer for some session was disconnected. param = remoteUID: user ID of remote side (to which connection was broken)
	BRT_SERVICE_DESTROYED,  /// service destroyed iformation request param=remote_svcId params="[srvId]" (param=remote_svcId correspond to dstUserId in the request; params="[srvId] correspond to srcUserId)
	BRT_SRV_STATE_CHANGED,  /// service state has been changed (for example DS becom turned ON or OFF). param = new_state_id see EServiceStateMessages, params = "Service name"
	BRT_GET_SERVICE_INFO,   /// get information about session param=reserved params="[userId];;[UserName];;[remoteUserId];;[remoteUserName];;[sId]"
	BRT_GET_SESSION_INFO,   /// get information about session. respond on this request: param=reserved params="[userId];;[UserName];;[remoteUserId];;[remoteUserName]"
	BRT_CONNECTION,         /// connection progress request params="[Message text]"
	BRT_RESPONSE_FAILED,    /// sent response is failed. THIS REQUEST HAS NO RESPONSE. param=[ErrorCode] params="[_dstUserId];;[_dstSvcId];;[_rType];;[_param];;[_params]" i.e. values of sent response
	BRT_MINIMIZE_WIDGET,    /// minimize service widget. no respond. no params
	BRT_SELECT_WIDGET,      /// selects (brings on top) sender service's widget. no respond. no params
	BRT_STOP_SERVICE,       /// asks service to stop. no respond. no params
	BRT_BROKER_STARTED,     /// notification request of new Broker instance started in case using separate process
	BRT_BROKERPROXY_INFO,   /// pass BrokerProxy information to Broker param=[processId] params=[threadId;;hWnd]
	BRT_PROPERTY_LOAD,      /// property load request params=[property name], response params=[property name;;property value]
	BRT_PROPERTY_SAVE,      /// property save request params=[property name;;property value]
	BRT_BROWSER_MINIMIZE,   /// minimize browser request. send to Broker by js
	BRT_RESERVED2           /// 
};

/// remote control access modes
enum ERCAccessMode
{
	RCAM_VIEW_ONLY=0,       /// Prevents input from the VCViewer from being injected into the local Windows
	RCAM_VISUAL_POINTER,    /// Causes mouse movement input from VCViewer to control a visual pointer (similar to a real mouse).  The visual pointer will have a different shape than the real mouse pointer, and it will not be able to control applications.  It will, however, have visual indications of remote Left and Right clicks.
	RCAM_FULL_CONTROL       /// VISUAL-POINTER mode can only be enabled if  VIEW-ONLY mode is also enabled.
};

/// user identifier predefined values
#define B_LOCALSID _T("")//TODO remove
#define BUSERIDPDV_LOCAL _T("LOCAL")      /// sid local side (Request()/Respond() does not send to remote user)
#define BUSERIDPDV_AUTOSET _T("AUTOSET")  /// must be set by request handler (by Broker/VBroker)

/// service identifier predefined values
enum EBrokerSvcIdPredefinedValues
{
	BSVCIDPDV_BROKER=0xffffff00,  /// request sends from/to Broker/VBroker
	BSVCIDPDV_JS,                 /// request sends from/to JS
	BSVCIDPDV_AUTOSET,            /// must be set by request handler  (for service it do by Broker/VBroker)
	BSVCIDPDV_BROKERPROXY,        /// request sends from/to BrokerProxy
	BSVCIDPDV_BROADCAST,          /// broadcast request (send to all LOCAL services)
	BSVCIDPDV_RESERVED1
};

/// request identifier predefined values
enum EBrokerReqeustIdPredefinedValues
{
	BRIDPDV_AUTOSET=0xffffff00,   //must be set by request handler (for service it do by Broker/VBroker)
	BRIDPDV_SERVICE_FIRST=0,      ///set when first service request send through js (jabber)
	BRIDPDV_RESERVED1
};


#define BRT_SERVICE_WAITING_APPROVE _T("Waiting for customer approval")
#define BRT_SERVICE_WAITING_APPROVE_APPLICATION _T("Waiting for application customer approval")
#define BRT_SERVICE_STOPPED_BY_CUSTOMER _T("The service was stopped by the customer")
#define BRT_SERVICE_APPROVE  _T("Approve")
#define BRT_SERVICE_DECLINE  _T("Decline")
#define BRT_SERVICE_DECLINED _T("Customer declined the request")
#define BRT_SERVICE_APPROVED _T("Customer granted permission")
#define BRT_SERVICE_BPFAILED _T("Customer declined the request. %s")
#define BRT_SERVICE_BUSY     _T("Another service is waiting for customer approval or installation. Please try again in a few minutes.")

#define BRT_SERVICE_FIRST_FORMAT _T("Support tools;;To resolve the case ###SUPPORTERNAME### needs to use support tools.;;%s;;%s")

#define BRT_SERVICE_RCFORMAT _T("Desktop Sharing;;%s (<u>%s</u>)%s;;%s;;%s")
#define BRT_SERVICE_RCTEXT _T("###SUPPORTERNAME###  requests permission to access your PC ")
#define BRT_SERVICE_RCVIEWONLY _T("View Only")
#define BRT_SERVICE_RCVISUALPOINTER _T("Visual Pointer")
#define BRT_SERVICE_RCFULLCONTROL _T("Full Control")
#define BRT_SERVICE_RCWALLPAPEROFF _T("<br><span style='font-size:87%'>Note: your desktop background will change for the duration of the session.</span>")
//#define BRT_SERVICE_RCWALLPAPEROFF _T("<br>Note: your desktop background will change for the duration of the session.")

#define BRT_SERVICE_SEFORMAT _T("%s;;%s;;%s;;%s")
#define BRT_SERVICE_SETEXT _T("To help assess your case, ###SUPPORTERNAME### would like to retrieve hardware & software details from your computer")

#define BRT_SERVICE_FAFORMAT _T("File manager;;%s;;%s;;%s")
#define BRT_SERVICE_FATEXT _T("To help solve your case, ###SUPPORTERNAME### would like to be able to send and receive files from and to your computer")

#define BRT_INSTALLATION_EXFORMAT    _T("%s %d%% complete")
#define BRT_INSTALLATION_EXTEXT      _T("%s service is being installed on customer`s computer...")
#define BRT_INSTALLATION_EXENDTEXT   _T("%s service successfully installed on customer`s computer")
#define BRT_INSTALLATION_EXERRORTEXT _T("%s service failed to install on customer`s computer. Code#%d")


/// Available param values for BRT_SRV_STATE_CHANGED request
typedef enum _EServiceStateMessages
{
	ESM_SERVICE_STOPPED				= 0,
	ESM_SERVICE_STARTED				= 1,
	ESM_SERVICE_ACTIVITY_CHANGED	= 2
} EServiceStateMessages;