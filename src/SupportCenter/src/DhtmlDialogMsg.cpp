// CDHTMLDialogMsg.cpp : implementation file 
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CDHTMLDialogMsg
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CDHTMLDialogMsg :	simple messgae with html. based on CDHtmlDialogEx
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 06/24/2008 05:21:10 PM
// Comments: First Issue
//===========================================================================

#include "stdafx.h"
#include "DhtmlDialogMsg.h"
#include "UrlOpener.h"
#include "LocalDataDef.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"
#include "SupportMessenger.h"

extern CSupportMessengerApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CDHTMLDialogDlg dialog

BEGIN_DHTML_EVENT_MAP(CDHTMLDialogMsg)
	DHTML_EVENT_ONCLICK(_T("IE6NOTIFButtonOK"), OnIE6NOTIFButtonOK)

	DHTML_EVENT_ONCLICK(_T("TeamViewerDownloadNow"), OnTeamViewerDownloadNow)
	DHTML_EVENT_ONCLICK(_T("TeamViewerNeverRemindMe"), OnTeamViewerNeverRemindMe)
	DHTML_EVENT_ONCLICK(_T("TeamViewerRemindMeLater"), OnTeamViewerRemindMeLater)
END_DHTML_EVENT_MAP()

CDHTMLDialogMsg::CDHTMLDialogMsg(UINT nRes, CWnd* pParent /*=NULL*/)	// standard constructor
	: CDHtmlDialogEx(CDHTMLDialogMsg::IDD, CDHTMLDialogMsg::IDH, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nRes = nRes;
	m_TeamViewerNeverRemindMe = FALSE;
}

void CDHTMLDialogMsg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDHTMLDialogMsg, CDHtmlDialog)
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CDHTMLDialogDlg message handlers

BOOL CDHTMLDialogMsg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	LoadFromResource(m_nRes);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDHTMLDialogMsg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDHtmlDialog::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDHTMLDialogMsg::OnPaint() 
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
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDHtmlDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDHTMLDialogMsg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
HRESULT CDHTMLDialogMsg::OnIE6NOTIFButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CDHTMLDialogMsg::OnTeamViewerDownloadNow(IHTMLElement* /*pElement*/)
{
TRY_CATCH
	
	Log.Add(_MESSAGE_,_T("CDHTMLDialogMsg::OnTeamViewerDownloadNow selected"));	
	//opend url to teamviewr downloadble package
	CUrlOpener cUrlOpener;

	if(theApp.m_cSettings.m_sTeamViewerDownload == "")
		cUrlOpener.Open(TEAMVIEWER_DOWNLOAD_URL);
	else
		cUrlOpener.Open(theApp.m_cSettings.m_sTeamViewerDownload);

	OnOK();
CATCH_LOG(_T("CDHTMLDialogMsg::OnTeamViewerDownloadNow"))
	return S_OK;
}

HRESULT CDHTMLDialogMsg::OnTeamViewerNeverRemindMe(IHTMLElement* /*pElement*/)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CDHTMLDialogMsg::OnTeamViewerNeverRemindMe selected"));	
	//update registry to remember this selection
	m_TeamViewerNeverRemindMe = TRUE;
	EndDialog(TEAMVIEWER_NEVER_REMIND_ME_RET_CODE);
CATCH_LOG(_T("CDHTMLDialogMsg::OnTeamViewerNeverRemindMe"))
	return S_OK;
}

HRESULT CDHTMLDialogMsg::OnTeamViewerRemindMeLater(IHTMLElement* /*pElement*/)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CDHTMLDialogMsg::OnTeamViewerRemindMeLater selected"));	
	//do nothing just close
	OnOK();
CATCH_LOG(_T("CDHTMLDialogMsg::OnTeamViewerRemindMeLater"))
	return S_OK;
}