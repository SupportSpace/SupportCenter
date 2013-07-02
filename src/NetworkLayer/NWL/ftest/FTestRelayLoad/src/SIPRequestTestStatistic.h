/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SIPRequestTestStatistic.h
///
///  Declares SIPRequestTestStatistic structure, storage for ip request test statistic
///
///  @author Dmitry Netrebenko @date 04.03.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>

#define INTERLOCK_CLEAR(field)		InterlockedExchange(&field,0)

/// SIPRequestTestStatistic structure, storage for ip request test statistic
struct SIPRequestTestStatistic
{
	volatile LONG	m_startedRequests;		/// Count of started requests
	volatile LONG	m_successRequests;		/// Count of successful requests
	volatile LONG	m_failedRequests;		/// Count of failed requests
	__int64			m_requestTime;			/// Total requests time

	void Clear()
	{
		INTERLOCK_CLEAR(m_startedRequests);
		INTERLOCK_CLEAR(m_successRequests);
		INTERLOCK_CLEAR(m_failedRequests);
		m_requestTime = 0;
	}
};
