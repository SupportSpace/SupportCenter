/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  NetworkLayer.cpp
///
///  Dynamic library entry point
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/NetworkLayer.h>
#include <windows.h>


#ifdef _DYNAMIC_NWL_

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
