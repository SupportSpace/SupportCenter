//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetLabel.h
///
///  Declares CWidgetLabel class
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
#include "..\..\RCUI\src\CSkinnedLabel.h"
#include "..\..\RCUI\src\CSkinnedLink.h"
#include "..\..\RCUI\src\CSkinnedLinkEx.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma warning (push)
#pragma warning (disable:4250)

class CWidgetLabel
	:	public CCommandProxy,
		virtual public CSkinnedLabel
{
	bool m_windowCreated;
	/// WM_LBUTTONUP message handler
	/// @param uMsg message code (expected to be WM_LBUTTONUP)
	/// @param wParam Indicates the mouse buttons and keys that the user pressed.
	/// @param lParam LOWORD - x mouse coordinate, HIWORD - y mouse coordinate
	/// @param bHandled set to TRUE if message should not be further processed
	/// @return return value is obsolete
	virtual LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	CWidgetLabel(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager, CSkinnedElement *parent = NULL);
	~CWidgetLabel();

	BEGIN_MSG_MAP(CWidgetLabel)

		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(CSkinnedLabel)

	END_MSG_MAP()

	void ResizeWidget(LPRECT rect);
	virtual void SetText(const tstring& text);
};

/// Link label class
class CWidgetLinkLabel 
	:	public CWidgetLabel,
		virtual public CSkinnedLink
{
	bool m_widgetVisible;
public:
	CWidgetLinkLabel(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager, CSkinnedElement *parent = NULL)
		:	CWidgetLabel(hParentWnd, commandManager),
			CSkinnedLink(parent),
			CSkinnedLabel(parent),
			m_widgetVisible(true)
	{
	}
	void ShowWidget(bool show = true);
};

/// Link label with image (something like checkbox control)
class CWidgetLinkLabelEx
	:	public CWidgetLabel,
		virtual public CSkinnedLinkEx
{
public:
	CWidgetLinkLabelEx(	HWND hParentWnd, 
						boost::shared_ptr<CAbstractCommandManager> commandManager, 
						const int regularImageId,
						const int mouseOverImageId,
						CSkinnedElement *parent = NULL,
						const UINT imgAlign = DT_RIGHT )
		:	CWidgetLabel(hParentWnd, commandManager),
			CSkinnedLinkEx(regularImageId, mouseOverImageId, parent, imgAlign),
			CSkinnedLink(parent),
			CSkinnedLabel(parent)
	{
	}
};

#pragma warning(pop)