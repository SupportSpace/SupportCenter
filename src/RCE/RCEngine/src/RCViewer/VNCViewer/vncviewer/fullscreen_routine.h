// fullscreenView.h : interface of the CFullscreenView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CHiderTaskBar
{
private:
	/// window style for Start Button in Windows Vista
	///static const DWORD m_dwPushButtonStyle = 0x94000C00;
	/// window extended style for Start Button in Windows Vista
	///static const DWORD m_dwPushButtonExtendedStyle = 0x00080088;

	enum
	{
		PushButtonStyle = 0x94000C00,
		PushButtonExtendedStyle = 0x00080088
	};


	static BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam)
	{
		BOOL fAction = static_cast<BOOL>( lParam );
		if( (GetWindowLong( hwnd , GWL_STYLE ) == CHiderTaskBar::PushButtonStyle ) &&
			(GetWindowLong( hwnd , GWL_EXSTYLE ) == CHiderTaskBar::PushButtonExtendedStyle ) 
			)
		{
			ShowWindow(hwnd, fAction?SW_HIDE:SW_SHOWNORMAL);
			return FALSE;
		}
		return TRUE;
	}

public:
	void ToggleFullScreen(bool state)
	{
		EnumWindows( EnumWindowsProc , state );
		ShowWindow(FindWindow(_T("Shell_TrayWnd"), NULL), state?SW_HIDE:SW_SHOWNORMAL);
	}

	~CHiderTaskBar()
	{
		ToggleFullScreen( false );
	}
};


