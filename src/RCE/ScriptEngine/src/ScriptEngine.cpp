/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  ScriptEngine.cpp
///
///  ScriptEngine library entry point
///
///  @author Dmitry Netrebenko @date 15.11.2007
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include <NWL/Streaming/CSocketSystem.h>
#include <NWL/TLS/CTLSSystem.h>

CSocketSystem sockSystem;
CTLSSystem tlsSystem;

// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[ module(dll, uuid = "{05467A6E-F20F-4546-8A6C-91D48FA1BCD9}", 
		 name = "ScriptEngine", 
		 helpstring = "ScriptEngine 1.0 Type Library",
		 resource_name = "IDR_SCRIPTENGINE") ]
class CScriptEngineModule
{
public:
// Override CAtlDllModuleT members
};
		 
