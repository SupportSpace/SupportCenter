/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  HelperService.cpp
///
///  Dual Helper Service / Proxy Application executable entry point
///
///  @author "Archer Software" Sogin M. @date 01.10.2007
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HelperService.h"
#include "CHelperService.h"
#include "CRCHostProxy/CRCHostProxy.h"
#include "CRCHostProxy/CWallpaperSwitch.h"
#include "Sddl.h"
#include <AidLib/Logging/CLogFolder.h>
#include "CWorkbench/CWorkbench.h"

#pragma comment(lib,"AidLib.lib")

const tstring g_shutdownCmd = _T("SHUTDOWN");
const tstring g_enableWallCmd = _T("ENABLEWALL");
const tstring g_disableWallCmd = _T("DISABLEWALL");
const tstring g_workbenchCmd = _T("WORKBENCH");
const tstring g_proxyAppCmd = _T("APPMODE");

CComModule _Module;

int APIENTRY _tWinMain(	HINSTANCE hInstance,
						HINSTANCE hPrevInstance,
						LPTSTR lpCmdLine,
						int nCmdShow)
{
TRY_CATCH

	CoInitialize(NULL);

	/// creating folder for logging
	LOGS_FOLDER_INSTANCE.CreateLogsFolder();

	if (tstring::npos != UpperCase(lpCmdLine).find(g_workbenchCmd))
	{
		///Initializing logger
		Log.RegisterLog(new cFileLog(Format(_T("%s\\WorkbenchApplication.Log"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str()).c_str()));

		tstring url = lpCmdLine;
		url = url.erase(0, UpperCase(url).find(g_workbenchCmd) + tstring(g_workbenchCmd).length());
		tstring terminals = _T(" \n");
		while(url.length()>0 && tstring::npos != terminals.find(url[0]))
			url = url.erase(0,1);

		/// Faked TYPELIB guid. Just to prevent asserts in VC8 without SP1
		static const GUID fakedLib = { 0xfc8f4a8b, 0x8af6, 0x4cec, { 0xa1, 0xe6, 0xf3, 0xa9, 0xac, 0xd2, 0x3f, 0x56 } };
		_Module.Init(0, hInstance, &fakedLib);
		
		//return CComObject<CWorkbench>().Run(url);
		return CWorkbench().Run(hInstance, url);
	} else
	if (tstring::npos != UpperCase(lpCmdLine).find(g_proxyAppCmd))
	{
		///Initializing logger
		Log.RegisterLog(new cFileLog(Format(_T("%s\\HelperApplication.Log"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str()).c_str()));

		/// Starting as application
		return RCHOST_PROXY_INSTANCE.Run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	} else if (tstring::npos != UpperCase(lpCmdLine).find(g_shutdownCmd))
	{

		///Initializing logger
		Log.RegisterLog(new cFileLog(Format(_T("%s\\HelperApplication.Log"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str()).c_str()));

		tstring threadId = UpperCase(lpCmdLine);
		threadId = threadId.erase(0,threadId.find(g_shutdownCmd)+g_shutdownCmd.length());
		/// Shutdown proxy with thread id
		return CRCHostProxy::Shutdown(_ttoi(threadId.c_str()));
	} else if (tstring::npos != UpperCase(lpCmdLine).find(g_enableWallCmd))
	{

		///Initializing logger
		Log.RegisterLog(new cFileLog(Format(_T("%s\\HelperApplication.Log"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str()).c_str()));

		CWallpaperSwitch::EnableActiveDesktop(true);
		return 0;
	} else if (tstring::npos != UpperCase(lpCmdLine).find(g_disableWallCmd))
	{

		///Initializing logger
		Log.RegisterLog(new cFileLog(Format(_T("%s\\HelperApplication.Log"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str()).c_str()));

		CWallpaperSwitch::EnableActiveDesktop(false);
		return 0;
	} else
	{

		///Initializing logger
		Log.RegisterLog(new cFileLog(Format(_T("%s\\HelperService.Log"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str()).c_str()));

		/// Starting as service
		CHelperService::instance()->Init(lpCmdLine,HELPER_SERVICE_NAME, HELPER_SERVICE_DISPLAY_NAME);
		return 0;
	}
	
CATCH_LOG()
	return -1;
}
