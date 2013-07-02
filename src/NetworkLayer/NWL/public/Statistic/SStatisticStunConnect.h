/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SStatisticStunConnect.h
///
///  Declares SStatisticStunConnect - structure of statistic message for
///    STUN connection
///
///  @author Dmitry Netrebenko @date 29.01.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/NetworkLayer.h>

#pragma pack(push)
#pragma pack(1)
/// SStunConnectStage - structure with statistic data for
///   STUN connection's stage
struct NWL_API SStunConnectStage
{
	unsigned int	m_time;			/// Time of passing stage
	unsigned int	m_avgTime;		/// Average request time
	unsigned int	m_requestCount;	/// Count of sent requests
	unsigned int	m_noResponded;	/// Count of requests without response
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
/// SStatisticStunConnect - structure of statistic message for
///   STUN connection
struct NWL_API SStatisticStunConnect
{
	unsigned int		m_timeout;		/// Timeout assigned by factory
	unsigned int		m_time;			/// Time of passing connection routine
	unsigned int		m_udtTime;		/// Time of passing connection through UDT
	unsigned int		m_tlsTime;		/// Time of passing GNU TLS authentication
	SStunConnectStage	m_stages[3];	/// Statistic for each connection stage
};
#pragma pack(pop)
