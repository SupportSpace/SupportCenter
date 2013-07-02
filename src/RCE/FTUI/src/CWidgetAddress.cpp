//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetAddress.cpp
///
///  Implements CWidgetAddress class
///  
///  
///  @author Alexander Novak @date 23.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWidgetAddress.h"
#include <boost/scoped_ptr.hpp>

// CWidgetAddress [BEGIN] ////////////////////////////////////////////////////////////////////////////////

CWidgetAddress::CWidgetAddress(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager)
	:	CCommandProxy(commandManager)
{
TRY_CATCH

	Create(hParentWnd, 0, NULL, WS_CHILD|WS_VISIBLE|WS_BORDER|ES_READONLY);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetAddress::~CWidgetAddress()
{
TRY_CATCH

	DestroyWindow();

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetAddress::ResizeWidget(LPRECT rect)
{
TRY_CATCH

	MoveWindow(rect,TRUE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetAddress::SetAddress(const tstring& address)
{
TRY_CATCH

	SetWindowText(address.c_str());	

CATCH_THROW()
}

//--------------------------------------------------------------------------------------------------------

tstring CWidgetAddress::GetAddress()
{
TRY_CATCH

	tstring address;

	int szBuffer = GetWindowTextLength() + 1;
	if ( szBuffer > 1 )
	{
		boost::scoped_ptr<TCHAR> strBuffer( new TCHAR[szBuffer] );
		if ( GetWindowText(strBuffer.get(), szBuffer) )
			address = strBuffer.get();
	}

	return address;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetAddress::OnEraseBkgnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	HDC hdc=reinterpret_cast<HDC>(wParam);
	PAINTSTRUCT m_ps;
	HDC hDC(hdc?hdc:(BeginPaint(&m_ps)));
	RECT windowRect;
	GetWindowRect(&windowRect);
	ScreenToClient(&windowRect);
	LRESULT res = 0L;
	bHandled=TRUE;
	HPEN pen = CreatePen(PS_SOLID, 1, EdgeColor1);
	HPEN penOld = (HPEN)SelectObject(hDC, pen);
	res = Rectangle(hDC, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
	SelectObject(hDC, penOld);
	DeleteObject(pen);
	if (!hdc) 
		EndPaint(&m_ps);
	return res;
CATCH_LOG()
	bHandled=FALSE;
	return 0;
}

LRESULT CWidgetAddress::OnPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	LRESULT res = OnEraseBkgnd(hWnd, uMsg, wParam, lParam, bHandled);

	// Drawing original ComboBox item
	RECT rc;
	GetClientRect(&rc);
	InvalidateRect(&rc, FALSE);
	DefWindowProc(WM_PAINT,wParam,lParam);
	return res;
CATCH_THROW()
}

LRESULT CWidgetAddress::OnPrint(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	RECT rc;
	GetClientRect(&rc);
	CMemoryDC memDC(reinterpret_cast<HDC>(wParam), rc);
	// Drawing original ComboBox item
	DefWindowProc(WM_PRINT,reinterpret_cast<WPARAM>(memDC.m_hDC),lParam);
	memDC.ExcludeClipRect(rc.left,rc.top,rc.right,rc.bottom);
	return OnEraseBkgnd(hWnd, uMsg,reinterpret_cast<WPARAM>(memDC.m_hDC), lParam, bHandled);
CATCH_THROW()
}

LRESULT CWidgetAddress::OnNCPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	return OnPaint(hWnd, uMsg, wParam, lParam, bHandled);
CATCH_THROW()
}


void CWidgetAddress::SetLogFont(LOGFONT newVal)
{
TRY_CATCH
	CSkinnedElement::SetLogFont(newVal);
	CFontHandle hFont;
	hFont.CreateFontIndirect(&newVal);
	SetFont(hFont,FALSE);
	m_hFont=hFont;
CATCH_THROW()
}

LRESULT CWidgetAddress::OnColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	return RGB(255,255,255);
CATCH_THROW()
}

// CWidgetAddress [END] //////////////////////////////////////////////////////////////////////////////////
