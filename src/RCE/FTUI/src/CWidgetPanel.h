//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetPanel.h
///
///  Declares CWidgetPanel class
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
#include "CWidgetContainer.h"
#include "..\..\RCUI\src\CSkinnedPanel.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWidgetPanel
	:	public CWidgetContainer
{
public:
	BEGIN_MSG_MAP(CWidgetPanel)

		CHAIN_MSG_MAP(CWidgetContainer)

	END_MSG_MAP()

	CWidgetPanel(	HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager, 
					const int imageId, 
					const int headImageId = -1,
					const int tailImageId = -1	);
	void ResizeWidget(LPRECT rect);
	void MoveWidget(const int xPos, const int yPos);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
