//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetSplitter.cpp
///
///  Implements CWidgetSplitter class
///  
///  
///  @author Alexander Novak @date 24.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWidgetSplitter.h"

// CWidgetSplitter [BEGIN] ///////////////////////////////////////////////////////////////////////////////

CWidgetSplitter::CWidgetSplitter(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager)
	:	CCommandProxy(commandManager)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CWidgetSplitter::CWidgetSplitter"));

	RECT rc;
	::GetClientRect(hParentWnd,&rc);

	Create(hParentWnd, &rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	m_cxyBarEdge = 3;

	Log.Add(_MESSAGE_,_T("END CWidgetSplitter::CWidgetSplitter"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetSplitter::~CWidgetSplitter()
{
TRY_CATCH

	DestroyWindow();

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetSplitter::SetControlledPanels(	const boost::shared_ptr<CWidgetContainer> leftPanel,
											const boost::shared_ptr<CWidgetContainer> rightPanel)
{
TRY_CATCH

	SetSplitterPanes(leftPanel->m_hWnd, rightPanel->m_hWnd);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetSplitter::ResizeWidget(LPRECT rect)
{
TRY_CATCH

	MoveWindow(rect,TRUE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetSplitter::DrawSplitterBar(CDCHandle dc)
{
TRY_CATCH

	RECT rect;
	if ( GetSplitterBarRect(&rect) )
	{
		RECT rc;
		GetClientRect(&rc);
		HRGN rgnWnd = CreateRectRgnIndirect(&rc);
		HRGN rgnBar = CreateRectRgnIndirect(&rect);
		CombineRgn(rgnWnd,rgnWnd,rgnBar,RGN_XOR);

		rc = rect;
		rc.left = rect.left + (rect.right - rect.left)/2;
		rc.right = rc.left + 1;

		HRGN rgnLine = CreateRectRgnIndirect(&rc);
		CombineRgn(rgnWnd,rgnWnd,rgnLine,RGN_OR);

		SetWindowRgn(rgnWnd,TRUE);

		DeleteObject(rgnBar);
		DeleteObject(rgnLine);
		DeleteObject(rgnWnd);

		dc.MoveTo(rc.left,rc.top);
		dc.LineTo(rc.left,rc.bottom);
	}

CATCH_THROW()
}
// CWidgetSplitter [END] /////////////////////////////////////////////////////////////////////////////////
