/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSessionDlg.h
///
///  CSessionDlg, RC session modeless dialog
///
///  @author "Archer Software" Kirill Solovyov @date 18.06.2006
///
////////////////////////////////////////////////////////////////////////
// CSessionDlg.h : Declaration of the CSessionDlg

#pragma once

#include "resource.h"       // main symbols

#include <atlhost.h>


// CSessionDlg

class CSessionDlg : 
	public CAxDialogImpl<CSessionDlg>
{
public:
	CSessionDlg();
	~CSessionDlg();
	enum { IDD = IDD_SESSIONDLG };

BEGIN_MSG_MAP(CSessionDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	CHAIN_MSG_MAP(CAxDialogImpl<CSessionDlg>)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};


