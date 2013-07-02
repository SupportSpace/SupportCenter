// RCInstaller.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include "resource.h"
#include <AidLib/Logging/CInstanceTracker.h>
#pragma comment(lib,"AidLib.lib")

CInstanceTracker m_moduleInsanceTracker(_T("RCInstaller module"));

// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[ module(dll, uuid = "{68498894-2E76-432D-8341-A949C201FB3F}", 
		 name = "RCInstaller", 
		 helpstring = "RCInstaller 1.0 Type Library",
		 resource_name = "IDR_RCINSTALLER") ]
class CRCInstallerModule
{
public:
// Override CAtlDllModuleT members
//};
	BOOL WINAPI DllMain(DWORD dwReason, LPVOID lpReserved);
	HRESULT DllCanUnloadNow(void); 
};
		 
BOOL WINAPI CRCInstallerModule::DllMain(DWORD dwReason, LPVOID lpReserved) 
{
	TCHAR threadId[200];
	_stprintf_s(threadId,_T("%x %x"),::GetCurrentProcessId(),::GetCurrentThreadId());
	TCHAR s[MAX_PATH];
	_stprintf_s(s,_T("CRCInstallerModule::DllMain dwReason=%x"),dwReason);
	//::MessageBox(NULL,s,threadId,0);
	return CAtlDllModuleT<CRCInstallerModule>::DllMain(dwReason, lpReserved);
}
HRESULT CRCInstallerModule::DllCanUnloadNow(void)
{
	HRESULT res;
	res=CAtlDllModuleT<CRCInstallerModule>::DllCanUnloadNow();
	TCHAR threadId[200];
	_stprintf_s(threadId,_T("%x %x"),::GetCurrentProcessId(),::GetCurrentThreadId());
	TCHAR s[MAX_PATH];
	_stprintf_s(s,_T("CRCInstallerModule::DllCanUnloadNow cRef=%d"),res);
	//::MessageBox(NULL,s,threadId,0);
	return res;
}
		 
