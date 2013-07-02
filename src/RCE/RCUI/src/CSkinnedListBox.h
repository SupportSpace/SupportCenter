/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedListBox.h
///
///  CSkinnedListBox, skinned list box
///
///  @author "Archer Software" Kirill Solovyov @date 10.07.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include "cskinnedelement.h"
#include <atlapp.h>
#include "wtl/atlctrls.h"

class CSkinnedListBox :
	public CWindowImpl<CSkinnedListBox, CListBox>,
	public CSkinnedElement
{
protected:
	virtual LRESULT OnEraseBkgnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnNCPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	DECLARE_WND_SUPERCLASS(_T("CSkinnedListBox"), CListBox::GetWndClassName())
	BEGIN_MSG_MAP(CSkinnedList)
		TRY_CATCH
		CHAIN_MSG_MAP(CSkinnedElement)
		CATCH_LOG()
	END_MSG_MAP()
	
	/// Accessor to underlying window handle in CSkinnedElement. @see CSkinnedElement
	/// return control window handle
	HWND GetWindowHandle()
	{
		return static_cast<HWND>(*this);
	}
	
	CSkinnedListBox(void);
	virtual ~CSkinnedListBox(void);
	
	/// Proper control initialization
	/// @param  hWnd Controls window handle
	/// @return TRUE if successful
	virtual BOOL Attach(HWND hWnd);
	
	/// Should be called each time skin was changed
	virtual void OnSkinChanged();
};
