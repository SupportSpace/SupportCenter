/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedLinkEx.h
///
///  Skinned Link implementation
///
///  @author "Archer Software" Sogin M. @date 20.12.2007
///
////////////////////////////////////////////////////////////////////////<summary>

#pragma once

// CSkinnedLinkEx
#include "CSkinnedLink.h"

/// Empty space between image and text
#define TEXT_IMAGE_SPACE 2

/// Skinned Link control with image implementation 
class CSkinnedLinkEx :
	public virtual CSkinnedLink
{
protected:
	/// Images
	CImage *m_regularImage;
	CImage *m_mouseOverImage;

	/// Image align
	UINT m_imgAlign;

	/// WM_PAINT message handler
	/// @param  hdc device context for drawing if not NULL
	/// @return return value is obsolete
	virtual LRESULT OnPaint(HDC hdc);

public:
	/// ctor
	/// @param parent pointer to parent control, that is needed since Link is redrawn in pair with it's parent
	CSkinnedLinkEx(	const int regularImageId,
					const int mouseOverImageId,
					CSkinnedElement *parent,
					const UINT imgAlign = DT_RIGHT);

	/// class destructor
	virtual ~CSkinnedLinkEx();

	/// Set up state images
	void SetImages(	const int regularImageId,
					const int mouseOverImageId );

	/// Returns link, in couple with image size
	SIZE GetTextSize();
};


