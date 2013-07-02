// FTestRelayLoad.cpp : Implementation of WinMain


#include "stdafx.h"
#include "resource.h"
#include <boost/shared_ptr.hpp>
#include "CLoadTestDlg.h"
#include <AidLib/CException/CException.h>
#include <NWL/Streaming/CSocketSystem.h>
#include <NWL/TLS/CTLSSystem.h>
#include "CTestCollection.h"
#include "CTestFactory.h"
#include <NWL/Streaming/CNetworkLayer.h>
#include <AidLib/CCrypto/CCrypto.h>
#include <AidLib/CSingleton/CSingleton.h>


CSocketSystem socks;
CTLSSystem tls;

// The module attribute causes WinMain to be automatically implemented for you
[ module(exe, uuid = "{A56818D6-BD71-4C5F-B91F-361D40A4D772}", 
		 name = "FTestRelayLoad", 
		 helpstring = "FTestRelayLoad 1.0 Type Library",
		 resource_name = "IDR_FTESTRELAYLOAD") ]
class CFTestRelayLoadModule
{
public:
	HRESULT Run(int nShowCmd = SW_HIDE)
	{
		Log.RegisterLog(new cFileLog());
		NWL_INSTANCE;
		CSingleton<CCrypto>::instance();
	TRY_CATCH
		TESTS_INSTANCE;
		TESTFACTORY_INSTANCE;
		boost::shared_ptr<CLoadTestDlg> m_dlg(new CLoadTestDlg());
		m_dlg->DoModal();
	CATCH_LOG()
		return S_OK;
	}
public:
// Override CAtlExeModuleT members
};
		 
