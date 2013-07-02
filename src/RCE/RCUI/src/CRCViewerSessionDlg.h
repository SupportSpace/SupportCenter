/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRCViewerSessionDlg.h
///
///  CRCViewerSessionDlg, RCViewer session modeless dialog
///
///  @author "Archer Software" Kirill Solovyov @date 18.06.2006
///
////////////////////////////////////////////////////////////////////////
// CRCViewerSessionDlg.h : Declaration of the CRCViewerSessionDlg

#pragma once

#include "resource.h"       // main symbols

#include <atlhost.h>
#include "CRCViewerPropertiesDlg.h"
#include "CSkinnedComboBox.h"
#include <atlapp.h>
#include <wtl\atlctrls.h>
#include <AidLib/CException/CException.h>
#include "CSkinnedLabel.h"
#include "CSkinnedButton.h"

// CRCViewerSessionDlg
class CRCViewerSessionDlg : 
	public CAxDialogImpl<CRCViewerSessionDlg>,
	public CSkinnedElement
{
protected:
	/// resource index of skin image
	const int m_skinId;
	BEGIN_MSG_MAP(CRCViewerSessionDlg)
		TRY_CATCH
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDC_VSPROPERTIES, BN_CLICKED, OnBnClickedVSProperties)
			MESSAGE_HANDLER(WM_SIZE, OnSize)
			COMMAND_HANDLER(IDC_VSSTART, BN_CLICKED, OnBnClickedVSStart)
			COMMAND_HANDLER(IDC_VSPERMISSION, CBN_SELCHANGE, OnCbnSelChangeVSPermission)
			COMMAND_HANDLER(IDC_HIDE_WALLPAPER_CHBOX, STN_CLICKED, OnHideWallpaperChboxClicked)
			//REFLECT_NOTIFICATIONS()
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			CHAIN_MSG_MAP(CSkinnedElement)// call SkinnedElement's handlers
			REFLECT_NOTIFICATIONS()
			CHAIN_MSG_MAP(CAxDialogImpl<CRCViewerSessionDlg>)
		CATCH_LOG()
	END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	/// settings bar button
	CSkinnedButton m_propertiesBtn;
	/// permission combobox
	CSkinnedComboBox m_permissionCmb;
	/// customer name title
	CSkinnedLabel m_customerTitleLbl;
	/// customer name
	CSkinnedLabel m_customerLbl;
	/// permission combobox title
	CSkinnedLabel m_permissionTitleLbl;
	/// tooltip
	CToolTipCtrl m_tip;

	/// If true wallpaper will be hidden on host
	bool m_hideWallpaper;
	/// State of "Hide wallpaper" control
	bool m_hideWallpaperEnabled;
	CSkinnedLinkExWithClick m_hideWallpaperChk;

	// control events
	LRESULT OnBnClickedVSProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedVSStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCbnSelChangeVSPermission(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	virtual LRESULT OnGetDispInfo(int id, LPNMHDR lpnmhdr, BOOL &bHandled);
	/// "Hide wallpaper" control clicked handler
	LRESULT OnHideWallpaperChboxClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

public:
	enum { IDD = IDD_RCVIEWERSESSIONDLG };
	CRCViewerSessionDlg(CSkinnedElement *parent);
	~CRCViewerSessionDlg();
	/// RCViewer session properties modeless dialog
	CRCViewerPropertiesDlg m_propertiesDlg;
	/// start button
	CSkinnedButton m_startBtn;
	/// call on start button click by session dialog
	boost::function<void (int)> m_startBtnClick;
	/// call on permission change by session dialog
	boost::function<void (int)> m_permissionCmbSelChange;
	/// call on show properties button
	boost::function<void ()> m_showPropertiesBtnClick;
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
	virtual void OnSkinChanged();
	
	/// Changes state of "Hide wallpaper control"
	void SetHideWallpaperEnabled(const bool enabled);
};


