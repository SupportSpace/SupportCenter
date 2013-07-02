//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetStatic.cpp
///
///  Implements CWidgetStatic class
///  
///  
///  @author Alexander Novak @date 22.12.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWidgetStatic.h"
#include "CCommandManager.h"

// CWidgetStatic [BEGIN] /////////////////////////////////////////////////////////////////////////////////


LRESULT CWidgetStatic::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	return TRUE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetStatic::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);
	
	HBRUSH hBrush = CreateSolidBrush(FontColor1);
	
	FillRect(hdc,&ps.rcPaint,hBrush);
	
	DeleteObject(hBrush);
	
	EndPaint(&ps);
	
	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetStatic::CWidgetStatic(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager)
	:	CSkinnedElement(commandManager.get()),
		CCommandProxy(commandManager)
{
TRY_CATCH

	Create(hParentWnd, 0, NULL, WS_CHILD|WS_VISIBLE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetStatic::~CWidgetStatic()
{
TRY_CATCH

	DestroyWindow();

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetStatic::ResizeWidget(LPRECT rect)
{
TRY_CATCH

	MoveWindow(rect,TRUE);

CATCH_THROW()
}
// CWidgetStatic [END] ///////////////////////////////////////////////////////////////////////////////////
