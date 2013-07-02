/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  NetLogLib.cpp
///
///  NetLogLib entry point
///
///  @author Dmitry Netrebenko @date 02.04.2007
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include <NWL/Streaming/CSocketSystem.h>

CSocketSystem g_socketSystem;

// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[ module(dll, uuid = "{5D282F46-A1B8-447B-97D4-8914CFE6EC40}", 
		 name = "NetLogLib", 
		 helpstring = "NetLogLib 1.0 Type Library",
		 resource_name = "IDR_NETLOGLIB") ]
class CNetLogLibModule
{
public:
// Override CAtlDllModuleT members
};
		 
