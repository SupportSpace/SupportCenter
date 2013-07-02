//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetBitmapButton.cpp
///
///  Implements CWidgetBitmapButton class
///  
///  
///  @author Alexander Novak @date 23.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWidgetBitmapButton.h"
#include "CCommandManager.h"

// CWidgetBitmapButton [BEGIN] ///////////////////////////////////////////////////////////////////////////

LRESULT CWidgetBitmapButton::OnButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
TRY_CATCH

	DispatchCommand(cmd_ButtonClick);
	
	return TRUE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------
//
//LRESULT CWidgetBitmapButton::OnButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//TRY_CATCH
//
//	SetCapture();
//
//CATCH_THROW()
//}
////--------------------------------------------------------------------------------------------------------
//
//LRESULT CWidgetBitmapButton::OnButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//TRY_CATCH
//
//	if ( m_hWnd == ::GetCapture() )
//	{
//		ReleaseCapture();
//
//		// To prevent a button click message when a cursor is out of the button
//		POINT pt = {LOWORD(lParam),HIWORD(lParam)};
//		RECT rc;
//		GetClientRect(&rc);
//
//		if ( PtInRect(&rc,pt) )
//			::PostMessage(GetParent(),WM_COMMAND,MAKEWPARAM(0,BN_CLICKED),(LPARAM)m_hWnd);
//	}
//
//CATCH_THROW()
//}
//--------------------------------------------------------------------------------------------------------

CWidgetBitmapButton::CWidgetBitmapButton(	HWND hParentWnd,
								boost::shared_ptr<CAbstractCommandManager> commandManager,
								const int normalImageId,
								const int disableImageId,
								const int pressedImageId,
								const int mouseOverImageId)
	:	CSkinnedButton(commandManager.get(),normalImageId,disableImageId,pressedImageId,mouseOverImageId),
		CCommandProxy(commandManager)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CWidgetBitmapButton::CWidgetBitmapButton"));

	Create(hParentWnd, 0, NULL, WS_CHILD|WS_VISIBLE|BS_OWNERDRAW);

	Log.Add(_MESSAGE_,_T("END CWidgetBitmapButton::CWidgetBitmapButton"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetBitmapButton::~CWidgetBitmapButton()
{
TRY_CATCH

	DestroyWindow();

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetBitmapButton::ResizeWidget(LPRECT rect)
{
TRY_CATCH

	MoveWindow(rect,TRUE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetBitmapButton::MoveWidget(const int xPos, const int yPos)
{
TRY_CATCH

	SetWindowPos(0,xPos,yPos,0,0,SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER|SWP_NOSENDCHANGING);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetBitmapButton::LockWidget()
{
TRY_CATCH

	EnableWindow(FALSE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetBitmapButton::UnlockWidget()
{
TRY_CATCH

	EnableWindow(TRUE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetBitmapButton::PostCommand()
{
TRY_CATCH

	PostMessage(OCM_COMMAND,MAKEWPARAM(0,BN_CLICKED));

CATCH_THROW()
}
// CWidgetBitmapButton [END] /////////////////////////////////////////////////////////////////////////////
