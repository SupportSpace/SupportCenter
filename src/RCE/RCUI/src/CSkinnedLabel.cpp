/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedLabel.cpp
///
///  Skinned label implementation
///
///  @author "Archer Software" Sogin M. @date 22.06.2007
///
////////////////////////////////////////////////////////////////////////

#include "CSkinnedLabel.h"
#include <AidLib/CScopedTracker/CScopedTracker.h>
#include <boost/bind.hpp>


// CSkinnedLabel
CSkinnedLabel::CSkinnedLabel(CSkinnedElement *parent)
	:	m_UnderMouse(false),
		CSkinnedElement(parent),
		m_textAlign(DT_LEFT)
{
TRY_CATCH
CATCH_LOG()
}

CSkinnedLabel::~CSkinnedLabel()
{
TRY_CATCH
	if (m_hFont)
		DeleteObject(m_hFont);
CATCH_LOG()
}

BOOL CSkinnedLabel::Attach(HWND hWnd)
{
TRY_CATCH

	ATLASSERT(::IsWindow(hWnd));
	BOOL bRet = CWindowImplEx<CSkinnedLabel, CStatic>::SubclassWindow(hWnd);
	TCHAR buf[MAX_PATH];

	/// Getting default text
	if (Text.empty())
	{
		if (FALSE == GetWindowText(buf, MAX_PATH))
		{
			Text = _T("Failed to get label text ");
			Log.WinError(_ERROR_,Text.c_str());
		}
		Text = buf;
	}

	/// Getting default font
	if (_tcscmp(Font.lfFaceName,NOFONT) == FALSE)
	{
		HFONT hFont = reinterpret_cast<HFONT>(SendMessage(WM_GETFONT, 0, 0));
		if (hFont != NULL)
		{
			LOGFONT font;
			if ( FALSE == GetObject(hFont, sizeof(LOGFONT), &font) )
				Log.WinError(_WARNING_,_T("Failed to get default font for label "));
			else
				Font = font;
		} else
			Log.WinError(_WARNING_,_T("Failed to get default font for label "));
	}

	/// Getting text alignment
	LONG style = GetWindowLong(GWL_STYLE);
	if (SS_CENTER == (style & SS_CENTER))
		TextAlign = DT_CENTER;
	else
	if (SS_RIGHT == (style & SS_RIGHT))
		TextAlign = DT_RIGHT;
	else
	if (SS_LEFT == (style & SS_LEFT))
		TextAlign = DT_LEFT;

	OnSkinChanged();
	return bRet;

CATCH_THROW()
}


LRESULT CSkinnedLabel::OnPaint(HDC hdc)
{
TRY_CATCH

	PAINTSTRUCT m_ps;
	HDC hDC(hdc ? hdc : (::BeginPaint(GetWindowHandle(), &m_ps)));
	RECT rc={0};
	GetWindowRect( &rc );	
	ScreenToClient( &rc );
	SetBkMode(hDC, TRANSPARENT);

	HFONT hFontOld;
	if (m_hFont)
		hFontOld = (HFONT)SelectObject(hDC,m_hFont);

	SetTextColor(hDC,TRUE==IsWindowEnabled()?FontColor1:FontColor2);
	DrawText( hDC , Text.c_str() , Text.length() , &rc , TextAlign );
	
	if (m_hFont)
		SelectObject(hDC,hFontOld);

	if (!hdc) ::EndPaint(GetWindowHandle(), &m_ps);

CATCH_THROW()
	return FALSE;
}

/// Repaints label and corresponding part of parent control
/// @param hDC corresponding device context
void CSkinnedLabel::Refresh(HDC &hDC)
{
TRY_CATCH

	if (Parent == NULL)
	{
		Invalidate(TRUE);
		return;
	}

	RECT rect;
	GetClientRect(&rect);
	
	//Redrawing parent element to MemDC
	CMemoryDC memDC(hDC, rect);
	
	OwnToParent(rect);

	//Drawing parent to mem DC
	Parent->DrawCurrentImage(memDC,0,0, rect);

	//Drawing self to meme DC
	OnPaint(memDC);

CATCH_THROW()
}

LRESULT CSkinnedLabel::OnEraseBkgnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	HDC hdc=reinterpret_cast<HDC>(wParam);
	PAINTSTRUCT m_ps;
	HDC hDC(hdc?hdc:(BeginPaint(&m_ps)));
	Refresh(hDC);
	if (!hdc) 
			EndPaint(&m_ps);
	return TRUE;
CATCH_THROW()
}

bool equalNewLine(TCHAR value)
{
    return _T('\n') == value;
}

/// Adjusts size of label to fit all text withing control
/// @param hDC corresponding device context
void CSkinnedLabel::AdjustSize(HDC &hDC)
{
TRY_CATCH

	HFONT hOldFont;
	if (m_hFont)
	{
		hOldFont = (HFONT)SelectObject(hDC,m_hFont);
	}

	SIZE size;
	GetTextExtentPoint32(hDC, Text.c_str(),Text.length(),&size);

	tstring str = Text;
	size.cy *= 1 + std::count_if(str.begin(), str.end(), equalNewLine);

	//SetSize(CSize(size));
	//MoveWindow(Location.x, Location.y, Size.cx, Size.cy, /*repaint*/ FALSE);
	
	//Releasing resources
	if (m_hFont)
		SelectObject(hDC,hOldFont);
	

CATCH_THROW()
}

void CSkinnedLabel::AdjustSize()
{
TRY_CATCH
	SIZE size = GetTextSize();
	SetWindowPos(NULL, 0,0, size.cx, size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
CATCH_THROW()
}

SIZE CSkinnedLabel::GetTextSize()
{
TRY_CATCH

	CScopedTracker<HDC> dc;
	dc.reset(GetDC(),boost::bind(&CSkinnedLabel::ReleaseDC, this, _1));
	
	HFONT hOldFont;
	if (m_hFont)
	{
		hOldFont = (HFONT)SelectObject(dc,m_hFont);
	}

	SIZE size;
	GetTextExtentPoint32(dc, Text.c_str(),Text.length(),&size);

	tstring str = Text;
	size.cy *= 1 + std::count_if(str.begin(), str.end(), equalNewLine);

	//Releasing resources
	if (m_hFont)
		SelectObject(dc,hOldFont);

	return size;

CATCH_THROW()
}

/// Notification from CSkinEngine after it applies skin attributes.
void CSkinnedLabel::OnSkinChanged()
{
TRY_CATCH

	//Creating font
	if (m_hFont)
		::DeleteObject(m_hFont);
	m_hFont = CreateFontIndirect(&Font);
	if (m_hFont == NULL)
		Log.Add(_WARNING_,_T("CSkinnedLabel::OnSkinChanged: Failed to CreateFontIndirect"));

	//Retriving paint DC (note not using CPaintDC, since CPaintDC calls BeginPaint
	//which inturn must be called only from WM_PAINT, WM_ERASEBGRND message handler
	HDC hDC = ::GetDC( GetWindowHandle() );

	//Refreshing parent
	Refresh(hDC);

	//Adjusting size;
	AdjustSize(hDC);

	//Refreshing parent
	Refresh(hDC);

	//Releasing resources
	ReleaseDC(hDC);

CATCH_THROW()
}


/// Left button down message handler
LRESULT CSkinnedLabel::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	SetCapture();
	bHandled = TRUE;
	return FALSE;

CATCH_THROW()
}

/// Left button up message handler
LRESULT CSkinnedLabel::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	if ( GetCapture() == m_hWnd )
		ReleaseCapture();

	// TODO: call events here

	bHandled = TRUE;
	return FALSE;

CATCH_THROW()
}

/// Left button click message handler
LRESULT CSkinnedLabel::OnLButronDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	// TODO: call events here

	bHandled = TRUE;
	return FALSE;

CATCH_THROW()
}

/// WM_NCHITTEST message handler
LRESULT CSkinnedLabel::OnNChitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	bHandled = TRUE;
	return HTCLIENT;

CATCH_THROW()
}

UINT CSkinnedLabel::GetTextAlign() const
{
TRY_CATCH
	return m_textAlign;
CATCH_THROW()
}

void CSkinnedLabel::SetTextAlign(UINT newVal)
{
TRY_CATCH
	m_textAlign = newVal;
CATCH_THROW()
}

LRESULT CSkinnedLabel::OnSetCursor(HWND hWnd, UINT, UINT)
{
TRY_CATCH
	SetMsgHandled(FALSE);
	return FALSE;
CATCH_THROW()
}
