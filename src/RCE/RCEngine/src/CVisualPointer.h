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
#pragma once
#include <windows.h>
#include <AidLib/CThread/CThread.h>
#include <AidLib/Loki/Singleton.h>
#include <map>

/// Minimal time of a click
#define MINIMAL_CLICK 200
#define CLICK_TIMER_ID 1
#define WAIT_VP_INNIT_TIMEOUT 30000 /*half of a minute maximum*/
#define VP_CLASS_NAME "VisualPointer"

/// visual pointer representation
class CVisualPointer : CThread
{
friend struct Loki::CreateUsingNew<CVisualPointer>;

protected:
	/// critical section for internal purposes
	CRITICAL_SECTION m_cs;

	/// Visual pointer window handle
	HWND m_hWnd;

	/// Window created event
	HANDLE m_hWndCreatedEvent;
public:
	/// Pointer state
	typedef enum _EState
	{
		NORMAL,			/// Normal - no buttons pressed
		RBTN_DOWN ,		/// Right button pressed
		LBTN_DOWN		/// Left button pressed
	} EState;
private:

	/// Current state
	EState m_state;
	bool m_clickProlonged,m_clicked;

	static unsigned int m_setStateMsg;
	static unsigned int m_MoveToMsg;
	static unsigned int m_showWindowMsg;
	static unsigned int m_hideWindowMsg;

	/// Cursor data type
	typedef struct _SCursorData
	{
		///ctor
		_SCursorData(HBITMAP hBmp)
		{
			if (!hBmp) Log.WinError(_ERROR_,"_SCursorData hBmp == NULL");
			hBitmap = hBmp;
			BitMapContourToWinRgn(hBitmap,hRegion,RGB(0,0,0));
			BITMAP bmp;
			GetObject(hBitmap, sizeof(bmp), &bmp);
			size.cx = bmp.bmWidth;
			size.cy = bmp.bmHeight;
		}
		_SCursorData(){};
		/// Size of cursor bitmap
		SIZE size;
		/// Cursor bitmap
		HBITMAP hBitmap;
		/// Region
		HRGN hRegion;
		///Taked from http://www.codeproject.com/dialog/BitmapHandling.asp
		BOOL BitMapContourToWinRgn( HBITMAP hBmp, HRGN& hRgn, COLORREF cTolerance );
	} SCursorData;

	/// Cursors map
	std::map<EState,SCursorData> m_cursors;

	/// Current cursor
	SCursorData m_currentCursor;
	
	/// Id of desktop thread
	unsigned int m_desktopThreadId;

	/// Custom message to tell dektop thread we're visible
	unsigned int m_noticeVisibleMsg;

	/// Set to true if screen update needed
	bool m_updateNeeded;

	/// initialises object instance
	CVisualPointer(void);

	/// Visual pointer window thread
	/// @param Params not used
	void Execute(void *Params);

	/// main window proc
	static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void SetStateInternal(EState state);
	void MoveToInternal(const int x, const int y);
	void OnPaint(HDC hdc);
	void OnTimer(int id);
public:
	/// dtor
	virtual ~CVisualPointer(void);

	/// Shows visual pointer
	void Show();
	/// Hides visual pointers
	void Hide();

	/// returns visual pointer window handle
	/// @return visual pointer window handle
	inline HWND GetWindowHandle()
	{
		return m_hWnd;
	}

	/// Moves pointer to specified coordinates
	/// @param x x coordinate
	/// @param y y coordinate
	void MoveTo(const int x, const int y);

	/// Set new visual pointer state
	/// @param state new state
	void SetState(const EState state);

	/// Return true if screen update needed
	/// @return true if screen update needed
	bool UpdateNeeded();

	/// Setter for desktop thread Id
	void SetDesktopThreadId(unsigned int desktopThreadId);

	/// Setter for visible notifications custom message Id
	void SetNoticeVisibleMsg(unsigned int noticeVisibleMsg);
};

#define VISUAL_POINTER_INSTANCE Loki::SingletonHolder<CVisualPointer, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
