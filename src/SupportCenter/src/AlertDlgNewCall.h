// AlertDlgNewCall.h : header file
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								AlertDlgNewCall
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CAlertDlgNewCall :	class to show new alert dialog
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
#include <windows.h>
#include "Call.h"

//	integration with maind dialog required defines 
#define WM_ALERTDLG_NEW_CALL_EXPAND		20005
#define WM_ALERTDLG_NEW_CALL_PICKUP		20006
#define WM_ALERTDLG_NEW_CALL_CLOSE		20007

#define ALERT_NEW_DIALOG_WIDTH				213//219
#define ALERT_NEW_DIALOG_HEIGHT				150//130
#define ALERT_NEW_DIALOG_OFFSET				25//offset between alerts 

class CAlertDlgNewCall : public CAlertDlg
{
public:
	CAlertDlgNewCall(
		CWnd*		pNotifDIalog,
		CTransparentWindow*	pTransWindow,
		HWND		hWndParent,
		CWnd*		pParent,
		DhtmlGuiLocation	eGUIlocation,
		CString		sGUIlocationFilePath,
		long		lFreePosition,
		long		lCallUID,
		CString     sCalls,
		eCallType	callType);
	
	virtual ~CAlertDlgNewCall(void);
public:
	HRESULT OnAlertPickUp(IHTMLElement *pElement);
	HRESULT OnAlertExpand(IHTMLElement *pElement);
	HRESULT OnAlertClose(IHTMLElement *pElement);
	HRESULT OnCallBackPageLoaded(IHTMLElement *pElement);

	eCallType GetCallType(){return m_eCallType;};
	long	  GetCallId(){return m_lCallUID;};	

	static BOOL	IsFreePositionAvailble(int iFreePosition);

protected:	
	virtual LRESULT OnAlertFadeCompleted(WPARAM wParam, LPARAM lParam ); //called by transparent window
	virtual HRESULT OnCbkEventsHandlerClickedEvent(IHTMLElement *pElement);
	virtual HRESULT CalcluateAlertPosition(RECT* pRc,WINDOWPOS* pPosc);
	virtual HRESULT OnPageLoaded();

	afx_msg void OnTimer(UINT nIDEvent);
	DECLARE_MESSAGE_MAP()

private:
	long		m_lFreePosition;
	long		m_lCallUID;
	CString		m_sCalls;
	eCallType	m_eCallType;
};