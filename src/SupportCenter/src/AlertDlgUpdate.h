// CAlertDlgUpdate.h : header file
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CAlertDlgUpdate
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
#include <windows.h>

//	integration with maind dialog required defines 
#define WM_ALERTDLG_UPDATE_PICKUP		20106
#define WM_ALERTDLG_UPDATE_CLOSE		20107

class CAlertDlgUpdate : public CAlertDlg
{
public:
	CAlertDlgUpdate(
		CWnd*		pNotifDIalog,
		CTransparentWindow*	pTransWindow,
		HWND		hWndParent,
		CWnd*		pParent,
		DhtmlGuiLocation	eGUIlocation,
		CString		sGUIlocationFilePath,
		long		lFreePosition);
	
	~CAlertDlgUpdate(void);
public:
	HRESULT OnAlertPickUp(IHTMLElement *pElement);
	HRESULT OnAlertClose(IHTMLElement *pElement);
protected:	
	virtual LRESULT OnAlertFadeCompleted(WPARAM wParam, LPARAM lParam ); //called by transparent window
	virtual HRESULT OnCbkEventsHandlerClickedEvent(IHTMLElement *pElement);
	virtual HRESULT CalcluateAlertPosition(RECT* pRc,WINDOWPOS* pPosc);
	virtual HRESULT OnPageLoaded();


private:
	long		m_lFreePosition;
	long		m_lCallUID;
};