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


// vncClient.h

// vncClient class handles the following functions:
// - Recieves requests from the connected client and
//   handles them
// - Handles incoming updates properly, using a vncBuffer
//   object to keep track of screen changes
// It uses a vncBuffer and is passed the vncDesktop and
// vncServer to communicate with.

class vncClient;
typedef SHORT vncClientId;

#if (!defined(_WINVNC_VNCCLIENT))
#define _WINVNC_VNCCLIENT

#include <list>

typedef std::list<vncClientId> vncClientList;

// Includes
#include "stdhdrs_srv.h"
#include <NWL/Streaming/CAbstractStream.h>
#include <boost/shared_ptr.hpp>
#include <boostThreads/boostThreads.h>
// Custom
#include "CRCHost.h"
#include "vncDesktop.h"
#include "rfbRegion.h"
#include "rfbUpdateTracker.h"
#include "vncBuffer.h"
#include "vncEncodeMgr.h"
//#include "ZipUnZip32/zipUnZip32.h"
//#include "timer.h"

// The vncClient class itself
typedef UINT (WINAPI *pSendinput)(UINT,LPINPUT,INT);
#define SPI_GETMOUSESPEED         0x0070
#define SPI_SETMOUSESPEED         0x0071
#define MOUSEEVENTF_VIRTUALDESK	  0x4000


extern int CheckUserGroupPasswordUni(char * userin,char *password,const char *machine);

using namespace rfb;

class vncClientThread;

class vncClient
{
private:
	/// Session stop reason
	ESessionStopReason m_stopReason;
	/// Session stream
	boost::shared_ptr<CAbstractStream> m_stream;
public:
	// Constructor/destructor
	vncClient();
	~vncClient();

	/// TRUE if alive response pending
	LONG m_alivePending;
	/// TRUE if mode send is pending
	LONG m_modeSendPending;
	/// TRUE if new pixel format send is pending
	LONG m_pixelFormatSendPending;
	/// Pending pixel format message
	char m_pixelFormatMsgPending[sz_rfbSetPixelFormatMsg-1 /*1 is type offset*/];

	/// Set new stop reason
	/// @param stopReason new stop reason
	void SetStopReason(const ESessionStopReason stopReason);

	/// Receive stop reason
	/// @retun client stop reason
	ESessionStopReason GetStopReason();

	// Allow the client thread to see inside the client object
	friend class vncClientThread;
	friend class vncClientUpdateThread;
	friend class CRCHost;

	// Init
	virtual bool Init(	vncServer *server, 
						boost::shared_ptr<CAbstractStream> stream,
						bool shared,
						vncClientId newid );

	/// Send stop to remote side
	void SendStop();

	// Kill
	// The server uses this to close the client socket, causing the
	// client thread to fail, which in turn deletes the client object
	virtual void Kill();

	// Client manipulation functions for use by the server
	virtual void SetBuffer(vncBuffer *buffer);

	// Update handling functions
	// These all lock the UpdateLock themselves
	virtual void UpdateMouse();
	virtual void UpdateClipText(const char* text);
	virtual void UpdatePalette();
	virtual void UpdateLocalFormat();

	// Is the client waiting on an update?
	// YES IFF there is an incremental update region,
	//     AND no changed or copied updates intersect it
	virtual bool UpdateWanted() 
	{
	TRY_CATCH

		boost::recursive_mutex::scoped_lock l(GetUpdateLock());
		return  !m_incr_rgn.is_empty() &&
			m_incr_rgn.intersect(m_update_tracker.get_changed_region()).is_empty() &&
			m_incr_rgn.intersect(m_update_tracker.get_cached_region()).is_empty() &&
			m_incr_rgn.intersect(m_update_tracker.get_copied_region()).is_empty();

	CATCH_THROW("vncClient::UpdateWanted")
	};

	// Has the client sent an input event?
	virtual bool RemoteEventReceived()
	{
	TRY_CATCH
		bool result = m_remoteevent;
		m_remoteevent = false;
		return result;
	CATCH_THROW("vncClient::RemoteEventReceived")
	};

	// The UpdateLock
	// This must be held for a number of routines to be successfully invoked...
	inline boost::recursive_mutex& GetUpdateLock() 
	{
	TRY_CATCH
		return m_encodemgr.GetUpdateLock();
	CATCH_THROW("vncClient::GetUpdateLock")
	};

	// Functions for setting & getting the client settings
	virtual void EnableKeyboard(bool enable) {m_keyboardenabled = enable;};
	virtual bool KeyboardEnabled() {return m_keyboardenabled;};
	virtual void EnablePointer(bool enable) {m_pointerenabled = enable;};
	virtual bool PointerEnabled() {return m_pointerenabled;};
	virtual void EnableVisualPointer(bool enable) {m_visualPointer = enable;};
	virtual bool VisualPointerEnabled() {return m_visualPointer;};
	virtual void SetCapability(int capability) {m_capability = capability;};

	virtual int GetCapability() {return m_capability;};
	virtual const char *GetClientName();
	virtual vncClientId GetClientId() {return m_id;};

	// Disable/enable protocol messages to the client
	virtual void DisableProtocol();
	virtual void EnableProtocol();
	// resize desktop
	virtual bool SetNewSWSize(long w,long h,bool desktop);
	virtual void SetSWOffset(int x,int y);
	virtual void SetScreenOffset(int x,int y,int type);

	virtual void SetUltraViewer(bool fTrue) { m_fUltraViewer = fTrue;};
	virtual bool IsUltraViewer() { return m_fUltraViewer;};

	virtual void EnableCache(bool enabled);

	// sf@2002
	virtual void SetConnectTime(long lTime) {m_lConnectTime = lTime;};
	virtual long GetConnectTime() {return m_lConnectTime;};
	virtual bool IsSlowEncoding() {return m_encodemgr.IsSlowEncoding();};
	virtual bool IsUltraEncoding() {return m_encodemgr.IsUltraEncoding();};
	void SetProtocolVersion(rfbProtocolVersionMsg *protocolMsg);
	void Clear_Update_Tracker();
	void UpdateCursorShape();

	// sf@2002 
	// Update routines
protected:
	bool SendUpdate(rfb::SimpleUpdateTracker &update);
	bool SendRFBMsg(CARD8 type, BYTE *buffer, int buflen);
	bool SendRectangles(const rfb::RectVector &rects);
	bool SendRectangle(const rfb::Rect &rect);
	bool SendCopyRect(const rfb::Rect &dest, const rfb::Point &source);
	bool SendPalette();
	// CACHE
	bool SendCacheRectangles(const rfb::RectVector &rects);
	bool SendCacheRect(const rfb::Rect &dest);
	bool SendCacheZip(const rfb::RectVector &rects); // sf@2002

	// Tight - CURSOR HANDLING
	bool SendCursorShapeUpdate();
	// nyama/marscha - PointerPos
	bool SendCursorPosUpdate();
	bool SendLastRect(); // Tight

	void TriggerUpdateThread();

	void PollWindow(HWND hwnd);
	// Specialised client-side UpdateTracker
protected:

	// This update tracker stores updates it receives and
	// kicks the client update thread every time one is received

	class ClientUpdateTracker : public rfb::SimpleUpdateTracker 
	{
	public:
		ClientUpdateTracker() : m_client(0) {};
		virtual ~ClientUpdateTracker() {};

		void init(vncClient *client) {m_client=client;}

		virtual void add_changed(const rfb::Region2D &region) 
		{
		TRY_CATCH
			{
				// RealVNC 336 change - boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
				SimpleUpdateTracker::add_changed(region);
				m_client->TriggerUpdateThread();
			}
		CATCH_THROW("ClientUpdateTracker::add_changed")
		}

		virtual void add_cached(const rfb::Region2D &region) 
		{
		TRY_CATCH
			{
				// RealVNC 336 change - boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
				SimpleUpdateTracker::add_cached(region);
				m_client->TriggerUpdateThread();
			}
		CATCH_THROW("ClientUpdateTracker::add_cached")
		}
		
		virtual void add_copied(const rfb::Region2D &dest, const rfb::Point &delta) 
		{
		TRY_CATCH

			{
				// RealVNC 336 change - boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
				SimpleUpdateTracker::add_copied(dest, delta);
				m_client->TriggerUpdateThread();
			}
		CATCH_THROW("ClientUpdateTracker::add_copied")
		}

		virtual void clear() 
		{
		TRY_CATCH
			// RealVNC 336 change - boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
			SimpleUpdateTracker::clear();
		CATCH_THROW("ClientUpdateTracker::clear")
		}

		virtual void flush_update(rfb::UpdateInfo &info, const rfb::Region2D &cliprgn) 
		{
		TRY_CATCH
			// RealVNC 336 change - boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
			SimpleUpdateTracker::flush_update(info, cliprgn);
		CATCH_THROW("ClientUpdateTracker::flush_update")
		}

		virtual void flush_update(rfb::UpdateTracker &to, const rfb::Region2D &cliprgn) 
		{
		TRY_CATCH
			// RealVNC 336 change - boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
			SimpleUpdateTracker::flush_update(to, cliprgn);
		CATCH_THROW("ClientUpdateTracker::flush_update")
		}

		virtual void get_update(rfb::UpdateInfo &info) const 
		{
		TRY_CATCH
			// RealVNC 336 change - boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
			SimpleUpdateTracker::get_update(info);
		CATCH_THROW("ClientUpdateTracker::get_update")
		}

		virtual void get_update(rfb::UpdateTracker &to) const 
		{
		TRY_CATCH
			// RealVNC 336 change - boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
			SimpleUpdateTracker::get_update(to);
		CATCH_THROW("ClientUpdateTracker::get_update")
		}

		virtual bool is_empty() const
		{
		TRY_CATCH
			// RealVNC 336 change -  boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
			return SimpleUpdateTracker::is_empty();
		CATCH_THROW("ClientUpdateTracker::is_empty")
		}
	protected:
		vncClient *m_client;
	};

	friend class ClientUpdateTracker;

	// Make the update tracker available externally
public:

	rfb::UpdateTracker &GetUpdateTracker() {return m_update_tracker;};
	int				m_SWOffsetx;
	int				m_SWOffsety;
	int				m_ScreenOffsetx;
	int				m_ScreenOffsety;
	int				m_display_type;
	bool			m_NewSWUpdateWaiting;
	rfbProtocolVersionMsg ProtocolVersionMsg;
	bool client_settings_passed;

	// Internal stuffs
protected:
	// Per-client settings
	bool			m_IsLoopback;
	bool			m_keyboardenabled;
	bool			m_pointerenabled;
	bool			m_visualPointer;
	int				m_capability;
	vncClientId		m_id;
	long			m_lConnectTime;

	// Pixel translation & encoding handler
	vncEncodeMgr	m_encodemgr;

	// The server
	vncServer		*m_server;

	// The socket
	char			*m_client_name;

	// The client thread
	std::auto_ptr<vncClientThread> m_thread;


	// Count to indicate whether updates, clipboards, etc can be sent
	// to the client.  If 0 then OK, otherwise not.
	ULONG			m_disable_protocol;

	// User input information
	rfb::Rect		m_oldmousepos;
	bool			m_mousemoved;
	rfbPointerEventMsg	m_ptrevent;
	// vncKeymap		m_keymap;

	// Update tracking structures
	ClientUpdateTracker	m_update_tracker;

	// Client update transmission thread
	std::auto_ptr<vncClientUpdateThread> m_updatethread;

	// Requested update region & requested flag
	rfb::Region2D	m_incr_rgn;


	// When the local display is palettized, it sometimes changes...
	bool			m_palettechanged;

	// Information used in polling mode!
	bool			m_remoteevent;

	// Clipboard data
	char*			m_clipboard_text;

	//SINGLE WINDOW
	bool			m_use_NewSWSize;
	bool			m_NewSWDesktop;
	int				NewsizeW;
	int				NewsizeH;

	// CURSOR HANDLING
	bool			m_cursor_update_pending;
	bool			m_cursor_update_sent;
	// nyama/marscha - PointerPos
	bool			m_cursor_pos_changed;
	bool			m_use_PointerPos;
	POINT			m_cursor_pos;

	// sf@2004 - Delta Transfer
	char*	m_lpCSBuffer;
	int		m_nCSOffset;
	int		m_nCSBufferSize;

	// Modif sf@2002 - Scaling
	rfb::Rect		m_ScaledScreen;
	UINT			m_nScale;
	bool			fNewScale;
	bool			m_fPalmVNCScaling;
	bool			fFTRequest;

	// sf@2002 - DSM Plugin Rects alignement buffer
	BYTE* m_pNetRectBuf;
	bool m_fReadFromNetRectBuf;  // 
	bool m_fWriteToNetRectBuf;
	int m_nNetRectBufOffset;
	int m_nReadSize;
	int m_nNetRectBufSize;

	// sf@2002 
	BYTE* m_pCacheZipBuf;
	int m_nCacheZipBufSize;
	BYTE* m_pRawCacheZipBuf;
	int m_nRawCacheZipBufSize;

	bool m_fUltraViewer; // sf@2002 

	//stats
	int totalraw;

	pSendinput Sendinput;
	// Modif cs@2005
#ifdef DSHOW
	HANDLE m_hmtxEncodeAccess;
#endif

};

#endif
