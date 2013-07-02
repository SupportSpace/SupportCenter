/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLogBuilder.cpp
///
///  class for building log object
///
///  @author "Archer Software" Sogin M. @date 02.08.2007
///
////////////////////////////////////////////////////////////////////////
#include <NetLog/CNetworkLog.h>
#include <AidLib/Logging/cLog.h>
#include "CLogBuilder.h"
#include <Shlobj.h>
#include <AidLib/Utils/Utils.h>
#include <AidLib/Logging/CLogFolder.h>

#pragma comment(lib, "NetLog.lib")
#pragma comment(lib, "ws2_32.lib")

#define DEF_FILE_LOG_NAME _T("RCUI.dll.log")

CLogBuilder::CLogBuilder()
{
TRY_CATCH
CATCH_LOG()
}

CLogBuilder::~CLogBuilder()
{
TRY_CATCH
	Log.ClearList();
CATCH_LOG()
}

void CLogBuilder::OnRefCountChanged(const int oldRefCount, const int newRefCount)
{
TRY_CATCH
	if (0 == newRefCount)
	{
		Log.Add(_MESSAGE_,_T("CLogBuilder no references found, shuting down log engine..."));
		Log.ClearList();
	} else
	if (1 == newRefCount && 0 == oldRefCount)
	{
		TRY_CATCH
			//TODO: rename log
			Log.RegisterLog(new cFileLog(Format(_T("%s\\Installer.Log"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str()).c_str()));
			//CSIDL_PERSONAL CSIDL_APPDATA
			//Log.RegisterLog(new cFileLog(Format(_T("%s\\%s"),GetSpecialFolderPath(CSIDL_PERSONAL).c_str(),DEF_FILE_LOG_NAME).c_str()));
		CATCH_LOG()
		TRY_CATCH
			TCHAR moduleName[MAX_PATH];
			GetModuleFileName(NULL, moduleName, MAX_PATH);
			if (UpperCase(moduleName).find(_T("DLLHOST.EXE")) == tstring::npos)
				Log.RegisterLog(new CNetworkLog("SupportSpaceTools"));
		CATCH_LOG()
		TRY_CATCH
			Log.RegisterLog(new cFileLog(Format(_T("%s\\Installer.Log"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str()).c_str()));
			//Log.RegisterLog(new cFileLog(Format(_T("%s\\%s"),GetModulePath(GetCurrentModule()).c_str(),DEF_FILE_LOG_NAME).c_str()));
		CATCH_LOG()
	}
CATCH_LOG()
}
