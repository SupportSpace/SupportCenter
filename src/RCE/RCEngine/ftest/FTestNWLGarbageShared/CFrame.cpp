/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFrame.cpp
///
///  Implements CFrame class, responsible for frame 
///
///  @author Dmitry Netrebenko @date 07.06.2007
///
////////////////////////////////////////////////////////////////////////

#include "CFrame.h"
#include <AidLib/CException/CException.h>
#include "CSettings.h"
#include <AidLib/CSingleton/CSingleton.h>

CFrame::CFrame()
{
TRY_CATCH
CATCH_THROW()
}

CFrame::~CFrame()
{
TRY_CATCH
CATCH_LOG()
}

void CFrame::Draw()
{
TRY_CATCH

	int width = SETTINGS_INSTANCE.GetWindowWidth();
	int height = SETTINGS_INSTANCE.GetWindowHeight();
	HDC dc = SETTINGS_INSTANCE.GetDC();

	/// Draw frame
	if(m_bitDC.get())
		BitBlt(dc, 0, 0, width, height, m_bitDC.get(), 0, 0, SRCCOPY);

CATCH_THROW()
}

void CFrame::Init(bool firstFilled)
{
TRY_CATCH

	int width = SETTINGS_INSTANCE.GetWindowWidth();
	int height = SETTINGS_INSTANCE.GetWindowHeight();
	HDC dc = SETTINGS_INSTANCE.GetDC();
	int cols = SETTINGS_INSTANCE.GetColCount();
	int rows = SETTINGS_INSTANCE.GetRowCount();
	int blockWidth = width / cols;
	int blockHeight = height / rows;

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = height;

	m_bitDC.reset(CreateCompatibleDC(dc), DeleteDC);
	if(!m_bitDC.get())
		throw MCException_Win(_T("Can not create compatible DC"));

	/// Create bitmap
	m_bitmap.reset(CreateCompatibleBitmap(dc, width, height), DeleteObject);
	if(!m_bitmap.get())
		throw MCException_Win(_T("Can not create bitmap"));

	/// Select bitmap
	SelectObject(m_bitDC.get(), m_bitmap.get());

	HBRUSH hWhiteBrush = CreateSolidBrush(RGB(255,255,255));
	HBRUSH hBlackBrush = CreateSolidBrush(RGB(0,0,0));

	FillRect(m_bitDC.get(), &rect, hWhiteBrush);

	m_points.clear();

	bool fillRow = firstFilled;
	bool fillCol = firstFilled;
	for(int i = 0; i < rows; ++i)
	{
		for(int j = 0; j < cols; ++j)
		{
			RECT rect;
			rect.left = blockWidth * j;
			rect.top = blockHeight * i;
			rect.right = rect.left + blockWidth;
			rect.bottom = rect.top + blockHeight;

			SPControlPoint point(new SControlPoint());
			point->m_point.x = rect.left + blockWidth / 2;
			point->m_point.y = rect.top + blockHeight / 2;

			HBRUSH hBrush;
			if(fillCol)
			{
				hBrush = hBlackBrush;
				point->m_color = RGB(0,0,0);
			}
			else
			{
				hBrush = hWhiteBrush;
				point->m_color = RGB(255,255,255);
			}
			FillRect(m_bitDC.get(), &rect, hBrush);
			fillCol = !fillCol;

			m_points.push_back(point);
		}
		fillRow = !fillRow;
		fillCol = fillRow;
	}
	DeleteObject(hWhiteBrush);
	DeleteObject(hBlackBrush);

CATCH_THROW()
}

bool CFrame::Check(HDC hdc)
{
TRY_CATCH

	ControlPoints::iterator index;
	for(index = m_points.begin(); index != m_points.end(); ++index)
	{
		SPControlPoint point = *index;
		COLORREF color = GetPixel(hdc, point->m_point.x, point->m_point.y);
		if(point->m_color != color)
			return false;
	}

	return true;

CATCH_THROW()
}

