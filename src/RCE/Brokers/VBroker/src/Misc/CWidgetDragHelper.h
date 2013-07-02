#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CWidgetDragHelper.h
///
///  Class, for helping in DOJO DHTML widget dragging
///
///  @author "Archer Software" Sogin M. @date 10.04.2008
///
////////////////////////////////////////////////////////////////////////
#include <AidLib/CException/CException.h>
#include <map>

/// Class, for helping in DOJO DHTML widget dragging
class CWidgetDragHelper
{
protected:
	/// Possible object states
	typedef enum _EState
	{
		MOUSE_UP,
		MOUSE_DOWN,
		MOUSE_CAPTURED
	} EState;

	/// Object invariant
	EState m_state;

	/// Root window for drag whatching
	HWND m_window;

	/// Map of rectangles for all root window children
	std::map<HWND, RECT> m_childRectanglesMap;

	/// Refills child window rectangles map
	void MakeChildrenSnapShot();

	/// Checks if drag of any child window was started
	/// @rerutn true if any child window is now dragging
	bool DragStarted();

public:
	/// ctor
	CWidgetDragHelper();

	/// Initializes class for new root window
	/// @param window new root window handle
	int Init(HWND window);

	/// Mouse down event handler
	void OnMouseDown();

	/// Mouse up event handler
	void OnMouseUp();

	/// Mouse move event handler
	void OnMouseMove();
};