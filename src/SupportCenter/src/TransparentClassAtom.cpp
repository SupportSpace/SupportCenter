#include "StdAfx.h"
#include "TransparentClassAtom.h"
#include "TrasparentWindow.h"

CTransparentClassAtom::CTransparentClassAtom(HINSTANCE Instance)
: className(_T("Transparent Class")),hInstance(Instance)
{
	if (CreateWinClass() == 0)
	{
	  throw new tstring(_T("Cannot crate class"));

	}
}

CTransparentClassAtom::~CTransparentClassAtom(void)
{
	UnregisterClass(className.c_str(),hInstance);
}


ATOM CTransparentClassAtom::CreateWinClass(void)
{
	 WNDCLASS wndClass;

	 //TBD
	 wndClass.style = CS_NOCLOSE;
	 wndClass.lpfnWndProc = CTransparentWindow::WindowProc;
	 wndClass.cbClsExtra = 0;
	 wndClass.cbWndExtra = 0;
	 wndClass.hInstance = hInstance;
	 wndClass.hIcon = NULL;
	 wndClass.hCursor = NULL;
	 wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	 wndClass.lpszMenuName = NULL;
	 wndClass.lpszClassName = className.c_str();

	 return RegisterClass(&wndClass);
}
