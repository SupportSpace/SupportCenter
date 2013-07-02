//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetPanel.cpp
///
///  Implements CWidgetPanel class
///  
///  
///  @author Alexander Novak @date 23.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWidgetPanel.h"

// CWidgetPanel [BEGIN] //////////////////////////////////////////////////////////////////////////////////

CWidgetPanel::CWidgetPanel(	HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager, 
							const int imageId,
							const int headImageId,
							const int tailImageId )
	:	CWidgetContainer(hParentWnd, commandManager, imageId, headImageId, tailImageId)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CWidgetPanel::CWidgetPanel"));

	Log.Add(_MESSAGE_,_T("END CWidgetPanel::CWidgetPanel"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetPanel::ResizeWidget(LPRECT rect)
{
TRY_CATCH

	MoveWindow(rect,TRUE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetPanel::MoveWidget(const int xPos, const int yPos)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CWidgetPanel::MoveWidget"));

	SetWindowPos(0,xPos,yPos,0,0,SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER|SWP_NOSENDCHANGING);

	Log.Add(_MESSAGE_,_T("END CWidgetPanel::MoveWidget"));

CATCH_THROW()
}
// CWidgetPanel [END] ////////////////////////////////////////////////////////////////////////////////////
