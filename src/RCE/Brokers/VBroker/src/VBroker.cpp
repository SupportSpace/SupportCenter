// VBroker.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"

#include <AidLib/Logging/CInstanceTracker.h>
CInstanceTracker m_moduleInsanceTracker(_T("VBroker module"));


// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[ module(dll, uuid = "{DD41B73B-F8D7-48D6-A4BC-B89C1ACC5B98}", 
		 name = "VBroker", 
		 helpstring = "VBroker 1.0 Type Library",
		 resource_name = "IDR_VBROKER") ]
class CVBrokerModule
{
public:
// Override CAtlDllModuleT members
};
		 
