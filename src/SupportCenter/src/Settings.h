#pragma once

#include "AidLib\Strings\tstring.h"
#include <io.h>

#define CONFIG_INI_FILENAME _T("config.ini") //TODO store in the regestry ?
#define MAX_KEY_LEN			256
#define MAX_LOGFILE_SIZE	20000000 //20MB 
#define MAX_NUMBER_OF_LOGS  3
#define SET_DEFAULT_SECURITY_ZONE  -1  // Trusted will be defult if not specified in config.ini

enum	DhtmlGuiLocation
{
	GuiLocationResource,
	GuiLocationFile,
	GuiLocationURL,
};

typedef struct _IMConnectivityQuality
{
	int PollingInterval;
	int TimePeriodForMonotoring;
	int MaxNumOfUnacceptableMsgs;
	int AcceptableRoundtripTimeLimit;
	int LogLevel;

}IMConnectivityQuality;

class CSettingsIni
{
public:
	CSettingsIni(void){};
	~CSettingsIni(void){};

    tstring				m_sResource;
	tstring				m_sServer;
	tstring				m_sServerAddr;

	bool				m_bLog ;
	DWORD				m_dwIdleTimeout;

	CString				m_sBaseUrlPickUp;
	CString				m_sBaseUrlPickupConsult;
	CString				m_sBaseUrlUpdateDownload;
	CString				m_sBaseUrlSupporterProfile;
	CString				m_sBaseUrlSupporterInbox;
	CString				m_sBaseUrlSupporterSessionHistory;
	CString				m_sBaseUrlHelp;
	CString				m_sBaseUrlExpertChat;
	CString				m_sBaseUrlExpertForum;
	CString				m_sBaseUrlExpertPortal;
	CString				m_sTeamViewerDownload;


	DhtmlGuiLocation	m_eGUIlocation;
	CString				m_sGUIlocationFilePath;
	CString				m_sGUIlocationUrl;

	CString				m_sPendingSupportRequestsTo;
	CString				m_sPendingSupportRequestsToResource;

	CString				m_szRelayServerAddress;
	unsigned short		m_dwRelayServerPort; 
	long				m_dwMaxLogfileSize;
	long				m_dwMaxNumOfLogs;

	DWORD				m_dwWorkbecnOpenMode;
	DWORD				m_dwMapUrlToZone;

	CString				m_sWorkbenchBrowser;

	IMConnectivityQuality m_stIMConnectivityQuality; 

public:
	LRESULT ReadConnectionInfo();
private:
	bool    InitFromIniFile(CString szFileName);
};

