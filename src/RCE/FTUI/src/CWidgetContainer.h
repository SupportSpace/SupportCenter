//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetContainer.h
///
///  Declares CWidgetContainer class
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
#include "..\..\RCUI\src\CSkinnedPanel.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWidgetContainer
	:	public CCommandProxy,
		public CSkinnedPanel
{
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	BEGIN_MSG_MAP(CWidgetContainer)

		MESSAGE_HANDLER(WM_SIZE, OnSize)
		REFLECT_NOTIFICATIONS()
		CHAIN_MSG_MAP(CSkinnedPanel)

	END_MSG_MAP()

	CWidgetContainer(	HWND hParentWnd, 
						boost::shared_ptr<CAbstractCommandManager> commandManager, 
						const int imageId,
						const int headImageId = -1,
						const int tailImageId = -1	);
	~CWidgetContainer();
	void ResizeWidget(LPRECT rect);
	void MoveWidget(const int xPos, const int yPos);
	void GetWidgetRect(LPRECT rect);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
