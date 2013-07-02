/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedButton.cpp
///
///  Skinned Button implementation
///
///  @author "Archer Software" Sogin M. @date 22.06.2007
///
////////////////////////////////////////////////////////////////////////

#include "CSkinnedButton.h"

// CSkinnedButton
CSkinnedButton::CSkinnedButton(	CSkinnedElement *parent,
								const int regularImageId,
								const int disabledImageId,
								const int pressedImageId,
								const int mouseOverImageId )
	:	m_UnderMouse(false),
		CSkinnedElement(parent),
		m_textAlign(DT_CENTER)
{
TRY_CATCH
	m_border = GetSystemMetrics(SM_CXBORDER) * 4;
	if (NULL == m_skinsImageList.get())
	{
		m_skinsImageList.reset(new CSkinsImageList());
	}
	if (NULL != m_skinsImageList.get())
	{
		m_skinsImageList->ImageFromRes(&m_regularImage,1,&regularImageId);
		m_skinsImageList->ImageFromRes(&m_mouseOverImage,1,&mouseOverImageId);
		m_skinsImageList->ImageFromRes(&m_pressedImage,1,&pressedImageId);
		m_skinsImageList->ImageFromRes(&m_disabledImage,1,&disabledImageId);
		m_currentImage = m_regularImage;
	} 
	else
		Log.Add(_WARNING_,_T("NULL images list while loading control"));
CATCH_LOG()
}

/// Set up state images
void CSkinnedButton::SetImages(	const int regularImageId,
								const int disabledImageId,
								const int pressedImageId,
								const int mouseOverImageId )
{
TRY_CATCH
	bool pressed = false;
	if (m_currentImage == m_pressedImage)
		pressed = true;
	if (NULL != m_skinsImageList.get())
	{
		m_skinsImageList->ImageFromRes(&m_regularImage,1,&regularImageId);
		m_skinsImageList->ImageFromRes(&m_mouseOverImage,1,&mouseOverImageId);
		m_skinsImageList->ImageFromRes(&m_pressedImage,1,&pressedImageId);
		m_skinsImageList->ImageFromRes(&m_disabledImage,1,&disabledImageId);
	}
	else
		Log.Add(_WARNING_,_T("NULL images list while setting images for control"));
	DRAWITEMSTRUCT ds;
	memset(&ds,0,sizeof(ds));
	if (pressed)
		ds.itemState = ODS_SELECTED;
	if (!IsWindowEnabled())
		ds.itemState |= ODS_DISABLED;
	ds.hDC = GetWindowDC();
	OnDrawItem(0,&ds);
	OnSkinChanged();
	ReleaseDC(ds.hDC);
CATCH_THROW()
}


CSkinnedButton::~CSkinnedButton()
{
TRY_CATCH
	if (m_hFont)
		DeleteObject(m_hFont);
CATCH_THROW()
}

BOOL CSkinnedButton::Attach(HWND hWnd)
{
TRY_CATCH

	ATLASSERT(::IsWindow(hWnd));
	BOOL bRet = CWindowImplEx<CSkinnedButton, CButton>::SubclassWindow(hWnd);
	ModifyStyle(0,BS_OWNERDRAW);

	/// Getting default text
	TCHAR buf[MAX_PATH];
		if (Text.empty())
	{
		if (FALSE == GetWindowText(buf, MAX_PATH))
		{
			Text = _T("Failed to get Button text ");
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
				Log.WinError(_WARNING_,_T("Failed to get default font for Button "));
			else
				Font = font;
		} else
			Log.WinError(_WARNING_,_T("Failed to get default font for Button "));
	}

	/// Getting text alignment
	LONG style = GetWindowLong(GWL_STYLE);
	if (BS_CENTER == (style & BS_CENTER))
		TextAlign = DT_CENTER;
	else
	if (BS_RIGHT == (style & BS_RIGHT))
		TextAlign = DT_RIGHT;
	else
	if (BS_LEFT == (style & BS_LEFT))
		TextAlign = DT_LEFT;

	OnSkinChanged();
	return bRet;

CATCH_THROW()
}

void CSkinnedButton::OnDrawItem(UINT idCtl, LPDRAWITEMSTRUCT lpdis)
{
TRY_CATCH
	CImage *prevImage = m_currentImage;
	if (ODS_DISABLED & lpdis->itemState)
	{
		m_currentImage = m_disabledImage;
	} else
	if (ODS_SELECTED & lpdis->itemState)
	{
		m_currentImage = m_pressedImage;
	} else
	{
		m_currentImage = m_UnderMouse?m_mouseOverImage:m_regularImage;
	}
	if (prevImage != m_currentImage)
		AdjustSize(lpdis->hDC);
	Refresh(lpdis->hDC);
CATCH_THROW()
}

LRESULT CSkinnedButton::OnPaint(HDC hdc)
{
TRY_CATCH

	PAINTSTRUCT m_ps;
	HDC hDC(hdc?hdc:(::BeginPaint(GetWindowHandle(), &m_ps)));

	if (NULL != m_currentImage)
	{
		m_currentImage->Draw(hDC,0,0);
	}
		
	RECT rc={0};
	GetClientRect( &rc );
	rc.top += m_border;
	rc.left += m_border*2;
	rc.bottom -= m_border;
	rc.right -= m_border*2;

	SetBkMode(hDC, TRANSPARENT);

	HFONT hFontOld;
	if (m_hFont)
		hFontOld = (HFONT)SelectObject(hDC,m_hFont);


	SetTextColor(hDC,TRUE==IsWindowEnabled()?FontColor1:FontColor2);
	DrawText( hDC , Text.c_str() , Text.length() , &rc , TextAlign | DT_VCENTER | DT_SINGLELINE);
	
	if (m_hFont)
		SelectObject(hDC,hFontOld);

	if (!hdc) ::EndPaint(GetWindowHandle(), &m_ps);

CATCH_THROW()
	return FALSE;
}

/// Repaints Button and corresponding part of parent control
/// @param hDC corresponding device context
void CSkinnedButton::Refresh(HDC &hDC)
{
TRY_CATCH

	if (Parent == NULL)
	{
		//Invalidate(TRUE);
		OnPaint(hDC);
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

/// Adjusts size of Button to fit all text withing control
/// @param hDC corresponding device context
void CSkinnedButton::AdjustSize(HDC &hDC)
{
TRY_CATCH

	if (NULL == m_currentImage)
		return;
	RECT origRect;
	GetClientRect(&origRect);
	int cx = m_currentImage->GetWidth();
	int cy = m_currentImage->GetHeight();
	if (cx != (origRect.right - origRect.left)
		||
		cy != (origRect.bottom - origRect.top))
	{
		Refresh(hDC);
		SetWindowPos(0,0,0,cx,cy, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOREPOSITION | SWP_NOZORDER);
	}

CATCH_THROW()
}


/// Notification from CSkinEngine after it applies skin attributes.
void CSkinnedButton::OnSkinChanged()
{
TRY_CATCH

	//Creating font
	if (m_hFont)
		DeleteObject(m_hFont);
	m_hFont = CreateFontIndirect(&Font);
	if (m_hFont == NULL)
		Log.Add(_WARNING_,_T("CSkinnedButton::OnSkinChanged: Failed to CreateFontIndirect"));

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
LRESULT CSkinnedButton::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	//SetCapture();
	//bHandled = TRUE;
	//return FALSE;
	
	SetCapture();
	return 0;

CATCH_THROW()
}

/// Left button up message handler
LRESULT CSkinnedButton::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	//if ( GetCapture() == m_hWnd )
	//	ReleaseCapture();

	//// TODO: call events here

	//bHandled = TRUE;
	//return FALSE;
	
	if ( m_hWnd == ::GetCapture() )
	{
		ReleaseCapture();

		// To prevent a button click message when a cursor is out of the button
		POINT pt = {LOWORD(lParam),HIWORD(lParam)};
		RECT rc;
		GetClientRect(&rc);

		if ( PtInRect(&rc,pt) )
			::SendMessage(GetParent(),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(),BN_CLICKED),(LPARAM)m_hWnd);
	}

CATCH_THROW()
}

/// Left button click message handler
LRESULT CSkinnedButton::OnLButronDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	// TODO: call events here

	bHandled = TRUE;
	return FALSE;

CATCH_THROW()
}

/// WM_NCHITTEST message handler
LRESULT CSkinnedButton::OnNChitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	bHandled = TRUE;
	return HTCLIENT;

CATCH_THROW()
}

UINT CSkinnedButton::GetTextAlign() const
{
TRY_CATCH
	return m_textAlign;
CATCH_THROW()
}

void CSkinnedButton::SetTextAlign(UINT newVal)
{
TRY_CATCH
	m_textAlign = newVal;
CATCH_THROW()
}
