// CSupporterLocalData.h : header file for class that contains local data per supporter
//
//===========================================================================
// SupportMessengerApp ltd. @{SRCH}
//								CUpdateMonitor
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// SupportMessengerApp :	
//
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 10/29/2006 05:21:10 PM
// Comments: First Issue
//===========================================================================
#pragma once

#include "LocalDataDef.h"
#define MAX_PASSWORD_LEN 512
//	
//  
//	  
struct ActivateAfterCounter
{
	BOOL	bActivated;
	DWORD	iAfterCounter;//minutes or number
};
//
//
//
class CLoginInfo
{
public:
	CLoginInfo(const tstring&  supporterId)
	: m_supporterId( supporterId )
	{
	
	}
	~CLoginInfo(){};

	void	setPassword(const tstring&  password){ m_password = password; };
	tstring getPassword(){return m_password;};

	void	setRememberMe(BOOL	bRememberMe){m_bRememberMe = bRememberMe;};
	BOOL	getRememberMe(){return m_bRememberMe;};

	DWORD	getLastSelectedStatus(){return m_LastSelectedStatus;};
	void	setLastSelectedStatus(DWORD status){m_LastSelectedStatus = status;};

	BOOL	Load();
	BOOL	Save();

private:

	BOOL		EncryptAndSave(
		LPCTSTR			pszValueName,
		const tstring&  plain_text);
	
	BOOL		SaveToRegistry(
		LPCTSTR			pszValueName,
		PBYTE			pData,
		DWORD			dwSize);

	BOOL		LoadAndDecrypt(
		LPCTSTR			pszValueName,
		tstring&		plain_text);

	BOOL		LoadFromRegistry(
		LPCTSTR			pszValueName,
		PBYTE			pData, 
		DWORD*			pDataSize);

	tstring		m_supporterId;
	tstring		m_password;
	BOOL		m_bRememberMe;
	DWORD		m_LastSelectedStatus;
};
//
//	  CSupportSettings contains data will be shown using CSupporterSettings Dialog 		
//
class CSettings
{
public:
	CSettings(const tstring&  supporterId);
	~CSettings(void);

	BOOL	Load();
	BOOL	Save();

	BOOL	UpdateAutomaticallyRun(BOOL);
	BOOL	IsAutomaticallyRunChecked();
	BOOL	UpdateFirstTimeRunningFlag();

	BOOL	ReadTeamViewerNeverRemindMeSelection();
	BOOL	UpdateTeamViewerNeverRemindMeSelection(BOOL bAutomaticallyRun);

	//todo -  to implement sprint 4
	BOOL	UpdateFirewallPortSelection();
	//todo -  to implement sprint 4
	BOOL	RetrieveFirewallPortSelection();

//pravate: todo
	//	Genaral 
	BOOL	bAutomaticallyRun;				   
	BOOL	bOpenMainWindowOnMessangerStartUp; 

	BOOL	bPortOpened; 
	DWORD	dwPortOpenedNumber; 

	BOOL	m_bTeamViewerNeverRemindMe;
		
	//	Status
	ActivateAfterCounter	strShowAwayAfterBeingInActive;
	ActivateAfterCounter	strHandleCallsAndThenDisplayBusy;
	ActivateAfterCounter	strShowBusyAfterMissedCallsNum;

	BOOL					bShowAwayWhenScreenSaverIsOn;

	//	Notifications, Sounds & Alerts
	BOOL	bOnIncomingCallsShowAlert;
	BOOL	bOnIncomingCallsAnimateTrayIcon;
	BOOL	bPromptOnItemsOnLogout;
	BOOL	bPromptOnSnoozingItemsOnLogout;
	BOOL	bPlaySoundUponIncomingCall;
	BOOL	bPlaySoundUponConnectingToCustomer;

	//  Display
	DWORD   iDisplayXItemsAtTime;
	BOOL	bWindowOnTop;					   
	BOOL	bDockWindow;					   
	
	//	CallItemSortingOrderForInboxTab; //todo 
	//	CallItemSortingOrderForInSessionTab; //todo
	//	Status	//todo onnline/offline

	static BOOL DefaultTeamViewerNeverRemindMe;

	static BOOL DefaultAutomaticallyRun;

	static BOOL  DefaultPortOpened;
	static DWORD DefaultPortOpenedNumber;

	static BOOL DefaultOpenMainWindowOnMessangerStartUp;
	static BOOL DefaultWindowOnTop;			
	static BOOL DefaultDockWindow;		

	static BOOL	DefaultShowAwayWhenScreenSaverIsOn;

	static BOOL DefaultOnIncomingCallsShowAlert;
	static BOOL DefaultOnIncomingCallsAnimateTrayIcon;
	static BOOL DefaultPromptOnItemsOnLogout;
	static BOOL DefaultPromptOnSnoozingItemsOnLogout;
	static BOOL DefaultPlaySoundUponIncomingCall;
	static BOOL DefaultPlaySoundUponConnectingToCustomer;

	static BOOL  DefaultShowAwayAfterBeingInActive;
	static DWORD DefaultShowAwayAfterBeingInActiveMinutes;

	static BOOL  DefaultHandleCallsAndThenDisplayBusy;
	static DWORD DefaultHandleCallsAndThenDisplayBusyNumber;

	static BOOL  DefaultShowBusyAfterMissedCalls; 
	static DWORD DefaultShowBusyAfterMissedCallsNum; 

	static DWORD DefaultDisplayXItemsAtTime;

private:
	tstring	m_supporterId;
};

class CSupporterLocalData
{
public:
	CSupporterLocalData(const tstring&  supporterId)
	: m_supporterId( supporterId )
	{
		m_pcSettings = new CSettings(supporterId);
		m_pcLoginInfo = new CLoginInfo(supporterId);
	};

	~CSupporterLocalData()
	{
		if(m_pcSettings) 
			delete m_pcSettings;

		if(m_pcLoginInfo)
			delete m_pcLoginInfo;
	};
	
	BOOL			 Remove();
	BOOL			 Add();

	CSettings*		 getSettings(){return m_pcSettings; };
	CLoginInfo*		 getLoginInfo(){return m_pcLoginInfo; };

private:
	CSettings*		 m_pcSettings;				 //
	CLoginInfo*		 m_pcLoginInfo;				 //	

	tstring			 m_supporterId;				 //
};