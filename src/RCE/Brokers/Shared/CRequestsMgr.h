/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRequestsMgr.h
///
///  CRequestsMgr object declaration. Requests manager
///
///  @author Kirill Solovyov @date 14.11.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include "BrokersTypes.h"

class CRequestsMgr;
#include "CSessionsMgr.h"
#include "CServicesMgr.h"
#include "SRequest.h"
#include <AidLib/WatchDog/CProcessWatchDog.h>

#include <set>


// inner block object
class CBlocker
{
	friend class CBlockerUse;
	friend class CBlockerBlock;
	/// uses counter
	unsigned long m_uses;
	/// block state
	bool m_blocked;
	/// m_uses and m_blocked variable
	CCriticalSection m_cs;
	/// wake block object. it is set when m_uses==0 by m_block=true
	CEvent m_wakeBlock;
	CBlocker(const CBlocker&);
	CBlocker& operator=(const CBlocker&);
public:
	CBlocker():
		m_uses(0),
		m_blocked(false),
		m_wakeBlock(FALSE,FALSE)
	{}
};

// CBlockerUse is created when blocking object is used
class CBlockerUse
{
	CBlockerUse(const CBlockerUse&);
	CBlockerUse& operator=(const CBlockerUse&);
	CBlocker *m_blocker;
public:
	CBlockerUse(CBlocker* blocker):
			m_blocker(blocker)
	{
		CCritSection cs(&m_blocker->m_cs);
		if(m_blocker->m_blocked)
			throw MCException("CBlockerUse::CBlockerUse() Object blocked");
		++m_blocker->m_uses;
	}
	~CBlockerUse()
	{
		CCritSection cs(&m_blocker->m_cs);
		--m_blocker->m_uses;
		if(!m_blocker->m_uses&&m_blocker->m_blocked)
			m_blocker->m_wakeBlock.Set();
	}
};
// CBlockerBlock is created when blocking object is blocked
class CBlockerBlock
{
	CBlockerBlock(const CBlockerBlock&);
	CBlockerBlock& operator=(const CBlockerBlock&);
	CBlocker *m_blocker;
public:
	CBlockerBlock(CBlocker* blocker):
			m_blocker(blocker)
	{
		CCritSection cs(&m_blocker->m_cs);
		if(m_blocker->m_blocked)
			throw MCException("CBlockerBlock::CBlockerBlock() Object blocked");
		m_blocker->m_blocked=true;
		if(m_blocker->m_uses)
		{
			cs.Unlock();
			if(WAIT_OBJECT_0!=WaitForSingleObject(m_blocker->m_wakeBlock,INFINITE))
				throw MCException_Win("CBlockerBlock::CBlockerBlock() WaitForSingleObject() failed");
		}
	}
	~CBlockerBlock()
	{
		CCritSection cs(&m_blocker->m_cs);
		m_blocker->m_blocked=false;
	}
};








class CRequestsMgr : public CInstanceTracker
{
protected:
	/// blocker when destroy
	CBlocker m_b;
	/// sessions manager
	boost::shared_ptr<CSessionsMgr> m_sessionsMgr;
	/// services manager
	boost::shared_ptr<CServicesMgr> m_servicesMgr;
	/// this side user parameters
	struct
	{
		tstring m_userId;   /// unique user identifier
		tstring m_passwd;   /// password
	} m_params;

	/// The method is manage request
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionsMgr.h file).
	void OnRequest(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params);

	/// The method handles service requests BRT_SERVICE
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionsMgr.h file).
	virtual void BRT_SERVICE_Handler(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params);


	/// sub stream query parameters
	struct SSubStreamQueryParams
	{
		/// destination user identifier
		tstring m_dstUserId;
		/// destination service identifier
		unsigned long m_dstSvcId;
		/// source service identifier
		unsigned long m_srcSvcId;
		/// service sub stream identifier (not real substream identifier of Stream Multiplexer)
		unsigned long m_svcSubStreamId;
		/// real sub stream identifier
		unsigned long m_subStreamId;
		/// Priority level for the substream
		unsigned long m_priorityLevel;

		SSubStreamQueryParams(){}

		SSubStreamQueryParams(const tstring dstUserId,unsigned long dstSvcId,unsigned long srcSvcId,unsigned long svcSubStreamId,unsigned long subStreamId,unsigned long priorityLevel)
		{
			m_dstUserId=dstUserId;
			m_dstSvcId=dstSvcId;
			m_srcSvcId=srcSvcId;
			m_svcSubStreamId=svcSubStreamId;
			m_subStreamId=subStreamId;
			m_priorityLevel=priorityLevel;
		}

		SSubStreamQueryParams(const SSubStreamQueryParams & instance)
		{
			m_dstUserId=instance.m_dstUserId;
			m_dstSvcId=instance.m_dstSvcId;
			m_srcSvcId=instance.m_srcSvcId;
			m_svcSubStreamId=instance.m_svcSubStreamId;
			m_subStreamId=instance.m_subStreamId;
			m_priorityLevel=instance.m_priorityLevel;
		}
		
		SSubStreamQueryParams& operator=(const SSubStreamQueryParams & instance)
		{
			m_dstUserId=instance.m_dstUserId;
			m_dstSvcId=instance.m_dstSvcId;
			m_srcSvcId=instance.m_srcSvcId;
			m_svcSubStreamId=instance.m_svcSubStreamId;
			m_subStreamId=instance.m_subStreamId;
			m_priorityLevel=instance.m_priorityLevel;
			return *this;
		}
	};
	/// vector of sub stream queries
	std::vector<SSubStreamQueryParams> m_subStreamQueries;
	CCriticalSection m_csSubStreamQueries;

	/// The method handle sub stream queries
	/// @param dstUserId unique destination user identifier
	/// @param dstSvcId unique per UserId destination service identifier
	/// @param srcSvcId source service identifier
	/// @param streamId unique for this service stream identifier. it is not real sub stream identifier of Stream Multimplexer
	/// @param priorityLevel Priority level for the substream
	void innerGetSubStream(const tstring& dstUserId, unsigned long dstSvcId, unsigned long srcSvcId, unsigned long svcStreamId, unsigned long subStreamId,unsigned long priorityLevel);

	//TODO remove
	//bool m_notConnectedServiceCrutch;

	///TODO rename
	

	/// Current request identifier. It's used by RId() method. 
	unsigned long m_currentRId;
	/// The method genarate new request identifier (rId) value. It's used by requests sending
	/// @return new value of rId
	unsigned long RId(void);

	/// requests pool, it's used for store requests by (for example) messagebox requests
	std::map<unsigned int,SRequest> m_requestsPool;
	CCriticalSection m_csRequestsPool;

	/// serivce request response from JS
	std::map<tstring,SRequest> m_svcReqFromJS;
	CCriticalSection m_csSvcReqFromJS;

	/// userId which sent BRT_SERVICE through jabber and waiting answer
	/// svcId service identifier which send request
	std::map<tstring,unsigned long> m_svcReqWaiting;
	CCriticalSection m_csSvcReqWaiting;

public:
	CRequestsMgr(void);
	virtual ~CRequestsMgr(void);

	/// The method is called by CSessionsMgr when request come from remote side
	/// @param fromUserId user identifier where from come request
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionsMgr.h file).
	void OnRemoteRequest(const tstring& fromUserId, const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params);

	/// The method is called by CSessionMgr when connection progress change its state
	/// @param fromUserId user identifier where from come request
	/// @param message text describe connection status
	void OnRemoteConnecting(const tstring& fromUserId,const tstring& message);

	/// The method is called by JS for request handle
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionsMgr.h file).
	void OnJSRequest(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params);

	/// The method is called by CServicesMgr when service send request
	/// @param svcId service identifier of service which call the method
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionsMgr.h file).
	void OnSvcRequest(unsigned long svcId, const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params);

	/// The method is called when sub stream is connected
	/// @param dstUserId user identifier remote side
	/// @param subStreamId Service identifier
	/// @param stream new sub stream
	void OnSubStreamTaken(const tstring& dstUserId, unsigned int subStreamId, boost::shared_ptr<CAbstractStream> stream);

	/// The method is called by CServiceMgr when it wants sub stream.
	/// @param svcId service identifier of service which call the method
	/// @param dstUserId unique destination user identifier
	/// @param dstSvcId unique per UserId destination service identifier
	/// @param streamId unique for this service stream identifier. it is not real sub stream identifier of Stream Multimplexer
	/// @param priorityLevel Priority level for the substream
	void OnGetSubStream(unsigned long svcId, const tstring& dstUserId, unsigned long dstSvcId, unsigned long streamId, unsigned long priorityLevel);


	/// Create service ActiveX COM object which host on DHTML object.
	/// @param remoteUserId remote user identifier
	/// @param remoteSvcId service identifier of remote side
	/// @param user identifiler
	/// @param svcId service identifier. it has predefined values (see EBrokerSvcIdPredefinedValues).
	/// @param svcType service type (see EBrokerServicesTypes).
	/// @param host DHTML host object
	/// @return real service identifier
	unsigned long CreateToolService(const tstring& remoteUserId,unsigned long remoteSvcId, const tstring& userId, unsigned long svcId,EBrokerServicesTypes svcType, IDispatch* host);

	/// Destroy Service object
	/// @param svcId service identifier
	void DestroyToolService(unsigned long svcId);

	/// The method initialize Requests manager's parameters
	/// @param unique user identifier
	/// @param user password
	void Init(const tstring& userId, const tstring& passwd);

	/// The method initialize new session (but doesn’t connect, session is created by demand (by first call SendRequest method)
	/// @param relaySrv Relay server address 
	/// @param sId session identifier
	/// @param user user name
	/// @param passwd user pass word
	/// @param remoteUser remote user name
	/// @param timeOut time of connection
	/// @param masterRole 
	/// @param change if flag set, existed session will be re-created or new session will be created
	void InitSession(const tstring& relaySrv, const tstring& sId, const tstring& user, const tstring& passwd, const tstring& remoteUser, unsigned int timeOut, bool masterRole, bool change=false);


	/// The method is called when request must be send through JS
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionsMgr.h file).
	virtual void SendRequestJS(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params){}

	/// NWL disconnection event handler
	/// Notifyes all services about NWL disconnection
	/// @param remoteUID - remote user id
	void OnNWLDisconnect(const tstring &remoteUID);
};
