//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetProgress.cpp
///
///  Implements CWidgetProgress class
///  
///  
///  @author Alexander Novak @date 03.12.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWidgetProgress.h"
#include "CCommandManager.h"
#include "resource.h"

// CWidgetProgress [BEGIN] ///////////////////////////////////////////////////////////////////////////////

LRESULT CWidgetProgress::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	return TRUE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetProgress::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);

	ps.rcPaint.top		= m_rectWidget.top;
	ps.rcPaint.bottom	= m_rectWidget.bottom;

	if ( ps.rcPaint.left < m_rectWidget.left + WPB_LEFT_PART_WIDTH )
	{
		RECT rcDraw = m_rectWidget;
		rcDraw.right = rcDraw.left + WPB_LEFT_PART_WIDTH;

		if ( m_progressPos > m_rectWidget.left )
			m_leftFull->Draw(hdc,rcDraw);
		else
			m_leftEmpty->Draw(hdc,rcDraw);

		ps.rcPaint.left = m_rectWidget.left + WPB_LEFT_PART_WIDTH;
	}
	if ( ps.rcPaint.right > m_rectWidget.right - WPB_RIGHT_PART_WIDTH )
	{
		RECT rcDraw = m_rectWidget;
		rcDraw.left = rcDraw.right - WPB_RIGHT_PART_WIDTH;

		if ( m_progressPos > m_rectWidget.right - WPB_RIGHT_PART_WIDTH )
			m_rightFull->Draw(hdc,rcDraw);
		else
			m_rightEmpty->Draw(hdc,rcDraw);
		
		ps.rcPaint.right = m_rectWidget.right - WPB_RIGHT_PART_WIDTH;
	}
	if ( ps.rcPaint.right > ps.rcPaint.left )
	{
		RECT rcDraw = ps.rcPaint;
		rcDraw.right	= min(m_progressPos,ps.rcPaint.right);
		
		if ( rcDraw.left < rcDraw.right )
			m_padFull->Draw(hdc,rcDraw);
		
		rcDraw.right	= ps.rcPaint.right;
		rcDraw.left		= max(m_progressPos,ps.rcPaint.left);
		
		if ( rcDraw.left < rcDraw.right )
			m_padEmpty->Draw(hdc,rcDraw);
	}
	
	EndPaint(&ps);
	
	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetProgress::CWidgetProgress(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager)
	:	CSkinnedElement(commandManager.get()),
		CCommandProxy(commandManager),
		m_widgetVisible(true),
		m_progressPos(0)
{
TRY_CATCH

	int resId = IDR_PB_LEFT_EMPTY_7x16;
	m_skinsImageList->ImageFromRes(&m_leftEmpty,1,&resId);

	resId = IDR_PB_RIGHT_EMPTY_7x16;
	m_skinsImageList->ImageFromRes(&m_rightEmpty,1,&resId);

	resId = IDR_PB_LEFT_FULL_7x16;
	m_skinsImageList->ImageFromRes(&m_leftFull,1,&resId);

	resId = IDR_PB_RIGHT_FULL_7x16;
	m_skinsImageList->ImageFromRes(&m_rightFull,1,&resId);

	resId = IDR_PB_PAD_EMPTY_1x16;
	m_skinsImageList->ImageFromRes(&m_padEmpty,1,&resId);

	resId = IDR_PB_PAD_FULL_1x16;
	m_skinsImageList->ImageFromRes(&m_padFull,1,&resId);

	Create(hParentWnd, 0, NULL, WS_CHILD|WS_VISIBLE);

	GetClientRect(&m_rectWidget);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetProgress::~CWidgetProgress()
{
TRY_CATCH

	DestroyWindow();

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetProgress::ResizeWidget(LPRECT rect)
{
TRY_CATCH

	MoveWindow(rect,TRUE);
	GetClientRect(&m_rectWidget);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetProgress::SetProgressState(const ULARGE_INTEGER total, const ULARGE_INTEGER current)
{
TRY_CATCH

	ULARGE_INTEGER progressPos;
	
	if ( total.QuadPart )
		progressPos.QuadPart = ( m_rectWidget.right - m_rectWidget.left) * current.QuadPart / total.QuadPart;
	else
		progressPos.QuadPart = m_rectWidget.right - m_rectWidget.left;
	
	int previousPos = m_progressPos;
	m_progressPos = progressPos.LowPart; // Because of the previous divide operation, HighPart will be zero
	
	if ( previousPos - m_progressPos )
	{
		RECT rcUpdate = m_rectWidget;
		rcUpdate.left	= min(previousPos,m_progressPos);
		rcUpdate.right	= max(previousPos,m_progressPos);
		
		InvalidateRect(&rcUpdate,FALSE);
	}

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetProgress::ShowWidget(bool show)
{
TRY_CATCH

	if ( m_widgetVisible != show )
		ShowWindow(( m_widgetVisible = show ) ? SW_SHOW : SW_HIDE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

bool CWidgetProgress::IsVisible()
{
TRY_CATCH

	return m_widgetVisible;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------
//
//void CWidgetProgress::GetWidgetRect(LPRECT rect)
//{
//TRY_CATCH
//
//	GetClientRect(rect);
//
//CATCH_THROW()
//}
// CWidgetProgress [END] /////////////////////////////////////////////////////////////////////////////////
