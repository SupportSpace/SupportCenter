/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  AidLib.cpp
///
///  Dynamic library entry point
///
///  @author Dmitry Netrebenko @date 25.12.2006
///
////////////////////////////////////////////////////////////////////////

#include <AidLib/AidLib.h>
#include <windows.h>
#include <AidLib/CThread/CThreadLSInitializer.h>

#pragma warning( disable : 4996 )

#ifdef _DYNAMIC_AID_

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#endif
