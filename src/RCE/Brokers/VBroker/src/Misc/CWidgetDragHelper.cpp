/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CWidgetDragHelper.cpp
///
///  Class, for helping in DOJO DHTML widget dragging
///
///  @author "Archer Software" Sogin M. @date 10.04.2008
///
////////////////////////////////////////////////////////////////////////
#include "CWidgetDragHelper.h"
#include <atlbase.h>
#include <atlwin.h>

CWidgetDragHelper::CWidgetDragHelper()
	:	m_state(MOUSE_UP),
		m_window(NULL)
{
TRY_CATCH
CATCH_LOG()
}

int CWidgetDragHelper::Init(HWND window)
{
TRY_CATCH
	m_window = window;
CATCH_LOG()
	return 0;
}

inline BOOL CALLBACK EnumWindows2Map(HWND hWnd, LPARAM lParam)
{
TRY_CATCH

	/// Saving window rectangle to map
	std::map<HWND, RECT> *rectsMap = reinterpret_cast<std::map<HWND, RECT>*>(lParam);
	GetWindowRect(hWnd, &(*rectsMap)[hWnd]);	
	return TRUE;

CATCH_THROW()
}

void CWidgetDragHelper::MakeChildrenSnapShot()
{
TRY_CATCH

	m_childRectanglesMap.clear();
	EnumChildWindows(m_window, EnumWindows2Map, reinterpret_cast<LPARAM>(&m_childRectanglesMap));
	CWindow window(m_window);
	for(std::map<HWND, RECT>::iterator childRect = m_childRectanglesMap.begin();
		m_childRectanglesMap.end() != childRect;
		++childRect)
	{
		window.ScreenToClient(&childRect->second);
	}
CATCH_THROW()
}

bool CWidgetDragHelper::DragStarted()
{
TRY_CATCH

	RECT rc;
	CWindow window(m_window);
	for(std::map<HWND, RECT>::iterator childRect = m_childRectanglesMap.begin();
		m_childRectanglesMap.end() != childRect;
		++childRect)
	{
		CWindow childWindow(childRect->first);
		childWindow.GetWindowRect(&rc);
		window.ScreenToClient(&rc);
		if (0 != memcmp(&rc, &childRect->second, sizeof(rc)) )
			return true;
	}
	return false;

CATCH_THROW()
}

void CWidgetDragHelper::OnMouseDown()
{
TRY_CATCH
	switch(m_state)
	{
		case MOUSE_UP:
			m_state = MOUSE_DOWN;
			MakeChildrenSnapShot();
			break;
		case MOUSE_DOWN:
			throw MCException("Repeated MouseDown without Mouse UP");
		case MOUSE_CAPTURED:
			throw MCException("MouseDown in captured state");
		default:
			throw MCException(Format("Unknown internal state %X",m_state));
	}
CATCH_LOG()
}

void CWidgetDragHelper::OnMouseUp()
{
TRY_CATCH
	switch(m_state)
	{
		case MOUSE_UP:
			throw MCException("MouseUP without mouse down");
		case MOUSE_DOWN:
			m_state = MOUSE_UP;
			break;
		case MOUSE_CAPTURED:
			ReleaseCapture();
			m_state = MOUSE_UP;
			break;
		default:
			throw MCException(Format("Unknown internal state %X",m_state));
	}
CATCH_LOG()
}

void CWidgetDragHelper::OnMouseMove()
{
TRY_CATCH
	switch(m_state)
	{
		case MOUSE_UP:
			break;
		case MOUSE_DOWN:
			if (DragStarted())
			{
				SetCapture(m_window);
				m_state = MOUSE_CAPTURED;
			}
			break;
		case MOUSE_CAPTURED:
			break;
		default:
			throw MCException(Format("Unknown internal state %X",m_state));
	}
CATCH_LOG()
}

