// SupportMessengerDlg.h : header file
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CWorkbenchDlg.h
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CWorkbenchDlg :	WorkbenchDlg window of SupportMessenger
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
#include "HTMLInterface.h"
#include "Call.h"
#include "resource.h"		// main symbols
#include "AppBar.H"

using namespace std;
#include <map>

#define WM_ONLINE4CUSTOMER_CLOSE	20507
#define WM_ONLINE4CUSTOMER_SUBMIT   20508

// CWorkbenchDlg  dialog
class COnline4CustomerDlg : public CDHtmlDialogEx
{
// Construction
public:
	COnline4CustomerDlg (CWnd* pParent, CString sNavigateUrl, long lSessionID, HWND	hNotifWnd, eCallType CallType, CString sCustomerDisplayName);	// standard constructor
	~COnline4CustomerDlg();	// standard constructor

// Dialog Data
	enum { IDD = IDD_ONLINE4CUSTOMER_DIALOG, IDH = 0 }; //

public:
	long GetSessionID(){ return m_lSessionID;}
	BOOL CreateControlSite(COleControlContainer* pContainer, COleControlSite** ppSite, UINT /*nID*/, REFCLSID /*clsid*/);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//void OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu);
	LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);

protected:  // Overridable functions
	void OnCancel();

//	afx_msg void OnTimer(UINT nIDEvent);
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
//	afx_msg void OnDockRight();

//	Menu calls end
public:
	void OnOK(){};//do nothing just handle

//	Callbacks called by JS as response to user inputs and events begin
	HRESULT OnCbkEventsHandlerClickedEvent(IHTMLElement *pElement);

	HRESULT OnPageLoaded(IHTMLElement *pElement);
	HRESULT OnClosePage(IHTMLElement *pElement);
	HRESULT OnSetTitle(IHTMLElement *pElement);
	HRESULT Online4CustomerPageSubmit(IHTMLElement *pElement);
	CString GetCustomerDisplayName(){return m_sCustomerDisplayName;};
	
private:

private:
	CString		m_sNavigateUrl;				//
	long		m_lSessionID;
	//CWnd*		m_pNotifWnd;
	eCallType	m_eCallType;
	CString		m_sCustomerDisplayName;
	HWND		m_hNotifWnd;
};