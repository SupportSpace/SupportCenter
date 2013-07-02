#include "StdAfx.h"
#include "TrasparentWindow.h"
#include "Resource.h"
#include "SupportMessenger.h"
#include <AidLib/Logging/cLog.h> 

#define FADE_TIMER_ID	0xDeedBeef

//CTransparentWindow*  CTransparentWindow::self = NULL;
TransparentWindowsMap CTransparentWindow::self_map;
const int CTransparentWindow::HORIZONTAL_BORDER_WIDTH = 219;
const int CTransparentWindow::HORIZONTAL_BORDER_HEIGHT = 12;
const int CTransparentWindow::HORIZONTAL_BORDER_FADE_TOP = 2;
const int CTransparentWindow::HORIZONTAL_BORDER_FADE_BOTTOM = 7;
const int CTransparentWindow::VERTICAL_BORDER_WIDTH = 5;


CTransparentWindow::CTransparentWindow(HINSTANCE instance,int x,int y, int height,int width)
: m_className(_T("Transparent Class")),m_hInstance(instance)
, m_iTransparency(0xff00ff),m_iOpacity(225)
, m_bMonitorMove(true)
, FADE_IDLE_TIMEOUT(10*1000)
, FADE_TIMEOUT(30),m_iFadingLevel(50),m_eFadeState(FADE_IN_PROGRESS),m_bFadeAscending(false)
{
	//if (CreateWinClass() == 0)
	//{
	//  throw new string("Cannot crate class");
	//}
	//WS_EX_LAYERED
	//
	m_hWnd = CreateWindowEx( WS_EX_LAYERED | WS_EX_NOACTIVATE , m_className.c_str() ,NULL, 0/*WS_VISIBLE*/,
				x-VERTICAL_BORDER_WIDTH,y-HORIZONTAL_BORDER_HEIGHT,
				width + 2*VERTICAL_BORDER_WIDTH, height + 2*HORIZONTAL_BORDER_HEIGHT,
				NULL,
				NULL,
				instance,
				NULL);

	AddToMap(m_hWnd, this);

	if (m_hWnd == INVALID_HANDLE_VALUE)
	{
		throw new tstring(_T("Cannot Create Window"));
	}
	// Configurating Opacity and Transparency
	SetLayeredWindowAttributes(m_hWnd, m_iTransparency, m_iOpacity,LWA_COLORKEY | LWA_ALPHA);
	LONG lWndStyle = GetWindowLong(m_hWnd,GWL_STYLE);
	lWndStyle &= ~(WS_CAPTION |WS_BORDER);
	lWndStyle |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	SetWindowLong(m_hWnd,GWL_STYLE ,lWndStyle);

	
	
	//Load Resources
	m_hBottomBmp =  LoadBitmap(instance,MAKEINTRESOURCE(IDB_BITMAP2));
	m_hTopBmp=  LoadBitmap(instance,MAKEINTRESOURCE(IDB_BITMAP1));
	m_hMiddleBmp=  LoadBitmap(instance,MAKEINTRESOURCE(IDB_BITMAP3));
	if (m_hMiddleBmp == INVALID_HANDLE_VALUE || 
		m_hTopBmp == INVALID_HANDLE_VALUE || 
		m_hBottomBmp == INVALID_HANDLE_VALUE)
	{
		throw new tstring(_T("cannot load bitmap"));
	}

	m_uiLastEvent= GetTickCount();
	//Aim Fade timer
	m_nIDFadeEvent = ::SetTimer(m_hWnd,(UINT_PTR)FADE_TIMER_ID, FADE_IDLE_TIMEOUT, NULL);
}


CTransparentWindow::~CTransparentWindow(void)
{
	Log.Add(_MESSAGE_, _T("CTransparentWindow::~CTransparentWindow" ));
	DeleteObject(m_hBottomBmp);
	DeleteObject(m_hTopBmp);
	DeleteObject(m_hMiddleBmp);

	//UnregisterClass(className.c_str(),hInstance);
	DeleteFromMap(m_hWnd);
	DestroyWindow(m_hWnd);
}


HWND CTransparentWindow::GetWindowHandle(void)
{
	return m_hWnd;
}


void CTransparentWindow::SetChildWindow(HWND hWindow)
{
	m_hChildWindow = hWindow; 
	FadeTimerOn();
}

void CTransparentWindow::AddToMap( HWND hWnd, CTransparentWindow* pcTransWin )
{ 
	Log.Add(_MESSAGE_, _T("CTransparentWindow::AddToMap")) ;
	self_map[hWnd]  = pcTransWin;
}

void CTransparentWindow::DeleteFromMap( HWND hWnd )
{ 
	CTransparentWindow*				  self = NULL;
	TransparentWindowsMap::iterator   it = self_map.find( hWnd );

	if( it != self_map.end() )
	{
		Log.Add(_MESSAGE_, _T("CTransparentWindow::DeleteFromMap")) ;	
		self = (*it).second;
		self_map.erase( it );
	}
}
bool CTransparentWindow::MouseHitTest()
{
	DWORD msPos = GetMessagePos();

	RECT rc;
	GetWindowRect(m_hWnd,&rc);

	int iMouse_x =  GET_X_LPARAM (msPos);
	int iMouse_y =  GET_Y_LPARAM (msPos);

	if ( iMouse_x >= rc.left && iMouse_x < rc.right)
		if (iMouse_y >= rc.top && iMouse_y <= rc.bottom)
			return true;
	return false;
}
void CTransparentWindow::FadeTimer(unsigned int uiTimer)
{
	KillTimer(m_hWnd,FADE_TIMER_ID);
	SetTimer(m_hWnd,FADE_TIMER_ID,uiTimer,NULL);
}
void CTransparentWindow::FadeTimerOn()
{
	KillTimer(m_hWnd,FADE_TIMER_ID);
	SetTimer(m_hWnd,FADE_TIMER_ID,FADE_TIMEOUT,NULL);
}
void CTransparentWindow::FadeTimerOff()
{
	KillTimer(m_hWnd,FADE_TIMER_ID);
	SetTimer(m_hWnd,FADE_TIMER_ID,FADE_IDLE_TIMEOUT,NULL);
}

bool CTransparentWindow::FadeWindow(bool Ascending)
{
	if (Ascending)
		m_iFadingLevel --;
	else
		m_iFadingLevel ++;

	if (m_iFadingLevel < 50)
		SetLayeredWindowAttributes(m_hWnd, m_iTransparency, m_iFadingLevel*m_iOpacity/50,LWA_COLORKEY | LWA_ALPHA);

	// Configurating Opacity and Transparency
	SetLayeredWindowAttributes(m_hChildWindow, 0, 255 * m_iFadingLevel/100, LWA_ALPHA);
	if (m_iFadingLevel == 100 || m_iFadingLevel == 0 || m_iFadingLevel == 50)
		return false;
	return true;
}

void CTransparentWindow::CloseWindow()
{
}

LRESULT CALLBACK CTransparentWindow::WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	CTransparentWindow* self = CTransparentWindow::GetWindow(hwnd);

	Log.Add(_MESSAGE_, _T("EVENT: %d"),uMsg);
	
	//::ShowWindow(hwnd, SW_HIDE);

	if ( self && ((uMsg != WM_TIMER) || ((UINT_PTR)wParam) != FADE_TIMER_ID) && (uMsg < WM_USER))
	{
		self->m_bFadeAscending = false;
		if (self->m_eFadeState == FADE_COMPLETED) 
			self->FadeTimerOn();
		if (self->m_iFadingLevel < 50)
		{
			self->m_iFadingLevel = 51;
			SetLayeredWindowAttributes(self->m_hWnd, self->m_iTransparency, self->m_iOpacity,LWA_COLORKEY | LWA_ALPHA);
		}
		self->m_uiLastEvent = GetTickCount();
		Log.Add(_MESSAGE_, _T("New Count down started cause: %d"),uMsg);	
		
	}
	else
	{
		//Log.Add(_MESSAGE_, _T("Just new Event: %d"),uMsg);	
		if(uMsg == 49442)
			Log.Add(_MESSAGE_, _T("HELLO from Skype: %d"),uMsg);	
	}
	
	switch (uMsg)
	{
	case WM_TIMER:
		if ( (UINT_PTR)wParam != FADE_TIMER_ID)
			return TRUE;

		OnFadeTimer(self);
			return TRUE;

	 case WM_NCHITTEST:
		 {
			 LRESULT res = OnNCHitTest(hwnd,self,lParam);
			 Log.Add(_MESSAGE_, _T("OnNCHitTest result: %d"),res ) ;	
  			 if (res == HTERROR)
				 break;
	 		return res;
		 }
	case WM_PAINT:
		OnPaint(hwnd,self);
		return TRUE;
	case WM_USER_MOVE_ACTIVITIES:
	{
		Log.Add(_MESSAGE_, _T("CTransparentWindow::WindowProc WM_USER_MOVE_ACTIVITIES") );
		self->m_bMonitorMove = (bool) lParam;
		return TRUE;
	}
	 case WM_MOVING:
		OnMoving(hwnd,self, *((RECT*)lParam));
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
		//return FALSE;
	case WM_SIZING:
		OnSizing(hwnd,self, *((RECT*)lParam));
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	case WM_USER_BORDER_MOVE:
		OnUserMove(hwnd,(LPRECT)lParam);
		return TRUE;

	case WM_DESTROY:
		//PostQuitMessage(0);
		break;

	case WM_CREATE:
		 ::SetWindowPos(hwnd ,       // handle to window
                HWND_TOPMOST,		 // placement-order handle
                0,					 // horizontal position
                0,					 // vertical position
                0,					 // width
                0,					 // height
                SWP_HIDEWINDOW);	 // window-positioning options - create invisible then show it
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void  CTransparentWindow::OnPaint(HWND hwnd,CTransparentWindow* self)
{

		HDC hdc,hBmpDC;
		PAINTSTRUCT ps;
		RECT rc;
		int width;
		int height;

		if (self == NULL)
			return;

		GetWindowRect(hwnd,&rc);

		width = rc.right-rc.left;
		height = rc.bottom - rc.top;


		hdc = BeginPaint(hwnd, &ps);

		hBmpDC = CreateCompatibleDC(hdc); 

		HGDIOBJ hOldBmp =  SelectObject(hBmpDC,self->m_hBottomBmp);
		StretchBlt(hdc, 0, height-self->HORIZONTAL_BORDER_HEIGHT,
			   width, self->HORIZONTAL_BORDER_HEIGHT,
			   hBmpDC, 0, 0, self->HORIZONTAL_BORDER_WIDTH, self->HORIZONTAL_BORDER_HEIGHT,SRCCOPY); 
		
		SelectObject(hBmpDC,self->m_hTopBmp);
		StretchBlt(hdc, 0, 0, 
				width, self->HORIZONTAL_BORDER_HEIGHT, 
				hBmpDC, 0, 0, self->HORIZONTAL_BORDER_WIDTH, self->HORIZONTAL_BORDER_HEIGHT,SRCCOPY); 

	
		SelectObject(hBmpDC,self->m_hMiddleBmp);
		StretchBlt(hdc, 0, self->HORIZONTAL_BORDER_HEIGHT,
				   width, height- 2*self->HORIZONTAL_BORDER_HEIGHT, hBmpDC, 0, 0, 
				   self->HORIZONTAL_BORDER_WIDTH, self->HORIZONTAL_BORDER_HEIGHT, SRCCOPY); 


		SelectObject(hBmpDC, hOldBmp);
		DeleteDC(hBmpDC);

		EndPaint(hwnd, &ps);
}

void  CTransparentWindow::OnMoving(HWND hwnd,CTransparentWindow* self,RECT rc )
{
	Log.Add(_MESSAGE_,_T("CTransparentWindow::WindowProc WM_MOVING %d"), hwnd );
	if (self == NULL)
		return;

	Log.Add(_MESSAGE_,_T("CTransparentWindow::WindowProc WM_MOVING. self->m_bMonitorMove = %d"), self->m_bMonitorMove );
	if (self->m_bMonitorMove) 
	{
		int		width;
		int		height;

		width = rc.right-rc.left -2*(self->VERTICAL_BORDER_WIDTH);
		height = rc.bottom - rc.top -(2*self->HORIZONTAL_BORDER_HEIGHT - (self->HORIZONTAL_BORDER_FADE_TOP+self->HORIZONTAL_BORDER_FADE_BOTTOM));
		rc.left += self->VERTICAL_BORDER_WIDTH;
		rc.top += (self->HORIZONTAL_BORDER_HEIGHT - self->HORIZONTAL_BORDER_FADE_TOP);

		MoveWindow(self->m_hChildWindow, rc.left,rc.top, width,height, TRUE);
	}
}
LRESULT  CTransparentWindow::OnNCHitTest(HWND hwnd,CTransparentWindow* self,LPARAM lParam)
{
	RECT rc;

	Log.Add(_MESSAGE_, _T("CTransparentWindow::WindowProc WM_NCHITTEST") );

	if (self == NULL)
		return HTERROR;
	
	if (self->m_hChildWindow == NULL) 
		return HTNOWHERE;

	//Sprint 3 - not moveble/not resizeble
	return HTNOWHERE;

	GetWindowRect(hwnd,&rc);
	//Calculate landing rect
	int xPos = GET_X_LPARAM(lParam); 
	int yPos = GET_Y_LPARAM(lParam); 

	if ( (yPos - rc.top) <= self->HORIZONTAL_BORDER_HEIGHT ) 
		return HTCAPTION;

	if ( (rc.bottom - yPos ) <= self->HORIZONTAL_BORDER_HEIGHT ) 
		if ( (rc.right - xPos) <= self->VERTICAL_BORDER_WIDTH)
			return HTBOTTOMRIGHT;
		else
			return HTBOTTOM;

	if ( (xPos-rc.left) <= self->VERTICAL_BORDER_WIDTH)
		return HTLEFT;

	if ( (rc.right - xPos) <= self->VERTICAL_BORDER_WIDTH)
		return HTRIGHT;
}

void  CTransparentWindow::OnFadeTimer(CTransparentWindow* self)
{
	unsigned int uiLastEventDelta = (GetTickCount() - self->m_uiLastEvent);

	switch (self->m_eFadeState)
	{
		case FADE_OFF:

			if (self->MouseHitTest())
			{
				self->FadeTimer(self->FADE_IDLE_TIMEOUT);
				break;
			}
			if (uiLastEventDelta >= self->FADE_IDLE_TIMEOUT) // Starting the Fade Faze
			{
				self->FadeTimerOn();
				self->m_bFadeAscending = true;
				self->FadeWindow(self->m_bFadeAscending);
				self->m_eFadeState = FADE_IN_PROGRESS;
			} else 
				self->FadeTimer(self->FADE_IDLE_TIMEOUT - uiLastEventDelta);
			break;
		case FADE_IN_PROGRESS:
			if (!self->FadeWindow(self->m_bFadeAscending)) // Move has returned to window
			{
				self->FadeTimerOff();
				if (!self->m_bFadeAscending)
					self->m_eFadeState = FADE_OFF;
				else
				{
					if (self->m_iFadingLevel > 0)
						self->m_eFadeState = FADE_COMPLETED;
					else
						PostMessage( self->m_hChildWindow, WM_FADE_COMPLETED, 0, (LPARAM)0 );
				}
			}
			break;
		case FADE_COMPLETED:
				self->m_eFadeState = FADE_IN_PROGRESS;
				self->FadeTimerOn();
			// ::DestroyWindow(self->m_hChildWindow); 
			// self->CloseWindow();
			// to keep CleanUp in one place, instaed of close itself and child window
			// parent window will send WM_FADE_COMPLETED to the child window
			break;
		default:
			throw "Unknown FADE mode";
		}

}
void  CTransparentWindow::OnSizing(HWND hwnd,CTransparentWindow* self,RECT rc )
{
	Log.Add(_MESSAGE_,_T("CTransparentWindow::WindowProc WM_MOVE %d"), hwnd );
	if (self == NULL)
		return;

	if (self->m_bMonitorMove) 
	{
		int		width;
		int		height;

		width = rc.right-rc.left -2*(self->VERTICAL_BORDER_WIDTH);
		height = rc.bottom - rc.top -(2*self->HORIZONTAL_BORDER_HEIGHT - (self->HORIZONTAL_BORDER_FADE_TOP+self->HORIZONTAL_BORDER_FADE_BOTTOM));
		rc.left += self->VERTICAL_BORDER_WIDTH;
		rc.top += (self->HORIZONTAL_BORDER_HEIGHT - self->HORIZONTAL_BORDER_FADE_TOP);
		Log.Add(_MESSAGE_,_T("Clients Width = %d , Height = %d "), width, height);
		MoveWindow(self->m_hChildWindow, rc.left,rc.top, width,height, TRUE);
	}
}
void  CTransparentWindow::OnUserMove(HWND hwnd,LPRECT rc )
{
	Log.Add(_MESSAGE_, _T("CTransparentWindow::WindowProc WM_USER_BORDER_MOVE") );

	MoveWindow(hwnd,
		rc->left-CTransparentWindow::VERTICAL_BORDER_WIDTH,
		rc ->top-(CTransparentWindow::HORIZONTAL_BORDER_HEIGHT - CTransparentWindow::HORIZONTAL_BORDER_FADE_TOP),
				rc->right - rc->left+ 2*CTransparentWindow::VERTICAL_BORDER_WIDTH,
				rc->bottom -rc->top + ( 2*CTransparentWindow::HORIZONTAL_BORDER_HEIGHT - (CTransparentWindow::HORIZONTAL_BORDER_FADE_TOP+CTransparentWindow::HORIZONTAL_BORDER_FADE_BOTTOM)),
				TRUE);
}

CTransparentWindow* CTransparentWindow::GetWindow(HWND hwnd)
{
	CTransparentWindow*				self = NULL;
	TransparentWindowsMapIterator   it = self_map.find( hwnd );

	if( it != self_map.end() )
		self = (*it).second;

	return self;
}

