//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetToolBar.h
///
///  Declares CWidgetToolBar class
///  
///  
///  @author Alexander Novak @date 28.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include "CCommandProxy.h"
#include "..\..\RCUI\src\CSkinnedElement.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWidgetToolBar
	:	public CWindowImpl<CWidgetToolBar, CToolBarCtrl>,
		public CSkinnedElement,
		public CCommandProxy
{
	static const int m_buttonCount = 5;//6;
	TBBUTTON m_buttonInfo[m_buttonCount];
	std::map<EWidgetCommand,tstring> m_toolTips;
	std::map<int,tstring> m_filterMenuText;
	HMENU m_mainMenu, m_filterMenu;
	unsigned int m_disabledAttributes;
	int m_maxItemWidth, m_maxItemHeight;
	HBRUSH m_bkBrush;
	HPEN m_highlightPen;
	int m_checkMarkWidth, m_checkMarkHeignt, m_marginSpace;
	
	virtual HWND GetWindowHandle(){return 0;}

	LRESULT OnButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnButtonDropDown(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);
	LRESULT OnCustomDraw(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);
	LRESULT OnGetToolTip(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);

	LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDrawMenuItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMenuCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	BEGIN_MSG_MAP(CWidgetToolBar)

//		MESSAGE_HANDLER(WM_MEASUREITEM,OnMeasureItem)
//		MESSAGE_HANDLER(WM_DRAWITEM,OnDrawMenuItem)
//		MESSAGE_HANDLER(WM_COMMAND,OnMenuCommand)
		 
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO,OnGetToolTip)
		REFLECTED_NOTIFY_CODE_HANDLER(TBN_DROPDOWN,OnButtonDropDown)
		REFLECTED_COMMAND_CODE_HANDLER(BN_CLICKED,OnButtonClick)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW,OnCustomDraw)
		DEFAULT_REFLECTION_HANDLER()

	END_MSG_MAP()

	CWidgetToolBar(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager);
	~CWidgetToolBar();
	void ResizeWidget(LPRECT rect);
	void LockWidget(EWidgetCommand command = cmd_NullCommand);
	void UnlockWidget(EWidgetCommand command = cmd_NullCommand);

	void SetDisabledAttributes(const unsigned int attributes);
	unsigned int GetDisabledAttributes();

	virtual void OnSkinChanged();
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
