/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedButton.h
///
///  Skinned Button implementation
///
///  @author "Archer Software" Sogin M. @date 22.06.2007
///
////////////////////////////////////////////////////////////////////////<summary>

#pragma once

// CSkinnedButton
#include "CSkinnedElement.h"
#include <wtl/atlapp.h>
///#include <wtl/atlmisc.h>
#include <wtl/atlcrack.h>
#include <wtl/atlctrls.h>


/// Skinned Button control implementation<summary>
/// @see CSkinnedButtonMultiline
class CSkinnedButton :
	public CWindowImplEx<CSkinnedButton, CButton>, public CSkinnedElement
{
protected:
	bool m_UnderMouse;

	/// Adjusts size of Button to fit all text withing control
	/// @param hDC corresponding device context
	void AdjustSize(HDC &hDC);

	/// Repaints Button and corresponding part of parent control
	/// @param hDC corresponding device context
	virtual void Refresh(HDC &hDC);

	/// Text alignment
	UINT m_textAlign;

	/// Button images
	CImage *m_regularImage;
	CImage *m_mouseOverImage;
	CImage *m_pressedImage;
	CImage *m_disabledImage;

	/// Border width
	int m_border;

public:
	/// ctor
	/// @param parent pointer to parent control, that is needed since Button is redrawn in pair with it's parent
	CSkinnedButton(	CSkinnedElement *parent,
					const int regularImageId,
					const int disabledImageId,
					const int pressedImageId,
					const int mouseOverImageId );
	/// class destructor
	virtual ~CSkinnedButton();

	/// Set up state images
	void SetImages(	const int regularImageId,
					const int disabledImageId,
					const int pressedImageId,
					const int mouseOverImageId );


	DECLARE_WND_SUPERCLASS(_T("CSkinnedButton"), CButton::GetWndClassName())

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
	BEGIN_MSG_MAP(CSkinnedButton)
		TRY_CATCH
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_OCM_DRAWITEM(OnDrawItem)
		MESSAGE_HANDLER(WM_NCHITTEST, OnNChitTest)
		//MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButronDblClk)
		
		// Uncommented to fix a double mouse click
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		// Uncommented to fix a double mouse click
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		
		MESSAGE_HANDLER(WM_SETTEXT, OnSetWindowText)
		//I think to use function of base class via
		//CHAIN_MSG_MAP(CSkinnedElement)// call SkinnedElement's handlers
		//but CSkinnedButton use only one function from base class
		//therefore this handler added to this class
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO,OnGetDispInfo)

		// After this line a behaviour become determined
		DEFAULT_REFLECTION_HANDLER()

		CATCH_LOG("BEGIN_MSG_MAP(CSkinnedButton)")
	END_MSG_MAP()
	/// DOXYS_ON

	/// WM_PAINT message handler
	/// @param  hdc device context for drawing if not NULL
	/// @return return value is obsolete
	virtual LRESULT OnPaint(HDC hdc);

	/// Notification from CSkinEngine after it applies skin attributes.
	void OnSkinChanged();

	/// Draw item event handler
	void OnDrawItem(UINT idCtl, LPDRAWITEMSTRUCT lpdis);
private:

	/// WM_MOUSEMOVE message handler
	/// @param Flags Indicates the mouse buttons and keys that the user pressed
	/// @param Pt mouse pointer coords
	/// @return return value is obsolete
	LRESULT OnMouseMove(UINT nFlags, CPoint point)
	{
		if (m_UnderMouse) return FALSE;
		m_UnderMouse = true;
		TRACKMOUSEEVENT tm = { sizeof( TRACKMOUSEEVENT ) , TME_LEAVE, m_hWnd , 1 };
		// TODO: call events here
		Invalidate(FALSE);
		return ::TrackMouseEvent( &tm );
	}
	
	/// WM_MOUSELEAVE message handler
	/// @return return value is obsolete
	LRESULT OnMouseLeave(void)
	{
		m_UnderMouse = false;
		// TODO: call events here
		Invalidate(FALSE);
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
	LRESULT OnNChitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/// WM_LBUTTONUP message handler
	/// @param uMsg message code (expected to be WM_LBUTTONUP)
	/// @param wParam Indicates the mouse buttons and keys that the user pressed.
	/// @param lParam LOWORD - x mouse coordinate, HIWORD - y mouse coordinate
	/// @param bHandled set to TRUE if message should not be further processed
	/// @return return value is obsolete
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/// WM_LBUTTONDOWN message handler
	/// @param uMsg message code (expected to be WM_LBUTTONDOWN)
	/// @param wParam Indicates the mouse buttons and keys that the user pressed.
	/// @param lParam LOWORD - x mouse coordinate, HIWORD - y mouse coordinate
	/// @param bHandled set to TRUE if message should not be further processed
	/// @return return value is obsolete
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/// WM_LBUTTONDBLCLK message handler
	/// @param uMsg message code (expected to be WM_LBUTTONDBLCLK)
	/// @param wParam Indicates the mouse buttons and keys that the user pressed.
	/// @param lParam LOWORD - x mouse coordinate, HIWORD - y mouse coordinate
	/// @param bHandled set to TRUE if message should not be further processed
	/// @return return value is obsolete
	LRESULT OnLButronDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/// WM_SETTEXT message handler
	/// @param uMsg message code (expected to be WM_SETTEXT)
	/// @param wParam set to 0
	/// @param lParam lParam = (LPARAM)(LPCTSTR) lpsz;
	/// @param bHandled set to TRUE if message should not be further processed
	/// @return return value is obsolete
	LRESULT OnSetWindowText(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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


