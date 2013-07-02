//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetStatic.h
///
///  Declares CWidgetStatic class
///  
///  
///  @author Alexander Novak @date 22.12.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include "CCommandProxy.h"
#include "..\..\RCUI\src\CSkinnedElement.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWidgetStatic
	:	public CWindowImpl<CWidgetStatic, CStatic>,
		public CSkinnedElement,
		public CCommandProxy
{
	virtual void OnSkinChanged(){};
	virtual HWND GetWindowHandle(){return 0;}

	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	BEGIN_MSG_MAP(CWidgetStatic)

		MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT,OnPaint)
		DEFAULT_REFLECTION_HANDLER()

	END_MSG_MAP()

	CWidgetStatic(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager);
	~CWidgetStatic();
	void ResizeWidget(LPRECT rect);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
