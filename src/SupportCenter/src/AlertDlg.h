// AlertDlg.h : header file
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CAlertDlg
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CAlertDlg :	Alert dialog
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

#include "resource.h"
#include "DHtmlDialogEx.h"

///////////////////////////////////////////////////
#include <map>
using namespace std;

#include "Settings.h"
//
//	Default size of Alerts
//
#define ALERT_DIALOG_WIDTH		 213//219
#define ALERT_DIALOG_HEIGHT		 145//130
#define ALERT_DIALOG_OFFSET		 25		//offset between alerts 

#define WM_ALERTDLG_CLOSE		 20007	

///////////////////////////////////////////////////////////////////////////
// fadein/fadeout features without 1/2 transparent border required defines
//
#define BALLOON_NEW_FIDEIN_TIMER				0x30
#define BALLOON_NEW_FIDEIN_TIMEOUT				1	//ms
#define BALLOON_NEW_FADEIN_STEP					4

#define BALLOON_MOUSEOVER_FADEIN_TIMER			0x40
#define BALLOON_MOUSEOVER_FADEIN_TIMEOUT		6

#define BALLOON_MOUSEOUT_FADEOUT_TIMER			0x50
#define BALLOON_MOUSEOUT_FADEOUT_TIMEOUT		6	

#define BALLOON_CLOSE_FADEOUT_TIMER				0x70
#define BALLOON_CLOSE_FADEOUT_TIMEOUT			1	//ms

//----------------------------------------------------------------------------
#define	BALLOON_CLOSEDOWN_TIMER					0x10
#define BALLOON_CLOSEDOWN_TIMEOUT				45*1000

#define BALLOON_MOUSEOUT_WAIT_FADEOUT_TIMER		0x60
#define BALLOON_MOUSEOUT_WAIT_FADEOUT_TIMEOUT	35*1000

#define	BALLON_START_TRNSPARENCY				90		//100% invisible
#define BALLON_NEW_MAX_TRANSPARENCY				0		//0%  invisible
#define BALLON_MOUSEOVER_MAX_TRANSPARENCY		0		//0%   invisible
#define BALLON_MOUSEOUT_MAX_TRANSPARENCY		50		//50%  invisible

// CAlertDlg dialog
class CAlertDlg : public CDHtmlDialogEx
{
// Construction
public:
	CAlertDlg(CWnd*						pNotifDIalog,
			  CTransparentWindow*		pTransWindow,
			  HWND						hWndParent,
			  CWnd*						pParent,
			  DhtmlGuiLocation			eGUIlocation,
			  CString					sGUIlocationFilePath,
			  UINT						dwResId,
			  CString					sNavigateUrl = _T(""));	// constructor

	virtual ~CAlertDlg();	//destructor

// Dialog Data
	enum { IDD = IDD_ALERT_DIALOG, IDH = 0 };//IDH is 0 - this nHtmlResID specified in call

	CTransparentWindow*	getParentWindow(){ return m_pTransWindow; };
	// enter and leave eventes required only for implementation of fadein -fadeout effects with popup window
	HRESULT OnAlertEnter( IHTMLElement* pElement );
	HRESULT OnAlertLeave( IHTMLElement* pElement );

	void	FadeOutAlert();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	virtual HRESULT OnCbkEventsHandlerClickedEvent(IHTMLElement *pElement) = 0;
	virtual LRESULT OnAlertFadeCompleted(WPARAM wParam, LPARAM lParam) = 0; //called by transparent window
	virtual HRESULT CalcluateAlertPosition(RECT* pRc,WINDOWPOS* pPosc) = 0;
	virtual HRESULT OnPageLoaded() = 0;

	HRESULT OnHtmlSelectStart(IHTMLElement* pElement);

	HRESULT SetForegroundWindowEx();

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnPaint();
	afx_msg void PostNcDestroy();
	void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
//private:
protected:
	CWnd*				m_pNotifDIalog;
	DhtmlGuiLocation	m_eGUIlocation;
    CString				m_sGUIlocationFilePath;
	UINT				m_dwResId;					
    CString				m_sNavigateUrl;				//optional parameter
	int					m_iCurrentTransparency;
	HWND				m_hwndFG;
};