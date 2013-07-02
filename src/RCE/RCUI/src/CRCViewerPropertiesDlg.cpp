/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRCViewerPropertiesDlg.cpp
///
///  CRCViewerPropertiesDlg, RCViewer properties modeless dialog
///
///  @author "Archer Software" Kirill Solovyov @date 18.06.2006
///
////////////////////////////////////////////////////////////////////////
// CRCViewerPropertiesDlg.cpp : Implementation of CRCViewerPropertiesDlg

#include "stdafx.h"
#include "CRCViewerPropertiesDlg.h"

// CRCViewerPropertiesDlg
CRCViewerPropertiesDlg::CRCViewerPropertiesDlg(CSkinnedElement *parent)
	:	CSkinnedElement(parent),
		m_skinId(IDR_VPROPERTIES_CNTR),
		m_titleLbl(this),
		m_displayTitleLbl(this),
		m_colorsTitleLbl(this),
		m_compressionTitleLbl(this),
		m_applyBtn(this, IDR_BTN_REGULAR_72_22, IDR_BTN_DISABLED_72_22, IDR_BTN_PRESSED_72_22, IDR_BTN_MOUSEOVER_72_22),
		m_cadBtn(this, IDR_CAD_REGULAR, IDR_CAD_DISABLED, IDR_CAD_REGULAR, IDR_CAD_MOUSEOVER),
		m_displayCmb(this,IDR_VSCOMBOBOXREGULAR, IDR_VSCOMBOBOXDISABLED, IDR_VSCOMBOBOXPRESSED, IDR_VSCOMBOBOXREGULAR),
		m_colorsCmb(this,IDR_VSCOMBOBOXREGULAR, IDR_VSCOMBOBOXDISABLED, IDR_VSCOMBOBOXPRESSED, IDR_VSCOMBOBOXREGULAR),
		m_compressionCmb(this,IDR_VSCOMBOBOXREGULAR, IDR_VSCOMBOBOXDISABLED, IDR_VSCOMBOBOXPRESSED, IDR_VSCOMBOBOXREGULAR),
		m_applyBtnClick(NULL)
{
TRY_CATCH
	//m_titleLbl.TextAlign = DT_RIGHT;
CATCH_LOG()
}

CRCViewerPropertiesDlg::~CRCViewerPropertiesDlg()
{
TRY_CATCH

CATCH_LOG()
}

LRESULT CRCViewerPropertiesDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	/// Treating images
	if (NULL != m_skinsImageList.get())
	{
		m_skinsImageList->ImageFromRes(&m_currentImage,1,&m_skinId);
		int id = IDR_VPROPERTIES_LEFT;
		m_skinsImageList->ImageFromRes(&m_leftMarginImage,1,&id);
		id = IDR_VPROPERTIES_RIGHT;
		m_skinsImageList->ImageFromRes(&m_rightMarginImage,1,&id);

		/*
		asd
		CImage *fullWindowImage;
		id = IDR_VPROPERTIES_FULL;
		m_skinsImageList->ImageFromRes(&fullWindowImage,1,&id);
		if (fullWindowImage)
		{
			asd
		}
		*/
	}
	else
		Log.Add(_WARNING_,_T("NULL images list while loading control"));

	/// Attaching classes
	CAxDialogImpl<CRCViewerPropertiesDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	m_applyBtn.Attach(GetDlgItem(IDC_VPAPPLY));
	m_cadBtn.Attach(GetDlgItem(IDC_CAD_BTN));
	m_displayCmb.Attach(GetDlgItem(IDC_VPDISPLAY));
	m_compressionCmb.Attach(GetDlgItem(IDC_VPCOMPRESSION));
	m_colorsCmb.Attach(GetDlgItem(IDC_VPCOLORS));
	m_titleLbl.Attach(GetDlgItem(IDC_VPTITLE));
	m_displayTitleLbl.Attach(GetDlgItem(IDC_VPDISPLAYTITLE));
	m_colorsTitleLbl.Attach(GetDlgItem(IDC_VPCOLORSTITLE));
	m_compressionTitleLbl.Attach(GetDlgItem(IDC_VSCOMPRESSIONTITLE));

	/// Adjusting controls
	m_compressionCmb.SetCurSel(0);
	m_displayCmb.SetCurSel(0);
	m_colorsCmb.SetCurSel(0);
	m_displayCmb.SetCurSel(0);

	/// Applying skin
	m_applyBtn.Font			= *m_logFont.get();
	m_applyBtn.FontColor1	= SCVUI_BTNFONTCOLOR1;
	m_applyBtn.FontColor2	= SCVUI_BTNFONTCOLOR2;

	m_titleLbl.Font					= *m_logFontBold.get();
	m_titleLbl.FontColor1			= SCVUI_LBLFONTCOLOR1;
	m_displayTitleLbl.Font			= *m_logFont.get();
	m_displayTitleLbl.FontColor1	= m_titleLbl.FontColor1;
	m_colorsTitleLbl.Font			= *m_logFont.get();
	m_colorsTitleLbl.FontColor1		= m_titleLbl.FontColor1;
	m_compressionTitleLbl.Font		= *m_logFont.get();
	m_compressionTitleLbl.FontColor1	= m_titleLbl.FontColor1;
	m_displayCmb.Font = *m_logFont.get();
	m_displayCmb.FontColor1=SCVUI_LBLFONTCOLOR1;
	m_displayCmb.FontColor2=SCVUI_LBLFONTCOLOR1;
	m_displayCmb.BkColor1=SCVUI_CMBBKCOLOR1;
	m_displayCmb.BkColor2=SCVUI_CMBBKCOLOR2;
	m_displayCmb.EdgeColor1=SCVUI_CMBEDGECOLOR1;
	m_compressionCmb.Font = *m_logFont.get();
	m_compressionCmb.FontColor1=SCVUI_LBLFONTCOLOR1;
	m_compressionCmb.FontColor2=SCVUI_LBLFONTCOLOR1;
	m_compressionCmb.BkColor1=SCVUI_CMBBKCOLOR1;
	m_compressionCmb.BkColor2=SCVUI_CMBBKCOLOR2;
	m_compressionCmb.EdgeColor1=SCVUI_CMBEDGECOLOR1;
	m_colorsCmb.Font = *m_logFont.get();
	m_colorsCmb.FontColor1=SCVUI_LBLFONTCOLOR1;
	m_colorsCmb.FontColor2=SCVUI_LBLFONTCOLOR1;
	m_colorsCmb.BkColor1=SCVUI_CMBBKCOLOR1;
	m_colorsCmb.BkColor2=SCVUI_CMBBKCOLOR2;
	m_colorsCmb.EdgeColor1=SCVUI_CMBEDGECOLOR1;

	/// Calling OnSkinChanged for certain contols
	m_titleLbl.OnSkinChanged();
	m_displayTitleLbl.OnSkinChanged();
	m_colorsTitleLbl.OnSkinChanged();
	m_compressionTitleLbl.OnSkinChanged();
	m_applyBtn.OnSkinChanged();
	m_colorsCmb.OnSkinChanged();
	m_compressionCmb.OnSkinChanged();
	m_displayCmb.OnSkinChanged();
	OnSkinChanged();

	bHandled = TRUE;
	return 1;  // Let the system set the focus
CATCH_THROW()
}


LRESULT CRCViewerPropertiesDlg::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
TRY_CATCH
	//The low-order word of lParam specifies the new width of the client area. 
	//The high-order word of lParam specifies the new height of the client area. 
	RECT rect;
	if(!m_applyBtn.GetWindowRect(&rect))
		throw MCException_Win("Apply button window rect obtaining failed");
	if(!ScreenToClient(&rect))
		throw MCException_Win("RCViewerPropertiesDlg ScreenToClient conversion failed");
	RECT parentRect;
	GetWindowRect(&parentRect);
	int delta = parentRect.bottom - parentRect.top - rect.bottom;
	if(!m_applyBtn.SetWindowPos(NULL,LOWORD(lParam)-(rect.right-rect.left)-delta,rect.top,0,0,
								SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE))
		throw MCException_Win("Viewer properties dialog Apply button resizing failed");
CATCH_LOG()
	return 0;	return 0;
}

LRESULT CRCViewerPropertiesDlg::OnComboSelected(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
TRY_CATCH
	if(NULL != m_comboSelChanged)
	{
		m_comboSelChanged();
	}
CATCH_LOG()
	return 0;
}

LRESULT CRCViewerPropertiesDlg::OnBnClickedCAD(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
TRY_CATCH
	if(NULL != m_cadBtnClick)
	{
		m_cadBtnClick();
	}
CATCH_LOG()
	return 0;
}


LRESULT CRCViewerPropertiesDlg::OnBnClickedVpapply(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
TRY_CATCH
	if(NULL != m_applyBtnClick)
	{
		m_applyBtnClick(m_displayCmb.GetCurSel(),m_colorsCmb.GetCurSel(),m_compressionCmb.GetCurSel());
	}
CATCH_LOG()
	return 0;
}

void CRCViewerPropertiesDlg::OnSkinChanged()
{
TRY_CATCH
	BOOL bHandled;
	OnSize(0,0,0,bHandled);
CATCH_THROW()
}

LRESULT CRCViewerPropertiesDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
TRY_CATCH
CATCH_LOG()
	return 0;
}

LRESULT CRCViewerPropertiesDlg::OnEraseBkgnd(HWND hWnd, UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
TRY_CATCH
	if(IsWindow())
	{
		HDC hdc=reinterpret_cast<HDC>(wParam);
		PAINTSTRUCT m_ps;
		HDC hDC(hdc?hdc:(BeginPaint(&m_ps)));
		RECT windowRect;
		GetWindowRect(&windowRect);
		ScreenToClient(&windowRect);
		LRESULT res = 0L;
		if(m_currentImage && RectIsntEmpty(windowRect))
		{
			bHandled=TRUE;
			res = m_currentImage->Draw(hDC,windowRect);
			if (m_leftMarginImage)
			{
				m_leftMarginImage->Draw(hDC,0,0,m_leftMarginImage->GetWidth(),windowRect.bottom - windowRect.top);
			}
			if (m_rightMarginImage)
			{
				m_rightMarginImage->Draw(hDC,windowRect.right - m_rightMarginImage->GetWidth(), 0, m_rightMarginImage->GetWidth(),windowRect.bottom - windowRect.top);
			}
		}
		if (!hdc) 
			EndPaint(&m_ps);
		return res;
	}
CATCH_LOG()
	bHandled=FALSE;
	return 0L;
}

LRESULT CRCViewerPropertiesDlg::OnNCPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
TRY_CATCH
	if(IsWindow())
	{
		HDC hdc;
		if(wParam==1)
			hdc=GetDCEx(0,DCX_WINDOW|DCX_PARENTCLIP);
		else
			hdc=GetDCEx(reinterpret_cast<HRGN>(wParam),DCX_WINDOW|DCX_INTERSECTRGN);
		LRESULT res = OnEraseBkgnd(hWnd,uMsg,(WPARAM)hdc,0,bHandled);
		ReleaseDC(hdc);
		if(res)
			return 0;
	}
CATCH_LOG()
	bHandled=FALSE;
	return 1L;//unprocessed
}