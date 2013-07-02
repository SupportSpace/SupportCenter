// BrokerProxy.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"

#include <AidLib/Logging/CInstanceTracker.h>
CInstanceTracker m_moduleInsanceTracker(_T("BrokerProxy module"));

// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[ module(dll, uuid = "{4FC815B5-B58B-49F3-B852-3FA875F5D56D}", 
		 name = "BrokerProxy", 
		 helpstring = "BrokerProxy 1.0 Type Library",
		 resource_name = "IDR_BROKERPROXY") ]
class CBrokerProxyModule
{
public:
// Override CAtlDllModuleT members
};
		 
