/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedLinkEx.cpp
///
///  Skinned Link implementation
///
///  @author "Archer Software" Sogin M. @date 20.12.2007
///
////////////////////////////////////////////////////////////////////////

#include "CSkinnedLinkEx.h"
#include <AidLib/CScopedTracker/CScopedTracker.h>
#include <boost/bind.hpp>

// CSkinnedLinkEx
CSkinnedLinkEx::CSkinnedLinkEx(	const int regularImageId,
								const int mouseOverImageId,
								CSkinnedElement *parent,
								const UINT imgAlign)
	:	CSkinnedLink(parent), 
		CSkinnedLabel(parent),
		m_imgAlign(imgAlign)
{
TRY_CATCH
	
	if (NULL == m_skinsImageList.get())
	{
		m_skinsImageList.reset(new CSkinsImageList());
	}
	m_skinsImageList->ImageFromRes(&m_regularImage,1,&regularImageId);
	m_skinsImageList->ImageFromRes(&m_mouseOverImage,1,&mouseOverImageId);
	m_currentImage = m_UnderMouse?m_mouseOverImage:m_regularImage;
	
CATCH_LOG()
}

void CSkinnedLinkEx::SetImages(	const int regularImageId, const int mouseOverImageId )
{
TRY_CATCH

	if (NULL == m_skinsImageList.get())
	{
		m_skinsImageList.reset(new CSkinsImageList());
	}
	m_skinsImageList->ImageFromRes(&m_regularImage,1,&regularImageId);
	m_skinsImageList->ImageFromRes(&m_mouseOverImage,1,&mouseOverImageId);
	m_currentImage = m_UnderMouse?m_mouseOverImage:m_regularImage;
	OnSkinChanged();

CATCH_THROW()
}

CSkinnedLinkEx::~CSkinnedLinkEx()
{
TRY_CATCH
CATCH_LOG()
}

LRESULT CSkinnedLinkEx::OnPaint(HDC hdc)
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

	m_currentImage = m_UnderMouse?m_mouseOverImage:m_regularImage;
	SetTextColor(hDC,TRUE==IsWindowEnabled()?FontColor1:FontColor2);
	switch(m_imgAlign)
	{
		case DT_LEFT:
			{
				// Drawing image
				SIZE textSize = CSkinnedLink::GetTextSize();				
				RECT rectDest;
				rectDest.left = 0;
				rectDest.top = (textSize.cy - m_currentImage->GetHeight())/2;
				rectDest.right = rectDest.left + m_currentImage->GetWidth();
				rectDest.bottom = rectDest.top + m_currentImage->GetHeight();
				if (RectIsntEmpty(rectDest))
					m_currentImage->Draw( hDC, rectDest );

				rc.left += m_currentImage->GetWidth() + TEXT_IMAGE_SPACE;
				// Drawing text				
				DrawText( hDC , Text.c_str() , Text.length() , &rc , TextAlign );

			}
			break;
		case DT_RIGHT:
		default:
			{
				// Drawing text
				DrawText( hDC , Text.c_str() , Text.length() , &rc , TextAlign );

				// Drawing image
				SIZE textSize = CSkinnedLink::GetTextSize();
				RECT rectDest;
				rectDest.left = textSize.cx + TEXT_IMAGE_SPACE;
				rectDest.top = (textSize.cy - m_currentImage->GetHeight())/2;
				rectDest.right = rectDest.left + m_currentImage->GetWidth();
				rectDest.bottom = rectDest.top + m_currentImage->GetHeight();
				if (RectIsntEmpty(rectDest))
					m_currentImage->Draw( hDC, rectDest );
			}
			break;
	}	
	if (m_hFont)
		SelectObject(hDC,hFontOld);

	if (!hdc) ::EndPaint(GetWindowHandle(), &m_ps);

CATCH_THROW()
	return FALSE;
}

SIZE CSkinnedLinkEx::GetTextSize()
{
TRY_CATCH


	SIZE size = CSkinnedLink::GetTextSize();
	if (NULL != m_currentImage)
		size.cx += (TEXT_IMAGE_SPACE*2 + m_currentImage->GetWidth());

	return size;

CATCH_THROW()
}
