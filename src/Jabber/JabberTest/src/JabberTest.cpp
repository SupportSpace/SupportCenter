// JabberTest.cpp : Implementation of WinMain

#include "stdafx.h"
#include "resource.h"
#include "CJabberTestDlg.h"
#include <AidLib/CException/CException.h>
#include <boost/shared_ptr.hpp>
/*
#include "CSyncTest.h"
#include "SConnectParams.h"
#include "STestParams.h"
*/
// The module attribute causes WinMain to be automatically implemented for you
[ module(exe, uuid = "{79EA761F-5C12-4EAE-B3A9-FC11D3419343}", 
		 name = "JabberTest", 
		 helpstring = "JabberTest 1.0 Type Library",
		 resource_name = "IDR_JABBERTEST") ]
class CJabberTestModule
{
public:
	HRESULT Run(int nShowCmd = SW_HIDE)
	{
		Log.RegisterLog(new cFileLog());
	TRY_CATCH
/*
		boost::shared_ptr<CAbstractTest> test(new CSyncTest());
		
		SConnectParams cp;
		cp.m_connectTimeout = 30000;
		cp.m_remoteUserName = _T("customer");
		cp.m_resource = _T("jabber_test");
		cp.m_serverAddr = _T("edem");
		cp.m_serverName = _T("edem");
		cp.m_userName = _T("utest1");
		cp.m_userPasswd = _T("utest1");

		STestParams tp;
		tp.m_bulkSize = 1;
		tp.m_clientType = ctGloox;
		tp.m_msgCount = 1;
		tp.m_sendDelay = 1000;
		tp.m_waitTimeout = 30000;

		test->Init(tp, cp);
		test->Start();
*/
		boost::shared_ptr<CJabberTestDlg> m_dlg(new CJabberTestDlg());
		m_dlg->DoModal();
	CATCH_LOG()
		return S_OK;
	}

// Override CAtlExeModuleT members
};
		 
