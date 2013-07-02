// SessionDlg.cpp : Implementation of CSessionDlg

#include "stdafx.h"
#include "SessionDlg.h"
#include <NWL/Streaming/CStreamException.h>
#include "wtl/atlctrls.h"

// CSessionDlg

LRESULT CSessionDlg::OnBnClickedStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	::Beep(440,50);
	if(m_startClick)
		m_startClick();
	return 0;
}
LRESULT CSessionDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CSessionDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	//propertiesDlg.reset(new CPropertiesDlg);
	//propertiesDlg->Create(GetParent());
	//if(!SetWindowPos(HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_FRAMECHANGED|SWP_NOOWNERZORDER))
		//throw MCException_Win("Z-order set failed");
	//if(!BringWindowToTop())
		//throw MCException_Win("BringWindowToTop() failed");
	CButton b;
	b=GetDlgItem(IDC_PROPERTIES);
	HRGN hRgn;
	RECT dlgRect;
	b.GetWindowRect(&dlgRect);
	b.ScreenToClient(&dlgRect);
	hRgn=CreateEllipticRgn(0,0,dlgRect.right-dlgRect.left,dlgRect.bottom-dlgRect.top);
	b.SetWindowRgn(hRgn);
	//b.SetWindowText(_T("gopa"));
	CComPtr<IStream> skinStream;
	skinStream.Attach(LoadStream(_AtlBaseModule.m_hInst,RT_SKINS,MAKEINTRESOURCE(IDR_SKINS1)));
	skinImage.Load(skinStream);
	m_currentImage=&skinImage;
	//m_propertiesBtn.SubclassWindow(GetDlgItem(IDC_COMBO1));
	//+combobox
	m_comboBox.m_currentImage=&skinImage;
	//m_comboBox.Attach(GetDlgItem(IDC_COMBO1));
	m_comboBox.SubclassWindow(GetDlgItem(IDC_COMBO1));

	
	//-combobox
	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

LRESULT CSessionDlg::OnBnClickedProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DWORD res=0;
	SetWindowPos(HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
	res=GetLastError();
	if(propertiesDlg->IsWindowVisible())
	{
		propertiesDlg->ShowWindow(SW_HIDE);
		//if(!::AnimateWindow(*propertiesDlg,ANIMATE_TIME,AW_VER_NEGATIVE|AW_HIDE|AW_SLIDE))
		//	throw MCException_Win("Properties dialog hiding failed");
	}
	else
	{
		propertiesDlg->ShowWindow(SW_SHOW);
		//if(!::AnimateWindow(*propertiesDlg,ANIMATE_TIME,AW_VER_POSITIVE|AW_SLIDE))
		//	throw MCException_Win("Properties dialog showing failed");
		
	}

	return 0;
}

LRESULT CSessionDlg::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	//propertiesDlg under by vertical and center by horizontal
	RECT dlgRect, propertiesDlgRect;
	GetWindowRect(&dlgRect);
	GetParent().ScreenToClient(&dlgRect);
	propertiesDlg->GetWindowRect(&propertiesDlgRect);
	GetParent().ScreenToClient(&propertiesDlgRect);
	propertiesDlg->SetWindowPos(NULL,dlgRect.left+(dlgRect.right-dlgRect.left)/2-(propertiesDlgRect.right-propertiesDlgRect.left)/2,dlgRect.bottom-10,
															0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
	return 0;
}

LRESULT CSessionDlg::OnBnClickedStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	::Beep(440,50);
	if(m_stopClick)
		m_stopClick();
	return 0;
}
LRESULT CSessionDlg::OnErasebkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	HDC hdc=reinterpret_cast<HDC>(wParam);
	RECT dlgRect;
	GetWindowRect(&dlgRect);
	ScreenToClient(&dlgRect);
	propertiesDlg->skinImage.Draw(hdc,dlgRect);
	return 1L;
}
LRESULT CSessionDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	//m_propertiesBtn.UnsubclassWindow();
	return 0;
}
LRESULT CSessionDlg::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DRAWITEMSTRUCT *draw=reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
	skinImage.Draw(draw->hDC,draw->rcItem);
	return 0;
}

LRESULT CSessionDlg::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	//::MessageBox(NULL,_T("CSessionDlg::OnLButtonDown"),NULL,0);
	::Beep(440,50);
	m_comboBox.ShowDropDown(m_comboBox.GetDroppedState()?0:1);
	return 0;
}
