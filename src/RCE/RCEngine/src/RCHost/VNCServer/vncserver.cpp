//  Copyright (C) 2002 Ultr@Vnc Team Members. All Rights Reserved.
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


// vncServer.cpp

// vncServer class implementation

// Includes
#include "stdhdrs_srv.h"
#include "CRCHost.h"
#include <string.h>
#include <lmcons.h>

// Custom
#include "WinVNC.h"
#include "vncServer.h"
#include "vncClient.h"
#include "vncService.h"
#include "mmSystem.h" // sf@2002
#include "CVisualPointer.h"

#pragma warning( disable: 4996 )//<func> was declared deprecated

bool g_Server_running;
extern bool g_Desktop_running;
void*	vncServer::pThis;

/// Crush for SetProtectedProcess method of RCHost
class COriginalIEPIDWrapper
{
private:
	DWORD m_pid;
public:
	COriginalIEPIDWrapper() : m_pid(-1)
	{
	}

	void SetPid(const DWORD pid)
	{
		m_pid = pid;
	}

	DWORD GetPid() const
	{
		return m_pid;
	}
};

bool vncServer::IsProcessIsolated()
{
	int result(true);
TRY_CATCH

	OSVERSIONINFOEX osInf;
	osInf.dwOSVersionInfoSize=sizeof(osInf);
	if(!GetVersionEx((OSVERSIONINFO*)&osInf))
	{
		Log.WinError(_ERROR_,_T("Failed to GetVersionEx"));
		return true;
	}
	return osInf.dwMajorVersion>5;

/// --------------------------------------------------
	
	static const tstring testKey(GetGUID());
	static const tstring valueName(_T("testValue"));
	static const DWORD value = 12345;
	{
		/// Creating key
		HKEY hkey;
		DWORD disposition;
		DWORD res;
		if ((res = RegCreateKeyEx(HKEY_CURRENT_USER, testKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkey, &disposition)) != ERROR_SUCCESS)
		{
			SetLastError(res);
			throw MCException_Win("Failed to RegCreateKeyEx");
		}
		boost::shared_ptr<boost::remove_pointer<HKEY>::type> key(hkey,::RegCloseKey);
		if ((res = RegSetValueEx(hkey, valueName.c_str(), 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(value))) != ERROR_SUCCESS)
		{
			SetLastError(res);
			throw MCException_Win("Failed to RegSetValueEx");
		}
	}
	{
		/// Reading key
		HKEY hkey;
		DWORD res;
		if ((res = RegOpenKey(HKEY_CURRENT_USER, testKey.c_str(), &hkey)) != ERROR_SUCCESS)
		{
			SetLastError(res);
			throw MCException_Win("Failed to RegCreateKeyEx");
		}
		boost::shared_ptr<boost::remove_pointer<HKEY>::type> key(hkey,::RegCloseKey);
		DWORD type = REG_DWORD;
		DWORD size = sizeof(DWORD);
		DWORD value;
		if ((res = RegQueryValueEx(hkey, valueName.c_str(), 0, &type, reinterpret_cast<BYTE*>(&value), &size)) == ERROR_SUCCESS)
		{
			result = false;
		}
	}
	/// Erasing key
	int res;
	if ((res = RegDeleteKey(HKEY_CURRENT_USER, testKey.c_str())) != ERROR_SUCCESS)
	{
		SetLastError(res);
		Log.WinError(_ERROR_,_T("Failed to RegDeleteKey"));
	}

	return result;
CATCH_LOG("IsProcessIsolated")
	return true;
}

// vncServer::UpdateTracker routines
void vncServer::EnableProtocol(const bool enable)
{
TRY_CATCH

	vncClientList::iterator i;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	// Post this update to all the connected clients
	for ( i=m_authClients.begin(); i!=m_authClients.end(); ++i)
	{
		if (enable)
			GetClient(*i)->EnableProtocol();
		else
			GetClient(*i)->DisableProtocol();
	}

CATCH_THROW()
}

void vncServer::ServerUpdateTracker::add_changed(const rfb::Region2D &rgn) 
{
TRY_CATCH

	vncClientList::iterator i;
	
	boost::recursive_mutex::scoped_lock l(m_server->m_clientsLock);

	// Post this update to all the connected clients
	for (i = m_server->m_authClients.begin(); i != m_server->m_authClients.end(); i++)
	{
		// Post the update
		// REalVNC 336 change
		// m_server->GetClient(*i)->GetUpdateTracker().add_changed(rgn);
		vncClient* client = m_server->GetClient(*i);
		boost::recursive_mutex::scoped_lock l(client->GetUpdateLock());
		client->GetUpdateTracker().add_changed(rgn);

	}

CATCH_THROW("vncServer::ServerUpdateTracker::add_changed")
}

void vncServer::ServerUpdateTracker::add_cached(const rfb::Region2D &rgn) 
{
TRY_CATCH

	vncClientList::iterator i;
	
	boost::recursive_mutex::scoped_lock l(m_server->m_clientsLock);

	// Post this update to all the connected clients
	for (i = m_server->m_authClients.begin(); i != m_server->m_authClients.end(); i++)
	{
		// Post the update
		// RealVNC 336 change
		// m_server->GetClient(*i)->GetUpdateTracker().add_cached(rgn);
		vncClient* client = m_server->GetClient(*i);
		boost::recursive_mutex::scoped_lock l(client->GetUpdateLock());
		client->GetUpdateTracker().add_cached(rgn);
	}

CATCH_THROW("vncServer::ServerUpdateTracker::add_cached")
}





void vncServer::ServerUpdateTracker::add_copied(const rfb::Region2D &dest, const rfb::Point &delta) 
{
TRY_CATCH

	vncClientList::iterator i;
	
	boost::recursive_mutex::scoped_lock l(m_server->m_clientsLock);

	// Post this update to all the connected clients
	for (i = m_server->m_authClients.begin(); i != m_server->m_authClients.end(); i++)
	{
		// Post the update
		// RealVNC 336 change
		//m_server->GetClient(*i)->GetUpdateTracker().add_copied(dest, delta);
		vncClient* client = m_server->GetClient(*i);
		boost::recursive_mutex::scoped_lock l(client->GetUpdateLock());
		client->GetUpdateTracker().add_copied(dest, delta);

	}

CATCH_THROW("vncServer::ServerUpdateTracker::add_copied")
}



// Constructor/destructor
vncServer::vncServer(CRCHost* RCHost)
	:	m_RCHost(RCHost),
		m_inputProxyNeeded(IsProcessIsolated()),
		m_fWallpaperHidden(false)
{
TRY_CATCH
	
	m_hooksProxyNeeded = m_inputProxyNeeded;

	if (!m_RCHost) 
		throw MCException("m_RCHost==NULL");

	try
	{
		m_srvCommunicator.reset(new CSrvSTDQueueComm(DEF_WND_CLS_NAME));
	}
	catch(CExceptionBase &exception)
	{
		Log.Add(_ERROR_,_T("Failed to create communication with service: %s"),exception.what());
	}

	// used for our retry timer proc;
	pThis = this;

	// Initialise some important stuffs...
	g_Server_running=true;
	m_desktop = NULL;
	m_name = NULL;
	m_querysetting = 2;
	m_queryaccept = 0;
	m_querytimeout = 10;

	// Autolock settings
	m_lock_on_exit = 0;

	// Set the polling mode options
	m_poll_fullscreen = FALSE;
	m_poll_foreground = FALSE;
	m_poll_undercursor = TRUE;

	m_poll_oneventonly = FALSE;
	m_poll_consoleonly = TRUE;

	m_driver = FALSE;
	m_hook = FALSE;
	m_virtual = FALSE;
	sethook=false;
	
	// General options
	m_lock_on_exit = 0;
	m_connect_pri = 0;

	// Set the input options
	m_enable_remote_inputs = TRUE;
	m_disable_local_inputs = FALSE;

	// Clear the client mapping table
	for (int x=0; x<MAX_CLIENTS; x++)
		m_clientmap[x] = NULL;
	m_nextid = 0;

	// Initialise the update tracker
	m_update_tracker.init(this);

	// Signal set when a client quits
	m_clientquitsig = new boost::condition();

	// Modif sf@2002
	m_SingleWindow = FALSE;
	strcpy(m_szWindowName, "");

	// Modif sf@2002
	m_TurboMode = false;
	// m_fCursorMoved = false;

	m_nDefaultScale = 1;

	m_fXRichCursor = false;

CATCH_THROW("vncServer::vncServer")
}

vncServer::~vncServer()
{
TRY_CATCH

	// if we are in the middle of retrying our autoreconnect - kill the timer
	if ( m_retry_timeout > 0 )
	{
		KillTimer( NULL, m_retry_timeout );
		m_retry_timeout = 0;
	}

	// Remove any active clients!
	KillAuthClients();
	KillUnauthClients();

	// Wait for all the clients to die
	WaitUntilAuthEmpty();
	WaitUntilUnauthEmpty();

	// Don't free the desktop until no KillClient is likely to free it
	{	boost::recursive_mutex::scoped_lock l(m_desktopLock);

		if (m_desktop != NULL)
		{
			delete m_desktop;
			m_desktop = NULL;
		}
	}
	while (g_Desktop_running)
	{
		Sleep(100);
		//vnclog.Print(LL_STATE, VNCLOG("Waiting for desktop to shutdown\n"));
	}

	if (m_name != NULL)
	{
		free(m_name);
		m_name = NULL;
	}

	if (m_clientquitsig != NULL)
	{
		delete m_clientquitsig;
		m_clientquitsig = NULL;
	}
	//We need to give the client thread to give some time to close
	// bad hack
	Sleep(500);
	//sometimes crash, vnclog seems already removed
//	vnclog.Print(LL_STATE, VNCLOG("shutting down server object(4)\n"));
	g_Server_running=false;

CATCH_LOG("vncServer::~vncServer")
}


vncClientId vncServer::AddClient(boost::shared_ptr<CAbstractStream> stream, bool shared, int capability, /*bool keysenabled, bool ptrenabled,*/rfbProtocolVersionMsg *protocolMsg)
{
TRY_CATCH

	vncClient *client;

	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	// Try to allocate a client id...
	vncClientId clientid = m_nextid;
	do
	{
		clientid = (clientid+1) % MAX_CLIENTS;
		if (clientid == m_nextid)
		{
			///TODO: how to handle stream?
			return -1;
		}
	}
	while (m_clientmap[clientid] != NULL);

	// Create a new client and add it to the relevant client list
	client = new vncClient();
	if (client == NULL) 
	{
		///TODO: how to handle stream?
		return -1;
	}

	// Set the client's settings
	client->SetProtocolVersion(protocolMsg);
	client->SetCapability(capability);
	client->EnableKeyboard(/*keysenabled &&*/ m_enable_remote_inputs);
	client->EnablePointer(/*ptrenabled &&*/ m_enable_remote_inputs);

	// Start the client
	if (!client->Init(this, stream, shared, clientid))
	{
		Log.Add(_MESSAGE_,"failed to initialise client object");
		///TODO: how to handle stream?
		return -1;
	}

	m_clientmap[clientid] = client;

	// Add the client to unauth the client list
	m_unauthClients.push_back(clientid);

	// Notify anyone interested about this event
	DoNotify(WM_SRV_CLIENT_CONNECT, 0, 0);

	return clientid;

CATCH_THROW("vncServer::AddClient")
}

bool vncServer::Authenticated(vncClientId clientid)
{
TRY_CATCH

	vncClientList::iterator i;
	bool authok = TRUE;
	boost::recursive_mutex::scoped_lock l1(m_desktopLock);
	boost::recursive_mutex::scoped_lock l2(m_clientsLock);

	// Search the unauthenticated client list
	for (i = m_unauthClients.begin(); i != m_unauthClients.end(); i++)
	{
		// Is this the right client?
		if ((*i) == clientid)
		{
			vncClient *client = GetClient(clientid);
			

			// Yes, so remove the client and add it to the auth list
			m_unauthClients.erase(i);

			// Create the screen handler if necessary
			if (m_desktop == NULL)
			{
				m_desktop = new vncDesktop(this);
				if (m_desktop == NULL)
				{
					client->Kill();
					authok = FALSE;
					break;
				}
				if (!m_desktop->Init(this))
				{
					Log.Add(_ERROR_,"Desktop init failed, unlock in application mode");
					client->Kill();
					authok = FALSE;
					delete m_desktop;
					m_desktop = NULL;
					break;
				}
			}

			// Tell the client about this new buffer
			client->SetBuffer(&(m_desktop->m_buffer));

			// Add the client to the auth list
			m_authClients.push_back(clientid);
			break;
		}
	}

	// Notify anyone interested of this event
	DoNotify(WM_SRV_CLIENT_AUTHENTICATED, 0, 0);

	return authok;

CATCH_THROW("vncServer::Authenticated")
}

void vncServer::KillClient(vncClientId clientid)
{
TRY_CATCH

	vncClientList::iterator i;
	bool done = FALSE;

	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	// Find the client in one of the two lists
	for (i = m_unauthClients.begin(); i != m_unauthClients.end(); i++)
	{
		// Is this the right client?
		if ((*i) == clientid)
		{
			// Ask the client to die
			vncClient *client = GetClient(clientid);
			client->Kill();

			done = TRUE;
			break;
		}
	}
	if (!done)
	{
		for (i = m_authClients.begin(); i != m_authClients.end(); i++)
		{
			// Is this the right client?
			if ((*i) == clientid)
			{
				// Yes, so kill it
				vncClient *client = GetClient(clientid);
				client->Kill();

				done = TRUE;
				break;
			}
		}
	}

CATCH_THROW("vncServer::KillClient")
}

void vncServer::KillClient(LPSTR szClientName)
{
TRY_CATCH

	vncClientList::iterator i;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	vncClient *pClient = NULL;

	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		pClient = GetClient(*i);
		if (!stricmp(pClient->GetClientName(), szClientName))
		{
			pClient->Kill();
		}
	}

CATCH_THROW("vncServer::KillClient")
}

bool vncServer::HasVisualPointerClient()
{
TRY_CATCH

	bool result(false);

	vncClientList::iterator i;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	vncClient *client;
	// Tell all the authorised clients to die!
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		client = GetClient(*i);
		if (!client->PointerEnabled() && client->VisualPointerEnabled())
		{
			result = true;
			break;
		}
	}

	return result;

CATCH_THROW("vncServer::HasVisualPointerClient")
}

void vncServer::KillAuthClients()
{
TRY_CATCH

	vncClientList::iterator i;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	// Tell all the authorised clients to die!
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Kill the client
		GetClient(*i)->Kill();
	}

CATCH_THROW("vncServer::KillAuthClients")
}

void vncServer::ListAuthClients(HWND hListBox)
{
TRY_CATCH

	vncClientList::iterator i;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		SendMessage(hListBox, 
					LB_ADDSTRING,
					0,
					(LPARAM) GetClient(*i)->GetClientName()
					);
	}

CATCH_THROW("vncServer::ListAuthClients")
}


//
// sf@2002 - test if there's a slow client connected
// The criteria is a client that has been using a slow
// encoding for more than 10 seconds after its connection
//
bool vncServer::IsThereASlowClient()
{
TRY_CATCH

	vncClientList::iterator i;
	bool fFound = false;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		if (GetClient(*i)->IsSlowEncoding())
		{
			if (timeGetTime() - GetClient(*i)->GetConnectTime() > 10000) 
			{
				fFound = true;
				break;
			}	
			else
				continue;
		}
		else
			continue;
	}
	return fFound;

CATCH_THROW("vncServer::IsThereASlowClient")
}

bool vncServer::IsThereAUltraEncodingClient()
{
TRY_CATCH

	vncClientList::iterator i;
	bool fFound = false;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		if (GetClient(*i)->IsUltraEncoding())
		{
			return true;
		}
	}
	return false;

CATCH_THROW("vncServer::IsThereAUltraEncodingClient")
}


void vncServer::KillUnauthClients()
{
TRY_CATCH

	vncClientList::iterator i;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	// Tell all the authorised clients to die!
	for (i = m_unauthClients.begin(); i != m_unauthClients.end(); i++)
	{
		// Kill the client
		GetClient(*i)->Kill();
	}

CATCH_THROW("vncServer::KillUnauthClients")
}


UINT vncServer::AuthClientCount()
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	return m_authClients.size();

CATCH_THROW("vncServer::AuthClientCount")
}

UINT vncServer::UnauthClientCount()
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	return m_unauthClients.size();

CATCH_THROW("")
}

bool vncServer::UpdateWanted()
{
TRY_CATCH

	vncClientList::iterator i;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	// Iterate over the authorised clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		if (GetClient(*i)->UpdateWanted())
			return TRUE;
	}
	return FALSE;

CATCH_THROW("vncServer::UpdateWanted")
}

bool vncServer::RemoteEventReceived()
{
TRY_CATCH

	vncClientList::iterator i;
	bool result = FALSE;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	// Iterate over the authorised clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		result = result || GetClient(*i)->RemoteEventReceived();
	}
	return result;

CATCH_THROW("vncServer::RemoteEventReceived")
}

void vncServer::WaitUntilAuthEmpty()
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	// Wait for all the clients to exit
	while (!m_authClients.empty())
	{
		// Wait for a client to quit
		m_clientquitsig->wait(l);
	}

CATCH_THROW("vncServer::WaitUntilAuthEmpty")
}

void vncServer::WaitUntilUnauthEmpty()
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	// Wait for all the clients to exit
	while (!m_unauthClients.empty())
	{
		// Wait for a client to quit
		m_clientquitsig->wait(l);
	}

CATCH_THROW("vncServer::WaitUntilUnauthEmpty")
}

// Client info retrieval/setup
vncClient* vncServer::GetClient(vncClientId clientid)
{
TRY_CATCH

	if ((clientid >= 0) && (clientid < MAX_CLIENTS))
		return m_clientmap[clientid];
	return NULL;

CATCH_THROW("vncServer::GetClient")
}

vncClientList vncServer::ClientList()
{
TRY_CATCH

	vncClientList clients;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	clients = m_authClients;
	return clients;

CATCH_THROW("vncServer::ClientList")
}

void vncServer::SetCapability(vncClientId clientid, int capability)
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	vncClient *client = GetClient(clientid);
	if (client != NULL)
		client->SetCapability(capability);

CATCH_THROW("vncServer::SetCapability")
}

void vncServer::SetKeyboardEnabled(vncClientId clientid, bool enabled)
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	vncClient *client = GetClient(clientid);
	if (client != NULL)
		client->EnableKeyboard(enabled);

CATCH_THROW("vncServer::SetKeyboardEnabled")
}

void vncServer::SetPointerEnabled(vncClientId clientid, bool enabled)
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	vncClient *client = GetClient(clientid);
	if (client != NULL)
		client->EnablePointer(enabled);

CATCH_THROW("vncServer::SetPointerEnabled")
}

void vncServer::SetVisualPointerEnabled(vncClientId clientid, bool enabled)
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	vncClient *client = GetClient(clientid);
	if (client != NULL)
		client->EnableVisualPointer(enabled);

CATCH_THROW("vncServer::SetPointerEnabled")
}

int vncServer::GetCapability(vncClientId clientid)
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	vncClient *client = GetClient(clientid);
	if (client != NULL)
		return client->GetCapability();
	return 0;

CATCH_THROW("vncServer::GetCapability")
}

const char* vncServer::GetClientName(vncClientId clientid)
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	vncClient *client = GetClient(clientid);
	if (client != NULL)
		return client->GetClientName();
	return NULL;

CATCH_THROW("vncServer::GetClientName")
}

// RemoveClient should ONLY EVER be used by the client to remove itself.
void vncServer::RemoveClient(vncClientId clientid, bool notify)
{
TRY_CATCH

	vncClientList::iterator i;
	bool done = FALSE;
	boost::recursive_mutex::scoped_lock l1(m_desktopLock);
	{	boost::recursive_mutex::scoped_lock l2(m_clientsLock);

		// Find the client in one of the two lists
		for (i = m_unauthClients.begin(); i != m_unauthClients.end(); i++)
		{
			// Is this the right client?
			if ((*i) == clientid)
			{
				//vnclog.Print(LL_INTINFO, VNCLOG("removing unauthorised client\n"));

				// Yes, so remove the client and kill it
				m_unauthClients.erase(i);
				m_clientmap[clientid] = NULL;
				done = TRUE;
				break;
			}
		}
		if (!done)
		{
			for (i = m_authClients.begin(); i != m_authClients.end(); i++)
			{
				// Is this the right client?
				if ((*i) == clientid)
				{
					//vnclog.Print(LL_INTINFO, VNCLOG("removing authorised client\n"));
					ESessionStopReason reason = GetClient(clientid)->GetStopReason();
					// Yes, so remove the client and kill it
					m_authClients.erase(i);

					/// Restoring wallpaper if no more clients
					if (m_authClients.empty())
						ChangeWallpaperState(false);

					// Notifying RCHost
					TRY_CATCH
						if (m_RCHost && notify && clientid != m_RCHost->m_shadowStreamClientId)
							m_RCHost->NotifySessionStopped(clientid, reason);
					CATCH_LOG("m_RCHost->NotifySessionStopped failed")
					m_clientmap[clientid] = NULL;

					done = TRUE;
					break;
				}
			}
		}

		// Signal that a client has quit
		m_clientquitsig->notify_one();

	} // Unlock the clientLock

	// Are there any authorised clients connected?
	if (m_authClients.empty() && (m_desktop != NULL))
	{
		//vnclog.Print(LL_STATE, VNCLOG("deleting desktop server\n"));

		// Are there locksettings set?
		//if (LockSettings() == 1)
		//{
		//    // Yes - lock the machine on disconnect!
		//	//vncService::LockWorkstation();
		//	//TODO: what we should do instead of lock?
		//} else if (LockSettings() > 1)
		//{
		//    char username[UNLEN+1];

		//    vncService::CurrentUser((char *)&username, sizeof(username));
		//    if (strcmp(username, "") != 0)
		//    {
		//		// Yes - force a user logoff on disconnect!
		//		if (!ExitWindowsEx(EWX_LOGOFF, 0))
		//			vnclog.Print(LL_CONNERR, VNCLOG("client disconnect - failed to logoff user!\n"));
		//    }
		//}

		// Delete the screen server
		delete m_desktop;
		m_desktop = NULL;
		//vnclog.Print(LL_STATE, VNCLOG("desktop deleted\n"));
	}

	// Notify anyone interested of the change
	DoNotify(WM_SRV_CLIENT_DISCONNECT, 0, 0);

	if (!HasVisualPointerClient())
		VISUAL_POINTER_INSTANCE.Hide();

CATCH_THROW("vncServer::RemoveClient")
}

// NOTIFICATION HANDLING!

// Connect/disconnect notification
bool vncServer::AddNotify(HWND hwnd)
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	// Add the window handle to the list
	m_notifyList.push_front(hwnd);
	return TRUE;

CATCH_THROW("vncServer::AddNotify")
}

bool vncServer::RemNotify(HWND hwnd)
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	// Remove the window handle from the list
	vncNotifyList::iterator i;
	for (i=m_notifyList.begin(); i!=m_notifyList.end(); i++)
	{
		if ((*i) == hwnd)
		{
			// Found the handle, so remove it
			m_notifyList.erase(i);
			return TRUE;
		}
	}
	return FALSE;

CATCH_THROW("vncServer::RemNotify")
}

// Send a notification message
void vncServer::DoNotify(UINT message, WPARAM wparam, LPARAM lparam)
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	// Send the given message to all the notification windows
	vncNotifyList::iterator i;
	for (i=m_notifyList.begin(); i!=m_notifyList.end(); i++)
	{
		PostMessage((*i), message, wparam, lparam);
	}

CATCH_THROW("vncServer::DoNotify")
}

void vncServer::UpdateMouse()
{
TRY_CATCH

	vncClientList::iterator i;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	// Post this mouse update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		GetClient(*i)->UpdateMouse();
	}

CATCH_THROW("vncServer::UpdateMouse")
}

void vncServer::UpdateClipText(const char* text)
{
TRY_CATCH

	vncClientList::iterator i;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	// Post this update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		GetClient(*i)->UpdateClipText(text);
	}
CATCH_THROW("vncServer::UpdateClipText")
}

void vncServer::UpdateCursorShape()
{
TRY_CATCH

	vncClientList::iterator i;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	// Post this update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		GetClient(*i)->UpdateCursorShape();
	}

CATCH_THROW("vncServer::UpdateCursorShape")
}

void vncServer::UpdatePalette()
{
TRY_CATCH

	vncClientList::iterator i;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	// Post this update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		GetClient(*i)->UpdatePalette();
	}

CATCH_THROW("vncServer::UpdatePalette")
}

void vncServer::UpdateLocalFormat()
{
TRY_CATCH

	vncClientList::iterator i;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	// Post this update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		GetClient(*i)->UpdateLocalFormat();
	}

CATCH_THROW("vncServer::UpdateLocalFormat")
}

void vncServer::UpdateLocalClipText(LPSTR text)
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_desktopLock);

	if (m_desktop != NULL)
		m_desktop->SetClipText(text);

CATCH_THROW("vncServer::UpdateLocalClipText")
}

// Name and port number handling
void vncServer::SetName(const char * name)
{
TRY_CATCH

	// Set the name of the desktop
	if (m_name != NULL)
	{
		free(m_name);
		m_name = NULL;
	}
	
	m_name = strdup(name);

CATCH_THROW("vncServer::SetName")
}

// Remote input handling
void vncServer::EnableRemoteInputs(bool enable)
{
TRY_CATCH

	m_enable_remote_inputs = enable;
	vncClientList::iterator i;
		
	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		GetClient(*i)->EnableKeyboard(m_enable_remote_inputs);
		GetClient(*i)->EnablePointer(m_enable_remote_inputs);
	}

CATCH_THROW("vncServer::EnableRemoteInputs")
}

bool vncServer::RemoteInputsEnabled()
{
TRY_CATCH

	return m_enable_remote_inputs;

CATCH_THROW("vncServer::RemoteInputsEnabled")
}

// Local input handling
void vncServer::DisableLocalInputs(bool disable)
{
TRY_CATCH

	m_disable_local_inputs = disable;

CATCH_THROW("vncServer::DisableLocalInputs")
}

bool vncServer::LocalInputsDisabled()
{
TRY_CATCH

	return m_disable_local_inputs;

CATCH_THROW("vncServer::LocalInputsDisabled")
}

void vncServer::GetScreenInfo(int &width, int &height, int &depth)
{
TRY_CATCH

	rfbServerInitMsg scrinfo;
	boost::recursive_mutex::scoped_lock l(m_desktopLock);

	// Is a desktop object currently active?
	// No, so create a dummy desktop and interrogate it
	if (m_desktop == NULL)
	{
		HDC				hrootdc;
		hrootdc = GetDC(NULL);
		if (hrootdc == NULL) 
		{
			//vnclog.Print(LL_INTERR, VNCLOG("Failed rootdc \n"));
			Log.Add(_ERROR_, "Failed rootdc");
			scrinfo.framebufferWidth = 0;
			scrinfo.framebufferHeight = 0;
			scrinfo.format.bitsPerPixel = 0;
		}
		else
		{
			scrinfo.framebufferWidth = GetDeviceCaps(hrootdc, HORZRES);
			scrinfo.framebufferHeight = GetDeviceCaps(hrootdc, VERTRES);
			HBITMAP membitmap = CreateCompatibleBitmap(hrootdc, scrinfo.framebufferWidth, scrinfo.framebufferHeight);
			if (membitmap == NULL) {
				scrinfo.framebufferWidth = 0;
				scrinfo.framebufferHeight = 0;
				scrinfo.format.bitsPerPixel = 0;
				}
			else
			{
				struct _BMInfo {
				bool			truecolour;
				BITMAPINFO		bmi;
				// Colormap info - comes straight after BITMAPINFO - **HACK**
				RGBQUAD			cmap[256];
				} m_bminfo;
				int result;
				HDC				hmemdc;
				memset(&m_bminfo, 0, sizeof(m_bminfo));
				m_bminfo.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				m_bminfo.bmi.bmiHeader.biBitCount = 0;
				hmemdc = CreateCompatibleDC(hrootdc);
				if (hmemdc == NULL) 
					{
						scrinfo.framebufferWidth = 0;
						scrinfo.framebufferHeight = 0;
						scrinfo.format.bitsPerPixel = 0;
					}
				else
					{
						result = ::GetDIBits(hmemdc, membitmap, 0, 1, NULL, &m_bminfo.bmi, DIB_RGB_COLORS);
						if (result == 0) 
							{
											scrinfo.framebufferWidth = 0;
											scrinfo.framebufferHeight = 0;
											scrinfo.format.bitsPerPixel = 0;
							}
						else
							{
								result = ::GetDIBits(hmemdc, membitmap,  0, 1, NULL, &m_bminfo.bmi, DIB_RGB_COLORS);
								if (result == 0) 
									{
										scrinfo.framebufferWidth = 0;
										scrinfo.framebufferHeight = 0;
										scrinfo.format.bitsPerPixel = 0;
									}
								else
									{
										scrinfo.format.bitsPerPixel = m_bminfo.bmi.bmiHeader.biBitCount;
										if (scrinfo.format.bitsPerPixel==24) scrinfo.format.bitsPerPixel=32;
									}
							}//result
					if (hmemdc != NULL) DeleteDC(hmemdc);
					}//memdc
				if (membitmap != NULL) DeleteObject(membitmap);
			}//membitmap
			if (hrootdc != NULL) ReleaseDC(NULL, hrootdc);
		}//rootdc
	
		// No, so create a dummy desktop and interrogate it

	}
	else
	{
		m_desktop->FillDisplayInfo(&scrinfo);
	}

	// Receive the info from the scrinfo structure
	width = scrinfo.framebufferWidth;
	height = scrinfo.framebufferHeight;
	depth = scrinfo.format.bitsPerPixel;

CATCH_THROW("vncServer::GetScreenInfo")
}

// Modif sf@2002
void vncServer::SetSingleWindowName(const char *szName)
{
TRY_CATCH

    memcpy(m_szWindowName, szName, 32);

CATCH_THROW("vncServer::SetSingleWindowName")
}

// Modef rdv@202
void vncServer::SetNewSWSize(long w,long h,bool desktop)
{
TRY_CATCH

	vncClientList::iterator i;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	// Post this screen size update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		if (!GetClient(*i)->SetNewSWSize(w,h,desktop)) 
		{
			//vnclog.Print(LL_INTINFO, VNCLOG("Unable to set new desktop size\n"));
			KillClient(*i);
		}
	}

CATCH_THROW("vncServer::SetNewSWSize")
}

void vncServer::SetSWOffset(int x,int y)
{
TRY_CATCH

	vncClientList::iterator i;
	
	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	// Post this screen size update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		GetClient(*i)->SetSWOffset(x,y);
	}

CATCH_THROW("vncServer::SetSWOffset")
}

void vncServer::SetScreenOffset(int x,int y,int type)
{
TRY_CATCH

	vncClientList::iterator i;
		
	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	// Post this screen size update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		GetClient(*i)->SetScreenOffset(x,y,type);
	}

CATCH_THROW("vncServer::SetScreenOffset")
}

// Modif sf@2002 - v1.1.0 - Default Scaling
UINT vncServer::GetDefaultScale()
{
TRY_CATCH

	return m_nDefaultScale;

CATCH_THROW("vncServer::GetDefaultScale")
}


bool vncServer::SetDefaultScale(int nScale)
{
TRY_CATCH

	m_nDefaultScale = nScale;
	return TRUE;

CATCH_THROW("vncServer::SetDefaultScale")
}

//
// sgf@2002 - for now, we disable cache rects when more than one client
// 
void vncServer::DisableCacheForAllClients()
{
TRY_CATCH

	vncClientList::iterator i;
		
	boost::recursive_mutex::scoped_lock l(m_clientsLock);

	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		GetClient(*i)->EnableCache(FALSE);
	}

CATCH_THROW("vncServer::DisableCacheForAllClients")
}

void vncServer::Clear_Update_Tracker() 
{
TRY_CATCH

	vncClientList::iterator i;
	boost::recursive_mutex::scoped_lock l(m_clientsLock);
	// Post this update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		GetClient(*i)->Clear_Update_Tracker();

	}

CATCH_THROW("vncServer::Clear_Update_Tracker")
}


void vncServer::Driver(bool enable)
{
TRY_CATCH

	sethook=true;
	m_driver = enable;

CATCH_THROW("vncServer::Driver")
}
	
void vncServer::Hook(bool enable)
{
TRY_CATCH

	sethook=true;
	m_hook=enable;

CATCH_THROW("vncServer::Hook")
}

void vncServer::SetHookings()
{
TRY_CATCH

	if (sethook && m_desktop)
	{
		 m_desktop->SethookMechanism(Hook(),Driver());
	}
	sethook=false;

CATCH_THROW("vncServer::SetHookings")
}

void vncServer::EnableXRichCursor(bool fEnable)
{
TRY_CATCH

	m_fXRichCursor = fEnable;

CATCH_THROW("vncServer::EnableXRichCursor")
}

bool vncServer::All_clients_initialalized() 
{
TRY_CATCH

	vncClientList::iterator i;
	// Post this update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		if (!GetClient(*i)->client_settings_passed) return false;

	}
	return true;

CATCH_THROW("vncServer::All_clients_initialalized")
}


bool vncServer::IsClient(vncClient* pClient)
{
TRY_CATCH

  vncClientList::iterator i;
  for (i = m_authClients.begin(); i != m_authClients.end(); i++)
    if (GetClient(*i) == pClient) return true;
  return false;

CATCH_THROW("vncServer::IsClient")
}

/// @modified Alexander Novak @date 05.11.2007 Added the method to support layered windows clipping
void vncServer::HideLayeredWindow(HWND hwnd, bool showWindow)
{
TRY_CATCH

	if (!m_desktop)
		throw MCException("Desktop wasn't initialized");
	
	if ( GetWindowLongPtr(hwnd,GWL_EXSTYLE) & WS_EX_LAYERED )
		
		if (showWindow)
			m_desktop->RemoveFromHiddenLayeredWindowList(hwnd);
		else
			m_desktop->AddToHiddenLayeredWindowList(hwnd);
		
	else
		throw MCException("Invalid window's handle, or window isn't layered");

CATCH_THROW()
}

void vncServer::SetHideWallpaper(const bool hideWallpaper)
{
TRY_CATCH
	if(1 == m_authClients.size())
		ChangeWallpaperState(hideWallpaper);
CATCH_THROW()
}

void vncServer::ChangeWallpaperState(const bool hideWallpaper)
{
TRY_CATCH
	if(m_fWallpaperHidden == hideWallpaper)
		return;
	m_fWallpaperHidden = hideWallpaper;
	if(NULL != m_srvCommunicator.get())
	{
		SHideWallpaper msg;
		msg.hide = m_fWallpaperHidden;
		//msg.pid = GetCurrentProcessId();
		msg.pid = CProcessSingleton<COriginalIEPIDWrapper>::instance().GetPid();
		m_srvCommunicator->SendMsg(SRVCOMM_RESET_WALLPAPER, reinterpret_cast<char*>(&msg), sizeof(msg));
	}
CATCH_THROW()
}

