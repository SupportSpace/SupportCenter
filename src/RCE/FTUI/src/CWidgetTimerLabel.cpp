//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetTimerLabel.cpp
///
///  Implements CWidgetTimerLabel class
///  
///  
///  @author Alexander Novak @date 10.01.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWidgetTimerLabel.h"
#include "CCommandManager.h"

// CWidgetTimerLabel [BEGIN] /////////////////////////////////////////////////////////////////////////////

LRESULT CWidgetTimerLabel::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	SetText(m_defaultText);

	KillTimer(1);
	
	DispatchCommand(cmd_DefaultTextAppeared);
	
	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetTimerLabel::CWidgetTimerLabel(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager, CSkinnedElement* parent)
	:	CWidgetLabel(hParentWnd,commandManager,parent),
		CSkinnedLabel(parent),
		m_elapse(0)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetTimerLabel::~CWidgetTimerLabel()
{
TRY_CATCH

	KillTimer(1);

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetTimerLabel::SetText(const tstring& text)
{
TRY_CATCH

	CWidgetLabel::SetText(text);

	if ( m_elapse )
		SetTimer(1,m_elapse);

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetTimerLabel::InitDefaultText(const tstring& defaultText, const UINT elapse)
{
TRY_CATCH

	m_defaultText	= defaultText;
	m_elapse		= elapse;

	if ( elapse )
		SetTimer(1,elapse);
	else
		KillTimer(1);

CATCH_LOG()
}
// CWidgetTimerLabel [END] ///////////////////////////////////////////////////////////////////////////////
