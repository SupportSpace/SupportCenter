/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedMsgBox.h
///
///  Skinned MsgBox implementation
///
///  @author "Archer Software" Sogin M. @date 26.12.2007
///
////////////////////////////////////////////////////////////////////////<summary>

#pragma once

#include "resource.h"
#include "..\..\RCUI\src\CSkinnedElement.h"
#include "..\..\RCUI\src\CSkinnedButton.h"
#include "..\..\RCUI\src\CSkinnedLabel.h"
#include "..\..\RCUI\src\CSkinnedLinkEx.h"
#include "CWidgetLabel.h"

#define CAPTION_HEIGHT 30

/// Skinned MsgBox control implementation
/// @see CSkinnedMsgBoxMultiline
class CSkinnedMsgBox :
	public CDialogImpl<CSkinnedMsgBox>,
	public CAbstractCommandManager
{
protected:

	/// List of messageBox types, which shouldn't be shown
	/// And last one user choice should be returned instead
	static std::map<int, INT_PTR> m_defaultList;

	bool m_modal;

	/// Question text
	tstring m_text;
	
	/// Text will be printing with words breaking
	bool m_autoBreakText;

	/// true if 'do not show this again messagebox' is clicked
	bool m_doNotShowAgain;

	/// Ok button control
	CSkinnedButton m_okBtn;

	/// Cancell btn control
	CSkinnedButton m_cancelBtn;

	/// MessageBox text
	CSkinnedLabel m_textLabel;

	/// 'Do not show this message again' checkbox control
	CWidgetLinkLabelEx m_showAgainCheckBox;

	/// true (default) if cancel button should be shown
	bool m_showCancel;

	/// true (default) if 'do not show this again messagebox' should be shown
	bool m_showCheckBox;

	/// Dispatched from CCommandProxy
	void ExecuteCommand(const CCommandProxy* sender, EWidgetCommand command, const void* commandData);
public:
	/// ctor
	/// @param parent - parent window - needed to center dialog
	/// @param showCancel true (default) if cancel button should be shown
	/// @param showCheckBox true (default) if 'do not show this again messagebox' should be shown
	CSkinnedMsgBox(CSkinnedElement *parent = NULL, const bool showCancel = true, const bool showCheckBox = true);
	/// class destructor
	virtual ~CSkinnedMsgBox();

	/// Returns IDOK or IDCANCEL, depending on user choice
	/// @param type - type of question - relates to 'do not show this again' checkbox.
	/// @param text - text, which will be shown to user
	/// @param autoBreakText  - use DT_WORDBREAK flag for text printing
	INT_PTR Show(const int type, const tstring& text, bool autoBreakText = false);

	/// Setup text message to show
	/// @param text - new dialog text
	void SetMessage(const tstring& text)
	{
	TRY_CATCH
		m_text = text;
	CATCH_THROW()
	}

	/// Setup show_cancel_button flag
	/// @param showCancel - if true - cancel button will be shown next show
	void SetShowCancelBtnFlag(const bool showCancel)
	{
	TRY_CATCH
		m_showCancel = showCancel;
	CATCH_THROW()
	}

	/// Setup show_checkbox flag
	/// @param showCancel - if true - check obx will be shown next show
	void SetShowCheckBoxFlag(const bool showCheckBox)
	{
	TRY_CATCH
		m_showCheckBox = showCheckBox;
	CATCH_THROW()
	}


	/// Shows dialog's window.
	/// @param modal Whether to show modal window.
	BOOL ShowDialog(bool modal = true, HWND hWndParent = ::GetActiveWindow(), LPARAM dwInitParam = 0  )
	{
		m_modal = modal;
		if( m_modal )
		{
			return DoModal(hWndParent, dwInitParam) != 0;
		}
		else
		{
			if( IsWindow() == FALSE ) Create(hWndParent, dwInitParam);
			return ShowWindow( SW_SHOW );
		}
	}
	
	/// Closes dialog's window.
	/// @param nRetCode Exit status for modal dialog.
	BOOL CloseDialog( INT_PTR nRetCode = 0 )
	{
		return ConditionalClose(nRetCode);
	}

	/// Conditionally closes (for modal dialog) or hides (for non-modal dialog) the window.
	/// @param nRetCode Exit status for modal dialog.
	BOOL ConditionalClose(INT_PTR nRetCode = 0)
	{
		if( m_modal )
		{
			m_modal = false;
			return EndDialog(nRetCode);
		}
		else
		{
			if (IsWindow())
				return DestroyWindow();
			return 0;
		}
	}


	/// Accessor to underlying window handle in CSkinnedElement. @see CSkinnedElement
	/// return control window handle
	HWND GetWindowHandle()
	{
		return static_cast<HWND>(*this);
	}
	
	enum { IDD = IDD_MSGBOX_DIALOG };

	/// DOXYS_OFF 
	BEGIN_MSG_MAP(CSkinnedMsgBox)
		TRY_CATCH
		if (uMsg == WM_NCPAINT)
			return 0;
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_NCHITTEST, OnNChitTest)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		CHAIN_MSG_MAP(CSkinnedElement)
		REFLECT_NOTIFICATIONS()
		CATCH_LOG("BEGIN_MSG_MAP(CSkinnedMsgBox)")
	END_MSG_MAP()
	/// DOXYS_ON

	/// Notification from CSkinEngine after it applies skin attributes.
	void OnSkinChanged();

	/// WM_NCHITTEST message handler
	/// @param uMsg message code (expected to be WM_NCHITTEST)
	/// @param wParam This parameter is not used
	/// @param lParam The low-order word specifies the x-coordinate of the cursor. The coordinate is relative to the upper-left corner of the screen. 
	/// The high-order word specifies the y-coordinate of the cursor. The coordinate is relative to the upper-left corner of the screen. 
	/// @param bHandled set to TRUE if message should not be further processed
	/// @return return value is obsolete
	virtual LRESULT OnNChitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/// WM_INITDIALOG message handler
	/// @param uMsg message code (expected to be WM_INITDIALOG)
	/// @param wParam Handle to the control to receive the default keyboard focus. The system assigns the default keyboard focus only if the dialog box procedure returns TRUE. 
	/// @param lParam Specifies additional initialization data. This data is passed to the system as the lParam parameter in a call to the CreateDialogIndirectParam, CreateDialogParam, DialogBoxIndirectParam, or DialogBoxParam function used to create the dialog box. For property sheets, this parameter is a pointer to the PROPSHEETPAGE structure used to create the page. This parameter is zero if any other dialog box creation function is used. 
	/// @param bHandled set to TRUE if message should not be further processed
	/// @return return value is obsolete
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	///  @param  wNotifyCode 
	///  @param  wID
	///  @param  hWndCtl 
	///  @return 0
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	///  @param  wNotifyCode 
	///  @param  wID
	///  @param  hWndCtl 
	///  @return 0
	LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};


