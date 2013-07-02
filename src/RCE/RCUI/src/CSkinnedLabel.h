/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedLabel.h
///
///  Skinned label implementation
///
///  @author "Archer Software" Sogin M. @date 22.06.2007
///
////////////////////////////////////////////////////////////////////////<summary>

#pragma once

// CSkinnedLabel
#include "CSkinnedElement.h"
#include <wtl/atlapp.h>
///#include <wtl/atlmisc.h>
#include <wtl/atlcrack.h>
#include <wtl/atlctrls.h>


/// Skinned label control implementation<summary>
/// @see CSkinnedLabelMultiline
class CSkinnedLabel :
	public CWindowImplEx<CSkinnedLabel, CStatic>, public CSkinnedElement
{
protected:
	bool m_UnderMouse;

	/// Adjusts size of label to fit all text withing control
	/// @param hDC corresponding device context
	void AdjustSize(HDC &hDC);

	/// Repaints label and corresponding part of parent control
	/// @param hDC corresponding device context
	virtual void Refresh(HDC &hDC);

	/// Text alignment
	UINT m_textAlign;
public:
	/// ctor
	/// @param parent pointer to parent control, that is needed since label is redrawn in pair with it's parent
	CSkinnedLabel(CSkinnedElement *parent);
	/// class destructor
	virtual ~CSkinnedLabel();

	DECLARE_WND_SUPERCLASS(_T("CSkinnedLabel"), CStatic::GetWndClassName())

	// Accessor to element's text alignment
	__declspec(property (get=GetTextAlign, put=SetTextAlign)) UINT TextAlign;

	UINT GetTextAlign() const;
	void SetTextAlign(UINT newVal);

	/// Proper control initialization
	/// @param  hWnd Controls window handle
	/// @param  Parent Parent control
	/// @return TRUE if successful
	virtual BOOL Attach(HWND hWnd);
	
	/// Accessor to underlying window handle in CSkinnedElement. @see CSkinnedElement
	/// return control window handle
	HWND GetWindowHandle()
	{
		return static_cast<HWND>(*this);
	}
	
	/// Text property setter method @see CSkinnedElement::SetText
	/// @param newVal new Text value
	void SetText(tstring newVal)
	{
		SetWindowText(newVal.c_str());
	}

	/// DOXYS_OFF 
	BEGIN_MSG_MAP(CSkinnedLabel)
		TRY_CATCH
		MSG_WM_SETCURSOR(OnSetCursor)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_PRINT(OnPrint)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		//MSG_WM_UPDATEUISTATE(OnUpdateUIState)
		if (uMsg == 0x0128) \
		{ \
			SetMsgHandled(TRUE); \
			OnUpdateUIState(LOWORD(wParam), HIWORD(wParam)); \
			lResult = 0; \
			if(IsMsgHandled()) \
				return TRUE; \
		}
		MESSAGE_HANDLER(WM_NCHITTEST, OnNChitTest)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButronDblClk)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_SETTEXT, OnSetWindowText)
		CHAIN_MSG_MAP(CSkinnedElement)
		CATCH_LOG("BEGIN_MSG_MAP(CSkinnedLabel)")
	END_MSG_MAP()
	/// DOXYS_ON

	/// WM_PAINT message handler
	/// @param  hdc device context for drawing if not NULL
	/// @return return value is obsolete
	virtual LRESULT OnPaint(HDC hdc);

	virtual LRESULT OnEraseBkgnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/// Notification from CSkinEngine after it applies skin attributes.
	void OnSkinChanged();

	void OnPrint(HDC hDC, int) {OnPaint(hDC);}
	void OnUpdateUIState(DWORD, DWORD) {};

	/// Returns label text size
	virtual SIZE GetTextSize();

	/// Adjusts size of label to fit all text withing control
	/// @param hDC corresponding device context
	void AdjustSize();

protected:

	/// WM_MOUSEMOVE message handler
	/// @param Flags Indicates the mouse buttons and keys that the user pressed
	/// @param Pt mouse pointer coords
	/// @return return value is obsolete
	virtual LRESULT OnMouseMove(UINT nFlags, CPoint point)
	{
		if (m_UnderMouse) return FALSE;
		m_UnderMouse = true;
		TRACKMOUSEEVENT tm = { sizeof( TRACKMOUSEEVENT ) , TME_LEAVE, m_hWnd , 1 };
		// TODO: call events here
		return ::TrackMouseEvent( &tm );
	}
	
	/// WM_MOUSELEAVE message handler
	/// @return return value is obsolete
	virtual LRESULT OnMouseLeave(void)
	{
		m_UnderMouse = false;
		// TODO: call events here
		TRACKMOUSEEVENT tm = { sizeof( TRACKMOUSEEVENT ) , TME_HOVER, m_hWnd , 1 };
		return ::TrackMouseEvent( &tm );
	}

	/// WM_NCHITTEST message handler
	/// @param uMsg message code (expected to be WM_NCHITTEST)
	/// @param wParam This parameter is not used
	/// @param lParam The low-order word specifies the x-coordinate of the cursor. The coordinate is relative to the upper-left corner of the screen. 
	/// The high-order word specifies the y-coordinate of the cursor. The coordinate is relative to the upper-left corner of the screen. 
	/// @param bHandled set to TRUE if message should not be further processed
	/// @return return value is obsolete
	virtual LRESULT OnNChitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/// WM_LBUTTONUP message handler
	/// @param uMsg message code (expected to be WM_LBUTTONUP)
	/// @param wParam Indicates the mouse buttons and keys that the user pressed.
	/// @param lParam LOWORD - x mouse coordinate, HIWORD - y mouse coordinate
	/// @param bHandled set to TRUE if message should not be further processed
	/// @return return value is obsolete
	virtual LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/// WM_LBUTTONDOWN message handler
	/// @param uMsg message code (expected to be WM_LBUTTONDOWN)
	/// @param wParam Indicates the mouse buttons and keys that the user pressed.
	/// @param lParam LOWORD - x mouse coordinate, HIWORD - y mouse coordinate
	/// @param bHandled set to TRUE if message should not be further processed
	/// @return return value is obsolete
	virtual LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/// WM_LBUTTONDBLCLK message handler
	/// @param uMsg message code (expected to be WM_LBUTTONDBLCLK)
	/// @param wParam Indicates the mouse buttons and keys that the user pressed.
	/// @param lParam LOWORD - x mouse coordinate, HIWORD - y mouse coordinate
	/// @param bHandled set to TRUE if message should not be further processed
	/// @return return value is obsolete
	virtual LRESULT OnLButronDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/// WM_SETCURSOR message handler
	virtual LRESULT OnSetCursor(HWND hWnd, UINT, UINT);

	/// WM_SETTEXT message handler
	/// @param uMsg message code (expected to be WM_SETTEXT)
	/// @param wParam set to 0
	/// @param lParam lParam = (LPARAM)(LPCTSTR) lpsz;
	/// @param bHandled set to TRUE if message should not be further processed
	/// @return return value is obsolete
	virtual LRESULT OnSetWindowText(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
	TRY_CATCH
		if (lParam == 0) return FALSE;
		if (Text == reinterpret_cast<TCHAR*>(lParam)) return TRUE;
		CSkinnedElement::SetText(reinterpret_cast<TCHAR*>(lParam));
		OnSkinChanged();
		bHandled = TRUE;
		return TRUE;
	CATCH_THROW()
	}
};


