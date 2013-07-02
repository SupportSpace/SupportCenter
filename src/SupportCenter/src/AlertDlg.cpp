// AlertDlg.cpp : implementation file
//
#include "stdafx.h"
#include "AlertDlg.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"
#include "Call.h"
#include "SupportMessenger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CSupportMessengerApp theApp;

///////////////////////////////////////////////////////////////////////////////////////////////////
// CAlertDlg dialog
BEGIN_DHTML_EVENT_MAP(CAlertDlg)
//main entry point for DHTML predefined callbacks
DHTML_EVENT_ONCLICK(_T("eCbkEventsHandler"), OnCbkEventsHandlerClickedEvent)
//ban the selecting action
DHTML_EVENT_TAG_ALL(DISPID_HTMLELEMENTEVENTS_ONSELECTSTART, OnHtmlSelectStart)
//handle all the dragging action
//DHTML_EVENT_TAG_ALL(DISPID_HTMLELEMENTEVENTS_ONDRAGSTART, OnHtmlDragStart)
END_DHTML_EVENT_MAP()

BEGIN_MESSAGE_MAP(CAlertDlg, CDHtmlDialogEx)
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(WM_FADE_COMPLETED, OnAlertFadeCompleted)
	ON_WM_TIMER()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CAlertDlg::CAlertDlg(
 		  CWnd*		pNotifDIalog,
		  CTransparentWindow*	pTransWindow,
		  HWND		hWndParent,
		  CWnd*		pParent,
		  DhtmlGuiLocation	eGUIlocation,
		  CString	sGUIlocationFilePath,
		  UINT		dwResId,
		  CString	sNavigateUrl)	// standard constructor
	: CDHtmlDialogEx(CAlertDlg::IDD, dwResId /*CAlertDlg::IDH*/, pParent)
{
TRY_CATCH

	m_pNotifDIalog = pNotifDIalog;		//alert notifications to main dialog
	m_hWndParent	= hWndParent;		
	m_pTransWindow  = pTransWindow;

	m_eGUIlocation  = eGUIlocation;
    m_sGUIlocationFilePath = sGUIlocationFilePath;

	m_dwResId = dwResId;
	m_sNavigateUrl = sNavigateUrl;

	m_hwndFG = ::GetForegroundWindow(); //STL-679

	Create(CAlertDlg::IDD, pParent); // Create modeless dialog - trick for Tray icon based on CDialog not on CFrame

CATCH_LOG(_T("CAlertDlg ::CAlertDlg"))
}



CAlertDlg::~CAlertDlg()
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CAlertDlg::~CAlertDlg()"));
	if( m_pTransWindow!= NULL )
		delete m_pTransWindow;

CATCH_LOG(_T("CAlertDlg ::~CAlertDlg"))
}

void CAlertDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialogEx::DoDataExchange(pDX);
}

// CAlertDlg message handlers

void CAlertDlg::PostNcDestroy()
{
TRY_CATCH
	
	delete this;
CATCH_LOG(_T("CAlertDlg::PostNcDestroy"))
}

void CAlertDlg::FadeOutAlert()
{
	// start fadeout timer
	// from  m_iCurrentTransparency 0% to 100% transparent 
	// wait till fade out completed and then return		
	KillTimer(BALLOON_MOUSEOUT_WAIT_FADEOUT_TIMER);
	KillTimer(BALLOON_CLOSEDOWN_TIMER);
	KillTimer(BALLOON_MOUSEOUT_FADEOUT_TIMER);
	KillTimer(BALLOON_NEW_FIDEIN_TIMER);
	KillTimer(BALLOON_MOUSEOVER_FADEIN_TIMER);//?
	
	SetTimer(BALLOON_CLOSE_FADEOUT_TIMER, BALLOON_CLOSE_FADEOUT_TIMEOUT, NULL);
}

BOOL CAlertDlg::OnInitDialog()
{
TRY_CATCH
	
	CDHtmlDialogEx::OnInitDialog();

	switch(m_eGUIlocation)
	{
	case GuiLocationResource:
		m_sDestUrl.FormatMessage(_T("res://%1!s!%2!s!.exe/%3!d!"), 
			theApp.m_sApplicationPath, theApp.m_pszExeName, m_dwResId);
		LoadFromResource(m_dwResId);
		break;
	case GuiLocationFile:
		m_sDestUrl.AppendFormat(m_sNavigateUrl, m_sGUIlocationFilePath);
		Navigate(m_sDestUrl);
		break;
	case GuiLocationURL:
		//m_sDestUrl = _T("http://www.google.co.il/"); - working
		//Navigate(m_sDestUrl);
		break;
	default:
		break;
	}

	// todo for border window 
	if(m_pTransWindow!=NULL)
	{
		m_pTransWindow->SetChildWindow(this->m_hWnd);
		::SendMessage(this->m_hWnd,(UINT) WM_CHANGEUISTATE,UIS_CLEAR | WS_POPUP, 0);  
		::SendMessage(this->m_hWnd,(UINT) WM_CHANGEUISTATE,UIS_SET | WS_CHILD, 0);  
	}
	
CATCH_LOG(_T("CAlertDlg::OnInitDialog"))
	return FALSE;  // STL-679 return TRUE  unless you set the focus to a control
}

void CAlertDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
//TRY_CATCH

	switch( lParam )
	{
	    case WM_NCPAINT:	
			//Log.Add(_MESSAGE_, _T("CAlertDlg::OnSysCommand WM_NCPAINT") );
			return;
		case WM_ERASEBKGND: 
			//Log.Add(_MESSAGE_, _T("CAlertDlg::OnSysCommand WM_ERASEBKGND") );
			return;
		default:
			//Log.Add(_MESSAGE_, _T("CAlertDlg::OnSysCommand. nID:%d, lParam%d"),nID, (UINT)lParam );
			break;
	}

	CDHtmlDialog::OnSysCommand(nID, lParam);

//CATCH_LOG(_T("CAlertDlg::OnSysCommand"))
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAlertDlg::OnPaint()
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
HCURSOR CAlertDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CAlertDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
TRY_CATCH

	CDHtmlDialogEx::OnDocumentComplete(pDisp, szUrl);

	if(IsPageLoaded() == TRUE)
	{	
		//
		//	We give an option for the derived dialog to perfom some actions like update GUI here
		//
		OnPageLoaded();
		//
		//
		//	We need to calculate the position of the Next Alert 	
		//
		
		RECT rc;
		WINDOWPOS posc;
		CalcluateAlertPosition(&rc, &posc);
		//
		//	We need this stuff for parent transparent window
		//
		if(m_hWndParent)
		{
			::SendMessage(m_hWndParent,WM_WINDOWPOSCHANGING,NULL,(LPARAM)&posc);
			CTransparentWindow::OnUserMove(this->m_hWndParent ,&rc);
		}
		//
		//
		//if( m_hWndParent )
		//	::SendMessage(m_hWndParent,WM_USER_BORDER_MOVE,NULL,(LPARAM)&rc);
		if(m_hWndParent)
			::ShowWindow(this->m_hWndParent,SW_SHOWNORMAL);

		//SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER );
	    SetWindowPos(NULL, posc.x, posc.y, posc.cx, posc.cy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME);
		//UpdateWindow();
		//

		//
		//  FADEIN/FAEOUT effect without 1/2-transparent border	
		//	
		m_layeredWnd.AddLayeredStyle(this->m_hWnd);			// Prepeare window to make it transparent
		m_iCurrentTransparency = BALLON_START_TRNSPARENCY;
		MakeWindowTransparent(m_iCurrentTransparency);
		SetTimer(BALLOON_NEW_FIDEIN_TIMER, BALLOON_NEW_FIDEIN_TIMEOUT, NULL);
 		SetTimer(BALLOON_CLOSEDOWN_TIMER, BALLOON_CLOSEDOWN_TIMEOUT, NULL);
		SetTimer(BALLOON_MOUSEOUT_WAIT_FADEOUT_TIMER, BALLOON_MOUSEOUT_WAIT_FADEOUT_TIMEOUT, NULL);

		// SetForegroundWindowEx may be called to fix some issue STL-679 
		SetForegroundWindowEx();

		ShowWindow(SW_NORMAL);
	}

CATCH_LOG(_T("CAlertDlg::OnDocumentComplete"))
}

HRESULT CAlertDlg::OnAlertEnter( IHTMLElement *pElement )
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CAlertDlg::OnAlertEnter. kill all timers and start mouse over timer"));

	KillTimer(BALLOON_MOUSEOUT_WAIT_FADEOUT_TIMER);
	KillTimer(BALLOON_CLOSEDOWN_TIMER);
	KillTimer(BALLOON_MOUSEOUT_FADEOUT_TIMER);
	KillTimer(BALLOON_NEW_FIDEIN_TIMER);
	KillTimer(BALLOON_MOUSEOVER_FADEIN_TIMER);//?
	SetTimer(BALLOON_MOUSEOVER_FADEIN_TIMER, BALLOON_MOUSEOVER_FADEIN_TIMEOUT, NULL);

CATCH_LOG(_T("CAlertDlg::OnAlertEnter"))
	return S_OK;
}

HRESULT CAlertDlg::OnAlertLeave( IHTMLElement *pElement )
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CAlertDlg::OnAlertLeave"));

	Log.Add(_MESSAGE_,_T("After 5 seconds will get BALLOON_MOUSEOUT_WAIT_FADEOUT_TIMER"));
	SetTimer( BALLOON_MOUSEOUT_WAIT_FADEOUT_TIMER, BALLOON_MOUSEOUT_WAIT_FADEOUT_TIMEOUT, NULL );

	Log.Add(_MESSAGE_,_T("After 10 seconds will get BALLOON_CLOSEDOWN_TIMER"));
	SetTimer( BALLOON_CLOSEDOWN_TIMER, BALLOON_CLOSEDOWN_TIMEOUT, NULL);

CATCH_LOG(_T("CAlertDlg::OnAlertLeave"))
	return S_OK;
}

HRESULT CAlertDlg::OnHtmlSelectStart(IHTMLElement* pElement)
{
TRY_CATCH
	//	prevent drugging and selecting Sprint3 - Alert dialog not moveble, not resizeble
CATCH_LOG(_T("CAlertDlg::OnHtmlSelectStart"))
	return S_FALSE;
}

//	OnTimer is responsoble for fadein/fadeout effects
// 
void CAlertDlg::OnTimer(UINT nIDEvent) 
{
TRY_CATCH

	CString		strTmp;

	switch( nIDEvent )
	{
	case BALLOON_CLOSEDOWN_TIMER:
		Log.Add(_MESSAGE_,_T("BALLOON_CLOSEDOWN_TIMER"));
		KillTimer(BALLOON_CLOSEDOWN_TIMER);
		//DestroyWindow();//todo anatoly - close also parent window
	
		//if( m_pNotifDIalog != NULL )
		//	m_pNotifDIalog->PostMessage( WM_ALERTDLG_CLOSE, 0, (LPARAM)m_lCallUID ); TODO anatoly 
		return;

	case BALLOON_MOUSEOUT_WAIT_FADEOUT_TIMER: //	After 5 seconds will start FADEOUT
		Log.Add(_MESSAGE_,_T("BALLOON_MOUSEOUT_WAIT_FADEOUT_TIMER"));
		KillTimer( BALLOON_MOUSEOUT_WAIT_FADEOUT_TIMER );
		SetTimer( BALLOON_MOUSEOUT_FADEOUT_TIMER, BALLOON_MOUSEOUT_FADEOUT_TIMEOUT, NULL );
		break;

	case BALLOON_NEW_FIDEIN_TIMER:			//	from 100% to 50% transparency
		if( m_iCurrentTransparency >= BALLON_NEW_MAX_TRANSPARENCY )	 
		{
			strTmp.FormatMessage(_T("BALLOON_FIDEIN_TIMER MakeWindowTransparent: %1!d! "), m_iCurrentTransparency );
			// todo verbosity 
			Log.Add(_CALL_, strTmp);
			MakeWindowTransparent( m_iCurrentTransparency );
			KillTimer(BALLOON_NEW_FIDEIN_TIMER);
			SetTimer(BALLOON_NEW_FIDEIN_TIMER, BALLOON_NEW_FIDEIN_TIMEOUT, NULL);
			//m_iCurrentTransparency--;
			if(m_iCurrentTransparency > 0)
				m_iCurrentTransparency = m_iCurrentTransparency - BALLOON_NEW_FADEIN_STEP;
			else
				m_iCurrentTransparency = -1;
		}
		else
		{
			strTmp.FormatMessage(_T("BALLOON_FIDEIN_TIMER NO MakeWindowTransparent %1!d! "), m_iCurrentTransparency );
			// todo verbosity 
			Log.Add(_CALL_, strTmp);
			KillTimer(BALLOON_NEW_FIDEIN_TIMER);
		}
		break;

	case BALLOON_MOUSEOVER_FADEIN_TIMER:	// from m_iCurrentTransparency 50% to 0% transparent
		if(  m_iCurrentTransparency > BALLON_MOUSEOVER_MAX_TRANSPARENCY )			
		{
			strTmp.FormatMessage(_T("BALLOON_MOUSEOVER_FADEIN_TIMER MakeWindowTransparent: %1!d! "), m_iCurrentTransparency );
			// todo verbosity 
			Log.Add(_CALL_, strTmp);
			m_iCurrentTransparency--;
			MakeWindowTransparent( m_iCurrentTransparency );		 
			KillTimer( BALLOON_MOUSEOVER_FADEIN_TIMER );
			SetTimer( BALLOON_MOUSEOVER_FADEIN_TIMER,  BALLOON_MOUSEOVER_FADEIN_TIMEOUT, NULL );
		}
		else
		{
			strTmp.FormatMessage(_T("BALLOON_MOUSEOVER_FADEIN_TIMER NO MakeWindowTransparent %1!d! "), m_iCurrentTransparency );
			// todo verbosity 
			//Log.Add(_MESSAGE_, strTmp);
			KillTimer( BALLOON_MOUSEOVER_FADEIN_TIMER );
		}
		break;

	case BALLOON_MOUSEOUT_FADEOUT_TIMER:	// from  m_iCurrentTransparency 0% to 50% transparent 
		if(m_iCurrentTransparency < BALLON_MOUSEOUT_MAX_TRANSPARENCY)
		{
			strTmp.FormatMessage(_T("BALLOON_MOUSEOUT_FADEOUT_TIMER MakeWindowTransparent %1!d! "), m_iCurrentTransparency );
			// todo verbosity 
			Log.Add(_CALL_, strTmp);
			MakeWindowTransparent(m_iCurrentTransparency);
			m_iCurrentTransparency++;
			KillTimer(BALLOON_MOUSEOUT_FADEOUT_TIMER);
			SetTimer(BALLOON_MOUSEOUT_FADEOUT_TIMER, BALLOON_MOUSEOUT_FADEOUT_TIMEOUT, NULL);
		}
		else
		{
			strTmp.FormatMessage(_T("BALLOON_MOUSEOUT_FADEOUT_TIMER NOT  MakeWindowTransparent %1!d! "), m_iCurrentTransparency );
			// todo verbosity 
			Log.Add(_CALL_, strTmp);
			KillTimer( BALLOON_MOUSEOUT_FADEOUT_TIMER );
		}
		break;

	case BALLOON_CLOSE_FADEOUT_TIMER:
		if(m_iCurrentTransparency < BALLON_START_TRNSPARENCY)
		{
			MakeWindowTransparent(m_iCurrentTransparency);
			m_iCurrentTransparency++;
			KillTimer(BALLOON_CLOSE_FADEOUT_TIMER);
			SetTimer(BALLOON_CLOSE_FADEOUT_TIMER, BALLOON_CLOSE_FADEOUT_TIMEOUT, NULL);
		}
		else
		{
			KillTimer(BALLOON_CLOSE_FADEOUT_TIMER);
			// notify parent window that close_fadeout completed
		}

	default:
		break;
	}
	
	CWnd::OnTimer(nIDEvent);

CATCH_LOG(_T("CAlertDlg::OnTimer"))
}

HRESULT CAlertDlg::SetForegroundWindowEx()
{
TRY_CATCH

	DWORD dwThreadId=GetCurrentThreadId();  

	if(m_hwndFG)
	{
	  TCHAR szTitle[1024] = { 0 };
	  ::GetWindowText(m_hwndFG, szTitle, 1024);
	  Log.Add(_MESSAGE_,_T("ForegroundWindow title before alert was: %s"), szTitle);

	  DWORD dwFGThreadId=GetWindowThreadProcessId(m_hwndFG,0); 
	  if(AttachThreadInput(dwThreadId,dwFGThreadId,TRUE))
	  { 
		 ::SetForegroundWindow(m_hwndFG); 
		 AttachThreadInput(dwThreadId,dwFGThreadId,FALSE); 
		 Log.Add(_MESSAGE_,_T("SetForegroundWindow passed ok"));
	  }
	  else
	  {
		 Log.Add(_WARNING_,_T("AttachThreadInput failed"));
		 return S_FALSE;
	  }
	}
	else
	{
		Log.Add(_WARNING_,_T("SetForegroundWindow not done. hwndFG is null"));
		return S_FALSE;
	}

CATCH_LOG(_T("CAlertDlg::SetForegroundWindowEx"))
	return S_OK;
}	

void CAlertDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	Log.Add(_WARNING_,_T("OnGetMinMaxInfo"));
}