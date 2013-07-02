// SupportMessengerDlg.cpp implementation
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								SupportMessengerDlg
//
//-------------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// SupportMessengerDlg : implementation of the main dialog window of the SupportMessenger
//-------------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 10/29/2006 05:21:10 PM
// Comments: First Issue
//===========================================================================
#include "stdafx.h"
#include "SupportMessenger.h"
#include "SupportMessengerDlg.h"
#include "TrayNotifyIcon.h"
#include "AidLib/CException/CException.h"
#include <AidLib/Logging/cLog.h> 
#include "Call.h"
#include <Mmsystem.h>
#include <boost/shared_ptr.hpp>
#include "UnicodeConvert.h"

//#include <atlconv.h>

//	urls of supportspace site
#define LINK_SUPPORTSPACE_HOMEPAGE				_T(".supportspace.com")

//	call processing 
#define URL_PICKUP_DIRECT_CUSTOMER_CALL			_T("/accept.s2?username=%1&supportRequestId=%2!d!")
#define URL_PICKUP_CONSULT_CALL					_T("/accept.s2?username=%1&notificationId=%2!d!")

#define URL_DOWNLOAD_UPDATE						_T("/download.s2?username=%1")	//todo 
#define URL_DOWNLOAD_UPDATE_SPRINT4				_T("/rcp/%1")				//todo 
#define URL_SUPPORTER_PROFILE					_T("/mySupporterProfile.s2?username=%1")
#define URL_SUPPORTER_INBOX						_T("/inbox_sc.s2?ju=%1") 
#define URL_SUPPORTER_INBOX_THREADID			_T("/inbox_sc.s2?ju=%1&mid=%2!d!") 
#define URL_SUPPORTER_SESSION_HISTORY			_T("/sessionHistory.s2?ju=%1")
#define URL_SUPPORTER_MY_MESSAGES				_T("/inbox_sc.s2?username=%1")
#define URL_HELP_HELP							_T("/expertHelp.s2")	//todo
#define URL_ONLINE4_FORCUSTOMER					_T("/onlineForCustomer.s2?ju=%1") 

#define URL_DOWNLOAD_MSI_NAME					_T("SupportSpace_Platform_Setup.exe")  //todo 
//	client/server protocol messages
#define PROTOCOL_MSG_GET_PENDINGCALLS			_T("getPendingSupportRequests")
#define PROTOCOL_MSG_GET_EXPERTTOOLS_VERSION	_T("getActivexVersion")
#define PROTOCOL_MSG_GET_UNREAD_MESSAGES		_T("getUnreadSupportMessages")
#define PROTOCOL_MSG_CUSTOM_REPLY_TO_CUSTOMER	_T("customeReplyToCustomer")
#define PROTOCOL_MSG_IM_CHANNEL_QUALITY_TEST	_T("testIMChannelQuality")
#define PROTOCOL_MSG_CLOSE_PENDING_CALL			_T("closePendingCall")

//
//	DHTML interface calls
//
#define DHTML_INTERFACE_LOGIN_INFO				_T("shuttle_login_info")
#define DHTML_INTERFACE_LOGOUT					_T("shuttle_logout")
#define DHTML_INTERFACE_SETTINGS				_T("shuttle_settings")
#define DHTML_INTERFACE_LOGIN_START				_T("shuttle_login_start")
#define DHTML_INTERFACE_LOGIN_STATE				_T("shuttle_login_state")
#define DHTML_INTERFACE_HELP_ABOUT				_T("shuttle_help_about")
#define DHTML_INTERFACE_FIREWALL_TEST_RESULT	_T("shuttle_connection_test")
#define DHTML_INTERFACE_OPEN_WINDOW				_T("shuttle_open_window")
#define DHTML_INTERFACE_SETTINGS_CLOSE			_T("shuttle_settings_close")

//
//	default login password - we don't want to send remembered password to login page
//  todo in the future server side will validate password lenght and we will use this 
#define DEFAULT_LOGINPAGE_PASSWORD	_T("*#@*$&_()[.]")

// SignIn state
typedef enum _eSignInState
{
	// 
	SIGNIN_STATE_SIGNEDOUT = 0,
	// initialisation of connection TLS, etc
	SIGNIN_STATE_INIT = 1,
	// 
	SIGNIN_STATE_CONNECTING = 2,
	//
	SIGNIN_STATE_AUTHENTICATION = 3,
	//
	SIGNIN_STATE_VERSION_VALIDATION = 4,
	//
	SIGNIN_STATE_SIGNNED_IN = 5,

} eSignInState;

// Structure to store information field properties
typedef struct _sSignInField
{
	// Field type
	eSignInState		m_type;
	// Information string
	TCHAR*				m_iSignInFieldString;

} sSignInField;

// static array 
const static sSignInField SigninStatePairs[] =
{
	{ SIGNIN_STATE_SIGNEDOUT,			_T("0") },
	{ SIGNIN_STATE_INIT,				_T("1") },
	{ SIGNIN_STATE_CONNECTING,			_T("2") },
	{ SIGNIN_STATE_AUTHENTICATION,		_T("3") },
	{ SIGNIN_STATE_VERSION_VALIDATION,	_T("4") },
	{ SIGNIN_STATE_SIGNNED_IN,			_T("5") },
};

//
#define WM_RELOAD_SETTINGS			 WM_USER + 500
#define WM_SETTINGS_DIALOG_CLOSE	 WM_USER + 501

#define	MAX_NUMBER_OF_ALERTS_TO_SHOW 5
#define MAX_NUMBER_OF_SUB_MENUS		 5 
#define MAX_CONNECT_ATTEMPT			 3000000
#define MAX_USERNAME_LEN			 512

#define TIMEOUT_FOR_CONNECT_ATTEMPT  10000

#define RECONNECT_TIMER_ID			 0x1001
#define RECONNECT_TIMER_TIMEOUT		 10000

#define DEFAULT_STATUS_ITEM_WIDTH  16
#define DEFAULT_STATUS_ITEM_HEIGTH 16

#define MIN_UI_WIDTH	280
#define MIN_UI_HEIGHT	500

typedef boost::shared_ptr<CNodeSendMessage> CNodeSendMessagePtr;

//
//	mapping of Messenger's GUI defined status and xmpp protocol Status and ShowMessage 
//
typedef std::pair<SupporterStatus, CPresence> StatusPresencePairs;
//
//	define pairs for static array for mapping of SupportMessenger statues and IM statuses
//	for each SupportMessenger status there is corespondent pair of IM status amd IM user defined message
//
const static StatusPresencePairs  arrStatusPresencePairs[] = {
   	StatusPresencePairs(StatusAvailible, CPresence(PresenceAvailable, _T("PresenceAvailable") )),
	StatusPresencePairs(StatusNotAcceptingCalls, CPresence(PresenceUnavailable, _T("PresenceUnavailable") )),
	StatusPresencePairs(StatusBusy, CPresence(PresenceDnd, _T("PresenceDnd") )),
	StatusPresencePairs(StatusAway, CPresence(PresenceAway, _T("PresenceAway") )),
	StatusPresencePairs(StatusOffline, CPresence(PresenceUnavailable, _T("PresenceUnavailable") )),
	StatusPresencePairs(StatusOnnline4Customer, CPresence(PresenceDnd, _T("PresenceOnline4Customer") ))
};

//
//	define pointer to Callback function of the class CSupportMessengerDlg
//
typedef HRESULT (CSupportMessengerDlg::*pfnCallbackMsngrDlg)(IHTMLElement *pElement);
//
//	define pair used for mapping string and callbacks
//
typedef std::pair<CString, pfnCallbackMsngrDlg> DHTML_CUSTOM_EVENT_MSNGR_DLG;
//
//	init pairs map of calbacks
//
static const DHTML_CUSTOM_EVENT_MSNGR_DLG callBacksPairsMsngrDlg[] =
{
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("LoginPageStart"),CSupportMessengerDlg::OnLoginPageStart),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("pageLoaded"),CSupportMessengerDlg::OnPageLoaded),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("FlagChanged"),CSupportMessengerDlg::OnFlagChanged),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("ManageFlags"),CSupportMessengerDlg::OnManageFlags),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("FwdToContact"),CSupportMessengerDlg::OnFwdToContact),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("ManageContacts"),CSupportMessengerDlg::OnManageContacts),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("Snooze"),CSupportMessengerDlg::OnSnooze),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("GoToCalender"),CSupportMessengerDlg::OnGoToCalender),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("SendReply"),CSupportMessengerDlg::OnSendReply),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("SendCustomReply"),CSupportMessengerDlg::OnSendCustomReply),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("GoToManageReplies"),CSupportMessengerDlg::OnGoToManageReplies),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("CloseMessenger"),CSupportMessengerDlg::OnCloseMessenger),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("MinMessenger"),CSupportMessengerDlg::OnMinMessenger),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("pickup"),CSupportMessengerDlg::OnInboxPickupCall),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("SettingsApp"),CSupportMessengerDlg::OnSettingsApp),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("RestoreApp"),CSupportMessengerDlg::OnRestoreApp),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("MinimizeApp"),CSupportMessengerDlg::OnMinimizeApp),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("CloseApp"),CSupportMessengerDlg::OnCloseApp),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("NewCallAdded"),CSupportMessengerDlg::OnNewCallAdded),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("requestStatusChanged"),CSupportMessengerDlg::OnRequestStatusChanged),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("NewConsultRequestAdded"),CSupportMessengerDlg::OnNewConsultRequestAdded),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("consultStatusChanged"),CSupportMessengerDlg::OnConsultStatusChanged),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("SendCustomReply"),CSupportMessengerDlg::OnSendCustomReply),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("updateVersion"),CSupportMessengerDlg::OnUpdateVersion),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("SettingsOk"),CSupportMessengerDlg::OnSettingsOk),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("SettingsClose"),CSupportMessengerDlg::OnSettingsClose),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("closeMessenger"),CSupportMessengerDlg::OnSettingsClose),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("openLink"),CSupportMessengerDlg::OnOpenLink),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("connectionTest"),CSupportMessengerDlg::OnConnectionTest),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("addDhtmlLog"),CSupportMessengerDlg::OnAddDhtmlLog),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("SetStargateStatus"),CSupportMessengerDlg::OnSetStargateStatus),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("showMessageBox"),CSupportMessengerDlg::OnShowMessageBox),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("StargateOfflineMsgNotification"),CSupportMessengerDlg::OnStargateOfflineMsgNotification)
};
//
//	define map of callbacks
//
const static std::map<CString, pfnCallbackMsngrDlg> mapCallbacksMsngrDlg(callBacksPairsMsngrDlg, callBacksPairsMsngrDlg + sizeof(callBacksPairsMsngrDlg)/sizeof(callBacksPairsMsngrDlg[0]));
typedef std::map<CString, pfnCallbackMsngrDlg>::const_iterator MapCallbacksMsngrDlgIterator;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_DHTML_EVENT_MAP(CSupportMessengerDlg)
//main entry point for DHTML predefined callbacks
DHTML_EVENT_ONCLICK(_T("eCbkEventsHandler"), OnCbkEventsHandlerClickedEvent)
//ban the selecting action
DHTML_EVENT_TAG_ALL(DISPID_HTMLELEMENTEVENTS_ONSELECTSTART, OnHtmlSelectStart)
//handle all the dragging action
DHTML_EVENT_TAG_ALL(DISPID_HTMLELEMENTEVENTS_ONDRAGSTART, OnHtmlDragStart)
//handle all the context menu
//DHTML_EVENT_TAG_ALL(DISPID_HTMLELEMENTEVENTS_ONCONTEXTMENU, OnHtmlContextMenu)
END_DHTML_EVENT_MAP()

BEGIN_MESSAGE_MAP(CSupportMessengerDlg , CAppBar)
	ON_WM_PAINT()

	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_MESSAGE(WM_TRAYNOTIFY, OnTrayNotification)

	ON_MESSAGE(WM_ALERTDLG_MISSEDCALLS_CLOSE, OnAlertDlgMissedCallsClose)
	
	ON_MESSAGE(WM_ALERTDLG_NEW_CALL_EXPAND, OnAlertDlgNewCallExpand)
	ON_MESSAGE(WM_ALERTDLG_NEW_CALL_PICKUP, OnAlertDlgNewCallPickUp)
	ON_MESSAGE(WM_ALERTDLG_NEW_CALL_CLOSE, OnAlertDlgNewCallClose)

	ON_MESSAGE(WM_ALERTDLG_UPDATE_PICKUP, OnAlertDlgUpdatePickUp)
	ON_MESSAGE(WM_ALERTDLG_UPDATE_CLOSE, OnAlertDlgUpdateClose)

	ON_MESSAGE(WM_ALERTDLG_OFFLINE_MSG_PICKUP, OnAlertDlgOfflineMsgPickUp)
	ON_MESSAGE(WM_ALERTDLG_OFFLINE_MSG_CLOSE, OnAlertDlgOfflineMsgClose)

	ON_MESSAGE(WM_ALERTDLG_INFO_MSG_HELP, OnAlertDlgInfoMsgHelp)
	ON_MESSAGE(WM_ALERTDLG_INFO_MSG_CLOSE, OnAlertDlgInfoMsgClose)

	ON_MESSAGE(WM_ONLINE4CUSTOMER_CLOSE, OnOnline4CustomerClose)
	ON_MESSAGE(WM_ONLINE4CUSTOMER_SUBMIT, OnOnline4CustomerSubmit)
	
	ON_MESSAGE(WM_ALERTDLG_MISSEDCALLS_PICKUP, OnAlertDlgMissedCallsPickUp)
	ON_MESSAGE(WM_WORKBENCH_CLOSE, OnWorkBenchClose)

	ON_MESSAGE(WM_ACTIVITY_HANDLER_INACTIVE, OnActivityHandlerInActive)
	ON_MESSAGE(WM_ACTIVITY_HANDLER_BACK_ACTIVE, OnActivityHandlerBackActive)

	ON_MESSAGE(WM_UPDATE_COMPLETED, OnUpdateCompleted)
	ON_MESSAGE(WM_UNINSTALL_COMMAND, OnUninstallStarted)
	ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadCast)
		
	ON_MESSAGE(WM_IM_CONNECTED, OnIMConnected)
	ON_MESSAGE(WM_IM_NEWCALL, OnIMNewCall)
	ON_MESSAGE(WM_IM_TLSCONNECTED, OnIMTLSConnected)
	ON_MESSAGE(WM_IM_DISCONNECT, OnIMDisconnect)
	ON_MESSAGE(WM_IM_CONNECT_FAILED, OnIMConnectFailed)
	ON_MESSAGE(WM_IM_CONNECTING, OnIMConnecting)
	
	ON_MESSAGE(WM_IM_CHANNEL_QUALITY_MONITOR_TEST_TIME, OnIMChannelQualityTestTime)
	ON_MESSAGE(WM_IM_CHANNEL_QUALITY_MONITOR_ISSUE_DETECTED, OnIMChannelQualityIssueDetected)
	ON_MESSAGE(WM_IM_CHANNEL_QUALITY_MONITOR_IMPROVEMENT_DETECTED, OnIMChannelQualityImprovementDetected)
	
	ON_COMMAND(ID_POPUP_SIGNIN, &CSupportMessengerDlg::OnPopupSignin)
	ON_COMMAND(ID_POPUP_SINGOUT, &CSupportMessengerDlg::OnPopupSingout)
	ON_COMMAND(ID_POPUP_EXIT, &CSupportMessengerDlg::OnPopupExit)
	ON_COMMAND(ID_POPUP_OPEN, &CSupportMessengerDlg::OnPopupOpen)
	ON_COMMAND(ID_POPUP_SETTINGS,&CSupportMessengerDlg::OnSettings)
	ON_COMMAND(ID_DOCK_RIGHT, &CSupportMessengerDlg::OnDockRight)
	ON_COMMAND(ID_DOCK_LEFT, &CSupportMessengerDlg::OnDockLeft)
	ON_COMMAND(IDI_MYSTATUS_AVAILIBLE, &CSupportMessengerDlg::OnStatusAvailible)
	ON_COMMAND(IDI_MYSTATUS_AWAY, &CSupportMessengerDlg::OnStatusAway)
	ON_COMMAND(IDI_MYSTATUS_BUSY, &CSupportMessengerDlg::OnStatusBusy)
	ON_COMMAND(IDI_MYSTATUS_ONNLINEFORCUSTOMER, &CSupportMessengerDlg::OnStatusOnnlineForCustomer)
	ON_COMMAND(ID_LINKS_SUPPORTSPACEHOMEPAGE, &CSupportMessengerDlg::OnLinksSupportspacehomepage)
	ON_COMMAND(ID_HELP_ABOUT, &CSupportMessengerDlg::OnHelpAbout)
	ON_COMMAND(ID_LINKS_MYPROFILE, &CSupportMessengerDlg::OnLinksMyprofile)
	ON_COMMAND(ID_HELP_HELP, &CSupportMessengerDlg::OnHelpHelp)
	ON_COMMAND(ID_LINKS_EXPERTFORUM, &CSupportMessengerDlg::OnLinksExpertForum)
	ON_COMMAND(ID_LINKS_SESSIONSHISTORY, &CSupportMessengerDlg::OnLinksSessionshistory)
	ON_COMMAND(ID_LINKS_EXPERTPORTAL, &CSupportMessengerDlg::OnLinksExpertportal)
	ON_COMMAND(ID_LINKS_MYMESSAGES, &CSupportMessengerDlg::OnLinksMymessages)

	ON_WM_INITMENUPOPUP()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_DRAWITEM()
    ON_WM_MEASUREITEM()

	ON_WM_TIMER()
	
END_MESSAGE_MAP()

CSupportMessengerDlg::CSupportMessengerDlg (CWnd* pParent)
: CAppBar(CSupportMessengerDlg::IDD, CSupportMessengerDlg::IDH, pParent)
{
TRY_CATCH 
	//Init Icons From Resource 
	m_srtIcons.m_hMainApp = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_srtIcons.m_Available = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_MYSTATUS_AVAILIBLE), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
	m_srtIcons.m_Busy = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_MYSTATUS_BUSY), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
	m_srtIcons.m_Away = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_MYSTATUS_AWAY), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
	m_srtIcons.m_Offline = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_OFFLINE), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
	m_srtIcons.m_NewCalls = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_NEW_CALLS), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
	m_srtIcons.m_NewCallsAnimated[0] = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_NEWCALLANIMATED1), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
	m_srtIcons.m_NewCallsAnimated[1] = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_NEWCALLANIMATED2), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
	m_srtIcons.m_NewCallsAnimated[2] = m_srtIcons.m_NewCalls;//the same icon used for animation and new call messge indicator
	m_srtIcons.m_UpdateRequired = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_UPDATE_REQUIRED), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
	m_srtIcons.m_SignedOut = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_OFFLINE), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);	//todo special icon for SignedOut

	m_srtIcons.m_ConnectingAnimated[0] = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_CONNECTINGANIMATED1), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
	m_srtIcons.m_ConnectingAnimated[1] = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_CONNECTINGANIMATED2), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);

	m_srtIcons.m_OnnlineForCustomer = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_MYSTATUS_ONNLINEFORCUSTOMER), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);

	m_srtIcons.m_AvailableLimitedCon  = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_MYSTATUS_AVAILIBLE_LIMITED_CON), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);	
	m_srtIcons.m_BusyLimitedCon = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_MYSTATUS_BUSY_LIMITED_CON), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
	m_srtIcons.m_AwayLimitedCon = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_MYSTATUS_AWAY_LIMITED_CON), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
	m_srtIcons.m_OfflineLimitedCon  = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_MYSTATUS_OFFLINE_LIMITED_CON), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
	m_srtIcons.m_OnnlineForCustomerLimitedCon  = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_MYSTATUS_ONNLINEFORCUSTOMER_LIMITED_CON), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
	
	//todo Sprint 4-5 make function that load string from string table depending on the language
	//Load Strings from String Table
	//m_strInfoError.m_sInfoUnattenedCallsInInbox.LoadString(IDS_INFO_UNATTENDED_CALLS_IN_INBOX);
	
	// Init state of the messenger 
	m_MessengerState.eTrayMenuOpenReason = None;
    m_MessengerState.eSupporterSelectedStatus = StatusAvailible; //Default only will be taken from GUI or Registry if exists
	m_MessengerState.bUserSelectedSignIn = FALSE;
	m_MessengerState.dwIMConnectAttempt = 0;	// for retry policy inside main dialog we would like to use this counter? todo - wait version from gloox 
	m_MessengerState.bStatusChangedOnIdle = FALSE;
	m_MessengerState.bWaitTillAllEventsClosed  = FALSE;
	m_MessengerState.bUpdateRequired = FALSE;		
	m_MessengerState.bAnimateTray = FALSE;
	m_MessengerState.dwNewCallsCounter = 0;
	m_MessengerState.dwInSessionCallsCounter = 0;
	m_MessengerState.bStatusChangedOnMaxHandled = FALSE;
	m_MessengerState.dwInSessionConsultCallCounter = 0;
	m_MessengerState.dwNewConsultCall = 0;
	m_MessengerState.bPowerStatusSuspended = FALSE;
	
	m_MessengerState.dwMissedCallsCounter = 0;
	m_MessengerState.bStatusChangedOnMissedCalls = FALSE;
	m_MessengerState.bStatusChangedOnStargateBusy = FALSE;

	m_MessengerState.bIMChannelQualityIssueDetected = FALSE;

	// Keep local data
	m_pcSupporterLocalData = NULL;
	// Update notification alert
	m_pcAlertDlgUpdate = NULL;
	// Missied calls notification alert
	m_pcAlertDlgMissedCalls = NULL;
	// Workbench Dlg
	m_pWorkbenchDlg = NULL;
	// Online4Customer Dlg 
	m_cOnline4CustomerDlg = NULL;

	m_lFreePosition = 0;

	//	Load LastLogiedInEntry
	m_cMessengerLocalData.getLastLogiedInEntry()->Load();

	//
	//	load data required to be presented in login window
	//
	LoadDHTMLLoginDialogData();
	
	Create(IDD, pParent); // Create modeless dialog - trick for Tray icon based on CDialog not on CFrame

CATCH_LOG(_T("CSupportMessengerDlg ::CSupportMessengerDlg"))
}

CSupportMessengerDlg::~CSupportMessengerDlg()
{
TRY_CATCH

	if(m_pcSupporterLocalData!=NULL)
		delete m_pcSupporterLocalData;

CATCH_LOG(_T("CSupportMessengerDlg::~CSupportMessengerDlg"))
}

void CSupportMessengerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}
//
//	You cannot change the state of a menu item from its command 
//	user-interface handler if the menu is attached to a dialog box in Visual C++
//  http://support.microsoft.com/default.aspx/kb/242577
//  http://www.codeproject.com/useritems/ICON_menus.asp?forumid=362424&exp=0&select=1870148&df=100#xx1870148xx
void CSupportMessengerDlg::OnInitMenuPopup(CMenu* pMenu, UINT nIndex, BOOL bSysMenu)
{
TRY_CATCH 

    Log.Add(_MESSAGE_,_T(__FUNCTION__) _T(": %#0x"), pMenu->GetSafeHmenu());
	CDialog::OnInitMenuPopup(pMenu, nIndex, bSysMenu);

    if (bSysMenu)
    {
        pMenu = GetSystemMenu(FALSE);
    }

	//
	//	Customize standard menu items with icons
	//
    MENUINFO mnfo;
    mnfo.cbSize = sizeof(mnfo);
    mnfo.fMask = MIM_STYLE;
    mnfo.dwStyle = MNS_CHECKORBMP | MNS_AUTODISMISS;
    pMenu->SetMenuInfo(&mnfo);

    MENUITEMINFO minfo;
    minfo.cbSize = sizeof(minfo);

    for (UINT pos=0; pos<pMenu->GetMenuItemCount(); pos++)
    {
        minfo.fMask = MIIM_FTYPE | MIIM_ID;
        pMenu->GetMenuItemInfo(pos, &minfo, TRUE);

        HICON hIcon = GetIconForItem(minfo.wID);

        if (hIcon && !(minfo.fType & MFT_OWNERDRAW))
        {
            Log.Add(_MESSAGE_,_T("replace for id=%u"), (WORD)minfo.wID);

            minfo.fMask = MIIM_FTYPE | MIIM_BITMAP | MIIM_DATA;
            minfo.hbmpItem = HBMMENU_CALLBACK;
            minfo.fType = MFT_STRING;

            pMenu->SetMenuItemInfo(pos, &minfo, TRUE);
        }
        else
		{
			//Log.Add(_T("keep for id=%u\n"), (WORD)minfo.wID);
		}
//      ::DestroyIcon(hIcon); // we use LR_SHARED instead
    }
	//
	//	If the index of the pop-up menu is 0 in the main menu.The same condition is if(nIndex == 0)
	//
	if( m_TrayIcon.GetMenu().GetSubMenu(0)==pMenu)
		CustomizeTrayMenu(pMenu);
	
CATCH_LOG(_T("CSupportMessengerDlg ::OnInitMenuPopup"))
}

// We scan the menus as they are to be displayed, and add a flag that the item bitmap should be owner-drawn.
// If the resorce file provides an icon for this item. The bitmaps are extracted from the icons. 
// The WM_MEASUREITEM message only asks for the size of the bitmap. 
// Note that all styles like grayed, default, etc. are still available. 
// We set the menu style to MNS_CHECKORBMP purely for aesthetical reasons. 
// But if some of these items with attached icons are checked, the check mark will override the custom 
// drawing callback. On the other hand, the presented approach may be easily generalized to 
// display custom colorful check signs. 
// Some words about menu bars
// To add the images next to some menu items, as the ones you see on the screenshot, 
// you simply add icons to your resources. 
// The icon ID should be the same as the menu ID. That's all. 
// It is the icon's responsibility to render itself nicely. 
// The size doesn't matter.
HICON CSupportMessengerDlg::GetIconForItem(UINT itemID)
{
	HICON hIcon = 0;
TRY_CATCH

	hIcon = (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(itemID), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);

	if(!hIcon)
	{
		CString sItem; // look for a named item in resources
		CMenu*  hPopUpMenu = m_TrayIcon.GetMenu().GetSubMenu(0);
		hPopUpMenu->GetMenuString(itemID, sItem, MF_BYCOMMAND);
		//GetMenu()->GetMenuString(itemID, sItem, MF_BYCOMMAND);
		sItem.Replace(_T(' '), _T('_')); // cannot have resource items with space in name

		if (!sItem.IsEmpty())
			hIcon = (HICON)::LoadImage(::AfxGetResourceHandle(), sItem, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
	}

CATCH_LOG(_T("CSupportMessengerDlg ::GetIconForItem"))
	return hIcon;  
}

void CSupportMessengerDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpdis)
{
TRY_CATCH

    if((lpdis==NULL)||(lpdis->CtlType != ODT_MENU))
    {
        CDialog::OnDrawItem(nIDCtl, lpdis);
        return; //not for a menu
    }

    bool useSelectedColor = true;

    if(lpdis->rcItem.left != 2)
    {
        lpdis->rcItem.left -= (lpdis->rcItem.left - 2);
        lpdis->rcItem.right -= 60;
        if (lpdis->itemState & ODS_SELECTED)
        {
            lpdis->rcItem.left++;
            lpdis->rcItem.right++;
        }
        useSelectedColor = false;
    }

    Log.Add(_MESSAGE_,_T("draw menu item: %u %s in (%d,%d,%d,%d)"), (WORD)lpdis->itemID, (lpdis->itemState & ODS_SELECTED)?_T("selected"):_T(""), lpdis->rcItem.left, lpdis->rcItem.top, lpdis->rcItem.right, lpdis->rcItem.bottom);

    //choose text colors
    if(useSelectedColor)
    {
        if (lpdis->itemState & ODS_SELECTED)
        {
            COLORREF crText;
            if (lpdis->itemState & ODS_GRAYED)
                crText = ::SetTextColor(lpdis->hDC, ::GetSysColor(COLOR_GRAYTEXT)); //RGB(128, 128, 128));
            else
                crText = ::SetTextColor(lpdis->hDC, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
            ::SetBkColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
        }
        // draw the background (if highlighted)
        ::ExtTextOut(lpdis->hDC, 0, 0, ETO_CLIPPED|ETO_OPAQUE, &(lpdis->rcItem), NULL, 0, (LPINT)NULL);
    }

    HICON hIcon = GetIconForItem(lpdis->itemID);
    if(hIcon)
    {
        ICONINFO iconinfo;
        ::GetIconInfo(hIcon, &iconinfo);

        BITMAP bitmap;
        ::GetObject(iconinfo.hbmColor, sizeof(bitmap), &bitmap);

        ::DrawIconEx(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, hIcon, bitmap.bmWidth, bitmap.bmHeight, 0, NULL, DI_NORMAL);
//      ::DestroyIcon(hIcon); // we use LR_SHARED instead
    }

CATCH_LOG(_T("CSupportMessengerDlg ::OnDrawItem"))
}

void CSupportMessengerDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpmis)
{
TRY_CATCH

    if((lpmis==NULL)||(lpmis->CtlType != ODT_MENU))
    {
        CDialog::OnMeasureItem(nIDCtl, lpmis); //not for a menu
        return;
    }

	lpmis->itemWidth = DEFAULT_STATUS_ITEM_WIDTH;
    lpmis->itemHeight = DEFAULT_STATUS_ITEM_HEIGTH;

    HICON hIcon = GetIconForItem(lpmis->itemID);

    if(hIcon)
    {
        ICONINFO iconinfo;
        ::GetIconInfo(hIcon, &iconinfo);

        BITMAP bitmap;
        ::GetObject(iconinfo.hbmColor, sizeof(bitmap), &bitmap);

        lpmis->itemWidth = bitmap.bmWidth;
        lpmis->itemHeight = bitmap.bmHeight;

		Log.Add(_MESSAGE_,_T(__FUNCTION__) _T(": %d %dx%d ==> %dx%d"), (WORD)lpmis->itemID, bitmap.bmWidth, bitmap.bmHeight, lpmis->itemWidth, lpmis->itemHeight);
    }

CATCH_LOG(_T("CSupportMessengerDlg ::OnMeasureItem"))
}
// CSupportMessengerDlg  message handlers
BOOL CSupportMessengerDlg::OnInitDialog()
{
TRY_CATCH 

	CAppBar::OnInitDialog();
	// Set the icon for this dialog.  The framework does this automatically
	// when the application's main window is not a dialog
	SetIcon(m_srtIcons.m_hMainApp, TRUE);		// Set big icon
	SetIcon(m_srtIcons.m_hMainApp, FALSE);		// Set small icon

	//todo take this from configuration
	//m_cSettings.ReadConnectionInfo();

	switch(theApp.m_cSettings.m_eGUIlocation)
	{
	case GuiLocationResource:
		m_sDestUrl.FormatMessage(_T("res://%1!s!%2!s!.exe/%3!d!"), 
			theApp.m_sApplicationPath, theApp.m_pszExeName, IDR_HTML_INDEX);
		LoadFromResource(IDR_HTML_INDEX);
		break;
	case GuiLocationFile: //only for test GUI purposes supported 
		m_sDestUrl.AppendFormat(_T("%s\\index.html"), theApp.m_cSettings.m_sGUIlocationFilePath);
		Navigate(m_sDestUrl);
		break;
	case GuiLocationURL:
		break;
	default:
		break;
	}

	//
	//	uninstall need Caption, but we don't need border
	//	caption added in resource file and border removed here
	//
	LONG nOldStyle = ::GetWindowLong(this->m_hWnd, GWL_STYLE);
	LONG nNewStyle = nOldStyle & ~WS_BORDER;
	::SetWindowLong(this->m_hWnd, GWL_STYLE, nNewStyle);


	//	Initiate Activity handler 
	m_cActivityHandler.Init(this);

	//	Init AppBar 
	InitAppBar();

	//
	//	Init Tray icon 
	if(!m_TrayIcon.Create(this, IDR_TRAY_MENU, (CString)MAKEINTRESOURCE(IDS_APP_FULL_NAME), m_srtIcons.m_hMainApp, WM_TRAYNOTIFY, IDR_TRAY_MENU))
	{
		Log.Add(_ERROR_, _T("Failed to create tray icon %d"), GetLastError());
		//ExitInstance(); //TODO:anatoly
	}
	
	//	Just modify icons 
	SetConnectionState(stateDisconnected);

CATCH_LOG(_T("CSupportMessengerDlg ::OnInitDialog"))	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSupportMessengerDlg::OnPaint()
{
	if(IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_srtIcons.m_hMainApp);
	}
	else
	{
		CDialog::OnPaint();
	}

	CDialog::OnPaint();
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSupportMessengerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_srtIcons.m_hMainApp);
}

//Delegate all the work back to the default implementation in CTrayNotifyIcon
LRESULT CSupportMessengerDlg::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
//TRY_CATCH
	CString str; 

	switch(lParam)
	{
		case WM_USER + 4: //event happens when close of ballon clicked
			str = m_TrayIcon.GetBalloonText();
			break;

		case WM_USER + 5: //event happens when body of ballon clicked
			str = m_TrayIcon.GetBalloonText();
			if( str.Compare( (CString)MAKEINTRESOURCE(IDS_TRAY_ICON_TOOLTIP_LIMITED_CONNECTION))==0)
			{
				//todo open url with help about limit connectivity 
			}
			break;

	    case WM_RBUTTONDOWN:	
			//	set flag to indicate that opened by WM_RBUTTONDOWN
			//	then OnInitMenuPopup will remove or add some fields in the menu
			m_MessengerState.eTrayMenuOpenReason = TrayIcon;
			m_TrayIcon.OnTrayNotification(wParam, lParam);
			break;
		case WM_LBUTTONDBLCLK: 
			if(m_MessengerState.eConnectionState==stateDisconnected)
			{
				ShowLogoutWindow(NULL);
				break;
			}

			if(m_MessengerState.bUpdateRequired==TRUE)
			{
				OnUpdateRequired();
				break;
			}

			if(wParam == OPEN_ABOUT_WINDOW)
			{
				OnHelpAbout();
				break;
			}
		
			// default bahaviour is open tray menu
			m_MessengerState.eTrayMenuOpenReason = TrayIcon;
			m_TrayIcon.OnTrayNotification(wParam, lParam);//TODO for version without UI and Open
			break;
		default:
			m_TrayIcon.OnTrayNotification(wParam, lParam);
			break;
	}

//CATCH_LOG(_T("CSupportMessengerDlg::OnTrayNotification"))
	return 0L;
}

LRESULT CSupportMessengerDlg::OnAlertDlgNewCallExpand(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 

	long lUid = (long)lParam;

	//	todo - this may also call JS function to put focus on the Call with specified UID
	CloseAllAlerts();

	// the same bahaivour like implemented in OnPopupOpen expected
	OnPopupOpen(); 

CATCH_LOG(_T("CSupportMessengerDlg::OnAlertDlgNewCallExpand"))
	return 0L;
}

LRESULT CSupportMessengerDlg::OnAlertDlgNewCallPickUp(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 

	CCall* pCall = (CCall*)lParam;
	long  lUid = 0;
	long  lWorkflowID= 0;
	eCallType callType;
	tstring sCustomerDisplayName;

	if(pCall!=NULL)
	{
		if (m_MessengerState.eSupporterSelectedStatus != StatusOnnline4Customer 
			&& m_MessengerState.eSupporterSelectedStatus != StatusAvailible)
		{
			m_MessengerState.eSupporterSelectedStatus = StatusAvailible;
			IMSendUpdateStatus (m_MessengerState.eSupporterSelectedStatus);
		}

		if (m_MessengerState.bStatusChangedOnIdle == TRUE)
		{
			OnActivityHandlerBackActive (NULL, NULL);
		}

		lUid = pCall->getUid();	
		callType = pCall->getCallType();
		lWorkflowID = pCall->getWorkflowID();
		sCustomerDisplayName = pCall->getCustomerDislpayName();
		DoPickUp(lUid, callType, sCustomerDisplayName.c_str());
		CloseAlert(lUid);
		delete pCall;
	}

	m_MessengerState.bAnimateTray = FALSE;//todo Stop animation? - not defined in PRD
	Log.Add(_MESSAGE_, _T("CSupportMessengerDlg::OnAlertDlgNewCallPickUp UpdateTrayIcon"));
	UpdateTrayIcon(NewCallsAnimationStoped);

CATCH_LOG(_T("CSupportMessengerDlg::OnAlertDlgPickUp"))
	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// "http://supportspace.com:8080/stargate/responseOnSupportRequest/
//  accept.s2?username=<jabberUserName>&supportRequestId=<supportRequestId>" 
//
//
//  When an expert accept a consult request the following url should be sent: 
//  "http://supportspace.com:8080/stargate/responseOnConsultRequest/
//  accept.s2?username=<jabberUserName>&notificationId=<notificationId>" 
//  jabberUserName - the jabber username of the expert (usually it is the hashed username) 
//  notificationId - the id of the consult notification that is being accepted 
//  the above url opens the expert desktop with the consult widget opened and ready for chat with the expert that requested the consult 
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CSupportMessengerDlg::DoPickUp(long lUid, eCallType callType, CString sCustomerDisplayName)
{
TRY_CATCH 

	CString sOtpUrlPart;	
	CString	sFullUrl;
	CString	sUid;

	//todo ask PM if bPlaySoundUponConnectingToCustomer may be in case of Consult also
	if(m_pcSupporterLocalData->getSettings()->bPlaySoundUponConnectingToCustomer == TRUE)
	{
		if(PlaySound(MAKEINTRESOURCE(IDW_WAVE_ONPICKUP),::AfxGetResourceHandle(),SND_RESOURCE | SND_ASYNC)==FALSE)
			Log.Add(_ERROR_, _T("PlaySound failed %d"), GetLastError());
	}

	Log.Add(_WARNING_, _T("In DoPickUp. lUid = %u"), lUid);

	// todo option 0

	switch(callType)
	{
	case CustomerDirectCall:
		sOtpUrlPart.FormatMessage(URL_PICKUP_DIRECT_CUSTOMER_CALL,
			((tstring)m_MessengerState.sCurrentSupporterCrypt).c_str(), lUid);
		sFullUrl = theApp.m_cSettings.m_sBaseUrlPickUp + sOtpUrlPart;
		//
		//
		//
		if(theApp.m_cSettings.m_dwWorkbecnOpenMode==1)
		{
			Log.Add(_WARNING_, _T("WorkbecnOpenMode is set to 1. Open URL"));
			m_cUrlOpener.Open(sFullUrl);
			break;
		}

		//
		// m_dwWorkbecnOpenMode is 0 default is separate process with IEWebControl
		//
		theApp.LauchWorkbenchProcess(sFullUrl, callType, sCustomerDisplayName, lUid);
		break;
	case ConsultCall:
		sOtpUrlPart.FormatMessage(URL_PICKUP_CONSULT_CALL, 
			((tstring)m_MessengerState.sCurrentSupporterCrypt).c_str(), lUid);
		sFullUrl = theApp.m_cSettings.m_sBaseUrlPickupConsult + sOtpUrlPart;
		//
		//
		//
		if(theApp.m_cSettings.m_dwWorkbecnOpenMode==1)
		{
			Log.Add(_WARNING_, _T("WorkbecnOpenMode is set to 1. Open URL"));
			m_cUrlOpener.Open(sFullUrl);
			break;
		}

		theApp.LauchWorkbenchProcess(sFullUrl, callType, sCustomerDisplayName, lUid);
		break;

		//	todo if already exisits we have not to add it ot the map
		//	this will cause problem with map of alerts
		//	this may not happnen, but still may be fixed 
		/*
		m_mapWorkbenchDlgConsults.insert( WorkbenchDlgConsultsMap::value_type( 
			lUid, 
			new CWorkbenchDlg(
				GetDesktopWindow(), 
				sFullUrl, 
				lUid, 
				this->m_hWnd,
				callType,
				sCustomerDisplayName)));// - better to use to get result
		*/
	default:
		break;
	}
	
	Log.Add(_WARNING_, _T("DoPickUp finished. lUid = %u"), lUid);

CATCH_LOG(_T("CSupportMessengerDlg::DoPickUp"))
	return 0L;
}


LRESULT CSupportMessengerDlg::OnAlertDlgNewCallClose(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 

	long lUid = (long)lParam;
	CloseAlert(lUid);
	
	char buf[16] = {'\0'};
	sprintf_s(buf, 16, "%d", lUid);  
	tstring body(buf);

	m_cCommunicator.EnqueueExecutionQueue(new CNodeSendMessage(
		(tstring)m_MessengerState.sCurrentSupporterCrypt.c_str(), 
		PROTOCOL_MSG_CLOSE_PENDING_CALL, 
		body,
		theApp.m_cSettings.m_sResource));//resource of myself always IMClient 

CATCH_LOG(_T("CSupportMessengerDlg::OnAlertDlgClose"))
	return 0L;
}

LRESULT CSupportMessengerDlg::CloseAlert(long lUid)
{
TRY_CATCH 

	CAlertDlg*			pAlertDlg = NULL; 
	AlertsMapIterator   it = m_mapAlerts.find(lUid);

	Log.Add(_MESSAGE_,_T("CloseAlert UID= %d"), lUid);

	if(it != m_mapAlerts.end())
	{
		pAlertDlg = (*it).second;
		if(pAlertDlg)
		{
			//  todo - first destroy parent window 
			//  pcTransparentWin = pAlertDlg->getParentWindow();
			//  if( pcTransparentWin )
			//	delete pcTransparentWin;
			//pAlertDlg->FadeOutAlert();//close alert by fade out
			Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::CloseAlert UID= %d"), lUid);
			pAlertDlg->DestroyModeless(); //call delete in the PostNcDestroy
		}

		m_mapAlerts.erase(it);
		StopAnimation();
	}

CATCH_LOG(_T("CSupportMessengerDlg::CloseAlert"))
	return 0L;
}

LRESULT CSupportMessengerDlg::CloseAllAlerts()
{
TRY_CATCH 

	CAlertDlg* pAlertDlg = NULL; 
	AlertsMapIterator it = m_mapAlerts.begin();
 	
	for( ; it != m_mapAlerts.end(); )
    {
		pAlertDlg = (*it).second;
		if( pAlertDlg )
		{
			Log.Add(_MESSAGE_,_T("Close alert %d"), ((CAlertDlgNewCall*)pAlertDlg)->GetCallId());
			pAlertDlg->DestroyModeless(); //call delete in the PostNcDestroy
		}
		else
		{
			Log.Add(_WARNING_,_T("Close alert FAILED. No alert %d"), ((CAlertDlgNewCall*)pAlertDlg)->GetCallId());		
		}
	
		it = m_mapAlerts.erase( it );
    }

	StopAnimation();

CATCH_LOG(_T("CSupportMessengerDlg::CloseAllAlerts"))
	return 0L;
}

LRESULT CSupportMessengerDlg::CloseAllNewCallAlerts()
{
TRY_CATCH 

	CAlertDlg* pAlertDlg = NULL; 
	AlertsMapIterator it = m_mapAlerts.begin();

	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::CloseAllNewCallAlerts called"));

	for( ;it!=m_mapAlerts.end(); )
    {
		pAlertDlg = (*it).second;

		if(pAlertDlg)
		{
			if( ((CAlertDlgNewCall*)pAlertDlg)->GetCallType()==CustomerDirectCall )
			{
				pAlertDlg->DestroyModeless(); //call delete in the PostNcDestroy
				m_mapAlerts.erase(it++);
				Log.Add(_MESSAGE_,_T("Close CustomerDirectCall Alert %d"),((CAlertDlgNewCall*)pAlertDlg)->GetCallId());
			}
			else
			{
				++it;
				Log.Add(_MESSAGE_,_T("Don't close not new call Alert %d"),((CAlertDlgNewCall*)pAlertDlg)->GetCallId());
			}
		}
		else
		{
			it = m_mapAlerts.erase(it++);
		}
    }

CATCH_LOG(_T("CSupportMessengerDlg::CloseAllNewCallAlerts"))
	return 0L;
}

LRESULT CSupportMessengerDlg::StopAnimation()
{
TRY_CATCH

	//
	//	todo for version without UI 
	//	tofix: need to stop animation if there is no more new call alerts
	//  
	//	if it is last alert then we need to stop animation
	//	StopAnimation()
	//  if( (long)m_mapAlerts.size() <= 0 )
	if(CountNewCallAlerts()==0)
	{
		m_MessengerState.bAnimateTray = FALSE;
		UpdateTrayIcon(NewCallsAnimationStoped);					
	}

CATCH_LOG(_T("CSupportMessengerDlg::StopAnimation"))
	return 0L;
}


DWORD CSupportMessengerDlg::CountNewCallAlerts()
{
	DWORD iCurrentNewCallAlerts = 0;

TRY_CATCH 
	CAlertDlg* pAlertDlg = NULL; 
	AlertsMapIterator it = m_mapAlerts.begin();
	
	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::CountNewCallAlerts called"));

	for( ;it!=m_mapAlerts.end(); )
    {
		pAlertDlg = (*it).second;
		if(pAlertDlg)
		{
			if( ((CAlertDlgNewCall*)pAlertDlg)->GetCallType()==CustomerDirectCall )
				iCurrentNewCallAlerts++;
		}
		++it;
    }

	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::CountNewCallAlerts iCurrentNewCallAlerts: %d"), iCurrentNewCallAlerts);

CATCH_LOG(_T("CSupportMessengerDlg::CloseAllNewCallAlerts"))
	return iCurrentNewCallAlerts;
}

void CSupportMessengerDlg::PostNcDestroy() 
{
TRY_CATCH 
   //delete this;
CATCH_LOG(_T("CSupportMessengerDlg::PostNcDestroy"))
}

HRESULT CSupportMessengerDlg::OnPageLoaded(IHTMLElement *pElement)
{
TRY_CATCH 

	if(IsPageLoaded())
	{
		//
		//	update login window
		//
		UpdateLoginWindow();

		//
		//	show window depending on options
		//
		if(theApp.m_bAppStartedAutomatiaclly==TRUE && m_pcSupporterLocalData!=NULL 
			&& m_pcSupporterLocalData->getLoginInfo()->getRememberMe()==TRUE)
		{
			//
			//	auto start of messenger, inform login page to start 
			//
			m_cHTMLInterface.INTERFACE_UpdateData(DHTML_INTERFACE_LOGIN_START, _T("")); 

			if(m_pcSupporterLocalData!=NULL)
			{
				if(m_pcSupporterLocalData->getSettings()->bOpenMainWindowOnMessangerStartUp == TRUE)
					ShowWindow(SW_NORMAL);
				else
					ShowWindow(SW_HIDE);
			}
			else
			{
				ShowWindow(SW_NORMAL);
			}
		}
		else
		{
			ShowWindow(SW_NORMAL);
		}
	}
CATCH_LOG(_T("CSupportMessengerDlg::OnPageLoaded"))
	return 0;
}


//	todo OnDocumentComplete is called not once as expected
//	we added special CALLBACK_PageLoaded
//
void CSupportMessengerDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
TRY_CATCH 

	CDHtmlDialogEx::OnDocumentComplete(pDisp, szUrl);


CATCH_LOG(_T("CSupportMessengerDlg::OnDocumentComplete"))
}
//
//	Inform DHTML dialog to show login information
//	we supply:
//	1) Array of Supporters "registrated" for this Windows Account
//  First Supporter (in the list) is the supporter that LogedIn last
//  2) RememberMe BOOL value stored flag for the Last LogedIn supporter
//  3) Last status selected by Supporter
//  4) Flag that indicates if AutoLogin is performing now - TODO - TBD
//  The function return emidiatly 
//  After User selectd one of the options in the DHTMLLogin screen 
//  CallBack function will be called
//
//  Start (with parameters)
//  - SupporterID
//  - RememberMe
//  - Login status: 
LRESULT CSupportMessengerDlg::LoadDHTMLLoginDialogData()
{
TRY_CATCH 

	tstring	supporterId;
	
	// 
	//
	supporterId = m_cMessengerLocalData.getLastLogiedInEntry()->getSupporterId(); 
	//
	//	Supporter at least once LogedIn on this PC
	//	We retrieve ReloadCurrentSupporterData
	//
	if(supporterId.size()!=0)
	{
		// set current supporter
		m_MessengerState.sCurrentSupporter = supporterId;
		// reload current supporter data
		ReloadCurrentSupporterData();
		
		if(m_pcSupporterLocalData!=NULL)
		{
			// Supporter's list
			m_cMessengerLocalData.LoadSupportersList();
			CStrList	supporterList = m_cMessengerLocalData.getSupporterList();

			for(StringListIter iter=supporterList.begin(); iter!=supporterList.end(); ++iter)
			{
				Log.Add(_MESSAGE_,_T("Supporter once tried SignedIn on the PC: %s"), (*iter).c_str());
			}

			Log.Add(_MESSAGE_,_T("SupporterId is: %s"), supporterId.c_str());
			Log.Add(_MESSAGE_,_T("LastSelectedStatus is: %d"), m_pcSupporterLocalData->getLoginInfo()->getLastSelectedStatus());
			Log.Add(_MESSAGE_,_T("Remember me flag is: %d"), m_pcSupporterLocalData->getLoginInfo()->getRememberMe());
		}
	}
	else
	{
		Log.Add(_MESSAGE_,_T("Not found any once logined supporter on the PC"));
	}
	
CATCH_LOG(_T("CSupportMessengerDlg::LoadDHTMLLoginDialogData"))
	return 0L;
}

LRESULT CSupportMessengerDlg::UpdateLoginWindow()
{
TRY_CATCH

	if(m_pcSupporterLocalData!=NULL)
	{
		CString	rememberMe;

		if(m_pcSupporterLocalData->getLoginInfo()->getRememberMe()==TRUE)
			rememberMe = _T("1");

		m_cHTMLInterface.INTERFACE_UpdateData(
			DHTML_INTERFACE_LOGIN_INFO, _T(""), m_MessengerState.sCurrentSupporter.c_str(), rememberMe);
	}
	else
	{
		Log.Add(_ERROR_,_T("SupporterLocalData not initialized"));
	}

CATCH_LOG(_T("CSupportMessengerDlg::UpdateLoginWindow"))
	return 0L;
}


//TODO critical section here to protect m_pcSupporterLocalData
void CSupportMessengerDlg::ReloadCurrentSupporterData()
{
	if( m_pcSupporterLocalData != NULL )
		delete m_pcSupporterLocalData;

	m_pcSupporterLocalData = new CSupporterLocalData(m_MessengerState.sCurrentSupporter);

	if(m_pcSupporterLocalData)
	{
		m_pcSupporterLocalData->getSettings()->Load();
		m_pcSupporterLocalData->getLoginInfo()->Load();
	}
	//
	//	
	//
	RestartActivityHandler();
}

void CSupportMessengerDlg::OnPopupExit()
{
TRY_CATCH 

	OnPopupSingout();

	// TODO:anatoly
	// Unregister our AppBar window with the Shell
	// SetState(ABE_UNKNOWN);
	// we have to close application properly
	if(m_MessengerState.bUserSelectedSignIn == FALSE)
	{
		OnClose();
		//DestroyWindow();
		DestroyModeless();
	}
CATCH_LOG(_T("CSupportMessengerDlg::OnPopupExit"))
}

void CSupportMessengerDlg::OnPopupOpen()
{
TRY_CATCH 

	// show main dialog
	if(IsWindowVisible()== FALSE)
		ShowWindow(SW_NORMAL);

	// stop animation 
	m_MessengerState.bAnimateTray = FALSE;
	UpdateTrayIcon(NewCallsAnimationStoped);

	// close all availible alerts
	CloseAllAlerts();
	
CATCH_LOG(_T("CSupportMessengerDlg::OnPopupOpen"))
}

BOOL CSupportMessengerDlg::InitAppBar() 
{
TRY_CATCH 
	APPBARSTATE abs;
	//
	// Check the registry to see if we have been used before and if so,
	// reload our persistent settings.
	DWORD cbData = sizeof(abs);

	CRect rc;

	// Set the CAppBar class's behavior flags
	m_fdwFlags = ABF_ALLOWANYWHERE | ABF_MIMICTASKBARAUTOHIDE | ABF_MIMICTASKBARALWAYSONTOP;

	GetClientRect(&rc);

	// Width has no limits, height sizes in client-area-height increments
	m_szSizeInc.cx = 1;
	m_szSizeInc.cy = rc.Height(); 

	// The appbar has a minimum client-area size that is determined by the 
	// client area set in the dialog box template.
	m_szMinTracking.cx = rc.Width();
	m_szMinTracking.cy = rc.Height();

	// Setup default state data for the AppBar
	abs.m_cbSize = sizeof(abs);
	abs.m_uState = ABE_FLOAT;
	abs.m_fAutohide = FALSE;
	abs.m_fAlwaysOnTop = TRUE;

	// Set the default floating location of the appbar
	GetWindowRect(&abs.m_rcFloat);

	// Make the default width twice the width set in the dialog editor
	// abs.m_rcFloat.right += abs.m_rcFloat.Width();
	abs.m_rcFloat.right = MIN_UI_WIDTH;

	// Make the default bottom x10 the width set in the dialog editor
	// abs.m_rcFloat.bottom = 10*abs.m_rcFloat.Height();
	abs.m_rcFloat.bottom = MIN_UI_HEIGHT;

	// Temporarily turn off window adornments to determine the dimensions
	// of the appbar when docked.
	HideFloatAdornments(TRUE);
	AdjustWindowRectEx(&rc, GetStyle(), FALSE, GetExStyle());
	HideFloatAdornments(FALSE);
	abs.m_auDimsDock[ABE_LEFT]   = rc.Width()*2;
	abs.m_auDimsDock[ABE_TOP]    = rc.Height();
	abs.m_auDimsDock[ABE_RIGHT]  = rc.Width()*2;
	abs.m_auDimsDock[ABE_BOTTOM] = rc.Height();
		
	m_cMessengerLocalData.getLastLogiedInEntry()->getAppState((PBYTE)&abs, &cbData);

	if (cbData != sizeof(abs)) 
	{
		if(cbData==0)
			Log.Add((_T("CSupportMessengerDlg::InitAppBar() Application opened first time"))); 	
		else
		{
			// The saved persistent data is a different size than what we expect. 
			// The user probably saved the data using an older version of the AppBar class.  
			// Do any version differencing here...
			Log.Add(_MESSAGE_,_T("getLastLogiedInEntry->getAppState saved persistent data is a different size than what we expect"));
		}
	}

	// Set the initial state of the appbar.
	SetState(abs);

CATCH_LOG(_T("CSupportMessengerDlg::InitAppBar"))

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSupportMessengerDlg::HideFloatAdornments(BOOL fHide) {
TRY_CATCH 

    //todo anatoly - we do not want Adornments- Titel bar for the window at all
/*
	if (fHide) {
		ModifyStyle(WS_CAPTION | WS_SYSMENU, 0, SWP_DRAWFRAME);
	} else {
		ModifyStyle(0, WS_CAPTION | WS_SYSMENU, SWP_DRAWFRAME);
	}
*/
CATCH_LOG(_T("CSupportMessengerDlg::HideFloatAdornments"))
}

// Tell our derived class that there is a proposed state change
void CSupportMessengerDlg::OnAppBarStateChange (BOOL fProposed, UINT uStateProposed) {

	// Hide the window adorments when docked.   //TODO Anatoly - we do not need to Hide it
	// HideFloatAdornments((uStateProposed == ABE_FLOAT) ? FALSE : TRUE);
}

void CSupportMessengerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_CLOSE) {
		// We have to manually add this so that the dialog box closes when 
		// when the user clicks the close button (which appears in the top, right)
		// corner of the dialog box when it is floating).
		OnCancel();
	}

	CAppBar::OnSysCommand(nID, lParam);
}

void CSupportMessengerDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
TRY_CATCH 

	// Get the minimum size of the window assuming it has no client area.
	// This is the width/height of the window that must always be present
	CRect rcBorder(0, 0, 0, 0);
	AdjustWindowRectEx(&rcBorder, GetStyle(), FALSE, GetExStyle());

	if (GetState() == ABE_FLOAT) {
		//todo anatoly - not need to limit when float
		//lpMMI->ptMinTrackSize.x = m_szMinTracking.cx + rcBorder.Width();
		//lpMMI->ptMinTrackSize.y = m_szMinTracking.cy + rcBorder.Height();
		lpMMI->ptMinTrackSize.x = MIN_UI_WIDTH; //todo max and min size
		lpMMI->ptMinTrackSize.y = MIN_UI_HEIGHT;

		lpMMI->ptMaxTrackSize.x = MIN_UI_WIDTH + 20;  //allow resizing a bit till Max size STL-393
		lpMMI->ptMaxTrackSize.y = MIN_UI_HEIGHT + 20; //allow resizing a bit till Max size STL-393
		return;
	}
	
	// The appbar can't be more than half the width or height
	// of the screen when docked or when floating
	lpMMI->ptMaxTrackSize.x = GetSystemMetrics(SM_CXSCREEN);
	lpMMI->ptMaxTrackSize.y = GetSystemMetrics(SM_CYSCREEN);
	if (!IsEdgeTopOrBottom(GetState()))
		lpMMI->ptMaxTrackSize.x /= 2;

	if (!IsEdgeLeftOrRight(GetState()))
		lpMMI->ptMaxTrackSize.y /= 2;

	CAppBar::OnGetMinMaxInfo(lpMMI);

CATCH_LOG(_T("CSupportMessengerDlg::OnGetMinMaxInfo"))
}


void CSupportMessengerDlg::OnSize(UINT nType, int cx, int cy) {

	CAppBar::OnSize(nType, cx, cy);
}

void CSupportMessengerDlg::OnCancel()
{
TRY_CATCH 
	//
	// Save the current state of the appbar in the registry so that we'll
	// come up in the same state the next time the user runs us.
	APPBARSTATE abs;
	abs.m_cbSize = sizeof(abs);
	GetState(&abs);

	// Store AppState to retrieve on Login 	
	m_cMessengerLocalData.getLastLogiedInEntry()->setAppState((PBYTE) &abs, sizeof(abs));
	m_cMessengerLocalData.getLastLogiedInEntry()->Save();

	// Store last status to retrieve on next Login 
	if(m_pcSupporterLocalData)
	{
		m_pcSupporterLocalData->getLoginInfo()->setLastSelectedStatus(m_MessengerState.eSupporterSelectedStatus);
		m_pcSupporterLocalData->getLoginInfo()->Save();
	}

	CAppBar::OnCancel();

CATCH_LOG(_T("CSupportMessengerDlg::OnCancel"))
}
void CSupportMessengerDlg::OnDockRight()
{
TRY_CATCH 

	SetState(ABE_RIGHT);
	if(IsWindowVisible() == FALSE)
		ShowWindow(SW_NORMAL);	

CATCH_LOG(_T("CSupportMessengerDlg::OnDockRight"))
}

void CSupportMessengerDlg::OnDockLeft()
{
TRY_CATCH 

	SetState(ABE_LEFT);
	if( IsWindowVisible() == FALSE )
		ShowWindow(SW_NORMAL);

CATCH_LOG(_T("CSupportMessengerDlg::OnDockLeft"))
}

void CSupportMessengerDlg::OnClose()
{
TRY_CATCH 

	SetState(ABE_FLOAT);//todo
	ShowWindow(SW_HIDE);
	//ShowWindow(SW_MINIMIZE);
	OnCancel();

CATCH_LOG(_T("CSupportMessengerDlg::OnClose"))
}

void CSupportMessengerDlg::OnLinksSupportspacehomepage()
{
TRY_CATCH 

	m_cUrlOpener.Open(LINK_SUPPORTSPACE_HOMEPAGE);
	//m_cUrlOpener.LauchIEBrowser(LINK_SUPPORTSPACE_HOMEPAGE);

CATCH_LOG(_T("CSupportMessengerDlg::OnLinksSupportspacehomepage"))
}

HRESULT CSupportMessengerDlg::OnCbkEventsHandlerClickedEvent(IHTMLElement *pElement)
{
TRY_CATCH 

	VARIANT AttributeValue;
	pElement->getAttribute( L"sEvent", 0, &AttributeValue);
	CString sEvent = (CString)AttributeValue;

	MapCallbacksMsngrDlgIterator it = mapCallbacksMsngrDlg.find(sEvent);

	if(it != mapCallbacksMsngrDlg.end())
	{
		Log.Add(_MESSAGE_,_T("Event found - eventID: '%s'"), sEvent);
		(this->*((*it).second))(pElement);
	}
	else
	{
		Log.Add(_MESSAGE_,_T("Warning - this eventID: '%s' not in the event map"), sEvent);
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnCbkEventsHandlerClickedEvent"))

	return S_OK;
}

HRESULT CSupportMessengerDlg::OnInboxPickupCall(IHTMLElement *pElement)
{
TRY_CATCH 
	long		lCallUID = 0;
	VARIANT		tmpCallUID;
	CString		sDisplayName;
	VARIANT     sTmpStr; 
	
	if(pElement!=NULL)
	{
		pElement->getAttribute(L"iCallUID", 0, &tmpCallUID);
		lCallUID = tmpCallUID.lVal;

		pElement->getAttribute(L"sDisplayUserName", 0, &sTmpStr);
		sDisplayName = (CString)sTmpStr;
		
		DoPickUp(lCallUID, CustomerDirectCall, sDisplayName);//TODO CallType param. not for closed betta- NO Inbox UI
	}
	
CATCH_LOG(_T("CSupportMessengerDlg::OnPickupCall"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnSettingsApp(IHTMLElement *pElement)
{
TRY_CATCH 

	long		lCallUID = 0;
	VARIANT		tmpStr;
	CString		sLoginEmail;
	
	//
	//
	if(pElement!=NULL)
	{
		pElement->getAttribute(L"sLoginEmail", 0, &tmpStr);
		sLoginEmail = (CString)tmpStr;

		if( sLoginEmail.Compare(m_MessengerState.sCurrentSupporter.c_str())!=0 )
		{
			m_MessengerState.sCurrentSupporter = sLoginEmail;
			ReloadCurrentSupporterData();
		}
	}

   // todo - need here Supporter's username as attribute
   // sLoginEmail attribute added
   m_MessengerState.eTrayMenuOpenReason = MainDialog;

   CPoint ptCursor;
   GetCursorPos(&ptCursor);
   CMenu*  hPopUpMenu = m_TrayIcon.GetMenu().GetSubMenu(0);
   hPopUpMenu->TrackPopupMenu(0, ptCursor.x, ptCursor.y, this, NULL);

CATCH_LOG(_T("CSupportMessengerDlg::OnSettingsApp"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnRestoreApp(IHTMLElement *pElement)
{
TRY_CATCH 
CATCH_LOG(_T("CSupportMessengerDlg::OnRestoreApp"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnMinimizeApp(IHTMLElement *pElement)
{
TRY_CATCH 
	OnClose();
	//ModifyStyle(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX , 0, SWP_DRAWFRAME);
	//ShowWindow(SW_SHOWMINIMIZED); //http://www.experts-exchange.com/Programming/System/Windows__Programming/MFC/Q_23296007.html
	//ShowWindow(SW_MINIMIZE);// WS_MINIMIZEBOX

CATCH_LOG(_T("CSupportMessengerDlg::OnMinimizeApp"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnCloseApp(IHTMLElement *pElement)
{
TRY_CATCH 
	OnClose();
CATCH_LOG(_T("CSupportMessengerDlg::OnCloseApp"))
	return S_OK;
}

/*
	This callback will be called by DHTML when it added call to the list and ready to show it.
	CallBack attributes are: CallID and jsonCallData
	OnCallAdded will check:
	- if Alert required by user settings;
	- if CSupportMessengerDlg  is closed
	Then will show alert and add this alert to the map of Alerts
	- the alert will be shown in the position due rule as describe in PRD

	Depending on ReqType different logic may be done
*/
HRESULT CSupportMessengerDlg::OnRequestAdded(IHTMLElement *pElement, eCallType callType)
{
TRY_CATCH

	long		iCallUID = 0;
	VARIANT		tmpCallUID;
	VARIANT		tmpCalls;

	switch(callType)
	{
	case CustomerDirectCall:
		Log.Add(_MESSAGE_, _T("CSupportMessengerDlg::OnRequestAdded NewCallRequest"));
		m_MessengerState.dwNewCallsCounter++;
		UpdateTrayIcon(NewCallsCounterChanged);
		break;
	case ConsultCall:
		m_MessengerState.dwNewConsultCall++;
		Log.Add(_MESSAGE_, _T("CSupportMessengerDlg::OnRequestAdded ConsultRequest"));
		break;
	default:
		Log.Add(_ERROR_, _T("CSupportMessengerDlg::OnRequestAdded Not specified"));
		break;
	}

	//	Animate the tray icon upon call receival 
	//	will be stoped by StopNewRequestAnimation on WM_LBUTTONDBLCLK?
	if(m_pcSupporterLocalData->getSettings()->bOnIncomingCallsAnimateTrayIcon == TRUE)
	{
		m_MessengerState.bAnimateTray = TRUE;
		UpdateTrayIcon(NewCallsAnimationActivated);
	}

	//	Play sound upon settings
	if(m_pcSupporterLocalData->getSettings()->bPlaySoundUponIncomingCall == TRUE)
	{
		if(PlaySound(MAKEINTRESOURCE(IDW_WAVE_MESSAGE),::AfxGetResourceHandle(),SND_RESOURCE | SND_ASYNC) == FALSE)
		{
			Log.Add(_MESSAGE_, _T("PlaySound failed %d"), GetLastError());
		}
	}
///
///	CHECK HERE IF WE HAVE PLACE TO SHOW ALERT 	
///
	if(IsFreeSpaceToShowAlert()==FALSE)
		return S_OK;

	//	Show alert upon settings
	//	if(IsWindowVisible()==FALSE)
	//	todo (not for beta) we have to show alert not depending on window visiability
	//	in the future we have to determine is MainWindowVisiable then not show alert
	if(m_pcSupporterLocalData->getSettings()->bOnIncomingCallsShowAlert==TRUE)
	{
		Log.Add(_MESSAGE_, _T("CSupportMessengerDlg:OnRequestAdded IsWindowVisible FALSE show alert if settings"));
		
		pElement->getAttribute(L"iCallUID", 0, &tmpCallUID);
		iCallUID = tmpCallUID.lVal;
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnRequestAdded Attribute iCallUID %d"), iCallUID);

		pElement->getAttribute(L"sCalls", 0, &tmpCalls);
		CString sCalls(ToUtf8FromUtf16(tmpCalls.bstrVal).c_str());
		
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnRequestAdded Attribute sCalls")); 
		// todo anatoly
		// we have to manage free position for each alert
		// than final position will be calculated due free position
		// m_lFreePosition = (long)m_mapAlerts.size() + 1; 
		m_lFreePosition++; 
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnRequestAdded FreePosition sCalls %d"), m_lFreePosition);

		
		//CTransparentWindow* pcTransparentWindow = new CTransparentWindow(GetModuleHandle(NULL), 0, 0, 100, 50);
		//HWND	hWndParent = pcTransparentWindow->GetWindowHandle();
		//CWnd*   pParent = CWnd::FromHandle( hWndParent );
		//CWnd*	pCWndNotify = CWnd::FromHandle(this->m_hWnd);
		

		//	todo if already exisits we have not to add it ot the map
		//	this will cause problem with map of alerts
		//	this may not happnen, but still may be fixed 
		m_mapAlerts.insert( AlertsMap::value_type( 
			iCallUID, 
			new CAlertDlgNewCall( 
				CWnd::FromHandle(this->m_hWnd), 
				NULL,//pcTransparentWindow,
				NULL,//hWndParent
				NULL,//pParent
				theApp.m_cSettings.m_eGUIlocation,
				theApp.m_cSettings.m_sGUIlocationFilePath,
				m_lFreePosition, 
				iCallUID, 
				sCalls,
				callType
				)));// - better to use to get result
	}
	else{
		Log.Add(_MESSAGE_,_T("OnRequestAdded:IsWindowVisible TRUE - do nothing"));
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnRequestAdded"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnNewCallAdded(IHTMLElement *pElement)
{
TRY_CATCH 

	OnRequestAdded(pElement, CustomerDirectCall);

CATCH_LOG(_T("CSupportMessengerDlg::OnNewCallAdded"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnNewConsultRequestAdded(IHTMLElement *pElement)
{
TRY_CATCH

	OnRequestAdded(pElement, ConsultCall);

CATCH_LOG(_T("CSupportMessengerDlg::OnNewConsultRequestAdded"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnNewConsultCallPicked(long lCallUID)
{
TRY_CATCH

	//
	//	Decrement counter if new call deleted from Inbox 
	if(lCallUID >0 && m_MessengerState.dwNewConsultCall>0)
		m_MessengerState.dwNewConsultCall--;

	//
	//	Update tray
	UpdateTrayIcon(NewConsultCounterChanged);

	//
	//	Close Alert if availible
	//
	if(lCallUID!=0)
	{
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnNewConsultCallPicked iCallUID %d"), lCallUID);
		CloseAlert(lCallUID);
	}

	//
	//	Stop animation if counter 0
	if(m_MessengerState.dwNewConsultCall == 0)
	{
		m_MessengerState.bAnimateTray = FALSE;
		UpdateTrayIcon(NewConsultCounterChanged);
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnNewConsultCallPicked"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnInSessionCounsltCallStatusChanged(long lCallUID)
{
TRY_CATCH

	//
	//	Decrement counter if new call deleted from Inbox 
	if(lCallUID >0 && m_MessengerState.dwInSessionConsultCallCounter  >0)
		m_MessengerState.dwInSessionConsultCallCounter--;

	//
	//	Update tray
	UpdateTrayIcon(InSessionConsultCallsCounterChanged);

	//
	//	Close Alert if availible
	//
	if(lCallUID!=0)
	{
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnInSessionCounsltCallStatusChanged iCallUID %d"), lCallUID);
		CloseAlert(lCallUID);
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnInSessionCallStatusChanged"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnConsultStatusChanged(IHTMLElement *pElement)
{
TRY_CATCH

	long lCallUID = 0;
	VARIANT	tmpCallUID;
	VARIANT	tmpState;
	CString	sState;
	
	pElement->getAttribute( L"iCallUID", 0, &tmpCallUID);
	if(tmpCallUID.vt != VT_NULL)
		lCallUID = tmpCallUID.lVal;
	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnConsultStatusChanged Attribute iCallUID: %d"), lCallUID);

	pElement->getAttribute( L"sState", 0, &tmpState);
	if(tmpState.vt != VT_NULL)
		sState = (CString)tmpState;
	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnConsultStatusChanged Attribute sState: %s"), sState);

	if(sState.Compare(_T("PICKED"))== 0)
	{
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnConsultStatusChanged PICKED iCallUID %d"), lCallUID);
		OnNewConsultCallPicked(lCallUID); 
	}
	else
	{
		OnInSessionCounsltCallStatusChanged(lCallUID);
		//Session end:     "state":"ENDED"
		//Missed call:     "state":"TIMEOUT"
		//Canceled call:   "state":"CANCELED"
	}

	//
	//
	//

CATCH_LOG(_T("CSupportMessengerDlg::OnConsultStatusChanged"))
	return S_OK;
}

//
//	The flow is like that:
//  1. Send IMConnect 
//  2. Handle OnIMConnected callback 
//  3. Send IMSendMessage protocol message UpdateCheckRequest with Tools package version 
//  (retrieved from the registry. May be set by Installer when installation done) 
//  4. Handle OnIMNewCall - look for UpdateCheckResponce
//	5. If UpdateRequired - Show AlertDlgUpdate (Disable all ...)
//  6. If UpdateNotRequired Send UpdateStatus and PROTOCOL_MSG_GETPENDINGCALLS
LRESULT CSupportMessengerDlg::OnIMConnected(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 

	//
	//	todo notify UI about conneciton progress
	//
	m_cHTMLInterface.INTERFACE_UpdateData(
			DHTML_INTERFACE_LOGIN_STATE, _T(""), SigninStatePairs[SIGNIN_STATE_VERSION_VALIDATION].m_iSignInFieldString); 

	SetConnectionState(stateConnected);
	//
	//
	if(m_cActivityHandler.IsInActivateFeatureStarted()==FALSE)
		RestartActivityHandler();

	// start IMChannelQualityMonitor
	m_cIMChannelQualityMonitor.Start(this->m_hWnd, 
		theApp.m_cSettings.m_stIMConnectivityQuality.PollingInterval,
		theApp.m_cSettings.m_stIMConnectivityQuality.TimePeriodForMonotoring,
		theApp.m_cSettings.m_stIMConnectivityQuality.MaxNumOfUnacceptableMsgs,
		theApp.m_cSettings.m_stIMConnectivityQuality.AcceptableRoundtripTimeLimit);

	//  calculate connection time for statistics 
	struct _timeb		tstConnectionEndTime;
    _ftime_s( &tstConnectionEndTime ); 
	double connectionEndTime =  tstConnectionEndTime.time + tstConnectionEndTime.millitm/1000.0; 
	double connectionTime = connectionEndTime - m_MessengerState.fConnectStartTime;  
	
	Log.Add(_MESSAGE_,_T("Connection take: %6.3lf seconds"), connectionTime);
	theApp.m_conQualityLog.AddMessage(cLog::_MESSAGE, Format( _T("Connection took: %6.3lf sec. Num retries: %d"), connectionTime, m_MessengerState.dwIMConnectAttempt));

	m_MessengerState.dwIMConnectAttempt = 0;

	//
	//	after any reconnect status may be set to last status 
	//	
	if(m_MessengerState.bStatusChangedOnIdle)
	{
		IMSendUpdateStatus(StatusAway);
		UpdateTrayIcon(MissedCallsCounterChanged);
	}
	else
	if(m_MessengerState.bStatusChangedOnMissedCalls)
	{
		UpdateTrayIcon(MissedCallsCounterChanged);
		IMSendUpdateStatus(StatusBusy);
	}
	else
	{
		IMSendUpdateStatus(m_MessengerState.eSupporterSelectedStatus);
		UpdateTrayIcon(RollbackToSupporterSelectedStatus);
	}

	//
	//	TODO here - if it is Reconnect then We have to inform GUI to Clear all and to show new info
	//  Or  just to reconect then it is bad to send PROTOCOL_MSG_GET_PENDINGCALLS again
	//  Talk with Server and GUI and PRD

	
	//
	//	In order to get the support requests that are waiting for a supporter
	//	we have to send jabber IM message OnIMConnected
	//	to:         supportspace@supportspace.com 
	//	subject:    getPendingSupportRequests 
	//
	//	rodo removed for version without UI
	//	m_cCommunicator.EnqueueExecutionQueue(new CNodeSendMessage(
	//	(tstring)m_cSettings.m_sPendingSupportRequestsTo, PROTOCOL_MSG_GET_PENDINGCALLS, (tstring)_T("")));

	//
	//
	//
	m_cCommunicator.EnqueueExecutionQueue(new CNodeSendMessage(
		(tstring)theApp.m_cSettings.m_sPendingSupportRequestsTo, 
		PROTOCOL_MSG_GET_UNREAD_MESSAGES, 
		(tstring)_T(""), 
		(tstring)theApp.m_cSettings.m_sPendingSupportRequestsToResource));

	m_cHTMLInterface.INTERFACE_UpdateData(
		DHTML_INTERFACE_LOGIN_STATE, _T(""), SigninStatePairs[SIGNIN_STATE_SIGNNED_IN].m_iSignInFieldString); 
	
	SetState(ABE_FLOAT);//todo
	ShowWindow(SW_HIDE);

CATCH_LOG(_T("CSupportMessengerDlg::OnIMConnected"))
	return 0L;
}

LRESULT	CSupportMessengerDlg::OnIMConnectFailed(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 
// from gloox 0.9 this callback is not required more for workaround of connect/disconnect issues
CATCH_LOG(_T("CSupportMessengerDlg::OnIMConnectFailed"))
	return 0L;
}

LRESULT CSupportMessengerDlg::OnIMTLSConnected(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 
// from gloox 0.9 this callback is not required more for workaround of connect/disconnect issues
CATCH_LOG(_T("CSupportMessengerDlg::OnIMTLSConnected"))
	return 0L;
}

LRESULT CSupportMessengerDlg::OnIMConnecting(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 
	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnIMConnecting"));
CATCH_LOG(_T("CSupportMessengerDlg::OnIMConnecting"))
	return 0L;
}

//todo 
LRESULT CSupportMessengerDlg::ShowLogoutWindow(sErrorInfo* pErrInfo)
{
TRY_CATCH

	CString	sMsg = _T("");

	//
	//	format error message
	//
	if(pErrInfo!=NULL)
	{
		sMsg.FormatMessage(_T("%1!s!. %2!s!%3!d!-%4!d!-%5!d!"),
			(CString)MAKEINTRESOURCE(pErrInfo->m_resIDforDescr),
			(CString)MAKEINTRESOURCE(IDS_ERROR_ERROR_CODE_PREFIX),
			pErrInfo->m_errSrcType,					
			pErrInfo->m_errCode, 
			pErrInfo->m_errDetailsCode);
	}

	m_cHTMLInterface.INTERFACE_UpdateData(DHTML_INTERFACE_LOGOUT, _T(""), sMsg); 

	ShowWindow(SW_NORMAL);
CATCH_LOG(_T("CSupportMessengerDlg::ShowLogoutWindow"))
	return 0L;
}

LRESULT CSupportMessengerDlg::OnIMDisconnect(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 

	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnIMDisconnect called wParam:%d, lParam:%d"), wParam, lParam);

	//
	//	todo - actually not need to call Destroy
	//  m_cCommunicator.DestroyIMClient();

	CString sMsg;
	long detailErr = 0;
	sErrorInfo errInfo;

	m_MessengerState.dwIMConnectAttempt++; //increment counter of Connect Attempts
	
	CloseAllAlerts();//close all alerts each time we get disconnect, 

	//
	//	reset counters and flags that may be reset on disconnect event
	//
	m_MessengerState.dwNewCallsCounter = 0;
	m_MessengerState.dwInSessionCallsCounter = 0;
	m_MessengerState.bStatusChangedOnMaxHandled = FALSE;
	m_MessengerState.bUpdateRequired = FALSE;
	m_MessengerState.bAnimateTray = FALSE;
	m_MessengerState.bWaitTillAllEventsClosed = FALSE; //flag to show alerts due defined in PRD rules
	m_MessengerState.bPowerStatusSuspended = FALSE;

	UpdateTrayIcon(NewCallsCounterChanged);
	SetConnectionState(stateDisconnected);
			
	if(m_MessengerState.bUserSelectedSignIn == FALSE)	//Disconnect activated by user
	{
		//
		//	TODO if disconnect was not initiated by Expert we come to this case
		//  we reset missed calls couter and close missed calls alert 
		//  may be better to move this code singout 
		//
		m_MessengerState.dwMissedCallsCounter = 0;				//todo??? if to reset this counter
		m_MessengerState.bStatusChangedOnMissedCalls = FALSE;	//todo??? if to reset this flag
		CloseMissedCallsAlert();
		StopActivityHandler(); // we also stop activity handler only in this disconnect case
		m_cIMChannelQualityMonitor.Stop(); 	// stop monitoring of connection only in this disconnect case
		m_MessengerState.bIMChannelQualityIssueDetected = FALSE;

		//  TODO if user selected to sign out. not need to call UpdateTrayIcon called with disconnect before
		m_MessengerState.bStatusChangedOnIdle = FALSE;

		ShowLogoutWindow(NULL);
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnIMDisconnect m_MessengerState.bUserSelectedSignIn == FALSE"));
		return 0;	
	}

	switch(lParam)
	{
	case ConnNoError:                    /**< Not really an error. Everything went just fine. */
	case ConnNoSupportedAuth:            /**< The auth mechanisms the server offers are not supported
										 * or the server offered no auth mechanisms at all. */
	case ConnTlsFailed:                  /**< The server's certificate could not be verified. */
    case ConnProxyAuthRequired:          /**< The HTTP/SOCKS5 proxy requires authentication.
									     * @since 0.9 */
    case ConnProxyAuthFailed:            /**< HTTP/SOCKS5 proxy authentication failed.
										 * @since 0.9 */
	case ConnProxyNoSupportedAuth:       /**< The HTTP/SOCKS5 proxy requires an unsupported auth mechanism.
										 * @since 0.9 */
	case ConnAuthenticationFailed:       /**< Authentication failed. Username/password wrong or account does
										 * not exist. */
		//	If user's password is incorrect then there is no sense in reconnect attampt
		m_MessengerState.dwIMConnectAttempt = 0;
		SetConnectionState(stateDisconnected);

		//
		//	prepeare struct to costruct ErrorMsg
		errInfo.m_errSrcType = ErrSrcJubberSrv;
		errInfo.m_resIDforDescr = IDS_ERROR_AUTHENTICATION_FAILED;
		errInfo.m_errCode = (long)lParam;
		errInfo.m_errDetailsCode = (long)wParam;

		ShowLogoutWindow(&errInfo);

		break;

	case ConnCompressionFailed:          /**< Negotiating/initializing compression failed.
			                            * @since 0.9 */
	case ConnConnectionRefused:          /**< The connection was refused by the server (on the socket level).
									      * @since 0.9 */
	case ConnDnsError:                   /**< Resolving the server's hostname failed.
										 * @since 0.9 */
	case ConnStreamError:                /**< A stream error occured. The stream has been closed. */
	case ConnStreamClosed:               /**< The stream has been closed graciously. */
		if((StreamError)wParam == StreamErrorConflict) //new stream has been initiated that conflicts with the existing stream. */
		{
			m_MessengerState.dwIMConnectAttempt = 0;
			SetConnectionState(stateDisconnected);
			//
			//	prepeare struct to costruct ErrorMsg
			errInfo.m_errSrcType = ErrSrcJubberSrv;
			errInfo.m_resIDforDescr = IDS_ERROR_YOU_ARE_SIGNIN_ON_ANOTHER_COMPUTER;
			errInfo.m_errCode = (long)lParam;
			errInfo.m_errDetailsCode = (long)wParam;

			ShowLogoutWindow(&errInfo);
			break;		
		}

	case ConnIoError:                    /**< An I/O error occured. */
	case ConnOutOfMemory:                /**< Out of memory. Uhoh. */

	case ConnUserDisconnected:           /**< The user (or higher-level protocol) requested a disconnect. */
	case ConnNotConnected:               /**< There is no active connection. */
	default:
		if(m_MessengerState.eConnectionState!=stateConnected)
		{
			sMsg.FormatMessage(_T("Initial connection failed. Error code: %1!d!-%2!d! Connection attempt: %3!d!"), 
					lParam, (StreamError)wParam , m_MessengerState.dwIMConnectAttempt);
			Log.Add(_ERROR_,sMsg);

			if(m_MessengerState.dwIMConnectAttempt <= 3)
				theApp.m_conQualityLog.AddMessage(cLog::_MESSAGE,sMsg.GetBuffer());

			if(m_MessengerState.dwIMConnectAttempt < MAX_CONNECT_ATTEMPT)
			{
				m_MessengerState.eConnectionState = stateWaitAutoReconnect;
				SetTimer(RECONNECT_TIMER_ID, RECONNECT_TIMER_TIMEOUT, NULL);
			}
			else
			{
				Log.Add(_ERROR_,sMsg);
				SetConnectionState(stateDisconnected);

				//
				//	prepeare struct to costruct ErrorMsg
				errInfo.m_errSrcType = ErrSrcJubberSrv;
				errInfo.m_resIDforDescr = IDS_ERROR_CONNECT_FAILED_LAST_ATTEPMT;
				errInfo.m_errCode = (long)lParam;
				errInfo.m_errDetailsCode = (long)wParam;

				ShowLogoutWindow(&errInfo);

				m_MessengerState.dwIMConnectAttempt = 0;
			}
		}
		break;
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnIMDisconnect"))
	return 0L;
}
/**
  * UpdateTrayIcon. 
  * This function may look updateReason and m_MessengerState and:  
  * 1) Update Application Tray Icon picture
  * 2) TooltipText
  * 3) SetBalloonDetails
  * 4) Call JS function to set correct Icon for correspondent status TODO
  * @param updateReason - the resaon why this function was called
  * @return @b Void
*/
void CSupportMessengerDlg::UpdateTrayIcon(TrayIconChangeReason updateReason)
{
TRY_CATCH

	if(updateReason == ConnectionStateChanged)//we have different handling with SetBalloonDetails in this case
	{
		switch(m_MessengerState.eConnectionState)
		{
			case stateDisconnected: 
				//	if dwIMConnectAttempt > 1 then reconnecting may be shown in tray
				if(m_MessengerState.dwIMConnectAttempt <= 1)
				{
					m_TrayIcon.SetIcon(m_srtIcons.m_SignedOut);	
					m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_OFFLINE);

					if(m_MessengerState.bUserSelectedSignIn == TRUE)
					{
						m_TrayIcon.SetBalloonDetails(
							(CString)MAKEINTRESOURCE(IDS_TRAY_ICON_TOOLTIP_IS_SIGNEDOUT),
							(CString)MAKEINTRESOURCE(IDS_APP_FULL_NAME), CTrayNotifyIcon::Warning, 3);
					}
				}
				break;
			case stateConnected:
				//
				//	todo - not always after state connected we have to show this in Balloon and tip:
				//	in the case of Idle or Missed calls
				//	
				if(m_MessengerState.bStatusChangedOnIdle == FALSE && 
				   m_MessengerState.bStatusChangedOnMissedCalls == FALSE)
				{
					//m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_AVAILIBLE);
					m_TrayIcon.SetBalloonDetails(
						(CString)MAKEINTRESOURCE(IDS_TRAY_ICON_TOOLTIP_IS_SIGNEDIN),
						(CString)MAKEINTRESOURCE(IDS_APP_FULL_NAME), CTrayNotifyIcon::Info, 3);
				}
				else
				{
					//you are connected but not availible "You are signed in" 
					m_TrayIcon.SetBalloonDetails(
						(CString)MAKEINTRESOURCE(IDS_TRAY_ICON_TOOLTIP_CONNECTED_NOT_AVAILIBLE),
						(CString)MAKEINTRESOURCE(IDS_APP_FULL_NAME), CTrayNotifyIcon::Warning, 3);
				}
				break;
			case stateConnecting://todo - not defined yet if we show this in tray 
			case stateWaitAutoReconnect:
				m_TrayIcon.SetIcon(m_srtIcons.m_ConnectingAnimated, 2, 500);	//todo animation for connecting
				m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_CONNECTING);	//			
				break;
			case stateDisconnecting://todo - not defined yet if we show this in tray 
				break;
			default:
				break;
		}	
		return;
	}

	if(m_MessengerState.eConnectionState==stateConnecting)
	{
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::UpdateTrayIcon stateConnecting"));
		return;
	}

	if(m_MessengerState.eConnectionState!=stateConnected) //Disconnected status has Priority 0
	{
		m_TrayIcon.SetIcon(m_srtIcons.m_Offline);
		m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_OFFLINE);
		return;
	}

	if(m_MessengerState.bUpdateRequired == TRUE)		 //Update required has Priority 1
	{
		m_TrayIcon.SetIcon(m_srtIcons.m_UpdateRequired);
		m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_UPDATE_REQUIRED);
		return;
	}

	//	todo - for not ui version if in session then make busy
	if(m_MessengerState.dwInSessionCallsCounter!=0)
	{
		 if(m_MessengerState.bIMChannelQualityIssueDetected==false)
		 {
			 m_TrayIcon.SetIcon(m_srtIcons.m_Busy);
			 m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_BUSY);
		 }
		 else
		 {
			 m_TrayIcon.SetIcon(m_srtIcons.m_BusyLimitedCon);
			 m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_BUSY_LIMITED_CON);
		 }
		 return;
	}
	
	if(m_MessengerState.bAnimateTray == TRUE)			//New calls animated has Priority 2
	{
		if(updateReason == NewCallsAnimationActivated)
		{
			m_TrayIcon.SetIcon(m_srtIcons.m_NewCallsAnimated, 3, 500);
			m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_NEW_CALL_ANIMATION);
		}
		return;	
	}
	else
	{
		if(updateReason == NewCallsAnimationStoped)
		{
			//stop animate will be done by setting up any next Icon by low priority 
		}
	}

/*	todo - this is disabled in version without UI - no sense in New message indicator 
	if(m_MessengerState.dwNewCallsCounter > 0)		    // New calls (X New Calls) has Priority 3
	{
		CString		sNewCalls;
		if(m_MessengerState.dwNewCallsCounter==1) //language exception
			sNewCalls.FormatMessage(IDS_TRAY_ICON_TOOLTIP_ONE_NEW_CALL, m_MessengerState.dwNewCallsCounter);
		else
			sNewCalls.FormatMessage(IDS_TRAY_ICON_TOOLTIP_X_NEW_CALLS, m_MessengerState.dwNewCallsCounter);

		m_TrayIcon.SetTooltipText(sNewCalls);
		m_TrayIcon.SetIcon(m_srtIcons.m_NewCalls);
		return;
	}
*/
	//
	//	ActivityHandlerInactiveEvent
	//	if not idle then inactive state will be set by low priority statuses
	if(m_MessengerState.bStatusChangedOnIdle == TRUE)	//todo, PRD to ask id this priority may be more then Animation ??? 
	{
		if(updateReason != SupporterChangedStatus)
		{
			m_TrayIcon.SetIcon(m_srtIcons.m_Away);
			m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_AWAY);

			if(m_MessengerState.bStatusChangedOnMissedCalls == TRUE)
			{
				m_TrayIcon.SetBalloonDetails(
					(CString)MAKEINTRESOURCE(IDS_TRAY_ICON_TOOLTIP_IS_OFFLINE_DUE_IDLE),
					(CString)MAKEINTRESOURCE(IDS_APP_FULL_NAME), CTrayNotifyIcon::Warning, 3);
			}

			return;
		}
		//	else if supporter selected status then we replace with this status below
	}

	if(m_MessengerState.bStatusChangedOnMissedCalls == TRUE)	//todo, PRD to ask id this priority may be more then Animation ??? 
	{
		if(updateReason != SupporterChangedStatus)
		{
			m_TrayIcon.SetIcon(m_srtIcons.m_Busy);
			m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_BUSY);
			return;
		}
		//	else if supporter selected status then we replace with this status below
	}

	if(m_MessengerState.bStatusChangedOnMaxHandled == TRUE)	//Priority not defined in PRD but seems to be more then 
	{
		if(updateReason != SupporterChangedStatus)
		{
			m_TrayIcon.SetIcon(m_srtIcons.m_Busy);
			m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_BUSY);
			return;
		}
		//	else if supporter selected status then we replace with this status below
	}

	if(m_MessengerState.bPowerStatusSuspended == TRUE)	//Priority not defined in PRD but seems to be more then 
	{
		m_TrayIcon.SetIcon(m_srtIcons.m_Busy);
		m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_BUSY);
		return;
		//	else if supporter selected status then we replace with this status below
	}
	
	switch(m_MessengerState.eSupporterSelectedStatus) // Back to Supporter's selected status Priority 4 - the lowest
	{
	 case StatusAvailible:
		 if(m_MessengerState.bIMChannelQualityIssueDetected==false)
		 {
			 m_TrayIcon.SetIcon(m_srtIcons.m_Available);
			 m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_AVAILIBLE);
		 }
		 else
		 {
			 m_TrayIcon.SetIcon(m_srtIcons.m_AvailableLimitedCon);
			 m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_AVAILIBLE_LIMITED_CON);
		 }
		 break;
	 case StatusBusy:
		 if(m_MessengerState.bIMChannelQualityIssueDetected==false)
		 {
			 m_TrayIcon.SetIcon(m_srtIcons.m_Busy);
			 m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_BUSY);
		 }
		 else
		 {
			 m_TrayIcon.SetIcon(m_srtIcons.m_BusyLimitedCon);
			 m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_BUSY_LIMITED_CON);
		 }
		 break;
	 case StatusAway:
		 if(m_MessengerState.bIMChannelQualityIssueDetected==false)
		 {
			 m_TrayIcon.SetIcon(m_srtIcons.m_Away);
			 m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_AWAY);
		 }
		 else
		 {
			 m_TrayIcon.SetIcon(m_srtIcons.m_AwayLimitedCon);
			 m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_AWAY_LIMITED_CON);
		 }
		break;
	 case StatusOffline:
		 if(m_MessengerState.bIMChannelQualityIssueDetected==false)
		 {
			m_TrayIcon.SetIcon(m_srtIcons.m_Offline);
			m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_OFFLINE);
		 }
		 else
		 {
			 m_TrayIcon.SetIcon(m_srtIcons.m_OfflineLimitedCon);
			 m_TrayIcon.SetTooltipText(IDS_TRAY_ICON_TOOLTIP_OFFLINE_LIMITED_CON);
		 }
 		 break;
	 case StatusOnnline4Customer:

		 CString sIconOnnline4CustToolTip;

		 if(m_MessengerState.bIMChannelQualityIssueDetected==false)
		 {
			m_TrayIcon.SetIcon(m_srtIcons.m_OnnlineForCustomer);
	
			sIconOnnline4CustToolTip.FormatMessage(IDS_TRAY_ICON_TOOLTIP_ONNLINE4CUSTOMER, 
				m_MessengerState.sOnnline4CustomerDisplayName.c_str());
		
			m_TrayIcon.SetTooltipText(sIconOnnline4CustToolTip);//todo here to add here customer's name
		 }
		 else
		 {
 			m_TrayIcon.SetIcon(m_srtIcons.m_OnnlineForCustomerLimitedCon);
			sIconOnnline4CustToolTip.FormatMessage(IDS_TRAY_ICON_TOOLTIP_ONNLINE4CUSTOMER_LIMITED_CON, 
				m_MessengerState.sOnnline4CustomerDisplayName.c_str());

			m_TrayIcon.SetTooltipText(sIconOnnline4CustToolTip);//todo here to add here customer's name
		 }
 		 break;
	 }

CATCH_LOG(_T("CSupportMessengerDlg::UpdateTrayIcon"))
}

LRESULT CSupportMessengerDlg::OnIMNewCall(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 

	CNodeSendMessagePtr	pcNodeSendMessage((CNodeSendMessage*)lParam);

	//this test message may be processed locally without sending to m_cHTMLInterface
	if(pcNodeSendMessage->GetSubject().compare(PROTOCOL_MSG_IM_CHANNEL_QUALITY_TEST)==0)
	{
		Log.Add(_CALL_,_T("PROTOCOL_MSG_IM_CHANNEL_QUALITY_TEST with subject: %s"),pcNodeSendMessage->GetBody().c_str());
		OnIMChannelQualityTestResponse(pcNodeSendMessage->GetBody().c_str());
		return 0L;
	}

	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnIMNewCall with subject: %s"), pcNodeSendMessage->GetSubject().c_str());

	m_cHTMLInterface.INTERFACE_UpdateData(
		pcNodeSendMessage->GetSubject().c_str(), pcNodeSendMessage->GetBody().c_str());

CATCH_LOG(_T("CSupportMessengerDlg::OnIMNewCall"))
	
	return 0L;
}

void CSupportMessengerDlg::OnPopupSingout()
{
TRY_CATCH 
	
	if(m_MessengerState.eConnectionState==stateConnected)
	{
		// todo for version without UI we have only alert indication, so the logic is deffer here
		// if there alerts than don't prompt on logout
		if((m_MessengerState.dwInSessionCallsCounter==0 && m_MessengerState.dwInSessionConsultCallCounter==0 &&
			m_mapAlerts.size()==0) || m_pcSupporterLocalData->getSettings()->bPromptOnItemsOnLogout==FALSE)
		{
			IMSignOut();
			m_MessengerState.bUserSelectedSignIn = FALSE;
			return;
		}

		if(m_MessengerState.dwInSessionCallsCounter!=0 || m_MessengerState.dwNewConsultCall!=0)
		{
			AfxMessageBox(IDS_INFO_CALLS_IN_INSESSION, MB_OK);
			m_MessengerState.bUserSelectedSignIn = TRUE;
			return;
		}

		if(m_MessengerState.dwNewCallsCounter!=0 || m_MessengerState.dwInSessionConsultCallCounter!=0)
		{
			if(AfxMessageBox(IDS_INFO_UNATTENDED_CALLS_IN_INBOX, MB_YESNO)==IDYES)
			{
				IMSignOut();
				m_MessengerState.bUserSelectedSignIn = FALSE;
			}
			else
			{
				//
				//	Supporter decided to stay SignedIn
				//
				m_MessengerState.bUserSelectedSignIn = TRUE;
			}
			return;
		}
	}
	else
	{
		m_MessengerState.dwIMConnectAttempt = 0;
		m_MessengerState.bUserSelectedSignIn = FALSE;
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnPopupSingout"))
}

HRESULT CSupportMessengerDlg::IMSignIn()
{
TRY_CATCH 

	if(m_MessengerState.eConnectionState==stateConnected)
	{
		Log.Add(_ERROR_,_T("CSupportMessengerDlg::IMSignIn(). Is not disconnected") );
		return 1;
	}

	m_MessengerState.bUserSelectedSignIn = TRUE;
	SetConnectionState(stateConnecting);

	//
	//	todo notify UI about conneciton progress
	//
	m_cHTMLInterface.INTERFACE_UpdateData(
		DHTML_INTERFACE_LOGIN_STATE, _T(""), SigninStatePairs[SIGNIN_STATE_INIT].m_iSignInFieldString); 
	m_cHTMLInterface.INTERFACE_UpdateData(
		DHTML_INTERFACE_LOGIN_STATE, _T(""), SigninStatePairs[SIGNIN_STATE_CONNECTING].m_iSignInFieldString); 
	m_cHTMLInterface.INTERFACE_UpdateData(
		DHTML_INTERFACE_LOGIN_STATE, _T(""), SigninStatePairs[SIGNIN_STATE_AUTHENTICATION].m_iSignInFieldString); 
	
	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::IMSignIn() CreateIMClient") );
	m_cCommunicator.CreateIMClient(
		m_MessengerState.sCurrentSupporterCrypt, 
		theApp.m_cSettings.m_sResource, 
		m_pcSupporterLocalData->getLoginInfo()->getPassword(),
		theApp.m_cSettings.m_sServer,  
		theApp.m_cSettings.m_sServerAddr, 
		this->m_hWnd, 
		theApp.m_cSettings.m_bLog,
		theApp.m_cSettings.m_dwIdleTimeout);

	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::IMSignIn() SendConnect") );

	struct _timeb		tstConnectionStartTime;
    _ftime_s( &tstConnectionStartTime ); 
	m_MessengerState.fConnectStartTime = tstConnectionStartTime.time + tstConnectionStartTime.millitm/1000.0; 
	
	//EndTimeSeconds = EndTime.time + EndTime.millitm/1000.00; 
	//m_MessengerState.bIMChannelQualityIssueDetected = FALSE;
	m_cCommunicator.EnqueueExecutionQueue(new CNodeSendConnect());

CATCH_LOG(_T("CSupportMessengerDlg::SignIn"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::IMSignOut()
{
TRY_CATCH

	m_MessengerState.bUserSelectedSignIn = FALSE;
	SetConnectionState(stateDisconnecting);

	IMSendUpdateStatus(StatusOffline); 
	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::IMSignOut() passed IMSendUpdateStatus(StatusOffline)") );

	m_cCommunicator.EnqueueExecutionQueue(new CNodeSendDisconnect()); 
	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::IMSignOut() passed CNodeSendDisconnect") );
		
	m_cCommunicator.DestroyIMClient();
	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::IMSignOut() passed DestroyIMClient") );

CATCH_LOG(_T("CSupportMessengerDlg ::SignOut"))
	return S_OK;
}
//
//  IMSendUpdateStatus look for IM protocol Status and Message by the SupporterStatus
//  send send it to Jubber Server
HRESULT CSupportMessengerDlg::IMSendUpdateStatus(SupporterStatus status)
{
TRY_CATCH 

	m_cCommunicator.EnqueueExecutionQueue(new CNodeUpdateStatus( 
		arrStatusPresencePairs[status].second.m_imStatus,
		arrStatusPresencePairs[status].second.m_imMsg));
	
	//
	//	TODO each time status changed to availible we have to request latest version number from server
	//	this is workaround for the case when broadcast is not sent to Expert that is not Onnline
	//
	if(status==StatusAvailible)
	{
		m_cCommunicator.EnqueueExecutionQueue(new CNodeSendMessage(
			(tstring)theApp.m_cSettings.m_sPendingSupportRequestsTo, 
			PROTOCOL_MSG_GET_EXPERTTOOLS_VERSION, 
			(tstring)_T(""), 
			(tstring)theApp.m_cSettings.m_sPendingSupportRequestsToResource));
	}

CATCH_LOG(_T("CSupportMessengerDlg ::IMSendUpdateStatus"))
    return S_OK;
}

void  CSupportMessengerDlg::SetConnectionState(IMConnectionState state)
{ 
TRY_CATCH 

	m_MessengerState.eConnectionState = state; 
	UpdateTrayIcon(ConnectionStateChanged);

CATCH_LOG(_T("CSupportMessengerDlg::SetConnectionState"))
}

void CSupportMessengerDlg::OnSettings()
{
TRY_CATCH 

	ShowSettingsWindow();

CATCH_LOG(_T("CSupportMessengerDlg::OnSettings"))
}

LRESULT CSupportMessengerDlg::OnActivityHandlerInActive(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 

	if(m_MessengerState.dwInSessionCallsCounter==0) // TODO temp workaround for NO UI limotations issue
	{
		m_MessengerState.bStatusChangedOnIdle = TRUE;
		IMSendUpdateStatus(StatusAway);
		UpdateTrayIcon(ActivityHandlerIdleEvent);

		if(m_pcAlertDlgMissedCalls!=NULL)
		{
			CString sMissedCallsAlert;
			sMissedCallsAlert.FormatMessage(IDS_MISSEDCALLS_OFFLINE_ALERT_TEXT, 
				m_pcSupporterLocalData->getSettings()->strShowAwayAfterBeingInActive.iAfterCounter);

			m_pcAlertDlgMissedCalls->UpdateAlertText(sMissedCallsAlert);
		}
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnActivityHandlerInActive"))
	return 0L;
}

LRESULT CSupportMessengerDlg::OnActivityHandlerBackActive(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 
	
	if(m_MessengerState.eSupporterSelectedStatus==StatusOnnline4Customer)
	{
		Log.Add(_MESSAGE_,_T("Expert back to be Active. Before idle expert status was online4customer"));
		m_MessengerState.eSupporterSelectedStatus = StatusAvailible;	
	}
	else
	{
		Log.Add(_MESSAGE_,_T("Expert back to be Active. Before idle expert status was NOT online4customer"));
	}

	Log.Add(_MESSAGE_,_T("Expert back to be Active, check missed and insession calls") );
	m_MessengerState.bStatusChangedOnIdle = FALSE;

	//
	//	The existing “Become offline after x minutes” should work as it is. 
    //  Once the relevant time elapsed, the expert status should be changed to offline 
	//  and the relevant message in the alert should be changed.  
	//	But when Expert back we do not change his status to Onnline if InSession or MissedCalls
	//
	if(m_MessengerState.dwInSessionCallsCounter==0 &&
	   m_MessengerState.bStatusChangedOnMissedCalls == FALSE) // TODO temp workaround for NO UI limotations issue
	{
		IMSendUpdateStatus(m_MessengerState.eSupporterSelectedStatus);
		UpdateTrayIcon(ActivityHandlerInactiveEvent);
	}
	else
	{
		Log.Add(_MESSAGE_,_T("Expert back to be Active, but have missed or insession calls") );
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnActivityHandlerBackActive"))
	return 0L;
}
//
//	StopActivityHandler		
//
void CSupportMessengerDlg::StopActivityHandler()
{
TRY_CATCH

	m_cActivityHandler.SetInActiveTimeout(FALSE, FALSE, 0);

CATCH_LOG(_T("CSupportMessengerDlg::StopActivityHandler"))
}

//
//	RestartActivityHandler when Expert selected sigin		
//
void CSupportMessengerDlg::RestartActivityHandler()
{
TRY_CATCH

	if(m_MessengerState.bUserSelectedSignIn == TRUE)
	{
		m_cActivityHandler.SetInActiveTimeout(
			m_pcSupporterLocalData->getSettings()->bShowAwayWhenScreenSaverIsOn,
			m_pcSupporterLocalData->getSettings()->strShowAwayAfterBeingInActive.bActivated,
			m_pcSupporterLocalData->getSettings()->strShowAwayAfterBeingInActive.iAfterCounter*1000*60);
	}

CATCH_LOG(_T("CSupportMessengerDlg::RestartActivityHandler"))
}

HRESULT CSupportMessengerDlg::OnLoginPageStart(IHTMLElement* pElement)
{
TRY_CATCH 

	CString		sLoginEmail;
	CString		sPassword;
	CString		sLoginStatus;
	BOOL		bLoginRememberMe = FALSE;
	VARIANT		tmpData;
	sErrorInfo  errInfo;

	//
	//	1)  Parse and validate parameters from login page
	//
	if(pElement!=NULL)
	{
		pElement->getAttribute(L"sLoginEmail", 0, &tmpData);
		sLoginEmail = (CString)tmpData;

		//
		//	todo - add here Validation of parameter (lenght @ and so on) - Osnat for requirements 
		//	todo DHTML to agree that will not check 
		if(sLoginEmail.GetLength()== 0)
		{	
			//todo actually may be show instead of update
			//	prepeare struct to costruct ErrorMsg
			errInfo.m_errSrcType = ErrSrcSupCenterApp;
			errInfo.m_resIDforDescr = IDS_ERROR_USERNAME_MISSED;
			errInfo.m_errCode = 0;
			errInfo.m_errDetailsCode = 0;
			// show logout window with formated text
			ShowLogoutWindow(&errInfo);
			return 0;
		}

		//
		//	todo - additional attribute will indicate if Password specified or stored password may be used
		//
		pElement->getAttribute(L"sPassword", 0, &tmpData);	
		sPassword = (CString)tmpData;

		pElement->getAttribute(L"sLoginRememberMe", 0, &tmpData);	
		bLoginRememberMe = abs((BOOL)tmpData.iVal);

		//pElement->getAttribute( L"sLoginStatus", 0, &tmpData);	
		//sLoginStatus = (CString)tmpData;
	}

	//
	//	2)  Save last loginedIn Entry (todo - skipp this if it is reconnect)
	//	
	m_cMessengerLocalData.getLastLogiedInEntry()->setSupporterId(sLoginEmail.GetBuffer()); 
	m_cMessengerLocalData.getLastLogiedInEntry()->Save();

	
	//
	//	3)  Reload current supporter's data MakeLower as required
	//
	m_MessengerState.sCurrentSupporter = sLoginEmail.MakeLower().GetBuffer();
	ReloadCurrentSupporterData(); //for new supporter that now current

	//
	//	4)	Crypt Supporter username with SHA1
	//
	char  hashBuf[SHA1Size + 1] = {0};
	m_crypt.MakeHash(
		(char*)m_MessengerState.sCurrentSupporter.c_str(), 
		(int)strlen((char*)m_MessengerState.sCurrentSupporter.c_str()), 
		hashBuf);

	char szReport[MAX_USERNAME_LEN] = {0};
	m_crypt.ReportHash((unsigned __int8*)hashBuf, szReport, MAX_USERNAME_LEN);
	Log.Add(_MESSAGE_,_T("CurrentSupporter SHA1 %s"), szReport);
	
	m_MessengerState.sCurrentSupporterCrypt = szReport;//TODO
	//m_MessengerState.sCurrentSupporterCrypt = m_MessengerState.sCurrentSupporter;//TODO

/*	todo - MakeHashStr is not working properly 
	char szReport2[1024];
	szReport2[0] = 0;
	char  hashBuf2[SHA1Size + 1] = {0};
	crypt.MakeHashStr(m_MessengerState.sCurrentSupporter.c_str(), hashBuf2, SHA1Size);
	Log.Add(_MESSAGE_, _T("MakeHashStr() function call with SHA1 Result: %s"), hashBuf2);
	ReportHash((unsigned __int8*)hashBuf2, szReport2, REPORT_HEX);
	AfxMessageBox(szReport2);
*/
	//
	//	5)  Remember/Forget password due latest selected 
	//

	if(sPassword.GetLength()==0)	
	{
		//	todo actually may be show instead of update
		//	prepeare struct to costruct ErrorMsg
		errInfo.m_errSrcType = ErrSrcSupCenterApp;
		errInfo.m_resIDforDescr = IDS_ERROR_PASSWORD_MISSEED;
		errInfo.m_errCode = 0;
		errInfo.m_errDetailsCode = 0;
		// show logout window with formated text
		ShowLogoutWindow(&errInfo);
		return 0;
	}
	else
	{
		//
		//	use password if password field was Edited
		if(sPassword.Compare(DEFAULT_LOGINPAGE_PASSWORD)!=0 )//todo with DHTML better
			m_pcSupporterLocalData->getLoginInfo()->setPassword(sPassword.GetBuffer());	

		if(m_pcSupporterLocalData->getLoginInfo()->getPassword().size() == 0)
		{
			Log.Add(_ERROR_,_T("Password is not specified and remember me never done for specified username"));

			//	todo actually may be show instead of update
			//	prepeare struct to costruct ErrorMsg
			errInfo.m_errSrcType = ErrSrcSupCenterApp;
			errInfo.m_resIDforDescr = IDS_ERROR_AUTHENTICATION_FAILED;
			errInfo.m_errCode = 0;
			errInfo.m_errDetailsCode = 0;
			// show logout window with formated text
			ShowLogoutWindow(&errInfo);
			return 0;
		}

		m_pcSupporterLocalData->getLoginInfo()->setRememberMe(bLoginRememberMe);

		m_pcSupporterLocalData->getLoginInfo()->Save();
	}
	
	//
	//	6)	SignIn with the information specified in LoginPage 
	//
	if(IMSignIn()!= S_OK)
	{
		//todo actually may be show instead of update
		errInfo.m_errSrcType = ErrSrcSupCenterApp;
		errInfo.m_resIDforDescr = IDS_ERROR_ALREADY_SIGNEDIN;
		errInfo.m_errCode = 0;
		errInfo.m_errDetailsCode = 0;
		// show logout window with formated text
		ShowLogoutWindow(&errInfo);
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnLoginPageStart"))
	return S_OK;
}

void CSupportMessengerDlg::OnStatusAvailible()
{	
	SetSupporterSelectedStatus(StatusAvailible);
}

void CSupportMessengerDlg::OnStatusAway()
{
	SetSupporterSelectedStatus(StatusAway);
}

void CSupportMessengerDlg::OnStatusBusy()
{
	SetSupporterSelectedStatus(StatusBusy);
}

void CSupportMessengerDlg::OnStatusOnnlineForCustomer()
{	
	//todo http://srv-filer/confluence/display/cmpt/Online+for+Customer
	//m_TrayIcon.SetIcon(m_srtIcons.m_OnnlineForCustomer); may be set on Submit called in the COnline4CustomerDlg
	long lUid = 0;
	CString sOtpUrlPart;	
	CString	sFullUrl;	
	tstring	sSupporterID = m_MessengerState.sCurrentSupporterCrypt;

	sOtpUrlPart.FormatMessage(URL_ONLINE4_FORCUSTOMER, sSupporterID.c_str());
	sFullUrl = theApp.m_cSettings.m_sBaseUrlSupporterInbox + sOtpUrlPart;
	
	eCallType callType = OfflineMsg;
	CString sCustomerDisplayName = "Offline Message";

	if(m_cOnline4CustomerDlg!=NULL)
		return;

	m_cOnline4CustomerDlg = new COnline4CustomerDlg(
		GetDesktopWindow(), 
		sFullUrl, 
		lUid, 
		this->m_hWnd,
		callType,
		sCustomerDisplayName);
}

void CSupportMessengerDlg::SetSelectedStatus(SupporterStatus status)
{
TRY_CATCH 
	
	m_MessengerState.eSupporterSelectedStatus = status;
	if(m_MessengerState.eConnectionState==stateConnected) //seems always will be TRUE
	{
		IMSendUpdateStatus(status);
		UpdateTrayIcon(SupporterChangedStatus);
	}

	//
	//	reset missed calls counter in this case
	//
	m_MessengerState.dwMissedCallsCounter = 0;
	m_MessengerState.bStatusChangedOnMissedCalls = FALSE;

	CloseMissedCallsAlert();

CATCH_LOG(_T("CSupportMessengerDlg::SetSelectedStatus"))
}


void CSupportMessengerDlg::SetSupporterSelectedStatus(SupporterStatus status)
{
TRY_CATCH 
	m_MessengerState.bStatusChangedOnStargateBusy = FALSE;
	SetSelectedStatus(status);

CATCH_LOG(_T("CSupportMessengerDlg::SetSupporterSelectedStatus"))
}

void CSupportMessengerDlg::CustomizeTrayMenu(CMenu* pMenu)
{
	MENUITEMINFO info;
	info.cbSize = sizeof (MENUITEMINFO); // must fill up this field
    info.fMask = MIIM_STATE;             // get the state of the menu item

	//
	//	version wuthout UI - no OPEN Item
	//
	Log.Add(_MESSAGE_, (_T("MenuOpenedByTray == FALSE")));
	pMenu->RemoveMenu(ID_POPUP_OPEN, MF_BYCOMMAND);//we want to remove this 

/*
	//
	//	version without UI - no OPEN Item	
	//
	//	Modify TrayMenu ID_POPUP_OPEN depending on eTrayMenuOpenReason and dwNewCallsCounter
	//
	if(m_MessengerState.eTrayMenuOpenReason == TrayIcon)
	{
		Log.Add(_MESSAGE_,(_T("MenuOpenedByTray == TRUE")));
		// anatoly todo "Open" may be replaced with "Tabs View"  Wait for final GUI to fix this 
		CString		sNewCalls;
		switch(m_MessengerState.dwNewCallsCounter)
		{
		case 0:
			sNewCalls.FormatMessage(IDS_MENU_ITEM_OPEN_TEXT);
			break;
		case 1:
			sNewCalls.FormatMessage(IDS_MENU_ITEM_OPEN_TEXT_ONE_NEW_CALL, m_MessengerState.dwNewCallsCounter);			
			break;
		default:	
			sNewCalls.FormatMessage(IDS_MENU_ITEM_OPEN_TEXT_X_NEW_CALLS, m_MessengerState.dwNewCallsCounter);
			break;
		}

		if(pMenu->ModifyMenu(ID_POPUP_OPEN, MF_BYCOMMAND, ID_POPUP_OPEN, sNewCalls)==FALSE)
		{
			pMenu->InsertMenu(0, MF_BYPOSITION | MF_DEFAULT, ID_POPUP_OPEN, sNewCalls);
			pMenu->SetDefaultItem(ID_POPUP_OPEN);
		}
	}
	else
	{
		Log.Add(_MESSAGE_, (_T("MenuOpenedByTray == FALSE")));
		pMenu->RemoveMenu(ID_POPUP_OPEN, MF_BYCOMMAND);//we want to remove this 
	}
*/
	
	//
	//	TODO - not for beta	Dock is grayed in Betta
	//	Modify ID_DOCK_LEFT/ID_DOCK_RIGHT checked sate of the button 
	//  
	UINT uState = GetState();
	switch( uState )
	{
	case ABE_LEFT:
		Log.Add(_MESSAGE_, (_T("CheckMenuItem ABE_LEFT")));
		pMenu->CheckMenuItem(ID_DOCK_LEFT, MF_BYCOMMAND | MF_CHECKED);
		pMenu->CheckMenuItem(ID_DOCK_RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
		break;
	case ABE_RIGHT:
		Log.Add(_MESSAGE_, (_T("CheckMenuItem ABE_RIGHT")));
		pMenu->CheckMenuItem(ID_DOCK_RIGHT, MF_BYCOMMAND | MF_CHECKED);
		pMenu->CheckMenuItem(ID_DOCK_LEFT, MF_BYCOMMAND | MF_UNCHECKED);
		break;
	default:
		Log.Add(_MESSAGE_, (_T("CheckMenuItem UNCHECKED")));
		pMenu->CheckMenuItem(ID_DOCK_LEFT, MF_BYCOMMAND | MF_UNCHECKED);
		pMenu->CheckMenuItem(ID_DOCK_RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
		break;
	}

	//
	//	TODO - if supporter not specified in LoginPage no settings page to display
	//
	if(m_pcSupporterLocalData==NULL)
		pMenu->EnableMenuItem(ID_POPUP_SETTINGS, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND); 
	else
		pMenu->EnableMenuItem(ID_POPUP_SETTINGS, MF_ENABLED | MF_BYCOMMAND); 

	//
	//	Modify ID_POPUP_SINGOUT items depending on connection state 
	//
	switch(m_MessengerState.eConnectionState)
	{
	case stateConnected:
		Log.Add(_MESSAGE_, (_T("Messnger is in connected state")));
		//
		//	If Update required then almost the same disabling like when disconnected
		//
		if(m_MessengerState.bUpdateRequired == TRUE)
		{
			Log.Add(_MESSAGE_, (_T("Messenger is in UPDATE required mode")));
			UpdateGroupOfItems(pMenu, FALSE);
		}
		else
		{
			UpdateGroupOfItems(pMenu, TRUE);
		}

		pMenu->RemoveMenu(ID_POPUP_SIGNIN, MF_BYCOMMAND);//we want to remove this 

		if(pMenu->GetMenuItemInfo(ID_POPUP_SINGOUT, &info)==FALSE) //if not exists we have to add menu
			pMenu->InsertMenu(ID_POPUP_EXIT, MF_BYCOMMAND | MF_DEFAULT, ID_POPUP_SINGOUT, (CString)MAKEINTRESOURCE(IDS_MENU_ITEM_SIGNOUT_TEXT));
		break;
	case stateConnecting:	//todo disable signin/sogout
	case stateWaitAutoReconnect:
		Log.Add(_MESSAGE_, (_T("Messenger is in stateConnecting")));
		pMenu->EnableMenuItem(ID_POPUP_SIGNIN, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND); 
	case stateDisconnected:
	case stateDisconnecting:
	default:
		Log.Add(_MESSAGE_, (_T("Messenger is NOT in connected State")));
		UpdateGroupOfItems(pMenu, FALSE);

		pMenu->RemoveMenu(ID_POPUP_SINGOUT, MF_BYCOMMAND); 
		
		if(pMenu->GetMenuItemInfo(ID_POPUP_SIGNIN, &info)==FALSE)//if not exists we have to add menu
			pMenu->InsertMenu(ID_POPUP_EXIT, MF_BYCOMMAND | MF_DEFAULT, ID_POPUP_SIGNIN, (CString)MAKEINTRESOURCE(IDS_MENU_ITEM_SIGNIN_TEXT));

		break;
	}
}

void CSupportMessengerDlg::UpdateGroupOfItems(CMenu* pMenu, BOOL bEnable)
{
	UINT nEnable = 0;

	if(bEnable == TRUE)
		nEnable = MF_ENABLED;
	else
		nEnable = MF_GRAYED | MF_DISABLED;

	/*
	//	todo rnot for veriosn without UI
	//	Enable/Disable menus without SubMenus by command 
	//
	pMenu->EnableMenuItem(ID_POPUP_VIEWHISTORY, nEnable | MF_BYCOMMAND); 
	pMenu->EnableMenuItem(ID_POPUP_CLEAREVENTS, nEnable | MF_BYCOMMAND); 
	*/
	pMenu->RemoveMenu(ID_POPUP_VIEWHISTORY, MF_BYCOMMAND); 
	pMenu->RemoveMenu(ID_POPUP_CLEAREVENTS, MF_BYCOMMAND); 

	//
	//	Find all Menues with with SubMenu and Enable/Disable by Position - No CommandID for them
	// 
	for(int iPos = 0; iPos < (int)pMenu->GetMenuItemCount(); iPos++)
	{
		CMenu* pSubMenu = pMenu->GetSubMenu(iPos);
		if(pSubMenu != NULL) //found one of subManues
		{
			for (int j = 0; j <= MAX_NUMBER_OF_SUB_MENUS; j++)
			{
				switch(pSubMenu->GetMenuItemID(j))
				{
				case ID_LINKS_MYACCOUNT:
				//case ID_HELP_HELP:
				//case ID_DOCK_RIGHT || - todo not for beta
					pMenu->EnableMenuItem(iPos, nEnable | MF_BYPOSITION); 
					break;
				case IDI_MYSTATUS_AVAILIBLE:
					if(m_MessengerState.dwInSessionCallsCounter!=0)
						pMenu->EnableMenuItem(iPos, MF_GRAYED | MF_DISABLED | MF_BYPOSITION); 
					else 				
						pMenu->EnableMenuItem(iPos, nEnable | MF_BYPOSITION); 
					break;
				default:
					break;
				}
			}
		}
	}
}
void CSupportMessengerDlg::OnPopupSignin()
{
TRY_CATCH 
	//	TODO - what we have to do
	//  just SignIn or Using Login Page - seems always usign LoginPage
	//	IMSignIn();

	//
	//	update login window
	//
	UpdateLoginWindow();

	//
	//	start signin 
	//	
	m_cHTMLInterface.INTERFACE_UpdateData(DHTML_INTERFACE_LOGIN_START, _T("")); 

CATCH_LOG(_T("CSupportMessengerDlg::OnPopupSignin"))
}

LRESULT CSupportMessengerDlg::OnAlertDlgUpdateClose(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 

	m_pcAlertDlgUpdate->DestroyModeless(); //call delete in the PostNcDestroy
	m_pcAlertDlgUpdate = NULL;

CATCH_LOG(_T("CSupportMessengerDlg::OnAlertDlgUpdateClose"))
	return 0L;
}

LRESULT CSupportMessengerDlg::OnAlertDlgUpdatePickUp(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 
	//
	//	Opens URL for download 
	//
	CString sOtpUrlPart;	
	CString	sFullUrl;	
	tstring	sSupporterID = m_MessengerState.sCurrentSupporterCrypt;

	//
	//	TODO
	//	http://dep-apache.supportspace.com/rcp/4.0.44.5/
	//  m_sBaseUrlUpdateDownload
	//  http://dep-apache.supportspace.com
	//  m_sMinRequiredVersion
	//  http://dep-apache.supportspace.com/rcp/4.0.44.5/SupportSpaceExpertSupportTools.msi
	sOtpUrlPart.FormatMessage(URL_DOWNLOAD_UPDATE_SPRINT4, m_sMinRequiredVersion);
	sFullUrl = theApp.m_cSettings.m_sBaseUrlUpdateDownload + sOtpUrlPart + _T("/")+ URL_DOWNLOAD_MSI_NAME;

	//m_cUrlOpener.LauchIEBrowser(sFullUrl);
	m_cUrlOpener.Open(sFullUrl);
	//
	//	Close dialog 
	//
	m_pcAlertDlgUpdate->DestroyModeless(); //call delete in the PostNcDestroy
	m_pcAlertDlgUpdate = NULL;
	
CATCH_LOG(_T("CSupportMessengerDlg::OnAlertDlgUpdatePickUp"))
	return 0L;
}

HRESULT CSupportMessengerDlg::OnUpdateRequired()
{
TRY_CATCH
	//	
	//	OnUpdate we need to close all Alerts first (May we also disconnect ? not in PRD)
	//
	CloseAllAlerts();
	
	//
	//	TODO Notify DHTML GUI to disable all
	//

	//
	//	Change SupporterStatus to busy - TODO ask PRD if it is not better to disconnect and require relogin after update
	//
	m_MessengerState.bUpdateRequired = TRUE; //todo to change this to False somewhere
	UpdateTrayIcon(UpdateRequiredStateChanged);
	IMSendUpdateStatus(StatusBusy);

	// we need to set m_lFreePosition to 1 in order to show offline messages alert at correct position
	m_lFreePosition = 1; 
	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnUpdateRequired FreePosition %d"), m_lFreePosition);
	
	//
	//	Create new alert dialog with Update information
	//
	if(m_pcAlertDlgUpdate==NULL)
	{
		m_pcAlertDlgUpdate = new CAlertDlgUpdate(
			CWnd::FromHandle(this->m_hWnd),
			NULL, /*pcTransparentWindow - version without transparent border*/ 
			NULL, /*hWndParent*/ 
			NULL, /*pParent*/ 
			theApp.m_cSettings.m_eGUIlocation, 
			theApp.m_cSettings.m_sGUIlocationFilePath, 
			1);
	}

	//
	//	TODO Start Monitoring for UpdateCompleted Event 
	//	
	m_cUpdateMonitor.Start(this->m_hWnd, m_sMinRequiredVersion);

CATCH_LOG(_T("CSupportMessengerDlg::OnUpdateRequired"))
	return S_OK;
}

void CSupportMessengerDlg::OnTimer(UINT nIDEvent)
{
	switch(nIDEvent)
	{
	case RECONNECT_TIMER_ID:
		KillTimer(RECONNECT_TIMER_ID);
		IMSignIn();
		return;
	default:
		break;
	}

	return CAppBar::OnTimer(nIDEvent);
}

LRESULT CSupportMessengerDlg::OnUpdateCompleted(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 

	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnUpdateCompleted with code %d"), (DWORD)lParam);
	//
	//
	//
	
	//
	//	
	//
	m_MessengerState.bUpdateRequired = FALSE;
	IMSendUpdateStatus(m_MessengerState.eSupporterSelectedStatus);
	UpdateTrayIcon(RollbackToSupporterSelectedStatus);
	
	//
	//	TODO Notify JavaScript that update completed - not for closed beta	
	//
	AfxMessageBox(IDS_INFO_MSG_UPDATE_SUCCESSFULLY_COMPLETED);

CATCH_LOG(_T("CSupportMessengerDlg::OnUpdateCompleted"))
	return 0L;
}

HRESULT CSupportMessengerDlg::OnNewCallPicked(long lCallUID)
{
TRY_CATCH

	//
	//	Decrement counter if new call deleted from Inbox 
	if(lCallUID >0 && m_MessengerState.dwNewCallsCounter>0)
		m_MessengerState.dwNewCallsCounter--;

	//
	//	Update tray
	UpdateTrayIcon(NewCallsCounterChanged);

	//
	//	Close Alert if availible
	//
	if(lCallUID!=0)
	{
		//Close alert and closeAllAlerts is nto correct. See STL-338
		Log.Add(_MESSAGE_,_T("Close all new call requests"));
		CloseAllNewCallAlerts();
	}

	//
	//	Stop animation if counter 0
	if(m_MessengerState.dwNewCallsCounter == 0)
	{
		m_MessengerState.bAnimateTray = FALSE;
		UpdateTrayIcon(NewCallsCounterChanged);
	}

	//   Reset missed calls counter
	m_MessengerState.dwMissedCallsCounter = 0;				
	m_MessengerState.bStatusChangedOnMissedCalls = FALSE;

	//	 reset also StatusOnnline4Customer
	if(m_MessengerState.eSupporterSelectedStatus == StatusOnnline4Customer)
		m_MessengerState.eSupporterSelectedStatus = StatusAvailible;

CATCH_LOG(_T("CSupportMessengerDlg::OnNewCallPicked"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnRequestStatusChanged(IHTMLElement *pElement)
{
TRY_CATCH 

	long	lCallUID = 0;
	VARIANT	tmpCallUID;
	VARIANT	tmpState;
	VARIANT tmpStayOnline4Customer;
	VARIANT tmpDisplayUserName;
	CString	sState;
	BOOL	bStayOnline4Customer = false;
	CString	sDisplayUserName;
	
	pElement->getAttribute( L"iCallUID", 0, &tmpCallUID);
	if(tmpCallUID.vt != VT_NULL)
		lCallUID = tmpCallUID.lVal;
	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnRequestStatusChanged Attribute iCallUID: %d"), lCallUID);

	pElement->getAttribute( L"sState", 0, &tmpState);
	if(tmpState.vt != VT_NULL)
		sState = (CString)tmpState;
	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnRequestStatusChanged Attribute sState: %s"), sState);

	if(sState.Compare(_T("PICKED"))== 0)
	{
		if(m_MessengerState.bIMChannelQualityIssueDetected == TRUE)
			theApp.m_conQualityLog.AddMessage(cLog::_MESSAGE,Format(_T("Session workflowid: %d started during a poor connection"), lCallUID ));					

		m_MessengerState.dwInSessionCallsCounter++;
		OnNewCallPicked(lCallUID); 
	}
	else
	if(sState.Compare(_T("ENDED"))== 0 || sState.Compare(_T("LEFT_OPEN"))== 0)
	{
		//additional flag for ENDED and LEFT_OPEN must be parsed
		pElement->getAttribute( L"bStayOnline4Customer", 0, &tmpStayOnline4Customer);
		if(tmpStayOnline4Customer.vt != VT_NULL)
			bStayOnline4Customer = abs((BOOL)tmpStayOnline4Customer.boolVal);
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnRequestStatusChanged Attribute bStayOnline4Customer: %d"), bStayOnline4Customer);

		//additional parameter for bStayOnline4Customer to know what CustomerName
		pElement->getAttribute( L"sDisplayUserName", 0, &tmpDisplayUserName);
		if(tmpStayOnline4Customer.vt != VT_NULL)
			sDisplayUserName = (CString)tmpDisplayUserName;
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnRequestStatusChanged Attribute sDisplayUserName: %s"), sDisplayUserName);

		OnInSessionCallStatusChanged(lCallUID, bStayOnline4Customer);
	}
	else
	{
		//  Missed call:     "state":"TIMEOUT"
		//  Canceled call:   "state":"CANCELED"
		//
		//	Just Close Alert if availible
		//
		if(lCallUID!=0)
		{
			Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnRequestStatusChanged iCallUID %d. CloseAlert on TIMEOUT or CANCELED"), lCallUID);
			CloseAlert(lCallUID);
		}

		//
		//  Missed calls should not be counted if the expert is in a session 
		//  (I know it’s not a realistic situation, but it can happen)
		//
		if(sState.Compare(_T("TIMEOUT"))== 0)
		{
			if(m_MessengerState.dwInSessionCallsCounter==0)
			{
				m_MessengerState.dwMissedCallsCounter++;
				OnMissedCallsCounterChnanged();
				return S_OK;
			}
			else
			{
				Log.Add(_WARNING_,_T("Missed Call received, but Expert have InSession calls counter"));
			}
		}

		//
		// if previous status was Onnline4Customer then keep it Onnline4Customer	
		//
		if(m_MessengerState.eSupporterSelectedStatus == StatusOnnline4Customer)
		{
			Log.Add(_MESSAGE_,_T("Session CANCELED or TIMEOUT but status was and stay online4customer %s"), m_MessengerState.sOnnline4CustomerDisplayName.c_str());
			return S_OK;
		}
	}

	//
	//	todo - for mulltiply session this will be differ
	//	since sprint 4. Excell ID:25
	//	
	//  Change status to busy once a call is connected (click on Start). 
	//	This doesn’t apply when the expert is in Consult/Chat mode TODO
	//
	//	Changes the expert’s status to busy once a set number of sessions are ongoing
	
	// todo this is commeneted for version without UI
	//if(m_pcSupporterLocalData->getSettings()->strHandleCallsAndThenDisplayBusy.bActivated  == TRUE &&
	//   m_pcSupporterLocalData->getSettings()->strHandleCallsAndThenDisplayBusy.iAfterCounter <= m_MessengerState.dwInSessionCallsCounter)
	if(m_MessengerState.dwInSessionCallsCounter)
	{
		Log.Add(_MESSAGE_,_T("Changes the expert’s status to busy once a set number of sessions are ongoing"));
		m_MessengerState.bStatusChangedOnMaxHandled = TRUE;
		IMSendUpdateStatus(StatusBusy);
		UpdateTrayIcon(HandledMaxNumberOfCalls);		
	}

	// todo this is commeneted for version without UI
	//	Changes the expert’s status from Busy StatusChangedOnMaxHandled and now number of calls is less
	//if(m_MessengerState.bStatusChangedOnMaxHandled &&
	//   m_pcSupporterLocalData->getSettings()->strHandleCallsAndThenDisplayBusy.iAfterCounter > m_MessengerState.dwInSessionCallsCounter)
	if(m_MessengerState.dwInSessionCallsCounter==0)
	{
		if(bStayOnline4Customer==true)
		{
			Log.Add(_MESSAGE_,_T("StayOnline4Customer is true..."));	
			m_MessengerState.eSupporterSelectedStatus = StatusOnnline4Customer;
			m_MessengerState.sOnnline4CustomerDisplayName = sDisplayUserName;
			m_MessengerState.bStatusChangedOnMaxHandled = FALSE;
			SetStargateSelectedStatus(StatusOnnline4Customer);
		}
		else
		{
			Log.Add(_MESSAGE_,_T("Changes the expert’s status from Busy StatusChangedOnMaxHandled and now number of InSessionCalls is less"));
			m_MessengerState.eSupporterSelectedStatus = StatusAvailible;//after session we back expert to onlne always to avoid issue after StayOnline4Customer 			
			m_MessengerState.bStatusChangedOnMaxHandled = FALSE;
			IMSendUpdateStatus(m_MessengerState.eSupporterSelectedStatus);
			UpdateTrayIcon(BackFromHandledMaxNumberOfCalls);		
		}
	}
	
CATCH_LOG(_T("CSupportMessengerDlg::OnRequestStatusChanged"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnInSessionCallStatusChanged(long lCallUID, BOOL bStayOnline4Customer)
{
TRY_CATCH

	//
	//	Decrement counter if new call deleted from Inbox 
	if(lCallUID >0 && m_MessengerState.dwInSessionCallsCounter>0)
	{
		m_MessengerState.dwInSessionCallsCounter = 0;//TODO refactoring needed for multisession support
		//m_MessengerState.dwInSessionCallsCounter--;
	}

	//
	//	Close Alert if availible
	//
	if(lCallUID!=0)
	{
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnRequestStatusChanged iCallUID %d. CloseAlert"), lCallUID);
		CloseAlert(lCallUID);
	}

	//
	//	Update tray
	UpdateTrayIcon(InSessionCallsCounterChanged);

CATCH_LOG(_T("CSupportMessengerDlg::OnInSessionCallStatusChanged"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnOpenLink(IHTMLElement *pElement)
{
TRY_CATCH

	VARIANT	tmpLink;
	CString	sLink;
	
	pElement->getAttribute( L"sLink", 0, &tmpLink);
	if(tmpLink.vt != VT_NULL)
	{
		sLink = (CString)tmpLink;
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnOpenLink Attribute sLink: %s"), sLink);
		m_cUrlOpener.Open(sLink);
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnOpenLink"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnSettingsClose(IHTMLElement *pElement)
{
TRY_CATCH
	//if opened from main window menu and disconnected then show logoutwindow
	if(m_MessengerState.eTrayMenuOpenReason == TrayIcon || m_MessengerState.eConnectionState == stateConnected)  
	{
		SetState(ABE_FLOAT);//todo
		ShowWindow(SW_HIDE);
	}
	else
	{
		ShowLogoutWindow(NULL);
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnSettingsClose"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnSettingsOk(IHTMLElement *pElement)
{
TRY_CATCH

	Log.Add(_MESSAGE_, _T("CSupportMessengerDlg::OnSettingsOk"));
	VARIANT	sTempData;

	BOOL	cOnIncomingCallsShowTrayMessage = FALSE;
	BOOL	cAutomaticallyRun = FALSE; 
	BOOL	cOpenMainWindowOnWindowsStartUp = FALSE;
	BOOL	cShowAway = FALSE;
	CString	sShowAway = FALSE;
	BOOL	cHadndleCallsDisplayBusy = FALSE;
	CString	sHadndleCallsDisplayBusy = FALSE;
	BOOL	cOnIncomingCallsAnimateTrayIcon = FALSE;
	BOOL	cPromptOnItemsOnLogout = FALSE;
	BOOL	cPromptAboutSnoozingItemsOnLogout = FALSE;
	BOOL	cPlaySoundUponIncomingCall = FALSE;
	BOOL	cPlaySoundUponConnectingToCustomer = FALSE;
	CString	sDisplayItemsAtTime = FALSE;
	BOOL	cPortOpened = FALSE;
	CString	sPortOpened;
	BOOL	cScreenSaverIsOn;

	if(pElement==NULL)
	{
		Log.Add(_ERROR_, _T("CSupportMessengerDlg::OnSettingsOk failed with pElement==NULL"));
		return S_FALSE;
	}

	pElement->getAttribute(L"cOnIncomingCallsShowTrayMessage", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		cOnIncomingCallsShowTrayMessage = abs((BOOL)sTempData.boolVal);

	pElement->getAttribute(L"cAutomaticallyRun", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		cAutomaticallyRun = abs((BOOL)sTempData.iVal);

	pElement->getAttribute(L"cOpenMainWindowOnWindowsStartUp", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		cOpenMainWindowOnWindowsStartUp = abs((BOOL)sTempData.iVal);

	pElement->getAttribute(L"cShowAway", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		cShowAway = abs((BOOL)sTempData.iVal);

	pElement->getAttribute(L"sShowAway", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		sShowAway = (CString)sTempData;

	pElement->getAttribute(L"cHadndleCallsDisplayBusy", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		cHadndleCallsDisplayBusy = abs((BOOL)sTempData.iVal);

	pElement->getAttribute(L"sHadndleCallsDisplayBusy", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		sHadndleCallsDisplayBusy = (CString)sTempData;
	
	pElement->getAttribute(L"cOnIncomingCallsShowTrayMessage", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		cOnIncomingCallsShowTrayMessage = abs((BOOL)sTempData.iVal);
	
	pElement->getAttribute(L"cOnIncomingCallsAnimateTrayIcon", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		cOnIncomingCallsAnimateTrayIcon = abs((BOOL)sTempData.iVal);

	pElement->getAttribute(L"cPromptOnItemsOnLogout", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		cPromptOnItemsOnLogout = abs((BOOL)sTempData.iVal);
	
	pElement->getAttribute(L"cPromptAboutSnoozingItemsOnLogout", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		cPromptAboutSnoozingItemsOnLogout = abs((BOOL)sTempData.iVal);
	
	pElement->getAttribute(L"cPlaySoundUponIncomingCall", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		cPlaySoundUponIncomingCall = abs((BOOL)sTempData.iVal);
	
	pElement->getAttribute(L"cPlaySoundUponConnectingToCustomer", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		cPlaySoundUponConnectingToCustomer = abs((BOOL)sTempData.iVal);

	pElement->getAttribute(L"sDisplayItemsAtTime", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		sDisplayItemsAtTime = (CString)sTempData;

	pElement->getAttribute(L"cPortOpened", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		cPortOpened = abs((BOOL)sTempData.iVal);

	pElement->getAttribute(L"sPortOpened", 0, &sTempData);
	if(sTempData.vt != VT_NULL)
		sPortOpened = (CString)sTempData;

	pElement->getAttribute(L"cScreenSaverIsOn", 0, &sTempData);	
	if(sTempData.vt != VT_NULL)
		cScreenSaverIsOn = abs((BOOL)sTempData.iVal);

	if(m_pcSupporterLocalData != NULL)
	{
		//
		//	validate input parameters
		//
		int iShowAway = _tstoi(sShowAway); 
		if(iShowAway > 60 || iShowAway < 1 )
		{
			AfxMessageBox(IDS_ERROR_SETTINGS_SHOWAWAY_NOT_CORRECT);
			return S_FALSE;
		}

		m_pcSupporterLocalData->getSettings()->bAutomaticallyRun = cAutomaticallyRun;
		m_pcSupporterLocalData->getSettings()->bOpenMainWindowOnMessangerStartUp = cOpenMainWindowOnWindowsStartUp;

		m_pcSupporterLocalData->getSettings()->strShowAwayAfterBeingInActive.bActivated = cShowAway;
		m_pcSupporterLocalData->getSettings()->strShowAwayAfterBeingInActive.iAfterCounter = _tstoi(sShowAway); 
		
		m_pcSupporterLocalData->getSettings()->strHandleCallsAndThenDisplayBusy.bActivated = cHadndleCallsDisplayBusy;
		m_pcSupporterLocalData->getSettings()->strHandleCallsAndThenDisplayBusy.iAfterCounter = _tstoi(sHadndleCallsDisplayBusy);

		m_pcSupporterLocalData->getSettings()->bOnIncomingCallsShowAlert = cOnIncomingCallsShowTrayMessage;
		m_pcSupporterLocalData->getSettings()->bOnIncomingCallsAnimateTrayIcon = cOnIncomingCallsAnimateTrayIcon;

		m_pcSupporterLocalData->getSettings()->bPromptOnItemsOnLogout = cPromptOnItemsOnLogout; //TODO check name
		m_pcSupporterLocalData->getSettings()->bPromptOnSnoozingItemsOnLogout = cPromptAboutSnoozingItemsOnLogout;
		m_pcSupporterLocalData->getSettings()->bPlaySoundUponIncomingCall = cPlaySoundUponIncomingCall;
		
		m_pcSupporterLocalData->getSettings()->bPlaySoundUponConnectingToCustomer = cPlaySoundUponConnectingToCustomer;
		m_pcSupporterLocalData->getSettings()->iDisplayXItemsAtTime = _tstoi(sDisplayItemsAtTime);

		m_pcSupporterLocalData->getSettings()->bShowAwayWhenScreenSaverIsOn = cScreenSaverIsOn;
		
		m_pcSupporterLocalData->getSettings()->Save();

		//
		//	
		//
		RestartActivityHandler();
	}

	m_iLastTestedCustDirPort = 0;

	//
	//
	//	SettingsClose
	//	DHTML_INTERFACE_SETTINGS_CLOSE
	m_cHTMLInterface.INTERFACE_UpdateData(DHTML_INTERFACE_SETTINGS_CLOSE, _T("")); 
	//ShowWindow(SW_NORMAL);

CATCH_LOG(_T("CSupportMessengerDlg::OnSettingsOk"))
	return S_OK;
}

LRESULT CSupportMessengerDlg::ShowSettingsWindow()
{
TRY_CATCH

	//todo ???
	if(m_pcSupporterLocalData==NULL)
	{
		sErrorInfo errInfo;
		//
		//	prepeare struct to costruct ErrorMsg
		errInfo.m_errSrcType = ErrSrcSupCenterApp;
		errInfo.m_resIDforDescr = IDS_ERROR_YOU_MAYBE_AUTHENTICATED_ONCE_ON_THIS_COMPUTER;
		errInfo.m_errCode = 0;
		errInfo.m_errDetailsCode = 0;
		ShowLogoutWindow(&errInfo);
		Log.Add(_ERROR_,_T("CSupportMessengerDlg::ShowSettingsWindow m_pcSupporterLocalData is NULL"));
		return 1;
	}

	BOOL	bcPortOpened = FALSE;

	CString sJsonData;
	sJsonData.FormatMessage(
			_T("[\
				{cAutomaticallyRun:%1!d!,\
				 cShowAway:%2!d!,\
				 sShowAway:%3!d!,\
				 cOnIncomingCallsAnimateTrayIcon:%4!d!,\
				 cPlaySoundUponIncomingCall:%5!d!,\
				 cPlaySoundUponConnectingToCustomer:%6!d!,\
				 cPromptOnItemsOnLogout:%7!d!,\
				 cPortOpened:%8!d!,\
				 sPortOpened:%9!d!,\
				 sDefaultPortOpened:%10!d!,\
				 cScreenSaverIsOn:%11!d!\
				 }\
 			]"), 
				m_pcSupporterLocalData->getSettings()->bAutomaticallyRun,
				m_pcSupporterLocalData->getSettings()->strShowAwayAfterBeingInActive.bActivated,
				m_pcSupporterLocalData->getSettings()->strShowAwayAfterBeingInActive.iAfterCounter,
				m_pcSupporterLocalData->getSettings()->bOnIncomingCallsAnimateTrayIcon,
				m_pcSupporterLocalData->getSettings()->bPlaySoundUponIncomingCall,
				m_pcSupporterLocalData->getSettings()->bPlaySoundUponConnectingToCustomer,
				m_pcSupporterLocalData->getSettings()->bPromptOnItemsOnLogout,
				bcPortOpened,	// GetDirectStreamPortMode	used to retrieve		
				0,
				0,
				FALSE
			);

	m_cHTMLInterface.INTERFACE_UpdateData(DHTML_INTERFACE_SETTINGS, sJsonData); 
	ShowWindow(SW_NORMAL);

	m_iLastTestedCustDirPort = 0;

CATCH_LOG(_T("CSupportMessengerDlg::ShowSettingsWindow"))
	return 0L;
}

HRESULT CSupportMessengerDlg::OnConnectionTest(IHTMLElement *pElement)
{
TRY_CATCH 

CATCH_LOG(_T("CSupportMessengerDlg::OnConnectionTest"))
	return S_OK;
}

void CSupportMessengerDlg::OnHelpAbout()
{
TRY_CATCH 
	m_cHTMLInterface.INTERFACE_UpdateData(
		DHTML_INTERFACE_HELP_ABOUT, _T(""), m_AboutDlgInfo.GetSupportMessngerVersion()); 

	ShowWindow(SW_NORMAL);
CATCH_LOG(_T("CSupportMessengerDlg::OnHelpAbout"))
}

HRESULT CSupportMessengerDlg::OnUpdateVersion(IHTMLElement *pElement)
{
TRY_CATCH 

	VARIANT	tmpProductVersion;

	pElement->getAttribute( L"sProductVersion", 0, &tmpProductVersion);
	if(tmpProductVersion.vt != VT_NULL)
	{
		m_sMinRequiredVersion = (CString)tmpProductVersion;
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnUpdateVersion Attribute sProductVersion: %s"), m_sMinRequiredVersion);

		if(m_cUpdateMonitor.IsUpdateRequired(m_sMinRequiredVersion)==TRUE)
		{
			OnUpdateRequired();
		}
		else	
		{
			if(m_cUpdateMonitor.IsMonitorRunning()==TRUE)
				m_cUpdateMonitor.UpdateCompleted();
		}
	}
	else
	{
		Log.Add(_ERROR_,_T("CSupportMessengerDlg::OnUpdateVersion Attribute sProductVersion is NULL"));
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnNewVersionAvailible"))
	return S_OK;
}

//	events maped based on js callback
HRESULT CSupportMessengerDlg::OnFlagChanged(IHTMLElement *pElement)
{
TRY_CATCH 

CATCH_LOG(_T("CSupportMessengerDlg::OnFlagChanged"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnManageFlags(IHTMLElement *pElement)
{
TRY_CATCH 
CATCH_LOG(_T("CSupportMessengerDlg::OnManageFlags"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnFwdToContact(IHTMLElement *pElement)
{
TRY_CATCH 
CATCH_LOG(_T("CSupportMessengerDlg::OnFwdToContact"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnManageContacts(IHTMLElement *pElement)
{
TRY_CATCH 
CATCH_LOG(_T("CSupportMessengerDlg::OnManageContacts"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnSnooze(IHTMLElement *pElement)
{
TRY_CATCH 
CATCH_LOG(_T("CSupportMessengerDlg::OnSnooze"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnGoToCalender(IHTMLElement *pElement)
{
TRY_CATCH 
CATCH_LOG(_T("CSupportMessengerDlg::OnGoToCalender"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnSendReply(IHTMLElement *pElement)
{
TRY_CATCH 
CATCH_LOG(_T("CSupportMessengerDlg::OnSendReply"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnSendCustomReply(IHTMLElement *pElement)
{
TRY_CATCH 
	
	VARIANT AttributeValue;
	pElement->getAttribute( L"sCustomReply", 0, &AttributeValue); ////todo Sprint 4,5?
	CString sReplyTxt(AttributeValue.bstrVal);
	
	std::wstring w_str(AttributeValue.bstrVal);
	std::string s_str;
	std::wstring w_str2;
	tstring s_to(_T("u1"));
		
	s_str  = ToUtf8FromUtf16(w_str);
	w_str2 = FromUtf8ToUtf16(s_str);
	
	//s_str = CW2CT(AttributeValue.bstrVal);
	//w_str2 = CT2CW(s_str.c_str());

	//example, W2A() converts a Unicode string to an MBCS string, and T2CW() converts a TCHAR string to a constant Unicode string.
	//USES_CONVERSION;
	//s_str  = W2A(w_str.c_str());
	//w_str2 = T2CW(s_str.c_str());
	//USES_CONVERSION;
	//s_str  = W2A(w_str.c_str());
	//w_str2 = A2W(s_str.c_str());
/*		
	m_cCommunicator.EnqueueExecutionQueue(new CNodeSendMessage(
		(tstring)m_cSettings.m_sPendingSupportRequestsTo, PROTOCOL_MSG_CUSTOM_REPLY_TO_CUSTOMER, s_str.c_str() ));
*/
	//m_cCommunicator.EnqueueExecutionQueue(new CNodeSendMessage(
	//	_T("u1"), PROTOCOL_MSG_CUSTOM_REPLY_TO_CUSTOMER, s_str.c_str() ));

CATCH_LOG(_T("CSupportMessengerDlg::OnSendCustomReply"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnGoToManageReplies(IHTMLElement *pElement)
{
TRY_CATCH
CATCH_LOG(_T("CSupportMessengerDlg::OnGoToManageReplies"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnCloseMessenger(IHTMLElement *pElement)
{
TRY_CATCH 
CATCH_LOG(_T("CSupportMessengerDlg::OnCloseMessenger"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnMinMessenger(IHTMLElement *pElement)
{
TRY_CATCH 
	//ShowWindow(SW_MINIMIZE);
CATCH_LOG(_T("CSupportMessengerDlg::OnMinMessenger"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnAddDhtmlLog(IHTMLElement *pElement)
{
TRY_CATCH 

	VARIANT	tmpLogStr;
	VARIANT	tmpLogType;
	
	pElement->getAttribute(L"sLogString", 0, &tmpLogStr);
	if(tmpLogStr.vt != VT_NULL)
	{
		pElement->getAttribute(L"sLogType", 0, &tmpLogType);
		if(tmpLogType.vt != VT_NULL)
		{
			//MESSAGE, WARNING, ERROR
			if(((CString)tmpLogType).Compare(_T("MESSAGE"))==0)
				Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnAddDhtmlLog: %s"), (CString)tmpLogStr);
			else
			if(((CString)tmpLogType).Compare(_T("WARNING"))==0)
				Log.Add(_WARNING_,_T("CSupportMessengerDlg::OnAddDhtmlLog: %s"), (CString)tmpLogStr);
			else
			if(((CString)tmpLogType).Compare(_T("ERROR"))==0)
				Log.Add(_ERROR_,_T("CSupportMessengerDlg::OnAddDhtmlLog: %s"), (CString)tmpLogStr);
		}
		else //default is _MESSAGE_
		{
			Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnAddDhtmlLog: %s"), (CString)tmpLogStr);		
		}
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnAddDhtmlLog"))
	return S_OK;
}
void CSupportMessengerDlg::OnLinksMyprofile()
{
TRY_CATCH 
	//
	//	Opens URL for SupporterProfile
	//	"http://supportspace.com:8080/stargate/mySupporterProfile.s2?username=<jabberUserName>"
	//
	CString sOtpUrlPart;	
	CString	sFullUrl;	
	tstring	sSupporterID = m_MessengerState.sCurrentSupporterCrypt;

	sOtpUrlPart.FormatMessage(URL_SUPPORTER_PROFILE, sSupporterID.c_str());
	sFullUrl = theApp.m_cSettings.m_sBaseUrlSupporterProfile + sOtpUrlPart;

	m_cUrlOpener.Open(sFullUrl);

CATCH_LOG(_T("CSupportMessengerDlg::OnLinksMyprofile"))
}

void CSupportMessengerDlg::OnLinksExpertForum()
{
TRY_CATCH 
	//
	//	Opens URL for SupporterProfile
	//	http://expertforum.supportspace.com
	//
	CString	sFullUrl;	
	sFullUrl = theApp.m_cSettings.m_sBaseUrlExpertForum;
	m_cUrlOpener.Open(sFullUrl);

CATCH_LOG(_T("CSupportMessengerDlg::OnLinksExpertForum"))
}

LRESULT CSupportMessengerDlg::OnUninstallStarted(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 
	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnUninstallStarted"));

	OnPopupExit();

CATCH_LOG(_T("CSupportMessengerDlg::OnUninstallStarted"))
	return 0L;
}

void CSupportMessengerDlg::OnHelpHelp()
{
TRY_CATCH 
	//
	//	Opens URL for help
	//	http://supportspace.com:8080/stargate/expertHelp.s2
	//
	CString sOtpUrlPart(URL_HELP_HELP);	
	CString	sFullUrl;	

	sFullUrl = theApp.m_cSettings.m_sBaseUrlHelp + sOtpUrlPart;
	m_cUrlOpener.Open(sFullUrl);

CATCH_LOG(_T("CSupportMessengerDlg::OnUninstallStarted"))
}

LRESULT CSupportMessengerDlg::OnPowerBroadCast(WPARAM wParam, LPARAM lParam)
{
	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnPowerBroadCast with Event: %d"), (int)wParam);

	//  Power-management event. This parameter can be one of the following events:
	switch(wParam)
	{
	case PBT_APMBATTERYLOW:  //Battery power is low. 
		break;
	case PBT_APMOEMEVENT: // OEM-defined event occurred. 
		break;
	case PBT_APMPOWERSTATUSCHANGE: //Power status has changed. 
		break;
	case PBT_APMRESUMEAUTOMATIC: // Operation resuming automatically after event.  18!!!
		break;
	case PBT_APMRESUMECRITICAL: // Operation resuming after critical suspension.  !!!
		break;

	case PBT_APMQUERYSUSPENDFAILED: // Suspension request denied. 
	case PBT_APMRESUMESUSPEND:	    // Operation resuming after suspension.  7

		//TODO not for Sprint5: Server know current selected status and may send it
		Log.Add(_WARNING_,_T("Operation resuming after suspension. Return SupporterSelected status"));
		if(m_MessengerState.bPowerStatusSuspended == TRUE)
		{
			m_MessengerState.bPowerStatusSuspended = FALSE;
			IMSendUpdateStatus(m_MessengerState.eSupporterSelectedStatus);
			UpdateTrayIcon(RollbackToSupporterSelectedStatus);//PowerStatusChanged
		}

		break;

	case PBT_APMQUERYSUSPEND: // Request for permission to suspend.   0 
		if(m_MessengerState.dwInSessionCallsCounter!=0)
		{
			Log.Add(_WARNING_,_T("Request for permission to suspend, but Exper have active Session and returned BROADCAST_QUERY_DENY"));
			m_MessengerState.bPowerStatusSuspended = TRUE;//for case if our deny ignored
			return BROADCAST_QUERY_DENY; // to deny a request. TODO ...PM question if it is correct?
		}
		break;

	case PBT_APMSUSPEND: // System is suspending operation.   4 !!! todo here change status to Offline

		Log.Add(_WARNING_,_T("System is suspending operation"));
		m_MessengerState.bPowerStatusSuspended = TRUE; //todo to change this to False somewhere
		UpdateTrayIcon(PowerStatusChanged);
		IMSendUpdateStatus(StatusBusy);
		break;
	default:
		break;
	}

	return TRUE;	// to grant a request and return BROADCAST_QUERY_DENY to deny a request.
}
HRESULT CSupportMessengerDlg::OnMissedCallsCounterChnanged()
{
TRY_CATCH

	Log.Add(_WARNING_,_T("OnMissedCallsCounterChnanged. Number of missed calls:%d"),m_MessengerState.dwMissedCallsCounter);
	
	if(m_MessengerState.dwMissedCallsCounter >= m_pcSupporterLocalData->getSettings()->strShowBusyAfterMissedCallsNum.iAfterCounter && 
	   m_pcSupporterLocalData->getSettings()->strShowBusyAfterMissedCallsNum.bActivated == TRUE &&
	   m_MessengerState.bStatusChangedOnMissedCalls==FALSE) 
	{
		Log.Add(_WARNING_,_T("Status changed to Busy. Missed calls couner more then allowed: %d"),
			m_pcSupporterLocalData->getSettings()->strShowBusyAfterMissedCallsNum.iAfterCounter );

		m_MessengerState.bStatusChangedOnMissedCalls = TRUE;
		UpdateTrayIcon(MissedCallsCounterChanged);
		IMSendUpdateStatus(StatusBusy);

		//
		//	if missed some calls we close all alerts and show alert with Missed calls notice
		//
		CloseAllAlerts();

		// we need to set m_lFreePosition to 1 in order to show offline messages alert at correct position
		m_lFreePosition = 1; 
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnMissedCallsCounterChnanged FreePosition %d"), m_lFreePosition);

		//
		//	Create new alert dialog with Missed Call information
		//
		if(m_pcAlertDlgMissedCalls==NULL)
		{
			m_pcAlertDlgMissedCalls = new CAlertDlgMissedCalls(
				CWnd::FromHandle(this->m_hWnd),
				NULL, /*pcTransparentWindow - version without transparent border*/ 
				NULL, /*hWndParent*/ 
				NULL, /*pParent*/ 
				theApp.m_cSettings.m_eGUIlocation, 
				theApp.m_cSettings.m_sGUIlocationFilePath, 
				1);
		}
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnMissedCallsCounterChnanged"))
	return S_OK;
}

LRESULT CSupportMessengerDlg::OnAlertDlgMissedCallsClose(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 

	CloseMissedCallsAlert();

CATCH_LOG(_T("CSupportMessengerDlg::OnAlertDlgUpdateClose"))
	return 0L;
}

LRESULT CSupportMessengerDlg::OnAlertDlgMissedCallsPickUp(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 

	Log.Add(_MESSAGE_,_T("OnAlertDlgMissedCallsPickUp"));

	//
	//
	//
	CloseMissedCallsAlert();
	
	//
	//
	// Change status to Onnline if connected, otherwise will be offline
	m_MessengerState.dwMissedCallsCounter = 0;
	m_MessengerState.bStatusChangedOnMissedCalls = FALSE;
	UpdateTrayIcon(MissedCallsCounterChanged);
	IMSendUpdateStatus(StatusAvailible);//todo may be to m_MessengerState.eSupporterSelectedStatus
	
CATCH_LOG(_T("CSupportMessengerDlg::OnAlertDlgUpdatePickUp"))
	return 0L;
}

void CSupportMessengerDlg::CloseMissedCallsAlert()
{
TRY_CATCH
	//
	//	Close dialog 
	//
	if(m_pcAlertDlgMissedCalls!=NULL)
	{
		m_pcAlertDlgMissedCalls->DestroyModeless(); //call delete in the PostNcDestroy
		m_pcAlertDlgMissedCalls = NULL;
	}
CATCH_LOG(_T("CSupportMessengerDlg::CloseMissedCallsAlert"))	
}


LRESULT CSupportMessengerDlg::OnWorkBenchClose(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 

	Log.Add(_MESSAGE_,_T("OnWorkBenchClose received... TODO"));
	//AfxMessageBox("Lynda, please check if SupportCenter proecess dead!!!");

	switch(wParam)
	{
	case CustomerDirectCall:
		/*
		if(m_pWorkbenchDlg){
			m_pWorkbenchDlg->DestroyModeless(); //call delete in the PostNcDestroy
			m_pWorkbenchDlg = NULL;

			//	todo - here we can change status to onnline  like when picked if not done yet
			if(m_MessengerState.dwInSessionCallsCounter > 0)
			{
				m_MessengerState.dwInSessionCallsCounter = 0;

				Log.Add(_MESSAGE_,_T("Changes the expert’s status from Busy StatusChangedOnMaxHandled and now number of InSessionCalls is less"));
				m_MessengerState.bStatusChangedOnMaxHandled = FALSE;
				IMSendUpdateStatus(m_MessengerState.eSupporterSelectedStatus);
				UpdateTrayIcon(BackFromHandledMaxNumberOfCalls);		
			}
			//  todo - signout initiated by Expert may also close all WorkbenchDlg?
		}
		*/
	//	todo - here we can change status to onnline  like when picked if not done yet
		if(m_MessengerState.dwInSessionCallsCounter > 0)
		{
			m_MessengerState.dwInSessionCallsCounter = 0;

			Log.Add(_MESSAGE_,_T("Changes the expert’s status from Busy StatusChangedOnMaxHandled and now number of InSessionCalls is less"));
			m_MessengerState.bStatusChangedOnMaxHandled = FALSE;
			IMSendUpdateStatus(m_MessengerState.eSupporterSelectedStatus);
			UpdateTrayIcon(BackFromHandledMaxNumberOfCalls);		
		}
		//  todo - signout initiated by Expert may also close all WorkbenchDlg?

		break;
	case ConsultCall:
		//CloseConsultWorkBench(lParam);
		break;
	default:
		break;
	}
	
CATCH_LOG(_T("CSupportMessengerDlg::OnWorkBenchClose"))
	return 0L;
}


LRESULT CSupportMessengerDlg::CloseConsultWorkBench(long lUid)
{
TRY_CATCH 

	WorkbenchDlgConsultsMapIterator   it = m_mapWorkbenchDlgConsults.find(lUid);

	if(it != m_mapWorkbenchDlgConsults.end())
	{
		CWorkbenchDlg*  pcWorkbenchDlg = (*it).second;
		if(pcWorkbenchDlg)
		{
			Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::CloseConsultWorkBench UID= %d"), lUid);
			pcWorkbenchDlg->DestroyModeless(); //call delete in the PostNcDestroy
		}

		m_mapWorkbenchDlgConsults.erase(it);
	}
	else
	{
		Log.Add(_WARNING_,_T("UID= %d not found in mapWorkbenchDlgConsults"), lUid);
	}

CATCH_LOG(_T("CSupportMessengerDlg::CloseConsultWorkBench"))
	return 0L;
}
/*
1. A message that contain only a subject:
1.1 stargate_busy.
2. A message that contains a subject and a body:
2.1. subject: stargate_busy_lock
2.2 body – a full support request object – the same object you receive today
3. A message that contains a subject only:
3.1  subject stargate_online

The messages should result the following behavior:
1.	For message 1 the client will change his status to busy (the same way as if the supporter would have change his status to busy)
a.	Note – the client should not send any message as a response to the message he is getting 
b.	The client will be able to change his status to online 
2.	For message 2 – the supporter is already busy, now we enter a state as if the supporter picked up a call, the client should not be able to change the status
a.	Note – the client should not send any message as a response to the message he is getting 
3.	For message 3 – the supporter should change his state to online again
a.	Note – the client should not send any message as a response to the message he is getting 
*/
HRESULT CSupportMessengerDlg::OnSetStargateStatus(IHTMLElement *pElement)
{
TRY_CATCH 

	long	lCallUID = 0;
	VARIANT	tmpCallUID;
	VARIANT	tmpStatus;
	CString	sStatus;
	
	pElement->getAttribute( L"sStatus", 0, &tmpStatus);
	if(tmpStatus.vt != VT_NULL)
		sStatus = (CString)tmpStatus;

	pElement->getAttribute(L"iCallUID", 0, &tmpCallUID);
	if(tmpCallUID.vt != VT_NULL)
		lCallUID = tmpCallUID.lVal;
	
	if(((CString)sStatus).Compare(_T("stargate_online_4customer"))==0){
		Log.Add(_MESSAGE_,_T("SetStargateStatus::SetStargateStatus: %s"), (CString)sStatus);
		m_MessengerState.eSupporterSelectedStatus = StatusOnnline4Customer;
		SetStargateSelectedStatus(StatusOnnline4Customer);
	}
	else
	if(((CString)sStatus).Compare(_T("stargate_busy"))==0){
		Log.Add(_MESSAGE_,_T("SetStargateStatus::SetStargateStatus: %s"), (CString)sStatus);
		m_MessengerState.bStatusChangedOnStargateBusy = TRUE;
		SetStargateSelectedStatus(StatusBusy);
	}
	else
	if(((CString)sStatus).Compare(_T("stargate_busy_lock"))==0){
		Log.Add(_WARNING_,_T("SetStargateStatus::SetStargateStatus: %s"), (CString)sStatus);

		m_MessengerState.bStatusChangedOnStargateBusy = FALSE;
		m_MessengerState.eSupporterSelectedStatus = StatusAvailible;//we assume that expert was availible before get stargate_busy
		m_MessengerState.dwInSessionCallsCounter++;

		OnNewCallPicked(lCallUID);

		Log.Add(_MESSAGE_,_T("Changes the expert’s status to busy once a set number of sessions are ongoing"));
		m_MessengerState.bStatusChangedOnMaxHandled = TRUE;
		IMSendUpdateStatus(StatusBusy);
		UpdateTrayIcon(HandledMaxNumberOfCalls);		
	}
	else
	if(((CString)sStatus).Compare(_T("stargate_online"))==0){//only for case when stargate_busy was sent before
		Log.Add(_ERROR_,_T("SetStargateStatus::SetStargateStatus: %s"), (CString)sStatus);
		if(m_MessengerState.bStatusChangedOnStargateBusy==TRUE)
		{
			m_MessengerState.bStatusChangedOnStargateBusy = FALSE;
			SetStargateSelectedStatus(StatusAvailible);
		}
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnSetStargateStatus"))
	return S_OK;
}

void CSupportMessengerDlg::SetStargateSelectedStatus(SupporterStatus status)
{
TRY_CATCH 
	
	SetSelectedStatus(status);

CATCH_LOG(_T("CSupportMessengerDlg::SetSupporterSelectedStatus"))
}

HRESULT CSupportMessengerDlg::OnShowMessageBox(IHTMLElement *pElement)
{
TRY_CATCH 

	VARIANT	tmpLogStr;
		
	pElement->getAttribute(L"sMboxText", 0, &tmpLogStr);
	if(tmpLogStr.vt != VT_NULL)
	{
		AfxMessageBox((CString)tmpLogStr);
		Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnShowMessageBox: %s"), (CString)tmpLogStr);		
	}

CATCH_LOG(_T("CSupportMessengerDlg::OnShowMessageBox"))
	return S_OK;
}

HRESULT CSupportMessengerDlg::OnStargateOfflineMsgNotification(IHTMLElement *pElement)
{
TRY_CATCH 

	Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnStargateOfflineMsgNotification"));

	VARIANT	tmpLogStr;
	pElement->getAttribute(L"sData", 0, &tmpLogStr);
	if(tmpLogStr.vt != VT_NULL)
		Log.Add(_MESSAGE_,_T("OnStargateOfflineMsgNotification data %s"), (CString)tmpLogStr);		

	long lMid = -1;
	VARIANT	tmplMid;

	//todo get here lThreadID
	pElement->getAttribute(L"lMid", 0, &tmplMid);
	if(tmplMid.vt != VT_NULL)
		lMid = tmplMid.lVal;

	Log.Add(_MESSAGE_,_T("OnStargateOfflineMsgNotification Mid:%d"), lMid);		

	if(IsFreeSpaceToShowAlert()==FALSE)
		return S_OK;

	// todo anatoly
	// we have to manage free position for each alert. than final position will be calculated due free position:
	// m_lFreePosition = (long)m_mapAlerts.size() + 1; 
	m_lFreePosition++; 
	long iCallUID = -1*(time(NULL)+m_lFreePosition);//generate negative unique id each time received offline message for map
	Log.Add(_MESSAGE_,_T("OnStargateOfflineMsgNotification FreePosition %d UID=%d"), m_lFreePosition, iCallUID);
	
	//	todo if already exisits we have not to add it ot the map
	//	this will cause problem with map of alerts
	//	this may not happnen, but still may be fixed 
	m_mapAlerts.insert( AlertsMap::value_type( 
		iCallUID, 
		new CAlertDlgOfflineMsg( 
		CWnd::FromHandle(this->m_hWnd), 
		NULL,//pcTransparentWindow,
		NULL,//hWndParent
		NULL,//pParent
		theApp.m_cSettings.m_eGUIlocation,
		theApp.m_cSettings.m_sGUIlocationFilePath,
		m_lFreePosition,
		iCallUID,
		(CString)tmpLogStr,
		OfflineMsg,
		lMid
		)));// - better to use to get result

CATCH_LOG(_T("CSupportMessengerDlg::OnStargateOfflineMsgNotification"))
	return S_OK;
}

LRESULT CSupportMessengerDlg::OnAlertDlgOfflineMsgPickUp(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 
	long lUid = (long)lParam;
	long lThreadID = (long)wParam;

	//
	//	Opens URL for SupporterProfile
	//	Url for getting the inbox with new message opened
	//	http://supportspace.com:8080/stargate/inbox.s2?action=new&ju=e965c8f147757a5e216b341046649e72344b9266
	//
	CString sOtpUrlPart;	
	CString	sFullUrl;	
	tstring	sSupporterID = m_MessengerState.sCurrentSupporterCrypt;

	if(lThreadID > 0)
		sOtpUrlPart.FormatMessage(URL_SUPPORTER_INBOX_THREADID, sSupporterID.c_str(),lThreadID);
	else
		sOtpUrlPart.FormatMessage(URL_SUPPORTER_INBOX, sSupporterID.c_str());
		
	sFullUrl = theApp.m_cSettings.m_sBaseUrlSupporterInbox + sOtpUrlPart;

	m_cUrlOpener.Open(sFullUrl);

	CloseAlert(lUid);

CATCH_LOG(_T("CSupportMessengerDlg::OnAlertDlgOfflineMsgPickUp"))
	return 0L;
}

LRESULT CSupportMessengerDlg::OnAlertDlgOfflineMsgClose(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 
	long lUid = (long)lParam;
	CloseAlert(lUid);
CATCH_LOG(_T("CSupportMessengerDlg::OnAlertDlgOfflineMsgClose"))
	return 0L;
}

LRESULT CSupportMessengerDlg::OnAlertDlgInfoMsgClose(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 
	long lUid = (long)lParam;
	CloseAlert(lUid);
CATCH_LOG(_T("CSupportMessengerDlg::OnAlertDlgOfflineMsgClose"))
	return 0L;
}

LRESULT CSupportMessengerDlg::OnAlertDlgInfoMsgHelp(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 
	long lUid = (long)lParam;
	CloseAlert(lUid);
	//open URL
	//
	//	Opens URL for help
	//	http://supportspace.com:8080/stargate/expertHelp.s2
	//
	CString sOtpUrlPart(URL_HELP_HELP);	
	CString	sFullUrl;	

	sFullUrl = theApp.m_cSettings.m_sBaseUrlHelp + sOtpUrlPart;
	m_cUrlOpener.Open(sFullUrl);

CATCH_LOG(_T("CSupportMessengerDlg::OnAlertDlgOfflineMsgHelp"))
	return 0L;
}

BOOL CSupportMessengerDlg::IsFreeSpaceToShowAlert()
{
TRY_CATCH 

	if(m_mapAlerts.size()==0 && m_pcAlertDlgMissedCalls==NULL && m_pcAlertDlgUpdate==NULL)
	{
		m_MessengerState.bWaitTillAllEventsClosed = FALSE;	
		m_lFreePosition=0;
	}

	//	We have a rull: to show not more then 5 alerts
	//	after 5 alerts appear we have to wait till all of them closed before show next alert
	if(m_lFreePosition >= MAX_NUMBER_OF_ALERTS_TO_SHOW - 1)
	{
		if(m_MessengerState.bWaitTillAllEventsClosed==TRUE)//case when number of alerts is less then MAX but was not reset to 0 yet
		{
			Log.Add(_WARNING_, _T("Wait till ALL of them will be closed. Currently oprned %d alerts."), m_mapAlerts.size());
			return FALSE; 
		}

		Log.Add(_MESSAGE_, _T("We show max defined number of Alerts.Wait till ALL of them will be closed"));
		m_MessengerState.bWaitTillAllEventsClosed = TRUE;
		AfxMessageBox(_T("Oops. Close all SupportCenter pop-up windows to continue to get notifications"));
		//return TRUE; //bug http://expertforum.supportspace.com/index.php?act=Msg&CODE=03&VID=in&MSID=767

		//CloseAllAlerts();
		//m_MessengerState.bWaitTillAllEventsClosed = FALSE;	
		//m_lFreePosition=0;
		//return TRUE;
	}
	else
	{
		if(m_MessengerState.bWaitTillAllEventsClosed==TRUE)//case when number of alerts is less then MAX but was not reset to 0 yet
		{
			Log.Add(_WARNING_, _T("Wait till ALL of them will be closed. Currently oprned %d alerts."), m_mapAlerts.size());
			return FALSE; 
		}
	}

	if(CAlertDlgNewCall::IsFreePositionAvailble(m_lFreePosition+1)==FALSE)
		return FALSE;

CATCH_LOG(_T("CSupportMessengerDlg::OnAlertDlgOfflineMsgClose"))
	return TRUE;
}

LRESULT CSupportMessengerDlg::OnOnline4CustomerClose(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 
	if(m_cOnline4CustomerDlg)
	{
		m_cOnline4CustomerDlg->DestroyModeless();
		m_cOnline4CustomerDlg = NULL;
	}
CATCH_LOG(_T("CSupportMessengerDlg::OnOnline4CustomerClose"))
	return 0L;
}

LRESULT CSupportMessengerDlg::OnOnline4CustomerSubmit(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 
	Log.Add(_MESSAGE_,_T("SCSupportMessengerDlg::OnOnline4CustomerSubmit"));
	
	CString sCustomerDisplayName;

	if(m_cOnline4CustomerDlg)
	{
		sCustomerDisplayName = m_cOnline4CustomerDlg->GetCustomerDisplayName();
		m_cOnline4CustomerDlg->DestroyModeless();
		m_cOnline4CustomerDlg = NULL;
	}
	else
		Log.Add(_ERROR_,_T("SCSupportMessengerDlg::OnOnline4CustomerSubmit. m_cOnline4CustomerDlg is NULL"));

	m_MessengerState.eSupporterSelectedStatus = StatusOnnline4Customer;
	m_MessengerState.sOnnline4CustomerDisplayName = sCustomerDisplayName;

	SetStargateSelectedStatus(StatusOnnline4Customer);

CATCH_LOG(_T("CSupportMessengerDlg::OnOnline4CustomerClose"))
	return 0L;
}

void CSupportMessengerDlg::OnLinksSessionshistory()
{
TRY_CATCH 
	//
	//	Opens URL for SupporterProfile
	//	"http://supportspace.com:8080/stargate/mySupporterProfile.s2?username=<jabberUserName>"
	//
	CString sOtpUrlPart;	
	CString	sFullUrl;	
	tstring	sSupporterID = m_MessengerState.sCurrentSupporterCrypt;

	sOtpUrlPart.FormatMessage(URL_SUPPORTER_SESSION_HISTORY, sSupporterID.c_str());
	sFullUrl = theApp.m_cSettings.m_sBaseUrlSupporterSessionHistory + sOtpUrlPart;

	m_cUrlOpener.Open(sFullUrl);

CATCH_LOG(_T("CSupportMessengerDlg::OnLinksMyprofile"))
}

void CSupportMessengerDlg::OnLinksExpertportal()
{
TRY_CATCH 
	//
	//	Opens URL for SupporterProfile
	//	http://expertforum.supportspace.com
	//
	CString	sFullUrl;	
	sFullUrl = theApp.m_cSettings.m_sBaseUrlExpertPortal;
	m_cUrlOpener.Open(sFullUrl);

CATCH_LOG(_T("CSupportMessengerDlg::OnLinksExpertportal"))
}

void CSupportMessengerDlg::OnLinksMymessages()
{
TRY_CATCH 
	//
	//	Opens URL for SupporterProfile
	//	"http://supportspace.com:8080/stargate/mySupporterMessages.s2?username=<jabberUserName>"
	//   http://supportspace.com/home/inbox_sc.s2?username=#jabberUserName#
	//
	CString sOtpUrlPart;	
	CString	sFullUrl;	
	tstring	sSupporterID = m_MessengerState.sCurrentSupporterCrypt;

	sOtpUrlPart.FormatMessage(URL_SUPPORTER_MY_MESSAGES, sSupporterID.c_str());
	sFullUrl = theApp.m_cSettings.m_sBaseUrlSupporterSessionHistory + sOtpUrlPart;

	m_cUrlOpener.Open(sFullUrl);

CATCH_LOG(_T("CSupportMessengerDlg::OnLinksMyprofile"))
}

LRESULT CSupportMessengerDlg::OnIMChannelQualityTestTime(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 

	time_t testTime  = (time_t)lParam;
	char buf[16] = {'\0'};
	sprintf_s(buf, 16, "%I64u", testTime);  
	tstring body(buf);

	m_cCommunicator.EnqueueExecutionQueue(new CNodeSendMessage(
		(tstring)m_MessengerState.sCurrentSupporterCrypt.c_str(), 
		PROTOCOL_MSG_IM_CHANNEL_QUALITY_TEST, 
		body,
		theApp.m_cSettings.m_sResource));//resource of myself always IMClient 

CATCH_LOG(_T("CSupportMessengerDlg::OnIMChannelQualityTestTime"))
	return 0L;
}

LRESULT CSupportMessengerDlg::OnIMChannelQualityTestResponse(tstring subject)
{
TRY_CATCH 

	time_t  testTime;
	testTime = _strtoui64(subject.c_str(),NULL,10);

	m_cIMChannelQualityMonitor.HandleIMChannelQualityTestResponse(testTime, subject);

CATCH_LOG(_T("CSupportMessengerDlg::OnIMChannelQualityTestTime"))
	return 0L;
}

LRESULT CSupportMessengerDlg::OnIMChannelQualityIssueDetected(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH
	m_MessengerState.bIMChannelQualityIssueDetected = TRUE; 
	UpdateTrayIcon(IMChannelQualityIssueDetected);

	m_TrayIcon.SetBalloonDetails(
			(CString)MAKEINTRESOURCE(IDS_TRAY_ICON_TOOLTIP_LIMITED_CONNECTION),
			(CString)MAKEINTRESOURCE(IDS_APP_FULL_NAME), CTrayNotifyIcon::Warning, 3);

	if(m_MessengerState.dwInSessionCallsCounter!=0)
		theApp.m_conQualityLog.AddMessage(cLog::_MESSAGE,Format(_T("Poor connectivity detected while in a session")));			

	if(IsFreeSpaceToShowAlert()==FALSE)
		return S_OK;

	// todo anatoly
	// we have to manage free position for each alert. than final position will be calculated due free position:
	// m_lFreePosition = (long)m_mapAlerts.size() + 1; 
	m_lFreePosition++; 
	long iCallUID = -1*(time(NULL)+m_lFreePosition);//generate negative unique id each time received offline message for map
	Log.Add(_MESSAGE_,_T("OnStargateOfflineMsgNotification FreePosition %d UID=%d"), m_lFreePosition, iCallUID);
	
	//	todo if already exisits we have not to add it ot the map
	//	this will cause problem with map of alerts
	//	this may not happnen, but still may be fixed 
	m_mapAlerts.insert( AlertsMap::value_type( 
		iCallUID, 
		new CAlertDlgInfoMsg( 
		CWnd::FromHandle(this->m_hWnd), 
		NULL,//pcTransparentWindow,
		NULL,//hWndParent
		NULL,//pParent
		theApp.m_cSettings.m_eGUIlocation,
		theApp.m_cSettings.m_sGUIlocationFilePath,
		m_lFreePosition,
		iCallUID,
		InfoMsg
		)));// - better to use to get result


CATCH_LOG(_T("CSupportMessengerDlg::OnIMChannelQualityIssueDetected"))
	return 0L;
}

LRESULT  CSupportMessengerDlg::OnIMChannelQualityImprovementDetected(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH
	m_MessengerState.bIMChannelQualityIssueDetected = FALSE;
	UpdateTrayIcon(IMChannelQualityImprovementDetected);

	m_TrayIcon.SetBalloonDetails(
			(CString)MAKEINTRESOURCE(IDS_TRAY_ICON_TOOLTIP_LIMITED_CONNECTION_IMPROVED),
			(CString)MAKEINTRESOURCE(IDS_APP_FULL_NAME), CTrayNotifyIcon::Info, 3);

CATCH_LOG(_T("CSupportMessengerDlg::OnIMChannelQualityImprovementDetected"))
	return 0L;
}




