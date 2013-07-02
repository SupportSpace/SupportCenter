#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#include <time.h>
HWND hwnd;
HINSTANCE hInst;
#ifndef LWA_COLORKEY 
	# define LWA_COLORKEY 1
#endif
#ifndef LWA_ALPHA
	# define LWA_ALPHA 2
#endif
#ifndef WS_EX_LAYERED
	# define WS_EX_LAYERED 0x80000
#endif


int wd=0;
int ht=0;
#include <AidLib/Strings/tstring.h>
#include "util.h"

#define BS_WND_CLASS_NAME "blackscreen"

HBITMAP
    DoGetBkGndBitmap2(
        IN CONST UINT uBmpResId
      )
    {
        static HBITMAP hbmBkGnd = NULL;
        if (NULL == hbmBkGnd)
        {
			hbmBkGnd = (HBITMAP)LoadImage(NULL, "background.bmp", IMAGE_BITMAP, 0, 0,LR_LOADFROMFILE);
			BITMAPINFOHEADER h2;
			h2.biSize=sizeof(h2);
			h2.biBitCount=0;
			// h2.biWidth=11; h2.biHeight=22; h2.biPlanes=1;
			HDC hxdc=CreateDC("DISPLAY",NULL,NULL,NULL);  
			GetDIBits(hxdc, hbmBkGnd, 0, 0, NULL, (BITMAPINFO*)&h2, DIB_RGB_COLORS);
			wd=h2.biWidth; ht=h2.biHeight;
			DeleteDC(hxdc);

           // hbmBkGnd = (HBITMAP)LoadImage(
           //     GetModuleHandle(NULL), MAKEINTRESOURCE(uBmpResId),
           //         IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
            if (NULL == hbmBkGnd)
                hbmBkGnd = (HBITMAP)-1;
        }
        return (hbmBkGnd == (HBITMAP)-1)
            ? NULL : hbmBkGnd;
    }
BOOL
    DoSDKEraseBkGnd2(
        IN CONST HDC hDC,
        IN CONST COLORREF crBkGndFill
      )
    {
        HBITMAP hbmBkGnd = DoGetBkGndBitmap2(0);
        if (hDC && hbmBkGnd)
        {
            RECT rc;
            if ((ERROR != GetClipBox(hDC, &rc)) && !IsRectEmpty(&rc))
            {
                HDC hdcMem = CreateCompatibleDC(hDC);
                if (hdcMem)
                {
                    HBRUSH hbrBkGnd = CreateSolidBrush(crBkGndFill);
                    if (hbrBkGnd)
                    {
                        HGDIOBJ hbrOld = SelectObject(hDC, hbrBkGnd);
                        if (hbrOld)
                        {
                            SIZE size = {
                                (rc.right-rc.left), (rc.bottom-rc.top)
                            };

                            if (PatBlt(hDC, rc.left, rc.top, size.cx, size.cy, PATCOPY))
                            {
                                HGDIOBJ hbmOld = SelectObject(hdcMem, hbmBkGnd);
                                if (hbmOld)
                                {
									StretchBlt(hDC,
										0,
										0,
										size.cx,
										size.cy,
                                        hdcMem,
										0,
										0,
										wd,
										ht,
										SRCCOPY);



  /*                                  BitBlt(hDC, rc.left, rc.top, size.cx, size.cy,
                                        hdcMem, rc.left, rc.top, SRCCOPY);*/
                                    SelectObject(hdcMem, hbmOld);
                                }
                            }
                            SelectObject(hDC, hbrOld);
                        }
                        DeleteObject(hbrBkGnd);
                    }
                    DeleteDC(hdcMem);
                }
            }
        }
        return TRUE;
    }


static LRESULT CALLBACK WndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam)    // second message parameter
{
    switch (uMsg)
    {
						case WM_CREATE:
//							SetTimer(hwnd,10,30000,NULL);
							SetTimer(hwnd,100,2000,NULL);
							break;
						case WM_TIMER:
							if (wParam==100) SetWindowPos(hwnd,HWND_TOPMOST,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN), NULL);
//							if (wParam==10) DestroyWindow(hwnd);
                         
						 case WM_ERASEBKGND:
							{
									DoSDKEraseBkGnd2((HDC)wParam, RGB(0,0,0));
									return true;
							}
						case WM_CTLCOLORSTATIC:
							{
									SetBkMode((HDC) wParam, TRANSPARENT);
									return GetStockObject(NULL_BRUSH) != 0;
							}
                         case WM_DESTROY:
								KillTimer(hwnd,100);
                                PostQuitMessage (0);
                                break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

bool
create_window(void)
{
	WNDCLASSEX wndClass;
	ZeroMemory (&wndClass, sizeof (wndClass));
	wndClass.cbSize        = sizeof (wndClass);
	wndClass.style         = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc   = WndProc;
	wndClass.cbClsExtra    = 0;
	wndClass.cbWndExtra    = 0;
	wndClass.hInstance     = hInst;
	wndClass.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
	wndClass.hIconSm       = NULL;
	wndClass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH) GetStockObject(GRAY_BRUSH);
	wndClass.lpszMenuName  = NULL;
	wndClass.lpszClassName = BS_WND_CLASS_NAME;

	RegisterClassForced(wndClass);

	RECT clientRect;
	clientRect.left = 0;
	clientRect.top = 0;
	clientRect.right = 640;
	clientRect.bottom = 480;
	AdjustWindowRect (&clientRect, WS_CAPTION, FALSE);

	hwnd = CreateWindowEx (0,
		"blackscreen",
		"blackscreen",
		WS_POPUP  | WS_CLIPSIBLINGS | WS_CLIPCHILDREN|WS_BORDER,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		clientRect.right - clientRect.left,
		clientRect.bottom - clientRect.top,
		NULL,
		NULL,
		hInst,
		NULL);
	typedef DWORD (WINAPI *PSLWA)(HWND, DWORD, BYTE, DWORD);

	PSLWA pSetLayeredWindowAttributes;
	/*
	* Code that follows allows the program to run in 
	* environment other than windows 2000
	* without crashing only difference being that 
	* there will be no transparency as 
	* the SetLayeredAttributes function is available only in
	* windows 2000
	*/
	HMODULE hDLL = LoadLibrary ("user32");
	pSetLayeredWindowAttributes = (PSLWA) GetProcAddress(hDLL,"SetLayeredWindowAttributes");
	/*
	* Make the windows a layered window
	*/
	LONG style = GetWindowLong(hwnd, GWL_STYLE);
	style = GetWindowLong(hwnd, GWL_STYLE);
	style &= ~(WS_DLGFRAME | WS_THICKFRAME);
	SetWindowLong(hwnd, GWL_STYLE, style);

	if (pSetLayeredWindowAttributes != NULL) {
		SetWindowLong (hwnd, GWL_EXSTYLE, GetWindowLong
			(hwnd, GWL_EXSTYLE) |WS_EX_LAYERED|WS_EX_TRANSPARENT );
		ShowWindow (hwnd, SW_SHOWNORMAL);
	}
	if (pSetLayeredWindowAttributes != NULL) {
		/**
		* Second parameter RGB(255,255,255) sets the colorkey to white
		* LWA_COLORKEY flag indicates that color key is valid
		* LWA_ALPHA indicates that ALphablend parameter (factor)
		* is valid
		*/
		pSetLayeredWindowAttributes (hwnd, RGB(255,255,255), 255, LWA_ALPHA);
	}
	SetWindowPos(hwnd,HWND_TOPMOST,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED|SWP_NOACTIVATE);
	//SM_CXVIRTUALSCREEN
	return true;
}

DWORD WINAPI BlackWindow(LPVOID lpParam)
{
 	// TODO: Place code here.
	create_window();
	MSG msg;
	while (GetMessage(&msg,0,0,0) != 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnregisterClass(BS_WND_CLASS_NAME, GetModuleHandle(NULL)/*handle to application instance*/);

	return 0;
}