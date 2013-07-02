// RCUI.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include "resource.h"
#include <AidLib/Logging/CInstanceTracker.h>

CInstanceTracker m_moduleInsanceTracker(_T("RCUI module"));

// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[ module(dll, uuid = "{15C5A5E9-7C69-436E-9E89-A2A566FBB77C}", 
		 name = "RCUI", 
		 helpstring = "RCUI 1.0 Type Library",
		 resource_name = "IDR_RCUI") ]
class CRCUIModule
{
public:
// Override CAtlDllModuleT members
};
		 
