// FTestResolutionServer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "FTestResolutionServer.h"
#include <NWL/Streaming/CSocketSystem.h>
#include <AidLib/CException/CException.h>
#include "CSettings.h"
#include <AidLib/CSingleton/CSingleton.h>
#include "CResolutionServer.h"
#include "CHelper.h"
#include "CResolutionManager.h"
#include <NetLog/CNetworkLog.h>

#define MAX_LOADSTRING 100

#pragma comment(lib, "NetLog.lib")

// Global Variables:
HINSTANCE g_hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
CSocketSystem sockSystem;
HWND hWnd;

// Forward declarations of functions included in this code module:
ATOM				RegisterWndClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void				ParseCommandLine();

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	try
	{
		Log.RegisterLog(new cFileLog());
		Log.RegisterLog(new CNetworkLog("FTestResolutionServer"));
	}
	catch(...)
	{
	}

TRY_CATCH

	ParseCommandLine();

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_FTESTRESOLUTIONSERVER, szWindowClass, MAX_LOADSTRING);
	RegisterWndClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	// Init CSettings
	SETTINGS_INSTANCE.SetWindow(hWnd);
	// Init CResolutionManager
	RESOLUTIONMANAGER_INSTANCE.Init();
	// Init CFrameRateServer
	RESOLUTIONSERVER_INSTANCE.Start();
	
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

CATCH_LOG()

	return 0;
}



//
//  FUNCTION: RegisterWndClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM RegisterWndClass(HINSTANCE hInstance)
{
TRY_CATCH

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FTESTRESOLUTIONSERVER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);

CATCH_THROW()
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
TRY_CATCH

   g_hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowEx(
		WS_EX_TOPMOST, 
		szWindowClass, 
		szTitle, 
		WS_POPUP,
		0, 
		0, 
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		NULL, 
		NULL, 
		hInstance, 
		NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;

CATCH_THROW()
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
TRY_CATCH

	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		RESOLUTIONSERVER_INSTANCE.DrawFrame();
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

CATCH_LOG()

	return 0;
}

void ParseCommandLine()
{
TRY_CATCH

	int nArgc = 0;
	// Extract arguments from command line
	PTCHAR* pArgs = CHelper::CommandLineToArgv(
			GetCommandLine(),
			&nArgc
		);

	if (!pArgs)
		return;

	// Create shared pointer for auto destroy pArgs handle
	boost::shared_ptr< boost::remove_pointer<HGLOBAL>::type > spArgs((HGLOBAL)pArgs, GlobalFree);

	tstring param(_T(""));
	while(*(++pArgs))
	{
		param.assign(*pArgs);
		tstring paramName = param.substr(0, 3);
		paramName = UpperCase(paramName);
		param.erase(0, 3);
		if(!paramName.compare(_T("-P=")))
		{
			SETTINGS_INSTANCE.SetLocalPort(atoi(param.c_str()));
		}
		else
		{
			if(!paramName.compare(_T("-T=")))
			{
				SETTINGS_INSTANCE.SetTestTime(atoi(param.c_str()));
			}
		}
	}

	return;

CATCH_THROW()
}
