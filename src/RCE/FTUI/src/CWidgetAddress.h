//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetAddress.h
///
///  Declares CWidgetAddress class
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
#include "..\..\RCUI\src\CSkinnedElement.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWidgetAddress
	:	public CWindowImpl<CWidgetAddress, CEdit>,
		public CSkinnedElement,
		public CCommandProxy
{
	/// WM_CTLCOLORSTATIC message handler, to avoid disables control graying
	LRESULT OnColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	BEGIN_MSG_MAP(CWidgetAddress)

		MESSAGE_HANDLER(OCM_CTLCOLORSTATIC, OnColorStatic)
		/*MESSAGE_HANDLER_HWND(WM_PRINT, OnPrint)
		CHAIN_MSG_MAP(CSkinnedElement)*/
		DEFAULT_REFLECTION_HANDLER()

	END_MSG_MAP()

	CWidgetAddress(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager);
	~CWidgetAddress();
	void ResizeWidget(LPRECT rect);
	void SetAddress(const tstring& address);
	tstring GetAddress();

	virtual LRESULT OnPrint(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnEraseBkgnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnNCPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	// Accessor to underlying window handle.
	virtual HWND GetWindowHandle()
	{
	TRY_CATCH
		return m_hWnd;
	CATCH_THROW()
	};

	/// Should be called each time skin was changed
	virtual void OnSkinChanged(){};

	/// create system object font and call CSkinnedElement::SetLogFont() method
	void SetLogFont(LOGFONT newVal);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
