// Broker.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"

#include <AidLib/Logging/CInstanceTracker.h>
#include <atldbcli.h>
CInstanceTracker m_moduleInsanceTracker(_T("Broker module"));

// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[ module(dll, uuid = "{B120F0F6-9AF6-44BB-9B61-DE96D2F79E72}", 
		 name = "Broker", 
		 helpstring = "Broker 1.0 Type Library",
		 resource_name = "IDR_BROKER") ]
class CBrokerModule
{
public:
// Override CAtlDllModuleT members
};
		 
