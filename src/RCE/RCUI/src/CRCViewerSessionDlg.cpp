/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRCViewerSessionDlg.cpp
///
///  CRCViewerSessionDlg, RCViewer session modeless dialog
///
///  @author "Archer Software" Kirill Solovyov @date 18.06.2006
///
////////////////////////////////////////////////////////////////////////
// CRCViewerSessionDlg.cpp : Implementation of CRCViewerSessionDlg

#include "stdafx.h"
#include "CRCViewerSessionDlg.h"

// CRCViewerSessionDlg
CRCViewerSessionDlg::CRCViewerSessionDlg(CSkinnedElement *parent)
	:	CSkinnedElement(parent),
		m_skinId(IDR_VSESSION),
		m_customerTitleLbl(this),
		m_customerLbl(this),
		m_permissionTitleLbl(this),
		m_propertiesBtn(this, IDR_DOWN_REGULAR_74_22, IDR_BTN_DISABLED_72_22, IDR_DOWN_PRESSED_74_22, IDR_DOWN_MOUSE_OVER_74_22),
		m_startBtn(this, IDR_BTN_REGULAR_72_22, IDR_BTN_DISABLED_72_22, IDR_BTN_PRESSED_72_22, IDR_BTN_MOUSEOVER_72_22),
		m_permissionCmb(this, IDR_VSCOMBOBOXREGULAR, IDR_VSCOMBOBOXDISABLED, IDR_VSCOMBOBOXPRESSED, IDR_VSCOMBOBOXREGULAR),
		m_startBtnClick(NULL),
		m_permissionCmbSelChange(NULL),
		m_propertiesDlg(this),
		m_hideWallpaperChk(IDR_CHB_CHECKED_REGULAR, IDR_CHB_CHECKED_MOUSEOVER, this, IDC_HIDE_WALLPAPER_CHBOX, DT_LEFT),
		m_hideWallpaper(true),
		m_hideWallpaperEnabled(TRUE)
{
TRY_CATCH
	//m_customerTitleLbl.TextAlign = DT_RIGHT;
	//m_permisionTitleLbl.TextAlign = DT_RIGHT;
CATCH_LOG()
}

CRCViewerSessionDlg::~CRCViewerSessionDlg()
{
TRY_CATCH
	m_currentImage = NULL;
CATCH_LOG()
}

LRESULT CRCViewerSessionDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	/// Treating images
	if (NULL != m_skinsImageList.get())
		m_skinsImageList->ImageFromRes(&m_currentImage,1,&m_skinId);
	else
		Log.Add(_WARNING_,_T("NULL images list while loading control"));

	/// Attaching classes
	CAxDialogImpl<CRCViewerSessionDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	m_propertiesBtn.Attach(GetDlgItem(IDC_VSPROPERTIES));
	m_startBtn.Attach(GetDlgItem(IDC_VSSTART));
	m_permissionCmb.Attach(GetDlgItem(IDC_VSPERMISSION));
	m_customerLbl.Attach(GetDlgItem(IDC_VSCUSTOMER));
	m_permissionTitleLbl.Attach(GetDlgItem(IDC_VSPERMISIONTITLE));
	m_customerTitleLbl.Attach(GetDlgItem(IDC_VSCUSTOMERTITLE));
	m_hideWallpaperChk.Attach(GetDlgItem(IDC_HIDE_WALLPAPER_CHBOX));

	/// Adjusting controls
	m_permissionCmb.SetCurSel(0);

	/// Aplying skin
	m_startBtn.Font				= *m_logFont.get();
	m_startBtn.FontColor1		= SCVUI_BTNFONTCOLOR1;
	m_startBtn.FontColor2		= SCVUI_BTNFONTCOLOR2;
	m_propertiesBtn.Font		= m_startBtn.Font;
	m_propertiesBtn.FontColor1	= SCVUI_BTNFONTCOLOR1;
	m_propertiesBtn.FontColor2	= SCVUI_BTNFONTCOLOR2;


	m_customerTitleLbl.Font			= *m_logFontBold.get();
	m_customerTitleLbl.FontColor1	= SCVUI_LBLFONTCOLOR1;
	m_customerLbl.Font				= *m_logFont.get();
	m_customerLbl.FontColor1		= SCVUI_LBLFONTCOLOR1;
	m_permissionTitleLbl.Font		= *m_logFontBold.get();
	m_permissionTitleLbl.FontColor1	= SCVUI_LBLFONTCOLOR1;

	m_permissionCmb.Setup(*m_logFont.get(),SCVUI_LBLFONTCOLOR1,SCVUI_LBLFONTCOLOR1,SCVUI_CMBBKCOLOR1,SCVUI_CMBBKCOLOR2,SCVUI_CMBEDGECOLOR1,_T(""),_T(""));

	m_hideWallpaperChk.Font = *m_logFont.get();
	m_hideWallpaperChk.Font2 = *m_logFont.get();
	m_hideWallpaperChk.FontColor1 = SCVUI_BTNFONTCOLOR1;
	m_hideWallpaperChk.FontColor2 = SCVUI_BTNFONTCOLOR2;
	m_hideWallpaperChk.BkColor1 = SCVUI_BTNFONTCOLOR2;
	m_hideWallpaperChk.SetText(_T("Disable customer background"));

	/// Calling OnSkinChanged for certain contols
	m_propertiesBtn.OnSkinChanged();
	m_startBtn.OnSkinChanged();
	m_customerTitleLbl.OnSkinChanged();
	m_customerLbl.OnSkinChanged();
	m_permissionTitleLbl.OnSkinChanged();
	m_permissionCmb.OnSkinChanged();
	m_hideWallpaperChk.OnSkinChanged();


	// tooltips
	m_startBtn.Hint=_T("Start button");
	m_tip.Create(m_hWnd);
	// add tooltip for start button
	CToolInfo toolInfoStartBtn(TTF_SUBCLASS|TTF_IDISHWND,m_startBtn.m_hWnd,reinterpret_cast<UINT_PTR>(m_startBtn.m_hWnd));
	if(!m_tip.AddTool(&toolInfoStartBtn))
		Log.Add(_WARNING_,_T("AddTool to sessionDlg start button failed"));
	// add tooltip for session dlg
	// for start button rectangle
	RECT rect;
	m_startBtn.GetWindowRect(&rect);
	ScreenToClient(&rect);
	CToolInfo toolInfo(TTF_SUBCLASS,m_hWnd,1,&rect);
	//add tooltip for whole session dlg
	//CToolInfo toolInfo(TTF_SUBCLASS|TTF_IDISHWND,m_hWnd,reinterpret_cast<UINT_PTR>(m_hWnd),&rect);
	if(!m_tip.AddTool(&toolInfo))
		Log.Add(_WARNING_,_T("AddTool to sessionDlg for start button failed"));
	m_tip.Activate(TRUE);

	OnSkinChanged();
CATCH_LOG()
	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

LRESULT CRCViewerSessionDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
TRY_CATCH

	if ( ::IsWindow(m_tip.m_hWnd) )		// May be this will remove ASSERT failure, if not delete this line
		m_tip.DestroyWindow();

CATCH_LOG()
	return 0;
}

LRESULT CRCViewerSessionDlg::OnBnClickedVSProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
TRY_CATCH

	if (NULL != m_showPropertiesBtnClick)
		m_showPropertiesBtnClick();

CATCH_LOG();
	return 0;
}


LRESULT CRCViewerSessionDlg::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
TRY_CATCH
	//The low-order word of lParam specifies the new width of the client area. 
	//The high-order word of lParam specifies the new height of the client area. 
	RECT rect;
	if(!m_propertiesBtn.GetWindowRect(&rect))
		throw MCException_Win("Properties button window rect obtaining failed");
	if(!ScreenToClient(&rect))
		throw MCException_Win("RCViewerSessionDlg ScreenToClient conversion failed");
	if(!m_propertiesBtn.SetWindowPos(	NULL,LOWORD(lParam)-(rect.right-rect.left)-10,rect.top,0,0,
																		SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE))
		throw MCException_Win("Viewer session dialog properties button resizing failed");
CATCH_LOG()
	return 0;
}

LRESULT CRCViewerSessionDlg::OnBnClickedVSStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
TRY_CATCH
	if(NULL != m_startBtnClick)
		m_startBtnClick(m_permissionCmb.GetCurSel());
CATCH_LOG()
	return 0;
}

void CRCViewerSessionDlg::OnSkinChanged()
{
TRY_CATCH
	BOOL bHandled;
	OnSize(0,0,0,bHandled);
CATCH_THROW()
}
LRESULT CRCViewerSessionDlg::OnCbnSelChangeVSPermission(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
TRY_CATCH
	if(NULL != m_permissionCmbSelChange)
		m_permissionCmbSelChange(m_permissionCmb.GetCurSel());
CATCH_LOG()
	return 0;
}

LRESULT CRCViewerSessionDlg::OnGetDispInfo(int id, LPNMHDR lpnmhdr, BOOL &bHandled)
{
TRY_CATCH
	// request hint from child window 
	POINT pt;
	::GetCursorPos(&pt);
	ScreenToClient(&pt);
	CWindow window=ChildWindowFromPoint(pt);
	if(window.IsWindow() && window.m_hWnd != m_hWnd)
		return	window.SendMessage(WM_NOTIFY,id,reinterpret_cast<LPARAM>(lpnmhdr));
	return CSkinnedElement::OnGetDispInfo(id,lpnmhdr,bHandled);
CATCH_LOG()
	return 0;
}

LRESULT CRCViewerSessionDlg::OnHideWallpaperChboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
TRY_CATCH

	m_hideWallpaper = !m_hideWallpaper;
	m_hideWallpaperChk.SetImages(m_hideWallpaper?IDR_CHB_CHECKED_REGULAR:IDR_CHB_UNCHECKED_REGULAR, m_hideWallpaper?IDR_CHB_CHECKED_MOUSEOVER:IDR_CHB_UNCHECKED_MOUSEOVER);
	m_hideWallpaperChk.OnSkinChanged();
	
CATCH_LOG()
	return 0;
}

void CRCViewerSessionDlg::SetHideWallpaperEnabled(const bool enabled)
{
TRY_CATCH
	if(enabled != m_hideWallpaperEnabled)
	{
		m_hideWallpaperEnabled = enabled;
		if(m_hideWallpaperEnabled)
			m_hideWallpaperChk.SetImages(m_hideWallpaper?IDR_CHB_CHECKED_REGULAR:IDR_CHB_UNCHECKED_REGULAR, m_hideWallpaper?IDR_CHB_CHECKED_MOUSEOVER:IDR_CHB_UNCHECKED_MOUSEOVER);
		else
			m_hideWallpaperChk.SetImages(m_hideWallpaper?IDR_CHB_CHECKED_DISABLED:IDR_CHB_UNCHECKED_DISABLED, m_hideWallpaper?IDR_CHB_CHECKED_DISABLED:IDR_CHB_UNCHECKED_DISABLED);
		m_hideWallpaperChk.EnableWindow(m_hideWallpaperEnabled);
		m_hideWallpaperChk.OnSkinChanged();
	}
CATCH_THROW()
}

