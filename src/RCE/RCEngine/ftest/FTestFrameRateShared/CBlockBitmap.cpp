/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CBlockBitmap.cpp
///
///  Implements CBlockBitmap class, responsible for template bitmap
///
///  @author Dmitry Netrebenko @date 14.05.2007
///
////////////////////////////////////////////////////////////////////////

#include "CBlockBitmap.h"
#include <AidLib/CSingleton/CSingleton.h>
#include "CSettings.h"
#include <AidLib/CException/CException.h>

CBlockBitmap::CBlockBitmap()
	:	m_bitmap(NULL)
	,	m_bitDC(NULL)
{
TRY_CATCH

	int width = SETTINGS_INSTANCE.GetBlockWidth();
	int height = SETTINGS_INSTANCE.GetBlockHeight();
	HDC dc = SETTINGS_INSTANCE.GetDC();

	/// Create DC
	m_bitDC = CreateCompatibleDC(dc);

	/// Create bitmap
	m_bitmap = CreateCompatibleBitmap(dc, width, height);

	/// Select bitmap
	SelectObject(m_bitDC, m_bitmap);

	/// Create brush
	HBRUSH brush = CreateSolidBrush(RGB(0,0,0));
	if(brush)
	{
		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = width;
		rect.bottom = height;
		/// Fill block bitmap
		FillRect(m_bitDC, &rect, brush);
		/// Destroy brush
		DeleteObject(brush);
	}

CATCH_THROW()
}

CBlockBitmap::~CBlockBitmap()
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

HDC CBlockBitmap::GetDC()
{
TRY_CATCH

	return m_bitDC;

CATCH_THROW()
}
