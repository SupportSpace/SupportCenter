// AlertDlg.cpp : implementation file
//
#include "stdafx.h"
#include "SettingsDlg.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"
#include "SupporterLocalData.h"
#include "SupportMessenger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CSupportMessengerApp theApp;

// CAlertDlg dialog

BEGIN_DHTML_EVENT_MAP(CSettingsDlg)
//main entry point for DHTML predefined callbacks
DHTML_EVENT_ONCLICK(_T("eCbkEventsHandler"), OnCbkEventsHandlerClickedEvent)
//ban the selecting action
DHTML_EVENT_TAG_ALL(DISPID_HTMLELEMENTEVENTS_ONSELECTSTART, OnHtmlSelectStart)
//handle all the dragging action
DHTML_EVENT_TAG_ALL(DISPID_HTMLELEMENTEVENTS_ONDRAGSTART, OnHtmlDragStart)
END_DHTML_EVENT_MAP()

BEGIN_MESSAGE_MAP(CSettingsDlg, CDHtmlDialogEx)
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CSettingsDlg::CSettingsDlg(
				 CWnd*						pParent,
 			     HWND						hWndNotify,
				 const	tstring&			sSupporterId,
				 DhtmlGuiLocation			eGUIlocation,
				 CString					sGUIlocationFilePath)
	:CDHtmlDialogEx(CSettingsDlg::IDD, CSettingsDlg::IDH, pParent)
{
TRY_CATCH
	
	m_mapEvents[_T("closeAlert")] = &CSettingsDlg::OnAlertClose;
	m_mapEvents[_T("SettingsOk")] = &CSettingsDlg::OnSettingsOk;
	m_mapEvents[_T("SettingsClose")] = &CSettingsDlg::OnSettingsClose;
	
	m_hWndNotify	= hWndNotify;

	m_sSupporterId = sSupporterId;

	m_eGUIlocation  = eGUIlocation;
    m_sGUIlocationFilePath = sGUIlocationFilePath;

	Create(IDD, pParent); // Create modeless dialog - trick for Tray icon based on CDialog not on CFrame

CATCH_LOG(_T("CSettingsDlg ::CSettingsDlg"))
}

CSettingsDlg::~CSettingsDlg()
{
TRY_CATCH
	
CATCH_LOG(_T("CSettingsDlg ::~CSettingsDlg"))
}

HRESULT CSettingsDlg::OnCbkEventsHandlerClickedEvent(IHTMLElement *pElement)
{
TRY_CATCH

	VARIANT AttributeValue1;
	pElement->getAttribute( L"sEvent", 0, &AttributeValue1);

	CString sEvent = (CString)AttributeValue1;
	if( m_mapEvents.find(sEvent) != m_mapEvents.end() )
	{
		 Log.Add(_MESSAGE_,_T("CSupportMessengerDlg::OnDocumentComplete '%s'"), sEvent  );
		(this->*m_mapEvents[sEvent])(pElement);
	}
	else
	{
		 Log.Add(_MESSAGE_,_T("CSettingsDlg::OnCbkEventsHandlerClickedEvent Warning - this eventID: '%s' not in the event map"), sEvent );
	}

CATCH_LOG(_T("CSettingsDlg ::OnCbkEventsHandlerClickedEvent"))

	return S_OK;
}

HRESULT CSettingsDlg::OnAlertClose(IHTMLElement *pElement )
{
TRY_CATCH
/*
	long		lCallUID;
	VARIANT		tmpCallUID;
	pElement->getAttribute( L"iCallUID", 0, &tmpCallUID);
	lCallUID	= tmpCallUID.lVal;
	Log.Add(_MESSAGE_, _T("CAlertDlg::OnAlertClose Attribute iCallUID %d"), lCallUID  );

	if( m_pNotifDIalog != NULL )
		m_pNotifDIalog->PostMessage( WM_ALERTDLG_CLOSE, 0, (LPARAM)lCallUID );
*/
CATCH_LOG(_T("CAlertDlg::OnAlertClose"))

	return S_OK;
}

HRESULT CSettingsDlg::OnSettingsClose(IHTMLElement *pElement)
{
TRY_CATCH

	if( m_hWndParent != NULL )
		::PostMessage(m_hWndNotify, WM_SETTINGS_DIALOG_CLOSE, 0, 0);

CATCH_LOG(_T("CSettingsDlg::OnSettingsClose"))

	return S_OK;
}

HRESULT CSettingsDlg::OnSettingsOk(IHTMLElement *pElement)
{
TRY_CATCH

	Log.Add(_MESSAGE_, _T("CCSettingsDlg::OnSettingsOk"));
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


	pElement->getAttribute(L"cOnIncomingCallsShowTrayMessage", 0, &sTempData);
	cOnIncomingCallsShowTrayMessage =  (-1)*(BOOL)sTempData.boolVal;

	pElement->getAttribute(L"cAutomaticallyRun", 0, &sTempData);
	cAutomaticallyRun = (-1)*(BOOL)sTempData.iVal;

	pElement->getAttribute(L"cOpenMainWindowOnWindowsStartUp", 0, &sTempData);
	cOpenMainWindowOnWindowsStartUp = (-1)*(BOOL)sTempData.iVal;

	pElement->getAttribute(L"cShowAway", 0, &sTempData);
	cShowAway = (-1)*(BOOL)sTempData.iVal;

	pElement->getAttribute(L"sShowAway", 0, &sTempData);
	sShowAway = (CString)sTempData;

	pElement->getAttribute(L"cHadndleCallsDisplayBusy", 0, &sTempData);
	cHadndleCallsDisplayBusy = (-1)*(BOOL)sTempData.iVal;

	pElement->getAttribute(L"sHadndleCallsDisplayBusy", 0, &sTempData);
	sHadndleCallsDisplayBusy = (CString)sTempData;
	
	pElement->getAttribute(L"cOnIncomingCallsShowTrayMessage", 0, &sTempData);
	cOnIncomingCallsShowTrayMessage = (-1)*(BOOL)sTempData.iVal;
	
	pElement->getAttribute(L"cOnIncomingCallsAnimateTrayIcon", 0, &sTempData);
	cOnIncomingCallsAnimateTrayIcon = (-1)*(BOOL)sTempData.iVal;
	
	pElement->getAttribute(L"cPromptOnItemsUpdate", 0, &sTempData);
	cPromptOnItemsOnLogout = (-1)*(BOOL)sTempData.iVal;
	
	pElement->getAttribute(L"cPromptAboutSnoozingItemsOnLogout", 0, &sTempData);
	cPromptAboutSnoozingItemsOnLogout = (-1)*(BOOL)sTempData.iVal;
	
	pElement->getAttribute(L"cPlaySoundUponIncomingCall", 0, &sTempData);
	cPlaySoundUponIncomingCall = (-1)*(BOOL)sTempData.iVal;
	
	pElement->getAttribute(L"cPlaySoundUponConnectingToCustomer", 0, &sTempData);
	cPlaySoundUponConnectingToCustomer = (-1)*(BOOL)sTempData.iVal;

	pElement->getAttribute(L"sDisplayItemsAtTime", 0, &sTempData);
	sDisplayItemsAtTime = (CString)sTempData;

	CSupporterLocalData  pSupporterLocalData(m_sSupporterId);

	//pSupporterLocalData->Add(); //if supporter not exists then add it
	pSupporterLocalData.getSettings()->Load();

	pSupporterLocalData.getSettings()->bAutomaticallyRun = cAutomaticallyRun;
	pSupporterLocalData.getSettings()->bOpenMainWindowOnMessangerStartUp = cOpenMainWindowOnWindowsStartUp;
	
	pSupporterLocalData.getSettings()->strShowAwayAfterBeingInActive.bActivated = cShowAway;
	pSupporterLocalData.getSettings()->strShowAwayAfterBeingInActive.iAfterCounter = _tstoi(sShowAway); 
	
	pSupporterLocalData.getSettings()->strHandleCallsAndThenDisplayBusy.bActivated = cHadndleCallsDisplayBusy;
	pSupporterLocalData.getSettings()->strHandleCallsAndThenDisplayBusy.iAfterCounter = _tstoi(sHadndleCallsDisplayBusy);

	pSupporterLocalData.getSettings()->bOnIncomingCallsShowAlert = cOnIncomingCallsShowTrayMessage;
	pSupporterLocalData.getSettings()->bOnIncomingCallsAnimateTrayIcon = cOnIncomingCallsAnimateTrayIcon;

	pSupporterLocalData.getSettings()->bPromptOnItemsOnLogout = cPromptOnItemsOnLogout; //TODO check name
	pSupporterLocalData.getSettings()->bPromptOnSnoozingItemsOnLogout = cPromptAboutSnoozingItemsOnLogout;
	pSupporterLocalData.getSettings()->bPlaySoundUponIncomingCall = cPlaySoundUponIncomingCall;
	
	pSupporterLocalData.getSettings()->bPlaySoundUponConnectingToCustomer = cPlaySoundUponConnectingToCustomer;
	pSupporterLocalData.getSettings()->iDisplayXItemsAtTime = _tstoi(sDisplayItemsAtTime);

	//pSupporterLocalData->getSettings()
	pSupporterLocalData.getSettings()->Save();

	if( m_hWndParent != NULL )
		::PostMessage(m_hWndNotify, WM_RELOAD_SETTINGS, 0, 0);

CATCH_LOG(_T("CSettingsDlg::OnSettingsOk"))

	return S_OK;
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialogEx::DoDataExchange(pDX);
}

// CAlertDlg message handlers

void CSettingsDlg::PostNcDestroy()
{
TRY_CATCH
	delete this;
CATCH_LOG(_T("CAlertDlg::PostNcDestroy"))
}

BOOL CSettingsDlg::OnInitDialog()
{
TRY_CATCH

	CDHtmlDialogEx::OnInitDialog();

	switch(m_eGUIlocation)
	{
	case GuiLocationResource:
		m_sDestUrl.FormatMessage(_T("res://%1!s!%2!s!.exe/%3!d!"), 
			theApp.m_sApplicationPath, theApp.m_pszExeName, IDR_HTML_SETTINGS);
		LoadFromResource(IDR_HTML_SETTINGS);// - another ooption
		break;
	case GuiLocationFile:
		m_sDestUrl.AppendFormat(_T("%s\\settings.html"), m_sGUIlocationFilePath);
		Navigate(m_sDestUrl );
		break;
	case GuiLocationURL:
		break;
	default:
		break;
	}

CATCH_LOG(_T("CSettingsDlg::OnInitDialog"))
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSettingsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
//TRY_CATCH

	switch( lParam )
	{
	    case WM_NCPAINT:	
			Log.Add(_MESSAGE_, _T("CSettingsDlg::OnSysCommand WM_NCPAINT") );
			break;
		case WM_ERASEBKGND: 
			Log.Add(_MESSAGE_, _T("CSettingsDlg::OnSysCommand WM_ERASEBKGND") );
			break;
		default:
			break;
	}

	CDHtmlDialogEx::OnSysCommand(nID, lParam);

//CATCH_LOG(_T("CAlertDlg::OnSysCommand"))
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSettingsDlg::OnPaint()
{
	if (IsIconic())
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
		//dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDHtmlDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSettingsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSettingsDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
TRY_CATCH

	CDHtmlDialogEx::OnDocumentComplete(pDisp, szUrl);

	if(IsPageLoaded() == TRUE && m_sDestUrl.Compare(szUrl)==0)
	{
		//m_cHTMLInterface.INTERFACE_UpdateData( _T("AddCalls"), m_sCalls); //TODO
		CRect	m_screen_rect;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &m_screen_rect, 0);
		
		int	m_nWidth =  240;
		int	m_nHeight = 695; //75

		int x = m_screen_rect.right -  m_nWidth - m_pTransWindow->VERTICAL_BORDER_WIDTH;
		int y = m_screen_rect.bottom - (m_nHeight + 25 );

		int cx = m_nWidth;
		int cy = m_nHeight;

		SetWindowPos( &CWnd::wndTopMost, 
				x,
				y, 
				cx, 
				cy, //For Dialog Border needed TODO
				SWP_NOZORDER | SWP_SHOWWINDOW);

		 RECT rc;

		 rc.left = x;
		 rc.top = y;
		 rc.right = x + cx;
		 rc.bottom= y + cy;
	}

CATCH_LOG(_T("CSettingsDlg::OnDocumentComplete"))
}
