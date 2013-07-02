/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRCViewerStatusBarDlg.cpp
///
///  CRCViewerStatusBarDlg, RCViewer status bar modeless dialog
///
///  @author "Archer Software" Kirill Solovyov @date 20.06.2006
///
////////////////////////////////////////////////////////////////////////
// CRCViewerStatusBarDlg.cpp : Implementation of CRCViewerStatusBarDlg

#include "CRCViewerStatusBarDlg.h"
#include "stdafx.h"


// CRCViewerStatusBarDlg
CRCViewerStatusBarDlg::CRCViewerStatusBarDlg(CSkinnedElement *parent)
	:	CSkinnedElement(parent),
		m_skinId(IDR_VSTATUS),
		m_statusMessage(this)
{
TRY_CATCH
CATCH_LOG()
}

CRCViewerStatusBarDlg::~CRCViewerStatusBarDlg()
{
TRY_CATCH
	
CATCH_LOG()
}

LRESULT CRCViewerStatusBarDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	CAxDialogImpl<CRCViewerStatusBarDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	if (NULL != m_skinsImageList.get())
		m_skinsImageList->ImageFromRes(&m_currentImage,1,&m_skinId);
	else
		Log.Add(_WARNING_,_T("NULL images list while loading control"));
	m_statusMessage.Font=*m_logFont.get();
	m_statusMessage.Attach(GetDlgItem(IDC_VSBMESSAGE));
	//+ drag image
	CStatic drag;
	int drag_skinId=IDR_VSTATUS_DRAG;
	CImage *drag_image;
	if (NULL != m_skinsImageList.get())
		m_skinsImageList->ImageFromRes(&drag_image,1,&drag_skinId);
	else
		Log.Add(_WARNING_,_T("NULL images list while loading control"));
	// obtaining transparent index color
	if(drag_image->GetBPP()!=8)
		Log.Add(_WARNING_,_T("Drag image bit per pixel differ from 8. Transparent color will not set."));
	else
	{
		TRY_CATCH
			drag_image->SetTransparentColor(*(reinterpret_cast<BYTE*>(drag_image->GetPixelAddress(0,drag_image->GetHeight()-1))));
		CATCH_LOG()
	}
	m_drag.m_currentImage=drag_image;
	m_drag.SubclassWindow(GetDlgItem(IDC_VSBDRAG));
	m_drag.SetWindowPos(NULL,0,0,drag_image->GetWidth(),drag_image->GetHeight(),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	//- drag image
	//+ status icon
	m_statusIcon.SubclassWindow(GetDlgItem(IDC_VSBSTATUSICON));
	//- status icon
CATCH_LOG()
	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

LRESULT CRCViewerStatusBarDlg::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	//The low-order word of lParam specifies the new width of the client area. 
	//The high-order word of lParam specifies the new height of the client area. 
TRY_CATCH
	RECT statusMessageRect;
	if(!m_statusMessage.GetWindowRect(&statusMessageRect))
		throw	MCException_Win("StatusMessage window rect obtaining failed");
	if(!ScreenToClient(&statusMessageRect))
		throw MCException_Win("StatusMessage window ScreenToClient conversion failed");
	if(!m_statusMessage.SetWindowPos(	NULL,0,0,
																		LOWORD(lParam)-statusMessageRect.left,statusMessageRect.bottom-statusMessageRect.top,
																		SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE))
		throw MCException_Win("StatusMessage resizing failed");
CATCH_LOG()
	return 0;
}

LRESULT CRCViewerStatusBarDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
TRY_CATCH
CATCH_LOG()
	return 0;
}
