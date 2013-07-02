// CDHTMLDialogMsg.h : header file
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
// Date    : 10/29/2006 05:21:10 PM
// Comments: First Issue
//===========================================================================

#pragma once

#include "resource.h"
#include "DHtmlDialogEx.h"

#define TEAMVIEWER_NEVER_REMIND_ME_RET_CODE			25
#define TEAMVIEWER_DOWNLOAD_URL				_T("http://www.teamviewer.com/download/TeamViewer_Setup.exe")

// CDHTMLDialogDlg dialog
class CDHTMLDialogMsg : public CDHtmlDialogEx
{
// Construction
public:
	CDHTMLDialogMsg(UINT nRes, CWnd* pParent = NULL);
	BOOL	m_TeamViewerNeverRemindMe;

// Dialog Data
	enum { IDD = IDD_DHTMLDIALOG_MSG, IDH = IDR_HTML_IE6NOTIF_MSG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	HRESULT OnIE6NOTIFButtonOK(IHTMLElement *pElement);
	
	HRESULT OnTeamViewerDownloadNow(IHTMLElement* /*pElement*/);
	HRESULT OnTeamViewerNeverRemindMe(IHTMLElement* /*pElement*/);
	HRESULT OnTeamViewerRemindMeLater(IHTMLElement* /*pElement*/);


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()

	UINT m_nRes;//resourse id to be spceified in LoadFromResource call
};
