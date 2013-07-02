// ActiveX.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include "resource.h"

// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[ module(dll, uuid = "{52FE366B-9026-42CA-A98A-DF55A12EFE52}", 
		 name = "ActiveX", 
		 helpstring = "ActiveX 1.0 Type Library",
		 resource_name = "IDR_ACTIVEX") ]
class CActiveXModule
{
public:
// Override CAtlDllModuleT members
//	BOOL WINAPI DllMain(DWORD dwReason, LPVOID /* lpReserved */) throw()
//	{
//		if (dwReason == DLL_PROCESS_ATTACH)
//		{
//			_Module.Init( 0 , 0 );
//			//if (CAtlBaseModule::m_bInitFailed)
//			//{
//			//	ATLASSERT(0);
//			//	return FALSE;
//			//}
//
//#ifdef _ATL_MIN_CRT
//			DisableThreadLibraryCalls(_AtlBaseModule.GetModuleInstance());
//#endif
//		}
//#ifdef _DEBUG
//		else if (dwReason == DLL_PROCESS_DETACH)
//		{
//			// Prevent false memory leak reporting. ~CAtlWinModule may be too late.
//			//_AtlWinModule.Term();	
//			_Module.Term();
//		}
//#endif	// _DEBUG
//		return TRUE;    // ok
//	}
};
		 
