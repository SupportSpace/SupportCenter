// FTestQualityClient.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "FTestQualityClient.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CSingleton/CSingleton.h>
#include <NWL/Streaming/CSocketSystem.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/scoped_ptr.hpp>
#include "CSettings.h"
#include "CHelper.h"
#include "CQualityClient.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND hWnd;
CSocketSystem sockSystem;

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
	Log.RegisterLog(new cFileLog());

TRY_CATCH

	ParseCommandLine();

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_FTESTQUALITYCLIENT, szWindowClass, MAX_LOADSTRING);
	RegisterWndClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	SETTINGS_INSTANCE.SetWindow(hWnd);
	QUALITYCLIENT_INSTANCE;

	if(SETTINGS_INSTANCE.GetClientAutoStart())
		QUALITYCLIENT_INSTANCE.StartViewer();

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FTESTQUALITYCLIENT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_FTESTQUALITYCLIENT);
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

   hWnd = CreateWindow(
		szWindowClass, 
		szTitle, 
		WS_OVERLAPPEDWINDOW,
		GetSystemMetrics(SM_CXSCREEN)/2, 
		0, 
		GetSystemMetrics(SM_CXSCREEN)/2,
		GetSystemMetrics(SM_CYSCREEN)/2,
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

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_CONNECT:
			QUALITYCLIENT_INSTANCE.StartViewer();
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
		if(!paramName.compare(_T("-A=")))
		{
			SETTINGS_INSTANCE.SetRemoteAddr(param);
		}
		else
		{
			if(!paramName.compare(_T("-P=")))
			{
				SETTINGS_INSTANCE.SetRemotePort(atoi(param.c_str()));
			}
			else
			{
				if(!paramName.compare(_T("-AS")))
				{
					SETTINGS_INSTANCE.SetClientAutoStart(true);
				}
				else
				{
					if(!paramName.compare(_T("-M=")))
					{
						SETTINGS_INSTANCE.SetDisplayMode(atoi(param.c_str()));
					}
					else
					{
						if(!paramName.compare(_T("-T=")))
						{
							SETTINGS_INSTANCE.SetTestTime(atoi(param.c_str()));
						}
						else
						{
							if(!paramName.compare(_T("-I=")))
							{
								SETTINGS_INSTANCE.SetInvalidPixelPercentage(atoi(param.c_str()));
							}
							else
							{
								if(!paramName.compare(_T("-D=")))
								{
									SETTINGS_INSTANCE.SetColorDepth(atoi(param.c_str()));
								}
								else
								{
									if(!paramName.compare(_T("-F=")))
									{
										SETTINGS_INSTANCE.SetFrameFile(param);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return;

CATCH_THROW()
}
