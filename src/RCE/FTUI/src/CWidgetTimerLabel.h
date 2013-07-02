//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetTimerLabel.h
///
///  Declares CWidgetTimerLabel class
///  
///  
///  @author Alexander Novak @date 10.01.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include "CCommandProxy.h"
#include "CWidgetLabel.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWidgetTimerLabel
	:	public CWidgetLabel
{
	tstring m_defaultText;
	UINT m_elapse;

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	BEGIN_MSG_MAP(CWidgetTimerLabel)

		MESSAGE_HANDLER(WM_TIMER,OnTimer)
		CHAIN_MSG_MAP(CWidgetLabel)
		DEFAULT_REFLECTION_HANDLER()

	END_MSG_MAP()

	CWidgetTimerLabel(	HWND hParentWnd,
						boost::shared_ptr<CAbstractCommandManager> commandManager,
						CSkinnedElement* parent = NULL);
	~CWidgetTimerLabel();
	virtual void SetText(const tstring& text);
	void InitDefaultText(const tstring& defaultText, const UINT elapse);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
