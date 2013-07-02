/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedPanel.cpp
///
///  Skinned Panel implementation
///
///  @author "Archer Software" Sogin M. @date 18.12.2007
///
////////////////////////////////////////////////////////////////////////

#include "CSkinnedPanel.h"
#include <AidLib/CScopedTracker/CScopedTracker.h>
#include <boost/bind.hpp>

// CSkinnedPanel
CSkinnedPanel::CSkinnedPanel(const int imageId,
							 const int headImageId,
							 const int tailImageId,
							 CSkinnedElement *parent)
	:	m_UnderMouse(false),
		CSkinnedElement(parent),
		m_textAlign(DT_LEFT),
		m_headImage(NULL),
		m_tailImage(NULL),
		m_middleImage(NULL)
{
TRY_CATCH
	if (NULL == m_skinsImageList.get())
	{
		m_skinsImageList.reset(new CSkinsImageList());
	}
	m_skinsImageList->ImageFromRes(&m_middleImage,1,&imageId);
	if (-1 != headImageId)
		m_skinsImageList->ImageFromRes(&m_headImage,1,&headImageId);
	if (-1 != tailImageId)
		m_skinsImageList->ImageFromRes(&m_tailImage,1,&tailImageId);
	m_currentImage = new CImage();
	m_currentImage->Create(1, 1, 24 /*full colors*/);
CATCH_LOG()
}

CSkinnedPanel::~CSkinnedPanel()
{
TRY_CATCH
	if (m_hFont)
		DeleteObject(m_hFont);
	if (m_currentImage)
		delete m_currentImage;
CATCH_LOG()
}

BOOL CSkinnedPanel::Attach(HWND hWnd)
{
TRY_CATCH

	ATLASSERT(::IsWindow(hWnd));
	BOOL bRet = CWindowImplEx<CSkinnedPanel, CStatic>::SubclassWindow(hWnd);
	SetWindowLong(GWL_STYLE, WS_CLIPCHILDREN | GetWindowLong(GWL_STYLE));
	OnSkinChanged();
	return bRet;

CATCH_THROW()
}

void CSkinnedPanel::OnSkinChanged()
{
TRY_CATCH
	RECT rc;
	GetWindowRect(&rc);
	OnSize(0,CSize(rc.right - rc.left, rc.bottom - rc.top));
	Invalidate(TRUE);
CATCH_THROW()
}

void CSkinnedPanel::OnSize(UINT, CSize& size)
{
TRY_CATCH

	CSize middleImageSize(size);

	/// Stretching image to memory DC
	if (size.cx <=0 || size.cy <=0)
		return;

	m_currentImage->Destroy();
	if (FALSE == m_currentImage->Create(size.cx, size.cy, 24 /*full colors*/))
		throw MCException_Win("Failed to m_currentImage->Create");

	CScopedTracker<HDC> hdc;
	hdc.reset(m_currentImage->GetDC(), boost::bind(&CImage::ReleaseDC, m_currentImage));

	int middleImageTop = 0;

	if (m_headImage && m_headImage->GetHeight() && size.cx)
	{
		if (FALSE == m_headImage->StretchBlt(hdc.get(), 0,0, size.cx, m_headImage->GetHeight(), SRCCOPY))
			Log.WinError(_WARNING_,_T("Failed to m_headImage->StretchBlt"));
		middleImageSize.cy -= m_headImage->GetHeight();
		middleImageTop += m_headImage->GetHeight();
	}

	if (m_tailImage)
	{
		middleImageSize.cy -= m_tailImage->GetHeight();
	}

	if (m_middleImage && middleImageSize.cx > 0 && middleImageSize.cy > 0)
		if (FALSE == m_middleImage->StretchBlt(hdc.get(), 0,middleImageTop, middleImageSize.cx, middleImageSize.cy, SRCCOPY))
			Log.WinError(_WARNING_,_T("Failed to m_middleImage->StretchBlt"));

	if (m_tailImage && m_tailImage->GetHeight() > 0 && size.cx > 0)
	{
		if (FALSE == m_tailImage->StretchBlt(hdc.get(), 0, size.cy - m_tailImage->GetHeight(), size.cx, m_tailImage->GetHeight(), SRCCOPY))
			Log.WinError(_WARNING_,_T("Failed to m_tailImage->StretchBlt"));
	}

CATCH_THROW()
}