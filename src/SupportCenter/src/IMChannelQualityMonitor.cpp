#include "StdAfx.h"
#include "IMChannelQualityMonitor.h"
#include "AidLib/CException/CException.h"
#include <AidLib/Logging/cLog.h> 
#include "LocalDataDef.h"
#include "SupportMessenger.h"

BOOL SimulateIssueDueConfiguation(DWORD* dwLatancy);

extern CSupportMessengerApp theApp;

WNDPROC CIMChannelQualityMonitor::m_origWndProc = NULL;
CIMChannelQualityMonitor* CIMChannelQualityMonitor::self = NULL;

#define POLLING_INTERVAL_TIMER_ID_TIMER_ID 61000
#define TIMEPERIOD_FOR_MONITORING_TIMER_ID 61001

#define DefaultPollingInterval					5*1000   // (T)
#define DefaultTimePeriodForMonotoring				60   // (Z) 
#define DefaultMaxNumOfUnacceptableMsgs			     3   // (Y) 
#define DefaultAcceptableRoundtripTimeLimit			10   // (X) 

CUnacceptableMsg::CUnacceptableMsg(__int64 timestamp)
{
	m_timestamp = timestamp; 
	m_etype = Never;
}

CUnacceptableMsg::~CUnacceptableMsg()
{
	Log.Add(_CALL_, _T("CUnacceptableMsg destructor working "));
}

CIMChannelQualityMonitor::CIMChannelQualityMonitor()
{
	self = this;
	m_hNotifyWnd = NULL;
	m_bStarted = FALSE;
}

CIMChannelQualityMonitor::~CIMChannelQualityMonitor(void)
{
}

BOOL CIMChannelQualityMonitor::IsMonitorRunning()
{
TRY_CATCH
CATCH_LOG(_T("CIMChannelQualityMonitor::IsMonotirRunning"))
	return m_bStarted;
}

void CIMChannelQualityMonitor::Start(HWND hWnd,
									 int  PollingInterval,
									 int  TimePeriodForMonotoring,
									 int  MaxNumOfUnacceptableMsgs,
									 int  AcceptableRoundtripTimeLimit)
{
TRY_CATCH

	if(PollingInterval==0)
	{
		Log.Add(_WARNING_,_T("PollingInterval is 0 means there is no monitoring required"));				
		return;
	}

	if(m_bStarted==TRUE)
	{
		Log.Add(_WARNING_,_T("CIMChannelQualityMonitor was already started"));				
		return; //Stop();	//todo critical section here
	}

	m_PollingInterval = PollingInterval;
	m_TimePeriodForMonotoring = TimePeriodForMonotoring;
	m_MaxNumOfUnacceptableMsgs = MaxNumOfUnacceptableMsgs;
	m_AcceptableRoundtripTimeLimit = AcceptableRoundtripTimeLimit;
	
	m_bStarted = TRUE;
	m_tIssueDetectedTime = 0;
	m_tOldestDelayedMsgTime = 0;

	m_hNotifyWnd = hWnd;
	m_origWndProc = (WNDPROC)SetWindowLong(m_hNotifyWnd, GWL_WNDPROC, (LONG)CIMChannelQualityMonitor::HookWndProc);
	::SetTimer(m_hNotifyWnd, POLLING_INTERVAL_TIMER_ID_TIMER_ID, m_PollingInterval*1000, NULL);
	::SetTimer(m_hNotifyWnd, TIMEPERIOD_FOR_MONITORING_TIMER_ID, m_TimePeriodForMonotoring*1000, NULL);

CATCH_LOG(_T("CIMChannelQualityMonitor::Start"))
}

void CIMChannelQualityMonitor::Stop()
{
TRY_CATCH

	::KillTimer(m_hNotifyWnd, POLLING_INTERVAL_TIMER_ID_TIMER_ID);
	::KillTimer(m_hNotifyWnd, TIMEPERIOD_FOR_MONITORING_TIMER_ID);
	if(m_origWndProc!=NULL)
	{
		SetWindowLong(m_hNotifyWnd, GWL_WNDPROC, (LONG)m_origWndProc);
		m_origWndProc = NULL;
	}

	m_bStarted = FALSE;
	ResetPeriodLifeTimeData();

CATCH_LOG(_T("CIMChannelQualityMonitor::Stop"))
}

LRESULT CALLBACK CIMChannelQualityMonitor::HookWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
TRY_CATCH
	
	TCHAR szTmp[512] = {'\0'};

	if(uMsg==WM_TIMER && (UINT_PTR)wParam == TIMEPERIOD_FOR_MONITORING_TIMER_ID)
	{
		time_t currentTime = time(NULL);
		int iTotalProblematicMsgs = self->m_QualityRequests.size();
		int iNumOfDelayedMsgs = self->NumDelayedMsgs();
		
		if(iTotalProblematicMsgs >= self->m_MaxNumOfUnacceptableMsgs)
		{
			if(self->m_tIssueDetectedTime!=0)
			{
				sprintf_s(szTmp,512,_T("Poor connectivity detected - Continued. Total unacceptable msgs: %d. Delayed: %d Never arrived: %d"),
					iTotalProblematicMsgs, iNumOfDelayedMsgs, iTotalProblematicMsgs - iNumOfDelayedMsgs);
				theApp.m_conQualityLog.AddMessage(cLog::_MESSAGE,szTmp);
			}
			else
			{
				sprintf_s(szTmp,512,_T("Poor connectivity detected. Total unacceptable msgs: %d. Delayed: %d Never arrived: %d"), 
					iTotalProblematicMsgs, iNumOfDelayedMsgs, iTotalProblematicMsgs - iNumOfDelayedMsgs);
				PostMessage(self->m_hNotifyWnd, WM_IM_CHANNEL_QUALITY_MONITOR_ISSUE_DETECTED, 0, (LPARAM)0); 
				theApp.m_conQualityLog.AddMessage(cLog::_MESSAGE,szTmp);
			}
					
			Log.Add(_WARNING_,szTmp);				
			
			ResetPeriodLifeTimeData();
			
			self->m_tIssueDetectedTime = currentTime;
			::KillTimer(self->m_hNotifyWnd, TIMEPERIOD_FOR_MONITORING_TIMER_ID);
			::SetTimer(self->m_hNotifyWnd, TIMEPERIOD_FOR_MONITORING_TIMER_ID, self->m_TimePeriodForMonotoring*1000, NULL);
		}
		else
		{
			if(self->m_tIssueDetectedTime!=0)
			{
				sprintf_s(szTmp,512,_T("Connectivity has been restored. Total unacceptable msgs: %d. Delayed: %d Never arrived: %d"), 
					iTotalProblematicMsgs, iNumOfDelayedMsgs, iTotalProblematicMsgs - iNumOfDelayedMsgs);
				self->m_tIssueDetectedTime = 0;
				PostMessage(self->m_hNotifyWnd, WM_IM_CHANNEL_QUALITY_MONITOR_IMPROVEMENT_DETECTED, 0, (LPARAM)0); 
				theApp.m_conQualityLog.AddMessage(cLog::_MESSAGE,szTmp);		
			}
			else
			{
				sprintf_s(szTmp,512,_T("Acceptable connectivity - Continued. Total unacceptable msgs: %d. Delayed: %d Never arrived: %d"), 
					iTotalProblematicMsgs, iNumOfDelayedMsgs, iTotalProblematicMsgs - iNumOfDelayedMsgs);
				theApp.m_conQualityLog.AddMessage(cLog::_FTEST_CASE,szTmp);		
			}

			
			Log.Add(_WARNING_,szTmp);	
			
			//  remove oldest element in the map and then move to iterator to next element
			if(self->m_tOldestDelayedMsgTime!=0)
				DeleteElement(self->m_tOldestDelayedMsgTime);

			//	goto next problematic message in queue
			self->m_tOldestDelayedMsgTime = GetOldestElement();
			
			time_t newTimerValue = 0;

			if(self->m_tOldestDelayedMsgTime!=0)//calculate it correct 
				newTimerValue = self->m_tOldestDelayedMsgTime + self->m_TimePeriodForMonotoring - currentTime;	
			else//just after period
				newTimerValue = self->m_TimePeriodForMonotoring;

			Log.Add(_MESSAGE_,_T("Next value for timer TIMEPERIOD_FOR_MONITORING_TIMER_ID is :%d seconds"), newTimerValue);	

			::KillTimer(self->m_hNotifyWnd, TIMEPERIOD_FOR_MONITORING_TIMER_ID);
			::SetTimer(self->m_hNotifyWnd, TIMEPERIOD_FOR_MONITORING_TIMER_ID, newTimerValue*1000, NULL);
		}

		Log.Add(_CALL_,_T("TIMEPERIOD_FOR_MONITORING_TIMER_ID"));
		return 0;
	}

	if(uMsg==WM_TIMER && (UINT_PTR)wParam == POLLING_INTERVAL_TIMER_ID_TIMER_ID)
	{
		time_t testTime = time(NULL);
		int    iNumUnresponcedMsgs = self->m_QualityRequests.size();
				
		Log.Add(_CALL_,_T("CIMChannelQualityMonitor::HookWndProc IM_CHANNEL_QUALITY_MONITOR_TIMER_ID"));

		//if(iNumUnresponcedMsgs!=0)
		//	Log.Add(_WARNING_,_T("Still waiting previously send responses: %d"), self->m_QualityRequests.size());

		if(self->m_hNotifyWnd)	
		{
			//add to map for waiting for test results 
			self->m_QualityRequests.insert(
				IMChannelQualityRequestsMap::value_type(
				testTime,
				new CUnacceptableMsg(testTime)));
			
			//send roundtrip message  
			PostMessage(self->m_hNotifyWnd, WM_IM_CHANNEL_QUALITY_MONITOR_TEST_TIME, 0, (LPARAM)testTime); 
		}
		return 0;
	}

CATCH_LOG(_T("CIMChannelQualityMonitor::HookWndProc"))

	return CallWindowProc(m_origWndProc, hWnd, uMsg, wParam, lParam);
}
//  look in the map by testTime to check http://srv-filer/confluence/display/cmpt/Expert%27s+Connection+Quality
//	Once the Support Center loads it will start sending messages to itself every (T) seconds and do this during the entire session to monitor the connection quality.
//	If the roundtrip time (X) is unacceptable and occurs (Y) times within a set period of time (Z) , 
//	the Support Center should display an alert to the expert. 
//	Configurable parameters: 
//	(X) - The acceptable time limit of a message's roundtrip. Anything longer that (X) will be 'unacceptable'. 
//	(Y) - The maximum number of times that an unacceptable message is allowed within a set period of time (Z). 
//	(Z) - The time period 
//	(T) - The elapsed time between each roundtrip message 
void CIMChannelQualityMonitor::HandleIMChannelQualityTestResponse(time_t testTime, tstring sTestTime)
{
TRY_CATCH

#ifdef DEBUG
	DWORD	dwLatancy = 0;
	SimulateIssueDueConfiguation(&dwLatancy);
	if(dwLatancy) Sleep(dwLatancy*1000);
#endif

	IMChannelQualityRequestsMapIterator   it = m_QualityRequests.find(testTime);

	if(it != m_QualityRequests.end())
	{
		time_t currentTime = time(NULL);
		long roundtripDuration = currentTime - ((CUnacceptableMsg*)(*it).second)->m_timestamp;

		if(roundtripDuration > m_AcceptableRoundtripTimeLimit)
		{
			((CUnacceptableMsg*)(*it).second)->m_etype = Delayed;

			TCHAR szTmp[512] = {'\0'};
			sprintf_s(szTmp,512,_T("Unacceptable roundtrip duration: %d sec. Timestamp: %s"), roundtripDuration, sTestTime.c_str() );

			Log.Add(_WARNING_, szTmp);
			theApp.m_conQualityLog.AddMessage(cLog::_MESSAGE, szTmp);		
		}
		else
		{
			delete ((CUnacceptableMsg*)(*it).second);
			m_QualityRequests.erase(it);
			theApp.m_conQualityLog.AddMessage(cLog::_FTEST_CASE,Format(_T("Acceptable roundtrip duration: %d sec"), roundtripDuration));		
		}
	}
	
CATCH_LOG(_T("CIMChannelQualityMonitor::HookWndProc"))
}

//reset all parameters before next lifeframe that will try to detect improvement or defect
void CIMChannelQualityMonitor::ResetPeriodLifeTimeData()
{
TRY_CATCH

	self->m_tIssueDetectedTime = 0;
	self->m_tOldestDelayedMsgTime = 0;

	IMChannelQualityRequestsMapIterator it = self->m_QualityRequests.begin();
	for( ; it != self->m_QualityRequests.end(); )
	{
		delete ((CUnacceptableMsg*)(*it).second);
		it = self->m_QualityRequests.erase(it);
	}
	
CATCH_LOG(_T("CIMChannelQualityMonitor::ResetPeriodLifeTimeData"))
}

BOOL SimulateIssueDueConfiguation(DWORD* dwLatancy)
{
	CRegKey  m_regKey;
	HRESULT	 lResult = S_OK;
	ULONG	 nBytes = 0;
	
	TCHAR	szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };
	_stprintf_s(szCurrentUserPath, MAX_REGBUF_LEN, _T("%s"), szBaseRegistryPath);//todo

	if( m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS )
		return FALSE;

	DWORD dwTmp = 0;
	
	lResult = m_regKey.QueryDWORDValue("Latency", (DWORD&)dwTmp);
	*dwLatancy = dwTmp;
	
	m_regKey.Close();
	return TRUE;
}

time_t CIMChannelQualityMonitor::GetOldestElement()
{
	time_t oldestUnresponsedMsgTime = 0;

	//find oldest request among all exists if more then one in map. If to use std::queue then another search needed
	IMChannelQualityRequestsMapIterator it = self->m_QualityRequests.begin();
	for( ; it != self->m_QualityRequests.end(); it++)
	{
		if(oldestUnresponsedMsgTime==0 || ((CUnacceptableMsg*)(*it).second)->m_timestamp < oldestUnresponsedMsgTime)
			oldestUnresponsedMsgTime = ((CUnacceptableMsg*)(*it).second)->m_timestamp;
	}

	return oldestUnresponsedMsgTime;
}

void CIMChannelQualityMonitor::DeleteElement(time_t testTime)
{
	IMChannelQualityRequestsMapIterator   it = self->m_QualityRequests.find(testTime);
	if(it != self->m_QualityRequests.end())
	{
		delete ((CUnacceptableMsg*)(*it).second);
		self->m_QualityRequests.erase(it);
	}
}

int CIMChannelQualityMonitor::NumDelayedMsgs()
{
	int NumDelayedMsgs = 0;

	IMChannelQualityRequestsMapIterator it = self->m_QualityRequests.begin();
	for( ; it != self->m_QualityRequests.end(); it++)
	{
		if( ((CUnacceptableMsg*)(*it).second)->m_etype == Delayed )
			NumDelayedMsgs++;
	}

	return NumDelayedMsgs;
}

