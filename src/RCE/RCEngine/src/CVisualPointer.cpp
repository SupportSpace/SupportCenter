/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CVisualPointer.h
///
///  visual pointer class
///
///  @author "Archer Software" Sogin M. @date 20.09.2006
///
////////////////////////////////////////////////////////////////////////
#include "cvisualpointer.h"
#include "RCE_resource.h"
#include <AidLib/CCritSection/CCritSection.h>
#include <AidLib/CThread/CThreadLS.h>
#include "util.h"

///Taked from http://www.codeproject.com/dialog/BitmapHandling.asp
BOOL CVisualPointer::_SCursorData::BitMapContourToWinRgn( HBITMAP hBmp, HRGN& hRgn, COLORREF cTolerance )
{// JES version making region without the use of first making a file.

	hRgn = NULL;	
    
	//COLORREF cTransparentColor = TranslateColor(m_transColor);
	//COLORREF cTransparentColor = m_skinTransColor;
	//COLORREF cTransparentColor = RGB(m_transColorR,m_transColorG,m_transColorB);
	//COLORREF cTransparentColor = RGB(255,255,255);
	// ORIGINAL VALUE ->COLORREF cTolerance = 0x101010;
	//COLORREF cTolerance = TranslateColor(m_transTolerance);

	if (hBmp)
	{
		// Create memory DC inside which we will scan bitmap content
		HDC hMemDC = CreateCompatibleDC(NULL);
		if (hMemDC)
		{
			// Receive bitmap size
			BITMAP bm;
			GetObject(hBmp, sizeof(bm), &bm);

			// Create 32 bits depth bitmap and select it into memory DC 
			BITMAPINFOHEADER RGB32BITSBITMAPINFO = {	
					sizeof(BITMAPINFOHEADER),	// biSize 
					bm.bmWidth,					// biWidth; 
					bm.bmHeight,				// biHeight; 
					1,							// biPlanes; 
					32,							// biBitCount 
					BI_RGB,						// biCompression; 
					0,							// biSizeImage; 
					0,							// biXPelsPerMeter; 
					0,							// biYPelsPerMeter; 
					0,							// biClrUsed; 
					0							// biClrImportant; 
			};
			VOID * pbits32; 
			HBITMAP hbm32 = CreateDIBSection(hMemDC, (BITMAPINFO *)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, NULL, 0);
			if (hbm32)
			{
				HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);

				// Create DC so we can copy bitmap into memory DC
				HDC hDC = CreateCompatibleDC(hMemDC);
				if (hDC)
				{
					// Receive bytes per row for bitmap bits (rounded up to 32 bits)
					BITMAP bm32;
					GetObject(hbm32, sizeof(bm32), &bm32);
					while (bm32.bmWidthBytes % 4)
						bm32.bmWidthBytes++;

					// Copy bitmap into memory DC
					HBITMAP holdBmp = (HBITMAP)SelectObject(hDC, hBmp);
					BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);

					// Receive color of pixel in upper-left hand corner for transparent color
					COLORREF cTransparentColor = GetPixel(hMemDC, 0, 0); 

					#define ALLOC_UNIT	100
					DWORD maxRects = ALLOC_UNIT;
					HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
					RGNDATA *pData = (RGNDATA *)GlobalLock(hData);
					pData->rdh.dwSize = sizeof(RGNDATAHEADER);
					pData->rdh.iType = RDH_RECTANGLES;
					pData->rdh.nCount = pData->rdh.nRgnSize = 0;
					SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

					BYTE lr = max(0, GetRValue(cTransparentColor) - GetRValue(cTolerance)); 
					BYTE lg = max(0, GetGValue(cTransparentColor) - GetGValue(cTolerance)); 
					BYTE lb = max(0, GetBValue(cTransparentColor) - GetBValue(cTolerance)); 
					BYTE hr = min(0xff, GetRValue(cTransparentColor) + GetRValue(cTolerance)); 
					BYTE hg = min(0xff, GetGValue(cTransparentColor) + GetGValue(cTolerance)); 
					BYTE hb = min(0xff, GetBValue(cTransparentColor) + GetBValue(cTolerance)); 

					// Scan each bitmap row from bottom to top (the bitmap is inverted vertically)
					BYTE *p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;
					for (int y = 0; y < bm.bmHeight; y++)
					{
						// Scan each bitmap pixel from left to right
						for (int x = 0; x < bm.bmWidth; x++)
						{
							// Search for continuous range of "non-transparent pixels"
							int x0 = x;
							LONG *p = (LONG *)p32 + x;
							while (x < bm.bmWidth)
							{
								BYTE b = ((RGBQUAD  *)  p)->rgbRed;
								if (b >= lr && b <= hr)
								{
									b = ((RGBQUAD  *)  p)->rgbGreen;
									if (b >= lg && b <= hg)
									{
										b = ((RGBQUAD  *)  p)->rgbBlue;
										if (b >= lb && b <= hb)
											// This pixel is "transparent"
											break;
									}
								}
								p++;
								x++;
							}

							if (x > x0)
							{
								// Add pixels (x0, y) to (x, y+1) as new rectangle in region
								if (pData->rdh.nCount >= maxRects)
								{
									GlobalUnlock(hData);
									maxRects += ALLOC_UNIT;
									hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE);
									pData = (RGNDATA *)GlobalLock(hData);
								}
								RECT *pr = (RECT *)&pData->Buffer;
								SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
								if (x0 < pData->rdh.rcBound.left)
									pData->rdh.rcBound.left = x0;
								if (y < pData->rdh.rcBound.top)
									pData->rdh.rcBound.top = y;
								if (x > pData->rdh.rcBound.right)
									pData->rdh.rcBound.right = x;
								if (y+1 > pData->rdh.rcBound.bottom)
									pData->rdh.rcBound.bottom = y+1;
								pData->rdh.nCount++;

								// On Windows98, ExtCreateRegion() may fail if number of rectangles
								// is too large (ie: > 4000), so create region by multiple steps.
								if (pData->rdh.nCount == 2000)
								{
									HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
									if (hRgn)
									{
										CombineRgn(hRgn, hRgn, h, RGN_OR);
										DeleteObject(h);
									}
									else
										hRgn = h;
									pData->rdh.nCount = 0;
									SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
								}
							}
						}

						// Go to next row (note: bitmap is inverted vertically)
						p32 -= bm32.bmWidthBytes;
					}

					// Create or extend the region with remaining rectangles
					HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
					if (hRgn)
					{
						CombineRgn(hRgn, hRgn, h, RGN_OR);
						DeleteObject(h);
					}
					else
						hRgn = h;

					// CLEAN UP COMPLETELY TO PREVENT MEMORY LEAK!
			        GlobalFree(hData);
					SelectObject(hDC, holdBmp);
					DeleteDC(hDC);
				}

				DeleteObject(SelectObject(hMemDC, holdBmp));
			}

			DeleteDC(hMemDC);
		}	
	}

	//if (hBmp){DeleteObject(hBmp);}

	if ( !hRgn ){return FALSE;}

    //NOTE: win32-based apps do not need to unlock or free loaded resources (not manually) 

    return TRUE; 
}


unsigned int CVisualPointer::m_setStateMsg;
unsigned int CVisualPointer::m_MoveToMsg;
unsigned int CVisualPointer::m_showWindowMsg;
unsigned int CVisualPointer::m_hideWindowMsg;

CVisualPointer::CVisualPointer(void) 
	:	CThread(), 
		m_state(RBTN_DOWN), 
		m_clickProlonged(false),
		m_updateNeeded(false),
		m_desktopThreadId(-1)
{
TRY_CATCH

	InitializeCriticalSection(&m_cs);

	/// creating bitmaps
	m_cursors[NORMAL] = SCursorData(LoadBitmap(::GetCurrentModule(),MAKEINTRESOURCE(IDB_CURSOR)));
	m_cursors[RBTN_DOWN] = SCursorData(LoadBitmap(::GetCurrentModule(),MAKEINTRESOURCE(IDB_CURSOR_RC)));
	m_cursors[LBTN_DOWN] = SCursorData(LoadBitmap(::GetCurrentModule(),MAKEINTRESOURCE(IDB_CURSOR_LC)));

	m_hWndCreatedEvent = CreateEvent(NULL,true,false,NULL);
	Start();

	m_setStateMsg = RegisterWindowMessage(_T("CVisualPointer::m_setStateMsg"));
	m_MoveToMsg = RegisterWindowMessage(_T("CVisualPointer::m_MoveToMsg"));
	m_showWindowMsg = RegisterWindowMessage(_T("CVisualPointer::m_showWindowMsg"));
	m_hideWindowMsg = RegisterWindowMessage(_T("CVisualPointer::m_hideWindowMsg"));

	if (WAIT_OBJECT_0 != WaitForSingleObject(m_hWndCreatedEvent, WAIT_VP_INNIT_TIMEOUT))
		throw MCException_Win("Internal thread creation timed out");

CATCH_LOG("CVisualPointer::CVisualPointer")
}

CVisualPointer::~CVisualPointer(void)
{
TRY_CATCH

	Terminate();
	PostMessage(m_hWnd,WM_DESTROY,0,0);
	DeleteCriticalSection(&m_cs);
	CloseHandle(m_hWndCreatedEvent);
	UnregisterClass(VP_CLASS_NAME, GetModuleHandle(NULL));

CATCH_LOG("CVisualPointer::~CVisualPointer")
}

void CVisualPointer::Execute(void *Params)
{
	SET_THREAD_LS;

TRY_CATCH

	//Creating visual pointer window
	WNDCLASSEX wcx; 
	memset(&wcx,0,sizeof(wcx));
	// Fill in the window class structure with parameters 
	// that describe the main window. 
	wcx.cbSize = sizeof(wcx);				// size of structure 
	wcx.style = CS_HREDRAW | 
	CS_VREDRAW;								// redraw if size changes 
	wcx.lpfnWndProc = MainWndProc;			// points to window procedure 
	wcx.cbClsExtra = 0;						// no extra class memory 
	wcx.cbWndExtra = 0;						// no extra window memory 
	wcx.hInstance = GetModuleHandle(NULL);	// handle to instance 
	wcx.hIcon = NULL;
	wcx.hCursor = LoadCursor(NULL,IDC_ARROW);	// predefined arrow 
	wcx.hbrBackground = NULL;
	wcx.lpszMenuName =  NULL;				// name of menu resource 
	wcx.lpszClassName = VP_CLASS_NAME;		// name of window class 

	// Register the window class
	RegisterClassForced(wcx);

	m_hWnd = CreateWindowEx(WS_EX_TOOLWINDOW,
							wcx.lpszClassName,			// name of window class 
							"Sample",					// title-bar string 
							WS_POPUP ,					// top-level window 
							CW_USEDEFAULT,				// default horizontal position 
							CW_USEDEFAULT,				// default vertical position 
							15,							// default width 
							15,							// default height 
							(HWND) NULL,				// no owner window 
							(HMENU) NULL,				// use class menu 
							GetModuleHandle(NULL),		// handle to application instance 
							(LPVOID) NULL);				// no window-creation data 
 
	if (!m_hWnd) 
		throw MCException_Win("Failed to CreateWindow");


	SetStateInternal(NORMAL);
	SetEvent(m_hWndCreatedEvent);

	MSG msg; 
	BOOL fGotMessage;
	/// Raising pointer thread priority
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
	while(	!Terminated() 
			&&
			(fGotMessage = GetMessage(&msg, m_hWnd, 0, 0)) != 0 && fGotMessage != -1) 
	{ 
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
	} 

CATCH_LOG("CVisualPointer::Execute")
}

void CVisualPointer::Show()
{
TRY_CATCH
	PostMessage(m_hWnd , m_showWindowMsg, 0, 0);
CATCH_THROW("CVisualPointer::Show")
}

void CVisualPointer::Hide()
{
TRY_CATCH
	PostMessage(m_hWnd , m_hideWindowMsg, 0, 0);
CATCH_THROW("CVisualPointer::Hide")
}

LRESULT CALLBACK CVisualPointer::MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
TRY_CATCH
	if (msg == m_setStateMsg)
	{
		VISUAL_POINTER_INSTANCE.SetStateInternal(static_cast<EState>(wParam));
		return TRUE;
	} else
	if (msg == m_MoveToMsg)
	{
		VISUAL_POINTER_INSTANCE.MoveToInternal(static_cast<int>(wParam),static_cast<int>(lParam));
		return TRUE;
	} else
	if (msg == m_showWindowMsg)
	{
		if (FALSE == IsWindowVisible(hWnd))
		{
			// Disabling vncHooks
			PostThreadMessage(VISUAL_POINTER_INSTANCE.m_desktopThreadId, VISUAL_POINTER_INSTANCE.m_noticeVisibleMsg, 1 /*shown*/, 0);
			ShowWindow(hWnd, SW_SHOW); 
		}
		SetWindowPos(hWnd, HWND_TOPMOST,0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
		UpdateWindow(hWnd); 
		return TRUE;
	} else
	if (msg == m_hideWindowMsg)
	{
		if (TRUE == IsWindowVisible(hWnd))
		{
			// Enabling vncHooks
			PostThreadMessage(VISUAL_POINTER_INSTANCE.m_desktopThreadId, VISUAL_POINTER_INSTANCE.m_noticeVisibleMsg, FALSE /*hidden*/, 0);
			ShowWindow(hWnd, SW_HIDE); 
		}
		return TRUE;
	}
	switch(msg)
	{
		case WM_TIMER:
			VISUAL_POINTER_INSTANCE.OnTimer(static_cast<int>(wParam));
			return TRUE;
		case WM_NCPAINT:
		case WM_PAINT:
		case WM_ERASEBKGND:
			VISUAL_POINTER_INSTANCE.OnPaint(NULL);
			return TRUE;
	}
CATCH_LOG("CVisualPointer::MainWndProc")
TRY_CATCH
	return DefWindowProc(hWnd,msg,wParam,lParam);
CATCH_LOG("CVisualPointer::MainWndProc DefWindowProc")
	return 0;
}

void CVisualPointer::OnTimer(int id)
{
TRY_CATCH
	if (id != CLICK_TIMER_ID) return;
	KillTimer(m_hWnd,id);
	m_clickProlonged = false;
	if (m_clicked)
		SetStateInternal(NORMAL);
CATCH_THROW("CVisualPointer::OnTimer")
}

void CVisualPointer::MoveToInternal(const int x, const int y)
{
TRY_CATCH
	//MoveWindow(m_hWnd,x,y,m_currentCursor.size.cx,m_currentCursor.size.cy,TRUE);
	SetWindowPos(m_hWnd, HWND_TOPMOST,x,y,0,0, SWP_NOSIZE);
	UpdateWindow(m_hWnd);
	m_updateNeeded = true;
CATCH_THROW("CVisualPointer::MoveToInternal")
}

void CVisualPointer::SetStateInternal(EState state)
{
TRY_CATCH
	if (m_state == state)
		return;
	if (state != NORMAL)
	{
		m_clickProlonged = true;
		m_clicked = false;
		///Setting corresponding timer
		SetTimer(m_hWnd,CLICK_TIMER_ID,MINIMAL_CLICK,NULL);
	}
	else
		if (m_clickProlonged)
		{
			m_clicked = true;
			return;
		}
	m_state = state;
	m_currentCursor = m_cursors[m_state];
	RECT rc;
	GetWindowRect(m_hWnd,&rc);
	MoveWindow(m_hWnd,rc.left,rc.top,m_currentCursor.size.cx,m_currentCursor.size.cy,TRUE);
	SetWindowRgn(m_hWnd, m_currentCursor.hRegion, TRUE);
	InvalidateRect(m_hWnd,NULL,TRUE);
	UpdateWindow(m_hWnd);
	m_updateNeeded = true;
CATCH_THROW("CVisualPointer::SetStateInternal")
}

void CVisualPointer::OnPaint(HDC hDc)
{
TRY_CATCH
	PAINTSTRUCT ps;
	HDC hdc = hDc?hDc:BeginPaint(m_hWnd,&ps);
	HDC hdcCompatible = CreateCompatibleDC(hdc); 
	HBITMAP hOldBmp = static_cast<HBITMAP>(SelectObject(hdcCompatible, m_currentCursor.hBitmap));
	BitBlt(hdc,0,0,m_currentCursor.size.cx,m_currentCursor.size.cy,hdcCompatible,0,0,SRCCOPY);
	EndPaint(m_hWnd,&ps);
	SelectObject(hdcCompatible, hOldBmp);
	ReleaseDC(m_hWnd,hdcCompatible);
CATCH_THROW("CVisualPointer::OnPaint")
}

void CVisualPointer::MoveTo(const int x, const int y)
{
TRY_CATCH
	PostMessage(m_hWnd,m_MoveToMsg,x,y);
CATCH_THROW("CVisualPointer::MoveTo")
}

void CVisualPointer::SetState(const EState state)
{
TRY_CATCH
	PostMessage(m_hWnd,m_setStateMsg,state,0);
CATCH_THROW("CVisualPointer::SetState")
}

bool CVisualPointer::UpdateNeeded()
{
TRY_CATCH
	bool res(m_updateNeeded && IsWindow(m_hWnd) && IsWindowVisible(m_hWnd));
	m_updateNeeded = false;
	return res;
CATCH_THROW("CVisualPointer::UpdateNeeded")
}

void CVisualPointer::SetDesktopThreadId(unsigned int desktopThreadId)
{
TRY_CATCH
	m_desktopThreadId = desktopThreadId;
CATCH_THROW()
}

void CVisualPointer::SetNoticeVisibleMsg(unsigned int noticeVisibleMsg)
{
TRY_CATCH
	m_noticeVisibleMsg = noticeVisibleMsg;
CATCH_THROW()
}
