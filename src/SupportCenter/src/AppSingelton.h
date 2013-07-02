// AppSingelton.h : header file
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CActivityHandler
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CAppSingelton :	class that limit application instance to only one
//
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 10/29/2006 05:21:10 PM
// Comments: First Issue
//===========================================================================

#pragma once

struct KSharedMemory
{
	BOOL isInitializationFinished;
	HWND hWnd;	// hWnd of the first and singelton instance may be shared between process
};

#define SC_SHARED_MEM TEXT("SupportCenterSharedMemory-26EC6ABA-F167-11DB-A517-57B655D89593")
#define WB_SHARED_MEM TEXT("WorkbenchSharedMemory-26EC6ABA-F167-11DB-A517-57B655D89593")

#define SC_SEM_NAME TEXT("SupportCenterSemaphore-26EC6ABA-F167-11DB-A517-57B655D89593")
#define WB_SEM_NAME TEXT("WorkbenchSharedSemaphore-26EC6ABA-F167-11DB-A517-57B655D89593")

class CAppSingelton
{
public:
	CAppSingelton(void);
	~CAppSingelton(void);

	BOOL IsRunning(BOOL bCreateThread, LPCSTR lpSemaphoreName);
	
	void CreateSharedMemory(LPCSTR lpSharedMemName, BOOL isInitializationFinished);
	HWND ReadSharedMemory(LPCSTR lpSharedMemName);
	void FinishInitialization();

private:
	static  VOID AppSingeltonHelperThreadFunction(LPVOID lpParam); 
	
	HANDLE	m_hAppSingeltonSemaphore;
    HANDLE	m_hAppSingeltonHelperThread; 
    DWORD	m_dwDThread; 

	KSharedMemory *m_kShMem;
	HANDLE m_hMapFile;
};
