/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedMsgBox.cpp
///
///  Skinned MsgBox implementation
///
///  @author "Archer Software" Sogin M. @date 26.12.2007
///
////////////////////////////////////////////////////////////////////////

#include "CSkinnedMsgBox.h"
#include <AidLib/CScopedTracker/CScopedTracker.h>
#include <boost/bind.hpp>
#include <boost/weak_ptr.hpp>

#define OUTER_CX 0
#define OUTER_CY 0

std::map<int, INT_PTR> CSkinnedMsgBox::m_defaultList;

///TODO: fix cruch (memory leack) with shared_ptr
CSkinnedMsgBox::CSkinnedMsgBox(CSkinnedElement *parent,  const bool showCancel, const bool showCheckBox)
	:	
		CAbstractCommandManager(parent),
		m_showCancel(showCancel),
		m_showCheckBox(showCheckBox),
		m_doNotShowAgain(false),
		m_autoBreakText(false),
		m_modal(false),
		m_textLabel(this),
		m_showAgainCheckBox(0, *new boost::shared_ptr<CAbstractCommandManager>(this), IDR_CHB_UNCHECKED_REGULAR, IDR_CHB_UNCHECKED_MOUSEOVER, this, DT_LEFT),
		m_okBtn(this, IDR_BTN_REGULAR_72_22, IDR_BTN_DISABLED_72_22, IDR_BTN_PRESSED_72_22, IDR_BTN_MOUSEOVER_72_22),
		m_cancelBtn(this, IDR_BTN_REGULAR_72_22, IDR_BTN_DISABLED_72_22, IDR_BTN_PRESSED_72_22, IDR_BTN_MOUSEOVER_72_22)
{
TRY_CATCH
	m_skinsImageList.reset(new CSkinsImageList());
	int imageId = IDR_MSG_BOX_DIALOG;
	m_skinsImageList->ImageFromRes(&m_currentImage, 1, &imageId);
CATCH_LOG()
}

CSkinnedMsgBox::~CSkinnedMsgBox()
{
TRY_CATCH
CATCH_LOG()
}

LRESULT CSkinnedMsgBox::OnNChitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	bHandled = TRUE;
	RECT rc;
	GetWindowRect(&rc);
	if (HIWORD(lParam) < rc.top + CAPTION_HEIGHT + OUTER_CY)
		return HTCAPTION;
	return HTCLIENT;

CATCH_THROW()
}

LRESULT CSkinnedMsgBox::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	//+create fonts
	HFONT font = GetFont();
	ATLASSERT(::GetObjectType(font) == OBJ_FONT);
	LOGFONT logFont;
	GetObject(font, sizeof(LOGFONT),&logFont);
	_tcscpy_s(logFont.lfFaceName, LF_FACESIZE, _T("Verdana")); 
	logFont.lfHeight = -11;
	Font = logFont;
	LOGFONT boldFont = Font;
	boldFont.lfWeight = FW_BOLD;
	LOGFONT underlinedFont = Font;
	underlinedFont.lfUnderline = TRUE;
	LOGFONT underlinedBoldFont = boldFont;
	underlinedBoldFont.lfUnderline = TRUE;
	LOGFONT smallFont = Font;
	smallFont.lfHeight = -11;
	//-create fonts

	/// Attaching controls
	m_okBtn.Attach(GetDlgItem(IDOK));
	m_okBtn.Font = boldFont;
	m_okBtn.FontColor1 = SCVUI_LBLFONTCOLOR1;
	m_okBtn.OnSkinChanged();

	m_cancelBtn.Attach(GetDlgItem(IDCANCEL));
	m_cancelBtn.Font = boldFont;
	m_cancelBtn.FontColor1 = SCVUI_LBLFONTCOLOR1;
	m_cancelBtn.OnSkinChanged();

	m_textLabel.Attach(GetDlgItem(IDC_TEXT_LBL));
	m_textLabel.Font = boldFont;
	m_textLabel.FontColor1 = SCVUI_LBLFONTCOLOR1;
	m_textLabel.FontColor2 = SCVUI_LBLFONTCOLOR1;
	m_textLabel.SetText(m_text);
	m_textLabel.OnSkinChanged();

	m_showAgainCheckBox.Attach(GetDlgItem(IDC_SHOWAGAIN_CHBOX));
	m_showAgainCheckBox.Font = smallFont;
	m_showAgainCheckBox.Font2 = smallFont;
	m_showAgainCheckBox.FontColor1 = SCVUI_BTNFONTCOLOR2;
	m_showAgainCheckBox.FontColor2 = SCVUI_LBLFONTCOLOR1;
	m_showAgainCheckBox.BkColor1 = SCVUI_LBLFONTCOLOR1;
	m_showAgainCheckBox.SetText(_T("Don't show me this dialog again"));
	m_showAgainCheckBox.OnSkinChanged();

	if (!m_showCheckBox)
		m_showAgainCheckBox.ShowWindow(SW_HIDE);

	if (!m_showCancel)
		m_cancelBtn.ShowWindow(SW_HIDE);
	
	//Proper window show happened here
	OnSkinChanged();

	return TRUE;

CATCH_THROW()
}

void CSkinnedMsgBox::OnSkinChanged()
{
TRY_CATCH

	if (NULL == m_currentImage)
		return;
	int cx = m_currentImage->GetWidth();
	int cy = m_currentImage->GetHeight();
	SetWindowPos(0, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER );
	FormShape(*this, UPPER_LEFT);
		
	// Aligning controls
	#define BTN_MARGIN 15
	#define BTN_INTERVAL 8

	int buttonWidth = m_cancelBtn.m_currentImage->GetWidth();
	int buttonHeight = m_cancelBtn.m_currentImage->GetHeight();

	m_cancelBtn.SetWindowPos(	0,
								cx - buttonWidth - BTN_MARGIN,
								cy - buttonHeight - BTN_MARGIN,
								0,0, SWP_NOSIZE | SWP_NOZORDER );

	if (m_showCancel)
		m_okBtn.SetWindowPos(	0,
								cx - buttonWidth *2 - BTN_MARGIN - BTN_INTERVAL,
								cy - buttonHeight - BTN_MARGIN,
								0,0, SWP_NOSIZE | SWP_NOZORDER );
	else
		m_okBtn.SetWindowPos(	0,
								(cx - OUTER_CX - buttonWidth)/2 + OUTER_CX,
								cy - buttonHeight - BTN_MARGIN,
								0,0, SWP_NOSIZE | SWP_NOZORDER );

	if (NULL != Parent)
	{
		RECT parentRC;
		::GetWindowRect(Parent->WindowHandle, &parentRC);
		RECT rc;
		GetWindowRect(&rc);
		POINT pt;
		pt.x = parentRC.left + (parentRC.right - parentRC.left - (rc.right - rc.left))/2;
		pt.y = parentRC.top + (parentRC.bottom - parentRC.top - (rc.bottom - rc.top ))/2 - OUTER_CY;
		//ScreenToClient(&pt);
		SetWindowPos(0, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	RECT rc;

	m_showAgainCheckBox.GetWindowRect(&rc);
	rc.left += OUTER_CX;
	rc.right += OUTER_CX;
	rc.top += OUTER_CY;
	rc.bottom += OUTER_CY;
	ScreenToClient(&rc);
	m_showAgainCheckBox.MoveWindow(&rc);

	if ( m_autoBreakText )
		m_textLabel.SetTextAlign( m_textLabel.GetTextAlign() | DT_WORDBREAK );
	m_textLabel.GetWindowRect(&rc);
	rc.left += OUTER_CX;
	rc.right += OUTER_CX;
	rc.top += OUTER_CY;
	rc.bottom += OUTER_CY;
	ScreenToClient(&rc);
	m_textLabel.MoveWindow(&rc);

	SetWindowText(_T("SupportSpace alert"));

CATCH_THROW()
}

LRESULT CSkinnedMsgBox::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
TRY_CATCH
	ConditionalClose(IDCANCEL);
	return 0;
CATCH_THROW()
}

LRESULT CSkinnedMsgBox::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
TRY_CATCH
	ConditionalClose(IDOK);
	return 0;
CATCH_THROW()
}

void CSkinnedMsgBox::ExecuteCommand(const CCommandProxy* sender, EWidgetCommand command, const void* commandData)
{
TRY_CATCH
	
	//if (this->m == sender)
	switch(command)
	{
		case cmd_ButtonClick:
			m_doNotShowAgain = !m_doNotShowAgain;
			m_showAgainCheckBox.SetImages(m_doNotShowAgain?IDR_CHB_CHECKED_REGULAR:IDR_CHB_UNCHECKED_REGULAR,m_doNotShowAgain?IDR_CHB_CHECKED_MOUSEOVER:IDR_CHB_UNCHECKED_MOUSEOVER);
			m_showAgainCheckBox.OnSkinChanged();
			break;
	}
CATCH_THROW()
}

INT_PTR CSkinnedMsgBox::Show(const int type, const tstring& text, bool autoBreakText)
{
TRY_CATCH
	if (m_defaultList.end() != m_defaultList.find(type))
		return m_defaultList.find(type)->second;
	m_autoBreakText	= autoBreakText;
	m_text			= text;
	m_modal = true;
	INT_PTR result = DoModal();
	if (m_doNotShowAgain && result == IDOK)
		m_defaultList[type] = result;
	return result;
CATCH_THROW()
}

