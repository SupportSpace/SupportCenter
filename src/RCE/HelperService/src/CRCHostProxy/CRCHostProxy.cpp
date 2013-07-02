/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRCHostProxy.cpp
///
///  Proxy application helper for RCHost. Capture/Input for winlogon, and so on
///
///  @author "Archer Software" Sogin M. @date 01.10.2007
///
////////////////////////////////////////////////////////////////////////


#include "..\stdafx.h"
#include "CRCHostProxy.h"
#include <boost/bind.hpp>
#include <AidLib/CScopedTracker/CScopedTracker.h>
#include <AidLib/CThread/CThreadLs.h>
#include <atlbase.h>

typedef boost::shared_ptr<boost::remove_pointer<HANDLE>::type> SPHandle;
typedef struct _SBrokerThreadEntryData
{
	char* buf;
	HANDLE clientProcess;
} SBrokerThreadEntryData;

unsigned int __stdcall CreateBrokerThreadEntry(void *data)
{
TRY_CATCH

	CoInitialize(0);

	SBrokerThreadEntryData* inData = reinterpret_cast<SBrokerThreadEntryData*>(data);
	SStartBroker *startBroker = reinterpret_cast<SStartBroker*>(inData->buf);
	if (NULL == startBroker->buf || 0 == startBroker->bufSize)
		throw MCException("NULL == startBroker->buf || 0 == startBroker->bufSize");

	CComPtr<IDispatch> broker;
	HRESULT hr;
	if((hr=broker.CoCreateInstance(L"Broker.CoBroker",NULL,CLSCTX_LOCAL_SERVER))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("CoBroker creation failed")),hr);

	CScopedTracker<HGLOBAL> globalMem;	
	globalMem.reset(GlobalAlloc(GMEM_MOVEABLE, startBroker->bufSize), GlobalFree);
	if (NULL == globalMem)
		throw MCException_Win("Failed to GlobalAlloc");

	CComPtr<IStream> stream;
	hr = CreateStreamOnHGlobal(globalMem, FALSE, &stream);
	if (S_OK != hr)
		throw MCException_Win(Format(_T("Failed to GlobalAlloc; result = %X"),hr));

	ULARGE_INTEGER size;
	size.QuadPart = startBroker->bufSize;
	hr = stream->SetSize(size);
	if (S_OK != hr)
		throw MCException_Win(Format(_T("Failed to stream->SetSize; result = %X"),hr));

	hr = CoMarshalInterface(stream, IID_IDispatch, broker, MSHCTX_LOCAL, NULL, MSHLFLAGS_NORMAL);
	if (S_OK != hr)
		throw MCException_Win(Format(_T("Failed to CoMarshalInterface; result = %X"),hr));

	ULARGE_INTEGER uLi;
	LARGE_INTEGER li;
	li.QuadPart = 0;

	/// Writing stream to client process memory
	stream->Seek(li, STREAM_SEEK_SET, NULL);
	boost::scoped_array<char> localBuf;
	ULONG readCount;
	localBuf.reset(new char[startBroker->bufSize]);
	hr = stream->Read(localBuf.get(), startBroker->bufSize, &readCount);
	if (S_OK != hr)
		throw MCException_Win(Format(_T("stream->Read; result = %X"),hr));

	/// Writing stream date into client process memory
	ULONG writtenCount;
	if (FALSE == WriteProcessMemory(inData->clientProcess, startBroker->buf, localBuf.get(), readCount, &writtenCount))
		throw MCException_Win("Failed to WriteProcessMemory ");

	CoUninitialize();

	Log.Add(_MESSAGE_,_T("Broker created and marshaled to BrokerProxy process"));

	return TRUE;

CATCH_LOG()

	CoUninitialize();
	return FALSE;
}

CRCHostProxy::CRCHostProxy()
	:	CInstanceTracker(_T("CRCHostProxy")),
		m_instance(NULL),
		m_unSetHooks(NULL),
		m_setMouseFilterHook(NULL),
		m_setKeyboardFilterHook(NULL),
		m_setHooks(NULL),
		m_hookinited(false),
		CThread()
{
TRY_CATCH

	/// Registering communication message handlers
	m_srvCommutator.RegisterMsgHandler(SRVCOMM_SEND_MOUSE_INPUT, boost::bind(&CRCHostProxy::OnSendMouseEventMsg, this, _1, _2, _3));
	m_srvCommutator.RegisterMsgHandler(SRVCOMM_SEND_KBD_INPUT, boost::bind(&CRCHostProxy::OnSendKbdEventMsg, this, _1, _2, _3));
	m_srvCommutator.RegisterMsgHandler(SRVCOMM_SET_VNCHOOKS, boost::bind(&CRCHostProxy::OnSetVNCHooksMsg, this, _1, _2, _3));
	m_srvCommutator.RegisterMsgHandler(SRVCOMM_GET_DESKTOP, boost::bind(&CRCHostProxy::OnGetWinLogonDesktopMsg, this, _1, _2, _3));
	m_srvCommutator.RegisterMsgHandler(SRVCOMM_SEND_CAD, boost::bind(&CRCHostProxy::OnSendCAD, this, _1, _2, _3));
	m_srvCommutator.RegisterMsgHandler(SRVCOMM_RESET_WALLPAPER, boost::bind(&CRCHostProxy::OnResetWallpaper, this, _1, _2, _3));
	m_srvCommutator.RegisterMsgHandler(SRVCOMM_TOGGLE_TASKBAR, boost::bind(&CRCHostProxy::OnToggleTaskBar, this, _1, _2, _3));
	m_srvCommutator.RegisterMsgHandler(SRVCOMM_ALLOW_CONNECTIONS, boost::bind(&CRCHostProxy::OnAllowConnections, this, _1, _2, _3));
	m_srvCommutator.RegisterMsgHandler(SRVCOMM_START_BROKER, boost::bind(&CRCHostProxy::OnStartBroker, this, _1, _2, _3));


	OSVERSIONINFOEX osInfo;
	osInfo.dwOSVersionInfoSize=sizeof(osInfo);
	int osVersion = 0;
	if(0 == GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&osInfo)))
	{
		osVersion = 5; //WinXP by default
		Log.WinError(_ERROR_,_T("Failed to GetVersionEx"));
	} else
	{
		osVersion = osInfo.dwMajorVersion;
	}
	/// Allowing lower integrity applications to communicate with proxy for Vista and higher
	if (osVersion > 5)
	{
		TRY_CATCH
			CScopedTracker<HMODULE> user32Module;
			user32Module.reset(LoadLibrary(_T("User32.dll")),FreeLibrary);
			if (NULL == user32Module)
				throw MCException_Win("NULL == user32Module");
			typedef BOOL (WINAPI* pChangeWindowMessageFilter)(UINT message, DWORD dwFlag);
			pChangeWindowMessageFilter ChangeWindowMessageFilter = reinterpret_cast<pChangeWindowMessageFilter>(GetProcAddress(user32Module, _T("ChangeWindowMessageFilter")));
			if (NULL == ChangeWindowMessageFilter)
				throw MCException_Win("NULL == ChangeWindowMessageFilter");
			if (FALSE == ChangeWindowMessageFilter(m_srvCommutator.m_commMsg, 1))
				throw MCException_Win("Failed to ChangeWindowMessageFilter for m_srvCommutator.m_commMsg");
		CATCH_LOG()
	}

	/// Preparing to set vncHooks
	m_vncHooks.reset(NULL, CloseHandle);
	LoadVNCHooks();

CATCH_THROW()
}

void CRCHostProxy::LoadVNCHooks()
{
TRY_CATCH
	if (NULL != m_vncHooks.get())
		return; //Already loaded
	tstring fileName = Format(_T("%s\\vnchooks.dll"),GetModulePath(NULL).c_str());
	m_vncHooks.reset(LoadLibrary(fileName.c_str()),FreeLibrary);
	if (NULL != m_vncHooks)
	{
		m_unSetHooks = (UnSetHooksFn) GetProcAddress( m_vncHooks, _T("UnSetHooks"));
		m_setMouseFilterHook  = (SetMouseFilterHookFn) GetProcAddress( m_vncHooks, _T("SetMouseFilterHook"));
		m_setKeyboardFilterHook  = (SetKeyboardFilterHookFn) GetProcAddress( m_vncHooks, _T("SetKeyboardFilterHook"));
		m_setHooks  = (SetHooksFn) GetProcAddress( m_vncHooks, _T("SetHooks"));
	} else
		throw MCException_Win(Format(_T("Failed to load %s"),fileName.c_str()));
	Log.Add(_MESSAGE_,_T("VNCHooks loaded"));
CATCH_LOG()
}

CRCHostProxy::~CRCHostProxy()
{
TRY_CATCH
	TRY_CATCH
		/// Unsetting vncHooks
		if (m_unSetHooks) // Always try to unset hooks, to unlock vnchooks.dll,
						  //no matter if it was done by host before
		{
			Log.Add(_MESSAGE_,_T("Reseting vncHooks..."));
			if(!m_unSetHooks(m_hooksTid))
				Log.Add(_ERROR_,"Failed to reset vncHooks ");
			else
				Log.Add(_MESSAGE_,_T("vncHooks reseted"));
		}
	CATCH_LOG()
	Terminate();
	PostThreadMessage(GetTid(),WM_QUIT,0,0);
CATCH_LOG()
}

void CRCHostProxy::Execute(void*)
{
TRY_CATCH
	SET_THREAD_LS;

	if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL))
		Log.WinError(_WARNING_,_T("Failed to increase priority of client thread"));

	///
	/// This thread used for input injection to winlogon desktop
	/// since we cannot perform SetDesktopThread for thread with window
	///
	MSG msg;
	boost::shared_ptr<boost::remove_pointer<HDESK>::type> desktop;
	while(!Terminated())
	{
		WaitMessage();
		while (FALSE != PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			switch(msg.message)
			{
				case WM_USER + SRVCOMM_SEND_MOUSE_INPUT:
				{
					boost::scoped_ptr<SSendMouseEventMsg> mouseEvent; 
					mouseEvent.reset(reinterpret_cast<SSendMouseEventMsg*>(msg.lParam));
					if (mouseEvent->useSendInput)
						SendInput(1, &mouseEvent->evt, sizeof(mouseEvent->evt));
					else
						mouse_event(mouseEvent->flags, mouseEvent->x, mouseEvent->y, mouseEvent->wheelMovement, mouseEvent->extraInfo);					
					break;
				}
				case WM_USER + SRVCOMM_SEND_KBD_INPUT:
				{
					boost::scoped_ptr<SSendKeyEventMsg> keyEvent;
					keyEvent.reset(reinterpret_cast<SSendKeyEventMsg*>(msg.lParam));
					keybd_event(keyEvent->vkCode, MapVirtualKey(keyEvent->vkCode, 0), keyEvent->flags, keyEvent->extraInfo);
					break;
				}
				case WM_USER + SRVCOMM_GET_DESKTOP:
				{
					desktop = GetWinlogonDesktop();
					if (FALSE == SetThreadDesktop(desktop.get()))
						Log.WinError(_ERROR_,_T("Failed to SetThreadDesktop"));
					break;
				}
				case WM_USER + SRVCOMM_SEND_CAD:
				{
					PostMessage(HWND_BROADCAST, WM_HOTKEY, 0, MAKELONG(MOD_ALT | MOD_CONTROL, VK_DELETE));
					break;
				}
			}
		}
	}
CATCH_LOG()
}


DWORD CRCHostProxy::OnStartBroker(WORD code, char* msg, const WORD msgSize)
{
TRY_CATCH

	HRESULT hr;
	SBrokerThreadEntryData inData;
	inData.clientProcess = m_srvCommutator.GetClientProcess().get();
	inData.buf = msg;
	SPHandle thread;
	thread.reset((HANDLE)(_beginthreadex( 0 , 0 , &CreateBrokerThreadEntry, &inData , 0 , 0)),CloseHandle);
	
	if (NULL == thread.get())
		throw MCException_Win("Failed to _beginthreadex ");
	
	if (WAIT_OBJECT_0 != WaitForSingleObject(thread.get(), INFINITE))
		throw MCException_Win("Failed to WaitForSingleObject ");

	DWORD result = FALSE;
	GetExitCodeThread(thread.get(), &result);
	return result;

CATCH_THROW()
}

DWORD CRCHostProxy::OnToggleTaskBar(WORD code, char* msg, const WORD msgSize)
{
TRY_CATCH
	SToggleTaskbar *toggleTaskBar = reinterpret_cast<SToggleTaskbar*>(msg);
	m_toggleTaskBar.ToggleTaskBar(toggleTaskBar->show);
	return TRUE;
CATCH_THROW()
}

DWORD CRCHostProxy::OnResetWallpaper(WORD code, char* msg, const WORD msgSize)
{
TRY_CATCH
	SHideWallpaper *hideWall = reinterpret_cast<SHideWallpaper*>(msg);
	if (hideWall->hide)
		m_wallpaperSwitch.HideWallpaperRequest(hideWall->pid);
	else
		m_wallpaperSwitch.RestoreWallpaperRequest(hideWall->pid);
	return TRUE;
CATCH_THROW()
}

DWORD CRCHostProxy::OnSendCAD(WORD code, char* msg, const WORD msgSize)
{
TRY_CATCH
	PostThreadMessage(GetTid(), WM_USER + SRVCOMM_GET_DESKTOP, 0, 0);
	return PostThreadMessage(GetTid(), WM_USER + SRVCOMM_SEND_CAD, 0, 0);
CATCH_THROW()
}

DWORD CRCHostProxy::OnSendMouseEventMsg(WORD code, char* msg, const WORD msgSize)
{
TRY_CATCH
	if (NULL == msg)
	{
		Log.Add(_ERROR_,_T("NULL == msg"));
		return FALSE;
	}
	SSendMouseEventMsg* mouseEvent =  reinterpret_cast<SSendMouseEventMsg*>(msg);
	if (mouseEvent->toWinlogon)
	{
		SSendMouseEventMsg *_mouseEvent = new SSendMouseEventMsg;
		*_mouseEvent = *mouseEvent;
		if (FALSE == PostThreadMessage(GetTid(), WM_USER + SRVCOMM_SEND_MOUSE_INPUT, 0, reinterpret_cast<LPARAM>(_mouseEvent)))
		{
			Log.WinError(_ERROR_,_T("Failed to PostThreadMessage "));
			delete _mouseEvent;
		}
	}
	else
	{
		if (mouseEvent->useSendInput)
			SendInput(1, &mouseEvent->evt, sizeof(mouseEvent->evt));
		else
			mouse_event(mouseEvent->flags, mouseEvent->x, mouseEvent->y, mouseEvent->wheelMovement, mouseEvent->extraInfo);					
	}
	return TRUE;
CATCH_THROW()
}

DWORD CRCHostProxy::OnSendKbdEventMsg(WORD code, char* msg, const WORD msgSize)
{
TRY_CATCH
	if (NULL == msg)
	{
		Log.Add(_ERROR_,_T("NULL == msg"));
		return FALSE;
	}
	SSendKeyEventMsg* keyEvent = reinterpret_cast<SSendKeyEventMsg*>(msg);
	if (keyEvent->toWinlogon)
	{
		SSendKeyEventMsg *_keyEvent = new SSendKeyEventMsg;
		*_keyEvent = *keyEvent;
		if (FALSE == PostThreadMessage(GetTid(), WM_USER + SRVCOMM_SEND_KBD_INPUT, 0, reinterpret_cast<LPARAM>(_keyEvent)))
		{
			Log.WinError(_ERROR_,_T("Failed to PostThreadMessage "));
			delete _keyEvent;
		}
	} else
		keybd_event(keyEvent->vkCode, MapVirtualKey(keyEvent->vkCode, 0), keyEvent->flags, keyEvent->extraInfo);
	return TRUE;
CATCH_THROW()
}

DWORD CRCHostProxy::OnSetVNCHooksMsg(WORD code, char* msg, const WORD msgSize)
{
TRY_CATCH
	if (NULL == msg)
	{
		Log.Add(_ERROR_,_T("NULL == msg"));
		return m_hookinited;
	}
	SSetVNCHooksMsg* setHooksMsg = reinterpret_cast<SSetVNCHooksMsg*>(msg);

	if (setHooksMsg->start)
	{
		LoadVNCHooks();
		m_hooksTid = setHooksMsg->tid;
		if (m_setHooks)
		{
			if (!m_setHooks(	setHooksMsg->tid,
								setHooksMsg->rfbScreenUpdate,
								setHooksMsg->rfbCopyRectUpdate,
								setHooksMsg->rfbMouseUpdate, 
								setHooksMsg->ddihook ))
			{
				Log.Add(_ERROR_,"failed to set system hooks");
				m_hookinited = false;
			} 
			else 
			{
				m_hookinited = true;
			}
		}
		// Start up the keyboard and mouse filters
		if (m_setKeyboardFilterHook) 
			m_setKeyboardFilterHook(setHooksMsg->localInputsDisabled);
		if (m_setMouseFilterHook)
			m_setMouseFilterHook(setHooksMsg->localInputsDisabled);
	}
	else if (m_hookinited)
	{
		if (m_unSetHooks)
		{
			if(!m_unSetHooks(setHooksMsg->tid))
				Log.Add(_ERROR_,"Unsethooks Failed");
			else
				m_hookinited = false;
		}
	}
CATCH_LOG()
	return m_hookinited;
}

boost::shared_ptr<boost::remove_pointer<HDESK>::type> CRCHostProxy::GetWinlogonDesktop()
{
TRY_CATCH
	CScopedTracker<HWINSTA> windowStation;
	windowStation.reset(OpenWindowStation(_T("winsta0"), FALSE,
										WINSTA_ACCESSCLIPBOARD   |
										WINSTA_ACCESSGLOBALATOMS |
										WINSTA_CREATEDESKTOP     |
										WINSTA_ENUMDESKTOPS      |
										WINSTA_ENUMERATE         |
										WINSTA_EXITWINDOWS       |
										WINSTA_READATTRIBUTES    |
										WINSTA_READSCREEN        |
										WINSTA_WRITEATTRIBUTES),
						CloseWindowStation);

	if (NULL == windowStation)
		throw MCException_Win("Failed to OpenWindowStation ");

	// Set the windowstation to be winsta0
	HWINSTA oldWindowStation = GetProcessWindowStation();

	if (!SetProcessWindowStation(windowStation))
		throw MCException_Win("Failed to SetProcessWindowStation ");

	// Getting winlogon desktop on winsta0
	boost::shared_ptr<boost::remove_pointer<HDESK>::type> desktop;
	desktop.reset(OpenDesktop(_T("winlogon"), 0, FALSE,
								  DESKTOP_CREATEMENU |
								  DESKTOP_CREATEWINDOW |
								  DESKTOP_ENUMERATE    |
								  DESKTOP_HOOKCONTROL  |
								  DESKTOP_JOURNALPLAYBACK |
								  DESKTOP_JOURNALRECORD |
								  DESKTOP_READOBJECTS |
								  DESKTOP_SWITCHDESKTOP |
								  DESKTOP_WRITEOBJECTS),
					CloseDesktop);
	DWORD lastError = GetLastError();
	SetProcessWindowStation(oldWindowStation);
	if (NULL == desktop.get())
	{
		SetLastError(lastError);
		throw MCException_Win("Failed to OpenDesktop ");
	}

	return desktop;

CATCH_THROW()
}

DWORD CRCHostProxy::OnGetWinLogonDesktopMsg(WORD code, char* msg, const WORD msgSize)
{
TRY_CATCH

	boost::shared_ptr<boost::remove_pointer<HDESK>::type> desktop = GetWinlogonDesktop();

	HANDLE desktopForClient;
	if (0 == DuplicateHandle(GetCurrentProcess(), desktop.get(), m_srvCommutator.GetClientProcess().get(), &desktopForClient, 0, FALSE, GENERIC_READ | DESKTOP_SWITCHDESKTOP ))
		throw MCException_Win("Failed to duplicate handle ");

	if (FALSE == PostThreadMessage(GetTid(), WM_USER + SRVCOMM_GET_DESKTOP, 0, 0))
		Log.WinError(_ERROR_,_T("Failed to PostThreadMessage "));

	return reinterpret_cast<DWORD>(desktopForClient);

CATCH_THROW()
}

int CRCHostProxy::Shutdown(int threadId)
{
TRY_CATCH
	if (0 == PostThreadMessage(threadId, WM_QUIT, 0, 0))
	{
		Log.WinError(_WARNING_,_T("Failed to send WM_QUIT to proxy object. Terminating forcedly. Pid(%d) "),threadId);
		return 1;
	}
	return 0;
CATCH_THROW()
}

int CRCHostProxy::Run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
TRY_CATCH

	/// Checking if there is already started proxy for session
	if (NULL != FindWindow(DEF_WND_CLS_NAME ,NULL))
		throw MCException("Already running for that session");

	if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL))
		Log.WinError(_WARNING_,_T("Failed to increase priority of client thread"));

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, m_title, MAX_PATH);
	//LoadString(hInstance, IDC_HELPERSERVICE, m_windowClass, MAX_PATH);
	_tcscpy_s(m_windowClass, MAX_PATH, DEF_WND_CLS_NAME);
	RegisterClass(hInstance);

	// Perform application initialization:
	InitInstance (hInstance, SW_HIDE /*nCmdShow*/);

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HELPERSERVICE));

	/// Starting internal thread
	Start();

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
CATCH_LOG()
	return FALSE;
}

ATOM CRCHostProxy::RegisterClass(HINSTANCE hInstance)
{
TRY_CATCH

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_HELPERSERVICE);
	wcex.lpszClassName	= m_windowClass;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);

CATCH_THROW()
}

void CRCHostProxy::InitInstance(HINSTANCE instance, int cmdShow)
{
TRY_CATCH

	HWND hWnd;
	m_instance = instance; // Store instance handle in our global variable
	hWnd = CreateWindow(m_windowClass, m_title, WS_OVERLAPPEDWINDOW,
						CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, instance, NULL);

	if (!hWnd)
		throw MCException_Win("Failed to CreateWindow");

	ShowWindow(hWnd, cmdShow);
	UpdateWindow(hWnd);

CATCH_THROW()
}

LRESULT CALLBACK CRCHostProxy::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
TRY_CATCH

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	if (RCHOST_PROXY_INSTANCE.m_srvCommutator.m_commMsg == message)
	{
		return RCHOST_PROXY_INSTANCE.m_srvCommutator.OnMsgReceived(wParam, lParam);
	}
	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(RCHOST_PROXY_INSTANCE.m_instance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;

CATCH_THROW()
}

INT_PTR CALLBACK CRCHostProxy::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
TRY_CATCH

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;

CATCH_THROW()
}

DWORD CRCHostProxy::OnAllowConnections(WORD code, char* msg, const WORD msgSize)
{
TRY_CATCH
	SAllowConnections* allow = reinterpret_cast<SAllowConnections*>(msg);
	if (allow->enabled)
		m_firewallRelaxator.AllowIncomingConnections(allow->pid);
	else
		m_firewallRelaxator.RestoreFirewallSettings(allow->pid);
	return 0;
CATCH_THROW()
}

