#pragma once

#include "resource.h"
#include "DHtmlDialogEx.h"

///////////////////////////////////////////////////
using namespace std;
#include <map>

#include "Settings.h"

#define WM_RELOAD_SETTINGS			WM_USER + 500
#define WM_SETTINGS_DIALOG_CLOSE	WM_USER + 501

class CSettingsDlg;
typedef map<CString, HRESULT (CSettingsDlg::*)(IHTMLElement *pElement)> SettingsCallBacksEventMap;

// CSettingsDlg dialog
class CSettingsDlg : public CDHtmlDialogEx
{
// Construction
public:
	CSettingsDlg(CWnd*						pParent,
 			     HWND						hWndNotify,
				 const	tstring&			sSupporterId,		
				 DhtmlGuiLocation			m_eGUIlocation,
				 CString					m_sGUIlocationFilePath);	// constructor

	~CSettingsDlg();	//destructor

// Dialog Data
	enum { IDD = IDD_SETTINGS_DIALOG, IDH = IDR_HTML_SupportMessenger_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	HRESULT OnCbkEventsHandlerClickedEvent(IHTMLElement *pElement);

	HRESULT OnAlertClose( IHTMLElement *pElement);
	HRESULT OnSettingsOk(IHTMLElement *pElement);
	HRESULT OnSettingsClose(IHTMLElement *pElement);

	// enter and leave eventes required only for implementation of fadein -fadeout effects with popup window
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void PostNcDestroy();
	void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
	afx_msg HCURSOR OnQueryDragIcon();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
private:

	HWND						m_hWndNotify;
	SettingsCallBacksEventMap	m_mapEvents;
	DhtmlGuiLocation			m_eGUIlocation;
    CString						m_sGUIlocationFilePath;
	tstring						m_sSupporterId;
};