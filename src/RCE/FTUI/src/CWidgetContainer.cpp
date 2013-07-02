//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetContainer.cpp
///
///  Implements CWidgetContainer class
///  
///  
///  @author Alexander Novak @date 23.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWidgetContainer.h"

// CWidgetContainer [BEGIN] //////////////////////////////////////////////////////////////////////////////

LRESULT CWidgetContainer::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	bHandled = FALSE;
	DispatchCommand(cmd_Size);
	return TRUE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetContainer::CWidgetContainer(	HWND hParentWnd, 
									boost::shared_ptr<CAbstractCommandManager> commandManager, 
									const int imageId,
									const int headImageId,
									const int tailImageId )
	:	CCommandProxy(commandManager),
		CSkinnedPanel(imageId, headImageId, tailImageId)

{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CWidgetContainer::CWidgetContainer"));

	Create(hParentWnd, 0, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);

	Log.Add(_MESSAGE_,_T("END CWidgetContainer::CWidgetContainer"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetContainer::~CWidgetContainer()
{
TRY_CATCH

	DestroyWindow();

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetContainer::ResizeWidget(LPRECT rect)
{
TRY_CATCH

	MoveWindow(rect,TRUE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetContainer::MoveWidget(const int xPos, const int yPos)
{
TRY_CATCH

	SetWindowPos(0,xPos,yPos,0,0,SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER|SWP_NOSENDCHANGING);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetContainer::GetWidgetRect(LPRECT rect)
{
TRY_CATCH

	GetClientRect(rect);

CATCH_THROW()
}
// CWidgetContainer [END] ////////////////////////////////////////////////////////////////////////////////
