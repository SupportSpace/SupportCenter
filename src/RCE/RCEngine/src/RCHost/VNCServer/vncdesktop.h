//  Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
//  Copyright (C) 2000-2002 Const Kaplinsky. All Rights Reserved.
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


// vncDesktop object

// The vncDesktop object handles retrieval of data from the
// display buffer.  It also uses the RFBLib DLL to supply
// information on mouse movements and screen updates to
// the server
#pragma once

class vncDesktopThread;

#include "CDisplayLockFix.h"

// Include files
#include "stdhdrs_srv.h"

class vncServer;
#include "rfbRegion.h"
#include "rfbUpdateTracker.h"
#include "vncBuffer.h"
#include "translate.h"
#include <boostThreads/boostThreads.h>

/// @modified Alexander Novak @date 05.11.2007 for supporting layered windows clipping
#include <AidLib/CCritSection/CCritSectionObject.h>

// Modif rdv@2002 - v1.1.x - videodriver
#include "videodriver.h"

// Modif sf@2002 - v1.1.0
#include <list>
//#include "textchat.h"
//#define COMPILE_MULTIMON_STUBS
//#include "Multimon.h"
#ifndef SM_CMONITORS

#define SM_XVIRTUALSCREEN       76
#define SM_YVIRTUALSCREEN       77
#define SM_CXVIRTUALSCREEN      78
#define SM_CYVIRTUALSCREEN      79
#define SM_CMONITORS            80
#define SM_SAMEDISPLAYFORMAT    81

// HMONITOR is already declared if WINVER >= 0x0500 in windef.h
// This is for components built with an older version number.
//
#if !defined(HMONITOR_DECLARED) && (WINVER < 0x0500)
DECLARE_HANDLE(HMONITOR);
#define HMONITOR_DECLARED
#endif

#define MONITOR_DEFAULTTONULL       0x00000000
#define MONITOR_DEFAULTTOPRIMARY    0x00000001
#define MONITOR_DEFAULTTONEAREST    0x00000002

#define MONITORINFOF_PRIMARY        0x00000001

typedef struct tagMONITORINFO
{
    DWORD   cbSize;
    RECT    rcMonitor;
    RECT    rcWork;
    DWORD   dwFlags;
} MONITORINFO, *LPMONITORINFO;

#ifndef CCHDEVICENAME
#define CCHDEVICENAME 32
#endif

#ifdef __cplusplus
typedef struct tagMONITORINFOEXA : public tagMONITORINFO
{
    CHAR        szDevice[CCHDEVICENAME];
} MONITORINFOEXA, *LPMONITORINFOEXA;
typedef struct tagMONITORINFOEXW : public tagMONITORINFO
{
    WCHAR       szDevice[CCHDEVICENAME];
} MONITORINFOEXW, *LPMONITORINFOEXW;
#ifdef UNICODE
typedef MONITORINFOEXW MONITORINFOEX;
typedef LPMONITORINFOEXW LPMONITORINFOEX;
#else
typedef MONITORINFOEXA MONITORINFOEX;
typedef LPMONITORINFOEXA LPMONITORINFOEX;
#endif // UNICODE
#else // ndef __cplusplus
typedef struct tagMONITORINFOEXA
{
    MONITORINFO;
    CHAR        szDevice[CCHDEVICENAME];
} MONITORINFOEXA, *LPMONITORINFOEXA;
typedef struct tagMONITORINFOEXW
{
    MONITORINFO;
    WCHAR       szDevice[CCHDEVICENAME];
} MONITORINFOEXW, *LPMONITORINFOEXW;
#ifdef UNICODE
typedef MONITORINFOEXW MONITORINFOEX;
typedef LPMONITORINFOEXW LPMONITORINFOEX;
#else
typedef MONITORINFOEXA MONITORINFOEX;
typedef LPMONITORINFOEXA LPMONITORINFOEX;
#endif // UNICODE
#endif
#endif

typedef std::list<COLORREF> RGBPixelList;   // List of RGB values (pixels)
// sf@2002 - Generates ClassName lenght warning in debug mode compile.
// typedef std::list<RGBPixelList*> GridsList; // List of Grids of pixels
typedef std::list<void*> GridsList; // List of Grids of pixels
typedef std::list<HWND> WindowsList;       // List of windows handles


// Constants
extern const UINT RFB_SCREEN_UPDATE;
extern const UINT RFB_COPYRECT_UPDATE;
extern const UINT RFB_MOUSE_UPDATE;
extern const UINT RFB_VP_VISIBILITY_CHANGED;
extern const char szDesktopSink[];

#define NONE 0
#define MIRROR 1
#define PSEUDO 2

typedef struct DrvWatch
{
	HWND hwnd;
	bool *stop;
}DrvWatch;

typedef BOOL (*SetHooksFn)(DWORD thread_id,UINT UpdateMsg,UINT CopyMsg,UINT MouseMsg,BOOL ddihook);
typedef BOOL (*UnSetHooksFn)(DWORD thread_id);
typedef BOOL (*SetKeyboardFilterHookFn)(BOOL activate);	
typedef BOOL (*SetMouseFilterHookFn)(BOOL activate);
typedef BOOL (WINAPI*  pBlockInput) (BOOL);
typedef BOOL (WINAPI* LPGETMONITORINFO)(HMONITOR, LPMONITORINFO);
typedef HMONITOR (WINAPI* LPMONITOTFROMPOINT) (POINT,DWORD);
// Class definition
// multi monitor
struct monitor
{
	int Width;
	int Height;
	int Depth;
	char device[32];
	int offsetx;
	int offsety;
};

class vncDesktop
{
friend class CDisplayLockFix;
// Fields
public:
	/// fixing desktop lock
	std::auto_ptr<CDisplayLockFix> m_lockFix;
protected:
	/// Recently sent clip text - to prevent infinite clipboard sending on several DS sessions
	tstring m_recentClipText;
// Methods
public:
	// Make the desktop thread & window proc friends
	friend class vncDesktopThread;
	friend LRESULT CALLBACK DesktopWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

	// Create/Destroy methods
	vncDesktop(vncServer *server);
	~vncDesktop();

	virtual bool CaptureAlphaBlending(){return m_fCaptureAlphaBlending;};
	virtual void CaptureAlphaBlending(bool fEnabled){m_fCaptureAlphaBlending = fEnabled;};

	BOOL Init(vncServer *pSrv);

	// Tell the desktop hooks to grab & update a particular rectangle
	void QueueRect(const rfb::Rect &rect);
	
	// Kick the desktop hooks to perform an update
	void TriggerUpdate();

	// Receive a reference to the desktop update lock
	// The lock is held while data is being grabbed and copied
	// to the back buffer, and while changes are being passed to
	// clients
	inline boost::recursive_mutex &GetUpdateLock() {return m_update_lock;};

	// Screen translation, capture, info
	void FillDisplayInfo(rfbServerInitMsg *scrInfo);
	void CaptureScreen(const rfb::Rect &UpdateArea, BYTE *scrBuff, UINT scrBuffSize);

	/// @modified Alexander Novak @date 05.11.2007 Added the method to support layered windows clipping
	void ClipHiddenWindows(const rfb::Rect &UpdateArea, BYTE *destBuff, UINT scrBuffSize);

	int ScreenBuffSize();
	HWND Window() {return m_hwnd;};

	// Mouse related
	void CaptureMouse(BYTE *scrBuff, UINT scrBuffSize);
	rfb::Rect MouseRect();
	void SetCursor(HCURSOR cursor);
	// CURSOR HANDLING
	BOOL GetRichCursorData(BYTE *databuf, HCURSOR hcursor, int width, int height);
	HCURSOR GetCursor() { return m_hcursor; }

	// Clipboard manipulation
	void SetClipText(LPSTR text);

	// Method to obtain the DIBsection buffer if fast blits are enabled
	// If they're disabled, it'll return NULL
	inline VOID *OptimisedBlitBuffer() {return m_DIBbits;};

	BOOL	m_initialClipBoardSeen;

	// Handler for pixel data grabbing and region change checking
	vncBuffer		m_buffer;
		//SINGLE WINDOW
	vncServer		*GetServerPointer() {return m_server;};
	HWND			m_Single_hWnd;
	HWND			m_Single_hWnd_backup;
	BOOL			CalculateSWrect(RECT &rect);
	rfb::Rect		GetSize();
	rfb::Rect		GetQuarterSize();

	// Modif rdv@2002 - v1.1.x - videodriver
	//BOOL IsVideoDriverEnabled();
	BOOL VideoBuffer();
	int m_ScreenOffsetx;
	int m_ScreenOffsety;
	int DriverType;
	DWORD color[10];
	// Modif rdv@2002 Dis/enable input
	void SetDisableInput(bool enabled);
	void SetSW(int x,int y);
	//hook selection
	BOOL m_hookdriver;
	void SethookMechanism(BOOL hookall,BOOL hookdriver);
	bool m_UltraEncoder_used;
	rfb::Rect		m_Cliprect;//the region to check
	bool StopDriverWatches;

	PCHANGES_BUF pchanges_buf;
	CHANGES_BUF changes_buf;

	int GetNrMonitors();
	void GetPrimaryDevice();
	void GetSecondaryDevice();
	void Checkmonitors();

	// Implementation
protected:

	// Routines to hook and unhook us
	BOOL Startup();
	BOOL Shutdown();
	
	// Init routines called by the child thread
	BOOL InitDesktop();
	void KillScreenSaver();
	void KillWallpaper();
	void RestoreWallpaper();
	BOOL InitBitmap();
	BOOL InitWindow();
	BOOL ThunkBitmapInfo();
	BOOL SetPixFormat();
	BOOL SetPixShifts();
	//BOOL InitHooks();
	BOOL SetPalette();
	int m_timer;

	// Fetching pixel data to a buffer, and handling copyrects
	void CopyToBuffer(const rfb::Rect &rect, BYTE *scrBuff, UINT scrBuffSize);
	bool CalcCopyRects(rfb::UpdateTracker &tracker);

	// Routine to attempt enabling optimised DIBsection blits
	void EnableOptimisedBlits();

	// Convert a bit mask eg. 00111000 to max=7, shift=3
	static void MaskToMaxAndShift(DWORD mask, CARD16 &max, CARD8 &shift);
	
	// Enabling & disabling clipboard handling
	void SetClipboardActive(BOOL active) {m_clipboard_active = active;};

	// Modif sf@2002 - v1.1.0 - FastDetectChanges stuff
	void FastDetectChanges(rfb::Region2D &rgn, rfb::Rect &rect, int nZone, bool fTurbo);
	GridsList    m_lGridsList;   // List of changes detection grids
	WindowsList  m_lWList;		 // List of Windows handles  
	// HDC	         m_hDC;			 // Local Screen Device context to capture our Grid of pixels 
	int          m_nGridCycle;   // Cycle index for grid shifting

	// Modif sf@2002 - v1.1.0
	long         m_lLastMouseUpdateTime;
	long         m_lLastSlowClientTestTime;
	// long			m_lLastTempo;

	// sf@2002 - TextChat - No more used for now
	// bool m_fTextChatRunning;
	// TextChat* m_pCurrentTextChat;

	BOOL m_fCaptureAlphaBlending;
	// DATA

	// Generally useful stuff
	vncServer 		*m_server;
	std::auto_ptr<vncDesktopThread> m_thread;
	HWND			m_hwnd;
	UINT			m_timerid;
	HWND			m_hnextviewer;
	BOOL			m_clipboard_active;

	// device contexts for memory and the screen
	HDC				m_hmemdc;
	HDC				m_hrootdc;

	/// @modified Alexander Novak @date 05.11.2007 For supporting layered windows clipping
	// Device context for clipping layered windows
	HDC				m_hLayerDC;
	// Memory bitmap for clipping layered windows
	HBITMAP			m_memLayerBitmap;
	// Pointer for direct access to the memory bitmap for layered windows
	VOID*			m_DIBitsLayer;

	// New and old bitmaps
	HBITMAP			m_membitmap;
	boost::recursive_mutex		m_update_lock;

	rfb::Rect		m_bmrect;
	struct _BMInfo {
		BOOL			truecolour;
		BITMAPINFO		bmi;
		// Colormap info - comes straight after BITMAPINFO - **HACK**
		RGBQUAD			cmap[256];
	} m_bminfo;

	// Screen info
	rfbServerInitMsg	m_scrinfo;

	// These are the red, green & blue masks for a pixel
	DWORD			m_rMask, m_gMask, m_bMask;

	// This is always handy to have
	int				m_bytesPerRow;

	// Handle of the default cursor
	HCURSOR			m_hcursor;
	HCURSOR			m_hOldcursor; // sf@2002

	// Handle of the basic arrow cursor
	HCURSOR			m_hdefcursor;
	// The current mouse position
	rfb::Rect		m_cursorpos;

	// Boolean flag to indicate when the display resolution has changed
	BOOL			m_displaychanged;

	// Boolean flag to indicate whether or not an update trigger message
	// is already in the desktop thread message queue
	BOOL			m_update_triggered;

	// Extra vars used for the DIBsection optimisation
	VOID			*m_DIBbits;
	BOOL			m_formatmunged;

	// Info used for polling modes
	UINT			m_pollingcycle;
	// rfb::Rect		m_fullscreen; // sf@2002 - v1.1.0

	// Handling of the foreground window, to produce CopyRects
	HWND			m_foreground_window;
	rfb::Rect		m_foreground_window_rect;

	//SINGLE WINDOW
	void SWinit();
	int m_SWHeight;
	int m_SWWidth;
	BOOL m_SWSizeChanged;
	BOOL m_SWmoved;
	BOOL m_SWtoDesktop;
	int m_SWOffsetx;
	int m_SWOffsety;

	//DDIHOOK
	PCOPYDATASTRUCT pMyCDS;

	// Modif rdv@2002 - v1.1.x - videodriver
	vncVideoDriver *m_videodriver;
	BOOL InitVideoDriver();
 	void ShutdownVideoDriver();
	boost::recursive_mutex		m_videodriver_lock;

	// Modif input dis/enabke
	DWORD m_thread_hooks;
	BOOL ddihook;
	UINT OldPowerOffTimeout;
	bool OldCaptureBlending;
	//hook selection
	BOOL m_hookdll;
	BOOL On_Off_hookdll;
	BOOL m_hookswitch;
	BOOL Hookdll_Changed;
	BOOL m_hookinited;
	HANDLE m_hddihook;
	void StartStopddihook(BOOL enabled);
	void StartStophookdll(BOOL enabled);
	void InitHookSettings();
	HMODULE hModule;
	SetHooksFn SetHooks;
	UnSetHooksFn  UnSetHooks;
	SetKeyboardFilterHookFn SetKeyboardFilterHook;
	SetMouseFilterHookFn SetMouseFilterHook;
	pBlockInput pbi;
	HMODULE hUser32;
	BOOL Temp_Resolution;
	BOOL m_OrigpollingSet;
	BOOL m_Origpolling;
/*	BOOL Check24bit();*/

	
BOOL DriverWanted;
BOOL HookWanted;
BOOL DriverWantedSet;

//Multi monitor
monitor mymonitor[3];
int nr_monitors;
int current_monitor;
int asked_display;

/// @modified Alexander Novak @date 05.11.2007 For supporting layered windows clipping
private:
	struct SLayeredWindowData
	{
		HWND m_hwnd;
		rfb::Rect m_oldPosition;
	};
	std::list<SLayeredWindowData> m_lstLayeredWindows;
	CCritSectionSimpleObject m_csLayeredListGuard;
public:
	void AddToHiddenLayeredWindowList(HWND hwnd);
	void RemoveFromHiddenLayeredWindowList(HWND hwnd);
	inline BOOL IsCaptureAlphaBlending()
	{
		return m_fCaptureAlphaBlending;
	}
};