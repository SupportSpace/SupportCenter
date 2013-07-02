/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SOpenPortTestStatistic.h
///
///  Declares SOpenPortTestStatistic structure, storage for open port test statistic
///
///  @author Dmitry Netrebenko @date 06.03.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>

#define INTERLOCK_CLEAR(field)		InterlockedExchange(&field,0)

/// SOpenPortTestStatistic structure, storage for open port test statistic
struct SOpenPortTestStatistic
{
	volatile LONG	m_startedRequests;		/// Count of started requests
	volatile LONG	m_successRequests;		/// Count of successful requests
	volatile LONG	m_failedRequests;		/// Count of failed requests
	volatile LONG	m_failedConnect;		/// Count of failed connections
	volatile LONG	m_failedAuth;			/// Count of failed authentications
	volatile LONG	m_failedOpenPort;		/// Count of failed Open Port requests
	__int64			m_requestTime;			/// Total requests time
	volatile LONG	m_serverRequests;		/// Count of requests from server to stub
	__int64			m_connectTime;			/// Total requests time
	__int64			m_authTime;				/// Total requests time
	__int64			m_portCheckTime;		/// Total requests time

	void Clear()
	{
		INTERLOCK_CLEAR(m_startedRequests);
		INTERLOCK_CLEAR(m_successRequests);
		INTERLOCK_CLEAR(m_failedRequests);
		INTERLOCK_CLEAR(m_serverRequests);
		INTERLOCK_CLEAR(m_failedConnect);
		INTERLOCK_CLEAR(m_failedAuth);
		INTERLOCK_CLEAR(m_failedOpenPort);
		m_requestTime = 0;
		m_connectTime = 0;
		m_authTime = 0;
		m_portCheckTime = 0;
	}
};
