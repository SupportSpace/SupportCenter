// SupportMessenger.cpp : Defines the class behaviors for the application.
//
#include "stdafx.h"
#include "afxole.h"
#include "SupportMessenger.h"
#include "SupportMessengerDlg.h"
#include "DHTMLDialogMsg.h"
//#include <AidLib/Logging/cLog.h>		//	<- todo remove this 
#include <AidLib/Logging/CLogFolder.h>	//  <- todo uncomment this
//#include "AidLib/CException/CException.h"
//#include <NetLog/CNetworkLog.h>
//#pragma comment(lib, "NetLog.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSupportMessengerApp

BEGIN_MESSAGE_MAP(CSupportMessengerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CSupportMessengerApp construction

CSupportMessengerApp::CSupportMessengerApp()
{
	TRY_CATCH
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	m_bAppStartedAutomatiaclly = FALSE;
	m_pCSupportMessengerDlg = NULL;
	m_dwIEMajorVersion = 0;

	CATCH_LOG(_T("CSupportMessengerApp::CSupportMessengerApp"))
}


// The one and only CSupportMessengerApp object

CSupportMessengerApp theApp;


// CSupportMessengerApp initialization
BOOL CSupportMessengerApp::InitInstance()
{
TRY_CATCH

	CWinApp::InitInstance();

#ifndef DEBUG
	Log.RegisterLog(new cDbgOutLog());
#endif

	SuppressServerBusyDialog();
 
	//
	//	Parse command line parameters - the only supported parameter is /autostart
	//	TODO: will add some class to parse parameter when one more parameter will be required
	//  if /autostart specified then application was started on windows login using 
	//  HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run
	// 
	BOOL	bUninstall = FALSE;
	BOOL	bEnableNetLog = FALSE;
	BOOL	bDisableFileLog = FALSE;
	eVerbosity	eLogVerbosity = _TRACE_DEBUG_;
	BOOL	bCreateThread = TRUE;
	BOOL	bWorkbench = FALSE;

	if(__argc > 1)
	{
		if(strcmp(CMD_LINE_PARAM_AUTOSTART, __argv[1])==0)
			m_bAppStartedAutomatiaclly = TRUE;
		else
		if(strcmp(CMD_LINE_PARAM_UNINSTALL, __argv[1])==0)
			bUninstall = TRUE;
		else
		if(strcmp(CMD_LINE_PARAM_ENABLE_NET_LOG, __argv[1])==0)
			bEnableNetLog = TRUE;
		else
		if(strcmp(CMD_LINE_PARAM_DISABLE_FILE_LOG, __argv[1])==0)
			bDisableFileLog = FALSE;
		else
		if(strcmp(CMD_LINE_PARAM_TRACE_LOG, __argv[1])==0)
			eLogVerbosity = _TRACE_CALLS_;
		else
		if(strcmp(CMD_LINE_PARAM_WORKBENCH_MODE, __argv[1])==0)
			bWorkbench = TRUE;
	}

	//
	//	retrieve application installation folder
	//	
	DWORD	dwRetVal = GetModuleFileName(m_hInstance, m_sApplicationPath.GetBuffer(MAX_PATH), MAX_PATH);
	m_sApplicationPath.ReleaseBuffer();
	if(dwRetVal == 0)
		m_sApplicationPath	= m_pszHelpFilePath;

	int	pos = 0;
	if((pos = m_sApplicationPath.ReverseFind(_T('\\'))) > 0)
		m_sApplicationPath	= m_sApplicationPath.Left(pos+1);

	//
	//	need to set current directory in order to work from not current folder 
	//	optionally to use GetCurrentDirectory() as well 
	//
	SetCurrentDirectory(m_sApplicationPath);

	//
	//	read configuration of application from ini file
	//
	m_cSettings.ReadConnectionInfo();

	//
	//  enable/disable logging 
	//
	if(bDisableFileLog==FALSE)
	{
		TRY_CATCH

			//
			//	construct application logfile name
			//
			CString sFullLogPathName;
			CString sLogFileName;

			if(bWorkbench==true)
				sLogFileName = _T("Workbench.Log");
		    else
				sLogFileName = _T("SupportCenter.Log");

			//tstring  sLogsFolderName = LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str();// have small issue in exception
			tstring  sLogsFolderName = Format(_T("%s\\Logs"),RemoveTrailingSlashes(GetModulePath(GetCurrentModule())).c_str());
			sFullLogPathName.Format(_T("%s\\%s"),sLogsFolderName.c_str(), sLogFileName); 
			
			cFileLog *fileLog = new cFileLog(sFullLogPathName);
			Log.RegisterLog(fileLog);

			// backup old log file as required STL-395
			BackupLogfile(sLogsFolderName.c_str(),sLogFileName);

		CATCH_LOG()
	}

	Log.SetVerbosity(eLogVerbosity);

	Log.Add(_MESSAGE_, _T("Command line parameter is %s:"),__argv[1]);

	if( bUninstall == TRUE )
		bCreateThread = FALSE;

	//
	//
	//
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(REGKEY_COMPANY);

	//
	// retrieve IE version running on local PC. Ignore error and assume that IE6 installed
	// 
	GetIEVersion();

	if(m_dwIEMajorVersion < IE_MAJOR_VERSION_REQUIRED)
	{
		CDHTMLDialogMsg dlg(IDR_HTML_IE6NOTIF_MSG);
		m_pMainWnd = &dlg;
		int nResponse = dlg.DoModal();
		return FALSE;
	}

	//
	//  we are in the case of trying to open Workbench
	//	if Workbench may be shown then 
	//
	if( bWorkbench==TRUE )
	{
		//C:\Program Files\SupportSpace\Support Platform>SupportCenter.exe /workbench "http://ws-anatoly:8080" 0 "DispName" 10
		CString	  sFullUrl(__argv[2]);
		eCallType callType = (eCallType)atoi(__argv[3]);
		CString   sCustomerDisplayName(__argv[4]);
		long	  lUid = atol(__argv[5]);//supportRequestID or notificationId

		Log.Add(_MESSAGE_, _T("start WorkbenchDlg process. URL: %s, callType %d, CustomerDisplayName :%s, lUid: %d"),sFullUrl, callType, sCustomerDisplayName, lUid );
		//
		//
		if(IsRunning(bCreateThread, WB_SEM_NAME)==TRUE)
		{
			int counter = 0;

			while (counter < 10) // waiting for at most 10 seconds
			{
				//
				//	if one instance of Workbench is running 
				//
				HWND  hFirstInstanceWnd = ReadSharedMemory(WB_SHARED_MEM);
				if(hFirstInstanceWnd != NULL)
				{
					if(callType == ConsultCall) 
					{
						PostMessage(hFirstInstanceWnd, WM_WORKBENCH_NEW_CONSULT_SESSION, 0, (LPARAM)lUid);
					}
					else
					{
						PostMessage(hFirstInstanceWnd, WM_WORKBENCH_NEW_SUPPORT_SESSION, 0, (LPARAM)lUid);
					}
					break;
				}
				else
				{
					counter++;
					Sleep(1000);
					Log.Add(_MESSAGE_, _T("CSupportMessengerApp::InitInstance() WB is not initialized yet. waiting (%d)... "), counter);			
				}
			}
			if (counter == 10)
			{
				Log.Add(_MESSAGE_, _T("CSupportMessengerApp::InitInstance() can not retrieve hWnd"));			
			}
			return FALSE;
		}
		
		m_pWorkbenchDlg = new CWorkbenchDlg(
			CWnd::FromHandle(GetDesktopWindow()),  //must be used to avoid minimize to TaksBar issue
			sFullUrl, 
			lUid, 
			ReadSharedMemory(SC_SHARED_MEM),
			callType,
			sCustomerDisplayName);

		m_pMainWnd = m_pWorkbenchDlg;

		//
		//	create map file with hWnd. Another instance of Workbench will be able:
		//  to read this shared memory, 
		//  to retireve value of HWND and then to send message to the window
		//  For example to send WM_WORKBENCH_NEW_SUPPORT_SESSION command 
		//
		CreateSharedMemory(WB_SHARED_MEM, FALSE); // initialization is not yet finished

		if (m_pMainWnd) 
		   return TRUE;
		else
		   return FALSE;
	}

	//
	//  we are in the case of running SupportCenter
	//
	if(IsRunning(bCreateThread, SC_SEM_NAME)==TRUE)
	{
		//
		//	if one instance is running 
		//
		if(bUninstall)
		{
			HWND  hFirstInstanceWnd = ReadSharedMemory(SC_SHARED_MEM);
			if(hFirstInstanceWnd)
			{
				PostMessage(hFirstInstanceWnd, WM_UNINSTALL_COMMAND, 0, 0);
			}
			else
			{
				Log.Add(_MESSAGE_, _T("CSupportMessengerApp::InitInstance() can not retrieve hWnd"));			
			}
		}
	
		return FALSE;
	}
	else
	{
		if(bUninstall)
		{
			Log.Add(_MESSAGE_, _T("CSupportMessengerApp::InitInstance nothing to stop"));			
			return FALSE;
		}
	}

	Log.Add(_MESSAGE_, _T("Application Installation path is %s:"), m_sApplicationPath);	

	m_pcTransparentClassAtom = new CTransparentClassAtom(GetModuleHandle(NULL));

	m_pCSupportMessengerDlg  = new CSupportMessengerDlg(CWnd::FromHandle(GetDesktopWindow()));

	m_pMainWnd = m_pCSupportMessengerDlg;

	//
	//	create map file with hWnd. Another instance of messnger will be able:
	//  to read this shared memory, 
	//  to retireve value of HWND and then to send message to the window
	//  For example to send /uninstall or /stop flag from command line 
	//
	CreateSharedMemory(SC_SHARED_MEM, TRUE);
	
	//  IMConnectivityQuality.log
	m_conQualityLog.Init(_T("IMConnectivityQuality"), m_cSettings.m_stIMConnectivityQuality.LogLevel);

CATCH_LOG(_T("CSupportMessengerApp::InitInstance"))

	if (m_pMainWnd) 
       return TRUE;
    else
       return FALSE;
}

int CSupportMessengerApp::ExitInstance() 
{
	int	RetVal = 0;
TRY_CATCH

	if( m_pcTransparentClassAtom)
		delete m_pcTransparentClassAtom;

	if( m_pCSupportMessengerDlg)
		delete m_pCSupportMessengerDlg;

	RetVal = CWinApp::ExitInstance();

CATCH_LOG(_T("CSupportMessengerApp::ExitInstance"))
	return RetVal;
}

void CSupportMessengerApp::SuppressServerBusyDialog()
{
	//	http://support.microsoft.com/default.aspx?scid=kb;EN-US;Q248019 
	//  "If you call a method on a COM server from an MFC COM client application and if the method takes a 
	//  long time to process and return back, you won't be able to do anything on the client application and 
	//  the OLE Server Busy dialog box pops up. This article explains how you can increase the time-out period 
	//  of the COM call and also shows you how to avoid this dialog box. 
	//
	
	//  Ensure that the MFC client application is calling AfxOleInit() to initialize COM. 
	//  This is important, because AfxOleInit() also initializes and registers a COleMessageFilter 
	//  data member in the CWinApp. 
	//  Alternatively, you can create your own COleMessageFilter object and register that during the startup. 
	AfxOleInit();

	//
	//	seems this call not required if we call AfxOleInit()
	//
	//  Unregister the existing message filter.
	//  AfxOleGetMessageFilter()->Revoke(); // tried - no effect

	//
	//	Determines the calling application's action when it receives a busy response 
	//	from a called application.
	//
	//  AfxOleGetMessageFilter()->SetRetryReply(3); tried - not helped

	//
	//  to set the wait period on outgoing COM calls. 
	//  If the COM call takes longer than nTimeout milliseconds, 
	//  then the MFC Client application displays the OLE Server Busy dialog box. 
	AfxOleGetMessageFilter()->SetMessagePendingDelay(60000);

	// Use to disable the Not Responding dialog box, which is displayed if a keyboard or mouse message 
	// is pending during an OLE call and the call has timed out. 
	AfxOleGetMessageFilter()->EnableNotRespondingDialog(FALSE);

	// Use to disable the busy dialog box from appearing after the COM call times out. 
	AfxOleGetMessageFilter()->EnableBusyDialog(FALSE);

	//
	//	seems this call not required if we call AfxOleInit()
	//  Re-register the message filter.
	//
	// AfxOleGetMessageFilter()->Register();	// tried - no effect

	// Another way to suppress the server busy dialog box is to use OleInitialize and OleUninitialize instead of 
	// AfxOleInit in your application. 
}

// Windows doesn't have an API to get a file size based on file name. This small function does that. 
// It returns -1 if a file doesn't exist. 
// It doesn't handle files > 2 GB (max positive number for 32 bit signed value). 
// It's quite easy to extend it to 64-bits if you know what is the 64 bit integer type in your compiler 
// (unfortunately there's no standard). 
long CSupportMessengerApp::GetFileSize(const TCHAR *fileName)
{
    BOOL                        fOk = FALSE;
    WIN32_FILE_ATTRIBUTE_DATA   fileInfo;

    if (NULL == fileName)
        return -1;

    fOk = GetFileAttributesEx(fileName, GetFileExInfoStandard, (void*)&fileInfo);
    if (!fOk)
        return -1;
    //assert(0 == fileInfo.nFileSizeHigh);
    return (long)fileInfo.nFileSizeLow;
}

// Get MS Internet Explorer version by reading the registry
// Set Return major version in m_dwIEMajorVersion
// Return S_OK for success, error otherwise S_FALSE
// Should work with Windows 95/98/ME/NT/2000/XP/Vista
// Comments: alternative way to retrieve value is using WebBrowserControl of access to dll version info
// pNavigator->get_appVersion(&bstrVersion) - alternative way to retrieve 
LRESULT CSupportMessengerApp::GetIEVersion()
{
	int	  iMajor = 0; 
	LONG  lResult = 0;
    HKEY  hKey;
	DWORD dwSize=100, dwType = 0;
	TCHAR szVAL[MAX_KEY_LEN] = {0} ;

    // Open the key for query access
	lResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    LPCTSTR("SOFTWARE\\Microsoft\\Internet Explorer"),
					0,KEY_QUERY_VALUE,&hKey);

	if(lResult != ERROR_SUCCESS)   // Unable to open Key
	{
		return S_FALSE;
	}

  	lResult=::RegQueryValueEx(hKey,LPTSTR("Version"), NULL, &dwType, LPBYTE(szVAL),&dwSize);

	if(lResult != ERROR_SUCCESS)    // Unable to get value
	{
		// Close the key before quitting
		lResult=::RegCloseKey(hKey);
		m_dwIEMajorVersion = 6;//Assume that default version is 6 - not possible case
		Log.Add(_ERROR_, _T("Internet Explorer version failed in RegQueryValueEx with code: %d"), lResult);	
		return S_FALSE;
	}

	// Close the key
    lResult=::RegCloseKey(hKey);
	CString sFullVersion(szVAL);

	CString sUrl = theApp.m_cSettings.m_sBaseUrlPickUp;
	int	ind = 0;
	ind = sFullVersion.Find(".");		//	first appearence of ","
	m_dwIEMajorVersion =  atoi(sFullVersion.Left(ind));
	
	Log.Add(_MESSAGE_, _T("Internet Explorer Version: %s. Major Version: %d"), sFullVersion ,m_dwIEMajorVersion);	

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//								BackupLogfile
//
//  Description:
//
//  If logfile size is greater then m_cSettings.m_dwMaxLogfileSize then 
//  it may be renamed to SupportCenter.Log.02-14-2008-10-04
//  and if there are more then MAX_NUMBER_OF_LOGS already backuped then the oldest one may be removed
//
//	Parameters:
//
//	logDirectory - full path to the log folder
//  fileName	 - filename for backup 
//
//	Return Values
//	If the function succeeds, the return value is nonzero.
//  If the function fails, the return value is zero and write log.
//////////////////////////////////////////////////////////////////////////////////////////
BOOL CSupportMessengerApp::BackupLogfile(const TCHAR* logDirectory, const TCHAR* fileName)
{
	DWORD dwError=0;
	TCHAR szFullLogFileName[MAX_PATH] = {0};
    WIN32_FILE_ATTRIBUTE_DATA   fileInfo;
	
    if(fileName==NULL || logDirectory==NULL){
		Log.Add(_ERROR_, _T("fileName or logDirectory not specified "));
		return FALSE;
	}

	_stprintf_s(szFullLogFileName, _T("%s\\%s"),logDirectory,fileName);

    if(GetFileAttributesEx(szFullLogFileName, GetFileExInfoStandard, (void*)&fileInfo)==FALSE){
	   dwError = GetLastError();
	   if(dwError == ERROR_FILE_NOT_FOUND)
		   Log.Add(_MESSAGE_, _T("BackupLogfile:The system cannot find the file specified: %s"), szFullLogFileName );
	   else
		   Log.Add(_ERROR_, _T("GetFileAttributesEx failed (%u) for file %s"), dwError, szFullLogFileName );
	   return FALSE;
	}
	
	if((long)fileInfo.nFileSizeLow < m_cSettings.m_dwMaxLogfileSize)
	{
		Log.Add(_MESSAGE_, _T("Nothing to backup. LogFileSize is: %d. MaxLogfileSize is: %d"),
			(long)fileInfo.nFileSizeLow, m_cSettings.m_dwMaxLogfileSize);
		return TRUE;
	}

	TCHAR szNewFullLogFileName[MAX_PATH] = {0};
	TCHAR szDisplayTime[MAX_PATH] = {0};
	TCHAR szFindPattern[MAX_PATH] = {0};

	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	FILETIME lFileTimeOldest;
	TCHAR szLogFileOldest[MAX_PATH] = {0};
	DWORD dwCountLogFiles = 0;
	
	//  1) backup file like SupportCenter.Log as SupportCenter.Log.02-14-2008-10-04
	GetDiplayTime(fileInfo.ftLastAccessTime, szDisplayTime);
	_stprintf_s( szNewFullLogFileName, _T("%s.%s"), szFullLogFileName, szDisplayTime);
	
	if(MoveFile(szFullLogFileName, szNewFullLogFileName)==FALSE)
	{
		dwError = GetLastError();
		Log.Add(_ERROR_, _T("MoveFile failed:%u. FullLogFileName:%s, NewFullLogFileName:%s"),
			dwError,szFullLogFileName, szNewFullLogFileName);
		return FALSE;
	}

	//	2) Scan directory for pattern like "C:\\Program Files\\SupportSpace\\Support Platform\\Logs\\SupportCenter.Log.*"
	_stprintf_s(szFindPattern, _T("%s\\%s.*"), logDirectory, fileName);
	hFind = FindFirstFile(szFindPattern, &ffd);

	if(INVALID_HANDLE_VALUE == hFind) 
	{
		dwError = GetLastError();
		Log.Add(_ERROR_, _T("FindFirstFile failed (%u)"),dwError);
		return FALSE;
	} 
	else 
	{
		dwCountLogFiles++;
		Log.Add(_MESSAGE_, _T("Find First: %s"), ffd.cFileName);
		lFileTimeOldest = ffd.ftCreationTime;
		_stprintf_s(szLogFileOldest, _T("%s\\%s"),logDirectory, ffd.cFileName);

		// List all the other files in the directory.
		while(FindNextFile(hFind, &ffd) != 0) 
		{
			dwCountLogFiles++;

			if(CompareFileTime(&lFileTimeOldest, &ffd.ftCreationTime)==1)
			{
				lFileTimeOldest = ffd.ftCreationTime;
				_stprintf_s(szLogFileOldest, _T("%s\\%s"),logDirectory, ffd.cFileName);
				Log.Add(_MESSAGE_, _T("Currently looks LogFileOldest is: %s"), szLogFileOldest);
			}
			Log.Add(_MESSAGE_, _T(" Next log number %d filename: %s"), dwCountLogFiles, ffd.cFileName);
		}

		dwError = GetLastError();
		if (dwError != ERROR_NO_MORE_FILES) 
		{
			Log.Add(_MESSAGE_, _T("FindNextFile returned NO_MORE_FILES (%u)"),dwError);
		}
	}

	FindClose(hFind);

	if(dwCountLogFiles > m_cSettings.m_dwMaxNumOfLogs)
	{
		if(DeleteFile(szLogFileOldest)==FALSE)
		{
			dwError = GetLastError();
			Log.Add(_ERROR_, _T("DeleteFile failed (%u)"), dwError);
			return FALSE;
		}
		Log.Add(_MESSAGE_, _T("Oldest logfile deleted: %s"), szLogFileOldest);
	}
	else
	{
		Log.Add(_MESSAGE_, _T("No logfile to delete. Num of logfiles is: %d. Maximum logs to save is: %d"),
			dwCountLogFiles, m_cSettings.m_dwMaxNumOfLogs);		
	}

	return TRUE;
}

// GetDiplayTime - Convert ftCreate time to a string
//
// Return value - TRUE if successful, FALSE otherwise
// hFile      - Valid file handle
// lpszString - Pointer to buffer to receive string
BOOL CSupportMessengerApp::GetDiplayTime(FILETIME ftCreate, LPTSTR lpszString)
{
    SYSTEMTIME stUTC, stLocal;

    // Convert the last-write time to local time.
    FileTimeToSystemTime(&ftCreate, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

    // Build a string showing the date and time.
    wsprintf(lpszString, TEXT("%02d-%02d-%d-%02d-%02d-%02d"),
        stLocal.wMonth, stLocal.wDay, stLocal.wYear,
        stLocal.wHour, stLocal.wMinute, stLocal.wSecond );

    return TRUE;
}

BOOL CSupportMessengerApp::LauchWorkbenchProcess(CString sFullUrl, eCallType callType, CString sCustomerDisplayName, long lUid)
{
TRY_CATCH

	STARTUPINFO si;
    PROCESS_INFORMATION pi;
	CString sCmdLine;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

	// http://jira/browse/STG-4441 if sCustomerDisplayName contains " then this may be \ before each "
	sCustomerDisplayName.Replace("\"", "\\\"");

	sCmdLine.FormatMessage(_T("%1!s!%2!s!.exe /workbench %3!s! %4!d! \"%5!s!\" %6!d!"), 
			theApp.m_sApplicationPath, 
			theApp.m_pszExeName, 
			sFullUrl, 
			callType,
			sCustomerDisplayName,
			lUid);

	//C:\Program Files\SupportSpace\Support Platform>SupportCenter.exe /workbench "http://ws-anatoly:8080" 0 "DispName" 10

    // Start the child process. 
    if(!CreateProcess( NULL,   // No module name (use command line)
		sCmdLine.GetBuffer(),        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) 
    {
		Log.Add(_ERROR_, _T("LauchWorkbenchProcess  CreateProcess failed (%d)"), GetLastError() );
        return FALSE;
    }

	// todo: can think to do some improvements to have list of process started then 

    // Wait until child process exits.
    // WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

CATCH_THROW(_T("CSupportMessengerDlg::LauchWorkbenchProcess"))
	return TRUE;
}

//  
//  HKEY_CURRENT_USER\Software\TeamViewer3
//  InstallationDirectory
//  C:\Program Files\TeamViewer3
//  C:\Program Files\TeamViewer3\TeamViewer.exe may exists
LRESULT CSupportMessengerApp::CheckTeamViewerInstalled()
{
	int	  iMajor = 0; 
	LONG  lResult = 0;
    HKEY  hKey;
	DWORD dwSize=100, dwType = 0;
	TCHAR szVAL[MAX_KEY_LEN] = {0} ;

    // Open the key for query access
	lResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    LPCTSTR("SOFTWARE\\TeamViewer3"),
					0,KEY_QUERY_VALUE,&hKey);

	if(lResult != ERROR_SUCCESS)   // Unable to open Key
	{
		return S_FALSE;
	}

  	lResult=::RegQueryValueEx(hKey,LPTSTR("InstallationDirectory"), NULL, &dwType, LPBYTE(szVAL),&dwSize);

	if(lResult != ERROR_SUCCESS)    // Unable to get value
	{
		// Close the key before quitting
		lResult=::RegCloseKey(hKey);
		Log.Add(_ERROR_, _T("TeamViewer3 is not installed %d"), lResult);	
		return S_FALSE;
	}

	// Close the key
    lResult=::RegCloseKey(hKey);	

	CString sFullPathName;
	sFullPathName.Append(szVAL);
	sFullPathName.Append(_T("\\"));
	sFullPathName.Append(_T("TeamViewer.exe"));

	//ReadDirectory to check if installed 
	if(_taccess(sFullPathName,0) != 0)
	{
		Log.Add(_ERROR_, _T("_taccess file: %s failed (%d)"),sFullPathName, GetLastError() );
		return S_FALSE;
	}

	return S_OK;
}
