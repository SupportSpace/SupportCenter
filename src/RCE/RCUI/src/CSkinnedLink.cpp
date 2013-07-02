/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedLink.cpp
///
///  Skinned Link implementation
///
///  @author "Archer Software" Sogin M. @date 19.12.2007
///
////////////////////////////////////////////////////////////////////////

#include "CSkinnedLink.h"

// CSkinnedLink
CSkinnedLink::CSkinnedLink(CSkinnedElement *parent)
	:	CSkinnedLabel(parent)
{
TRY_CATCH
CATCH_LOG()
}

CSkinnedLink::~CSkinnedLink()
{
TRY_CATCH
CATCH_LOG()
}

LRESULT CSkinnedLink::OnMouseMove(UINT nFlags, CPoint point)
{
TRY_CATCH

	if (m_UnderMouse) 
		return FALSE;
	m_UnderMouse = true;
	TRACKMOUSEEVENT tm = { sizeof( TRACKMOUSEEVENT ) , TME_LEAVE, m_hWnd , 1 };
	
	LOGFONT font1 = Font;
	Font = Font2;
	Font2 = font1;
	COLORREF color1 = FontColor1;
	FontColor1 = BkColor1;
	BkColor1 = color1;	

	OnSkinChanged();

	::TrackMouseEvent( &tm );

	return 0;

CATCH_THROW()
}

LRESULT CSkinnedLink::OnMouseLeave(void)
{
	m_UnderMouse = false;

	LOGFONT font1 = Font;
	Font = Font2;
	Font2 = font1;
	COLORREF color1 = FontColor1;
	FontColor1 = BkColor1;
	BkColor1 = color1;	

	OnSkinChanged();

	TRACKMOUSEEVENT tm = { sizeof( TRACKMOUSEEVENT ) , TME_HOVER, m_hWnd , 1 };
	return ::TrackMouseEvent( &tm );
}

LRESULT CSkinnedLink::OnSetCursor(HWND hWnd, UINT, UINT)
{
TRY_CATCH
	
	::SetCursor(LoadCursor(NULL, m_UnderMouse?MAKEINTRESOURCE(32649/*IDC_HAND*/):IDC_ARROW));
	SetMsgHandled(TRUE);
	return TRUE;

CATCH_THROW()
}