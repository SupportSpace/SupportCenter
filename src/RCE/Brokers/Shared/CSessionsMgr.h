
///
///  Archer Software.
///
///  CSessionMgr.h
///
///  CSessionMgr object declaration (Sessions Manager)
///
///  @author Kirill Solovyov @date 05.11.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once


#include <AidLib/Strings/tstring.h>


class CSessionsMgr;
#include "CMultiplexedSession.h"
#include "CRequestsMgr.h"


/// It is current(05.11.2007) state of Request topic of Infrastructure document
/// Request is ask to perform standard action, can contain following data:
/// sId – unique session id. It has predefined value:
///		BSID_LOCAL – request sends to local side
///		BSID_AUTOSET – must be set by request handler
/// svcId – unique for SID service id. It has predefined value:
///		BSVCID_BROKER – request sends to Broker/VBroker
///		BSVCID_JS – request sends to JS;
///		BSVCID_AUTOSET – must be set by request handler  (for service it do by Broker/VBroker)
///	rId – unique for Service request id. It has predefined value: 
///		BRID_AUTOSET – must be set by request handler (for service it do by Broker/VBroker).
/// rType – one of actions types. For type requests see Request types.
///		param – DWORD parameter, rType depended parameters. When request is respond, the param is result of the request.
///		params - packed to string, rType depended parameters

//TODO current state of requests types



/// The class is Session Manager
class CSessionsMgr : public CInstanceTracker
{
protected:
	/// owner of object
	CRequestsMgr *m_owner;
	/// map of sessions
	std::map<tstring,boost::shared_ptr<CMultiplexedSession> > m_sessions;

	//TODO remove this, all thread safe in RequestMgr
	/// m_sessions safe access 
	CCriticalSection m_csSessions;
public:
	///ctor
	CSessionsMgr(CRequestsMgr *owner=NULL);
	///dtor
	virtual ~CSessionsMgr(void);

	/// Submets connection statistics to relay
	void ReportConnectionStatistics(const tstring& dstUserId);

	/// The method initialize new session (but don’t connect, session is created by demand (by first call SendRequest method)
	/// @param relaySrv Relay server address 
	/// @param sId session identifier
	/// @param user user name
	/// @param passwd user pass word
	/// @param remoteUser remote user name
	/// @param timeOut time of connection
	/// @param masterRole
	/// @param change if flag set, existed session will be recreated
	void RegSession(const tstring& relaySrv, const tstring& sId, const tstring& user, const tstring& passwd, const tstring& remoteUser, unsigned int timeOut, bool masterRole, bool change=false);

	/// The method retrieve (master/slave) session's side role
	/// @param dstUserId destination user identifier
	/// @return true if it is muster role and false if slave
	bool IsMasterRole(const tstring& dstUserId);

	/// The method disconnect session
	/// @param dstUserId destination user identifier
	void Disconnect(const tstring& dstUserId);

	/// The method pack and send request to remote side identified by destination user id
	/// Connect new session if session has not yet been connected
	/// for parameters detail see Request topic of Infrastructure document (current (file creation) state see on top of file).
	/// @return state of message. See ESessionSentMessageState for details
	ESessionSentMessageState SendRequest(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params);

	/// The method is called by request retrieved from remote side
	/// @param sId session identifier
	/// @param data packed request
	void OnRequestReceived(const tstring& fromUserId, boost::shared_ptr<SData> data);
	
	/// The method is called when connection progress change its state
	/// @param fromUserId user identifier where from come request
	/// @param message text describe connection status
	void OnConnecting(const tstring& fromUserId,const tstring& message);


	/// The method ask sub stream (asynchronous).
	/// @param dstUserId unique destination user identifier
	/// @param subStreamId Service identifier - real sub stream identifier, can be SUBSTREAMID_AUTOSET
	/// @param priorityLevel Priority level for the substream
	/// @return real stream identifier
	unsigned int GetSubStream(const tstring& dstUserId, unsigned int subStreamId, unsigned int priorityLevel);

	/// The method is called when sub stream is connected
	/// @param dstUserId unique destination user identifier
	/// @param subStreamId Service identifier
	/// @param stream new sub stream
	void OnSubStreamTaken(const tstring& dstUserId, unsigned int subStreamId, boost::shared_ptr<CAbstractStream> stream);

	/// The method return session state
	/// @param dstUserId unique destination user identifier
	/// @return ESessionState state of session
	ESessionState GetSessionState(const tstring& dstUserId);

	/// The method return session object
	/// @param dstUserId unique destination user identifier
	/// @return CMultiplexedSession object
	boost::shared_ptr<CMultiplexedSession> GetSession(const tstring& dstUserId);

	/// Returns pointer to request manager object
	/// or NULL if it didn't set
	inline CRequestsMgr* GetRequestManager()
	{
		return m_owner;
	}
};
