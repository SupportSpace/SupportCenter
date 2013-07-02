/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SStreamTestStatistic.h
///
///  Declares SStreamTestStatistic structure, storage for stream test statistic
///
///  @author Dmitry Netrebenko @date 28.02.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>

#define INTERLOCK_CLEAR(field)		InterlockedExchange(&field,0)

/// SStreamTestStatistic structure, storage for stream test statistic
struct SStreamTestStatistic
{
	volatile LONG	m_startedConnections;	/// Count of started connections
	volatile LONG	m_successConnections;	/// Count of successful connections
	volatile LONG	m_failedConnections;	/// Count of failed connections
	__int64			m_connectTime;			/// Total connection time
	__int64			m_sentBytes;			/// Total sent bytes
	__int64			m_recvBytes;			/// Total received bytes
	__int64			m_transferTime;			/// Total data transfer time

	void Clear()
	{
		INTERLOCK_CLEAR(m_startedConnections);
		INTERLOCK_CLEAR(m_successConnections);
		INTERLOCK_CLEAR(m_failedConnections);
		m_connectTime = 0;
		m_sentBytes = 0;
		m_recvBytes = 0;
		m_transferTime = 0;
	}
};
