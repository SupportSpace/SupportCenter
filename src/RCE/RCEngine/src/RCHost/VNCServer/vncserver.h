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


// vncServer.h

// vncServer class handles the following functions:
// - Allowing clients to be dynamically added and removed
// - Propagating updates from the local vncDesktop object
//   to all the connected clients
// - Propagating mouse movements and keyboard events from
//   clients to the local vncDesktop
// It also creates the vncSockConnect
// servers, which respectively allow connections via sockets
// and via the ORB interface

class vncServer;

#if (!defined(_WINVNC_VNCSERVER))
#define _WINVNC_VNCSERVER

// Custom
#include "vncClient.h"
#include "rfbRegion.h"

// Includes
#include "stdhdrs_srv.h"
#include <boostThreads/boostThreads.h>
#include <list>
#include "RCEngine.h"
#include <NWL/Streaming/CAbstractStream.h>
#include <boost/shared_ptr.hpp>
#include <HelperService/CSrvComm.h>

typedef bool (WINAPI *WTSREGISTERSESSIONNOTIFICATION)(HWND, DWORD);
typedef bool (WINAPI *WTSUNREGISTERSESSIONNOTIFICATION)(HWND);
#define WM_WTSSESSION_CHANGE            0x02B1
#define WTS_CONSOLE_CONNECT                0x1
#define WTS_CONSOLE_DISCONNECT             0x2
#define WTS_REMOTE_CONNECT                 0x3
#define WTS_REMOTE_DISCONNECT              0x4
#define WTS_SESSION_LOGON                  0x5
#define WTS_SESSION_LOGOFF                 0x6
#define WTS_SESSION_LOCK                   0x7
#define WTS_SESSION_UNLOCK                 0x8
#define WTS_SESSION_REMOTE_CONTROL         0x9

// Define a datatype to handle lists of windows we wish to notify
typedef std::list<HWND> vncNotifyList;

// Some important constants;
const int MAX_CLIENTS = 128;

class CRCHost;
// The vncServer class itself
class vncServer
{
private:
	CRCHost *m_RCHost;
	boost::shared_ptr<CSrvSTDQueueComm> m_srvCommunicator;

	/// Returns true if process isolated
	static bool IsProcessIsolated();

	/// true if process is running in low integrity
	bool m_inputProxyNeeded;
	bool m_hooksProxyNeeded;

public:

	// Constructor/destructor
	vncServer(CRCHost* RCHost);
	~vncServer();

	/// Returns true if process is running in low integrity
	inline bool IsInputProxyNeeded()
	{
		return m_inputProxyNeeded;
	}

	/// Returns true if process is running in low integrity
	inline bool IsHooksProxyNeeded()
	{
		return m_hooksProxyNeeded;
	}


	inline CRCHost* GetRCHost()
	{
		return m_RCHost;
	}

	inline CSrvSTDQueueComm* GetSrvCommunicator()
	{
		if (NULL == m_srvCommunicator.get())
		{
			/// Try to create communicatior if there's none for a moment
			try
			{
				m_srvCommunicator.reset(new CSrvSTDQueueComm(DEF_WND_CLS_NAME));
			}
			catch(CExceptionBase&)
			{
				// Doesn't logging this exception, since it called to often
			}
		}
		return m_srvCommunicator.get();
	}

	/// Enables or disables protocol for clients
	void EnableProtocol(const bool enable);

	// Client handling functions
	virtual vncClientId AddClient(boost::shared_ptr<CAbstractStream> stream, bool shared, int capability, /*bool keysenabled, bool ptrenabled,*/rfbProtocolVersionMsg *protocolMsg);
	virtual bool Authenticated(vncClientId client);
	virtual void KillClient(vncClientId client);
	virtual void KillClient(LPSTR szClientName); // sf@2002

	virtual UINT AuthClientCount();
	virtual UINT UnauthClientCount();

	/// returns true if we have client with visual pointer
	virtual bool HasVisualPointerClient();

	virtual void KillAuthClients();
	virtual void ListAuthClients(HWND hListBox);
	virtual void WaitUntilAuthEmpty();

	virtual void KillUnauthClients();
	virtual void WaitUntilUnauthEmpty();

	// Are any clients ready to send updates?
	virtual bool UpdateWanted();

	// Has at least one client had a remote event?
	virtual bool RemoteEventReceived();

	// Client info retrieval/setup
	virtual vncClient* GetClient(vncClientId clientid);
	virtual vncClientList ClientList();

	virtual void SetCapability(vncClientId client, int capability);
	virtual void SetKeyboardEnabled(vncClientId client, bool enabled);
	virtual void SetPointerEnabled(vncClientId client, bool enabled);
	virtual void SetVisualPointerEnabled(vncClientId client, bool enabled);

	virtual int GetCapability(vncClientId client);
	virtual const char* GetClientName(vncClientId client);

	// Let a client remove itself
	virtual void RemoveClient(vncClientId client, bool notify=true);

	// Connect/disconnect notification
	virtual bool AddNotify(HWND hwnd);
	virtual bool RemNotify(HWND hwnd);

	// Modif sf@2002 - Single Window
	virtual void SingleWindow(bool fEnabled) { m_SingleWindow = fEnabled; };
	virtual bool SingleWindow() { return m_SingleWindow; };
	virtual void SetSingleWindowName(const char *szName);
	virtual char *GetWindowName() { return m_szWindowName; };
	inline vncDesktop* GetDesktopPointer() {return m_desktop;}
	virtual void SetNewSWSize(long w,long h,bool desktop);
	virtual void SetSWOffset(int x,int y);
	virtual void SetScreenOffset(int x,int y,int type);

	virtual bool All_clients_initialalized();

	// Lock to protect the client list from concurrency - lock when reading/updating client list
	boost::recursive_mutex			m_clientsLock;

protected:
	// Send a notification message
	virtual void DoNotify(UINT message, WPARAM wparam, LPARAM lparam);

public:
	// Update handling, used by the screen server
	virtual rfb::UpdateTracker &GetUpdateTracker() {return m_update_tracker;};
	virtual void UpdateMouse();
	virtual void UpdateClipText(const char* text);
	virtual void UpdatePalette();
	virtual void UpdateLocalFormat();

	// Polling mode handling
	virtual void PollUnderCursor(bool enable) {m_poll_undercursor = enable;};
	virtual bool PollUnderCursor() {return m_poll_undercursor;};
	virtual void PollForeground(bool enable) {m_poll_foreground = enable;};
	virtual bool PollForeground() {return m_poll_foreground;};
	virtual void PollFullScreen(bool enable) {m_poll_fullscreen = enable;};
	virtual bool PollFullScreen() {return m_poll_fullscreen;};

	virtual void Driver(bool enable);
	virtual bool Driver() {return m_driver;};
	virtual void Hook(bool enable);
	virtual bool Hook() {return m_hook;};
	virtual void Virtual(bool enable) {m_virtual = enable;};
	virtual bool Virtual() {return m_virtual;};
	virtual void SetHookings();

	virtual void PollConsoleOnly(bool enable) {m_poll_consoleonly = enable;};
	virtual bool PollConsoleOnly() {return m_poll_consoleonly;};
	virtual void PollOnEventOnly(bool enable) {m_poll_oneventonly = enable;};
	virtual bool PollOnEventOnly() {return m_poll_oneventonly;};

	// Client manipulation of the clipboard
	virtual void UpdateLocalClipText(LPSTR text);

	// Name and port number handling
	// TightVNC 1.2.7
	virtual void SetName(const char * name);
 
	// Remote input handling
	virtual void EnableRemoteInputs(bool enable);
	virtual bool RemoteInputsEnabled();

	// Local input handling
	virtual void DisableLocalInputs(bool disable);
	virtual bool LocalInputsDisabled();

	// General connection handling
	virtual void SetConnectPriority(UINT priority) {m_connect_pri = priority;};
	virtual UINT ConnectPriority() {return m_connect_pri;};


	virtual void GetScreenInfo(int &width, int &height, int &depth);

	// Connection querying settings
	virtual void SetQuerySetting(const UINT setting) {m_querysetting = setting;};
	virtual UINT QuerySetting() {return m_querysetting;};
	virtual void SetQueryAccept(const UINT setting) {m_queryaccept = setting;};
	virtual UINT QueryAccept() {return m_queryaccept;};
	virtual void SetQueryTimeout(const UINT setting) {m_querytimeout = setting;};
	virtual UINT QueryTimeout() {return m_querytimeout;};

	// Timeout for automatic disconnection of idle connections
	virtual void SetAutoIdleDisconnectTimeout(const UINT timeout) {m_idle_timeout = timeout;};
	virtual UINT AutoIdleDisconnectTimeout() {return m_idle_timeout;};

	// sf@2002 - v1.1.x - Server Default Scale
	virtual UINT GetDefaultScale();
	virtual bool SetDefaultScale(int nScale);
	virtual bool BlankMonitorEnabled() {return m_fBlankMonitorEnabled;};
	virtual void BlankMonitorEnabled(bool fEnable) {m_fBlankMonitorEnabled = fEnable;};

	// sf@2002 - Cursor handling
	virtual void EnableXRichCursor(bool fEnable);
	virtual bool IsXRichCursorEnabled() {return m_fXRichCursor;}; 

	// sf@2002
	virtual void DisableCacheForAllClients();
	virtual bool IsThereASlowClient();
	virtual bool IsThereAUltraEncodingClient();

	// sf@2002 - Turbo Mode
	virtual void TurboMode(bool fEnabled) { m_TurboMode = fEnabled; };
	virtual bool TurboMode() { return m_TurboMode; };

	virtual bool CaptureAlphaBlending(){return m_fCaptureAlphaBlending;};
	virtual void CaptureAlphaBlending(bool fEnabled){m_fCaptureAlphaBlending = fEnabled;};
	virtual bool BlackAlphaBlending(){return m_fBlackAlphaBlending;};
	virtual void BlackAlphaBlending(bool fEnabled){m_fBlackAlphaBlending = fEnabled;};

	virtual void Clear_Update_Tracker();
	virtual void UpdateCursorShape();

	bool IsClient(vncClient* pClient);

	/// Hide or restore wallpaper depending current state
	void SetHideWallpaper(const bool hideWallpaper);
	/// Hide or restore wallpaper
	void ChangeWallpaperState(const bool hideWallpaper);

protected:
	// The vncServer UpdateTracker class
	// Behaves like a standard UpdateTracker, but propagates update
	// information to active clients' trackers

	class ServerUpdateTracker : public rfb::UpdateTracker {
	public:
		ServerUpdateTracker() : m_server(0) {};

		virtual void init(vncServer *server) {m_server=server;};

		virtual void add_changed(const rfb::Region2D &region);
		virtual void add_cached(const rfb::Region2D &region);
		virtual void add_copied(const rfb::Region2D &dest, const rfb::Point &delta);
	protected:
		vncServer *m_server;
	};

	friend class ServerUpdateTracker;

	ServerUpdateTracker	m_update_tracker;

	// Internal stuffs
protected:
	static void*	pThis;

	// The desktop handler
	vncDesktop			*m_desktop;

	// General preferences
	bool				m_enable_remote_inputs;
	bool				m_disable_local_inputs;
	int					m_lock_on_exit;
	int					m_connect_pri;
	UINT				m_querysetting;
	UINT				m_queryaccept;
	UINT				m_querytimeout;
	bool				m_queryifnologon;
 	UINT				m_idle_timeout;
	UINT				m_retry_timeout;

	// Polling preferences
	bool				m_poll_fullscreen;
	bool				m_poll_foreground;
	bool				m_poll_undercursor;

	bool				m_poll_oneventonly;
	bool				m_poll_consoleonly;

	bool				m_driver;
	bool				m_hook;
	bool				m_virtual;
	bool				sethook;

	// Name of this desktop
	char				*m_name;

	// The client lists - list of clients being authorised and ones
	// already authorised
	vncClientList		m_unauthClients;
	vncClientList		m_authClients;
	vncClient			*m_clientmap[MAX_CLIENTS];
	vncClientId			m_nextid;

	// Lock to protect the client list from concurrency - lock when reading/updating client list
//	boost::recursive_mutex			m_clientsLock;
	// Lock to protect the desktop object from concurrency - lock when updating client list
	boost::recursive_mutex			m_desktopLock;

	// Signal set when a client removes itself
	boost::condition		*m_clientquitsig;

	// Set of windows to send notifications to
	vncNotifyList		m_notifyList;

		// Modif sf@2002 - Single Window
	bool    m_SingleWindow;
	char    m_szWindowName[32]; // to keep the window name

	// Modif sf@2002
	bool    m_TurboMode;

	// Modif sf@2002 - v1.1.x
	// bool    m_fQueuingEnabled;
	bool    m_fBlankMonitorEnabled;
	int     m_nDefaultScale;

	// sf@2002 - Cursor handling
	bool m_fXRichCursor; 

	// sf@2005
	bool m_fCaptureAlphaBlending;
	bool m_fBlackAlphaBlending;
	/// Is wallpaper hidden
	bool m_fWallpaperHidden;

	HINSTANCE   hWtsLib;

/// @modified Alexander Novak @date 05.11.2007 Added the method to support layered windows clipping
public:
	void HideLayeredWindow(HWND hwnd, bool showWindow);
};

#endif
