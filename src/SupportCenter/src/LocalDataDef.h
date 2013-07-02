#pragma once

#include <tchar.h>
#include "AidLib\Strings\tstring.h"

#define MAX_REGBUF_LEN		1024
#define MAX_BIGBUF_LEN		2048

// HKEY_CURRENT_USER\<company>\<application>\<section>
#define	REGKEY_COMPANY  	_T("SupportSpace")			

//	Define the key handle 
#define HKEY_DEFINED_KEY	HKEY_CURRENT_USER

//===========================================================================
//	Registry root path and subkeys
//===========================================================================
static TCHAR   szBaseRegistryPath[] = _T("SOFTWARE\\SupportSpace\\SupportCenter");

//===========================================================================
// BasePath first level SubeKeys
// HKEY_CURRENT_USER\Software\SupportSpace\SupportMessenger
//===========================================================================
static TCHAR   szLastEntryKey[] =  _T("LastEntry");
static TCHAR   szSupportersKey[] = _T("Supporters");
static TCHAR   szCommonPrefKey[] = _T("CommonPref");

//===========================================================================
// LastEntry subkeys
// HKEY_CURRENT_USER\Software\SupportSpace\SupportMessenger\LastEntry
//===========================================================================
static TCHAR   sLastLogedInSupporterId[] = _T("LastLogedInSupporterId");
static TCHAR   sLastAppState[] = _T("LastAppState");

//===========================================================================
// Supporters subkeys (each subkey is under the username folder)	
// HKEY_CURRENT_USER\Software\SupportSpace\SupportMessenger\Supporters\<username>\LoginInfo 
//===========================================================================
static TCHAR   szSupportersSettingsSubKey[] = _T("Settings"); 
static TCHAR   szSupportersLoginInfoSubKey[] = _T("LoginInfo");

//===========================================================================
//	Registry names of local settings per supporter 
//  HKEY_CURRENT_USER\Software\SupportSpace\SupportMessenger\Supporters\<username>\Settings
//===========================================================================
static TCHAR   sOnIncomingCallsAnimateTrayIcon[] = _T("OnIncomingCallsAnimateTrayIcon");
static TCHAR   sOnIncomingCallsShowAlert[] = _T("OnIncomingCallsShowAlert");
static TCHAR   sPromptOnItemsOnLogout[] = _T("PromptOnItemsOnLogout");
static TCHAR   sPromptOnSnoozingItemsOnLogout[] = _T("PromptOnSnoozingItemsOnLogout");
static TCHAR   sPlaySoundUponIncomingCall[] = _T("PlaySoundUponIncomingCall");
static TCHAR   sPlaySoundUponConnectingToCustomer[] = _T("PlaySoundUponConnectingToCustomer");
static TCHAR   sShowAwayAfterBeingInActive[] = _T("ShowAwayAfterBeingInActive");
static TCHAR   sShowAwayAfterBeingInActiveMinutes[] = _T("ShowAwayAfterBeingInActiveMinutes");
static TCHAR   sShowAwayWhenScreenSaverIsOn[] = _T("ShowAwayWhenScreenSaverIsOn");
static TCHAR   sHandleCallsAndThenDisplayBusy[] = _T("HandleCallsAndThenDisplayBusy");
static TCHAR   sHandleCallsAndThenDisplayBusyCounter[] = _T("HandleCallsAndThenDisplayBusyCounter");
static TCHAR   sShowBusyAfterMissedCallsNum[] = _T("ShowBusyAfterMissedCallsNum");
static TCHAR   sShowBusyAfterMissedCallsNumCounter[] = _T("ShowBusyAfterMissedCallsNumCounter");
static TCHAR   sDisplayXItemsAtTime[] = _T("DisplayXItemsAtTime");
static TCHAR   sAutomaticallyRun[] = _T("AutomaticallyRun");
static TCHAR   sOpenMainWindowOnMessangerStartUp[] = _T("OpenMainWindowOnMessangerStartUp");
static TCHAR   sWindowOnTop[] = _T("WindowOnTop");
static TCHAR   sDockWindow[] = _T("DockWindow");

//===========================================================================
//	Registry names of local settings per Messenger 
//  HKEY_CURRENT_USER\Software\SupportSpace\SupportCenter\CommonPref\PortOpened
//===========================================================================
static TCHAR   sPortOpened[] = _T("PortOpened");
static TCHAR   sPortOpenedNumber[] = _T("PortOpenedNumber");
static TCHAR   sFirstTimeRunning[] = _T("FirstTimeRunning");
static TCHAR   sTeamViewerNeverRemindMe[] = _T("TeamViewerNeverRemindMe");

//===========================================================================
//	Registry names of LoginInfo per supporter 
//  HKEY_CURRENT_USER\Software\SupportSpace\SupportMessenger\Supporters\<username>\LoginInfo
//===========================================================================
static TCHAR   sPassword[] = _T("Password");
static TCHAR   sRememberMe[] = _T("RememberMe");
static TCHAR   sLastSelectedStatus[] = _T("LastSelectedStatus");

//===========================================================================
//  Automatically run specific keys
//  HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run\SupportMessenger or ??? todo Sprint3-4
//  HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Run\SupportMessenger
//===========================================================================
static TCHAR   sCurrentUserWindowsRun[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
static TCHAR   sSupportMessngerAppName[] = _T("SupportCenter");

// Update feature specific parameter TODO Sprint3-4 - may be agreed with installer
static TCHAR   sVersion[] = _T("Version");