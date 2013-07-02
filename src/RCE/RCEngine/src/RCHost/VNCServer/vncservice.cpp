//  Copyright (C) 2002 RealVNC Ltd. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


// vncService

// Implementation of service-oriented functionality of WinVNC

#include "stdhdrs_srv.h"

// Header
#include <boostThreads/boostThreads.h>
#include "vncService.h"
#include <lmcons.h>
#include "WinVNC.h"
#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>

#pragma warning( disable : 4996 ) // <str func> was declared deprecated

// Error message logging
void LogErrorMsg(char *message);
BOOL serviceshutdown;

// OS-SPECIFIC ROUTINES

// Create an instance of the vncService class to cause the static fields to be
// initialised properly

vncService init;

DWORD	g_platform_id;
BOOL	g_impersonating_user = 0;
DWORD	g_version_major;
DWORD	g_version_minor;




vncService::vncService()
{
    OSVERSIONINFO osversioninfo;
    osversioninfo.dwOSVersionInfoSize = sizeof(osversioninfo);

    // Receive the current OS version
    if (!GetVersionEx(&osversioninfo))
	    g_platform_id = 0;
    g_platform_id = osversioninfo.dwPlatformId;
	g_version_major = osversioninfo.dwMajorVersion;
	g_version_minor = osversioninfo.dwMinorVersion;
	serviceshutdown=false;
}

// CurrentUser - fills a buffer with the name of the current user!
BOOL
GetCurrentUser(char *buffer, UINT size) // RealVNC 336 change
{	// How to obtain the name of the current user depends upon the OS being used
	if ((g_platform_id == VER_PLATFORM_WIN32_NT) && vncService::RunningAsService())
	{
		// Windows NT, service-mode

		// -=- FIRSTLY - verify that a user is logged on

		// Receive the current Window station
		HWINSTA station = GetProcessWindowStation();
		if (station == NULL)
			return FALSE;

		// Receive the current user SID size
		DWORD usersize;
		GetUserObjectInformation(station,
			UOI_USER_SID, NULL, 0, &usersize);
		DWORD  dwErrorCode = GetLastError();
		SetLastError(0);

		// Check the required buffer size isn't zero
		if (usersize == 0)
		{
			// No user is logged in - ensure we're not impersonating anyone
			RevertToSelf();
			g_impersonating_user = FALSE;

			// Return "" as the name...
			if (strlen("") >= size)
				return FALSE;
			strcpy(buffer, "");

			return TRUE;
		}

		// -=- SECONDLY - a user is logged on but if we're not impersonating
		//     them then we can't continue!
		if (!g_impersonating_user) {
			// Return "" as the name...
			if (strlen("") >= size)
				return FALSE;
			strcpy(buffer, "");
			return TRUE;
		}
	}
		
	// -=- When we reach here, we're either running under Win9x, or we're running
	//     under NT as an application or as a service impersonating a user
	// Either way, we should find a suitable user name.

	switch (g_platform_id)
	{

	case VER_PLATFORM_WIN32_WINDOWS:
	case VER_PLATFORM_WIN32_NT:
		{
			// Just call GetCurrentUser
			DWORD length = size;

			if (GetUserName(buffer, &length) == 0)
			{
				UINT error = GetLastError();

				if (error == ERROR_NOT_LOGGED_ON)
				{
					// No user logged on
					if (strlen("") >= size)
						return FALSE;
					strcpy(buffer, "");
					return TRUE;
				}
				else
				{
					// Genuine error...
					//vnclog.Print(LL_INTERR, VNCLOG("getusername error %d\n"), GetLastError());
					Log.WinError(_ERROR_, "getusername error %d", GetLastError());
					return FALSE;
				}
			}
		}
		return TRUE;
	};

	// OS was not recognised!
	return FALSE;
}

// RealVNC 336 change
BOOL
vncService::CurrentUser(char *buffer, UINT size)
{
  BOOL result = GetCurrentUser(buffer, size);
  if (result && (strcmp(buffer, "") == 0) && !vncService::RunningAsService()) {
    strncpy(buffer, "Default", size);
  }
  return result;
}


BOOL vncService::IsWSLocked()
{
	if (!IsWinNT()) 
		return false;

	bool bLocked = false;


	HDESK hDesk;
	BOOL bRes;
	DWORD dwLen;
	char sName[200];
	
	hDesk = OpenInputDesktop(0, FALSE, 0);

	if (hDesk == NULL)
	{
		 bLocked = true;
	}
	else 
	{
		bRes = GetUserObjectInformation(hDesk, UOI_NAME, sName, sizeof(sName), &dwLen);

		if (bRes)
			sName[dwLen]='\0';
		else
			sName[0]='\0';

		CloseDesktop(hDesk);

		if (stricmp(sName,"Default") != 0)
			 bLocked = true; // WS is locked or screen saver active
		else
			 bLocked = false ;
	}

	return bLocked;
}


// IsWin95 - returns a BOOL indicating whether the current OS is Win95
BOOL
vncService::IsWin95()
{
	return (g_platform_id == VER_PLATFORM_WIN32_WINDOWS);
}

// IsWinNT - returns a bool indicating whether the current OS is WinNT
BOOL
vncService::IsWinNT()
{
	return (g_platform_id == VER_PLATFORM_WIN32_NT);
}

// Version info
DWORD
vncService::VersionMajor()
{
	return g_version_major;
}

DWORD
vncService::VersionMinor()
{
	return g_version_minor;
}

// Static routines only used on Windows NT to ensure we're in the right desktop
// These routines are generally available to any thread at any time.

// - SelectDesktop(HDESK)
// Switches the current thread into a different desktop by deskto handle
// This call takes care of all the evil memory management involved

BOOL
vncService::SelectHDESK(HDESK new_desktop)
{
	// Are we running on NT?
	if (IsWinNT())
	{
		HDESK old_desktop = GetThreadDesktop(GetCurrentThreadId());
		//You do not need to call the CloseDesktop function to close the returned handle.

		DWORD dummy;
		char new_name[256];

		if (!GetUserObjectInformation(new_desktop, UOI_NAME, &new_name, 256, &dummy)) {
			return FALSE;
		}

		//vnclog.Print(LL_INTERR, VNCLOG("SelectHDESK to %s (%x) from %x\n"), new_name, new_desktop, old_desktop);
		
		// Switch the desktop
		if(!SetThreadDesktop(new_desktop)) 
		{
			return FALSE;
		}

		return TRUE;
	}

	return TRUE;
}

boost::shared_ptr<boost::remove_pointer<HDESK>::type> vncService::g_desktop;
// - SelectDesktop(char *)
// Switches the current thread into a different desktop, by name
// Calling with a valid desktop name will place the thread in that desktop.
// Calling with a NULL name will place the thread in the current input desktop.
BOOL vncService::SelectDesktop(char *name)
{
	// Are we running on NT?
	if (IsWinNT())
	{
		//vnclog.Print(LL_INTERR, VNCLOG("SelectDesktop \n"));
		if (name != NULL)
		{
			// Attempt to open the named desktop
			g_desktop.reset(OpenDesktop(	name, 0, FALSE,
														DESKTOP_CREATEMENU |
														DESKTOP_CREATEWINDOW |
														DESKTOP_ENUMERATE |
														DESKTOP_HOOKCONTROL |
														DESKTOP_JOURNALPLAYBACK |
														DESKTOP_JOURNALRECORD |
														DESKTOP_READOBJECTS |
														DESKTOP_SWITCHDESKTOP |
														DESKTOP_WRITEOBJECTS),
											/*DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
											DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
											DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
											DESKTOP_SWITCHDESKTOP | GENERIC_WRITE),*/
											CloseDesktop);
			if (g_desktop.get() != NULL)
				Log.Add(_MESSAGE_,_T("Desktop named %s opened"),name);
			else
				Log.WinError(_WARNING_,_T("Failed to open display named %s"),name);
		}
		else
		{
			// No, so open the input desktop
			g_desktop.reset(OpenInputDesktop(	0, FALSE,
												DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
												DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
												DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
												DESKTOP_SWITCHDESKTOP | GENERIC_WRITE),
												CloseDesktop);
			// Did we succeed?
			if (g_desktop.get() == NULL) 
			{
				g_desktop.reset(	OpenDesktop(TEXT("WINLOGON"), 0, false, DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW | DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS | STANDARD_RIGHTS_REQUIRED),
									CloseDesktop);
				if (g_desktop.get() == NULL)
				{
					///Log.WinError(_ERROR_,"Failed to open current or WINLOGON desktop ");
					///Sleep(OPEN_DISPLAY_RETRY_TIMEOUT); //Since it will affect SelectDesktop pooling sleeping delay to reduce CPU usage
					return FALSE;
				}
			}
		}

		// Switch to the new desktop
		if (!SelectHDESK(g_desktop.get()))
		{
			Log.WinError(_ERROR_,"Failed to SelectHDESK");
			// Failed to enter the new desktop, so free it!
			g_desktop.reset();
			return FALSE;
		}
		// We successfully switched desktops!
		return TRUE;
	}
	return (name == NULL);
}

// Find the visible window station and switch to it
// This would allow the service to be started non-interactive
// Needs more supporting code & a redesign of the server core to
// work, with better partitioning between server & UI components.

static HWINSTA home_window_station = GetProcessWindowStation();

BOOL CALLBACK WinStationEnumProc(LPTSTR name, LPARAM param) {
	HWINSTA station = OpenWindowStation(name, FALSE, GENERIC_ALL);
	HWINSTA oldstation = GetProcessWindowStation();
	USEROBJECTFLAGS flags;
	if (!GetUserObjectInformation(station, UOI_FLAGS, &flags, sizeof(flags), NULL)) {
		return TRUE;
	}
	BOOL visible = flags.dwFlags & WSF_VISIBLE;
	if (visible) {
		if (SetProcessWindowStation(station)) {
			if (oldstation != home_window_station) {
				CloseWindowStation(oldstation);
			}
		} else {
			CloseWindowStation(station);
		}
		return FALSE;
	}
	return TRUE;
}

BOOL
vncService::SelectInputWinStation()
{
	home_window_station = GetProcessWindowStation();
	return EnumWindowStations(&WinStationEnumProc, NULL);
}

void
vncService::SelectHomeWinStation()
{
	HWINSTA station=GetProcessWindowStation();
	SetProcessWindowStation(home_window_station);
	CloseWindowStation(station);
}

// NT only function to establish whether we're on the current input desktop

BOOL
vncService::InputDesktopSelected()
{
	// Are we running on NT?
//	if (serviceshutdown==true) return TRUE;
	if (IsWinNT())
	{
		// Receive the input and thread desktops
		HDESK threaddesktop = GetThreadDesktop(GetCurrentThreadId());
		HDESK inputdesktop = OpenInputDesktop(0, FALSE,
				DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
				DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
				DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
				DESKTOP_SWITCHDESKTOP | GENERIC_WRITE);


		// Receive the desktop names:
		// *** I think this is horribly inefficient but I'm not sure.
		if (inputdesktop == NULL)
			{
				DWORD lasterror;
				lasterror=GetLastError();
				//vnclog.Print(LL_INTERR, VNCLOG("OpenInputDesktop I\n"));
				if (lasterror==170) 
				{
					return TRUE;
				}
				//vnclog.Print(LL_INTERR, VNCLOG("OpenInputDesktop II\n"));
				return FALSE;
			}

		DWORD dummy;
		char threadname[256];
		char inputname[256];

		if (!GetUserObjectInformation(threaddesktop, UOI_NAME, &threadname, 256, &dummy)) 
		{
			if (!CloseDesktop(inputdesktop))
			{
				//vnclog.Print(LL_INTWARN, VNCLOG("failed to close input desktop\n"));
				Log.WinError(_ERROR_, "failed to close input desktop");
			}

			return FALSE;
		}
		_ASSERT(dummy <= 256);
		if (!GetUserObjectInformation(inputdesktop, UOI_NAME, &inputname, 256, &dummy)) {
			if (!CloseDesktop(inputdesktop))
			{
				//vnclog.Print(LL_INTWARN, VNCLOG("failed to close input desktop\n"));
				Log.WinError(_ERROR_, "failed to close input desktop");
			}
			return FALSE;
		}
		_ASSERT(dummy <= 256);

		if (!CloseDesktop(inputdesktop))
			Log.WinError(_ERROR_, "failed to close input desktop");
			//vnclog.Print(LL_INTWARN, VNCLOG("failed to close input desktop\n"));

		if (strcmp(threadname, inputname) != 0)
			return FALSE;
	}
	return TRUE;
}

// Static routine used to fool Winlogon into thinking CtrlAltDel was pressed

void SimulateCtrlAltDelThreadFn()
{
	HDESK old_desktop = GetThreadDesktop(GetCurrentThreadId());

	// Switch into the Winlogon desktop
	if (!vncService::SelectDesktop("Winlogon"))
	{
		//vnclog.Print(LL_INTERR, VNCLOG("failed to select logon desktop\n"));
		Log.WinError(_ERROR_, "failed to select logon desktop");
		if (!vncService::SelectDesktop("Default"))
		{
			Log.WinError(_ERROR_, "failed to select default desktop");
		}
	}

	//vnclog.Print(LL_ALL, VNCLOG("generating ctrl-alt-del\n"));

	// Fake a hotkey event to any windows we find there.... :(
	// Winlogon uses hotkeys to trap Ctrl-Alt-Del...
	PostMessage(HWND_BROADCAST, WM_HOTKEY, 0, MAKELONG(MOD_ALT | MOD_CONTROL, VK_DELETE));

	// Switch back to our original desktop
	if (old_desktop != NULL)
		vncService::SelectHDESK(old_desktop);
}

// Static routine to simulate Ctrl-Alt-Del locally

BOOL
vncService::SimulateCtrlAltDel()
{
	//vnclog.Print(LL_ALL, VNCLOG("preparing to generate ctrl-alt-del\n"));

	// Are we running on NT?
	if (IsWinNT())
	{
		//vnclog.Print(LL_ALL, VNCLOG("spawn ctrl-alt-del thread...\n"));

		// *** This is an unpleasant hack.  Oh dear.

		// I simulate CtrAltDel by posting a WM_HOTKEY message to all
		// the windows on the Winlogon desktop.
		// This requires that the current thread is part of the Winlogon desktop.
		// But the current thread has hooks set & a window open, so it can't
		// switch desktops, so I instead spawn a new thread & let that do the work...

		boost::thread thread(boost::bind(&SimulateCtrlAltDelThreadFn));
		thread.join();
		return TRUE;
	}
	return TRUE;
}


// SERVICE-MODE ROUTINES

// Service-mode defines:

// Executable name
#define VNCAPPNAME            "winvnc"

// Internal service name
#define VNCSERVICENAME        "winvnc"

// Displayed service name
#define VNCSERVICEDISPLAYNAME "VNC Server"

// List of other required services ("dependency 1\0dependency 2\0\0")
// *** These need filling in properly
#define VNCDEPENDENCIES       ""

// Internal service state
SERVICE_STATUS          g_srvstatus;       // current status of the service
SERVICE_STATUS_HANDLE   g_hstatus;
DWORD                   g_error = 0;
DWORD					g_servicethread = NULL;
char*                   g_errortext[256];

// Forward defines of internal service functions
void WINAPI ServiceMain(DWORD argc, char **argv);

void ServiceWorkThread(void *arg);
void ServiceStop();
void WINAPI ServiceCtrl(DWORD ctrlcode);

bool WINAPI CtrlHandler (DWORD ctrltype);

BOOL ReportStatus(DWORD state, DWORD exitcode, DWORD waithint);

// ROUTINE TO QUERY WHETHER THIS PROCESS IS RUNNING AS A SERVICE OR NOT

BOOL	g_servicemode = FALSE;

BOOL
vncService::RunningAsService()
{
	return g_servicemode;
}


////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////


// SERVICE STOP ROUTINE - post a quit message to the relevant thread
void ServiceStop()
{
	// Post a quit message to the main service thread
	if (g_servicethread != NULL)
	{
		//vnclog.Print(LL_INTINFO, VNCLOG("quitting from ServiceStop\n"));
		PostThreadMessage(g_servicethread, WM_QUIT, 0, 0);
	}
}

// Service manager status reporting
BOOL ReportStatus(DWORD state,
				  DWORD exitcode,
				  DWORD waithint)
{
	static DWORD checkpoint = 1;
	BOOL result = TRUE;

	// If we're in the start state then we don't want the control manager
	// sending us control messages because they'll confuse us.
    if (state == SERVICE_START_PENDING)
		g_srvstatus.dwControlsAccepted = 0;
	else
		g_srvstatus.dwControlsAccepted = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;

	// Save the new status we've been given
	g_srvstatus.dwCurrentState = state;
	g_srvstatus.dwWin32ExitCode = exitcode;
	g_srvstatus.dwWaitHint = waithint;

	// Update the checkpoint variable to let the SCM know that we
	// haven't died if requests take a long time
	if ((state == SERVICE_RUNNING) || (state == SERVICE_STOPPED))
		g_srvstatus.dwCheckPoint = 0;
	else
        g_srvstatus.dwCheckPoint = checkpoint++;

	// Tell the SCM our new status
	if (!(result = SetServiceStatus(g_hstatus, &g_srvstatus)))
		LogErrorMsg("SetServiceStatus failed");

    return result;
}

// Error reporting
void LogErrorMsg(char *message)
{
    char	msgbuff[256];
    HANDLE	heventsrc;
    char *	strings[2];

	// Save the error code
	g_error = GetLastError();

	// Use event logging to log the error
    heventsrc = RegisterEventSource(NULL, VNCSERVICENAME);

	_snprintf(msgbuff, 256, "%s error: %d", VNCSERVICENAME, g_error);
    strings[0] = msgbuff;
    strings[1] = message;

	if (heventsrc != NULL)
	{
		MessageBeep(MB_OK);

		ReportEvent(
			heventsrc,				// handle of event source
			EVENTLOG_ERROR_TYPE,	// event type
			0,						// event category
			0,						// event ID
			NULL,					// current user's SID
			2,						// strings in 'strings'
			0,						// no bytes of raw data
			(const char **)strings,	// array of error strings
			NULL);					// no raw data

		DeregisterEventSource(heventsrc);
	}
}

