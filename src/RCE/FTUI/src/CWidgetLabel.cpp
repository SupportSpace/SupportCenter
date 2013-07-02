//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetLabel.cpp
///
///  Implements CWidgetLabel class
///  
///  
///  @author Alexander Novak @date 23.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWidgetLabel.h"

CWidgetLabel::CWidgetLabel(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager, CSkinnedElement *parent)
	:	CCommandProxy(commandManager),
		CSkinnedLabel(parent),
		m_windowCreated(false)
{
TRY_CATCH

	if (NULL != hParentWnd)
	{
		Create(hParentWnd, 0, NULL, WS_CHILD|WS_VISIBLE);
		if (IsWindow())
			m_windowCreated = true;
	}
	SetClassLongPtr(m_hWnd, GCL_HCURSOR, NULL);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetLabel::~CWidgetLabel()
{
TRY_CATCH

	if (m_windowCreated)
		DestroyWindow();

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetLabel::ResizeWidget(LPRECT rect)
{
TRY_CATCH

	MoveWindow(rect,TRUE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetLabel::SetText(const tstring& text)
{
TRY_CATCH

	SetWindowText(text.c_str());	

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetLabel::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	bHandled = FALSE;
	DispatchCommand(cmd_ButtonClick);
	return CSkinnedLabel::OnLButtonUp(uMsg, wParam, lParam, bHandled);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetLabel::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	//Retriving paint DC (note not using CPaintDC, since CPaintDC calls BeginPaint
	//which inturn must be called only from WM_PAINT, WM_ERASEBGRND message handler
	HDC hDC = ::GetDC( GetWindowHandle() );

	//Adjusting size;
	AdjustSize(hDC);

	//Refreshing parent
	Refresh(hDC);

	//Releasing resources
	ReleaseDC(hDC);
	
	return 0;

CATCH_THROW()
}
// CWidgetLabel [END] ////////////////////////////////////////////////////////////////////////////////////

// CWidgetLinkLabel [BEGIN] //////////////////////////////////////////////////////////////////////////////

void CWidgetLinkLabel::ShowWidget(bool show)
{
TRY_CATCH

	if ( m_widgetVisible != show )
		ShowWindow(( m_widgetVisible = show ) ? SW_SHOW : SW_HIDE);

CATCH_THROW()
}
// CWidgetLinkLabel [END] ////////////////////////////////////////////////////////////////////////////////
