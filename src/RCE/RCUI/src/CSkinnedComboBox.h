/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedComboBox.cpp
///
///  CSkinnedComboBox, skinned combobox
///
///  @author "Archer Software" Kirill Solovyov @date 10.07.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include "CSkinnedElement.h"
#include "CSkinnedListBox.h"
#include <atlapp.h>
#include "wtl/atlctrls.h"
#include <wtl/atlgdi.h>

/// CSkinnedComboBox class
class CSkinnedComboBox :
	public CWindowImpl<CSkinnedComboBox, CComboBox>,
	public CSkinnedElement
{
private:
	/// Button images
	CImage *m_regularImage;
	CImage *m_mouseOverImage;
	CImage *m_pressedImage;
	CImage *m_disabledImage;
	//current font of control
	CFont m_hFont;
	/// owning list box object
	CSkinnedListBox m_list;
public:
	DECLARE_WND_SUPERCLASS(_T("CSkinnedComboBox"), CComboBox::GetWndClassName())
	
	BEGIN_MSG_MAP(CSkinnedButton)
		TRY_CATCH
		MESSAGE_HANDLER_HWND(WM_PRINT, OnPrint)
		if (WM_LBUTTONDOWN == uMsg) 
			OnLMouseDown(); 
		if (WM_LBUTTONUP == uMsg) 
			OnLMouseUp(); 
		if (WM_MOUSEMOVE == uMsg)
			OnMouseMove((int)wParam);
		if (WM_ENABLE == uMsg)
			OnSkinChanged();
		MESSAGE_HANDLER_HWND(OCM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER_HWND(OCM_MEASUREITEM, OnMeasureItem)
		CHAIN_MSG_MAP(CSkinnedElement)
		CATCH_LOG()
	END_MSG_MAP()

	virtual LRESULT OnPrint(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnEraseBkgnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnNCPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnDrawItem(HWND hWnd,UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	virtual LRESULT OnMeasureItem(HWND hWnd,UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
public:
	/// create system object font and call CSkinnedElement::SetLogFont() method
	void SetLogFont(LOGFONT newVal);

	/// set EdgeColor1 for owning ListBox control and call CSkinnedElement::SetEdgeColor1() method
	virtual void SetEdgeColor1(COLORREF newVal);

	/// Accessor to underlying window handle in CSkinnedElement. @see CSkinnedElement
	/// return control window handle
	HWND GetWindowHandle()
	{
		return static_cast<HWND>(*this);
	}
	CSkinnedComboBox(	CSkinnedElement *parent,
						const int regularImageId,
						const int disabledImageId,
						const int pressedImageId,
						const int mouseOverImageId );
	virtual ~CSkinnedComboBox(void);

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	// adjust control's size to current image size
	void SizeToImage();
	/// Proper control initialization
	/// @param  hWnd Controls window handle
	/// @return TRUE if successful
	virtual BOOL Attach(HWND hWnd);

	/// Should be called each time skin was changed
	virtual void OnSkinChanged();

	/// Left mouse button down event handler
	void OnLMouseDown();

	/// Left mouse button up event handler
	void OnLMouseUp();

	/// Mouse move event handler
	void OnMouseMove(int fwKeys);
};
