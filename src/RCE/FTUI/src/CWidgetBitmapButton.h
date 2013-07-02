//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetBitmapButton.h
///
///  Declares CWidgetBitmapButton class
///  
///  
///  @author Alexander Novak @date 23.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include "CCommandProxy.h"
#include "..\..\RCUI\src\CSkinnedButton.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWidgetBitmapButton
	:	public CSkinnedButton,
		public CCommandProxy
{
	LRESULT OnButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//	LRESULT OnButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	LRESULT OnButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:

	BEGIN_MSG_MAP(CWidgetBitmapButton)

		REFLECTED_COMMAND_CODE_HANDLER(BN_CLICKED,OnButtonClick)
//		MESSAGE_HANDLER(WM_LBUTTONDOWN,OnButtonDown);
//		MESSAGE_HANDLER(WM_LBUTTONUP,OnButtonUp);

		CHAIN_MSG_MAP(CSkinnedButton)
		DEFAULT_REFLECTION_HANDLER()

	END_MSG_MAP()

	CWidgetBitmapButton(HWND hParentWnd,
						boost::shared_ptr<CAbstractCommandManager> commandManager,
						const int normalImageId,
						const int disableImageId,
						const int pressedImageId,
						const int mouseOverImageId);
	~CWidgetBitmapButton();
	void ResizeWidget(LPRECT rect);
	void MoveWidget(const int xPos, const int yPos);
	void LockWidget();
	void UnlockWidget();
	void PostCommand();
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
