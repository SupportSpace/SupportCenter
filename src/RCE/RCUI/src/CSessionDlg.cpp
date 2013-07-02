// CSessionDlg.cpp : Implementation of CSessionDlg
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSessionDlg.cpp
///
///  CSessionDlg, RC session modeless dialog
///
///  @author "Archer Software" Kirill Solovyov @date 18.06.2006
///
////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CSessionDlg.h"


// CSessionDlg
CSessionDlg::CSessionDlg()
{
}

CSessionDlg::~CSessionDlg()
{
}

LRESULT CSessionDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CSessionDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;
	return 1;  // Let the system set the focus
}
