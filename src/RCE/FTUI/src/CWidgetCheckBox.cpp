//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetCheckBox.cpp
///
///  Implements CWidgetCheckBox class
///  
///  
///  @author Alexander Novak @date 19.12.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWidgetCheckBox.h"
#include "CCommandManager.h"

// CWidgetCheckBox [BEGIN] ///////////////////////////////////////////////////////////////////////////////

LRESULT CWidgetCheckBox::OnButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
TRY_CATCH

	DispatchCommand(cmd_ButtonClick);
	return TRUE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetCheckBox::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);
	
	RECT rcWidget;
	GetClientRect(&rcWidget);
	int oldMode = SetBkMode(hdc,TRANSPARENT);
	DrawText(hdc,m_caption.c_str(),static_cast<int>(m_caption.size()),&rcWidget,DT_VCENTER|DT_SINGLELINE|DT_LEFT);
	SetBkMode(hdc,oldMode);
	
	EndPaint(&ps);
	
	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetCheckBox::OnCustomDraw(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

	RECT rcWidget;
	GetClientRect(&rcWidget);
	int oldMode = SetBkMode(lpdis->hDC,TRANSPARENT);
	DrawText(lpdis->hDC,m_caption.c_str(),static_cast<int>(m_caption.size()),&rcWidget,DT_VCENTER|DT_SINGLELINE|DT_LEFT);
	SetBkMode(lpdis->hDC,oldMode);

	return TRUE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetCheckBox::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	//HWND hwndParent = GetParent();

	//RECT rcWidget;
	//GetClientRect(&rcWidget);
	//POINT ptParent = {rcWidget.left, rcWidget.top};
	//
	//MapWindowPoints(hwndParent,&ptParent,1);

	//RECT rcParent;
	//rcParent.left = ptParent.x;
	//rcParent.top = ptParent.y;
	//rcParent.right = rcParent.left + rcWidget.right - rcWidget.left;
	//rcParent.bottom = rcParent.top + rcWidget.bottom - rcWidget.top;
	//
	//::InvalidateRect(hwndParent,&rcParent,TRUE);
	//::UpdateWindow(hwndParent);
	//::RedrawWindow(hwndParent,&rcParent,NULL,RDW_NOCHILDREN|RDW_UPDATENOW|RDW_INVALIDATE);

	return TRUE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetCheckBox::CWidgetCheckBox(	HWND hParentWnd,
									boost::shared_ptr<CCommandManager> commandManager,
									bool rightAlignText,
									const int checkedImageId,
									const int uncheckedImageId)
	:	CSkinnedElement(commandManager.get()),
		CCommandProxy(commandManager),
		m_rightAlignText(rightAlignText)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CWidgetCheckBox::CWidgetCheckBox"));

	Create(hParentWnd, 0, NULL, WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,WS_EX_TRANSPARENT);
	
	m_skinsImageList->ImageFromRes(&m_checkedImage,1,&checkedImageId);
	m_skinsImageList->ImageFromRes(&m_uncheckedImage,1,&uncheckedImageId);

	Log.Add(_MESSAGE_,_T("END CWidgetCheckBox::CWidgetCheckBox"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetCheckBox::~CWidgetCheckBox()
{
TRY_CATCH

	DestroyWindow();

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetCheckBox::ResizeWidget(LPRECT rect)
{
TRY_CATCH

	MoveWindow(rect,FALSE);
	InvalidateRect(NULL,TRUE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetCheckBox::MoveWidget(const int xPos, const int yPos)
{
TRY_CATCH

	SetWindowPos(0,xPos,yPos,0,0,SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER|SWP_NOSENDCHANGING);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetCheckBox::SetText(const tstring& text)
{
TRY_CATCH

	m_caption = text;

CATCH_THROW()
}
// CWidgetCheckBox [END] /////////////////////////////////////////////////////////////////////////////////
