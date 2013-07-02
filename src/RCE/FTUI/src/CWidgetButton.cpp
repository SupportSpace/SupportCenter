//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetButton.cpp
///
///  Implements CWidgetButton class
///  
///  
///  @author Alexander Novak @date 23.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWidgetButton.h"
#include "CCommandManager.h"

// CWidgetButton [BEGIN] /////////////////////////////////////////////////////////////////////////////////

LRESULT CWidgetButton::OnButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
TRY_CATCH

	DispatchCommand(cmd_ButtonClick);
	return TRUE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetButton::CWidgetButton(	HWND hParentWnd,
								boost::shared_ptr<CAbstractCommandManager> commandManager)
	:	CCommandProxy(commandManager)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CWidgetButton::CWidgetButton"));

	Create(hParentWnd, 0, NULL, WS_CHILD|WS_VISIBLE);

	Log.Add(_MESSAGE_,_T("END CWidgetButton::CWidgetButton"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetButton::~CWidgetButton()
{
TRY_CATCH

	DestroyWindow();

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetButton::ResizeWidget(LPRECT rect)
{
TRY_CATCH

	MoveWindow(rect,TRUE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetButton::MoveWidget(const int xPos, const int yPos)
{
TRY_CATCH

	SetWindowPos(0,xPos,yPos,0,0,SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER|SWP_NOSENDCHANGING);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetButton::SetText(const tstring& text)
{
TRY_CATCH

	SetWindowText(text.c_str());

CATCH_THROW()
}
// CWidgetButton [END] ///////////////////////////////////////////////////////////////////////////////////
