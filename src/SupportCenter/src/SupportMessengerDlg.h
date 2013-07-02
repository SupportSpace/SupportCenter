// SupportMessengerDlg.h : header file
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CSupportSpaceNotifierDlg
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CSupportSpaceNotifierDlg :	main dialog based window of SupportMessenger
//
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 10/29/2006 05:21:10 PM
// Comments: First Issue
//===========================================================================
#pragma once

#include "AppBar.h"
#include "TrayNotifyIcon.h"
#include "UrlOpener.h"
#include "HTMLInterface.h"
#include "AlertDlg.h"
#include "AlertDlgNewCall.h"
#include "AlertDlgUpdate.h"
#include "AlertDlgMissedCalls.h"
#include "AlertDlgOfflineMsg.h"
#include "Communicator/Communicator.h"
#include "MessengerLocalData.h"	//todo
#include "SupporterLocalData.h"	//todo
#include "ActivityHandler.h"	//todo
#include "UpdateMonitor.h"
#include "AboutDlgInfo.h"
#include "SHA1.h"
#include "WorkbenchDlg.h"
#include "Online4CustomerDlg.h"
#include "IMChannelQualityMonitor.h"
#include <sys/timeb.h>
#include <time.h>
#include "AlertDlgInfoMsg.h"

using namespace std;
#include <map>

#define WM_TRAYNOTIFY				WM_USER + 100
#define IM_KEEPALIVE_TIMER			0x150
#define OPEN_ABOUT_WINDOW			WM_USER + 1502


typedef std::map<const long, CAlertDlg*> AlertsMap;
typedef std::map<const long, CAlertDlg*>::iterator  AlertsMapIterator;

typedef std::map<const long, CWorkbenchDlg*> WorkbenchDlgConsultsMap;
typedef std::map<const long, CWorkbenchDlg*>::iterator  WorkbenchDlgConsultsMapIterator;


class CPresence
{
public:
	CPresence(Presence imStatus, const tstring& imMsg):m_imStatus(imStatus), m_imMsg(imMsg){};
	~CPresence(){};

	Presence	m_imStatus;			// status defined in xmpp protocol
	tstring		m_imMsg;			// user defined message
};

typedef struct _IconsStr
{
	HICON	m_hMainApp;				// icon of the main app
	HICON	m_Available;			// Online
	HICON	m_Busy;
	HICON	m_Away;
	HICON	m_Offline;
	HICON	m_NewCalls;
	HICON	m_NewCallsAnimated[3];
	HICON	m_Connecting;
	HICON	m_SignedOut;
	HICON	m_UpdateRequired;
	HICON	m_ConnectingAnimated[2];
	HICON	m_OnnlineForCustomer;

	HICON	m_AvailableLimitedCon;	
	HICON	m_BusyLimitedCon;
	HICON	m_AwayLimitedCon;
	HICON	m_OfflineLimitedCon;
	HICON	m_OnnlineForCustomerLimitedCon;

} IconsStr;

typedef struct _InfoStringsStr
{
	CString  m_sInfoUnattenedCallsInInbox;
	CString  m_sInfoSnoozinCallsInInbox;
	CString  m_sInfoCallWasTakenCantBeSnoozed;
	CString  m_sErrUsernameUnknown;
	CString  m_sErrPasswordIncorect;
	CString  m_sInfoChangingUserFlag;

} InfoErrorStringsStr;


typedef enum _TrayIconChangeReason
{
	SupporterChangedStatus = 0,
	ConnectionStateChanged = 1,
	NewCallsCounterChanged = 2,
	NewCallsAnimationActivated = 3,
	NewCallsAnimationStoped = 4, 
	ActivityHandlerIdleEvent = 5,
	ActivityHandlerInactiveEvent = 6,
	UpdateRequiredStateChanged = 7,
	RollbackToSupporterSelectedStatus = 8,
	HandledMaxNumberOfCalls = 9,
	BackFromHandledMaxNumberOfCalls = 10,
	InSessionCallsCounterChanged = 11,
	NewConsultCounterChanged = 12,
	InSessionConsultCallsCounterChanged = 13,
	PowerStatusChanged = 14,
	MissedCallsCounterChanged = 15,
	IMChannelQualityIssueDetected = 16,
	IMChannelQualityImprovementDetected = 17,
	
}TrayIconChangeReason;


typedef enum _SupporterStatus
{
	StatusAvailible = 0,//Supporter is logged in and available to accept calls.
	StatusNotAcceptingCalls = 1,//(Appear offline) Supporter is logged in and cannot accept calls.
								// Supporters using this status can conduct all activities which 
								// require login (browse the supporters’ site, update profile, read blog)
								// but are not calculated in the pull of available supporters. 
								// Users see these supporters as Offline,
	StatusBusy = 2, //upporter is online but in session or doing other tasks.
					//Per supporter’s settings, this status can be set automatically when x number
					//of calls are handled in parallel.  
	
	StatusAway = 3, // Supporter is online but cannot attend calls as he is way from the PC or has 
					// selected this option.
					// Per supporter’s settings, this status can be set automatically when idle for 
					// more than X minutes.
					// Supporters with this status are not calculated in the pull of available 
					// supporters. Users see these supporters as Offline.
	StatusOffline = 4, //Offline	Supporter is logged out. 
					//There is no available data displayed on the Calls Manager if the supporter is offline.

	StatusOnnline4Customer = 5, //StatusOnnline4Customer

} SupporterStatus;

typedef enum _IMConnectionState
{
	stateDisconnected = 0,
	stateConnecting = 1,
	stateConnected = 2,
	stateDisconnecting = 3,
	stateWaitAutoReconnect = 4,

}IMConnectionState;

typedef enum _TrayMenuOpenReason
{
	None = 0,
	TrayIcon = 1,		// right click on the tray icon caused 
	MainDialog = 2,
    AboutDialog = 3,
	SettingsDialog = 4,

}TrayMenuOpenReason;

//
// The structure describes state of the Messenger 
//  
// 
typedef struct _MessengerStateStr
{
	tstring				sCurrentSupporter;		// currently loaded supporter 
	tstring				sCurrentSupporterCrypt; // SHA-1 encrypted value of CurrentSupporter email address 
	BOOL				bUserSelectedSignIn;	// flag to remember user's signed/signedout selection
	DWORD				dwNewCallsCounter;		// current number of New calls 
	DWORD				dwInSessionCallsCounter;// current number of InSession calls 
	DWORD				dwNewConsultCall;		// current number of NewConsult calls
	DWORD				dwInSessionConsultCallCounter; //current number of NewConsult calls picked
	DWORD				dwMissedCallsCounter;	// current number of Missed calls 
	BOOL				bStatusChangedOnIdle;	// flag indicates that status was changed when user become idle 
	BOOL				bStatusChangedOnMaxHandled;	// flag indicates that status was changed to busy - because max number of calls
	BOOL				bUpdateRequired;		// flag indicator if update required
	BOOL				bAnimateTray;			// flag indicates that Animation may be running
	TrayMenuOpenReason  eTrayMenuOpenReason;	// indicates what module of the application opened TrayMenu	
	DWORD				dwIMConnectAttempt;		//  counter of connections attempts 
	BOOL				bWaitTillAllEventsClosed; //flag to show alerts due defined in PRD rules
	IMConnectionState	eConnectionState;		// actual connection state
	SupporterStatus		eSupporterSelectedStatus;// flag to store current selected user's status 
	BOOL				bPowerStatusSuspended;	//  flag indicates that PowerStatusSuspended - Hibenate/Standby mode
	BOOL				bStatusChangedOnMissedCalls; // flag status changed on missed calls
	BOOL				bStatusChangedOnStargateBusy; // flag status changed on missed calls
	tstring				sOnnline4CustomerDisplayName; //current sOnnline4CustomerDisplayName if exists and only one
	double				fConnectStartTime;

	BOOL				bIMChannelQualityIssueDetected;

}MessengerStateStr;

typedef enum _ErrSrcType
{
	ErrSrcJubberSrv = 1,
	ErrSrcSupCenterApp = 2,

}ErrSrcType;

// Structure to store error information 
typedef struct _sErrorInfo
{
	long	m_errSrcType;			//	
	long	m_resIDforDescr;		//	
	long	m_errCode;				//
	long	m_errDetailsCode;		//
}sErrorInfo;

// CSupportMessengerDlg  dialog
class CSupportMessengerDlg  : public CAppBar
{
// Construction
public:
	CSupportMessengerDlg (CWnd* pParent = NULL);	// standard constructor
	~CSupportMessengerDlg();	// standard constructor

// Dialog Data
	enum { IDD = IDD_SupportMessenger_DIALOG, IDH = 0 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu);
	LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);

//APPBAR required begin
protected:  // Overridable functions
	void OnAppBarStateChange (BOOL fProposed, UINT uStateProposed);
	void HideFloatAdornments(BOOL fHide);
	BOOL InitAppBar();
	void OnCancel();

	 afx_msg void OnTimer(UINT nIDEvent);

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnClose();

	CSize m_szMinTracking;  // The minimum size of the client area
	// CSRBar's class-specific constants
//APPBAR required end

// Implementation
protected:
	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void PostNcDestroy();
	//afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	
	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
	//DECLARE_EVENTSINK_MAP() //todo needed

public:
//	Menu calls begin
	afx_msg void OnDockRight();
	afx_msg void OnDockLeft();
	afx_msg void OnPopupExit();
	afx_msg void OnPopupSignin();
	afx_msg void OnPopupOpen();
	afx_msg void OnPopupSingout();
	afx_msg void OnLinksSupportspacehomepage();
	afx_msg void OnStatusAvailible();
	afx_msg void OnStatusAway();
	afx_msg void OnStatusBusy();
	afx_msg void OnSettings();
	afx_msg void OnHelpAbout();
	afx_msg void OnLinksMyprofile();
	afx_msg void OnLinksExpertChat();
	afx_msg void OnLinksExpertForum();
	afx_msg void OnHelpHelp();
	afx_msg void OnStatusOnnlineForCustomer();
	afx_msg void OnLinksSessionshistory();
	afx_msg void OnLinksExpertportal();
	afx_msg void OnLinksMymessages();

//	Menu calls end
public:
	// Added 3 functions GetIconForItem OnDrawItem OnMeasureItem
	// Modified one funcion OnInitMenuPopup
	// ON_WM_DRAWITEM()  ON_WM_MEASUREITEM() ON_WM_INITMENUPOPUP()
	HICON		 GetIconForItem(UINT itemID);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpdis);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpmis);
	void OnOK(){};//do nothing just handle

//	Callbacks called by JS as response to user inputs and events begin
	HRESULT OnCbkEventsHandlerClickedEvent(IHTMLElement *pElement);

	HRESULT OnPageLoaded(IHTMLElement *pElement);
	HRESULT OnFlagChanged(IHTMLElement *pElement);
	HRESULT OnManageFlags(IHTMLElement *pElement);
	HRESULT OnFwdToContact(IHTMLElement *pElement);
	HRESULT OnManageContacts(IHTMLElement *pElement);
	HRESULT OnSnooze(IHTMLElement *pElement);
	HRESULT OnGoToCalender(IHTMLElement *pElement);
	HRESULT OnSendReply(IHTMLElement *pElement);
	HRESULT OnSendCustomReply(IHTMLElement *pElement);
	HRESULT OnGoToManageReplies(IHTMLElement *pElement);
	HRESULT OnCloseMessenger(IHTMLElement *pElement);
	HRESULT OnMinMessenger(IHTMLElement *pElement);
	HRESULT OnInboxPickupCall(IHTMLElement *pElement);
	HRESULT OnSettingsApp(IHTMLElement *pElement);
	HRESULT OnRestoreApp(IHTMLElement *pElement);
	HRESULT OnMinimizeApp(IHTMLElement *pElement);
	HRESULT OnCloseApp(IHTMLElement *pElement);
	HRESULT OnNewCallAdded(IHTMLElement *pElement);
	HRESULT OnLoginPageStart(IHTMLElement *pElement);
	HRESULT OnUpdateVersion(IHTMLElement *pElement);
	HRESULT OnRequestStatusChanged(IHTMLElement *pElement);
	HRESULT OnSettingsOk(IHTMLElement *pElement);
	HRESULT OnSettingsClose(IHTMLElement *pElement);
	HRESULT OnOpenLink(IHTMLElement *pElement);
	HRESULT OnConnectionTest(IHTMLElement *pElement);
	HRESULT OnNewConsultRequestAdded(IHTMLElement *pElement);
	HRESULT OnConsultStatusChanged(IHTMLElement *pElement);
	HRESULT OnAddDhtmlLog(IHTMLElement *pElement);
	HRESULT OnRequestAdded(IHTMLElement *pElement, eCallType ReqType);
	HRESULT OnSetStargateStatus(IHTMLElement *pElement);
	HRESULT OnShowMessageBox(IHTMLElement *pElement);
	HRESULT OnStargateOfflineMsgNotification(IHTMLElement *pElement);
	
//	Callbacks called by JS as response to user inputs and events end

//	Events from system begin
	HRESULT OnPowerBroadCast(WPARAM wParam, LPARAM lPara);
//  Events from system end

//	Events come from alert dialogs begin
	LRESULT OnAlertDlgNewCallExpand( WPARAM wParam, LPARAM lParam );
	LRESULT OnAlertDlgNewCallPickUp( WPARAM wParam, LPARAM lParam );
	LRESULT OnAlertDlgNewCallClose( WPARAM wParam, LPARAM lParam );

	LRESULT OnAlertDlgMissedCallsClose(WPARAM wParam, LPARAM lParam);
	LRESULT OnAlertDlgMissedCallsPickUp(WPARAM wParam, LPARAM lParam);

	LRESULT OnAlertDlgOfflineMsgPickUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnAlertDlgOfflineMsgClose(WPARAM wParam, LPARAM lParam);

	LRESULT OnAlertDlgInfoMsgClose(WPARAM wParam, LPARAM lParam);
	LRESULT OnAlertDlgInfoMsgHelp(WPARAM wParam, LPARAM lParam);

	LRESULT OnWorkBenchClose(WPARAM wParam, LPARAM lParam);
	LRESULT OnOnline4CustomerClose(WPARAM wParam, LPARAM lParam);
	LRESULT OnOnline4CustomerSubmit(WPARAM wParam, LPARAM lParam);

	LRESULT CloseConsultWorkBench(long lUid);

	BOOL IsFreeSpaceToShowAlert();


//	Events come from alert dialogs end

//	Events from Update dialog
	LRESULT OnAlertDlgUpdateClose(WPARAM wParam, LPARAM lParam);
	LRESULT OnAlertDlgUpdatePickUp(WPARAM wParam, LPARAM lParam);

//	Events come from IM client begin
	LRESULT OnIMConnected( WPARAM wParam, LPARAM lParam );
	LRESULT OnIMNewCall( WPARAM wParam, LPARAM lParam );
	LRESULT OnIMTLSConnected( WPARAM wParam, LPARAM lParam );
	LRESULT OnIMDisconnect( WPARAM wParam, LPARAM lParam );
	LRESULT	OnIMConnectFailed( WPARAM wParam, LPARAM lParam );
	LRESULT	OnIMConnecting( WPARAM wParam, LPARAM lParam );
//	Events come from IM client end

	LRESULT OnIMChannelQualityTestTime(WPARAM wParam, LPARAM lParam);
	LRESULT OnIMChannelQualityIssueDetected(WPARAM wParam, LPARAM lParam);
	LRESULT OnIMChannelQualityImprovementDetected(WPARAM wParam, LPARAM lParam);

	LRESULT OnIMChannelQualityTestResponse(tstring subject);

//  Commands wrappers to IM client begin
	LRESULT IMSignIn();
	HRESULT IMSignOut();
	HRESULT IMSendUpdateStatus(SupporterStatus status);
	HRESULT OnMissedCallsCounterChnanged();
//  Commands to IM client end

//  Actual operations with calls
	LRESULT DoPickUp(long lUid, eCallType callType, CString sCustomerDisplayName);
	//LRESULT DoReply( long  lUid );
	//LRESULT DoFdwToConatct( long  lUid );

//	Local set/get functions that may be used to modify m_MessengerState parameters
	void	SetSelectedStatus(SupporterStatus status);	
	void	SetSupporterSelectedStatus(SupporterStatus status);
	void	SetStargateSelectedStatus(SupporterStatus status);
	void	SetConnectionState(IMConnectionState state);

	void	CloseMissedCallsAlert();

//	Update 	tray icon due defined priority 
	void	UpdateTrayIcon(TrayIconChangeReason updateReason);

	LRESULT StopAnimation();
	LRESULT CloseAlert(long lUid);
	LRESULT CloseAllAlerts(); 
	LRESULT CloseAllNewCallAlerts();
	DWORD   CountNewCallAlerts();

	LRESULT OnUpdateRequired();

	LRESULT	LoadDHTMLLoginDialogData();

	HRESULT UpdateLoginWindow();
	LRESULT ShowLogoutWindow(sErrorInfo* pErrInfo);
	HRESULT ShowSettingsWindow();

	void	ReloadCurrentSupporterData();

	LRESULT OnActivityHandlerInActive(WPARAM wParam, LPARAM lParam);
	LRESULT OnActivityHandlerBackActive(WPARAM wParam, LPARAM lParam);

	LRESULT OnUpdateCompleted(WPARAM wParam, LPARAM lParam);
	LRESULT OnUninstallStarted(WPARAM wParam, LPARAM lParam);

	void	StopActivityHandler();
	void	RestartActivityHandler();

	HRESULT OnNewCallPicked(long lCallUID);
	HRESULT OnInSessionCallStatusChanged(long lCallUID, BOOL bStayOnline4Customer);

	HRESULT OnNewConsultCallPicked(long lCallUID);
	HRESULT OnInSessionCounsltCallStatusChanged(long lCallUID);

private:
	void	CustomizeTrayMenu(CMenu* pMenu);
	void    UpdateGroupOfItems(CMenu* pMenu, BOOL bEnable);

public:
	CSupporterLocalData* m_pcSupporterLocalData;// supporter's local data stored in registry

private:
	IconsStr			m_srtIcons;				// struct with all icons used by the application 
	
	CUrlOpener			m_cUrlOpener;			// helper class for opening urls 
	CMessengerLocalData m_cMessengerLocalData;  // messnger's local data stored in registry 

	AlertsMap			m_mapAlerts;			// map of alerts shown for user 

	CTrayNotifyIcon		m_TrayIcon;				// tray icon encapsulation helper class
	CCommunicator		m_cCommunicator;		// communicator interface to gloox client
	CActivityHandler	m_cActivityHandler;		// handle of supporter activity on PC for "idle" feature

	MessengerStateStr   m_MessengerState;		// keep information about state of Messnger
	
	CAlertDlgUpdate*    m_pcAlertDlgUpdate;		// pointer to Update modelless Dialog
	CAlertDlgMissedCalls*	m_pcAlertDlgMissedCalls;
	COnline4CustomerDlg* m_cOnline4CustomerDlg;
	
	CUpdateMonitor		m_cUpdateMonitor;		// update monitoring 
	CString				m_sMinRequiredVersion;	// minimum required version TODO in sprint 4
	long				m_lFreePosition;
	CAboutDlgInfo		m_AboutDlgInfo;
	CSHA1				m_crypt;
	int					m_iLastTestedCustDirPort;
	CWorkbenchDlg*		m_pWorkbenchDlg;		// pointer to CWorkbenchDlg
	WorkbenchDlgConsultsMap	m_mapWorkbenchDlgConsults; //contains map of WorkBanches opened for Consult
	CIMChannelQualityMonitor m_cIMChannelQualityMonitor;
	
};
