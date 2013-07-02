/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRCHost.cpp
///
///  desctop capturing interface
///
///  @author "Archer Software" Sogin M. @date 20.09.2006
///
////////////////////////////////////////////////////////////////////////
#include <winsock2.h>
#include "CRCHost.h"
#define LOCALIZATION_MESSAGES
#include "RCE_resource.h"
#include "vncserver.h"
#include <exception>
#include "CVisualPointer.h"
#include "CShadowedClient.h"
#include <AidLib/CThread/ThreadUtil.h>
#include <AidLib/Utils/Utils.h>

#pragma warning( disable: 4996 )//<strfunc> was declared deprecated

// gloabal vnc native variables
// Application instance and name
HINSTANCE	hAppInstance;
const char	*szAppName = "RCEngine";
DWORD		mainthreadId;

bool CRCHost::globalStuffInited = false;

CRCHost::CRCHost()
	:	CInstanceTracker(_T("RCHost")),
		m_vncServer(NULL),
		m_hProtectedWindow(NULL),
		m_shadowStreamClientId(-1)
{
TRY_CATCH

	if (!globalStuffInited)
	{
		/// initializing global vnc structures
		globalStuffInited = true;
		HINSTANCE hInstance = NULL;
		setbuf(stderr, 0);

		// Save the application instance and main thread id
		hAppInstance = hInstance;
		mainthreadId = GetCurrentThreadId();
	}

	/// Configuring server
	m_vncServer.reset(new vncServer(this));
	m_vncServer->SetName("RCE");
	m_vncServer->CaptureAlphaBlending(false); // sf@2005
	m_vncServer->BlackAlphaBlending(false); // sf@2005
	m_vncServer->BlankMonitorEnabled(false);
	m_vncServer->SetDefaultScale(1/*m_pref_DefaultScale*/);
	m_vncServer->SetAutoIdleDisconnectTimeout(100000);
	//m_vncServer->EnableRemoveWallpaper(false);
	/// Enabling videodriver
	m_vncServer->Driver(true);
	
CATCH_THROW("CRCHost::CRCHost")
}

CRCHost::~CRCHost()
{
TRY_CATCH

	SetProtectedWindow(NULL);

CATCH_LOG("CRCHost::~CRCHost")
}

int CRCHost::StartClient(boost::shared_ptr<CAbstractStream> stream, const int priority)
{
TRY_CATCH

	int id = m_vncServer->AddClient(stream,false,0,NULL);
	if (id == -1) 
		throw MCException("failed to m_vncServer->AddClient");
	return id;

CATCH_THROW("CRCHost::StartClient")
}

void CRCHost::StopClient(const int clientId)
{
TRY_CATCH

	vncClient *client = m_vncServer->GetClient(clientId);
	if (!client)
		throw MCException(Format("Client id(%d) not found",clientId));
	client->SetStopReason(LOCAL_STOP);
	client->SendStop();
	m_vncServer->KillClient(clientId);

CATCH_THROW("CRCHost::StopClient")
}

HWND g_hWnd;
BOOL CALLBACK EnumWindowProc(HWND hWnd, LPARAM lParam)
{
	DWORD dwProcessID = 0;
	GetWindowThreadProcessId(hWnd, &dwProcessID);
	if (dwProcessID != (DWORD)lParam)
	return TRUE;
	if (GetParent(hWnd) == NULL)
	{
		g_hWnd = hWnd;
		return FALSE;
	}
	return TRUE;
}

void CRCHost::SetProtectedProcess(const DWORD processId)
{
TRY_CATCH

	m_protectedThreads = GetProcessThreads(processId);
	FilterProtectedThreads(m_protectedThreads);
	for(std::set<DWORD>::const_iterator thread = m_protectedThreads.begin();
		thread != m_protectedThreads.end();
		++thread)
	{
		boost::shared_ptr<boost::remove_pointer<HANDLE>::type> hook;
		hook.reset(SetWindowsHookEx(WH_MOUSE, (HOOKPROC)MouseHook, processId==GetCurrentProcessId()?0:GetCurrentModule() /*hInstance*/, *thread),UnhookWindowsHookEx);
		if (NULL == hook)
			Log.WinError(_ERROR_,_T("Failed to SetWindowHookEx for proc(%d) thread(%d)"),processId, *thread);
		else
			m_mouseHooks.push_back(hook);
	}

	/// Retriwing process main window
	g_hWnd = NULL;
	EnumWindows((WNDENUMPROC)EnumWindowProc, (LPARAM)processId);
	m_hProtectedWindow = g_hWnd;
	if (NULL == m_hProtectedWindow)
		Log.WinError(_ERROR_,_T("Failed to get main app window. Keyboard input injection still can occur to process %d"),processId);
	
CATCH_THROW()
}

void CRCHost::SetProtectedWindow(HANDLE hWnd)
{
TRY_CATCH

	m_mouseHooks.clear();
	m_hProtectedWindow = hWnd;
	if (hWnd == NULL) 
		return;
	DWORD processId;
	GetWindowThreadProcessId(static_cast<HWND>(m_hProtectedWindow),&processId); //No error handling since no errors mentioned in MSDN, donno how to detect, by 0 returned?
	
	m_protectedThreads = GetProcessThreads(processId);
	FilterProtectedThreads(m_protectedThreads);

	for(std::set<DWORD>::const_iterator thread = m_protectedThreads.begin();
		thread != m_protectedThreads.end();
		++thread)
	{
		boost::shared_ptr<boost::remove_pointer<HANDLE>::type> hook;
		hook.reset(SetWindowsHookEx(WH_MOUSE, (HOOKPROC)MouseHook, processId==GetCurrentProcessId()?0:GetCurrentModule() /*hInstance*/, *thread),UnhookWindowsHookEx);
		if (NULL == hook)
			Log.WinError(_ERROR_,_T("Failed to SetWindowHookEx for proc(%d) thread(%d)"),processId, *thread);
		else
			m_mouseHooks.push_back(hook);
	}

CATCH_THROW("CRCHost::SetProtectedWindow")
}

LRESULT CRCHost::MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
TRY_CATCH

	if (nCode < 0)
		return CallNextHookEx(NULL /*anyway ignored*/,nCode,wParam,lParam);
	if (lParam != NULL)
	{
		MOUSEHOOKSTRUCT* mh = reinterpret_cast<MOUSEHOOKSTRUCT*>(lParam);
		if (mh->dwExtraInfo == MESS_MARKER && 
			mh->wHitTestCode != HTMINBUTTON &&
			mh->wHitTestCode != HTCAPTION &&
			wParam != WM_RBUTTONUP &&
			wParam != WM_LBUTTONUP
			) 
			return TRUE;
	}
	return CallNextHookEx(NULL /*anyway ignored*/,nCode,wParam,lParam);

CATCH_THROW("CRCHost::MouseHook")
}

void CRCHost::SetSessionMode(vncClient* client, const ESessionMode mode, const bool state)
{
TRY_CATCH

	char code = 0;
	switch(mode)
	{
		case VIEW_ONLY:
			client->EnableKeyboard(!state);
			client->EnablePointer(!state);
			code = rfbSetViewOnly;
			break;
		case VISUAL_POINTER:
			client->EnableVisualPointer(state);
			code = rfbSetVisualPointer;
			break;
		default:
			throw MCException(Format("Unknown mode %d",mode));
	}
	TRY_CATCH //TODO: or throw exception outside?
	///Checking if we have even one client with viewonly mode and visual pointer

	if (m_vncServer->HasVisualPointerClient())
	{
		VISUAL_POINTER_INSTANCE.Show();
	}
	else
	{
		VISUAL_POINTER_INSTANCE.Hide();
	}
	CATCH_LOG("CRCHost::SetSessionMode Failed to set visual pointer")

	/// Updating viewer
	InterlockedExchange(&client->m_modeSendPending, TRUE);
	/*if (this->m_vncServer->All_clients_initialalized())
	{
		client->DisableProtocol();
		char buf[2];
		buf[0] = code;
		buf[1] = static_cast<char>(state);
		client->m_stream->Send(buf, 2);
		client->EnableProtocol();
	} else
	{
		Log.Add(_ERROR_,_T("Not all clients inited while setting session mode. Session mode on viewer side will not be affected"));
	}*/

CATCH_THROW("CRCHost::SetSessionMode")
}

bool CRCHost::GetSessionMode(const int clientId, const ESessionMode mode)
{
TRY_CATCH

	vncClient* client = m_vncServer->GetClient(clientId);
	if (!client) throw MCException(Format("client with id(%d) not found",clientId).c_str());
	switch(mode)
	{
		case VIEW_ONLY:
			return !client->KeyboardEnabled();
		case VISUAL_POINTER:
			return client->VisualPointerEnabled();
	}
	throw MCException(Format("Unknown mode %d",mode));
CATCH_THROW("CRCHost::GetSessionMode")
}

void CRCHost::SetSessionMode(const int clientId, const ESessionMode mode, const bool state)
{
TRY_CATCH
	
	vncClient* client = m_vncServer->GetClient(clientId);
	if (!client) throw MCException(Format("client with id(%d) not found",clientId).c_str());
	SetSessionMode(client, mode, state);

CATCH_THROW("CRCHost::SetSessionMode")
}

void CRCHost::HideVisualPointer()
{
TRY_CATCH
	VISUAL_POINTER_INSTANCE.Hide();
CATCH_LOG("CRCHost::HideVisualPointer Failed to hide visual pointer")
}

void CRCHost::ShowVisualPointer()
{
TRY_CATCH
	VISUAL_POINTER_INSTANCE.Show();
CATCH_LOG("CRCHost::ShowVisualPointer Failed to show visual pointer")
}

void CRCHost::SetShadowStream(boost::shared_ptr<CAbstractStream> stream)
{
TRY_CATCH
	if (stream.get())
	{
		if (m_shadowStreamClientId >= 0)
		{
			//removing old client
			StopClient(m_shadowStreamClientId);
			m_shadowStreamClientId = -1;
		}
		//Setting shadow stream
		m_shadowStreamClientId = m_vncServer->AddClient(boost::shared_ptr<CAbstractStream>(new CShadowedClient(stream)),false,0,NULL);
		if (m_shadowStreamClientId < 0)
			throw MCException("failed to m_vncServer->AddClient");

	} else
	{
		//removing shadow stream
		if (m_shadowStreamClientId >= 0)
		{
			StopClient(m_shadowStreamClientId);
			m_shadowStreamClientId = -1;
		}
	}
CATCH_THROW("CRCHost::SetShadowStream")
}

void CRCHost::SetCaptureAlphaBlend(bool captureAlplhaBlend)
{
TRY_CATCH
	m_vncServer->CaptureAlphaBlending(captureAlplhaBlend);
	if (m_vncServer->GetDesktopPointer())
	{
		m_vncServer->GetDesktopPointer()->CaptureAlphaBlending(captureAlplhaBlend);
	}
CATCH_THROW("CRCHost::SetCaptureAlphaBlend")
}

bool CRCHost::GetCaptureAlphaBlend()
{
TRY_CATCH
	return m_vncServer->CaptureAlphaBlending();
CATCH_THROW("CRCHost::GetCaptureAlphaBlend")
}

boost::shared_ptr<CAbstractStream> CRCHost::GetClientStream(const int clientId)
{
TRY_CATCH
	if (m_vncServer.get() == NULL)
		throw MCException("m_vncServer.get() == NULL");
	vncClient *client = m_vncServer->GetClient(clientId);
	if (client == NULL)
		throw MCException("Client not found");
	return client->m_stream;
CATCH_THROW()
}

/// @modified Alexander Novak @date 05.11.2007 Added the method to support layered windows clipping
void CRCHost::HideLayeredWindow(HWND hwnd, bool showWindow)
{
TRY_CATCH

	m_vncServer->HideLayeredWindow(hwnd, showWindow);

CATCH_THROW()
}

unsigned int CRCHost::GetClientsCount()
{
TRY_CATCH
	return m_vncServer->AuthClientCount();
CATCH_THROW()
}