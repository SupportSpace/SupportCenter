#pragma once

#include <windows.h>
#include <AidLib/AidLib.h>
#include <TCHAR.h>
#include <set>

/// The GetCurrentThread function retrieves a handle for the module where it's compiled. 
/// The function retrieves pointer to the base address of the region of pages where it's compiled. It has tested badly.
inline HINSTANCE GetCurrentModule(void)
{	
	MEMORY_BASIC_INFORMATION mem;
	if(VirtualQuery(GetCurrentModule, &mem, sizeof(mem)))
	{
		//_ASSERTE(mem.Type == MEM_IMAGE);
		//_ASSERTE(mem.AllocationBase != NULL);
		return (HINSTANCE)mem.AllocationBase;
	}
	return NULL;
}

/// Releases PROCESS_INFORMATION structure, this could be usefull for smartpointers
/// @param @pi structure to release
inline void ReleaseProcessInformation(PROCESS_INFORMATION *pi)
{
	CloseHandle(pi->hProcess);
	CloseHandle(pi->hThread);
}

inline int GetOSVersion()
{
	OSVERSIONINFOEX osInfo;
	osInfo.dwOSVersionInfoSize=sizeof(osInfo);
	int osVersion = 0;
	if(0 == GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&osInfo)))
	{
		osVersion = 5; //WinXP by default
		//Log.WinError(_ERROR_,_T("Failed to GetVersionEx"));
	} else
	{
		osVersion = osInfo.dwMajorVersion;
	}
	return osVersion;
}

/// Waits for single object with processing messages
inline DWORD WaitForSingleObjectWithMessageLoop(HANDLE hEvent, DWORD dwMilliseconds)
{
	DWORD ret;
	MSG msg;
	DWORD timeout = dwMilliseconds;
	DWORD startTime = GetTickCount();
	while(true)
	{
		/// Wait for event or message
		ret = MsgWaitForMultipleObjects(1, &hEvent, FALSE, timeout, QS_ALLINPUT);
		switch(ret)
		{
		/// Known results
		case WAIT_FAILED:
		case WAIT_OBJECT_0:
		case WAIT_TIMEOUT:
		case WAIT_ABANDONED_0:
			return ret;
		default:
			/// Process messages
			while(PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				/// Check up is event signaled
				ret = WaitForSingleObject(hEvent, 0);
				if(WAIT_OBJECT_0 == ret)
					return ret;
				else
				{
					/// Recalculate timeout
					if(INFINITE != dwMilliseconds)
					{
						DWORD currentTime = GetTickCount();
						if(currentTime - startTime >= timeout)
							return WAIT_TIMEOUT;
						else
						{
							timeout -= currentTime - startTime;
							startTime = currentTime;
						}
					}
				}
			}
		}
	}
}

/// Terminates child processes for specified process id
AIDLIB_API DWORD TerminateChildProcesses(DWORD parentProcessId, std::set<DWORD>& exceptions);

/// Terminates process by id
AIDLIB_API void TerminateProcess(DWORD processId, std::set<DWORD>& exceptions);

/// Class for hiding/showing taskbar
class CToggleTaskBar
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
		if( (GetWindowLong( hwnd , GWL_STYLE ) == CToggleTaskBar::PushButtonStyle ) &&
			(GetWindowLong( hwnd , GWL_EXSTYLE ) == CToggleTaskBar::PushButtonExtendedStyle ) 
			)
		{
			ShowWindow(hwnd, fAction?SW_HIDE:SW_SHOWNORMAL);
			return FALSE;
		}
		return TRUE;
	}

public:
	/// Shows/hides taskbar
	void ToggleTaskBar(bool show)
	{
		EnumWindows( EnumWindowsProc , !show );
		ShowWindow( FindWindow(_T("Shell_TrayWnd"), NULL), show?SW_SHOWNORMAL:SW_HIDE );
	}

	virtual ~CToggleTaskBar()
	{
		ToggleTaskBar( true );
	}
};
