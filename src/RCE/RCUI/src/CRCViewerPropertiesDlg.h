/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRCViewerPropertiesDlg.h
///
///  CRCViewerPropertiesDlg, RCViewer properties modeless dialog
///
///  @author "Archer Software" Kirill Solovyov @date 18.06.2006
///
////////////////////////////////////////////////////////////////////////
// CRCViewerPropertiesDlg.h : Declaration of the CRCViewerPropertiesDlg

#pragma once

#include "resource.h"       // main symbols

#include <atlhost.h>
#include "CSkinnedElement.h"
#include "atlapp.h"
#include "wtl/atlctrls.h"
#include "CSkinnedLabel.h"
#include "CSkinnedButton.h"
#include "CSkinnedComboBox.h"
#include "CSkinnedLinkEx.h"

/// CSkinnedLinkEx with handler for mouse left button up
class CSkinnedLinkExWithClick
	:	public virtual CSkinnedLinkEx
{
private:
/// Resource id of control
	int m_id;
public:
/// Constructor
	CSkinnedLinkExWithClick(const int regularImageId,
							const int mouseOverImageId,
							CSkinnedElement *parent,
							int id,
							const UINT imgAlign = DT_RIGHT)
		:	CSkinnedLinkEx(regularImageId, mouseOverImageId, parent, imgAlign)
		,	CSkinnedLink(parent)
		,	CSkinnedLabel(parent)
		,	m_id(id)
	{};
/// Handler for mouse left button up
	virtual LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
	TRY_CATCH
		/// Get parent control
		CSkinnedElement* parent = GetParentElement();
		if(parent)
		{
			/// Get handle of parent control
			HWND parentHWND = parent->GetWindowHandle();
			/// Send click on static control to parent
			SendMessage(parentHWND, WM_COMMAND, (STN_CLICKED<<16)|m_id, (LPARAM)GetWindowHandle());
		}
		/// Call method of base class
		return CSkinnedLabel::OnLButtonUp(uMsg, wParam, lParam, bHandled);
	CATCH_THROW()
	}
};


// CRCViewerPropertiesDlg

class CRCViewerPropertiesDlg : 
	public CAxDialogImpl<CRCViewerPropertiesDlg>,
	public CSkinnedElement
{
	friend class CRCViewerUIMediator;
protected:
	/// Image for left margin
	CImage *m_leftMarginImage;
	/// Image for right margin
	CImage *m_rightMarginImage;
	/// resource index of skin image
	const int m_skinId;
	/// Applay button
	CSkinnedButton m_applyBtn;
	/// Send ctl+alt+del btn
	CSkinnedButton m_cadBtn;
	/// Dialog lables
	CSkinnedLabel m_titleLbl;
	CSkinnedLabel m_displayTitleLbl;
	CSkinnedLabel m_colorsTitleLbl;
	CSkinnedLabel m_compressionTitleLbl;

BEGIN_MSG_MAP(CRCViewerPropertiesDlg)
	TRY_CATCH
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		COMMAND_HANDLER(IDC_VPAPPLY, BN_CLICKED, OnBnClickedVpapply)
		COMMAND_HANDLER(IDC_CAD_BTN, BN_CLICKED, OnBnClickedCAD)
		COMMAND_HANDLER(IDC_VPDISPLAY, CBN_SELCHANGE, OnComboSelected)
		COMMAND_HANDLER(IDC_VPCOLORS, CBN_SELCHANGE, OnComboSelected)
		COMMAND_HANDLER(IDC_VPCOMPRESSION, CBN_SELCHANGE, OnComboSelected)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(CSkinnedElement)// call SkinnedElement's handlers
		REFLECT_NOTIFICATIONS()
	CHAIN_MSG_MAP(CAxDialogImpl<CRCViewerPropertiesDlg>)
	CATCH_LOG()
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedVpapply(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCAD(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnComboSelected(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	/// Erase background event handler
	LRESULT OnEraseBkgnd(HWND hWnd, UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	/// Nonclient area redraw event handler
	LRESULT OnNCPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
public:
	enum { IDD = IDD_RCVIEWERPROPERTIESDLG };
	CRCViewerPropertiesDlg(CSkinnedElement *parent);
	~CRCViewerPropertiesDlg();
	/// Applay button event
	boost::function<void (int display,int colors, int compression)> m_applyBtnClick;
	/// One of comboboxes was selected event
	boost::function<void ()> m_comboSelChanged;
	/// Send Ctrl + Alt + Del button was pressed
	boost::function<void ()> m_cadBtnClick;
	/// display combobox
	CSkinnedComboBox m_displayCmb;
	/// colors combobox
	CSkinnedComboBox m_colorsCmb;
	/// compression combobox
	CSkinnedComboBox m_compressionCmb;

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
};


