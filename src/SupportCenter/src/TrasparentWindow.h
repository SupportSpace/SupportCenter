#pragma once
#include "AidLib\Strings\tstring.h"
#include <map>


#define WM_USER_BORDER_MOVE		WM_USER + 1
#define WM_USER_MOVE_ACTIVITIES	WM_USER + 2
#define WM_FADE_COMPLETED		20050

typedef enum EFadeStatus {FADE_OFF=0,FADE_IN_PROGRESS,FADE_COMPLETED};

class CTransparentWindow;

typedef std::map<HWND, CTransparentWindow*> TransparentWindowsMap;
typedef std::map<HWND, CTransparentWindow*>::iterator  TransparentWindowsMapIterator;

class CTransparentWindow
{
public:
	CTransparentWindow(HINSTANCE instance,int x,int y, int height,int width);

	~CTransparentWindow(void);

	HWND GetWindowHandle(void);

	void   SetChildWindow(HWND hWindow);

	static void AddToMap( HWND hWnd, CTransparentWindow* pcTransWin );
	static void DeleteFromMap( HWND hWnd );
	static LRESULT CALLBACK WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

	static const int HORIZONTAL_BORDER_WIDTH;
	static const int HORIZONTAL_BORDER_HEIGHT;
	static const int HORIZONTAL_BORDER_FADE_TOP;
	static const int HORIZONTAL_BORDER_FADE_BOTTOM;
	static const int VERTICAL_BORDER_WIDTH;
	const int FADE_TIMEOUT;
	const int FADE_IDLE_TIMEOUT;

	static void  OnUserMove(HWND hwnd,LPRECT rc );

private:

	const int m_iTransparency;
	const int m_iOpacity;

	// Reponse on Window Move activities
	bool	m_bMonitorMove;
	HBITMAP m_hBottomBmp;
	HBITMAP m_hTopBmp;
	HBITMAP m_hMiddleBmp;
	UINT    m_nIDFadeEvent; 

	//Atom Class Name
	tstring m_className;
	//Handle of Border Window
	HWND	m_hWnd;
	//Handle of Child Window
	HWND    m_hChildWindow;
	//Instance of Application
	HINSTANCE m_hInstance;

	/////////////////Fading section///////////////////
	
	EFadeStatus m_eFadeState;
	//Last Event Mark
	unsigned int m_uiLastEvent;
	bool m_bFadeAscending;
	int  m_iFadingLevel;
	/////////////////////////////////////////////////
	bool MouseHitTest();
	void FadeTimer(unsigned int uiTimer);
	void FadeTimerOn();
	void FadeTimerOff();
	bool FadeWindow(bool Ascending);
	void CloseWindow();

	



	//static CTransparentWindow* self;
	static TransparentWindowsMap self_map;
	static void OnPaint(HWND hwnd,CTransparentWindow* self);
	static void OnMoving(HWND hwnd,CTransparentWindow* self,RECT rc );
	static void OnSizing(HWND hwnd,CTransparentWindow* self,RECT rc );
	static LRESULT OnNCHitTest(HWND hwnd,CTransparentWindow* self,LPARAM lParam);
	static void  OnFadeTimer(CTransparentWindow* self);
	static CTransparentWindow* GetWindow(HWND hwnd);
	
};
