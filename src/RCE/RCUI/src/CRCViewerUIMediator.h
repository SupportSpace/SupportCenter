/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRCViewerUIMediator.h
///
///  CRCViewerUIMediator, mediator between CCoRCViewer and his UI
///
///  @author "Archer Software" Kirill Solovyov @date 18.06.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include "CRCViewerSessionDlg.h"
#include "CRCViewerStatusBarDlg.h"
#include "RCViewerImpl.h"
#include "CSkinnedElement.h"

/// Extended desktop sharing state enum
enum EDSStateEx
{
	EDSS_PERMISSION_REQUEST_SENT = 1,
	EDSS_PERMISSION_RECEIVED,
	EDSS_PERMISSION_DENIED,
	EDSS_INSTALLATION_PROGRESS,
	EDSS_INSTALLATION_SUCCESSFUL,
	EDSS_INSTALLATION_FAILED,
	EDSS_CONNECTING,
	EDSS_SESSION_FAILED,
	EDSS_INIFIAL,
	EDSS_STATES_COUNT
};

/// Desktop sharing viewr invariant
typedef struct _SDSViewerState
{
	/// true if desktop sharing is off
	bool m_on;

	/// Extra desktop sharing state
	EDSStateEx m_extendedState;

	/// ctor
	_SDSViewerState(const bool on, const EDSStateEx extendedState)
		: m_on(on), m_extendedState(extendedState)
	{
	}

	/// Comparision operator
	bool operator != (const _SDSViewerState& state)
	{
		return m_on != state.m_on || m_extendedState != state.m_extendedState;
	}

} SDSViewerState;

///// UI statuses enumerator
//enum ERCViewerUIStatuses
//{
//	//mn blue	Desktop Sharing - Off. Waiting for customer approval.
//	UIS_PERMISSION_REQUEST_SEND=0,
//	//mn green Desktop Sharing - Off. Customer approved.
//	UIS_PERMISSION_RECIEVED,
//	//mn red Desktop Sharing - Off. Customer declined.
//	UIS_PERMISSION_DENIED,
//	//as blue Desktop Sharing - Off. Installation in progress on the customer's computer.
//	UIS_INSTALLATION_INPROGRESS,
//	//as green Desktop Sharing - Off. Successful installation.
//	UIS_INSTALLATION_SUCCESSFUL,
//	//as red Desktop Sharing - Off. Installaion failed.
//	UIS_INSTALLATION_FAILED,
//	//el gray	Desktop Sharing - Off.
//	UIS_SESSION_OFF,
//	//el blue Desktop Sharing - Off. Connecting to customer.
//	UIS_SESSION_CONNECTING,
//	//el green Desktop Sharing - On.
//	UIS_SESSION_ON,
//	//el red Desktop Sharing - Off. Connection failed. Click Retry.
//	UIS_SESSION_FAILED,
//	
//	//new statuses add here
//
//	// statuses count
//	UIS_STATUSES_COUNT
//};

/// UI events enumerator
enum ERCViewerUIEvents
{
	/// session start request
	UIE_SESSION_START=0,
	/// session stop request
	UIE_SESSION_STOP,
	/// permission request
	UIE_PERMISSION_REQUEST
};

class CCoRCViewer;
/// class mediator between UI and CCoRCViewer class and creater of UI
class CRCViewerUIMediator : public CSkinnedElement
{
private:
	/// first time creation flag for prohibition more one creation.
	bool m_firstTimeCreation;
	// parent window of UI
	CWindow m_parentWnd;
	/// pointer of owner CCoRCViewer object
	CCoRCViewer *m_viewer;
	//RC session modeless dialog
	CRCViewerSessionDlg m_sessionDlg;
	// Status bar modeless dialog 
	CRCViewerStatusBarDlg m_statusBarDlg;
	//For vertical Scroll Bar
	CScrollBar	m_vscrollBar;
	//For Horizontal Scroll Bar
	CScrollBar	m_hscrollBar;
	/// Viewer displayMode
	int m_displayMode;
protected:
	/// Specifies how long it takes to play the animation, in milliseconds. 
	enum { ANIMATE_TIME=100 };
	/// handler Start button click
	/// @param permission current permission
	void OnSessionStartBtnClick(int permission);
	/// handler of permission combox selection change
	/// @param permission current permission
	void OnSessionPermissionCmbSelChange(int permission);

	/// current DSViewer status 
	SDSViewerState m_status;
	/// poineter on image of statuses icon
	CImage *m_uiStatusIcons[EDSS_STATES_COUNT];
	/// handler properties Apply button click
	void OnPropertiesApplyBtnClick(int displayIndex,int colorsIndex,int compressionIndex);
	/// Send ctrl + alt + del button click handler
	void OnCADBtnClick();
	/// Combobox selection changed on properties page event handler
	void OnPropertiesComboboxSelChanged();
	/// ShowProperties button click handler
	void OnPropertiesBtnClick();
	/// common font
	boost::shared_ptr<LOGFONT> m_logFont;
	/// bold font
	boost::shared_ptr<LOGFONT> m_logFontBold;
	/// peer id of host side. for UI use.
	tstring m_peerId;
public:
	CRCViewerUIMediator(CCoRCViewer* owner);
	virtual ~CRCViewerUIMediator(void);
	/// The method create UI dialogs
	void CreateUI(HWND parentWnd);
	/// The method destroy UI dialogs
	void DestroyUI();
	/// handler of parent WM_SIZE
	LRESULT Size(WPARAM wParam, LPARAM lParam);
	/// set UI status, call by owner
	/// @param status
	/// @param message
	void SetUIStatus(const SDSViewerState& status, const tstring& message);

	/// set UI status, not changing session on/off state
	void SetExtendedUIStatus(const EDSStateEx& status, const tstring& message);
	
	/// Method retrieve HWND of viewer host window
	/// @return HWND of viewer host window
	HWND GetViewerHostWnd(void);

	/// Returns vertical scroll bar window handle
	HWND GetVertScrollBar();

	/// Returns horisontal scroll bar window handle
	HWND GetHorSrcrollBar();

	/// Viewer Init()
	void OnViewerInit(tstring& peerId);

	/// Session started event handler
	void OnSessionStarted();

	/// Session stopped event handler
	void OnSessionStopped(const ESessionStopReason stopReason);

	/// Display mode changed event handler
	/// @param mode new display mode
	void OnDisplayModeChanged(EDisplayMode mode);

	/// Should be called each time skin was changed
	virtual void OnSkinChanged() {};

	/// Accessor to underlying window handle.
	virtual HWND GetWindowHandle() { return NULL; };
};
// because RCViewerUIMediator is created static as data member
#include "RCViewerAXCtrl.h"
