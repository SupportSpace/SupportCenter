// ActivityHandler.cpp : implementation file
//

#include "stdafx.h"
#include "SupportMessenger.h"
#include "ActivityHandler.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"

#define  IDT_TIMER_0			 WM_USER + 200  
#define  IDT_TIMER_INACTIVE		 IDT_TIMER_0 + 1
#define  IDT_TIMER_ACTIVITY_BACK IDT_TIMER_0 + 2
#define  IDT_TIMER_SCREEN_SAVER	 IDT_TIMER_0 + 3 	

#define	 ACTIVITY_BACK_POLLING_INTERVAL 5000 

// CActivityHandler

IMPLEMENT_DYNAMIC(CActivityHandler, CWnd)

CActivityHandler::CActivityHandler()
: m_InactivituTimerVal(0),m_ActivityBackTimerVal(0),m_iUserSelectedInactiveTimeout(0),m_bActivateFeature(0),
m_bInitialized(FALSE),m_bShowAwayWhenScreenSaverIsOn(FALSE)
{
TRY_CATCH
CATCH_LOG(_T("CActivityHandler::CActivityHandler()"))
}

CActivityHandler::~CActivityHandler()
{
TRY_CATCH
CATCH_LOG(_T("CActivityHandler::~CActivityHandler()"))
}

BEGIN_MESSAGE_MAP(CActivityHandler, CWnd)
	  ON_WM_TIMER()
END_MESSAGE_MAP()
// CActivityHandler message handlers

void  CActivityHandler::Init(CWnd* pParentWnd)
{
TRY_CATCH
	if(m_bInitialized == FALSE)
	{
		if(Create(_T("STATIC"), _T("SMActivityWindow"), WS_CAPTION, CRect(0, 0, 0, 0), pParentWnd, 0)==TRUE)
			m_bInitialized = TRUE;
	}
CATCH_LOG(_T("CActivityHandler::Init"))
}

void  CActivityHandler::SetInActiveTimeout(BOOL bShowAwayWhenScreenSaverIsOn, BOOL bActivateFeature, UINT iUserSelectedInactiveTimeout)
{
TRY_CATCH
	if(m_bInitialized == FALSE)
		return;

	StopScreenSaverTimer();
	StopInactivityTimer();
	StopActivityBackTimer();

	//todo critical section here to protect access to m_bActivateFeature and m_iUserSelectedInactiveTimeout
	m_iUserSelectedInactiveTimeout = iUserSelectedInactiveTimeout;
	m_bActivateFeature = bActivateFeature;
	m_bShowAwayWhenScreenSaverIsOn = bShowAwayWhenScreenSaverIsOn;

	StartInactivityTimer(iUserSelectedInactiveTimeout);

	//ifActiveScreenSaver detection 
	GetScreenSaverSettings();
	StartScreenSaverTimer(m_dwScreenSaveTimeOut);

CATCH_LOG(_T("CActivityHandler::SetInActiveTimeout"))
}

void CActivityHandler::OnTimer(UINT nIDEvent)
{
TRY_CATCH
	DWORD	dwIdleTimeout = 0;

	//todo critical section here to protect access to m_bActivateFeature and m_iUserSelectedInactiveTimeout
	switch(nIDEvent)
	{
	case IDT_TIMER_INACTIVE:
		if( IsInActivityDetected(m_iUserSelectedInactiveTimeout, &dwIdleTimeout) == TRUE)
		{
			Log.Add(_MESSAGE_,_T("Supporter is InActive!!!"));
			//notify parent window about Supporter's Inactivity GetParent()
			HWND hWnd = GetParent()->m_hWnd;
			if(hWnd)
				::PostMessage(hWnd, WM_ACTIVITY_HANDLER_INACTIVE, 0, 0);

			StopScreenSaverTimer();
			StopInactivityTimer();
			StartActivityBackTimer();
		}
		else
		{
			// example to explain logic here 
			// m_iUserSelectedInactiveTimeout = 10 seconds
			// dwIdleTimeout = 6 seconds
			// we will try to detect InActivity after 10-6=4 seconds
			StartInactivityTimer(m_iUserSelectedInactiveTimeout - dwIdleTimeout);
			// todo verbosity 
			Log.Add(_MESSAGE_,_T("Supporter still Active."));
			// continue wait for OnTimer with IDT_TIMER_INACTIVE nIDEvent
		}
		break;

	case IDT_TIMER_ACTIVITY_BACK:
		if(IsScreenSaverRunning()&& m_bShowAwayWhenScreenSaverIsOn)
		{
			// continue wait for OnTimer with IDT_TIMER_ACTIVEBACK nIDEvent
			Log.Add(_MESSAGE_,_T("ScreenSaver is still running"));
		}
		else
		if(IsInActivityDetected(m_iUserSelectedInactiveTimeout, &dwIdleTimeout) == TRUE)
		{
			// continue wait for OnTimer with IDT_TIMER_ACTIVEBACK nIDEvent
			Log.Add(_MESSAGE_,_T("Supporter didn't back to be Active."));
		}
		else
		{
			Log.Add(_MESSAGE_,_T("Supporter back to be Active."));
			HWND hWnd = GetParent()->m_hWnd;
			if(hWnd)
				::PostMessage(hWnd, WM_ACTIVITY_HANDLER_BACK_ACTIVE, 0, 0);

			StopActivityBackTimer();
			StartInactivityTimer(m_iUserSelectedInactiveTimeout);
			StartScreenSaverTimer(m_dwScreenSaveTimeOut);
		}
		break;

	case IDT_TIMER_SCREEN_SAVER:
		if(IsScreenSaverRunning()==TRUE)
		{
			Log.Add(_MESSAGE_,_T("Screen saver is Running"));
			//	notify parent window about Supporter's Inactivity GetParent()
			HWND hWnd = GetParent()->m_hWnd;
			if(hWnd)
				::PostMessage(hWnd, WM_ACTIVITY_HANDLER_INACTIVE, 0, 0);

			StopScreenSaverTimer();
			StopInactivityTimer();
			StartActivityBackTimer();
		}
		else
		{
			if(IsInActivityDetected(m_dwScreenSaveTimeOut, &dwIdleTimeout)==TRUE)
			{
				Log.Add(_WARNING_,_T("TIMER_SCREEN_SAVER. IsInActivityDetected TRUE. ScreenSaver settings modified???"));	
				dwIdleTimeout = 0;
			}
			else
			{
				Log.Add(_MESSAGE_,_T("TIMER_SCREEN_SAVER. IsInActivityDetected FALSE."));	
			}

			//	we will try to detect InActivity later 
			StartScreenSaverTimer(m_dwScreenSaveTimeOut - dwIdleTimeout);	
			Log.Add(_MESSAGE_,_T("Screen saver is NOT Running. Try to detect InActivity after: %d seconds"), 
					(m_dwScreenSaveTimeOut - dwIdleTimeout)/1000);
		}
		break;

	default:
		break;
	}

   CWnd::OnTimer(nIDEvent);
CATCH_LOG(_T("CActivityHandler::OnTimer"))
}

UINT  CActivityHandler::StartInactivityTimer(UINT iMaxInActivityDuration)
{
TRY_CATCH

	if(m_bActivateFeature == FALSE) 
		return 0;

	// todo verbosity 
	Log.Add(_CALL_,_T("StartInactivityTimer %d"), iMaxInActivityDuration);

	StopInactivityTimer();
	m_InactivituTimerVal = (UINT)SetTimer(IDT_TIMER_INACTIVE, iMaxInActivityDuration, NULL);

CATCH_LOG(_T("CActivityHandler::StartInactivityTimer"))
	return m_InactivituTimerVal;
}// end StartTimer

BOOL CActivityHandler::StopInactivityTimer()
{
TRY_CATCH
	if( m_InactivituTimerVal ) 
	{
		m_InactivituTimerVal = 0;
		if (!KillTimer(IDT_TIMER_INACTIVE))
		{
			return FALSE;
		}
	}
CATCH_LOG(_T("CActivityHandler::StopInactivityTimer"))
	return TRUE;
} // end StopTimer


//	Function retrieve Idle timeout in milliseconds
DWORD CActivityHandler::GetIdleTimeout()
{
	LASTINPUTINFO lpi;
TRY_CATCH
	lpi.cbSize = sizeof(LASTINPUTINFO);

	if(!GetLastInputInfo(&lpi))
	{
		return 0;		// TODO failed, use GetLastError to get error code
	}

	// GetTickCount() - return the number of milliseconds that have elapsed since the system was started
	// lpi.dwTime - now holds the tick count when last input was made
CATCH_LOG(_T("CActivityHandler::GetIdleTimeout"))
	return (GetTickCount() - lpi.dwTime);
}

BOOL CActivityHandler::IsInActivityDetected(UINT iMaxInActivityDuration, DWORD* pdwIdleTimeout) 
{
TRY_CATCH
	DWORD	dwIdleTimeout = 0;

	dwIdleTimeout = GetIdleTimeout();
	*pdwIdleTimeout = dwIdleTimeout;

	if((double)(dwIdleTimeout) > iMaxInActivityDuration)
	{
		// todo verbosity 
		Log.Add(_MESSAGE_,_T("Idle Timeout more then %d seconds. Actually %d seconds."), iMaxInActivityDuration/1000, dwIdleTimeout/1000 );
		return TRUE;
	}
	else
	{
		// todo verbosity 
		Log.Add(_MESSAGE_,_T("Actual Idle Timeout is only %d seconds"), dwIdleTimeout/1000);
		return FALSE;
	}
CATCH_LOG(_T("CActivityHandler::IsInActivityDetected"))
	return FALSE;
}

void CActivityHandler::StartActivityBackTimer()
{
TRY_CATCH
	StopActivityBackTimer();
	m_ActivityBackTimerVal = (UINT)SetTimer(IDT_TIMER_ACTIVITY_BACK, ACTIVITY_BACK_POLLING_INTERVAL, NULL);
CATCH_LOG(_T("CActivityHandler::StartActivityBackTimer"))
}

void CActivityHandler::StopActivityBackTimer()
{
TRY_CATCH
	if( m_ActivityBackTimerVal != 0 )
	{
		m_ActivityBackTimerVal = 0;
		KillTimer(IDT_TIMER_ACTIVITY_BACK);
	}
CATCH_LOG(_T("CActivityHandler::StopActivityBackTimer"))
}



void CActivityHandler::StartScreenSaverTimer(DWORD dwScreenSaveTimeOut)
{
TRY_CATCH

	if(m_bShowAwayWhenScreenSaverIsOn == FALSE) 
		return ;

	if(m_bScreenSaveActive)	
		m_ScreenSaverTimerVal = (UINT)SetTimer(IDT_TIMER_SCREEN_SAVER, dwScreenSaveTimeOut, NULL);

CATCH_LOG(_T("CActivityHandler::StartScreenSaverTimer"))
}

BOOL CActivityHandler::StopScreenSaverTimer()
{
TRY_CATCH
	if(m_ScreenSaverTimerVal) 
	{
		m_ScreenSaverTimerVal = 0;
		KillTimer(IDT_TIMER_SCREEN_SAVER);
	}
CATCH_LOG(_T("CActivityHandler::StopScreenSaverTimer"))
	return TRUE;
}

//  http://support.microsoft.com/kb/140723
//  http://www.experts-exchange.com/Programming/Languages/CPP/Q_10152190.html
//  http://support.microsoft.com/kb/q126627/
//	Here are two methods that you can use to detect if the screen saver is currently running: 
//• Get the name of the current screen saver from the registry, parse the PE header of the screen saver binary to get the process name, then check for an active process with that name in the performance registry.  
//• Write a screen saver that would be spawned by the system and would in turn spawn the "real" screen saver. The first screen saver could notify your application when the screen saver has been activated or deactivated. 
BOOL CActivityHandler::IsScreenSaverRunning()
{
TRY_CATCH

	BOOL	bIsScreenSaverRunning = FALSE;
	SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &bIsScreenSaverRunning, 0);
	return bIsScreenSaverRunning;

CATCH_LOG(_T("CActivityHandler::IsScreenSaverRunning"))
}

//	RegNotifyChangeKeyValue to monitor about 
//	
LRESULT CActivityHandler::GetScreenSaverSettings()
{
	//the same as "HKEY_CURRENT_USER\Control Panel\Desktop" ScreenSaveActive
	if(SystemParametersInfo( SPI_GETSCREENSAVEACTIVE, 0, &m_bScreenSaveActive, 0)==FALSE)
	{
		Log.Add(_ERROR_, _T("SystemParametersInfo with SPI_GETSCREENSAVEACTIVE failed. GetLastError:%d"),GetLastError());	
		m_bScreenSaveActive = TRUE;//default
	}

	//the same as "HKEY_CURRENT_USER\Control Panel\Desktop" ScreenSaveTimeOut
	if(SystemParametersInfo( SPI_GETSCREENSAVETIMEOUT, 0, &m_dwScreenSaveTimeOut, 0)==FALSE)
	{
		Log.Add(_ERROR_, _T("SystemParametersInfo with SPI_GETSCREENSAVETIMEOUT failed. GetLastError:%d"),GetLastError());	
		m_dwScreenSaveTimeOut = 10*1000 ;//default
	}
	else
		m_dwScreenSaveTimeOut = (m_dwScreenSaveTimeOut+5)*1000;//give 5 seconds to load ScreenSaver

	return S_OK;
}