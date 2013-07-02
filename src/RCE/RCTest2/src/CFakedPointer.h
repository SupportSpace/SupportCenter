//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFakedPointer.h
///
///  Faked pointer class
///
///  @author "Archer Software" Sogin M. @date 20.09.2006
///
///  @modified Alexander Novak @date 06.11.2007 from CVisualPointer.h
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <tchar.h>
#include <windows.h>
#include <AidLib/CThread/CThread.h>
#include <AidLib/Loki/Singleton.h>
#include <AidLib/CCritSection/CCritSectionObject.h>
#include <map>
//========================================================================================================

/// Minimal time of a click
#define MINIMAL_CLICK			200
#define CLICK_TIMER_ID			1
#define WAIT_VP_INNIT_TIMEOUT	30000	/* half of a minute maximum */
#define VP_CLASS_NAME			_T("FakedPointer")
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFakedPointer
	:	CThread
{
	friend struct Loki::CreateUsingNew<CFakedPointer>;

public:
	/// Pointer state
	typedef enum _EState
	{
		NORMAL,			/// Normal - no buttons pressed
		RBTN_DOWN ,		/// Right button pressed
		LBTN_DOWN		/// Left button pressed
	} EState;

	/// dtor
	virtual ~CFakedPointer();

	/// Shows visual pointer
	void Show();

	/// Hides visual pointer
	void Hide();

	/// Returns visual pointer window handle
	/// @return visual pointer window handle
	HWND GetWindowHandle();

	/// Moves pointer to specified coordinates
	/// @param x x-coordinate
	/// @param y y-coordinate
	void MoveTo(const int x, const int y);
	void MoveToRel(const int x, const int y);
	POINT GetPos();

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

private:
	/// Current state
	EState m_state;
	bool m_clickProlonged;
	bool m_clicked;
	int m_absoluteX;
	int m_absoluteY;

	static unsigned int m_setStateMsg;
	static unsigned int m_moveToMsg;
	static unsigned int m_showWindowMsg;
	static unsigned int m_hideWindowMsg;

	/// Cursor data type
	typedef struct _SCursorData
	{
		/// Size of cursor bitmap
		SIZE size;
		/// Cursor bitmap
		HBITMAP hBitmap;
		/// Region
		HRGN hRegion;

		_SCursorData();
		_SCursorData(HBITMAP hBmp);

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
	CFakedPointer();

	/// Visual pointer window thread
	/// @param Params not used
	void Execute(void *Params);

	/// Main window proc
	static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void SetStateInternal(EState state);
	void MoveToInternal(const int x, const int y);
	void OnPaint(HDC hdc);
	void OnTimer(int id);

protected:
	/// critical section for internal purposes
	CCritSectionSimpleObject m_cs;

	/// Visual pointer's window handle
	HWND m_hWnd;

	/// Window created event
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_hWndCreatedEvent;
};
//--------------------------------------------------------------------------------------------------------

inline HWND CFakedPointer::GetWindowHandle()
{
	return m_hWnd;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FAKED_POINTER_INSTANCE Loki::SingletonHolder<CFakedPointer, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
//////////////////////////////////////////////////////////////////////////////////////////////////////////
