// COnline4CustomerDlg.cpp implementation
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								COnline4CustomerDlg
//
//-------------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// COnline4CustomerDlg : implementation of the COnline4CustomerDlg Dialog window
//-------------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 10/29/2006 05:21:10 PM
// Comments: First Issue
//===========================================================================
#include "stdafx.h"
#include "Online4CustomerDlg.h"
#include "AidLib/CException/CException.h"
#include <AidLib/Logging/cLog.h> 
#include "SupportMessenger.h"
#include "custsite.h"
 
extern CSupportMessengerApp theApp;

#define MIN_UI_WIDTH	249
#define MIN_UI_HEIGHT	219

//
//	define pointer to Callback function of the class COnline4CustomerDlg
//
typedef HRESULT (COnline4CustomerDlg::*pfnCallbackMsngrDlg)(IHTMLElement *pElement);
//
//	define pair used for mapping string and callbacks
//
typedef std::pair<CString, pfnCallbackMsngrDlg> DHTML_CUSTOM_EVENT_MSNGR_DLG;
//
//	init pairs map of calbacks
//
static const DHTML_CUSTOM_EVENT_MSNGR_DLG callBacksPairsMsngrDlg[] =
{
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("pageLoaded"),COnline4CustomerDlg::OnPageLoaded),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("closePage"),COnline4CustomerDlg::OnClosePage),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("Online4CustomerPageSubmit"),COnline4CustomerDlg::Online4CustomerPageSubmit),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("Online4CustomerPageClose"),COnline4CustomerDlg::OnClosePage),
	DHTML_CUSTOM_EVENT_MSNGR_DLG(_T("SetTitle"),COnline4CustomerDlg::OnSetTitle)
	
};
//
//	define map of callbacks
//
const static std::map<CString, pfnCallbackMsngrDlg> mapCallbacksMsngrDlg(callBacksPairsMsngrDlg, callBacksPairsMsngrDlg + sizeof(callBacksPairsMsngrDlg)/sizeof(callBacksPairsMsngrDlg[0]));
typedef std::map<CString, pfnCallbackMsngrDlg>::const_iterator MapCallbacksMsngrDlgIterator;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_DHTML_EVENT_MAP(COnline4CustomerDlg)
//main entry point for DHTML predefined callbacks
DHTML_EVENT_ONCLICK(_T("eCbkEventsHandler"), OnCbkEventsHandlerClickedEvent)
//ban the selecting action
DHTML_EVENT_TAG_ALL(DISPID_HTMLELEMENTEVENTS_ONSELECTSTART, OnHtmlSelectStart)
//handle all the dragging action
DHTML_EVENT_TAG_ALL(DISPID_HTMLELEMENTEVENTS_ONDRAGSTART, OnHtmlDragStart)
//handle all the context menu
//DHTML_EVENT_TAG_ALL(DISPID_HTMLELEMENTEVENTS_ONCONTEXTMENU, OnHtmlContextMenu)
END_DHTML_EVENT_MAP()

BEGIN_MESSAGE_MAP(COnline4CustomerDlg , CDHtmlDialogEx)
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
	
END_MESSAGE_MAP()

COnline4CustomerDlg::COnline4CustomerDlg (CWnd* pParent, CString sNavigateUrl, long lSessionID, HWND	hNotifWnd, eCallType CallType, CString sCustomerDisplayName)
: CDHtmlDialogEx(COnline4CustomerDlg::IDD, COnline4CustomerDlg::IDH, pParent)
{
TRY_CATCH 
	
	m_lSessionID = lSessionID;
	m_sDestUrl = sNavigateUrl;
	m_hNotifWnd = hNotifWnd;
	m_eCallType = CallType;
	m_sCustomerDisplayName = sCustomerDisplayName;

	Create(IDD, pParent); // Create modeless dialog - trick for Tray icon based on CDialog not on CFrame

CATCH_LOG(_T("COnline4CustomerDlg ::COnline4CustomerDlg"))
}

COnline4CustomerDlg::~COnline4CustomerDlg()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("COnline4CustomerDlg::COnline4CustomerDlg"));

CATCH_LOG(_T("COnline4CustomerDlg::~COnline4CustomerDlg"))
}

void COnline4CustomerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

// CSupportMessengerDlg  message handlers
BOOL COnline4CustomerDlg::OnInitDialog()
{
TRY_CATCH 

	CDHtmlDialogEx::OnInitDialog();
	//	Set the icon for this dialog.  The framework does this automatically
	//	when the application's main window is not a dialog
	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);	// Set big icon
	//SetIcon(m_srtIcons.m_hMainApp, FALSE);	// Set small icon

	Navigate(m_sDestUrl);
	//
	//	Change text of the window caption STL-419
	//
	CString	sTitle = _T("Online for Customer");

	//  set window text
	SetWindowText(sTitle);

	//  Due request from Noam we enable F5 in  
	EnableF5(TRUE);

	// Due request from Noam there is option to close COnline4CustomerDlg using Ctrl+F4
	EnableCtrlF4(TRUE);

	// Workbench issue STL-616 and SupportCenter STL-682 
	EnableWorkbenchMode(TRUE);

	// Show Maximized window like F11 mode in previous version
	// ShowWindow(SW_SHOWMAXIMIZED);

CATCH_LOG(_T("CWorkbenchDlg::OnInitDialog"))	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COnline4CustomerDlg::OnPaint()
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
HCURSOR COnline4CustomerDlg::OnQueryDragIcon()
{
	return 0;
	//return static_cast<HCURSOR>(m_srtIcons.m_hMainApp);// TODO?
}

void COnline4CustomerDlg::PostNcDestroy() 
{
TRY_CATCH 
	
	Log.Add(_MESSAGE_,_T("COnline4CustomerDlg::PostNcDestroy is called before delete this"));
	delete this;
	Log.Add(_MESSAGE_,_T("COnline4CustomerDlg::PostNcDestroy is called after delete this"));

CATCH_LOG(_T("COnline4CustomerDlg::PostNcDestroy"))
}
//todo this mehotd is not called by js of CWorkbenchDlg
HRESULT COnline4CustomerDlg::OnPageLoaded(IHTMLElement *pElement)
{
TRY_CATCH 

	if(IsPageLoaded())
	{
		//ShowWindow(SW_SHOWMAXIMIZED);
	}

CATCH_LOG(_T("COnline4CustomerDlg::OnPageLoaded"))
	return 0;
}

//todo this mehotd is not called by js of CWorkbenchDlg
HRESULT COnline4CustomerDlg::OnClosePage(IHTMLElement *pElement)
{
TRY_CATCH 
	
	OnCancel();
	OnClose();
	
CATCH_LOG(_T("COnline4CustomerDlg::OnPageLoaded"))
	return 0;
}

//this function called to set workbench title
HRESULT COnline4CustomerDlg::OnSetTitle(IHTMLElement *pElement)
{
TRY_CATCH 
	
	VARIANT	tmpTitleText;
	CString	sTitleText;
	
	pElement->getAttribute( L"sTitleTxt", 0, &tmpTitleText);
	if(tmpTitleText.vt != VT_NULL)
	{
		sTitleText = (CString)tmpTitleText;
		Log.Add(_MESSAGE_,_T("COnline4CustomerDlg::OnSetTitle %s"), sTitleText);
		//  set window text
		SetWindowText(sTitleText);
	}
	
CATCH_LOG(_T("CWorkbenchDlg::OnSetTitle"))
	return 0;
}

HRESULT COnline4CustomerDlg::Online4CustomerPageSubmit(IHTMLElement *pElement)
{
TRY_CATCH 
	
	VARIANT	tmpText;
	
	pElement->getAttribute( L"sCustomerDisplayName", 0, &tmpText);
	if(tmpText.vt != VT_NULL)
	{
		m_sCustomerDisplayName = (CString)tmpText;
		Log.Add(_MESSAGE_,_T("COnline4CustomerDlg::Online4CustomerPageSubmit.sCustomerDisplayName: %s"), m_sCustomerDisplayName);
	}

	if(m_hNotifWnd != NULL)
		::PostMessage(m_hNotifWnd, WM_ONLINE4CUSTOMER_SUBMIT, 0, 0);
	else
		Log.Add(_ERROR_,_T("COnline4CustomerDlg::Online4CustomerPageSubmit. m_hNotifWnd is NULL"));		
	
CATCH_LOG(_T("COnline4CustomerDlg::Online4CustomerPageSubmit"))
	return 0;
}

//	todo OnDocumentComplete is called not once as expected
//	we added special CALLBACK_PageLoaded
//
void COnline4CustomerDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
TRY_CATCH 

	CDHtmlDialogEx::OnDocumentComplete(pDisp, szUrl);

	if(IsPageLoaded())
	{
		Log.Add((_T("Page is Loaded for URL: %s"), szUrl )); 	
		//ShowWindow(SW_SHOWMAXIMIZED);
	}

CATCH_LOG(_T("COnline4CustomerDlg::OnDocumentComplete"))
}

void COnline4CustomerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_CLOSE) {
		// We have to manually add this so that the dialog box closes when 
		// when the user clicks the close button (which appears in the top, right)
		// corner of the dialog box when it is floating).

		// we have to call JS interface method STL-587
		Log.Add((_T("COnline4CustomerDlg call INTERFACE_UpdateData with parameter: shuttle_close_workbench")));
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

void COnline4CustomerDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
TRY_CATCH 

	// Get the minimum size of the window assuming it has no client area.
	// This is the width/height of the window that must always be present
	CRect rcBorder(0, 0, 0, 0);
	AdjustWindowRectEx(&rcBorder, GetStyle(), FALSE, GetExStyle());

	lpMMI->ptMinTrackSize.x = MIN_UI_WIDTH; //todo max and min size
	lpMMI->ptMinTrackSize.y = MIN_UI_HEIGHT;
	
	lpMMI->ptMaxTrackSize.x = MIN_UI_HEIGHT;
	lpMMI->ptMaxTrackSize.y = MIN_UI_HEIGHT;

	return;

CATCH_LOG(_T("COnline4CustomerDlg::OnGetMinMaxInfo"))
}


void COnline4CustomerDlg::OnSize(UINT nType, int cx, int cy) {

	CDHtmlDialogEx::OnSize(nType, cx, cy);
}

void COnline4CustomerDlg::OnCancel()
{
TRY_CATCH 

	CDHtmlDialogEx::OnCancel();

CATCH_LOG(_T("COnline4CustomerDlg::OnCancel"))
}

void COnline4CustomerDlg::OnClose()
{
TRY_CATCH 

	if(m_hNotifWnd != NULL)
	{	
		::PostMessage(m_hNotifWnd, WM_ONLINE4CUSTOMER_CLOSE, 0, 0);
	}
	else
	{
		Log.Add((_T("COnline4CustomerDlg::OnClose(). m_hNotifWnd is NULL"))); 		
	}

	//close application properly
	//DestroyModeless();
	//PostQuitMessage(0); 
	
CATCH_LOG(_T("COnline4CustomerDlg::OnClose"))
}

HRESULT COnline4CustomerDlg::OnCbkEventsHandlerClickedEvent(IHTMLElement *pElement)
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

CATCH_LOG(_T("COnline4CustomerDlg::OnCbkEventsHandlerClickedEvent"))

	return S_OK;
}

BOOL COnline4CustomerDlg::CreateControlSite(COleControlContainer* pContainer, COleControlSite** ppSite, UINT, REFCLSID) 
{
	Log.Add(_MESSAGE_,_T("COnline4CustomerDlg::CreateControlSite"));
	CCustomControlSite *pBrowserSite = new CCustomControlSite(pContainer, this);
	if (!pBrowserSite)
		return FALSE;
	*ppSite = pBrowserSite;
	return TRUE;
}