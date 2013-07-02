/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRCViewerStatusBarDlg.h
///
///  CRCViewerStatusBarDlg, RCViewer status bar modeless dialog
///
///  @author "Archer Software" Kirill Solovyov @date 20.06.2006
///
////////////////////////////////////////////////////////////////////////
// CRCViewerStatusBarDlg.h : Declaration of the CRCViewerStatusBarDlg

#pragma once

#include "CSkinnedElement.h"
#include "CSkinnedLabel.h"
#include "resource.h"       // main symbols
#include <atlhost.h>
#include "AidLib/CException/CException.h"
#include <atlapp.h>
#include "wtl/atlctrls.h"
#include "CSkinnedStatic.h"

// CRCViewerStatusBarDlg
class CRCViewerStatusBarDlg : 
	public CAxDialogImpl<CRCViewerStatusBarDlg>,
	public CSkinnedElement
{
protected:
		/// resource index of skin image
	const int m_skinId;

BEGIN_MSG_MAP(CRCViewerStatusBarDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	// call SkinnedElement's handlers
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	CHAIN_MSG_MAP(CSkinnedElement)
	CHAIN_MSG_MAP(CAxDialogImpl<CRCViewerStatusBarDlg>)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

		//Dialog message handlers
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		/// left image
		CSkinnedStatic m_drag;

public:
	enum { IDD = IDD_RCVIEWERSTATUSBARDLG };
	CRCViewerStatusBarDlg(CSkinnedElement *parent);
	~CRCViewerStatusBarDlg();

	//status bar status message control
	CSkinnedLabel m_statusMessage;
	// status bar status icon
	CSkinnedStatic m_statusIcon;

	/// Accessor to underlying window handle in CSkinnedElement. @see CSkinnedElement
	/// return control window handle
	HWND GetWindowHandle()
	{
		return static_cast<HWND>(*this);
	}
	/// common font
	boost::shared_ptr<LOGFONT> m_logFont;
	/// bold font
	boost::shared_ptr<LOGFONT> m_logFontBold;

	/// Should be called each time skin was changed
	virtual void OnSkinChanged() {};

};
