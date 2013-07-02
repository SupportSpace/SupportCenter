// CIMChannelQualityMonitor.h : header file for class that monitor IM channel quality
//
//===========================================================================
// SupportMessengerApp ltd. @{SRCH}
//								CIMChannelQualityMonitor
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

#define WM_UPDATE_COMPLETED      9001
#define WM_IM_CHANNEL_QUALITY_MONITOR_TEST_TIME		 WM_USER + 1099
#define WM_IM_CHANNEL_QUALITY_MONITOR_ISSUE_DETECTED WM_USER + 1100
#define WM_IM_CHANNEL_QUALITY_MONITOR_IMPROVEMENT_DETECTED WM_USER + 1101



#include "AidLib\Strings\tstring.h"
#include "stdafx.h"
#include "CVersionReader.h"
#include <msi.h>
#include <queue>

#pragma comment(lib, "msi.lib")

// Length of GUID
#define MSI_GUID_LEN 38
#define TOOLS_PRODUCT_CODE			_T("{B359C619-3526-4216-BA49-7022953D0C8E}")

/// 
typedef enum _EUnacceptableType{

	Delayed		= 0,
	Never		= 1

} EAnacceptableType;

class CUnacceptableMsg
{
public:
	CUnacceptableMsg(__int64 timestamp);
	~CUnacceptableMsg();

public:
	__int64 m_timestamp;
	EAnacceptableType m_etype;
};

typedef std::map<__int64, CUnacceptableMsg*> IMChannelQualityRequestsMap;
typedef std::map<__int64, CUnacceptableMsg*>::iterator  IMChannelQualityRequestsMapIterator;

class CIMChannelQualityMonitor
{
public:
	CIMChannelQualityMonitor();
	~CIMChannelQualityMonitor(void);

	void Start(HWND hWnd,int PollingInterval,int TimePeriodForMonotoring,int MaxNumOfUnacceptableMsgs,int AcceptableRoundtripTimeLimit);
	void Stop();

	BOOL IsMonitorRunning();

	void HandleIMChannelQualityTestResponse(time_t testTime, tstring sTestTime);

private:

	static  time_t GetOldestElement();
	static 	void   DeleteElement(time_t testTime);
	static  int    NumDelayedMsgs();

	// CIMChannelQualityMonitor will hook OnTimer and implement check quolity function 
	static LRESULT CALLBACK HookWndProc(HWND hwnd,UINT uMsg, WPARAM wParam, LPARAM lParam);

	static void ResetPeriodLifeTimeData();

	// Original windows procedure
	static WNDPROC   m_origWndProc;
	// Handle to notification window
	HWND m_hNotifyWnd;
	// self class static 
	static CIMChannelQualityMonitor* self;

	BOOL		m_bStarted;	//flag indicates if monitor started 

	IMChannelQualityRequestsMap m_QualityRequests;//map (queue) of send requests 

	time_t		m_tOldestDelayedMsgTime; //time when oldest message was not delivererd at time 
	time_t		m_tIssueDetectedTime;	 //time when connectivity issue detected 

	int			m_PollingInterval;
	int			m_TimePeriodForMonotoring;
	int			m_MaxNumOfUnacceptableMsgs;
	int			m_AcceptableRoundtripTimeLimit;
};
