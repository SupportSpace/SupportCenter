#include "stdafx.h"
#include "SupporterLocalData.h"
#include <atlbase.h>
#include "ProtectedData.h"
#include <windows.h>
#include <wincrypt.h>
#include "SupportMessenger.h"


//===========================================================================
//	Default values for local settings
//===========================================================================
BOOL  CSettings::DefaultShowAwayWhenScreenSaverIsOn = TRUE;
BOOL  CSettings::DefaultOnIncomingCallsAnimateTrayIcon = TRUE;
BOOL  CSettings::DefaultOnIncomingCallsShowAlert = TRUE;
BOOL  CSettings::DefaultPromptOnItemsOnLogout  = FALSE;
BOOL  CSettings::DefaultPromptOnSnoozingItemsOnLogout  = TRUE;
BOOL  CSettings::DefaultPlaySoundUponIncomingCall  = TRUE;
BOOL  CSettings::DefaultPlaySoundUponConnectingToCustomer  = TRUE;
BOOL  CSettings::DefaultShowAwayAfterBeingInActive  = TRUE;
DWORD CSettings::DefaultShowAwayAfterBeingInActiveMinutes = 10;
BOOL  CSettings::DefaultHandleCallsAndThenDisplayBusy = FALSE; // TODO - in the end of Sprint 3 Product changed requirement
DWORD CSettings::DefaultHandleCallsAndThenDisplayBusyNumber = 1;
DWORD CSettings::DefaultDisplayXItemsAtTime = 20;
BOOL  CSettings::DefaultShowBusyAfterMissedCalls = TRUE; 
DWORD CSettings::DefaultShowBusyAfterMissedCallsNum = 3; 

BOOL  CSettings::DefaultTeamViewerNeverRemindMe = FALSE;
BOOL  CSettings::DefaultAutomaticallyRun = TRUE;
BOOL  CSettings::DefaultOpenMainWindowOnMessangerStartUp = FALSE;
BOOL  CSettings::DefaultWindowOnTop = TRUE;
BOOL  CSettings::DefaultDockWindow = TRUE;

BOOL  CSettings::DefaultPortOpened = FALSE;
DWORD CSettings::DefaultPortOpenedNumber = 2203;

/**
	* Constructs a new CLocalSettings 
	* @param 
*/
CSettings::CSettings(const tstring&  supporterId)
{
	bShowAwayWhenScreenSaverIsOn = DefaultShowAwayWhenScreenSaverIsOn;
	bOnIncomingCallsShowAlert = DefaultOnIncomingCallsShowAlert;
	bOnIncomingCallsAnimateTrayIcon = DefaultOnIncomingCallsAnimateTrayIcon;
	bPromptOnItemsOnLogout = DefaultPromptOnItemsOnLogout;
	bPromptOnSnoozingItemsOnLogout = DefaultPromptOnSnoozingItemsOnLogout;
	bPlaySoundUponIncomingCall = DefaultPlaySoundUponIncomingCall;
	bPlaySoundUponConnectingToCustomer = DefaultPlaySoundUponConnectingToCustomer;

	strShowAwayAfterBeingInActive.bActivated = DefaultShowAwayAfterBeingInActive;
	strShowAwayAfterBeingInActive.iAfterCounter = DefaultShowAwayAfterBeingInActiveMinutes;

	strHandleCallsAndThenDisplayBusy.bActivated = DefaultHandleCallsAndThenDisplayBusy;
	strHandleCallsAndThenDisplayBusy.iAfterCounter = DefaultHandleCallsAndThenDisplayBusyNumber;//todo number of calls

	strShowBusyAfterMissedCallsNum.bActivated = DefaultShowBusyAfterMissedCalls;
	strShowBusyAfterMissedCallsNum.iAfterCounter = DefaultShowBusyAfterMissedCallsNum;

	iDisplayXItemsAtTime = DefaultDisplayXItemsAtTime;

	bAutomaticallyRun = DefaultAutomaticallyRun;
	bOpenMainWindowOnMessangerStartUp = DefaultOpenMainWindowOnMessangerStartUp;
	bWindowOnTop = DefaultWindowOnTop;
	bDockWindow = DefaultDockWindow;

	bPortOpened = DefaultPortOpened;//todo - another registry path
	dwPortOpenedNumber = DefaultPortOpenedNumber;//todo - another default value

	m_bTeamViewerNeverRemindMe = DefaultTeamViewerNeverRemindMe;

	m_supporterId = supporterId;
}

CSettings::~CSettings(void)
{
}
/**
	* SaveToRegistry local settings
	* @param 
*/
BOOL CSettings::Save()
{
	HRESULT  hResult = S_OK;
	CRegKey  m_regKey;

	TCHAR	 szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };
	_stprintf_s( szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s\\%s\\%s"),
		szBaseRegistryPath, szSupportersKey ,m_supporterId.c_str(), szSupportersSettingsSubKey);//todo

	if(m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS)
		return FALSE;

	//	Genaral 
	hResult = m_regKey.SetDWORDValue( sAutomaticallyRun, bAutomaticallyRun);
	
	if(IsAutomaticallyRunChecked()!=bAutomaticallyRun)
		UpdateAutomaticallyRun(bAutomaticallyRun);

	UpdateFirewallPortSelection();
		
	hResult = m_regKey.SetDWORDValue( sOpenMainWindowOnMessangerStartUp,bOpenMainWindowOnMessangerStartUp);
	
	//	Status
	hResult = m_regKey.SetDWORDValue( sShowAwayAfterBeingInActive,strShowAwayAfterBeingInActive.bActivated) ;
	hResult = m_regKey.SetDWORDValue( sShowAwayAfterBeingInActiveMinutes,strShowAwayAfterBeingInActive.iAfterCounter);
	hResult = m_regKey.SetDWORDValue( sShowAwayWhenScreenSaverIsOn, bShowAwayWhenScreenSaverIsOn) ;
	
	hResult = m_regKey.SetDWORDValue( sHandleCallsAndThenDisplayBusy,strHandleCallsAndThenDisplayBusy.bActivated) ;
	hResult = m_regKey.SetDWORDValue( sHandleCallsAndThenDisplayBusyCounter,strHandleCallsAndThenDisplayBusy.iAfterCounter) ;
	//hResult = m_regKey.SetDWORDValue( sShowBusyAfterMissedCallsNum,strShowBusyAfterMissedCallsNum.bActivated) ;
	//hResult = m_regKey.SetDWORDValue( sShowBusyAfterMissedCallsNumCounter,strShowBusyAfterMissedCallsNum.iAfterCounter) ;

	//	Notifications, Sounds & Alerts
	hResult = m_regKey.SetDWORDValue( sOnIncomingCallsAnimateTrayIcon,bOnIncomingCallsAnimateTrayIcon);
	hResult = m_regKey.SetDWORDValue( sOnIncomingCallsShowAlert,bOnIncomingCallsShowAlert);
	hResult = m_regKey.SetDWORDValue( sPromptOnSnoozingItemsOnLogout,bPromptOnSnoozingItemsOnLogout);
	hResult = m_regKey.SetDWORDValue( sPromptOnItemsOnLogout,bPromptOnItemsOnLogout);
	hResult = m_regKey.SetDWORDValue( sPlaySoundUponIncomingCall,bPlaySoundUponIncomingCall) ;
	hResult = m_regKey.SetDWORDValue( sPlaySoundUponConnectingToCustomer,bPlaySoundUponConnectingToCustomer);

	//  Display
	hResult = m_regKey.SetDWORDValue( sDisplayXItemsAtTime,iDisplayXItemsAtTime);
	hResult = m_regKey.SetDWORDValue( sWindowOnTop,bWindowOnTop);
	hResult = m_regKey.SetDWORDValue( sDockWindow,bDockWindow);
	
	m_regKey.Close();
	return TRUE;	
}
/**
	* LoadFromRegistry local settings
	* @param 
*/
BOOL CSettings::Load()
{
	CRegKey  m_regKey;
	HRESULT	 lResult = S_OK;
	TCHAR	szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };

	_stprintf_s( szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s\\%s\\%s"),
		szBaseRegistryPath, szSupportersKey, m_supporterId.c_str(), szSupportersSettingsSubKey);//todo

	if( m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS )
		return FALSE;

	UpdateFirstTimeRunningFlag();

	RetrieveFirewallPortSelection();

	//	Genaral 
	lResult = m_regKey.QueryDWORDValue( sAutomaticallyRun,(DWORD&)bAutomaticallyRun);
	bAutomaticallyRun = IsAutomaticallyRunChecked();

	ReadTeamViewerNeverRemindMeSelection();


	lResult = m_regKey.QueryDWORDValue( sOpenMainWindowOnMessangerStartUp, (DWORD&)bOpenMainWindowOnMessangerStartUp);
	
	//	Status
	lResult = m_regKey.QueryDWORDValue( sShowAwayAfterBeingInActive, (DWORD&) strShowAwayAfterBeingInActive.bActivated);
	lResult = m_regKey.QueryDWORDValue( sShowAwayAfterBeingInActiveMinutes, (DWORD&) strShowAwayAfterBeingInActive.iAfterCounter);
	lResult = m_regKey.QueryDWORDValue( sHandleCallsAndThenDisplayBusy,(DWORD&) strHandleCallsAndThenDisplayBusy.bActivated);
	lResult = m_regKey.QueryDWORDValue( sHandleCallsAndThenDisplayBusyCounter,(DWORD&) strHandleCallsAndThenDisplayBusy.iAfterCounter);
	lResult = m_regKey.QueryDWORDValue( sShowBusyAfterMissedCallsNum,(DWORD&) strShowBusyAfterMissedCallsNum.bActivated);
	lResult = m_regKey.QueryDWORDValue( sShowBusyAfterMissedCallsNumCounter,(DWORD&) strShowBusyAfterMissedCallsNum.iAfterCounter);
	lResult = m_regKey.QueryDWORDValue( sShowAwayWhenScreenSaverIsOn, (DWORD&)bShowAwayWhenScreenSaverIsOn) ;

	//	Notifications, Sounds & Alerts
	lResult = m_regKey.QueryDWORDValue( sOnIncomingCallsShowAlert, (DWORD&)bOnIncomingCallsShowAlert);
	lResult = m_regKey.QueryDWORDValue( sOnIncomingCallsAnimateTrayIcon, (DWORD&)bOnIncomingCallsAnimateTrayIcon);
	lResult = m_regKey.QueryDWORDValue( sPromptOnItemsOnLogout,(DWORD&)bPromptOnItemsOnLogout);
	lResult = m_regKey.QueryDWORDValue( sPromptOnSnoozingItemsOnLogout, (DWORD&)bPromptOnSnoozingItemsOnLogout);
	lResult = m_regKey.QueryDWORDValue( sPlaySoundUponIncomingCall,(DWORD&)bPlaySoundUponIncomingCall);
	lResult = m_regKey.QueryDWORDValue( sPlaySoundUponConnectingToCustomer,(DWORD&)bPlaySoundUponConnectingToCustomer);

	//  Display
	lResult = m_regKey.QueryDWORDValue( sDisplayXItemsAtTime, (DWORD&)iDisplayXItemsAtTime);
	lResult = m_regKey.QueryDWORDValue( sWindowOnTop, (DWORD&)bWindowOnTop);
	lResult = m_regKey.QueryDWORDValue( sDockWindow,(DWORD&)bDockWindow);

	m_regKey.Close();
	return TRUE;
}

BOOL CSupporterLocalData::Remove()
{
	CRegKey  m_regKey;
	DWORD	 dwValue = 0;
	DWORD    dwTypeDword = REG_DWORD;
	ULONG	 nBytes = sizeof(dwValue);	
	HRESULT	 lResult = S_OK;

	TCHAR	szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };
	_stprintf_s( szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s"),szBaseRegistryPath,szSupportersKey);//todo

	if( m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS )
		return FALSE;

	lResult = m_regKey.RecurseDeleteKey( m_supporterId.c_str() );

	m_regKey.Close();
	return TRUE;
}

BOOL CSupporterLocalData::Add()
{
	CRegKey  m_regKey;
	DWORD	 dwValue = 0;
	DWORD    dwTypeDword = REG_DWORD;
	ULONG	 nBytes = sizeof(dwValue);	
	HRESULT	 lResult = S_OK;

	TCHAR	szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };
	_stprintf_s( szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s\\%s"),
		szBaseRegistryPath,szSupportersKey,m_supporterId.c_str());//todo

	if( m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS )
		return FALSE;

	m_regKey.Close();
	return TRUE;
}

BOOL CLoginInfo::Load()
{
	CRegKey  m_regKey;
	HRESULT	 lResult = S_OK;

	TCHAR	szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };
	_stprintf_s( szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s\\%s\\%s"),
		szBaseRegistryPath, szSupportersKey, m_supporterId.c_str(), szSupportersLoginInfoSubKey);//todo

	if( m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS )
		return FALSE;
	
	lResult = m_regKey.QueryDWORDValue(sRememberMe,(DWORD&)m_bRememberMe);
	
	if(m_bRememberMe==TRUE)
		LoadAndDecrypt(sPassword, m_password);

	lResult = m_regKey.QueryDWORDValue(sLastSelectedStatus,(DWORD&)m_LastSelectedStatus);
	
	m_regKey.Close();
	return TRUE;
}

BOOL CLoginInfo::Save()
{
	HRESULT  hResult = S_OK;
	CRegKey  m_regKey;

	TCHAR	 szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };
	_stprintf_s(szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s\\%s\\%s"), 
		szBaseRegistryPath, szSupportersKey ,m_supporterId.c_str(), szSupportersLoginInfoSubKey);//todo

	if( m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath )!=ERROR_SUCCESS )
		return FALSE;

	//change flag - todo not need it actually
	hResult = m_regKey.SetDWORDValue(sRememberMe, m_bRememberMe);

	if(m_bRememberMe==TRUE)
	{
		EncryptAndSave(sPassword, m_password);
	}
	else
	{
		m_regKey.DeleteValue(sPassword);
	}

	hResult = m_regKey.SetDWORDValue(sLastSelectedStatus, m_LastSelectedStatus);

	m_regKey.Close();
	return TRUE;
}

BOOL CLoginInfo::EncryptAndSave(
				LPCTSTR			pszValueName,
				const tstring&  plain_text)
{
	CProtectedData		pData;
	DATA_BLOB			protected_data_saved;
	TCHAR				str_plain_text[MAX_PASSWORD_LEN] = { 0 };

	_tcscpy_s(str_plain_text, MAX_PASSWORD_LEN ,plain_text.c_str());
	pData.CryptData(str_plain_text , &protected_data_saved);
	_tprintf(_T("CryptData() crypted data: %s"), protected_data_saved.pbData );

	SaveToRegistry(pszValueName,protected_data_saved.pbData, protected_data_saved.cbData );

//	CleanUp
	if(protected_data_saved.pbData)
	{
		::LocalFree(protected_data_saved.pbData);
		protected_data_saved.pbData = NULL;
		protected_data_saved.cbData = 0;
	}

	return TRUE;
}

BOOL CLoginInfo::SaveToRegistry(
			LPCTSTR			pszValueName,
			PBYTE			pData, 
			DWORD			dwSize)
{
	HRESULT  hResult = S_OK;
	CRegKey  m_regKey;

	TCHAR	 szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };
	_stprintf_s( szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s\\%s\\%s"),
		szBaseRegistryPath, szSupportersKey ,m_supporterId.c_str(), szSupportersLoginInfoSubKey);//todo

	if( m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath )!=ERROR_SUCCESS )
		return FALSE;

//	Genaral 
	hResult = m_regKey.SetBinaryValue(pszValueName, pData, dwSize) ;
	m_regKey.Close();
	return TRUE;
}

BOOL CLoginInfo::LoadAndDecrypt(
				LPCTSTR			pszValueName,
				tstring&		plain_text)
{
	CProtectedData		pData;
	DATA_BLOB			protected_data_loaded;
	TCHAR				str_out_decrypted[MAX_PASSWORD_LEN] = { 0 };

	DWORD	dwDataSize = 4096;
	byte	buf[4096] = { '\0' };   // make it big enough for any kind of values
	
	LoadFromRegistry(pszValueName, buf, &dwDataSize);

	protected_data_loaded.pbData = buf;
	protected_data_loaded.cbData = dwDataSize;
	
	pData.DeCryptData(&protected_data_loaded, str_out_decrypted);
	_tprintf(_T("DeCryptData() decrypted data: %s\n"), str_out_decrypted );

	plain_text = str_out_decrypted;

	return TRUE;
}

BOOL CLoginInfo::LoadFromRegistry(
			LPCTSTR			pszValueName,
			PBYTE			pData, 
			DWORD*			pDataSize)
{
	HRESULT  hResult = S_OK;
	CRegKey  m_regKey;

	TCHAR	 szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };
	_stprintf_s(szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s\\%s\\%s"),
		szBaseRegistryPath, szSupportersKey ,m_supporterId.c_str(), szSupportersLoginInfoSubKey);//todo

	if( m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath )!=ERROR_SUCCESS )
		return FALSE;

//	Genaral 
	hResult = m_regKey.QueryBinaryValue(pszValueName, pData, pDataSize);
	m_regKey.Close();
	return TRUE;
}

BOOL CSettings::UpdateAutomaticallyRun(BOOL bAutomaticallyRun)
{
	HRESULT	 lResult = S_OK;
	CRegKey  m_regKey;
	TCHAR	 szSupportMessngerAppPath[MAX_REGBUF_LEN] = {0};
														
	if( m_regKey.Create(HKEY_DEFINED_KEY, sCurrentUserWindowsRun)!=ERROR_SUCCESS )
		return FALSE;

	_stprintf_s(szSupportMessngerAppPath, MAX_REGBUF_LEN, _T("\"%s%s.exe\" %s"),
		theApp.m_sApplicationPath, theApp.m_pszExeName, CMD_LINE_PARAM_AUTOSTART);//todo
	
	if(bAutomaticallyRun)
		lResult = m_regKey.SetStringValue(sSupportMessngerAppName, szSupportMessngerAppPath);
	else
		lResult = m_regKey.DeleteValue(sSupportMessngerAppName);

	m_regKey.Close();
	return TRUE;	
}

BOOL CSettings::IsAutomaticallyRunChecked()
{
	HRESULT	 lResult = S_OK;
	CRegKey  m_regKey;
	BOOL	 bRetVal = 0;
	TCHAR	 szSupportMessngerAppPath[MAX_REGBUF_LEN] = {0};

	if( m_regKey.Create(HKEY_DEFINED_KEY, sCurrentUserWindowsRun)!=ERROR_SUCCESS )
		return FALSE;

	_stprintf_s(szSupportMessngerAppPath, MAX_REGBUF_LEN, _T("%s\\%s %s"),
		theApp.m_sApplicationPath, theApp.m_pszExeName, CMD_LINE_PARAM_AUTOSTART);//todo

	ULONG	 nBytes = 0;
	nBytes = sizeof(szSupportMessngerAppPath);
	
	lResult = m_regKey.QueryStringValue(sSupportMessngerAppName, szSupportMessngerAppPath, &nBytes);
	if(lResult==S_OK)
	{
		bRetVal = TRUE;
	}

	m_regKey.Close();
	return bRetVal;	
}

BOOL CSettings::UpdateFirstTimeRunningFlag()
{
	HRESULT  hResult = S_OK;
	CRegKey  m_regKey;
	BOOL	bFirstTimeRunning = FALSE;

	TCHAR	 szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };

	_stprintf_s(szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s"),
		szBaseRegistryPath, szCommonPrefKey);//todo

	if(m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS)
		return FALSE;

	hResult = m_regKey.QueryDWORDValue(sFirstTimeRunning, (DWORD&)bFirstTimeRunning);
	if(hResult!=S_OK)
	{	
		//
		//	FirstTimeRunning detected. One of the actions is UpdateAutomaticallyRun once
		//
		bFirstTimeRunning = 0;
		hResult = m_regKey.SetDWORDValue(sFirstTimeRunning, bFirstTimeRunning);
		UpdateAutomaticallyRun(DefaultAutomaticallyRun);
	}
		
	m_regKey.Close();
	return TRUE;	
}

BOOL CSettings::UpdateFirewallPortSelection()
{
	HRESULT  hResult = S_OK;
	CRegKey  m_regKey;

	TCHAR	 szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };

	_stprintf_s(szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s"),
		szBaseRegistryPath, szCommonPrefKey);//todo

	if(m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS)
		return FALSE;

	hResult = m_regKey.SetDWORDValue( sPortOpened, bPortOpened);//todo - another registry path
	hResult = m_regKey.SetDWORDValue( sPortOpenedNumber, dwPortOpenedNumber);//todo - another registry path
		
	m_regKey.Close();
	return TRUE;	
}

BOOL CSettings::RetrieveFirewallPortSelection()
{
	CRegKey  m_regKey;
	HRESULT	 lResult = S_OK;
	TCHAR	szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };
	
	_stprintf_s(szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s"),
		szBaseRegistryPath, szCommonPrefKey);//todo

	if( m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS )
		return FALSE;

	lResult = m_regKey.QueryDWORDValue( sPortOpened, (DWORD&)bPortOpened);//todo - another registry path
	lResult = m_regKey.QueryDWORDValue( sPortOpenedNumber,(DWORD&)dwPortOpenedNumber);//todo - another registry path

	m_regKey.Close();
	return TRUE;
}

BOOL CSettings::UpdateTeamViewerNeverRemindMeSelection(BOOL bTeamViewerNeverRemindMe)
{
	HRESULT  hResult = S_OK;
	CRegKey  m_regKey;
	BOOL	bFirstTimeRunning = FALSE;

	TCHAR	 szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };

	_stprintf_s(szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s"),
		szBaseRegistryPath, szCommonPrefKey);//todo

	if(m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS)
		return FALSE;

	hResult = m_regKey.SetDWORDValue(sTeamViewerNeverRemindMe, bTeamViewerNeverRemindMe);
	
	m_regKey.Close();
	return TRUE;	
}

BOOL CSettings::ReadTeamViewerNeverRemindMeSelection()
{
	HRESULT	 lResult = S_OK;
	CRegKey  m_regKey;
	BOOL	 bRetVal = 0;
	TCHAR	 szSupportMessngerAppPath[MAX_REGBUF_LEN] = {0};

	TCHAR	 szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };

	_stprintf_s(szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s"),
		szBaseRegistryPath, szCommonPrefKey);//todo

	if( m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS )
		return FALSE;
	
	lResult = m_regKey.QueryDWORDValue( sTeamViewerNeverRemindMe, (DWORD&)m_bTeamViewerNeverRemindMe);//todo - another registry path
	if(lResult==S_OK)
	{
		bRetVal = TRUE;
	}

	m_regKey.Close();
	return bRetVal;	
}