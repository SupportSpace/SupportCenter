#include "StdAfx.h"
#include "AppSingelton.h"
#include "SupportMessenger.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"

extern CSupportMessengerApp theApp;

CAppSingelton::CAppSingelton(void)
:m_hAppSingeltonSemaphore(0),m_hAppSingeltonHelperThread(0),m_dwDThread(0),m_kShMem(NULL),m_hMapFile(0)
{
TRY_CATCH
CATCH_LOG(_T("CAppSingelton::CAppSingelton"))
}

BOOL CAppSingelton::IsRunning(BOOL bCreateThread, LPCSTR lpSemaphoreName)
{
TRY_CATCH
	//
	//	CreateSemaphore in order to make application a singelton
	//	-------------------------------------------------------
	m_hAppSingeltonSemaphore = CreateSemaphore(
			NULL, 0, 1,	lpSemaphoreName );
	DWORD dwError = ::GetLastError(); 
	if (dwError == ERROR_ACCESS_DENIED || (m_hAppSingeltonSemaphore && dwError == ERROR_ALREADY_EXISTS))
	{
		Log.Add(_MESSAGE_, _T("Notify singelton application and close handle..."));
		//
		//	Notify singelton application and close handle
		//	----------------------------------------------
		ReleaseSemaphore(m_hAppSingeltonSemaphore, 1, NULL);
		::CloseHandle(m_hAppSingeltonSemaphore);

		return TRUE;
	}
	else 
	{
		// for example supportcenter /uinstall may not create thread. just do nothing and return FALSE
		if(bCreateThread)
		{
			 Log.Add(_MESSAGE_, _T("Create AppSingeltonHelperThread"));

			 //	Ctreate Thread wating for event from new instance or terminate event
			 m_hAppSingeltonHelperThread = ::CreateThread(
				 NULL,
				 0,
				 (LPTHREAD_START_ROUTINE)AppSingeltonHelperThreadFunction, 
				 &m_hAppSingeltonSemaphore,  // pass event handle
				 0,
				 &m_dwDThread); 

			 if (m_hAppSingeltonHelperThread == NULL) 
			 {
				Log.Add(_ERROR_, _T("CreateThread AppSingeltonHelperThreadFunction failed (%d)\n"), GetLastError() );
      			return TRUE;
			 }

			 return FALSE;
		}
		else
		{
			Log.Add(_MESSAGE_, _T("Thread will not started..."));
			::CloseHandle(m_hAppSingeltonSemaphore);
		}
	}
CATCH_LOG(_T("CAppSingelton::IsRunning"))
	return FALSE;
}

CAppSingelton::~CAppSingelton(void)
{
TRY_CATCH

	// CleanUp properly
	if(m_kShMem!=NULL)
		UnmapViewOfFile(m_kShMem);

	if(m_hMapFile)
		CloseHandle(m_hMapFile);

	//Close simple AppSingeltonHelperThread
	TerminateThread(m_hAppSingeltonHelperThread, 0);
	CloseHandle(m_hAppSingeltonHelperThread);

CATCH_LOG(_T("CAppSingelton::~CAppSingelton"))
}

VOID CAppSingelton::AppSingeltonHelperThreadFunction(LPVOID lpParam) 
{
TRY_CATCH

	DWORD  dwWaitResult = 0;
	HANDLE hAppSingeltonSemaphore = *(HANDLE*)lpParam;  // thread's read event
	HANDLE hEvents[1];

	hEvents[0] = hAppSingeltonSemaphore;

	do
	{
		Log.Add(_MESSAGE_, _T("CAppSingelton::AppSingeltonHelperThreadFunction Wait..."));

		dwWaitResult = WaitForMultipleObjects( 
			1,             // number of handles in array
			hEvents,       // array of event handles
			FALSE,         // wait till all are signaled
			INFINITE);     // indefinite wait

		Log.Add(_MESSAGE_, _T("CAppSingelton::AppSingeltonHelperThreadFunction Wait passed with dwWaitResult: %d"), dwWaitResult );

		switch (dwWaitResult) 
		{
			// Semaphore event object was signaled.
		case WAIT_OBJECT_0: 
			if(theApp.m_pCSupportMessengerDlg != NULL)
			{
				PostMessage(theApp.m_pCSupportMessengerDlg->m_hWnd, WM_TRAYNOTIFY, OPEN_ABOUT_WINDOW, WM_LBUTTONDBLCLK);
			} 
			else
			{
				Log.Add(_WARNING_, _T("m_pCSupportMessengerDlg is still not initialized"));
			}
			break; 
			// Terminate event was signaled
		case WAIT_OBJECT_0 + 1 : 
			return;
			// An error occurred.
		default: 
			Log.Add(_ERROR_, _T("AppSingeltonHelperThreadFunction Wait error(%d)"), GetLastError() );
			ExitThread(0); 
		}
	}
	while(TRUE);//till terminate event come

CATCH_LOG(_T("CAppSingelton::~AppSingeltonHelperThreadFunction"))
}

void CAppSingelton::CreateSharedMemory(LPCSTR lpSharedMemName, BOOL isInitializationFinished)
{
TRY_CATCH
 
   m_hMapFile = CreateFileMapping(
                 INVALID_HANDLE_VALUE,    // use paging file
                 NULL,                    // default security 
                 PAGE_READWRITE,          // read/write access
                 0,                       // max. object size 
                 sizeof(KSharedMemory),                // buffer size  
                 lpSharedMemName);                 // name of mapping object
 
   if(m_hMapFile == NULL || m_hMapFile == INVALID_HANDLE_VALUE) 
   { 
	   Log.Add(_ERROR_, _T("CreateFileMapping failed with error: (%d)"), GetLastError() );
       return;
   }

   m_kShMem = (KSharedMemory*)MapViewOfFile(m_hMapFile,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,                   
                        0,                   
                        sizeof(KSharedMemory));           
 
   if(m_kShMem == NULL) 
   { 
	   Log.Add(_ERROR_, _T("MapViewOfFile failed with error: (%d)."), GetLastError() );
       return;
   }
  
   CopyMemory((PVOID)&m_kShMem->hWnd, &theApp.m_pMainWnd->m_hWnd, sizeof(HWND));
   m_kShMem->isInitializationFinished = isInitializationFinished;
   Log.Add(_MESSAGE_, _T("CAppSingelton::CreateMapFile m_hWnd: (%d)."), theApp.m_pMainWnd->m_hWnd );

CATCH_LOG(_T("CAppSingelton::CreateSharedMemory()"))
}

HWND  CAppSingelton::ReadSharedMemory(LPCSTR lpSharedMemName)
{
TRY_CATCH

   HANDLE hMapFile = 0;
   KSharedMemory* kShMem;
   HWND	hWnd = 0;

   hMapFile = OpenFileMapping(
                   FILE_MAP_ALL_ACCESS,   // read/write access
                   FALSE,                 // do not inherit the name
                   lpSharedMemName);               // name of mapping object 
 
   if (hMapFile == NULL) 
   { 
	  Log.Add(_ERROR_, _T("Could not open file mapping object (%d)."), GetLastError() );
      return 0; 
   } 
 
   kShMem = (KSharedMemory*)::MapViewOfFile(hMapFile,    // handle to mapping object
               FILE_MAP_ALL_ACCESS,  // read/write permission
               0,                    
               0,                    
               sizeof(KSharedMemory));                   
 
   if(kShMem == NULL) 
   { 
	  Log.Add(_ERROR_, _T("Could not map view of file (%d)."), GetLastError());
      return 0;
   }

   if (kShMem->isInitializationFinished == FALSE)
   {
		Log.Add(_MESSAGE_, _T("CAppSingelton::ReadMapFile Initialization is not yet finished."));
		hWnd = 0; 
   }
   else
   {
		Log.Add(_MESSAGE_, _T("CAppSingelton::ReadMapFile m_hWnd: (%d)."), kShMem->hWnd);
		hWnd = kShMem->hWnd; 
   }

   Log.Add(_MESSAGE_, _T("CAppSingelton::ReadMapFile() UnmapViewOfFile"));
   UnmapViewOfFile(kShMem);
   Log.Add(_MESSAGE_, _T("CAppSingelton::ReadMapFile() CloseHandle"));
   CloseHandle(hMapFile);

   return hWnd;

CATCH_LOG(_T("CAppSingelton::ReadSharedMemory()"))

   return 0;		
}

void CAppSingelton::FinishInitialization()
{
	if (m_kShMem != NULL && m_kShMem->isInitializationFinished == FALSE)
	{
		m_kShMem->isInitializationFinished = TRUE;
	}
}