/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CServicesMgr.h
///
///  CServicesMgr object declaration Service manager
///
///  @author Kirill Solovyov @date 12.11.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once

//#define MAX_SVC_COUNT_PER_SID 0x0000ffff

class CServicesMgr;
#include "CService.h"
#include "CRequestsMgr.h"

class CServicesMgr:
	public CInstanceTracker
{
	friend class CService;
protected:
	/// owner of object
	CRequestsMgr *m_owner;
	/// Services array
	std::map<unsigned long,boost::shared_ptr<CService> > m_services;
	/// last created service identifier. used by new service identifier generation
	unsigned long m_lastSvcId;

	/// The method is called by Service COM object when it wants send a request.
	/// @param svcId service identifier of service which call the method
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionMgr.h file).
	void RequestSent(unsigned long svcId, const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params);

	/// The method is called by Service COM object when it wants sub stream.
	/// @param svcId service identifier of service which call the method
	/// @param dstUserId unique destination user identifier
	/// @param dstSvcId unique per UserId destination service identifier
	/// @param streamId unique for this service stream identifier. it is not real sub stream identifier of Stream Multimplexer
	/// @param priorityLevel Priority level for the substream
	void GetSubStream(unsigned long svcId, const tstring& dstUserId, unsigned long dstSvcId, unsigned long streamId, unsigned long priorityLevel);

	CCriticalSection m_csServices;

public:
	CServicesMgr(CRequestsMgr *owner=NULL);
	virtual ~CServicesMgr(void);

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

	/// The method find service by session id and service id
	/// @param sId session identifier
	/// @param svcId service identifier
	/// @return service object
	boost::shared_ptr<CService> GetService(unsigned long svcId);

	/// The method determine whether service with some type has been created already
	/// @param svcType service types
	/// @return true if service with this type exist and false in otherwise
	bool IsSrvWithTypeExist(EBrokerServicesTypes svcType);

	/// The method pass request to Service COM object for handling
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of CSessionMgr.h file).
	void HandleRequest(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params);

	/// The method pass stream to the Service COM object
	/// @param svcId service identifier of service which called appropriate GetSubStream() method
	/// @param dstUserId unique destination user identifier
	/// @param dstSvcId unique per UserId destination service identifier
	/// @param streamId unique for this service stream identifier. it is not real sub stream identifier of Stream Multimplexer
	/// @param stream new sub stream
	void SetSubStream(unsigned long svcId, const tstring& dstUserId, unsigned long dstSvcId, unsigned long streamId, boost::shared_ptr<CAbstractStream> stream);

};
