
#define BOOST_THREAD_NO_LIB
#define _WINSOCKAPI_

#include <tchar.h>
#include <windows.h>
#include "resource.h"
#include <boost/threadpool.hpp>
#include <NWL/Streaming/CSocketSystem.h>
#include <AidLib/CCrypto/CCrypto.h>
#include <NWL/Streaming/CMatchPortMapping.h>
#include <NWL/Streaming/CFirewallConfigurator.h>
#include "CPortMappingHelper.h"
//========================================================================================

const int CountThreadsInPool=100;
const int RefreshGUITimerID=1;
HINSTANCE hInstance;
HWND hMainWnd;
CSocketSystem g_SockSystem;
boost::threadpool::pool* g_ThreadsPool;
CPortMappingHelper* g_PortMappingHelper;
//========================================================================================

void threadPortMapping(CPortMappingHelper* portMappingHelper)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("START TO DO PORT MAPPING"));

	CoInitialize(NULL);

	portMappingHelper->ThreadStartedNotify();

	SPortMappingInfo pmi=portMappingHelper->GetDataForPortMapping();
	
	CMatchPortMapping MatchPortMapping(	pmi.m_server_address,
										pmi.m_server_port,
										pmi.m_search_delay,
										pmi.m_count_to_try);

	while (!MatchPortMapping.CreateMappedPorts(	pmi.m_user_id,
												pmi.m_user_password,
												&pmi.m_user_external_port,
												0,
												&pmi.m_user_internal_port,
												pmi.m_protocol.c_str() ))
		Sleep(pmi.m_search_delay);

	if (pmi.m_delete_mapping)
		MatchPortMapping.DeleteMappedPorts(	pmi.m_user_id,
											pmi.m_user_password,
											pmi.m_user_external_port,
											pmi.m_user_internal_port,
											pmi.m_protocol.c_str());

	portMappingHelper->ThreadSuccessedNotify();
	Log.Add(_MESSAGE_,_T("ALL OPERATIONS WERE SUCCESSFUL"));

	CoUninitialize();

	return;
CATCH_LOG()
	Log.Add(_MESSAGE_,_T("SOME OPERATION WAS FAILED"));
	portMappingHelper->ThreadFailedNotify();
}
//----------------------------------------------------------------------------------------

void threadPortCheckOnly(CPortMappingHelper* portMappingHelper)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("START TO DO PORT CHECKING"));

	CoInitialize(NULL);

	portMappingHelper->ThreadStartedNotify();

	SPortMappingInfo pmi=portMappingHelper->GetDataForPortMapping();

	Log.Add(_MESSAGE_,_T("Check ports for %s with ExPort: %u and InPort: %u"),
			pmi.m_user_id.c_str(),pmi.m_user_external_port,pmi.m_user_internal_port);
			
	COpenPortRequest OpenPortRequest(pmi.m_server_address,pmi.m_server_port);

	OpenPortRequest.CheckPortAvailability(	pmi.m_user_id,
											pmi.m_user_password,
											pmi.m_user_external_port,
											pmi.m_user_internal_port);

	portMappingHelper->ThreadSuccessedNotify();
	Log.Add(_MESSAGE_,_T("ALL OPERATIONS WERE SUCCESSFUL"));

	CoUninitialize();

	return;
CATCH_LOG()
	Log.Add(_MESSAGE_,_T("SOME OPERATION WAS FAILED"));
	portMappingHelper->ThreadFailedNotify();
}
//----------------------------------------------------------------------------------------

INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_TIMER:
			SetDlgItemInt(hwnd,IDC_ACTIVETHREADS,g_PortMappingHelper->GetCountActiveThread(),FALSE);
			SetDlgItemInt(hwnd,IDC_FAILEDTHREADS,g_PortMappingHelper->GetCountFailThread(),FALSE);
			SetDlgItemInt(hwnd,IDC_SUCCESSEDTHREADS,g_PortMappingHelper->GetCountSuccessThread(),FALSE);
			return TRUE;
			
		case WM_INITDIALOG:
		{
			tstring strIniFile=CCommandLinePath(GetCommandLine()).GetDir();
			strIniFile+=_T("FTestNWLPortMappingClient.ini");
						
			CIniFile iniFile;
			if (!iniFile.loadAnsiFile(strIniFile.c_str()))
			{
				MessageBox(hwnd,_T("Ini file not found"),strIniFile.c_str(),0);
				SendMessage(hwnd,WM_CLOSE,0,0);
				return TRUE;
			}
			Log.Add(_MESSAGE_,_T("\r\nSTART FTestNWLPortMappingClient with %u threads\r\n"),
					(int)iniFile[_T("CONFIG")][_T("CountToStart")]);
			
			g_PortMappingHelper=new CPortMappingHelper(strIniFile.c_str());
			g_ThreadsPool=new boost::threadpool::pool(CountThreadsInPool);

			SetTimer(hwnd,RefreshGUITimerID,100,NULL);

			if ( iniFile[_T("CONFIG")][_T("PortMapping")] )
				for (int i=iniFile[_T("CONFIG")][_T("CountToStart")]; i>0; i--)
					g_ThreadsPool->schedule( boost::bind(threadPortMapping, g_PortMappingHelper) );
			else
				for (int i=iniFile[_T("CONFIG")][_T("CountToStart")]; i>0; i--)
					g_ThreadsPool->schedule( boost::bind(threadPortCheckOnly, g_PortMappingHelper) );

			return TRUE;
		}
		case WM_CLOSE:
			g_ThreadsPool->wait();
			
			delete g_ThreadsPool;
			delete g_PortMappingHelper;

			KillTimer(hwnd,RefreshGUITimerID);
			DestroyWindow(hwnd);
			
			Log.Add(_MESSAGE_,_T("\r\nFINISH FTestNWLPortMappingClient\r\n"));
			return TRUE;
		
		case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE;
	}
	return FALSE;
}
//----------------------------------------------------------------------------------------

int WinMain(HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
	Log.RegisterLog(new cFileLog());

TRY_CATCH
	
	CoInitialize(NULL);

	TRY_CATCH

		FIREWALL_CONFIGURATOR_INSTANCE;

	CATCH_LOG()

	CRYPTO_INSTANCE;
	NWL_INSTANCE;
	
	hInstance=hInst;

	hMainWnd=CreateDialog(hInstance,MAKEINTRESOURCE(IDD_MAIN),0,DialogProc);
	
	MSG msg;
	while (GetMessage(&msg,0,0,0))
	{
		if (!IsDialogMessage(hMainWnd,&msg))
			DispatchMessage(&msg);
	}

CATCH_LOG()

	return 0;
}
