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
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See teh
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// If the source code for the VNC system is fnot available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


// vncClient.cpp

// The per-client object.  This object takes care of all per-client stuff,
// such as socket input and buffering of updates.

// vncClient class handles the following functions:
// - Recieves requests from the connected client and
//   handles them
// - Handles incoming updates properly, using a vncBuffer
//   object to keep track of screen changes
// It uses a vncBuffer and is passed the vncDesktop and
// vncServer to communicate with.

// Includes
#include "stdhdrs_srv.h"
#include "RCE_resource.h"
#include "CVisualPointer.h"

// Custom
#include "CTokenCatcher.h"
#include "vncServer.h"
#include "vncClient.h"
//#include "VSocket.h"
#include "vncDesktop.h"
#include "rfbRegion.h"
#include "vncBuffer.h"
#include "vncService.h"
#include "vncKeymap.h"

#include "rfb/dh.h"

#include "zlib/zlib.h" // sf@2002
#include "mmSystem.h" // sf@2002
#include "shlobj.h"
#include <AidLib/CThread/CThreadLS.h>

#pragma warning( disable: 4996 )//<func> was declared deprecated

// #include "rfb.h"

typedef bool (WINAPI *PGETDISKFREESPACEEX)(LPCSTR,PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
// vncClient update thread class

CARD16 screenWidth;
CARD16 screenHeight;

class vncClientUpdateThread
{
private:
	/// Thread object
	std::auto_ptr<boost::thread> m_thread;
	/// Fields
protected:
	vncClient *m_client;
	boost::condition *m_signal;
	boost::condition *m_sync_sig;
	bool m_active;
	bool m_enable;

public:
	virtual ~vncClientUpdateThread();

	/// Init
	bool Init(vncClient *client);

	/// Kick the thread to send an update
	void Trigger();

	/// Perform threads join
	void join();

	/// Kill the thread
	void Kill();

	/// Disable/enable updates
	void EnableUpdates(bool enable, boost::recursive_mutex::scoped_lock &l);

	void get_time_now(unsigned long* abs_sec, unsigned long* abs_nsec);

	/// The main thread function
    virtual void run_undetached();

};

// Modif cs@2005
#ifdef DSHOW
class MutexAutoLock 
{
public:
	MutexAutoLock(HANDLE* phMutex) 
	{ 
	TRY_CATCH

		m_phMutex = phMutex;

		if(WAIT_OBJECT_0 != WaitForSingleObject(*phMutex, INFINITE))
		{
			//vnclog.Print(LL_INTERR, VNCLOG("Could not get access to the mutex\n"));
			Log.Add(_ERROR_, "Could not get access to the mutex");
		}
	CATCH_THROW("MutexAutoLock::MutexAutoLock")
	}

	~MutexAutoLock() 
	{ 
	TRY_CATCH
		ReleaseMutex(*m_phMutex);
	CATCH_LOG("MutexAutoLock::~MutexAutoLock")
	}

	HANDLE* m_phMutex;
};
#endif

void vncClientUpdateThread::join()
{
TRY_CATCH
	if (m_thread.get())
		m_thread->join();
CATCH_THROW()
}

bool vncClientUpdateThread::Init(vncClient *client)
{
TRY_CATCH

	m_client = client;
	boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
	m_signal = new boost::condition();
	m_sync_sig = new boost::condition();
	m_active = true;
	m_enable = m_client->m_disable_protocol == 0;
	if (m_signal && m_sync_sig) 
	{
		m_thread.reset(new boost::thread( boost::bind( &vncClientUpdateThread::run_undetached, this )));
		return true;
	}
	return false;

CATCH_THROW("vncClientUpdateThread::Init")
}

vncClientUpdateThread::~vncClientUpdateThread()
{
TRY_CATCH

	if (m_signal) delete m_signal;
	if (m_sync_sig) delete m_sync_sig;
	m_client->m_updatethread.reset();

CATCH_LOG("vncClientUpdateThread::~vncClientUpdateThread")
}

void vncClientUpdateThread::Trigger()
{
TRY_CATCH

	// ALWAYS lock client UpdateLock before calling this!
	// Only trigger an update if protocol is enabled
	if (m_client->m_disable_protocol == 0) {
		m_signal->notify_one();
	}

CATCH_THROW("vncClientUpdateThread::Trigger")
}

void vncClientUpdateThread::Kill()
{
TRY_CATCH

	Log.Add(_MESSAGE_,"kill update thread\n");
	boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
	m_active=false;
	m_signal->notify_one();

CATCH_THROW("vncClientUpdateThread::Kill")
}


void vncClientUpdateThread::get_time_now(unsigned long* abs_sec, unsigned long* abs_nsec)
{
TRY_CATCH

    static int days_in_preceding_months[12]
	= { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    static int days_in_preceding_months_leap[12]
	= { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };

    SYSTEMTIME st;

    GetSystemTime(&st);
    *abs_nsec = st.wMilliseconds * 1000000;

    // this formula should work until 1st March 2100

    DWORD days = ((st.wYear - 1970) * 365 + (st.wYear - 1969) / 4
		  + ((st.wYear % 4)
		     ? days_in_preceding_months[st.wMonth - 1]
		     : days_in_preceding_months_leap[st.wMonth - 1])
		  + st.wDay - 1);

    *abs_sec = st.wSecond + 60 * (st.wMinute + 60 * (st.wHour + 24 * days));

CATCH_THROW("vncClientUpdateThread::get_time_now")
}

void vncClientUpdateThread::EnableUpdates(bool enable, boost::recursive_mutex::scoped_lock &l)
{
TRY_CATCH
	m_enable = enable;
	m_signal->notify_one();
	unsigned long now_sec, now_nsec;
	get_time_now(&now_sec, &now_nsec);
	m_sync_sig->wait(l);
CATCH_THROW("vncClientUpdateThread::EnableUpdates")
}

void vncClientUpdateThread::run_undetached()
{
SET_THREAD_LS;
TRY_CATCH

	rfb::SimpleUpdateTracker update;
	rfb::Region2D clipregion;
	char *clipboard_text = 0;
	update.enable_copyrect(true);
	bool send_palette = false;

	unsigned long updates_sent=0;

	///TODO: Hi or nehi?
	// Set client update threads to high priority
	// *** set_priority(omni_thread::PRIORITY_HIGH);

	while(true)
	{
		// Block waiting for an update to send
		{
			boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
			m_client->m_incr_rgn = m_client->m_incr_rgn.union_(clipregion);

			// We block as long as updates are disabled, or the client
			// isn't interested in them, unless this thread is killed.
			if (updates_sent < 1) 
			{
				while (	m_active 
						&& (!m_enable 
							|| (m_client->m_update_tracker.get_changed_region().intersect(m_client->m_incr_rgn).is_empty() &&
								m_client->m_update_tracker.get_copied_region().intersect(m_client->m_incr_rgn).is_empty() &&
								m_client->m_update_tracker.get_cached_region().intersect(m_client->m_incr_rgn).is_empty() &&
								!m_client->m_clipboard_text &&
								!m_client->m_cursor_pos_changed // nyama/marscha - PointerPos
								)
							)
					) 
				{
					// Issue the synchronisation signal, to tell other threads
					// where we have got to
					m_sync_sig->notify_all();

					// Wait to be kicked into action
					m_signal->wait(l);
				}
			}
			else
			{
				while (m_active && (
						!m_enable || (
							m_client->m_update_tracker.get_changed_region().intersect(m_client->m_incr_rgn).is_empty() &&
							m_client->m_update_tracker.get_copied_region().intersect(m_client->m_incr_rgn).is_empty() &&
							m_client->m_update_tracker.get_cached_region().intersect(m_client->m_incr_rgn).is_empty() &&
							!m_client->m_encodemgr.IsCursorUpdatePending() &&
							!m_client->m_clipboard_text &&
							!m_client->m_NewSWUpdateWaiting &&
							!m_client->m_cursor_pos_changed // nyama/marscha - PointerPos
							)
						)
					) 
				{
					// Issue the synchronisation signal, to tell other threads
					// where we have got to
					m_sync_sig->notify_all();

					// Wait to be kicked into action
					m_signal->wait(l);

					if (m_active && InterlockedCompareExchange(&m_client->m_alivePending,FALSE,TRUE))
					{
						char code = rfbAliveMsg;
						Log.Add(_MESSAGE_,_T("Sending alive response"));
						m_client->m_stream->Send(&code, 1);
					}


				}
			}
			// If the thread is being killed then quit
			if (!m_active)
				break;

			// SEND AN UPDATE!
			// The thread is active, updates are enabled, and the
			// client is expecting an update - let's see if there
			// is anything to send.

			// Modif sf@2002 - FileTransfer - Don't send anything if a file transfer is running !
			// if (m_client->m_fFileTransferRunning) return 0;
			// if (m_client->m_pTextChat->m_fTextChatRunning) return 0;

			// sf@2002
			// New scale requested, we do it before sending the next Update
			if (m_client->fNewScale)
			{
				// Send the new framebuffer size to the client
				rfb::Rect ViewerSize = m_client->m_encodemgr.m_buffer->GetViewerSize();
				
				// Copyright (C) 2001 - Harakan software
				if (m_client->m_fPalmVNCScaling)
				{
					rfb::Rect ScreenSize = m_client->m_encodemgr.m_buffer->GetSize();
					rfbPalmVNCReSizeFrameBufferMsg rsfb;
					rsfb.type = rfbPalmVNCReSizeFrameBuffer;
					rsfb.desktop_w = Swap16IfLE(ScreenSize.br.x);
					rsfb.desktop_h = Swap16IfLE(ScreenSize.br.y);
					rsfb.buffer_w = Swap16IfLE(ViewerSize.br.x);
					rsfb.buffer_h = Swap16IfLE(ViewerSize.br.y);
					///TODO: handle stream errors here?
					m_client->m_stream->Send(	(char*)&rsfb,
												sz_rfbPalmVNCReSizeFrameBufferMsg );
				}
				else // eSVNC-UltraVNC Scaling
				{
					rfbResizeFrameBufferMsg rsmsg;
					rsmsg.type = rfbResizeFrameBuffer;
					rsmsg.framebufferWidth  = Swap16IfLE(ViewerSize.br.x);
					rsmsg.framebufferHeigth = Swap16IfLE(ViewerSize.br.y);
					///TODO: handle stream errors here?
					m_client->m_stream->Send( (char*)&rsmsg, sz_rfbResizeFrameBufferMsg );
				}
				m_client->m_encodemgr.m_buffer->ClearCache();
				m_client->fNewScale = false;
				m_client->m_fPalmVNCScaling = false;
				// return 0;
			}

			// Has the palette changed?
			send_palette = m_client->m_palettechanged;
			m_client->m_palettechanged = false;

			// Fetch the incremental region
			clipregion = m_client->m_incr_rgn;
			m_client->m_incr_rgn.clear();

			// Receive the clipboard data, if any
			if (m_client->m_clipboard_text) 
			{
				clipboard_text = m_client->m_clipboard_text;
				m_client->m_clipboard_text = 0;
			}
			// Receive the update details from the update tracker
			m_client->m_update_tracker.flush_update(update, clipregion);

			//if (!m_client->m_encodemgr.m_buffer->m_desktop->IsVideoDriverEnabled())
			//TEST if (!m_client->m_encodemgr.m_buffer->m_desktop->m_hookdriver)
			{
				// Render the mouse if required

				if (updates_sent > 1 ) /*m_client->m_cursor_update_pending =*/ 
					m_client->m_encodemgr.WasCursorUpdatePending();
				if (updates_sent == 1 ) 
					m_client->m_cursor_update_pending = true;

				if (!m_client->m_cursor_update_sent && !m_client->m_cursor_update_pending) 
				{
					if (m_client->m_mousemoved)
					{
						// Re-render its old location
						m_client->m_oldmousepos =
							m_client->m_oldmousepos.intersect(m_client->m_ScaledScreen); // sf@2002
						if (!m_client->m_oldmousepos.is_empty())
							update.add_changed(m_client->m_oldmousepos);

						// And render its new one
						m_client->m_encodemgr.m_buffer->GetMousePos(m_client->m_oldmousepos);
						m_client->m_oldmousepos =
							m_client->m_oldmousepos.intersect(m_client->m_ScaledScreen);
						if (!m_client->m_oldmousepos.is_empty())
							update.add_changed(m_client->m_oldmousepos);

						m_client->m_mousemoved = false;
					}
				}
			}
		} //End of wait block
		// SEND THE CLIPBOARD
		// If there is clipboard text to be sent then send it
		// Also allow in loopbackmode
		// Loopback mode with winvncviewer will cause a loping
		// But ssh is back working
		if (clipboard_text)
		{
			Log.Add(_MESSAGE_,"New clipboaed text detected, sending it to viewer");
			rfbServerCutTextMsg message;
			message.length = Swap32IfLE(strlen(clipboard_text));
			try
			{
				if (!m_client->SendRFBMsg(rfbServerCutText,(BYTE *) &message, sizeof(message)))
					throw MCStreamException("failed to m_client->SendRFBMsg");
				m_client->m_stream->Send(clipboard_text, strlen(clipboard_text));
			}
			catch(CStreamException& e)
			{
				MLog_Exception(e);
				m_client->m_updatethread.reset();
				m_signal->notify_all(); // Releasing all waiting for us threads
				break;
			}
			free(clipboard_text);
			clipboard_text = 0;
		}

		// SEND AN UPDATE
		// We do this without holding locks, to avoid network problems
		// stalling the server.
		// Update the client palette if necessary
		try
		{
			if (send_palette) 
			{
				m_client->SendPalette();
			}

			// Send updates to the client - this implicitly clears
			// the supplied update tracker
			if (m_client->SendUpdate(update)) 
			{
				updates_sent++;
				clipregion.clear();
			}
		}
		catch(CStreamException& e)
		{
			MLog_Exception(e);
			break;
		}
		if (m_thread.get())
			m_thread->yield();
	}
CATCH_LOG()
TRY_CATCH
	m_sync_sig->notify_all();
CATCH_LOG()
}

/// vncClient thread class
class vncClientThread
{
	// Fields
protected:
	std::auto_ptr<boost::thread> m_thread;
	boost::shared_ptr<CAbstractStream> m_stream;
	vncServer *m_server;
	vncClient *m_client;
	bool m_shared;
	bool m_ms_logon;

public:
	/// Initializes object instance
	vncClientThread(	vncClient *client, 
						vncServer *server,	
						boost::shared_ptr<CAbstractStream> stream,
						bool auth,
						bool shared);
	virtual ~vncClientThread();

	/// Sub-Init routines
	virtual bool InitAuthenticate();
	virtual void Initialize();
	virtual void DoMainLoop();
	/// Try to restart session
	virtual void TrySessionRestart();

	/// The main thread function (thread entry point)
	void run();
};

vncClientThread::~vncClientThread()
{
TRY_CATCH

	// If we have a client object then delete it
	if (m_client != NULL)
		delete m_client;

	if (m_thread.get() != NULL)
		m_thread->join();

CATCH_LOG()
}

vncClientThread::vncClientThread(vncClient *client, vncServer *server, boost::shared_ptr<CAbstractStream> stream, bool auth, bool shared)
	:	m_server(server),
		m_stream(stream),
		m_client(client),
		m_shared(shared)
{
TRY_CATCH
	m_thread.reset(new boost::thread(boost::bind( &vncClientThread::run, this )));
CATCH_THROW()
}


bool vncClientThread::InitAuthenticate()
{
TRY_CATCH

	try
	{
		/// Sending start command
		tstring startToken(START_COMMAND);
		m_stream->Send(startToken.c_str(),startToken.length());

		/// Waiting session start
		char code;
		CTokenCatcher tockenCatcher(startToken.c_str(),startToken.length());
		while(true)
		{
			m_stream->Receive(&code,1);
			if (tockenCatcher.Send(&code,1))
				break;
		}
	}
	catch(CStreamException&)
	{
		/// Preventing unstarted sessions in ActiveX
		m_server->GetRCHost()->NotifySessionStarted(m_client->GetClientId());
		m_server->GetRCHost()->NotifySessionStopped(m_client->GetClientId(),STREAM_ERROR);
		throw;
	}



	// Read the client's initialisation message
	rfbClientInitMsg client_ini;
	m_stream->Receive((char *)&client_ini, sz_rfbClientInitMsg);

	// Tell the server that this client is ok
	return m_server->Authenticated(m_client->GetClientId());
	
CATCH_THROW("vncClientThread::InitAuthenticate")
}


void ClearKeyState(BYTE key)
{
TRY_CATCH

	// This routine is used by the VNC client handler to clear the
	// CAPSLOCK, NUMLOCK and SCROLL-LOCK states.
	BYTE keyState[256];
	GetKeyboardState((LPBYTE)&keyState);
	if(keyState[key] & 1)
	{
		// Simulate the key being pressed
		keybd_event(key, 0, KEYEVENTF_EXTENDEDKEY, 0);
		// Simulate it being release
		keybd_event(key, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
CATCH_THROW()
}

char desktopname[MAX_COMPUTERNAME_LENGTH + 1 + 256];

void vncClientThread::Initialize()
{
TRY_CATCH

	Log.Add(_MESSAGE_,	("client id(%hd) connected"), m_client->GetClientId());
	
	// LOCK INITIAL SETUP
	// All clients have the m_protocol_ready flag set to false initially, to prevent
	// updates and suchlike interfering with the initial protocol negotiations.

	m_client->m_encodemgr.EnableQueuing(true);

	try
	{
		// AUTHENTICATE LINK
		if (!InitAuthenticate())
			throw MCStreamException("InitAuthenticate failed");
	}
	catch(CStreamException&)
	{
		m_server->RemoveClient(m_client->GetClientId());
		return;	
	}
	// Set Client Connect time
	m_client->SetConnectTime(timeGetTime());

	// INIT PIXEL FORMAT

	// Modif sf@2002 - Scaling
	{
		boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
		m_client->m_encodemgr.m_buffer->SetScale(m_server->GetDefaultScale()); // v1.1.2
	}
	m_client->m_ScaledScreen = m_client->m_encodemgr.m_buffer->GetViewerSize();
	m_client->m_nScale = m_client->m_encodemgr.m_buffer->GetScale();


	// Receive the name of this desktop
	// sf@2002 - v1.1.x - Complete the computer name with the IP address if necessary
	bool fIP = false;
	DWORD desktopnamelen = MAX_COMPUTERNAME_LENGTH + 1 + 256;
	memset((char*)desktopname, 0, sizeof(desktopname));
	if (GetComputerName(desktopname, &desktopnamelen))
	{
		// Make the name lowercase
		for (int x=0; x<strlen(desktopname); x++)
		{
			desktopname[x] = tolower(desktopname[x]);
		}
		// Check for the presence of "." in the string (then it's presumably an IP adr)
		if (strchr(desktopname, '.') != NULL) fIP = true;
	}
	else
	{
		strcpy(desktopname, "WinVNC");
	}

	// Send the server format message to the client
	rfbServerInitMsg server_ini;
	server_ini.format = m_client->m_encodemgr.m_buffer->GetLocalFormat();

	// Endian swaps
	// Modif sf@2002 - Scaling
	server_ini.framebufferWidth = Swap16IfLE(m_client->m_ScaledScreen.br.x - m_client->m_ScaledScreen.tl.x);
	server_ini.framebufferHeight = Swap16IfLE(m_client->m_ScaledScreen.br.y - m_client->m_ScaledScreen.tl.y);
	screenWidth = server_ini.framebufferWidth;
	screenHeight = server_ini.framebufferHeight;
	server_ini.format.redMax = Swap16IfLE(server_ini.format.redMax);
	server_ini.format.greenMax = Swap16IfLE(server_ini.format.greenMax);
	server_ini.format.blueMax = Swap16IfLE(server_ini.format.blueMax);

	server_ini.nameLength = Swap32IfLE(strlen(desktopname));
	try
	{
		m_stream->Send((char *)&server_ini, sizeof(server_ini));
		m_stream->Send(desktopname, strlen(desktopname));
	}
	catch(CStreamException &e)
	{
		MLog_Exception(e);
		m_server->RemoveClient(m_client->GetClientId());
		return;
	}

	// UNLOCK INITIAL SETUP
	// Initial negotiation is complete, so set the protocol ready flag
	m_client->EnableProtocol();
	
	// Add a fullscreen update to the client's update list
	// sf@2002 - Scaling
	// m_client->m_update_tracker.add_changed(m_client->m_fullscreen);
	{ // RealVNC 336
		boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
		m_client->m_update_tracker.add_changed(m_client->m_ScaledScreen);
	}


	// Clear the CapsLock and NumLock keys
	if (m_client->m_keyboardenabled)
	{
		ClearKeyState(VK_CAPITAL);
		// *** JNW - removed because people complain it's wrong
		//ClearKeyState(VK_NUMLOCK);
		ClearKeyState(VK_SCROLL);
	}
CATCH_THROW()
}

void vncClientThread::TrySessionRestart()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("Try to restart broken session"));

	m_server->GetRCHost()->NotifySessionBroke();

	m_client->DisableProtocol();

	/// Reseting encoder streams
	m_client->m_encodemgr.ResetZRLEEncoding();

	/// Firstly braking connection on viewer side
	char code = rfbRetrySession;
	for(int i=0; i< MAX_PATH; ++i) //MAX_PATH is safely more than any of VNC commands length
		m_stream->Send( &code, 1 );

	
	tstring startToken(START_COMMAND);
	/// Waiting session start
	CTokenCatcher tockenCatcher(startToken.c_str(),startToken.length());
	while(true)
	{
		m_stream->Receive(&code,1);
		if (tockenCatcher.Send(&code,1))
			break;
	}

	Log.Add(_MESSAGE_,_T("Start command from viewer received. Restarting session"));

	/// Sending start command
	m_stream->Send(startToken.c_str(),startToken.length());

	// UNLOCK INITIAL SETUP
	m_client->EnableProtocol();
	
	m_server->GetRCHost()->NotifySessionRestored();

	// Add a fullscreen update to the client's update list
	{
		boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
		m_client->m_update_tracker.add_changed(m_client->m_ScaledScreen);
	}

CATCH_THROW()
}

void vncClientThread::DoMainLoop()
{
TRY_CATCH

	bool connected = true;

	// Notify RCHost
	TRY_CATCH
		if (m_server->GetRCHost() && m_client->m_id != m_server->GetRCHost()->m_shadowStreamClientId)
			m_server->GetRCHost()->NotifySessionStarted(m_client->m_id);
	CATCH_LOG("m_RCHost->NotifySessionStarted failed")

	while (connected)
	{
		rfbClientToServerMsg msg;

		/*// Ensure that we're running in the correct desktop
		if (!vncService::InputDesktopSelected())
			vncService::SelectDesktop(NULL);*/

		// sf@2002 - v1.1.2
		int nTO = 1; // Type offset
		// If DSM Plugin, we must read all the transformed incoming rfb messages (type included)
		// Try to read a message ID
		m_stream->Receive((char *)&msg.type, sizeof(msg.type));

		// What to do is determined by the message id
		switch(msg.type)
		{
		case rfbAliveMsg:
			{
				Log.Add(_MESSAGE_,_T("Alive message received"));
				InterlockedExchange(&m_client->m_alivePending, TRUE);
				{
					/// Triggering update thread to send alive response
					boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
					m_client->TriggerUpdateThread();
				}
			}
			break;
		case rfbResetStreams:
			{
				//Reseting encoder
				m_client->DisableProtocol();
				//delete m_client->m_encodemgr;
				//m_client->m_encodemgr = new vncEncodeMgr();
				m_client->m_encodemgr.ResetZRLEEncoding();

				//Sending echo to viewer
				char code = rfbResetStreams;
				m_stream->Send(&code, 1);
				m_client->EnableProtocol();
				//Sending full screen update
				m_client->m_update_tracker.add_changed(m_client->m_ScaledScreen);
				Log.Add(_MESSAGE_,"Client setup shadow stream. Encoding restarted");
			}
			break;
		case rfbSetVisualPointer:
			{
				char state;
				m_stream->Receive(&state,1);
				m_server->GetRCHost()->SetSessionMode(m_client,VISUAL_POINTER, state!=0);
			}
			break;
		case rfbSetViewOnly:
			{
				char state;
				m_stream->Receive(&state,1);
				m_server->GetRCHost()->SetSessionMode(m_client,VIEW_ONLY, state!=0);
			}
			break;
		case rfbSetCaptureAlphaBlend:
			{
				char captureAlphaBlend;
				m_stream->Receive(&captureAlphaBlend,1);
				m_server->GetRCHost()->SetCaptureAlphaBlend(captureAlphaBlend != 0);
			}
			break;
		case rfbHideWallpaper:
			{
				char hideWallpaper;
				m_stream->Receive(&hideWallpaper,1);
				m_server->SetHideWallpaper(hideWallpaper != 0);
			}
			break;
		case rfbSendCAD:
			{
				if (!m_client->m_pointerenabled)
					break;
				if (NULL != m_server->GetSrvCommunicator())
				{
					m_server->GetSrvCommunicator()->SendMsg(SRVCOMM_SEND_CAD, NULL, 0);
				} else
				{
					Log.Add(_ERROR_,_T("Can't simulate ctrl+alt+del without service"));
				}
			}
			break;
		case rfbSetPixelFormat:
			// Read the rest of the message:
			m_stream->Receive(((char *) &msg)+nTO, sz_rfbSetPixelFormatMsg-nTO);

			// Prevent updates while the pixel format is changed
			m_client->DisableProtocol();

			// Sending duplicate to server (shadow streams issues)
			{
				InterlockedExchange(&m_client->m_pixelFormatSendPending, TRUE);
				//char code = rfbSetPixelFormatClient;
				boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
				{
					//m_stream->Send(&code,1);
					memcpy(m_client->m_pixelFormatMsgPending, ((char *) &msg)+nTO, sz_rfbSetPixelFormatMsg-1 /*1 is type offset*/);
					//m_stream->Send(((char *) &msg)+nTO, sz_rfbSetPixelFormatMsg-nTO);
				}
			}

			// Swap the relevant bits.
			msg.spf.format.redMax = Swap16IfLE(msg.spf.format.redMax);
			msg.spf.format.greenMax = Swap16IfLE(msg.spf.format.greenMax);
			msg.spf.format.blueMax = Swap16IfLE(msg.spf.format.blueMax);

			// sf@2005 - Additional param for Grey Scale transformation
			m_client->m_encodemgr.EnableGreyPalette((msg.spf.format.pad1 == 1));			
				
			// Tell the buffer object of the change			
			if (!m_client->m_encodemgr.SetClientFormat(msg.spf.format))
			{
				Log.Add(_ERROR_,_T("remote pixel format invalid. clientid(%d)"),m_client->GetClientId());
				connected = false;
			}

			// Set the palette-changed flag, just in case...
			m_client->m_palettechanged = true;

			// Re-enable updates
			m_client->EnableProtocol();
			
			break;

		case rfbSetEncodings:
			//Log.Add(_MESSAGE_,"rfbSetEncodings begin_______________________");
			// Prevent updates while the encoder is changed
			//m_client->DisableProtocol();

			// Read the rest of the message:
			m_stream->Receive(((char *) &msg)+nTO, sz_rfbSetEncodingsMsg-nTO);

			// RDV cache
			m_client->m_encodemgr.EnableCache(false);

	        // RDV XOR and client detection
			m_client->m_encodemgr.AvailableXOR(false);
			m_client->m_encodemgr.AvailableZRLE(false);
			m_client->m_encodemgr.AvailableTight(false);

			// sf@2002 - Tight
			m_client->m_encodemgr.SetQualityLevel(-1);
			m_client->m_encodemgr.SetCompressLevel(6);
			m_client->m_encodemgr.EnableLastRect(false);

			// Tight - CURSOR HANDLING
			m_client->m_encodemgr.EnableXCursor(false);
			m_client->m_encodemgr.EnableRichCursor(false);
			m_server->EnableXRichCursor(false);
			m_client->m_cursor_update_pending = false;
			m_client->m_cursor_update_sent = false;

			// Prevent updates while the encoder is changed
			m_client->DisableProtocol();

			// Read in the preferred encodings
			msg.se.nEncodings = Swap16IfLE(msg.se.nEncodings);
			{
				int x;
				bool encoding_set = false;

				// By default, don't use copyrect!
				m_client->m_update_tracker.enable_copyrect(false);

				for (x=0; x<msg.se.nEncodings; x++)
				{
					CARD32 encoding;

					// Read an encoding in
					m_stream->Receive((char *)&encoding, sizeof(encoding));

					// Is this the CopyRect encoding (a special case)?
					if (Swap32IfLE(encoding) == rfbEncodingCopyRect)
					{
						m_client->m_update_tracker.enable_copyrect(true);
						//Log.Add(_MESSAGE_,"CopyRect enabled");
						continue;
					}

					// Is this a NewFBSize encoding request?
					if (Swap32IfLE(encoding) == rfbEncodingNewFBSize) {
						m_client->m_use_NewSWSize = true;
						//Log.Add(_MESSAGE_,"New FB size");
						continue;
					}

					// CACHE RDV
					if (Swap32IfLE(encoding) == rfbEncodingCacheEnable)
					{
						m_client->m_encodemgr.EnableCache(true);
						//Log.Add(_MESSAGE_,"Cache protocol extension enabled");
						continue;
					}


					// XOR zlib
					if (Swap32IfLE(encoding) == rfbEncodingXOREnable) {
						m_client->m_encodemgr.AvailableXOR(true);
						//Log.Add(_MESSAGE_,"XOR protocol extension enabled");
						continue;
					}


					// Is this a CompressLevel encoding?
					if ((Swap32IfLE(encoding) >= rfbEncodingCompressLevel0) &&
						(Swap32IfLE(encoding) <= rfbEncodingCompressLevel9))
					{
						// Client specified encoding-specific compression level
						int level = (int)(Swap32IfLE(encoding) - rfbEncodingCompressLevel0);
						m_client->m_encodemgr.SetCompressLevel(level);
						//Log.Add(_MESSAGE_,"compression level requested: %d", level);
						continue;
					}

					// Is this a QualityLevel encoding?
					if ((Swap32IfLE(encoding) >= rfbEncodingQualityLevel0) &&
						(Swap32IfLE(encoding) <= rfbEncodingQualityLevel9))
					{
						// Client specified image quality level used for JPEG compression
						int level = (int)(Swap32IfLE(encoding) - rfbEncodingQualityLevel0);
						m_client->m_encodemgr.SetQualityLevel(level);
						//Log.Add(_MESSAGE_,"image quality level requested: %d", level);
						continue;
					}

					// Is this a LastRect encoding request?
					if (Swap32IfLE(encoding) == rfbEncodingLastRect) {
						m_client->m_encodemgr.EnableLastRect(true); // We forbid Last Rect for now 
						//Log.Add(_MESSAGE_,"LastRect protocol extension enabled");
						continue;
					}

					// Is this an XCursor encoding request?
					if (Swap32IfLE(encoding) == rfbEncodingXCursor) {
						m_client->m_encodemgr.EnableXCursor(true);
						m_server->EnableXRichCursor(true);
						//Log.Add(_MESSAGE_,"X-style cursor shape updates enabled");
						continue;
					}

					// Is this a RichCursor encoding request?
					if (Swap32IfLE(encoding) == rfbEncodingRichCursor) {
						m_client->m_encodemgr.EnableRichCursor(true);
						m_server->EnableXRichCursor(true);
						//Log.Add(_MESSAGE_,"Full-color cursor shape updates enabled");
						continue;
					}

					// Is this a PointerPos encoding request? nyama/marscha - PointerPos
					if (Swap32IfLE(encoding) == rfbEncodingPointerPos) {
						m_client->m_use_PointerPos = true;
						//Log.Add(_MESSAGE_,"PointerPos protocol extension enabled");
						continue;
					}

					// RDV - We try to detect which type of viewer tries to connect
					if (Swap32IfLE(encoding) == rfbEncodingZRLE) {
						m_client->m_encodemgr.AvailableZRLE(true);
						//Log.Add(_MESSAGE_,"ZRLE found");
						// continue;
					}

					if (Swap32IfLE(encoding) == rfbEncodingTight) {
						m_client->m_encodemgr.AvailableTight(true);
						//Log.Add(_MESSAGE_,"Tight found");
						// continue;
					}

					// Have we already found a suitable encoding?
					if (!encoding_set)
					{
						// No, so try the buffer to see if this encoding will work...
						boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
						if (m_client->m_encodemgr.SetEncoding(Swap32IfLE(encoding),false))
							encoding_set = true;
					}
				}

				// If no encoding worked then default to RAW!
				if (!encoding_set)
				{
					boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
					if (!m_client->m_encodemgr.SetEncoding(Swap32IfLE(rfbEncodingRaw),false))
					{
						Log.Add(_WARNING_,"failed to select raw encoder!");
						connected = false;
					}
				}

				// sf@2002 - For now we disable cache protocole when more than one client are connected
				// (But the cache buffer (if exists) is kept intact (for XORZlib usage))
				if (m_server->AuthClientCount() > 1)
					m_server->DisableCacheForAllClients();

			}
			// Re-enable updates
			m_client->client_settings_passed=true;
			m_client->EnableProtocol();
			break;
			
		case rfbFramebufferUpdateRequest:
			// Read the rest of the message:
			m_stream->Receive(((char *) &msg)+nTO, sz_rfbFramebufferUpdateRequestMsg-nTO);
			/*{
				m_client->Sendtimer.stop();
				int sendtime=m_client->Sendtimer.read()*1000;
				if (m_client->Totalsend>1500 && sendtime!=0) 
					{
						//vnclog.Print(LL_SOCKERR, VNCLOG("Send Size %i %i %i %i\n"),m_stream->Totalsend,sendtime,m_stream->Totalsend/sendtime,m_client->m_encodemgr.m_encoding);
						m_client->timearray[m_client->m_encodemgr.m_encoding][m_client->roundrobin_counter]=sendtime;
						m_client->sizearray[m_client->m_encodemgr.m_encoding][m_client->roundrobin_counter]=m_client->Totalsend;
						m_client->Sendtimer.reset();
						for (int j=0;j<17;j++)
						{
							int totsize=0;
							int tottime=0;
							for (int i=0;i<31;i++)
								{
								totsize+=m_client->sizearray[j][i];
								tottime+=m_client->timearray[j][i];
								}
							if (tottime!=0 && totsize>1500)
								vnclog.Print(LL_SOCKERR, VNCLOG("Send Size %i %i %i %i\n"),totsize,tottime,totsize/tottime,j);
						}
						m_client->roundrobin_counter++;
						if (m_client->roundrobin_counter>30) m_client->roundrobin_counter=0;
					}
				m_client->Sendtimer.reset();
				m_client->Totalsend=0;

			}*/

			{
				rfb::Rect update;

				// Receive the specified rectangle as the region to send updates for
				// Modif sf@2002 - Scaling.
				update.tl.x = (Swap16IfLE(msg.fur.x) + m_client->m_SWOffsetx) * m_client->m_nScale;
				update.tl.y = (Swap16IfLE(msg.fur.y) + m_client->m_SWOffsety) * m_client->m_nScale;
			//	update.tl.x = 0;
			//	update.tl.y = 0;
				update.br.x = update.tl.x + Swap16IfLE(msg.fur.w) * m_client->m_nScale;
				update.br.y = update.tl.y + Swap16IfLE(msg.fur.h) * m_client->m_nScale;
			//	update.br.x = 2880;
			//	update.br.y = 1200;
				rfb::Region2D update_rgn = update;
//				vnclog.Print(LL_SOCKERR, VNCLOG("Update asked for region %i %i %i %i %i\n"),update.tl.x,update.tl.y,update.br.x,update.br.y,m_client->m_SWOffsetx);

				// RealVNC 336
				if (update_rgn.is_empty()) 
				{
					Log.Add(_ERROR_,"Client update region is empty! Switching to fullscreen rect");
					update_rgn = m_server->GetDesktopPointer()->m_Cliprect;
				}

				{
					boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());

					// Add the requested area to the incremental update cliprect
					m_client->m_incr_rgn = m_client->m_incr_rgn.union_(update_rgn);

					// Is this request for a full update?
					if (!msg.fur.incremental)
					{
						// Yes, so add the region to the update tracker
						m_client->m_update_tracker.add_changed(update_rgn);
						
						// Tell the desktop grabber to fetch the region's latest state
						m_client->m_encodemgr.m_buffer->m_desktop->QueueRect(update);
					}
					
					/* RealVNC 336 (removed)
					// Check that this client has an update thread
					// The update thread never dissappears, and this is the only
					// thread allowed to create it, so this can be done without locking.
					if (!m_client->m_updatethread)
					{
						m_client->m_updatethread = new vncClientUpdateThread;
						connected = (m_client->m_updatethread &&
							m_client->m_updatethread->Init(m_client));
					}
					*/

					 // Kick the update thread (and create it if not there already)
					m_client->m_encodemgr.m_buffer->m_desktop->TriggerUpdate();
					m_client->TriggerUpdateThread();
				}
			}
			break;
	
			case rfbKeyEvent:
			{
				// Read the rest of the message:
				m_stream->Receive(((char *) &msg)+nTO, sz_rfbKeyEventMsg-nTO);
				if (!m_client->m_keyboardenabled) break;
				//Preventing keyb. events from protected window
				if (m_server && m_server->GetRCHost())
				{
					HWND hProtectedWnd(static_cast<HWND>(m_server->GetRCHost()->GetProtectedWindow()));
					if (hProtectedWnd && m_server->GetRCHost()->m_protectedThreads.find(GetWindowThreadProcessId(GetForegroundWindow(),0)) != m_server->GetRCHost()->m_protectedThreads.end())
					{
						break;
					}
				}
				if (m_client->m_keyboardenabled)
				{
					msg.ke.key = Swap32IfLE(msg.ke.key);
					// Receive the keymapper to do the work
					// m_client->m_keymap.DoXkeysym(msg.ke.key, msg.ke.down);
					//Log.Add(_MESSAGE_,_T("Key event code(%X) down(%d)"), msg.ke.key, msg.ke.down);
					vncKeymap::keyEvent(msg.ke.key, msg.ke.down, m_server);
					m_client->m_remoteevent = TRUE;
					if (m_server && m_server->GetRCHost())
						m_server->GetRCHost()->GetActivityMonitor().SetNewActivity(CActivityMonitor::EAT_KEYBOARD);
				}
			}
			break;
			
			case rfbPointerEvent:
			{
				// Read the rest of the message:
				m_stream->Receive(((char *) &msg)+nTO, sz_rfbPointerEventMsg-nTO);
				if ((!m_client->m_pointerenabled)
					&& 
					(!m_client->m_visualPointer)) break;
				{
					POINT cursorPos; GetCursorPos(&cursorPos);
					// Convert the coords to Big Endian
					// Modif sf@2002 - Scaling
					msg.pe.x = (Swap16IfLE(msg.pe.x) + m_client->m_SWOffsetx+m_client->m_ScreenOffsetx) * m_client->m_nScale;
					msg.pe.y = (Swap16IfLE(msg.pe.y) + m_client->m_SWOffsety+m_client->m_ScreenOffsety) * m_client->m_nScale;
					// Work out the flags for this event
					DWORD flags = MOUSEEVENTF_ABSOLUTE;
					if (msg.pe.x != m_client->m_ptrevent.x ||
						msg.pe.y != m_client->m_ptrevent.y)
						flags |= MOUSEEVENTF_MOVE;
					if ( (msg.pe.buttonMask & rfbButton1Mask) != 
						(m_client->m_ptrevent.buttonMask & rfbButton1Mask) )
					{
						if (GetSystemMetrics(SM_SWAPBUTTON))
						flags |= (msg.pe.buttonMask & rfbButton1Mask) 
							? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
						else
						flags |= (msg.pe.buttonMask & rfbButton1Mask) 
							? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
					}
					if ( (msg.pe.buttonMask & rfbButton2Mask) != 
						(m_client->m_ptrevent.buttonMask & rfbButton2Mask) )
					{
						flags |= (msg.pe.buttonMask & rfbButton2Mask) 
							? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
					}
					if ( (msg.pe.buttonMask & rfbButton3Mask) != 
						(m_client->m_ptrevent.buttonMask & rfbButton3Mask) )
					{
						if (GetSystemMetrics(SM_SWAPBUTTON))
						flags |= (msg.pe.buttonMask & rfbButton3Mask) 
							? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
						else
						flags |= (msg.pe.buttonMask & rfbButton3Mask) 
							? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
					}
					// Treat buttons 4 and 5 presses as mouse wheel events
					DWORD wheel_movement = 0;
					if (m_client->m_encodemgr.IsMouseWheelTight())
					{
						if ((msg.pe.buttonMask & rfbButton4Mask) != 0 &&
							(m_client->m_ptrevent.buttonMask & rfbButton4Mask) == 0)
						{
							flags |= MOUSEEVENTF_WHEEL;
							wheel_movement = (DWORD)+120;
						}
						else if ((msg.pe.buttonMask & rfbButton5Mask) != 0 &&
									(m_client->m_ptrevent.buttonMask & rfbButton5Mask) == 0)
						{
							flags |= MOUSEEVENTF_WHEEL;
							wheel_movement = (DWORD)-120;
						}
					}
					else
					{
						// RealVNC 335 Mouse wheel support
						if (msg.pe.buttonMask & rfbWheelUpMask) 
						{
							flags |= MOUSEEVENTF_WHEEL;
							wheel_movement = WHEEL_DELTA;
						}
						if (msg.pe.buttonMask & rfbWheelDownMask) 
						{
							flags |= MOUSEEVENTF_WHEEL;
							wheel_movement = -WHEEL_DELTA;
						}
					}
					// Generate coordinate values
					// bug fix John Latino
					// offset for multi display
					int screenX, screenY, screenDepth;
					m_server->GetScreenInfo(screenX, screenY, screenDepth);
					if ((!m_client->m_pointerenabled)
						&& 
						m_client->m_visualPointer)
					{
						unsigned int x(0);
						unsigned int y(0);
						//Using visual pointer
						if (m_client->m_display_type==1)
						{
							//primary display always have (0,0) as corner
							x = msg.pe.x;//(msg.pe.x *  65535) / (screenX-1);
							y = msg.pe.y;//(msg.pe.y * 65535) / (screenY-1);
						} else
						{
							///TODO: move for case of second or spanned
							x = msg.pe.x;
							y = msg.pe.y;
						}
						TRY_CATCH
							if (flags & MOUSEEVENTF_LEFTDOWN)
							{
								VISUAL_POINTER_INSTANCE.SetState(CVisualPointer::LBTN_DOWN);
							} else
							if (flags & MOUSEEVENTF_RIGHTDOWN)
							{
								VISUAL_POINTER_INSTANCE.SetState(CVisualPointer::RBTN_DOWN);
							} else
							if (flags & MOUSEEVENTF_RIGHTUP || flags & MOUSEEVENTF_LEFTUP)
							{
								VISUAL_POINTER_INSTANCE.SetState(CVisualPointer::NORMAL);
							}
							VISUAL_POINTER_INSTANCE.MoveTo(x,y);
						CATCH_LOG("Failed to move visual pointer")
					} else
						if (m_client->m_display_type==1)
						{//primary display always have (0,0) as corner
							unsigned long x = (msg.pe.x *  65535) / (screenX-1);
							unsigned long y = (msg.pe.y * 65535) / (screenY-1);
							// Do the pointer event
							if ((m_server->IsInputProxyNeeded() || m_server->GetDesktopPointer()->m_lockFix->Active()) && NULL != m_server->GetSrvCommunicator())
							{
								SSendMouseEventMsg mouseMsg;
								mouseMsg.flags = flags;
								mouseMsg.x = x;
								mouseMsg.y = y;
								mouseMsg.wheelMovement = wheel_movement;
								mouseMsg.extraInfo = MESS_MARKER;
								mouseMsg.toWinlogon = m_server->GetDesktopPointer()->m_lockFix->Active();
								mouseMsg.useSendInput = false;
								if (FALSE == m_server->GetSrvCommunicator()->SendMsg(SRVCOMM_SEND_MOUSE_INPUT, reinterpret_cast<char*>(&mouseMsg), sizeof(mouseMsg)))
								{
									try
									{
										m_server->GetSrvCommunicator()->ReBind(); /// Try to rebind
									}
									catch(CExceptionBase&)
									{
										/// Don't logging to prevent flood messging
									}
								}
							} else
							{
								::mouse_event(flags, (DWORD) x, (DWORD) y, wheel_movement, MESS_MARKER);
							}
						}
						else
						{//second or spanned
							if ((m_server->IsInputProxyNeeded() || m_server->GetDesktopPointer()->m_lockFix->Active()) && NULL != m_server->GetSrvCommunicator())
							{
								SSendMouseEventMsg mouseMsg;
								INPUT evt;
								evt.type = INPUT_MOUSE;
								msg.pe.x=msg.pe.x-GetSystemMetrics(SM_XVIRTUALSCREEN);
								msg.pe.y=msg.pe.y-GetSystemMetrics(SM_YVIRTUALSCREEN);
								evt.mi.dx = (msg.pe.x * 65535) / (GetSystemMetrics(SM_CXVIRTUALSCREEN)-1);
								evt.mi.dy = (msg.pe.y* 65535) / (GetSystemMetrics(SM_CYVIRTUALSCREEN)-1);
								evt.mi.dwFlags = flags | MOUSEEVENTF_VIRTUALDESK;
								evt.mi.dwExtraInfo = MESS_MARKER;
								evt.mi.mouseData = wheel_movement;
								evt.mi.time = 0;
								mouseMsg.evt = evt;
								mouseMsg.toWinlogon = m_server->GetDesktopPointer()->m_lockFix->Active();
								mouseMsg.useSendInput = true;
								if (FALSE == m_server->GetSrvCommunicator()->SendMsg(SRVCOMM_SEND_MOUSE_INPUT, reinterpret_cast<char*>(&mouseMsg), sizeof(mouseMsg)))
								{
									try
									{
										m_server->GetSrvCommunicator()->ReBind(); /// Try to rebind
									}
									catch(CExceptionBase&)
									{
										/// Don't logging to prevent flood messging
									}
								}

							} else
							if (m_client->Sendinput)
							{							
								INPUT evt;
								evt.type = INPUT_MOUSE;
								msg.pe.x=msg.pe.x-GetSystemMetrics(SM_XVIRTUALSCREEN);
								msg.pe.y=msg.pe.y-GetSystemMetrics(SM_YVIRTUALSCREEN);
								evt.mi.dx = (msg.pe.x * 65535) / (GetSystemMetrics(SM_CXVIRTUALSCREEN)-1);
								evt.mi.dy = (msg.pe.y* 65535) / (GetSystemMetrics(SM_CYVIRTUALSCREEN)-1);
								evt.mi.dwFlags = flags | MOUSEEVENTF_VIRTUALDESK;
								evt.mi.dwExtraInfo = MESS_MARKER;
								evt.mi.mouseData = wheel_movement;
								evt.mi.time = 0;
								m_client->Sendinput(1, &evt, sizeof(evt));
							}
							else
							{
								//POINT cursorPos; GetCursorPos(&cursorPos);
								ULONG oldSpeed, newSpeed = 10;
								ULONG mouseInfo[3];
								if (flags & MOUSEEVENTF_MOVE) 
									{
										flags &= ~MOUSEEVENTF_ABSOLUTE;
										SystemParametersInfo(SPI_GETMOUSE, 0, &mouseInfo, 0);
										SystemParametersInfo(SPI_GETMOUSESPEED, 0, &oldSpeed, 0);
										ULONG idealMouseInfo[] = {10, 0, 0};
										SystemParametersInfo(SPI_SETMOUSESPEED, 0, &newSpeed, 0);
										SystemParametersInfo(SPI_SETMOUSE, 0, &idealMouseInfo, 0);
									}
								::mouse_event(flags, msg.pe.x-cursorPos.x, msg.pe.y-cursorPos.y, wheel_movement, MESS_MARKER);
								if (flags & MOUSEEVENTF_MOVE) 
									{
										SystemParametersInfo(SPI_SETMOUSE, 0, &mouseInfo, 0);
										SystemParametersInfo(SPI_SETMOUSESPEED, 0, &oldSpeed, 0);
									}
							}
						}
					// Save the old position
					m_client->m_ptrevent = msg.pe;

					// Flag that a remote event occurred
					m_client->m_remoteevent = TRUE;

					// Tell the desktop hook system to grab the screen...
					m_client->m_encodemgr.m_buffer->m_desktop->TriggerUpdate();

					if ((msg.pe.x-cursorPos.x || msg.pe.y-cursorPos.y) &&
						m_server && m_server->GetRCHost())
						m_server->GetRCHost()->GetActivityMonitor().SetNewActivity(CActivityMonitor::EAT_MOUSE);
				}	
			}			
			break;
		case rfbClientCutText:
			// Read the rest of the message:
			m_stream->Receive(((char *) &msg)+nTO, sz_rfbClientCutTextMsg-nTO);
			{
				// Allocate storage for the text
				const UINT length = Swap32IfLE(msg.cct.length);
				char *text = new char [length+1];
				if (text == NULL)
					break;
				text[length] = 0;

				// Read in the text
				try
				{
					m_stream->Receive(text, length);
				}
				catch(CStreamException &e)
				{
					delete [] text;
					throw e;
				}

				// Receive the server to update the local clipboard
				m_server->UpdateLocalClipText(text);

				// Free the clip text we read
				delete [] text;
			}
			break;


		// Modif sf@2002 - Scaling
		// Server Scaling Message received
		case rfbPalmVNCSetScaleFactor:
			m_client->m_fPalmVNCScaling = true;
		case rfbSetScale: // Specific PalmVNC SetScaleFactor
			{
			// m_client->m_fPalmVNCScaling = false;
			// Read the rest of the message 
			if (m_client->m_fPalmVNCScaling)
			{
				m_stream->Receive(((char *) &msg) + nTO, sz_rfbPalmVNCSetScaleFactorMsg - nTO);
			}
			else
			{
				m_stream->Receive(((char *) &msg) + nTO, sz_rfbSetScaleMsg - nTO);
			}

			// Only accept reasonable scales...
			if (msg.ssc.scale < 1 || msg.ssc.scale > 9) break;

			m_client->m_nScale = msg.ssc.scale;
			{
				boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
				if (!m_client->m_encodemgr.m_buffer->SetScale(msg.ssc.scale))
					{
						connected = false;
						break;
					}
	
				m_client->fNewScale = true;
				InvalidateRect(NULL,NULL,true);
				m_client->TriggerUpdateThread();
			}

			}
			break;


		// Set Server Input
		case rfbSetServerInput:
			m_stream->Receive(((char *) &msg) + nTO, sz_rfbSetServerInputMsg - nTO);
			if (m_client->m_keyboardenabled)
				{
					if (msg.sim.status==1) m_client->m_encodemgr.m_buffer->m_desktop->SetDisableInput(true);
					if (msg.sim.status==0) m_client->m_encodemgr.m_buffer->m_desktop->SetDisableInput(false);
				}
			break;


		// Set Single Window
		case rfbSetSW:
			m_stream->Receive(((char *) &msg) + nTO, sz_rfbSetSWMsg - nTO);
			if (Swap16IfLE(msg.sw.x)<5 && Swap16IfLE(msg.sw.y)<5) 
			{
				m_client->m_encodemgr.m_buffer->m_desktop->SetSW(1,1);
				break;
			}
			m_client->m_encodemgr.m_buffer->m_desktop->SetSW(
				(Swap16IfLE(msg.sw.x) + m_client->m_SWOffsetx+m_client->m_ScreenOffsetx) * m_client->m_nScale,
				(Swap16IfLE(msg.sw.y) + m_client->m_SWOffsety+m_client->m_ScreenOffsety) * m_client->m_nScale);
			break;
#ifdef DSHOW
		case rfbKeyFrameRequest:
			{
				MutexAutoLock l_Lock(&m_client->m_hmtxEncodeAccess);

				boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());

				if(m_client->m_encodemgr.ResetZRLEEncoding())
				{
					rfb::Rect update;

					rfb::Rect ViewerSize = m_client->m_encodemgr.m_buffer->GetViewerSize();

					update.tl.x = 0;
					update.tl.y = 0;
					update.br.x = ViewerSize.br.x;
					update.br.y = ViewerSize.br.y;
					rfb::Region2D update_rgn = update;

					// Add the requested area to the incremental update cliprect
					m_client->m_incr_rgn = m_client->m_incr_rgn.union_(update_rgn);

					// Yes, so add the region to the update tracker
					m_client->m_update_tracker.add_changed(update_rgn);
					
					// Tell the desktop grabber to fetch the region's latest state
					m_client->m_encodemgr.m_buffer->m_desktop->QueueRect(update);

					// Kick the update thread (and create it if not there already)
					m_client->TriggerUpdateThread();

					// Send a message back to the client to confirm that we have reset the zrle encoding			
					rfbKeyFrameUpdateMsg header;
					header.type = rfbKeyFrameUpdate;
					m_client->SendRFBMsg(rfbKeyFrameUpdate, (BYTE *)&header, sz_rfbKeyFrameUpdateMsg);
				}				
				//else
				//{
				//	  vnclog.Print(LL_INTERR, VNCLOG("[rfbKeyFrameRequest] Unable to Reset ZRLE Encoding\n"));
				//}
			}
			break;
#endif
		case rfbStopSession:
			///Stop session request
			connected = false;
			m_client->SetStopReason(REMOTE_STOP);
			{
				char code(rfbStopSession);
				m_stream->Send(&code,1);
			}
			break;
		case rfbRetrySession:
			///Try restart connection
			Log.Add(_ERROR_,_T("Message 'Try session restart' received"));
			TrySessionRestart();
			break;
		default:
			///Try restart connection
			Log.Add(_ERROR_,_T("Unknown message received"));
			TrySessionRestart();
			break;
		}
	}
CATCH_THROW()
}

void vncClientThread::run()
{
SET_THREAD_LS;
TRY_CATCH

	// All this thread does is go into a stream-receive loop,
	// waiting for stuff on the given stream

	// IMPORTANT : ALWAYS call RemoveClient on the server before quitting
	// this thread.

	// Save the handle to the thread's original desktop
	HDESK home_desktop = GetThreadDesktop(GetCurrentThreadId());

	TRY_CATCH

		Initialize();

		// MAIN LOOP
		// Set the input thread to a high priority
		if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL))//THREAD_PRIORITY_TIME_CRITICAL))
			Log.WinError(_WARNING_,_T("Failed to increase priority of client thread"));


		DoMainLoop();

	CATCH_LOG("vncClientThread::run Initialize\\DoMainLoop")

	// Move into the thread's original desktop
	vncService::SelectHDESK(home_desktop);

	// Quit this thread.  This will automatically delete the thread and the
	// associated client.
	Log.Add(_MESSAGE_,	"client id(%hd) disconnected", m_client->GetClientId());


	// Disable the protocol to ensure that the update thread
	// is not accessing the desktop and buffer objects
	m_client->DisableProtocol();

	// Finally, it's safe to kill the update thread here
	if (m_client->m_updatethread.get()) 
	{
		m_client->m_updatethread->Kill();
		m_client->m_updatethread->join();
	}
	// Remove the client from the server
	// This may result in the desktop and buffer being destroyed
	// It also guarantees that the client will not be passed further
	// updates
	m_server->RemoveClient(m_client->GetClientId());

CATCH_LOG()
}

// The vncClient itself

vncClient::vncClient() :
	m_pointerenabled(true), 
	m_visualPointer(false),
	m_stream(reinterpret_cast<CAbstractStream*>(NULL)),
	m_stopReason(STREAM_ERROR), //STREAM_ERROR is default stop reason
	m_modeSendPending(FALSE),
	m_pixelFormatSendPending(TRUE),
	m_alivePending(FALSE)
{
TRY_CATCH

	m_client_name = 0;

	// Initialise mouse fields
	m_mousemoved = false;
	m_ptrevent.buttonMask = 0;
	m_ptrevent.x = 0;
	m_ptrevent.y=0;

	// Other misc flags
	m_thread.reset();
	m_palettechanged = false;

	// Initialise the two update stores
	m_updatethread.reset();
	m_update_tracker.init(this);

	m_remoteevent = false;

	m_clipboard_text = 0;

	// IMPORTANT: Initially, client is not protocol-ready.
	m_disable_protocol = 1;

	//SINGLE WINDOW
	m_use_NewSWSize = false;
	m_SWOffsetx=0;
	m_SWOffsety=0;
	m_ScreenOffsetx=0;
	m_ScreenOffsety=0;

	// sf@2002 
	fNewScale = false;
	m_fPalmVNCScaling = false;
	fFTRequest = false;

	// sf@2004 - Delta Transfer
	m_lpCSBuffer = NULL;
	m_nCSOffset = 0;
	m_nCSBufferSize = 0;

	// CURSOR HANDLING
	m_cursor_update_pending = false;
	m_cursor_update_sent = false;
	// nyama/marscha - PointerPos
	m_use_PointerPos = false;
	m_cursor_pos_changed = false;
	m_cursor_pos.x = 0;
	m_cursor_pos.y = 0;

	//cachestats
	totalraw=0;

	m_pRawCacheZipBuf = NULL;
	m_nRawCacheZipBufSize = 0;
	m_pCacheZipBuf = NULL;
	m_nCacheZipBufSize = 0;


	// Modif sf@2002 - Text Chat
	m_fUltraViewer = true;
	m_IsLoopback=false;
	m_NewSWUpdateWaiting=false;
	client_settings_passed=false;
/*	roundrobin_counter=0;
	for (int i=0;i<rfbEncodingZRLE+1;i++)
		for (int j=0;j<31;j++)
		{
		  timearray[i][j]=0;
		  sizearray[i][j]=0;
		}*/
	Sendinput=0;
	Sendinput=(pSendinput) GetProcAddress(LoadLibrary("user32.dll"),"SendInput");
// Modif cs@2005
#ifdef DSHOW
	m_hmtxEncodeAccess = CreateMutex(NULL, false, NULL);
#endif
CATCH_THROW()
}

vncClient::~vncClient()
{
TRY_CATCH

	// We now know the thread is dead, so we can clean up
	if (m_client_name != 0) {
		free(m_client_name);
		m_client_name = 0;
	}
	if (m_pRawCacheZipBuf != NULL)
		{
			delete [] m_pRawCacheZipBuf;
			m_pRawCacheZipBuf = NULL;
		}
	if (m_pCacheZipBuf != NULL)
		{
			delete [] m_pCacheZipBuf;
			m_pCacheZipBuf = NULL;
		}
	if (m_clipboard_text) {
		free(m_clipboard_text);
	}
// Modif cs@2005
#ifdef DSHOW
	CloseHandle(m_hmtxEncodeAccess);
#endif
    
CATCH_LOG()
}

// Init
bool vncClient::Init(	vncServer *server,
						boost::shared_ptr<CAbstractStream> stream,
						bool shared,
						vncClientId newid )
{
TRY_CATCH

	// Save the server id;
	m_server = server;

	// Save the socket
	m_stream = stream;

	m_client_name = strdup("<unknown>");

	// Save the client id
	m_id = newid;

	// Spawn the child thread here
	m_thread.reset(new vncClientThread(this, m_server, m_stream, 0, shared));
	if (m_thread.get() == NULL)
	{
		Log.WinError(_ERROR_,_T("Failed to create vncClientThreadObject"));
		return false;
	}
	return true;

CATCH_THROW()
}

void vncClient::Kill()
{
TRY_CATCH

	// Close the socket
	if (m_stream != NULL)
		m_stream->CancelReceiveOperation();

CATCH_THROW()
}

// Client manipulation functions for use by the server
void vncClient::SetBuffer(vncBuffer *buffer)
{
TRY_CATCH

	// Until authenticated, the client object has no access
	// to the screen buffer.  This means that there only need
	// be a buffer when there's at least one authenticated client.
	m_encodemgr.SetBuffer(buffer);

CATCH_THROW()
}

void vncClient::TriggerUpdateThread()
{
TRY_CATCH

	// ALWAYS lock the client UpdateLock before calling this!
	// RealVNC 336	
	// Check that this client has an update thread
	// The update thread never dissappears, and this is the only
	// thread allowed to create it, so this can be done without locking.
	
	if (m_updatethread.get() == NULL)
	{
		m_updatethread.reset(new vncClientUpdateThread);
		if (m_updatethread.get() == NULL || 
			!m_updatethread->Init(this)) 
		{
			Kill();
		}
	}				
	if (m_updatethread.get() != NULL)
		m_updatethread->Trigger();

CATCH_THROW()
}

void vncClient::UpdateMouse()
{
TRY_CATCH

	//boost::recursive_mutex::scoped_lock l(GetUpdateLock());
	if (!m_mousemoved && !m_cursor_update_sent)
	{
		boost::recursive_mutex::scoped_lock l(GetUpdateLock());
		m_mousemoved=true;
	}
	// nyama/marscha - PointerPos
	if (m_use_PointerPos && !m_cursor_pos_changed) {
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		//vnclog.Print(LL_INTINFO, VNCLOG("UpdateMouse m_cursor_pos(%d, %d), new(%d, %d)\n"), 
		//  m_cursor_pos.x, m_cursor_pos.y, cursorPos.x, cursorPos.y);
		if (cursorPos.x != m_cursor_pos.x || cursorPos.y != m_cursor_pos.y) {
			// This movement isn't by this client, but generated locally or by other client.
			// Send it to this client.
			boost::recursive_mutex::scoped_lock l(GetUpdateLock());
			m_cursor_pos.x = cursorPos.x;
			m_cursor_pos.y = cursorPos.y;
			m_cursor_pos_changed = true;
			TriggerUpdateThread();
		}
	}

CATCH_THROW()
}

void vncClient::UpdateClipText(const char* text)
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(GetUpdateLock());
	if (m_clipboard_text) {
		free(m_clipboard_text);
		m_clipboard_text = 0;
	}
	m_clipboard_text = strdup(text);
	TriggerUpdateThread();

CATCH_THROW()
}

void vncClient::UpdateCursorShape()
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(GetUpdateLock());
	m_cursor_update_pending = true;
	TriggerUpdateThread();

CATCH_THROW()
}

void vncClient::UpdatePalette()
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(GetUpdateLock());
	m_palettechanged = true;

CATCH_THROW()
}

void vncClient::UpdateLocalFormat()
{
TRY_CATCH

	DisableProtocol();
	m_encodemgr.SetServerFormat();
	EnableProtocol();

CATCH_THROW()
}

bool vncClient::SetNewSWSize(long w,long h,bool Desktop)
{
TRY_CATCH

	if (!m_use_NewSWSize) return false;
	DisableProtocol();
	//Log.Add(_MESSAGE_,"SetNewSWSize old(%d,%d) new(%d,%d)",NewsizeW,NewsizeH,w,h);
	m_encodemgr.SetServerFormat();
	m_palettechanged = true;
	// no lock needed Called from desktopthread
	if (Desktop) m_encodemgr.SetEncoding(0,true);//0=dummy
//	m_fullscreen = m_encodemgr.GetSize();
	m_NewSWUpdateWaiting=true;
	NewsizeW=w;
	NewsizeH=h;
	EnableProtocol();
//	TriggerUpdateThread();
	return true;

CATCH_THROW()
}

// Functions used to set and retrieve the client settings
const char* vncClient::GetClientName()
{
	return m_client_name;
}

// Enabling and disabling clipboard/GFX updates
void vncClient::DisableProtocol()
{
TRY_CATCH

	bool disable = false;
	{	
		if (!m_encodemgr.HasUpdateLock()) 
		{
			Log.Add(_ERROR_,_T("No update lock while disabling protocol. This can cause serious threading problems"));
			return;
		}
		boost::recursive_mutex::scoped_lock l(GetUpdateLock());
		if (m_disable_protocol == 0)
			disable = true;
		m_disable_protocol++;
		if (disable && (m_updatethread.get() != NULL))
			m_updatethread->EnableUpdates(false,l);
	}

CATCH_THROW()
}

void vncClient::EnableProtocol()
{
TRY_CATCH

	if (!m_encodemgr.HasUpdateLock()) 
	{
		Log.Add(_ERROR_,_T("No update lock while enabling protocol. This can cause serious threading problems"));
		return;
	}
	{	boost::recursive_mutex::scoped_lock l(GetUpdateLock());
		if (m_disable_protocol == 0) 
		{
			//vnclog.Print(LL_INTERR, VNCLOG("protocol enabled too many times!\n"));
			m_stream->CancelReceiveOperation();
			return;
		}
		m_disable_protocol--;
		if ((m_disable_protocol == 0) && (m_updatethread.get() != NULL))
			m_updatethread->EnableUpdates(true,l);
	}

CATCH_THROW()
}

// Internal methods
bool vncClient::SendRFBMsg(CARD8 type, BYTE *buffer, int buflen)
{
TRY_CATCH

	// Set the message type
	((rfbServerToClientMsg *)buffer)->type = type;

	// Send the message
	try
	{
		m_stream->Send((char *) buffer, buflen );
	}
	catch(CStreamException& e)
	{
		MLog_Exception(e);
		Kill();
		return false;
	}
	return true;

CATCH_THROW()
}


bool vncClient::SendUpdate(rfb::SimpleUpdateTracker &update)
{
TRY_CATCH

	/// Sending new session mode to client if necessary
	if (InterlockedCompareExchange(&m_modeSendPending,FALSE,TRUE))
	{
		char buf[2];
		buf[0] = rfbSetViewOnly;
		buf[1] = static_cast<char>(!PointerEnabled());
		m_stream->Send(buf, 2);
		Log.Add(_MESSAGE_,_T("Setting view only (%d)"),buf[1]);
		buf[0] = rfbSetVisualPointer;
		buf[1] = static_cast<char>(VisualPointerEnabled());
		m_stream->Send(buf, 2);
		Log.Add(_MESSAGE_,_T("Setting visual pointer (%d)"),buf[1]);
	}

	// If there is nothing to send then exit
	if (update.is_empty() && !m_cursor_update_pending && !m_NewSWUpdateWaiting && !m_cursor_pos_changed) return false;
	
	// Receive the update info from the tracker
	rfb::UpdateInfo update_info;
	update.get_update(update_info);
	update.clear();
	//Old update could be outsite the new bounding
	//We first make sure the new size is send to the client
	//The cleint ask a full update after screen_size change
	if (m_NewSWUpdateWaiting) 
	{
		m_NewSWUpdateWaiting=false;
		rfbFramebufferUpdateRectHeader hdr;
		if (m_use_NewSWSize) 
		{
			hdr.r.x = 0;
			hdr.r.y = 0;
			hdr.r.w = Swap16IfLE(NewsizeW);
			hdr.r.h = Swap16IfLE(NewsizeH);
			hdr.encoding = Swap32IfLE(rfbEncodingNewFBSize);
			rfbFramebufferUpdateMsg header;
			header.nRects = Swap16IfLE(1);
			SendRFBMsg(rfbFramebufferUpdate, (BYTE *)&header,sz_rfbFramebufferUpdateMsg);
			m_stream->Send((char *)&hdr, sizeof(hdr));
//			m_NewSWUpdateWaiting=false;
			return true;
		}
	}

	// Find out how many rectangles in total will be updated
	// This includes copyrects and changed rectangles split
	// up by codings such as CoRRE.
	
	boost::recursive_mutex::scoped_lock l(GetUpdateLock());

	/// Sending pending pixel format request
	if (InterlockedCompareExchange(&m_pixelFormatSendPending,FALSE,TRUE))
	{
		/// This context is protected by UpdateLock and DisabledProtocol
		char code = rfbSetPixelFormatClient;
		m_stream->Send(&code, 1);
		m_stream->Send(m_pixelFormatMsgPending, sz_rfbSetPixelFormatMsg-1 /*1 is type offset*/);
		Log.Add(_MESSAGE_,_T("Echo rfbSetPixelFormatClient to client send"));
	}


	int updates = 0;
	int numsubrects = 0;
	updates += update_info.copied.size();
	if (m_encodemgr.IsCacheEnabled())
	{
		if (update_info.cached.size() > 5)
		{
			updates++;
		}
		else 
		{
			updates += update_info.cached.size();
			//vnclog.Print(LL_INTERR, "cached %d\n", updates);
		}
	}
	
	rfb::RectVector::const_iterator i;
	if (updates!= 0xFFFF)
	{
		for ( i=update_info.changed.begin(); i != update_info.changed.end(); i++)
		{
			// Tight specific (lastrect)
			numsubrects = m_encodemgr.GetNumCodedRects(*i);
			
			// Skip rest rectangles if an encoder will use LastRect extension.
			if (numsubrects == 0) {
				updates = 0xFFFF;
				break;
			}
			updates += numsubrects;
			//vnclog.Print(LL_INTERR, "changed %d\n", updates);
		}
	}	
	
	// if no cache is supported by the other viewer
	// We need to send the cache as a normal update
	if (!m_encodemgr.IsCacheEnabled() && updates!= 0xFFFF) 
	{
		for (i=update_info.cached.begin(); i != update_info.cached.end(); i++)
		{
			// Tight specific (lastrect)
			numsubrects = m_encodemgr.GetNumCodedRects(*i);
			
			// Skip rest rectangles if an encoder will use LastRect extension.
			if (numsubrects == 0) {
				updates = 0xFFFF;
				break;
			}	
			updates += numsubrects;
			//vnclog.Print(LL_INTERR, "cached2 %d\n", updates);
		}
	}
	
	// 
	if (!m_encodemgr.IsXCursorSupported()) m_cursor_update_pending=false;
	// Tight specific (lastrect)
	if (updates != 0xFFFF)
	{
		// Tight - CURSOR HANDLING
		if (m_cursor_update_pending)
		{
			updates++;
		}
		// nyama/marscha - PointerPos
		if (m_cursor_pos_changed)
			updates++;
		if (updates == 0) 
			return false;
	}

//	Sendtimer.start();

	///boost::recursive_mutex::scoped_lock l(GetUpdateLock());
	// Otherwise, send <number of rectangles> header
	rfbFramebufferUpdateMsg header;
	header.nRects = Swap16IfLE(updates);
	if (!SendRFBMsg(rfbFramebufferUpdate, (BYTE *) &header, sz_rfbFramebufferUpdateMsg))
		return true;
	
	// CURSOR HANDLING
	if (m_cursor_update_pending)
	{
		if (!SendCursorShapeUpdate())
			return false;

		/*if (1 == updates)
		{
			Log.Add(_MESSAGE_,_T("No updates, excepting cursor shape, exiting...."));
			return true;
		}*/
	}

	// nyama/marscha - PointerPos
	if (m_cursor_pos_changed)
		if (!SendCursorPosUpdate())
			return false;
	
	// Send the copyrect rectangles
	if (!update_info.copied.empty()) 
	{
		rfb::Point to_src_delta = update_info.copy_delta.negate();
		for (i=update_info.copied.begin(); i!=update_info.copied.end(); i++) 
		{
			rfb::Point src = (*i).tl.translate(to_src_delta);
			if (!SendCopyRect(*i, src))
				return false;
		}
	}
	
	if (m_encodemgr.IsCacheEnabled())
	{
		if (update_info.cached.size() > 5)
		{
			if (!SendCacheZip(update_info.cached))
				return false;
		}
		else
		{
			if (!SendCacheRectangles(update_info.cached))
				return false;
		}
	}
	else 
	{
		if (!SendRectangles(update_info.cached))
			return false;
	}
	
	if (!SendRectangles(update_info.changed))
		return false;

	// Tight specific - Send LastRect marker if needed.
	if (updates == 0xFFFF)
	{
		m_encodemgr.LastRect(m_stream);
		if (!SendLastRect())
			return false;
	}
	m_stream->FlushQueue(); //TODO is that right????
	return true;

CATCH_THROW()
}

// Send a set of rectangles
bool vncClient::SendRectangles(const rfb::RectVector &rects)
{
TRY_CATCH

	// Modif cs@2005
#ifdef DSHOW
	MutexAutoLock l_Lock(&m_hmtxEncodeAccess);
#endif
//	rfb::Rect rect;
	rfb::RectVector::const_iterator i;

	// Work through the list of rectangles, sending each one
	for (i=rects.begin();i!=rects.end();++i) {
		if (!SendRectangle(*i))
			return false;
	}

	return true;

CATCH_THROW()
}

// Tell the encoder to send a single rectangle
bool vncClient::SendRectangle(const rfb::Rect &rect)
{
TRY_CATCH

	// Receive the buffer to encode the rectangle
	// Modif sf@2002 - Scaling
	rfb::Rect ScaledRect;
	ScaledRect.tl.y = rect.tl.y / m_nScale;
	ScaledRect.br.y = rect.br.y / m_nScale;
	ScaledRect.tl.x = rect.tl.x / m_nScale;
	ScaledRect.br.x = rect.br.x / m_nScale;

	ScaledRect = ScaledRect.intersect(m_server->GetDesktopPointer()->m_Cliprect); /// Fix of STL-184
	if (ScaledRect.is_empty())
	{
		Log.Add(_WARNING_,_T("Empty rectangle detected. Switching to fullscreen rect"));
		ScaledRect = m_server->GetDesktopPointer()->m_Cliprect;
	}

	//	Totalsend+=(ScaledRect.br.x-ScaledRect.tl.x)*(ScaledRect.br.y-ScaledRect.tl.y);

	// sf@2002 - DSMPlugin
	// Some encoders (Hextile, ZRLE, Raw..) store all the data to send into 
	// m_clientbuffer and return the total size from EncodeRect()
	// Some Encoders (Tight, Zlib, ZlibHex..) send data on the fly and return
	// a partial size from EncodeRect(). 
	// On the viewer side, the data is read piece by piece or in one shot 
	// still depending on the encoding...
	// It is not compatible with DSM: we need to read/write data blocks of same 
	// size on both sides in one shot
	// We create a common method to send the data 
	UINT bytes = m_encodemgr.EncodeRect(ScaledRect, m_stream);

	// Send the encoded data
	m_stream->Send2Queue((char *)(m_encodemgr.GetClientBuffer()), bytes);
	return true;

CATCH_THROW()
}

// Send a single CopyRect message
bool vncClient::SendCopyRect(const rfb::Rect &dest, const rfb::Point &source)
{
TRY_CATCH

	// Create the message header
	// Modif sf@2002 - Scaling
	rfbFramebufferUpdateRectHeader copyrecthdr;
	copyrecthdr.r.x = Swap16IfLE((dest.tl.x - m_SWOffsetx) / m_nScale );
	copyrecthdr.r.y = Swap16IfLE((dest.tl.y - m_SWOffsety) / m_nScale);
	copyrecthdr.r.w = Swap16IfLE((dest.br.x - dest.tl.x) / m_nScale);
	copyrecthdr.r.h = Swap16IfLE((dest.br.y - dest.tl.y) / m_nScale);
	copyrecthdr.encoding = Swap32IfLE(rfbEncodingCopyRect);

	// Create the CopyRect-specific section
	rfbCopyRect copyrectbody;
	copyrectbody.srcX = Swap16IfLE((source.x- m_SWOffsetx) / m_nScale);
	copyrectbody.srcY = Swap16IfLE((source.y- m_SWOffsety) / m_nScale);

	// Now send the message;
	m_stream->Send2Queue((char *)&copyrecthdr, sizeof(copyrecthdr));
	m_stream->Send2Queue((char *)&copyrectbody, sizeof(copyrectbody));

	return true;

CATCH_THROW()
}

// Send the encoder-generated palette to the client
// This function only returns false if the SendExact fails - any other
// error is coped with internally...
bool vncClient::SendPalette()
{
TRY_CATCH

	rfbSetColourMapEntriesMsg setcmap;
	RGBQUAD *rgbquad;
	UINT ncolours = 256;

	// Reserve space for the colour data
	rgbquad = new RGBQUAD[ncolours];
	if (rgbquad == NULL)
		return true;
					
	// Receive the data
	if (!m_encodemgr.GetPalette(rgbquad, ncolours))
	{
		delete [] rgbquad;
		return true;
	}

	// Compose the message
	setcmap.type = rfbSetColourMapEntries;
	setcmap.firstColour = Swap16IfLE(0);
	setcmap.nColours = Swap16IfLE(ncolours);

	try
	{
		m_stream->Send((char *) &setcmap, sz_rfbSetColourMapEntriesMsg );
	}
	catch(CStreamException& e)
	{
		MLog_Exception(e);
		delete [] rgbquad;
		return false;
	}

	// Now send the actual colour data...
	for (int i=0; i<ncolours; i++)
	{
		struct _PIXELDATA {
			CARD16 r, g, b;
		} pixeldata;

		pixeldata.r = Swap16IfLE(((CARD16)rgbquad[i].rgbRed) << 8);
		pixeldata.g = Swap16IfLE(((CARD16)rgbquad[i].rgbGreen) << 8);
		pixeldata.b = Swap16IfLE(((CARD16)rgbquad[i].rgbBlue) << 8);

		try
		{
			m_stream->Send((char *) &pixeldata, sizeof(pixeldata));
		}
		catch(CStreamException &e)
		{
			MLog_Exception(e);
			delete [] rgbquad;
			return false;
		}
	}
	// Delete the rgbquad data
	delete [] rgbquad;
	return true;

CATCH_THROW()
}

void vncClient::SetSWOffset(int x,int y)
{
TRY_CATCH
	//if (m_SWOffsetx!=x || m_SWOffsety!=y) m_encodemgr.m_buffer->ClearCache();
	m_SWOffsetx=x;
	m_SWOffsety=y;
	m_encodemgr.SetSWOffset(x,y);
CATCH_THROW("vncClient::SetSWOffset")
}

void vncClient::SetScreenOffset(int x,int y,int type)
{
TRY_CATCH
	m_ScreenOffsetx=x;
	m_ScreenOffsety=y;
	m_display_type=type;
CATCH_THROW()
}

// CACHE RDV
// Send a set of rectangles
bool vncClient::SendCacheRectangles(const rfb::RectVector &rects)
{
TRY_CATCH

	rfb::RectVector::const_iterator i;

	if (rects.size() == 0) return true;

	// Work through the list of rectangles, sending each one
	for (i= rects.begin();i != rects.end();i++)
	{
		if (!SendCacheRect(*i))
			return false;
	}

	return true;

CATCH_THROW()
}

// Tell the encoder to send a single rectangle
bool vncClient::SendCacheRect(const rfb::Rect &dest)
{
TRY_CATCH

	// Create the message header
	// Modif rdv@2002 - v1.1.x - Application Resize
	// Modif sf@2002 - Scaling
	rfbFramebufferUpdateRectHeader cacherecthdr;
	cacherecthdr.r.x = Swap16IfLE((dest.tl.x - m_SWOffsetx) / m_nScale );
	cacherecthdr.r.y = Swap16IfLE((dest.tl.y - m_SWOffsety) / m_nScale);
	cacherecthdr.r.w = Swap16IfLE((dest.br.x - dest.tl.x) / m_nScale);
	cacherecthdr.r.h = Swap16IfLE((dest.br.y - dest.tl.y) / m_nScale);
	cacherecthdr.encoding = Swap32IfLE(rfbEncodingCache);

	totalraw+=(dest.br.x - dest.tl.x)*(dest.br.y - dest.tl.y)*32 / 8; // 32bit test
	// Create the CopyRect-specific section
	rfbCacheRect cacherectbody;
	cacherectbody.special = Swap16IfLE(9999); //not used dummy

	// Now send the message;
	m_stream->Send((char *)&cacherecthdr, sizeof(cacherecthdr));
	m_stream->Send((char *)&cacherectbody, sizeof(cacherectbody));
	return true;

CATCH_THROW()
}

bool vncClient::SendCursorShapeUpdate()
{
TRY_CATCH

	m_cursor_update_pending = false;

	if (!m_encodemgr.SendCursorShape(m_stream)) 
	{
		m_cursor_update_sent = false;
		return m_encodemgr.SendEmptyCursorShape(m_stream);
	}

	m_cursor_update_sent = true;
	return true;

CATCH_THROW()
}

bool vncClient::SendCursorPosUpdate()
{
TRY_CATCH

	m_cursor_pos_changed = false;

	rfbFramebufferUpdateRectHeader hdr;
	hdr.r.x = Swap16IfLE(m_cursor_pos.x);
	hdr.r.y = Swap16IfLE(m_cursor_pos.y);
	hdr.r.w = 0;
	hdr.r.h = 0;
	hdr.encoding = Swap32IfLE(rfbEncodingPointerPos);

	m_stream->Send((char *)&hdr, sizeof(hdr));
	return true;

CATCH_THROW()
}

// Tight specific - Send LastRect marker indicating that there are no more rectangles to send
bool vncClient::SendLastRect()
{
TRY_CATCH

	// Create the message header
	rfbFramebufferUpdateRectHeader hdr;
	hdr.r.x = 0;
	hdr.r.y = 0;
	hdr.r.w = 0;
	hdr.r.h = 0;
	hdr.encoding = Swap32IfLE(rfbEncodingLastRect);

	// Now send the message;
	m_stream->Send((char *)&hdr, sizeof(hdr));
	return true;

CATCH_THROW()
}


//
// sf@2002 - New cache rects transport - Uses Zlib
//
// 
bool vncClient::SendCacheZip(const rfb::RectVector &rects)
{
TRY_CATCH

	int nNbCacheRects = rects.size();
	if (!nNbCacheRects) return true;
	unsigned long rawDataSize = nNbCacheRects * sz_rfbRectangle;
	unsigned long maxCompSize = (rawDataSize + (rawDataSize/100) + 8);

	// Check RawCacheZipBuff
	// create a space big enough for the Zlib encoded cache rects list
	if (m_nRawCacheZipBufSize < rawDataSize)
	{
		if (m_pRawCacheZipBuf != NULL)
		{
			delete [] m_pRawCacheZipBuf;
			m_pRawCacheZipBuf = NULL;
		}
		m_pRawCacheZipBuf = new BYTE [rawDataSize+1];
		if (m_pRawCacheZipBuf == NULL) 
			return false;
		m_nRawCacheZipBufSize = rawDataSize;
	}

	// Copy all the cache rects coordinates into the RawCacheZip Buffer 
	rfbRectangle theRect;
	rfb::RectVector::const_iterator i;
	BYTE* p = m_pRawCacheZipBuf;
	for (i = rects.begin();i != rects.end();i++)
	{
		theRect.x = Swap16IfLE(((*i).tl.x - m_SWOffsetx) / m_nScale );
		theRect.y = Swap16IfLE(((*i).tl.y - m_SWOffsety) / m_nScale);
		theRect.w = Swap16IfLE(((*i).br.x - (*i).tl.x) / m_nScale);
		theRect.h = Swap16IfLE(((*i).br.y - (*i).tl.y) / m_nScale);
		memcpy(p, (BYTE*)&theRect, sz_rfbRectangle);
		p += sz_rfbRectangle;
	}

	// Create a space big enough for the Zlib encoded cache rects list
	if (m_nCacheZipBufSize < maxCompSize)
	{
		if (m_pCacheZipBuf != NULL)
		{
			delete [] m_pCacheZipBuf;
			m_pCacheZipBuf = NULL;
		}
		m_pCacheZipBuf = new BYTE [maxCompSize+1];
		if (m_pCacheZipBuf == NULL) return 0;
		m_nCacheZipBufSize = maxCompSize;
	}

	int nRet = compress((unsigned char*)(m_pCacheZipBuf),
						(unsigned long*)&maxCompSize,
						(unsigned char*)m_pRawCacheZipBuf,
						rawDataSize
						);

	if (nRet != 0)
	{
		return false;
	}

	//vnclog.Print(LL_INTINFO, VNCLOG("*** Sending CacheZip Rects=%d Size=%d (%d)\r\n"), nNbCacheRects, maxCompSize, nNbCacheRects * 14);

	// Send the Update Rect header
	rfbFramebufferUpdateRectHeader CacheRectsHeader;
	CacheRectsHeader.r.x = Swap16IfLE(nNbCacheRects);
	CacheRectsHeader.r.y = 0;
	CacheRectsHeader.r.w = 0;
 	CacheRectsHeader.r.h = 0;
	CacheRectsHeader.encoding = Swap32IfLE(rfbEncodingCacheZip);

	// Format the ZlibHeader
	rfbZlibHeader CacheZipHeader;
	CacheZipHeader.nBytes = Swap32IfLE(maxCompSize);

	// Now send the message
	m_stream->Send((char *)&CacheRectsHeader, sizeof(CacheRectsHeader));
	m_stream->Send((char *)&CacheZipHeader, sizeof(CacheZipHeader));
	m_stream->Send((char *)m_pCacheZipBuf, maxCompSize);
	return true;

CATCH_THROW("vncClient::SendCacheZip")
}

void vncClient::EnableCache(bool enabled)
{
TRY_CATCH
	m_encodemgr.EnableCache(enabled);
CATCH_THROW()
}

void vncClient::SetProtocolVersion(rfbProtocolVersionMsg *protocolMsg)
{
TRY_CATCH
	if (protocolMsg!=NULL) memcpy(ProtocolVersionMsg,protocolMsg,sz_rfbProtocolVersionMsg);
	else strcpy(ProtocolVersionMsg,"0.0.0.0");
CATCH_THROW()
}

void vncClient::Clear_Update_Tracker()
{
TRY_CATCH
	m_update_tracker.clear();
CATCH_THROW()
}

void vncClient::SendStop()
{
TRY_CATCH
	DisableProtocol();
	char code = rfbStopSession;
	m_stream->Send(&code,1);
	EnableProtocol();
CATCH_THROW()
}

void vncClient::SetStopReason(const ESessionStopReason stopReason)
{
TRY_CATCH
	if (m_stopReason == STREAM_ERROR)	//Setting reason only once
		m_stopReason = stopReason;
CATCH_THROW()
}

ESessionStopReason vncClient::GetStopReason()
{
TRY_CATCH
	return m_stopReason;
CATCH_THROW()
}