// Communicator.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "resource.h"
#include "Communicator/Communicator.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

const TCHAR* g_RequestTemplate = 
   _T("<?xml version='1.0' encoding='UTF-8' ?> ")
   _T("<soapenv:Envelope xmlns:soapenv='http://schemas.xmlsoap.org/soap/envelope/' xmlns:q0='http://webservice.service.supportspace.com' xmlns:xsd='http://www.w3.org/2001/XMLSchema' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'>")
   _T("  <soapenv:Body>")
   _T("        <q0:getUserByRemoteToken>")
   _T("        <q0:in0>1234</q0:in0> ")
   _T("        </q0:getUserByRemoteToken>")
   _T("  </soapenv:Body>")
   _T("</soapenv:Envelope>");


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
CCommunicator *communicator=NULL;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_COMMUNICATOR, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COMMUNICATOR));

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
}



//
//  FUNCTION: MyRegisterClass()
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
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COMMUNICATOR));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_COMMUNICATOR);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
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
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   communicator= new CCommunicator();
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
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
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	tstring sServerName(_T("ws-kobi")); //Can be any https server address
	tstring sObjectName( _T("supportspaceweb/services/ActiveXTokenWS") );//there should be an object to send a verb
	WORD	dwPort = 8080;
	//tstring	sRequestTemplate; see global sRequestTemplate
	
	tstring sUserName(_T(""));//if required
	tstring sPassword(_T("")); //if required

    tstring			im_username = "sp4";
    tstring			im_resource = "IMClient";
	tstring			im_password = "1234";
	tstring			im_server = "ws-anatoly2";
	tstring			im_server_addr = "ws-anatoly2";
	bool			im_log = true;
	Presence		status = PresenceAvailable;
	unsigned int	im_keep_alive_timeout = 10;
	
	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
//IM
		case ID_IM_CREATE:
			communicator->CreateIMClient(
				im_username, im_resource, im_password, im_server, im_server_addr, hWnd , im_log, im_keep_alive_timeout);

			communicator->EnqueueExecutionQueue(new CNodeSendConnect());
			break;
		case ID_IM_DESTROY:
			communicator->DestroyIMClient();
			break;
		case ID_IM_CONNECT:
			communicator->EnqueueExecutionQueue(new CNodeSendConnect());
			break;
		case ID_IM_SENDMESSAGE:
			communicator->EnqueueExecutionQueue(new CNodeSendMessage("u1","Hello","World"));
			break;
		case ID_IM_UPDATE_STATUS:
			communicator->EnqueueExecutionQueue(new CNodeUpdateStatus(status));
			break;
		case ID_IM_DISCONNECT:
			communicator->EnqueueExecutionQueue(new CNodeSendDisconnect()) ;
			//communicator->DestroyIMClient();
			break;
//SOAP
		case ID_FILE_CREATEWEBCLIENT:
			OutputDebugString(_T("CreateWebClient"));
		    communicator->CreateWebClient(
						 sServerName,
						 sObjectName,
						 dwPort,
						 sUserName,
						 sPassword,
						 hWnd);
			break;
		case ID_FILE_DESTROYWEBCLIENT:
			OutputDebugString(_T("DestroyWebClient"));
			communicator->DestroyWebClient();
			break;
		case ID_FILE_SENDWEBCLIENTREQUEST:
			OutputDebugString(_T("SendWebClientRequest"));
			communicator->SendWebClientRequest(g_RequestTemplate);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_WC_THREAD_COMPLETED:
		//MessageBox( hWnd, _T("WM_WC_THREAD_COMPLETED"),_T("Notification"), MB_OK );  
		break;
	case WM_IM_CONNECTED:
		//MessageBox( hWnd, _T("WM_IM_CONNECTED"),_T("Notification"), MB_OK );  
		communicator->EnqueueExecutionQueue(new CNodeSendDisconnect());
		break;

	case WM_IM_NEWCALL:
		MessageBox( hWnd, _T("WM_IM_NEWCALL"),_T("Notification"), MB_OK );  
		break;

	case WM_IM_DISCONNECT:
		communicator->DestroyWebClient();
		//MessageBox( hWnd, _T("WM_IM_DISCONNECT"),_T("Notification"), MB_OK );  
		communicator->CreateIMClient(
				im_username, im_resource, im_password, im_server, im_server_addr, hWnd , im_log, im_keep_alive_timeout);

		communicator->EnqueueExecutionQueue(new CNodeSendConnect());

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
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
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
}

