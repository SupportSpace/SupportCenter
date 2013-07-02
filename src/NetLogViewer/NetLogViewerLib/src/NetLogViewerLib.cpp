/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  NetLogViewerLib.cpp
///
///  Implementation of DLL Exports.
///
///  @author Sogin Max @date 27.03.2007
///
////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "resource.h"
#include <NetLog/CNetLog.h>

#pragma comment(lib,"AidLib.lib")
#pragma comment(lib,"NWL.lib")
#pragma comment(lib,"Ws2_32.lib")

#include <NWL/Streaming/CSocketSystem.h>
//#include <NWL/TLS/CTLSSystem.h>
CSocketSystem sockSystem;
//CTLSSystem tlsSystem;

// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[ module(dll, uuid = "{0AEC8A83-F7D3-49C8-95E6-827B7D236BE2}", 
		 name = "NetLogViewerLib", 
		 helpstring = "NetLogViewerLib 1.0 Type Library",
		 resource_name = "IDR_NETLOGVIEWERLIB") ]
class CNetLogViewerLibModule
{
public:
// Override CAtlDllModuleT members
};
		 
