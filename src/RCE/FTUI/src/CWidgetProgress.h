//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetProgress.h
///
///  Declares CWidgetProgress class
///  
///  
///  @author Alexander Novak @date 03.12.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include "CCommandProxy.h"
#include "..\..\RCUI\src\CSkinnedElement.h"
//========================================================================================================

#define WPB_LEFT_PART_WIDTH			10
#define WPB_PAD_PART_WIDTH			1
#define WPB_RIGHT_PART_WIDTH		10


//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWidgetProgress
	:	public CWindowImpl<CWidgetProgress, CStatic>,
		public CSkinnedElement,
		public CCommandProxy
{
	bool m_widgetVisible;
	RECT m_rectWidget;
	int m_progressPos;
	CImage* m_leftEmpty;
	CImage* m_rightEmpty;
	CImage* m_leftFull;
	CImage* m_rightFull;
	CImage* m_padEmpty;
	CImage* m_padFull;

	virtual void OnSkinChanged(){};
	virtual HWND GetWindowHandle(){return 0;}

	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	BEGIN_MSG_MAP(CWidgetAddress)

		MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT,OnPaint)
		DEFAULT_REFLECTION_HANDLER()

	END_MSG_MAP()

	CWidgetProgress(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager);
	~CWidgetProgress();
	void ResizeWidget(LPRECT rect);
	void SetProgressState(const ULARGE_INTEGER total, const ULARGE_INTEGER current);
	void ShowWidget(bool show = true);
	bool IsVisible();
	//void GetWidgetRect(LPRECT rect);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

