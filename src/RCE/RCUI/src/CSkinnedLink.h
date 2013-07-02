/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedLink.h
///
///  Skinned Link implementation
///
///  @author "Archer Software" Sogin M. @date 19.12.2007
///
////////////////////////////////////////////////////////////////////////<summary>

#pragma once

// CSkinnedLink
#include "CSkinnedLabel.h"

/// Skinned Link control implementation
class CSkinnedLink :
	public virtual CSkinnedLabel
{
protected:
	/// WM_MOUSEMOVE message handler
	/// @param Flags Indicates the mouse buttons and keys that the user pressed
	/// @param Pt mouse pointer coords
	/// @return return value is obsolete
	virtual LRESULT OnMouseMove(UINT nFlags, CPoint point);
	
	/// WM_MOUSELEAVE message handler
	/// @return return value is obsolete
	virtual LRESULT OnMouseLeave(void);

	/// WM_SETCURSOR message handler
	virtual LRESULT OnSetCursor(HWND hWnd, UINT, UINT);

public:
	/// ctor
	/// @param parent pointer to parent control, that is needed since Link is redrawn in pair with it's parent
	CSkinnedLink(CSkinnedElement *parent);
	/// class destructor
	virtual ~CSkinnedLink();
};


