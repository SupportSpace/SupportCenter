//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetCheckBox.h
///
///  Declares CWidgetCheckBox class
///  
///  
///  @author Alexander Novak @date 19.12.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include "CCommandProxy.h"
#include "..\..\RCUI\src\CSkinnedElement.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWidgetCheckBox
	:	public CWindowImpl<CWidgetCheckBox, CButton>,
		public CSkinnedElement,
		public CCommandProxy
{
	bool m_rightAlignText;
	CImage* m_checkedImage;
	CImage* m_uncheckedImage;
	tstring m_caption;

	virtual void OnSkinChanged(){};
	virtual HWND GetWindowHandle(){return 0;}

	LRESULT OnButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCustomDraw(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:

	BEGIN_MSG_MAP(CWidgetCheckBox)

		MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBackground)
		//MESSAGE_HANDLER(WM_PAINT,OnPaint)
		MESSAGE_HANDLER(OCM_DRAWITEM,OnCustomDraw)
		REFLECTED_COMMAND_CODE_HANDLER(BN_CLICKED,OnButtonClick)
		DEFAULT_REFLECTION_HANDLER()

	END_MSG_MAP()

	CWidgetCheckBox(	HWND hParentWnd,
						boost::shared_ptr<CCommandManager> commandManager,
						bool rightAlignText,
						const int checkedImageId,
						const int uncheckedImageId);
	~CWidgetCheckBox();
	void ResizeWidget(LPRECT rect);
	void MoveWidget(const int xPos, const int yPos);
	void SetText(const tstring& text);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
