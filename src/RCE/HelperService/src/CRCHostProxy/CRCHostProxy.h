#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRCHostProxy.h
///
///  Proxy application helper for RCHost. Capture/Input for winlogon, and so on
///
///  @author "Archer Software" Sogin M. @date 01.10.2007
///
////////////////////////////////////////////////////////////////////////

#include "resource.h"
#include <AidLib/Loki/Singleton.h>
#include <AidLib/CScopedTracker/CScopedTracker.h>
#include <HelperService/CSrvComm.h>
#include <AidLib/CThread/CThread.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include "CWallpaperSwitch.h"
#include <AidLib/Utils/Utils.h>
#include "CFirewallRelaxator.h"

/// Proxy application helper for RCHost. Capture/Input for winlogon, and so on
class CRCHostProxy  
	:	private CThread,
		public CInstanceTracker
{
	friend struct Loki::CreateUsingNew<CRCHostProxy>;
//	friend class CSingleton<CRCHostProxy>;
private:

	/// Commutator with client application
	CSrvSTDQueueComm m_srvCommutator;

	/// Wallpaper switcher: shows/hides wallpaper
	CWallpaperSwitch m_wallpaperSwitch;

	/// Allows incoming connections for 
	CFirewallRelaxator m_firewallRelaxator;

	/// private ctor to protect from multiple instances
	CRCHostProxy();
	virtual ~CRCHostProxy();

	/// Hides/shows taskbar
	CToggleTaskBar m_toggleTaskBar;

	/// current instance
	HINSTANCE m_instance;
	/// The title bar text
	TCHAR m_title[MAX_PATH];
	/// the main window class name
	TCHAR m_windowClass[MAX_PATH];
	/// tid for vncHooksDll
	int m_hooksTid;

	/// VNCHooks
	bool m_hookinited;
	typedef BOOL (*SetHooksFn)(DWORD thread_id,UINT UpdateMsg,UINT CopyMsg,UINT MouseMsg,BOOL ddihook);
	typedef BOOL (*UnSetHooksFn)(DWORD thread_id);
	typedef BOOL (*SetKeyboardFilterHookFn)(BOOL activate);	
	typedef BOOL (*SetMouseFilterHookFn)(BOOL activate);
	SetHooksFn m_setHooks;
	UnSetHooksFn m_unSetHooks;
	SetKeyboardFilterHookFn m_setKeyboardFilterHook;
	SetMouseFilterHookFn m_setMouseFilterHook;
	CScopedTracker<HMODULE> m_vncHooks;

	/// Registers the window class.
	ATOM RegisterClass(HINSTANCE instance);
	/// Saves instance handle and creates main window
	void InitInstance(HINSTANCE instance, int cmdShow);
	/// Main window procedure
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	// Message handler for about box.
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	// Client message handlers-------------------------------------------------------

	/// Send mouse event message handler
	DWORD OnSendMouseEventMsg(WORD code, char* msg, const WORD msgSize);

	/// Send keyboard event message handler
	DWORD OnSendKbdEventMsg(WORD code, char* msg, const WORD msgSize);

	/// Get winlogon desktop handle message handler
	DWORD OnGetWinLogonDesktopMsg(WORD code, char* msg, const WORD msgSize);

	/// Set VNCHooks message handler
	DWORD OnSetVNCHooksMsg(WORD code, char* msg, const WORD msgSize);

	/// Send ctrl + alt + del message handler
	DWORD OnSendCAD(WORD code, char* msg, const WORD msgSize);

	/// Handles wallpaper reset requests
	DWORD OnResetWallpaper(WORD code, char* msg, const WORD msgSize);

	/// Handles hide/show taskbar requests
	DWORD OnToggleTaskBar(WORD code, char* msg, const WORD msgSize);

	/// Handles requests for allowing / disabling incoming socket connections
	DWORD OnAllowConnections(WORD code, char* msg, const WORD msgSize);

	/// Handles start broker requests 
	DWORD OnStartBroker(WORD code, char* msg, const WORD msgSize);

	/// Internal thread entry point
	virtual void Execute(void*);

	/// Returns winlogon desktop
	static boost::shared_ptr<boost::remove_pointer<HDESK>::type> GetWinlogonDesktop();

	/// Loads vncHooks dll
	void LoadVNCHooks();

public:
	/// Application entry point
	int Run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);

	/// Shutdown proxy with thread id
	/// @param thread id of proxy to shutdown
	static int Shutdown(int threadId);
};

/// To avoid different singleton types use this macro to access CRCHostProxy instance 
#define RCHOST_PROXY_INSTANCE Loki::SingletonHolder<CRCHostProxy, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
//#define RCHOST_PROXY_INSTANCE CSingleton<CRCHostProxy>::instance()

