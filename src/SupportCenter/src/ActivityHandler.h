// ActivityHandler.h : header file
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CActivityHandler
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CActivityHandler :	class is responsoble for Supporter Activity handling
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

#define  WM_ACTIVITY_HANDLER_INACTIVE		 20100	
#define  WM_ACTIVITY_HANDLER_BACK_ACTIVE	 20101	

// CActivityHandler

class CActivityHandler : public CWnd
{
	DECLARE_DYNAMIC(CActivityHandler)

public:
	CActivityHandler();
	virtual ~CActivityHandler();

	void  Init(CWnd* pParentWnd);
	void  SetInActiveTimeout(BOOL bShowAwayWhenScreenSaverIsOn, BOOL bActivateFeature, UINT iUserSelectedInactiveTimeout);
	BOOL  IsInActivateFeatureStarted(){ return m_bActivateFeature;} ;

	LRESULT GetScreenSaverSettings();
	BOOL	IsScreenSaveActive(){ return m_bScreenSaveActive;} ;
	DWORD   ScreenSaveTimeOut(){ return m_dwScreenSaveTimeOut;} ;


private:
	UINT  StartInactivityTimer (UINT TimerDuration);
	BOOL  StopInactivityTimer();

	void  StartActivityBackTimer();
	void  StopActivityBackTimer();

	BOOL  IsInActivityDetected(UINT   iMaxInActivityDurationm, DWORD* pdwIdleTimeout);
	DWORD GetIdleTimeout();
	void  StartScreenSaverTimer(DWORD dwScreenSaveTimeOut);
	BOOL  StopScreenSaverTimer();
	BOOL  IsScreenSaverRunning();

//
	BOOL   m_bShowAwayWhenScreenSaverIsOn;
	BOOL   m_bScreenSaveActive;
	DWORD  m_dwScreenSaveTimeOut;
	UINT   m_ScreenSaverTimerVal;

	UINT   m_iUserSelectedInactiveTimeout;
	BOOL   m_bActivateFeature;

	UINT   m_InactivituTimerVal;
	UINT   m_ActivityBackTimerVal;

	BOOL   m_bInitialized;

protected:
	DECLARE_MESSAGE_MAP()
    afx_msg void OnTimer(UINT nIDEvent);
};


