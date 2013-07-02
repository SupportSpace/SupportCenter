/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SStatisticMessage.h
///
///  Declares SStatisticMessage - structure of statistic message
///
///  @author Dmitry Netrebenko @date 11.10.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Statistic/EStatisticMessageType.h>
#include <NWL/Statistic/EConnectType.h>
#include <NWL/Statistic/SStatisticError.h>
#include <NWL/UPnP/SGatewayInfo.h>
#include <NWL/Statistic/SStatisticStunConnect.h>
#include <NWL/Statistic/CMeasurement.h>
#include <NWL/NetworkLayer.h>
#include <AidLib/Strings/tstring.h>

#define STAT_ID_SIZE			64
#define STAT_PEERID_SIZE		64
#define STAT_CONNECTID_SIZE		64
#define STAT_VERSION_SIZE		20


#define STAT_MESSAGE_SIZE sizeof(SStatisticMessage)

#pragma pack(push)
#pragma pack(1)
struct NWL_API SStatisticMessage
{
	char						m_id[STAT_ID_SIZE];					/// Session Id
	char						m_peerId[STAT_PEERID_SIZE];			/// Peer Id
	char						m_connectId[STAT_CONNECTID_SIZE];	/// Connection Id
	char						m_version[STAT_VERSION_SIZE];		/// Tool's version
	int							m_featureId;						/// Feature Id
	EStatisticMessageType		m_type;								/// Message type
	union
	{
		EConnectType			m_connect;							/// Connection type
		SStatisticError			m_error;							/// Error structure
		SGatewayInfo			m_router;							/// Router information
		SStatisticStunConnect	m_stun;								/// Stun connection info
		unsigned int			m_sessionLength;					/// Feature Session length
		double					m_rcSpeed;							/// Remote Control average speed
		bool					m_recordEnabled;					/// Recording feature enabled
		char					m_measure;							/// Keep a live connection speed
	}							m_data;								/// Message additional data

	/// Default constructor
	SStatisticMessage()
	{
		memset(this, 0, STAT_MESSAGE_SIZE);
	};
	/// Constructor
	SStatisticMessage(
		const tstring& id, 
		const tstring& peerId, 
		const tstring& connectId, 
		const tstring& version, 
		int featureId,
		EStatisticMessageType type)
	{
		memset(this, 0, STAT_MESSAGE_SIZE);
		memcpy(m_id, id.c_str(), min(STAT_ID_SIZE, id.length()));
		memcpy(m_peerId, peerId.c_str(), min(STAT_PEERID_SIZE, peerId.length()));
		memcpy(m_connectId, connectId.c_str(), min(STAT_CONNECTID_SIZE, connectId.length()));
		memcpy(m_version, version.c_str(), min(STAT_VERSION_SIZE, version.length()));
		m_featureId = featureId;
		m_type = type;
	};
};
#pragma pack(pop)
