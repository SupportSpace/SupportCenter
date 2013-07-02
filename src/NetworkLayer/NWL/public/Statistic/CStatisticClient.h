/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStatisticClient.h
///
///  Declares CStatisticClient class, responsible for client of statistic service
///
///  @author Dmitry Netrebenko @date 15.10.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <winsock2.h>
#include <AidLib/CThread/CThread.h>
#include <AidLib/Loki/Singleton.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/shared_ptr.hpp>
#include <queue>
#include <NWL/Streaming/CSSocket.h>
#include <NWL/Streaming/relay_messages.h>
#include <AidLib/Strings/tstring.h>
#include <NWL/Statistic/SStatisticMessage.h>
#include <AidLib/CVersionInfo/CVersionInfo.h>
#include <NWL/Streaming/CStreamException.h>
#include <AidLib/CCritSection/CCritSectionObject.h>
#include <NWL/NetworkLayer.h>
#include <NWL/Statistic/CMeasurement.h>

#define STAT_CLIENT_BUF_SIZE		(STUN_MSG_HEAD_SIZE + STUN_CHALLENGE_SIZE)
#define STAT_THREAD_TERM_TIMEOUT	5000

///  CStatisticClient class, responsible for client of statistic service
///  Base class CThread
class NWL_API CStatisticClient
	:	public CThread
{
private:
/// Prevents making copies of CStatisticClient objects
	CStatisticClient(const CStatisticClient&);
	CStatisticClient& operator=(const CStatisticClient&);
public:
/// Constructor
	CStatisticClient();
/// Destructor
	~CStatisticClient();
/// Thread entry point
	virtual void Execute(void*);
/// Adds message to queue
/// @param code - message code
/// @param buf - buffer
/// @param len - size of buffer
/// @param connectId - connection id
/// @param peerId - peer id
	void AddMsg(int code, const char* buf, unsigned int len, const tstring& connectId, const tstring& peerId);
/// Returns user id
	tstring GetUserId() const;
/// Sets new userId for authentication on server
/// @param userId - new user id
	void SetUserId(const tstring& userId);
private:
/// Event for message
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type >	m_event;
/// Queue to store statistic messages
	std::queue<SPStunMessage>									m_queue;
/// Critical section to access queue
	CCritSectionSimpleObject									m_section;
/// Socket to access server
	CSSocket													m_socket;
/// Message idenficator
	unsigned int												m_ident;
/// Internal buffer
	char														m_buffer[STAT_CLIENT_BUF_SIZE];
/// Buffer for received challenge
	char														m_challenge[STUN_CHALLENGE_SIZE];
/// User Id for authentication on server
	tstring														m_userId;
private:
/// Sends request for authentication
/// @param message - statistic message
	void SendAuthRequest(SPStunMessage message);

public:
/// Creates and sends statistic message
/// @param id - session id
/// @param peerId - peer id
/// @param connectId - connection id
/// @param featureId - feature id
/// @param type - message type
/// @param data - pointer to additional data
/// @param size - size of additional data
	inline static void CreateStatisticMessage(
		const tstring& id, 
		const tstring& peerId, 
		const tstring& connectId, 
		int featureId, 
		EStatisticMessageType type,
		void* data,
		unsigned int size);
};

/// Should be used to CStatisticClient as single instance
#define STATISTIC_CLIENT_INSTANCE Loki::SingletonHolder<CStatisticClient, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()

inline void CStatisticClient::CreateStatisticMessage(const tstring& id, 
											  const tstring& peerId, 
											  const tstring& connectId, 
											  int featureId,
											  EStatisticMessageType type,	
											  void* data,
											  unsigned int size)
{
TRY_CATCH

	/// Get file version
	static tstring version(VERSION_INFO_INSTANCE.GetCurrentFileVersion());

	/// Create message
	SStatisticMessage msg(id, peerId, connectId, version, featureId, type);

	/// Fill message data
	switch(msg.m_type)
	{
	case sttExpertRequestFeature:
	case sttCustomerApprovedFeature:
	case sttCustomerDeclinedFeature:
	case sttFeatureTerminatedByExpert:
	case sttFeatureTerminatedByCustomer:
	case sttCustomerFeatureFirtsInit:
	case sttToolsVersion:
		break;

	case sttCustomerInstallFailed:
	case sttCustomerUpdateFailed:
	case sttNWLInitFailed:
	case sttFeatureTerminatedOnError:
		{
			if(!data)
				throw MCStreamException(_T("Error information is missed"));
			if(size != sizeof(SStatisticError))
				throw MCStreamException(_T("Invalid message parameter"));
			SStatisticError* error = reinterpret_cast<SStatisticError*>(data);
			memcpy(&msg.m_data.m_error, error, sizeof(SStatisticError));
		}
		break;

	case sttRouterInfo:
		{
			if(!data)
				throw MCStreamException(_T("Router information is missed"));
			if(size != sizeof(SGatewayInfo))
				throw MCStreamException(_T("Invalid message parameter"));
			SGatewayInfo* gate = reinterpret_cast<SGatewayInfo*>(data);
			memcpy(&msg.m_data.m_router, gate, sizeof(SGatewayInfo));
		}
		break;

	case sttConnectType:
		{
			if(!data)
				throw MCStreamException(_T("Connection type parameter is missed"));
			if(size != sizeof(EConnectType))
				throw MCStreamException(_T("Invalid message parameter"));
			EConnectType* connType = reinterpret_cast<EConnectType*>(data);
			memcpy(&msg.m_data.m_connect, connType, sizeof(EConnectType));
		}
		break;

	case sttFeatureSessionLength:
		{
			if(!data)
				throw MCStreamException(_T("Session length parameter is missed"));
			if(size != sizeof(unsigned int))
				throw MCStreamException(_T("Invalid message parameter"));
			unsigned int* length = reinterpret_cast<unsigned int*>(data);
			memcpy(&msg.m_data.m_sessionLength, length, sizeof(unsigned int));
		}
		break;

	case sttRCAvgSpeed:
		{
			if(!data)
				throw MCStreamException(_T("RS speed parameter is missed"));
			if(size != sizeof(double))
				throw MCStreamException(_T("Invalid message parameter"));
			double* speed = reinterpret_cast<double*>(data);
			memcpy(&msg.m_data.m_rcSpeed, speed, sizeof(double));
		}
		break;

	case sttRecordingEnabled:
		{
			if(!data)
				throw MCStreamException(_T("Recording enabled parameter is missed"));
			if(size != sizeof(bool))
				throw MCStreamException(_T("Invalid message parameter"));
			bool* enabled = reinterpret_cast<bool*>(data);
			memcpy(&msg.m_data.m_recordEnabled, enabled, sizeof(bool));
		}
		break;

	case sttSTUNConnect:
		{
			if(!data)
				throw MCStreamException(_T("STUN statistics parameter is missed"));
			if(size != sizeof(SStatisticStunConnect))
				throw MCStreamException(_T("Invalid message parameter"));
			SStatisticStunConnect* stun = reinterpret_cast<SStatisticStunConnect*>(data);
			memcpy(&msg.m_data.m_stun, stun, sizeof(SStatisticStunConnect));
		}
		break;
	case sttPingPongData:
		{
			if(!data)
				throw MCStreamException(_T("Measurement parameter is missed"));
			if(size != sizeof(CMeasurement))
				throw MCStreamException(_T("Invalid message parameter"));
			CMeasurement* measure = reinterpret_cast<CMeasurement*>(data);
			const char* pData = measure->data();
			DWORD dSize = min (sizeof(msg.m_data),measure->size());
			memcpy(&msg.m_data.m_measure,pData,dSize);
		}
		break;
	default:
		throw MCStreamException(_T("Invalid message type"));
	};

	/// Send message
	STATISTIC_CLIENT_INSTANCE.AddMsg(0, reinterpret_cast<char*>(&msg), sizeof(SStatisticMessage), connectId, peerId);

CATCH_LOG()
}

