#include "StdAfx.h"
#include "Settings.h"

LRESULT CSettingsIni::ReadConnectionInfo()
{
	return !InitFromIniFile(CONFIG_INI_FILENAME);
}

bool CSettingsIni::InitFromIniFile(CString szFileName)
{
	TCHAR	szServer[MAX_KEY_LEN] = {0} ;
	TCHAR	szServerAddress[MAX_KEY_LEN] = {0};
	TCHAR	szResource[MAX_KEY_LEN] = {0};
	TCHAR	szBaseUrlPickUp[MAX_KEY_LEN] = {0};
	TCHAR	szBaseUrlPickupConsult[MAX_KEY_LEN] = {0};
	TCHAR	szIdleTimeout[MAX_KEY_LEN] = {0};
	TCHAR	szDhtmlGuiLocation[MAX_KEY_LEN] = {0};
	TCHAR	szDhtmlGuiLocationFilePath[MAX_KEY_LEN] = {0};
	TCHAR	szBaseUrlUpdateDownload[MAX_KEY_LEN] = {0};
	TCHAR	szPendingSupportRequestsTo[MAX_KEY_LEN] = {0};
	TCHAR	szPendingSupportRequestsToResource[MAX_KEY_LEN] = {0};
	TCHAR	szBaseUrlSupporterProfile[MAX_KEY_LEN] = {0};
	TCHAR	szBaseUrlSupporterInbox[MAX_KEY_LEN] = {0};
	TCHAR	szBaseUrlSupporterSessionHistory[MAX_KEY_LEN] = {0};
	TCHAR	szBaseUrlHelp[MAX_KEY_LEN] = {0};
	TCHAR	szBaseUrlExpertChat[MAX_KEY_LEN] = {0};
	TCHAR	szBaseUrlExpertForum[MAX_KEY_LEN] = {0};
	TCHAR	szBaseUrlExpertPortal[MAX_KEY_LEN] = {0};
	TCHAR	szTeamViewerDownload[MAX_KEY_LEN] = {0};

	TCHAR	szRelayServerAddress[MAX_KEY_LEN] = {0};
	TCHAR	szRelayServerPort[MAX_KEY_LEN] = {0};
	TCHAR	szMaxLogfileSize[MAX_KEY_LEN] = {0};
	TCHAR	szMaxNumOfLogs[MAX_KEY_LEN] = {0};
	TCHAR	szWorkbecnOpenMode[MAX_KEY_LEN] = {0};
	TCHAR   szWorkbenchBrowser[MAX_KEY_LEN] = {0};
	TCHAR	szMapUrlToZone[MAX_KEY_LEN] = {0};

	TCHAR	szTmp[MAX_KEY_LEN] = {0};
		
	CString sFullPathName;
	TCHAR   sCurrentDirectory[MAX_PATH]  = { 0 };  
	//
	::GetCurrentDirectory(MAX_PATH, sCurrentDirectory) ;
	
	sFullPathName.Append(sCurrentDirectory);
	sFullPathName.Append(_T("\\"));
	sFullPathName.Append(szFileName);
	//
	//	.ini file exists ?
	//	------------------
	if( _taccess(sFullPathName,0) != 0 )
	{	
		CString sErrMsg;
		sErrMsg.FormatMessage(_T("Failed to access to config.ini Please check that file %1!s! exists."), sFullPathName);
		AfxMessageBox(sErrMsg);
		return false;
	}
	//
	//	Read file vaues.
	//	----------------
	GetPrivateProfileString(_T("ConnectionProfile"), 
		_T("Server"), 
		_T(""), 
		szServer, 
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("ConnectionProfile"), 
		_T("ServerAddress"), 
		_T(""), 
		szServerAddress, 
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("ConnectionProfile"), 
		_T("UserResource"), 
		_T(""), 
		szResource, 
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("ConnectionProfile"), 
		_T("IdleTimeout"), 
		_T(""), 
		szIdleTimeout, 
		MAX_KEY_LEN, 
		sFullPathName);
//section DhtmlGui
	GetPrivateProfileString(_T("DhtmlGui"), 
		_T("Location"), 
		_T(""), 
		szDhtmlGuiLocation, 
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("DhtmlGui"), 
		_T("LocationFilePath"), 
		_T(""), 
		szDhtmlGuiLocationFilePath, 
		MAX_KEY_LEN, 
		sFullPathName);

//section PendingSupportRequests	
	GetPrivateProfileString(_T("PendingSupportRequests"), 
		_T("To"), 
		_T(""), 
		szPendingSupportRequestsTo, 
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("PendingSupportRequests"), 
		_T("ToResource"), 
		_T(""), 
		szPendingSupportRequestsToResource, 
		MAX_KEY_LEN, 
		sFullPathName);

//section UrlRedirectionsProfile	
	GetPrivateProfileString(_T("UrlRedirectionsProfile"), 
		_T("Pickup"), 
		_T(""), 
		szBaseUrlPickUp, 
		MAX_KEY_LEN, 
		sFullPathName);
//
	GetPrivateProfileString(_T("UrlRedirectionsProfile"), 
		_T("PickupConsult"), 
		_T(""), 
		szBaseUrlPickupConsult, 
		MAX_KEY_LEN, 
		sFullPathName);
//
	GetPrivateProfileString(_T("UrlRedirectionsProfile"), 
		_T("UpdateDownload"), 
		_T(""), 
		szBaseUrlUpdateDownload, 
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("UrlRedirectionsProfile"), 
		_T("SupporterProfile"), 
		_T(""), 
		szBaseUrlSupporterProfile, 
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("UrlRedirectionsProfile"), 
		_T("SupporterInbox"), 
		_T(""), 
		szBaseUrlSupporterInbox, 
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("UrlRedirectionsProfile"), 
		_T("SupporterSessionHistory"), 
		_T(""), 
		szBaseUrlSupporterSessionHistory, 
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("UrlRedirectionsProfile"), 
		_T("Help"), 
		_T(""), 
		szBaseUrlHelp,  
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("UrlRedirectionsProfile"), 
		_T("WorkbechOpenMode"), 
		_T(""), 
		szWorkbecnOpenMode,  
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("UrlRedirectionsProfile"), 
		_T("WorkbenchBrowser"), 
		_T(""), 
		szWorkbenchBrowser,  
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("UrlRedirectionsProfile"), 
		_T("ExpertChat"), 
		_T(""), 
		szBaseUrlExpertChat,  
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("UrlRedirectionsProfile"), 
		_T("ExpertForum"), 
		_T(""), 
		szBaseUrlExpertForum,  
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("UrlRedirectionsProfile"), 
		_T("ExpertPortal"), 
		_T(""), 
		szBaseUrlExpertPortal,  
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("UrlRedirectionsProfile"), 
		_T("MapUrlToZone"), 
		_T(""), 
		szMapUrlToZone,  
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("UrlRedirectionsProfile"), 
		_T("TeamViewerDownload"), 
		_T(""), 
		szTeamViewerDownload,  
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("RelayServerConnectionProfile"), 
		_T("RelayServerAddress"), 
		_T(""), 
		szRelayServerAddress, 
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("RelayServerConnectionProfile"), 
		_T("RelayServerPort"), 
		_T(""), 
		szRelayServerPort, 
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("LogPolicy"), 
		_T("MaxLogFileSize"), 
		_T(""), 
		szMaxLogfileSize, 
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("LogPolicy"), 
		_T("MaxNumOfLogs"), 
		_T(""), 
		szMaxNumOfLogs, 
		MAX_KEY_LEN, 
		sFullPathName);

	GetPrivateProfileString(_T("IMConnectivityQuality"), 
		_T("PollingInterval"), 
		_T(""), 
		szTmp, 
		MAX_KEY_LEN, 
		sFullPathName);
	m_stIMConnectivityQuality.PollingInterval = _tstoi(szTmp);

	GetPrivateProfileString(_T("IMConnectivityQuality"), 
		_T("TimePeriodForMonitoring"), 
		_T(""), 
		szTmp, 
		MAX_KEY_LEN, 
		sFullPathName);
	m_stIMConnectivityQuality.TimePeriodForMonotoring = _tstoi(szTmp);

	GetPrivateProfileString(_T("IMConnectivityQuality"), 
		_T("MaxNumOfUnacceptableMsgs"), 
		_T(""), 
		szTmp, 
		MAX_KEY_LEN, 
		sFullPathName);
	m_stIMConnectivityQuality.MaxNumOfUnacceptableMsgs = _tstoi(szTmp);

	GetPrivateProfileString(_T("IMConnectivityQuality"), 
		_T("AcceptableRoundtripTime"), 
		_T(""), 
		szTmp, 
		MAX_KEY_LEN, 
		sFullPathName);
	m_stIMConnectivityQuality.AcceptableRoundtripTimeLimit = _tstoi(szTmp);

	GetPrivateProfileString(_T("IMConnectivityQuality"), 
		_T("LogLevel"), 
		_T(""), 
		szTmp, 
		MAX_KEY_LEN, 
		sFullPathName);
	m_stIMConnectivityQuality.LogLevel = _tstoi(szTmp);


	m_bLog = TRUE;// always TRUE - enable logs  

	//
	//	Check if the return values is correct.
	//	--------------------------------------
	if (szServer[0]== 0 || szServerAddress[0]==0 || szResource[0]==0)
		return false;

	m_sResource = szResource;
	m_sServer = szServer;
	m_sServerAddr = szServerAddress;

	m_sBaseUrlPickUp = szBaseUrlPickUp;
	m_sBaseUrlPickupConsult = szBaseUrlPickupConsult;
	m_sBaseUrlUpdateDownload = szBaseUrlUpdateDownload;
	m_sBaseUrlSupporterProfile = szBaseUrlSupporterProfile;
	m_sBaseUrlSupporterInbox = szBaseUrlSupporterInbox;
	m_sBaseUrlHelp = szBaseUrlHelp;
	m_sBaseUrlExpertChat = szBaseUrlExpertChat;
	m_sBaseUrlExpertForum = szBaseUrlExpertForum;
	m_sBaseUrlExpertPortal = szBaseUrlExpertPortal;
	m_sBaseUrlSupporterSessionHistory = szBaseUrlSupporterSessionHistory;
	m_sTeamViewerDownload = szTeamViewerDownload;

	m_dwIdleTimeout	 = _tstoi(szIdleTimeout);

	m_eGUIlocation 	 = (DhtmlGuiLocation)_tstoi(szDhtmlGuiLocation);
	if(m_eGUIlocation==GuiLocationFile)
		m_sGUIlocationFilePath = szDhtmlGuiLocationFilePath;

	m_sPendingSupportRequestsTo = szPendingSupportRequestsTo;
	m_sPendingSupportRequestsToResource = szPendingSupportRequestsToResource;


	m_szRelayServerAddress = szRelayServerAddress;
	m_dwRelayServerPort = _tstoi(szRelayServerPort); 

	m_dwMaxLogfileSize = _tstol(szMaxLogfileSize);
	if(m_dwMaxLogfileSize == 0)
		m_dwMaxLogfileSize = MAX_LOGFILE_SIZE;

	m_dwMaxNumOfLogs = _tstol(szMaxNumOfLogs);
	if(m_dwMaxNumOfLogs == 0)
		m_dwMaxNumOfLogs = MAX_NUMBER_OF_LOGS;

	m_dwWorkbecnOpenMode = _tstoi(szWorkbecnOpenMode);

	m_sWorkbenchBrowser = szWorkbenchBrowser;

	if(szMapUrlToZone[0]!='\0')
		m_dwMapUrlToZone =  _tstoi(szMapUrlToZone); 
	else
		m_dwMapUrlToZone = SET_DEFAULT_SECURITY_ZONE;
	
	return true;
}