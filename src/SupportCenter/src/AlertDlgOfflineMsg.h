// AlertDlgOfflineMsg.h : header file
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CAlertDlgOfflineMsg
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CAlertDlgUpdate :	class to show new update dialog
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

#include "AlertDlg.h"
#include "Call.h"
#include <windows.h>

//	integration with maind dialog required defines 
#define WM_ALERTDLG_OFFLINE_MSG_PICKUP		20208
#define WM_ALERTDLG_OFFLINE_MSG_CLOSE		20209

class CAlertDlgOfflineMsg : public CAlertDlg
{
public:
	CAlertDlgOfflineMsg(
		CWnd*		pNotifDIalog,
		CTransparentWindow*	pTransWindow,
		HWND		hWndParent,
		CWnd*		pParent,
		DhtmlGuiLocation	eGUIlocation,
		CString		sGUIlocationFilePath,
		long		lFreePosition,
		long		lCallUID,
		CString		sCalls,
		eCallType	callType,
		long		lMid
		);
	
	~CAlertDlgOfflineMsg(void);
public:
	HRESULT OnAlertPickUp(IHTMLElement *pElement);
	HRESULT OnAlertClose(IHTMLElement *pElement);

	HRESULT UpdateAlertText(CString sNewAlertText);

protected:	
	virtual LRESULT OnAlertFadeCompleted(WPARAM wParam, LPARAM lParam ); //called by transparent window
	virtual HRESULT OnCbkEventsHandlerClickedEvent(IHTMLElement *pElement);
	virtual HRESULT CalcluateAlertPosition(RECT* pRc,WINDOWPOS* pPosc);
	virtual HRESULT OnPageLoaded();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnTimer(UINT nIDEvent);

private:
	long		m_lFreePosition;
	long		m_lCallUID;
	eCallType	m_eCallType;
	CString		m_sCalls;
	long		m_lMid;
};