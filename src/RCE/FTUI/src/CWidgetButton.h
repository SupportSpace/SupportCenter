//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetButton.h
///
///  Declares CWidgetButton class
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
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWidgetButton
	:	public CWindowImpl<CWidgetButton, CButton>,
		public CCommandProxy
{
	LRESULT OnButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
public:

	BEGIN_MSG_MAP(CWidgetButton)

		REFLECTED_COMMAND_CODE_HANDLER(BN_CLICKED,OnButtonClick)
		DEFAULT_REFLECTION_HANDLER()

	END_MSG_MAP()

	CWidgetButton(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager);
	~CWidgetButton();
	void ResizeWidget(LPRECT rect);
	void MoveWidget(const int xPos, const int yPos);
	void SetText(const tstring& text);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
