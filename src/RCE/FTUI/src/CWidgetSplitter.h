//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetSplitter.h
///
///  Declares CWidgetSplitter class
///  
///  
///  @author Alexander Novak @date 24.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <atlsplit.h>
#include "CCommandProxy.h"
#include "CWidgetContainer.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWidgetSplitter
	:	public CSplitterWindowImpl<CWidgetSplitter>,
		public CCommandProxy
{
public:
	DECLARE_WND_CLASS_EX(_T("FTCWidgetSplitter"), CS_DBLCLKS, COLOR_WINDOW)

	CWidgetSplitter(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager);
	~CWidgetSplitter();
	void SetControlledPanels(	const boost::shared_ptr<CWidgetContainer> leftPanel,
								const boost::shared_ptr<CWidgetContainer> rightPanel);
	void ResizeWidget(LPRECT rect);
	void DrawSplitterBar(CDCHandle dc);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
