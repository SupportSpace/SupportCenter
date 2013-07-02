// CWorkbenchDlg.cpp implementation
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CWorkbenchDlg
//
//-------------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CWorkbenchDlg : implementation of the Workbench Dialog window
//-------------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 10/29/2006 05:21:10 PM
// Comments: First Issue
//===========================================================================
#include "stdafx.h"
#include "WorkbenchDlg.h"
#include "AidLib/CException/CException.h"
#include <AidLib/Logging/cLog.h> 
#include "SupportMessenger.h"
#include "custsite.h"

  // The structure below contains all of the AppBar settings that
   // can be saved/loaded in/from the Registry.
   typedef struct {
      DWORD m_cbSize;         // Size of this structure
      UINT  m_uState;         // ABE_UNKNOWN, ABE_FLOAT, or ABE_edge
      BOOL  m_fAutohide;      // Should AppBar be auto-hidden when docked?
      BOOL  m_fAlwaysOnTop;   // Should AppBar always be on top?
      UINT  m_auDimsDock[4];  // Width/height for docked bar on 4 edges
      CRect m_rcFloat;        // Floating rectangle (in screen coordinates)
   } APPBARSTATE, *PAPPBARSTATE;

extern CSupportMessengerApp theApp;

#define MIN_UI_WIDTH	263
#define MIN_UI_HEIGHT	480

//
//	define pointer to Callback function of the class CWorkbenchDlg
//
typedef HRESULT (CWorkbenchDlg::*pfnCallbackMsngrDlg)(IHTMLElement *pElement);
//
//	define pair used for mapping string and callbacks
//
typedef std::pair<CString, pfnCallbackMsngrDlg> DHTML_CUSTOM_EVENT_MSNGR_DLG;
//
//	init pairs map of calbacks
//
static const DHTML_CUSTOM_EVENT_MSNGR_DLG callBacksPairsMsngrDlg[] =
{
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("pageLoaded"),CWorkbenchDlg::OnPageLoaded),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("closePage"),CWorkbenchDlg::OnClosePage),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("SetTitle"),CWorkbenchDlg::OnSetTitle)
	
};
//
//	define map of callbacks
//
const static std::map<CString, pfnCallbackMsngrDlg> mapCallbacksMsngrDlg(callBacksPairsMsngrDlg, callBacksPairsMsngrDlg + sizeof(callBacksPairsMsngrDlg)/sizeof(callBacksPairsMsngrDlg[0]));
typedef std::map<CString, pfnCallbackMsngrDlg>::const_iterator MapCallbacksMsngrDlgIterator;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_DHTML_EVENT_MAP(CWorkbenchDlg)
//main entry point for DHTML predefined callbacks
DHTML_EVENT_ONCLICK(_T("eCbkEventsHandler"), OnCbkEventsHandlerClickedEvent)
//ban the selecting action
//DHTML_EVENT_TAG_ALL(DISPID_HTMLELEMENTEVENTS_ONSELECTSTART, OnHtmlSelectStart)
//handle all the dragging action
//DHTML_EVENT_TAG_ALL(DISPID_HTMLELEMENTEVENTS_ONDRAGSTART, OnHtmlDragStart)
//handle all the context menu
//DHTML_EVENT_TAG_ALL(DISPID_HTMLELEMENTEVENTS_ONCONTEXTMENU, OnHtmlContextMenu)
END_DHTML_EVENT_MAP()

BEGIN_MESSAGE_MAP(CWorkbenchDlg , CDHtmlDialogEx)
	ON_WM_PAINT()

	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	
	//ON_COMMAND(ID_POPUP_SIGNIN, &CSupportMessengerDlg::OnPopupSignin)

	ON_WM_INITMENUPOPUP()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_DRAWITEM()
    ON_WM_MEASUREITEM()

	ON_WM_TIMER()
	ON_MESSAGE(WM_WORKBENCH_NEW_SUPPORT_SESSION, OnNewSupportSession)
	ON_MESSAGE(WM_WORKBENCH_NEW_CONSULT_SESSION, OnNewConsultSession)
	
END_MESSAGE_MAP()

CWorkbenchDlg::CWorkbenchDlg (CWnd* pParent, CString sNavigateUrl, long lSessionID, HWND	hNotifWnd, eCallType CallType, CString sCustomerDisplayName)
: CDHtmlDialogEx(CWorkbenchDlg::IDD, CWorkbenchDlg::IDH, pParent)
{
TRY_CATCH 
	
	m_lSessionID = lSessionID;
	m_sDestUrl = sNavigateUrl;
	m_hNotifWnd = hNotifWnd;
	m_eCallType = CallType;
	m_sCustomerDisplayName = sCustomerDisplayName;
	m_PageLoadedFlag = FALSE;
	m_RetryNum = 0;

	Create(IDD, pParent); // Create modeless dialog - trick for Tray icon based on CDialog not on CFrame

CATCH_LOG(_T("CWorkbenchDlg ::CWorkbenchDlg"))
}

CWorkbenchDlg::~CWorkbenchDlg()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CWorkbenchDlg::CWorkbenchDlg"));

CATCH_LOG(_T("CWorkbenchDlg::~CWorkbenchDlg"))
}

void CWorkbenchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

// CSupportMessengerDlg  message handlers
BOOL CWorkbenchDlg::OnInitDialog()
{
TRY_CATCH 

	// PRB: App Desktop Toolbars Must Have WS_EX_TOOLWINDOW Style
	// http://support.microsoft.com/kb/132965

	CDHtmlDialogEx::OnInitDialog();

	//
	//	TODO after Noam will modify Worknench. This may be drgaeble and Minimize,Maximize,Close 
	//  uninstall need Caption, but we don't need border
	//	caption added in resource file and border removed here
	//
	//  LONG nOldStyle = ::GetWindowLong(this->m_hWnd, GWL_STYLE);
	//  LONG nNewStyle = nOldStyle & ~WS_BORDER;
	//  ::SetWindowLong(this->m_hWnd, GWL_STYLE, nNewStyle);

	//	Set the icon for this dialog.  The framework does this automatically
	//	when the application's main window is not a dialog
	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);	// Set big icon
	//SetIcon(m_srtIcons.m_hMainApp, FALSE);	// Set small icon

	//
	//
	m_WorkbenchStartTime = time(NULL);

	//	todo take this from configuration
	//	m_cSettings.ReadConnectionInfo();
	Log.Add (_MESSAGE_, _T("m_sDestUrl = %s"), m_sDestUrl);
	Navigate(m_sDestUrl);

	//	Init AppBar 
	InitAppBar();

	//
	//	Change text of the window caption STL-419
	//
	CString		sTitle;

	switch(m_eCallType)
	{
	case CustomerDirectCall:
		sTitle.FormatMessage(_T("SupportSpace Workbench - session with %1!s!"), m_sCustomerDisplayName); 
		break;
	case ConsultCall:
		sTitle = _T("SupportSpace Workbench - consultation"); //todo - consulting [Expert Name]" 
		break;
	default:
		break;
	}

	//  set window text
	SetWindowText(sTitle);

	//  Due request from Noam we enable F5 in  
	EnableF5(TRUE);

	// Due request from Noam there is option to close workbench using Ctrl+F4
	EnableCtrlF4(TRUE);

	// Workbench issue STL-616 and SupportCenter STL-682 
	EnableWorkbenchMode(TRUE);

	//	Show Maximized window like F11 mode in previous version
	ShowWindow(SW_SHOWMAXIMIZED);

CATCH_LOG(_T("CWorkbenchDlg::OnInitDialog"))	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWorkbenchDlg::OnPaint()
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
		//dc.DrawIcon(x, y, m_srtIcons.m_hMainApp); TODO?
	}
	else
	{
		CDialog::OnPaint();
	}

	CDialog::OnPaint();
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWorkbenchDlg::OnQueryDragIcon()
{
	return 0;
	//return static_cast<HCURSOR>(m_srtIcons.m_hMainApp);// TODO?
}

void CWorkbenchDlg::PostNcDestroy() 
{
TRY_CATCH 
	
	Log.Add(_MESSAGE_,_T("CWorkbenchDlg::PostNcDestroy is called before delete this"));
	delete this;
	Log.Add(_MESSAGE_,_T("CWorkbenchDlg::PostNcDestroy is called after delete this"));

CATCH_LOG(_T("CWorkbenchDlg::PostNcDestroy"))
}
//todo this mehotd is not called by js of CWorkbenchDlg
HRESULT CWorkbenchDlg::OnPageLoaded(IHTMLElement *pElement)
{
TRY_CATCH 

	if(IsPageLoaded())
	{
		//ShowWindow(SW_SHOWMAXIMIZED);
	}

CATCH_LOG(_T("CWorkbenchDlg::OnPageLoaded"))
	return 0;
}

//todo this mehotd is not called by js of CWorkbenchDlg
HRESULT CWorkbenchDlg::OnClosePage(IHTMLElement *pElement)
{
TRY_CATCH 
	
	OnCancel();
	OnClose();
	
CATCH_LOG(_T("CWorkbenchDlg::OnPageLoaded"))
	return 0;
}

//this function called to set workbench title
HRESULT CWorkbenchDlg::OnSetTitle(IHTMLElement *pElement)
{
TRY_CATCH 
	
	VARIANT	tmpTitleText;
	CString	sTitleText;
	
	pElement->getAttribute( L"sTitleTxt", 0, &tmpTitleText);
	if(tmpTitleText.vt != VT_NULL)
	{
		m_PageLoadedFlag = TRUE;
		sTitleText = (CString)tmpTitleText;
		Log.Add(_MESSAGE_,_T("CWorkbenchDlg::OnSetTitle %s"), sTitleText);
		//  set window text
		sTitleText.Replace("&#034;","\"");//STG-4441
		SetWindowText(sTitleText);
	}
	
CATCH_LOG(_T("CWorkbenchDlg::OnSetTitle"))
	return 0;
}


//	todo OnDocumentComplete is called not once as expected
//	we added special CALLBACK_PageLoaded
//
void CWorkbenchDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
TRY_CATCH 

	CDHtmlDialogEx::OnDocumentComplete(pDisp, szUrl);

	theApp.FinishInitialization();
	if(IsPageLoaded())
	{
		if(m_PageLoadedFlag == TRUE)
		{
			Log.Add(_MESSAGE_,_T("Page is loaded correct for URL: %s"), szUrl);
		}
		else
		{
			time_t workbenchOpenDuration  = time(NULL) - m_WorkbenchStartTime;
			Log.Add(_ERROR_,_T("Page is loaded for URL: %s. SetTitle not called. RetryNum: %d. Duration: %d"), szUrl, m_RetryNum, workbenchOpenDuration);
			
			if(m_RetryNum==0 && workbenchOpenDuration < 15) 
			{
				Log.Add(_WARNING_,_T("Renavigate silently"));
				Navigate(m_sDestUrl);
				m_RetryNum++;
			}
			else
			{
				Log.Add(_WARNING_,_T("Suggest to reopen Workbench"));

				if(AfxMessageBox(_T("The Workbench could not be loaded. \nWould you like to close and reopen the Workbench to try and solve the problem? \n\nTo report the problem to SupportSpace, send an email with error code: 4128 to bugs@supprtspace.com"), MB_OKCANCEL) == IDOK )
				{
					Log.Add(_WARNING_,_T("reopen Workbench selected...Open new Workbench and close old"));
					//Navigate(m_sDestUrl); //not helpful in case of hosts changes for example
					theApp.LauchWorkbenchProcess(m_sDestUrl, m_eCallType, m_sCustomerDisplayName, m_lSessionID);
					PostQuitMessage(0);
				}
				else
				{
					Log.Add(_WARNING_,_T("Close messagebox selected...Do nothing"));
				}
			}
		}
		//ShowWindow(SW_SHOWMAXIMIZED);
	}

CATCH_LOG(_T("CWorkbenchDlg::OnDocumentComplete"))
}


BOOL CWorkbenchDlg::InitAppBar() 
{
TRY_CATCH 
	APPBARSTATE abs;
	//
	// Check the registry to see if we have been used before and if so,
	// reload our persistent settings.
	DWORD cbData = sizeof(abs);

	CRect rc;

	// Set the CAppBar class's behavior flags
	// m_fdwFlags = ABF_ALLOWANYWHERE | ABF_MIMICTASKBARAUTOHIDE | ABF_MIMICTASKBARALWAYSONTOP;

	GetClientRect(&rc);

	// Width has no limits, height sizes in client-area-height increments
	//m_szSizeInc.cx = 1;
	//m_szSizeInc.cy = rc.Height(); 

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

	// 
	CRect	screen_rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &screen_rect, 0);

	// Make the default width twice the width set in the dialog editor
	// abs.m_rcFloat.right += abs.m_rcFloat.Width();
	// abs.m_rcFloat.bottom = MIN_UI_WIDTH;
	abs.m_rcFloat.right = screen_rect.right;

	// Make the default bottom x10 the width set in the dialog editor
	// abs.m_rcFloat.bottom = 10*abs.m_rcFloat.Height();
	// abs.m_rcFloat.bottom = MIN_UI_HEIGHT;
	abs.m_rcFloat.bottom = screen_rect.bottom;

	// Temporarily turn off window adornments to determine the dimensions
	// of the appbar when docked.
	//HideFloatAdornments(TRUE);
	AdjustWindowRectEx(&rc, GetStyle(), FALSE, GetExStyle());
	//HideFloatAdornments(FALSE); TODO?
	abs.m_auDimsDock[ABE_LEFT]   = rc.Width()*2;
	abs.m_auDimsDock[ABE_TOP]    = rc.Height();
	abs.m_auDimsDock[ABE_RIGHT]  = rc.Width()*2;
	abs.m_auDimsDock[ABE_BOTTOM] = rc.Height();
	
	// Set the initial state of the appbar.
  	SetWindowPos(
		NULL, abs.m_rcFloat.left, abs.m_rcFloat.top, abs.m_rcFloat.Width(), abs.m_rcFloat.Height(), 
	SWP_NOZORDER | SWP_NOACTIVATE);

CATCH_LOG(_T("CWorkbenchDlg::InitAppBar"))

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CWorkbenchDlg::HideFloatAdornments(BOOL fHide) {
TRY_CATCH 

	//
	//	change style: delete WS_EX_TOOLWINDOW style to show Minimized
	//
	if(fHide==TRUE)
	{
		SetWindowLong(this->m_hWnd, GWL_EXSTYLE, ::GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
	}
	else
	{
		SetWindowLong(this->m_hWnd, GWL_EXSTYLE, ::GetWindowLong(this->m_hWnd, GWL_EXSTYLE) & ~WS_EX_TOOLWINDOW);
	}

CATCH_LOG(_T("CWorkbenchDlg::HideFloatAdornments"))
}

// Tell our derived class that there is a proposed state change
void CWorkbenchDlg::OnAppBarStateChange (BOOL fProposed, UINT uStateProposed) {

	// Hide the window adorments when docked.   //TODO Anatoly - we do not need to Hide it
	HideFloatAdornments((uStateProposed == ABE_FLOAT) ? FALSE : TRUE);
}

void CWorkbenchDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_CLOSE) {
		// We have to manually add this so that the dialog box closes when 
		// when the user clicks the close button (which appears in the top, right)
		// corner of the dialog box when it is floating).

		// we have to call JS interface method STL-587
		Log.Add((_T("CWorkbenchDlg call INTERFACE_UpdateData with parameter: shuttle_close_workbench")));
		if( m_cHTMLInterface.INTERFACE_UpdateData("shuttle_close_workbench", _T(""))  == S_FALSE )
		{
			// if we have a case when workbench is opened with error 500 and so on ...
			// then INTERFACE_UpdateData is not exists as destop.js as well
			// in this case workbench is closed by c++ code
			OnCancel();
			OnClose();
		}
		return;
	}

	CDHtmlDialogEx::OnSysCommand(nID, lParam);
}

void CWorkbenchDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
TRY_CATCH 

	// Get the minimum size of the window assuming it has no client area.
	// This is the width/height of the window that must always be present
	CRect rcBorder(0, 0, 0, 0);
	AdjustWindowRectEx(&rcBorder, GetStyle(), FALSE, GetExStyle());

	lpMMI->ptMinTrackSize.x = MIN_UI_WIDTH; //todo max and min size
	lpMMI->ptMinTrackSize.y = MIN_UI_HEIGHT;
	return;

CATCH_LOG(_T("CWorkbenchDlg::OnGetMinMaxInfo"))
}


void CWorkbenchDlg::OnSize(UINT nType, int cx, int cy) {

	CDHtmlDialogEx::OnSize(nType, cx, cy);
}

void CWorkbenchDlg::OnCancel()
{
TRY_CATCH 

	CDHtmlDialogEx::OnCancel();

CATCH_LOG(_T("CWorkbenchDlg::OnCancel"))
}

void CWorkbenchDlg::OnClose()
{
TRY_CATCH 

	if(m_hNotifWnd != NULL)
	{	
		// TODO notify SupportCenter here that Workbench is closed ... 
		//AfxPostQuitMessage(WM_QUIT);
		::PostMessage(m_hNotifWnd, WM_WORKBENCH_CLOSE, 0, 0);
	}
	else
	{
		Log.Add((_T("CWorkbenchDlg::OnClose(). m_hNotifWnd is NULL"))); 		
	}

	// close application properly
	DestroyModeless();
	//PostQuitMessage(0); 
	
CATCH_LOG(_T("CWorkbenchDlg::OnClose"))
}


HRESULT CWorkbenchDlg::OnCbkEventsHandlerClickedEvent(IHTMLElement *pElement)
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

CATCH_LOG(_T("CWorkbenchDlg::OnCbkEventsHandlerClickedEvent"))

	return S_OK;
}

BOOL CWorkbenchDlg::CreateControlSite(COleControlContainer* pContainer, COleControlSite** ppSite, UINT, REFCLSID) 
{
	Log.Add(_MESSAGE_,_T("CWorkbenchDlg::CreateControlSite"));
	CCustomControlSite *pBrowserSite = new CCustomControlSite(pContainer, this);
	if (!pBrowserSite)
		return FALSE;
	*ppSite = pBrowserSite;
	return TRUE;
}

LRESULT CWorkbenchDlg::OnNewSupportSession(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 
	
	long lUid = (long)lParam;//uid of support request or notification
	CString sUid;

	sUid.FormatMessage(_T("%1!d!"), lUid); 

	Log.Add(_MESSAGE_,_T("CWorkbenchDlg::OnNewSupportSession called '%s' "), sUid);

	Log.Add((_T("CWorkbenchDlg::OnNewSupportSession call INTERFACE_UpdateData with parameter: shuttle_new_support_session")));

	if( m_cHTMLInterface.INTERFACE_UpdateData("shuttle_new_support_session", sUid)  == S_FALSE )
	{
		// if we have a case when workbench is opened with error 500 and so on ...
		// then INTERFACE_UpdateData is not exists as destop.js as well
		Log.Add(_ERROR_,_T("call INTERFACE_UpdateData(shuttle_new_support_session) failed"));
	}

CATCH_LOG(_T("CWorkbenchDlg::OnNewSupportSession"))
	return 0L;
}

LRESULT CWorkbenchDlg::OnNewConsultSession(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH 

	long lUid = (long)lParam;//uid of consult notification
	CString sUid;
	sUid.FormatMessage(_T("%1!d!"), lUid); 

	Log.Add(_MESSAGE_,_T("CWorkbenchDlg::OnNewConsultSession called '%s' "), sUid);

	Log.Add((_T("CWorkbenchDlg::OnNewConsultSession call INTERFACE_UpdateData with parameter: shuttle_new_consult_session")));

	if( m_cHTMLInterface.INTERFACE_UpdateData("shuttle_new_consult_session", sUid)  == S_FALSE )
	{
		// if we have a case when workbench is opened with error 500 and so on ...
		// then INTERFACE_UpdateData is not exists as destop.js as well
		Log.Add(_ERROR_,_T("call INTERFACE_UpdateData(shuttle_new_consult_session) failed"));
	}

CATCH_LOG(_T("CWorkbenchDlg::OnNewSupportSession"))
	return 0L;
}

