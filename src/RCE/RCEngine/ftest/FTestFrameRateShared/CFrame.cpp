/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFrame.cpp
///
///  Implements CFrame class, responsible for one frame
///
///  @author Dmitry Netrebenko @date 14.05.2007
///
////////////////////////////////////////////////////////////////////////

#include "CFrame.h"
#include <AidLib/CSingleton/CSingleton.h>
#include "CSettings.h"
#include "CBlockBitmap.h"
#include <AidLib/CException/CException.h>
#include <numeric>

CFrame::CFrame()
	:	m_bitmap(NULL)
	,	m_bitDC(NULL)
{
TRY_CATCH
CATCH_THROW()
}

CFrame::~CFrame()
{
TRY_CATCH

	/// Destroy bitmap
	if(m_bitmap)
		DeleteObject(m_bitmap);
	/// Destroy DC
	if(m_bitDC)
		DeleteDC(m_bitDC);

CATCH_LOG()
}

void CFrame::Draw()
{
TRY_CATCH

	if(SETTINGS_INSTANCE.GetCreateFrameBitmaps())
	{
		int width = m_boundRect->right - m_boundRect->left;
		int height = m_boundRect->bottom - m_boundRect->top;
		HDC dc = SETTINGS_INSTANCE.GetDC();

		/// Draw frame
		BitBlt(dc, m_boundRect->left, m_boundRect->top, width, height, m_bitDC, 0, 0, SRCCOPY);
	}

CATCH_THROW()
}

void CFrame::Init(const Rects& rects, const ControlPoints& points, SPRect prevFrameRect)
{
TRY_CATCH

	bool createBitmaps = SETTINGS_INSTANCE.GetCreateFrameBitmaps();
	bool createPoints = SETTINGS_INSTANCE.GetCreateControlPoints();
	/// Get DC of the block bitmap
	HDC dc = BLOCKBITMAP_INSTANCE.GetDC();

	if(createBitmaps)
	{
		m_boundRect.reset(new RECT());
		m_boundRect = std::accumulate(rects.begin(), rects.end(), m_boundRect, RectComposition());

		m_boundRect = RectComposition()(m_boundRect, prevFrameRect);

		int width = m_boundRect->right - m_boundRect->left;
		int height = m_boundRect->bottom - m_boundRect->top;

		/// Create DC
		m_bitDC = CreateCompatibleDC(dc);
		if(!m_bitDC)
			throw MCException_Win(_T("Can not create compatible DC"));

		/// Create bitmap
		m_bitmap = CreateCompatibleBitmap(dc, width, height);
		if(!m_bitmap)
			throw MCException_Win(_T("Can not create bitmap"));

		/// Select bitmap
		SelectObject(m_bitDC, m_bitmap);

		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = width;
		rect.bottom = height;
		/// Fill frame bitmap
		HBRUSH brush = CreateSolidBrush(RGB(255,255,255));
		FillRect(m_bitDC, &rect, brush);
		DeleteObject(brush);

		Rects::const_iterator index;
		for(index = rects.begin(); index != rects.end(); ++index)
		{
			SPRect rect = *index;
			int x = rect->left - m_boundRect->left;
			int y = rect->top - m_boundRect->top;
			int width = rect->right - rect->left;
			int height = rect->bottom - rect->top;

			/// Draw block bitmap on frame
			BitBlt(m_bitDC, x, y, width, height, dc, 0, 0, SRCCOPY);
		}

	}

	/// Store control points
	if(createPoints)
		m_controlPoints = points;

CATCH_THROW()
}

ControlPoints& CFrame::GetControlPoints()
{
TRY_CATCH

	return m_controlPoints;

CATCH_THROW()
}

SPRect CFrame::GetBoundRect() const
{
TRY_CATCH

	return m_boundRect;

CATCH_THROW()
}
