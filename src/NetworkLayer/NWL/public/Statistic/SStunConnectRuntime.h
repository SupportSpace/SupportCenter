/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SStunConnectRuntime.h
///
///  Declares SStunConnectRuntime - structure for runtime statistic data for
///    STUN connection
///
///  @author Dmitry Netrebenko @date 29.01.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/NetworkLayer.h>
#include <vector>
#include <NWL/Statistic/SStatisticStunConnect.h>

/// STUN connection stage types
enum EStanStageType
{
	srtAuth		= 0,
	srtBind		= 1,
	srtProbe	= 2
};

/// SStunStageTime - structure to store time data for request or stage
struct NWL_API SStunStageTime
{
	unsigned int	m_startTime;	/// Start time of stage or request
	unsigned int	m_endTime;		/// End time of stage or request
/// Default constructor
	SStunStageTime()
	{
		memset(this, 0, sizeof(SStunStageTime));
	};
/// Conversion operator
	operator unsigned int() const
	{
		unsigned int time = 0;
		if(m_endTime > m_startTime)
			time = m_endTime - m_startTime;
		return time;
	};
};

/// SStunStageRuntime - structure to store data for each stage with requests
struct NWL_API SStunStageRuntime
{
	SStunStageTime				m_total;	/// Total stage time
	std::vector<SStunStageTime>	m_requests;	/// Vector with times of requests
/// Conversion operator
	operator SStunConnectStage() const
	{
		/// Create structure
		SStunConnectStage stage;
		/// Set stage time
		stage.m_time = m_total;
		/// Calculate average request time, count of requests and count of no responded requests
		unsigned int time = 0;
		unsigned int count = 0;
		unsigned int responds = 0;
		for(std::vector<SStunStageTime>::const_iterator index = m_requests.begin();
			index != m_requests.end();
			++index)
		{
			count++;
			SStunStageTime requestTime = *index;
			unsigned int timeValue = requestTime;
			if(timeValue)
			{
				time += timeValue;
				responds++;
			}
		}
		stage.m_noResponded = count - responds;
		stage.m_requestCount = count;
		if(count)
			stage.m_avgTime = time / count;
		else
			stage.m_avgTime = 0;
		return stage;
	};
};

/// SStunConnectRuntime - structure for runtime statistic data for
///   STUN connection
struct NWL_API SStunConnectRuntime
{
	unsigned int				m_timeout;		/// Timeout assigned by factory
	SStunStageTime				m_total;		/// Total connection time
	SStunStageTime				m_udt;			/// UDT connection time
	SStunStageTime				m_tls;			/// GNU TLS authentication time
	SStunStageRuntime			m_stages[3];	/// STUN request stages times
/// Default constructor	
	SStunConnectRuntime()
		:	m_timeout(0)
	{
	};
/// Conversion operator
	operator SStatisticStunConnect() const
	{
		SStatisticStunConnect statistic;
		statistic.m_timeout = m_timeout;
		statistic.m_time = m_total;
		statistic.m_tlsTime = m_tls;
		statistic.m_udtTime = m_udt;
		for(int i = 0 ; i < 3; ++i)
			statistic.m_stages[i] = m_stages[i];
		return statistic;
	};
};

