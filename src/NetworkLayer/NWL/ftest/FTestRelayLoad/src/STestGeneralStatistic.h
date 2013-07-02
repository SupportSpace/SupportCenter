/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  STestGeneralStatistic.h
///
///  Declares STestGeneralStatistic structure, storage for general test statistic
///
///  @author Dmitry Netrebenko @date 28.02.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>

/// STestGeneralStatistic structure, storage for general test statistic
struct STestGeneralStatistic
{
	volatile LONG		m_cancelled;		/// Number of cancelled connections
	__int64				m_testTime;			/// Total test time
/// Clears members
	void Clear()
	{
		InterlockedExchange(&m_cancelled,0);
		m_testTime = 0;
	}
};
