/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSessionsMgr.cpp
///
///  CSessionsMgr object implementation (Sessions Manager)
///
///  @author Kirill Solovyov @date 05.11.2007
///
////////////////////////////////////////////////////////////////////////
#include "CSessionsMgr.h"


CSessionsMgr::CSessionsMgr(CRequestsMgr *owner):
	m_owner(owner),
	CInstanceTracker(_T("CSessionsMgr"))
{
TRY_CATCH

CATCH_LOG()
}

CSessionsMgr::~CSessionsMgr(void)
{
TRY_CATCH
	m_sessions.clear();
CATCH_LOG()
}


void CSessionsMgr::RegSession(const tstring& relaySrv, const tstring& sId, const tstring& user, const tstring& passwd, const tstring& remoteUser, unsigned int timeOut, bool masterRole, bool change)
{
TRY_CATCH
	CCritSection cs(&m_csSessions);
	if(m_sessions.find(remoteUser)!=m_sessions.end())
		if(change)//recreation session object
			Log.Add(_MESSAGE_,_T("Session with remote user id = [%s] RE-registered"),remoteUser.c_str());
		else 
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Session with remote user id = [%s] has been registered already"),remoteUser.c_str());
	m_sessions[remoteUser]=boost::shared_ptr<CMultiplexedSession>(new CMultiplexedSession(this));
	m_sessions[remoteUser]->Init(relaySrv,sId,user,passwd,remoteUser,timeOut,masterRole);
CATCH_THROW()
}

ESessionSentMessageState CSessionsMgr::SendRequest(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params)
{
TRY_CATCH
	CCritSection cs(&m_csSessions);
	if(m_sessions.find(dstUserId)==m_sessions.end())
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Session with destination user id = [%s] is not regestered"),dstUserId.c_str());
	//else
	//	Log.Add(_MESSAGE_,_T("CSessionsMgr::SendRequest(%s)"),sId.c_str());
	unsigned long dstUserIdSize=(dstUserId.size()+1)*sizeof(tstring::traits_type);
	unsigned long srcUserIdSize=(srcUserId.size()+1)*sizeof(tstring::traits_type);
	unsigned long paramsSize=(params.size()+1)*sizeof(tstring::traits_type);
	unsigned long requestSize=sizeof(dstUserIdSize)+dstUserIdSize+sizeof(dstSvcId)+sizeof(srcUserIdSize)+srcUserIdSize+sizeof(srcSvcId)+sizeof(rId)+sizeof(rType)+sizeof(param)+paramsSize;
	Log.Add(_MESSAGE_,_T("CSessionsMgr::SendRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s) request size =%d bytes"),dstUserId.c_str(),dstSvcId,srcUserId.c_str(),srcSvcId,rId,rType,param,params.c_str(),requestSize);
	boost::shared_ptr<SData> buf(reinterpret_cast<SData*>(new char[sizeof(SData)+requestSize]));
	buf->m_size=requestSize;
	char *request=buf->m_data;

	*reinterpret_cast<unsigned long*>(request)=dstUserIdSize;
	request+=sizeof(unsigned long);
	memcpy_s(request,dstUserIdSize,dstUserId.c_str(),dstUserIdSize);
	request+=dstUserIdSize;

	*reinterpret_cast<unsigned long*>(request)=dstSvcId;
	request+=sizeof(unsigned long);

	*reinterpret_cast<unsigned long*>(request)=srcUserIdSize;
	request+=sizeof(unsigned long);
	memcpy_s(request,srcUserIdSize,srcUserId.c_str(),srcUserIdSize);
	request+=srcUserIdSize;

	*reinterpret_cast<unsigned long*>(request)=srcSvcId;
	request+=sizeof(unsigned long);

	*reinterpret_cast<unsigned long*>(request)=rId;
	request+=sizeof(unsigned long);

	*reinterpret_cast<unsigned long*>(request)=rType;
	request+=sizeof(unsigned long);

	*reinterpret_cast<unsigned long*>(request)=param;
	request+=sizeof(unsigned long);

	memcpy_s(request,paramsSize,params.c_str(),paramsSize);
	request+=paramsSize;

	if(request-requestSize-sizeof(SData)!=reinterpret_cast<char*>(buf.get()))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Request pack failed. last address must be 0x%x real 0x%x"),reinterpret_cast<char*>(buf.get())+requestSize,request);

	ESessionSentMessageState state=m_sessions[dstUserId]->Send(buf);
	if(SSMS_ENQUEUE==state)
		m_sessions[dstUserId]->Connect();
	return state;
CATCH_THROW()
}

void CSessionsMgr::OnRequestReceived(const tstring& fromUserId, boost::shared_ptr<SData> data)
{
TRY_CATCH
	//Log.Add(_MESSAGE_,_T("CSessionsMgr::OnRequestReceived(%s,size=%d)"),fromUserId.c_str(),data->m_size);

	unsigned long dstUserIdSize;
	tstring dstUserId;
	unsigned long dstSvcId;
	unsigned long srcUserIdSize;
	tstring srcUserId;
	unsigned long srcSvcId;
	unsigned long rId;
	unsigned long rType;
	unsigned long param;
	tstring params;

	if(sizeof(dstUserIdSize)+sizeof(dstSvcId)+sizeof(srcUserIdSize)+sizeof(srcSvcId)+sizeof(rId)+sizeof(rType)+sizeof(param)>data->m_size)
		throw MCException("Data of the request was not recognized");

	char *request=data->m_data;

	dstUserIdSize=*reinterpret_cast<unsigned long*>(request);
	request+=sizeof(unsigned long);
	if(request>data->m_data+data->m_size)
		throw MCException("Data of the request was not recognized");
	dstUserId=reinterpret_cast<tstring::pointer>(request);
	request+=dstUserIdSize;
	if(request>data->m_data+data->m_size)
		throw MCException("Data of the request was not recognized");

	dstSvcId=*reinterpret_cast<unsigned long*>(request);
	request+=sizeof(unsigned long);
	if(request>data->m_data+data->m_size)
		throw MCException("Data of the request was not recognized");

	srcUserIdSize=*reinterpret_cast<unsigned long*>(request);
	request+=sizeof(unsigned long);
	if(request>data->m_data+data->m_size)
		throw MCException("Data of the request was not recognized");
	srcUserId=reinterpret_cast<tstring::pointer>(request);
	request+=srcUserIdSize;
	if(request>data->m_data+data->m_size)
		throw MCException("Data of the request was not recognized");

	srcSvcId=*reinterpret_cast<unsigned long*>(request);
	request+=sizeof(unsigned long);
	if(request>data->m_data+data->m_size)
		throw MCException("Data of the request was not recognized");

	rId=*reinterpret_cast<unsigned long*>(request);
	request+=sizeof(unsigned long);
	if(request>data->m_data+data->m_size)
		throw MCException("Data of the request was not recognized");

	rType=*reinterpret_cast<unsigned long*>(request);
	request+=sizeof(unsigned long);
	if(request>data->m_data+data->m_size)
		throw MCException("Data of the request was not recognized");

	param=*reinterpret_cast<unsigned long*>(request);
	request+=sizeof(unsigned long);
	if(request>data->m_data+data->m_size)
		throw MCException("Data of the request was not recognized");

	params=reinterpret_cast<tstring::pointer>(request);

	Log.Add(_MESSAGE_,_T("CSessionsMgr::OnRequestReceived(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),dstUserId.c_str(),dstSvcId,srcUserId.c_str(),srcSvcId,rId,rType,param,params.c_str());
	if(!m_owner)
		throw MCException("Owner has not been set");
	m_owner->OnRemoteRequest(fromUserId,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
CATCH_THROW()
}

bool CSessionsMgr::IsMasterRole(const tstring& dstUserId)
{
TRY_CATCH
	CCritSection cs(&m_csSessions);
	if(m_sessions.find(dstUserId)==m_sessions.end())
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("The session with userId=[%s] has not yet been registered"),dstUserId.c_str());
	return m_sessions[dstUserId]->IsMasterRole();
CATCH_THROW()
}

void CSessionsMgr::Disconnect(const tstring& dstUserId)
{
TRY_CATCH
	CCritSection cs(&m_csSessions);
	if(m_sessions.find(dstUserId)==m_sessions.end())
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("The session with userId=[%s] has not yet been registered"),dstUserId.c_str());
	m_sessions[dstUserId]->Disconnect();
CATCH_THROW()
}

void CSessionsMgr::ReportConnectionStatistics(const tstring& dstUserId)
{
	m_sessions[dstUserId]->ReportConnectionSpeed();
}

unsigned int CSessionsMgr::GetSubStream(const tstring& dstUserId, unsigned int subStreamId, unsigned int priorityLevel)
{
TRY_CATCH
	CCritSection cs(&m_csSessions);
	if(m_sessions.find(dstUserId)==m_sessions.end())
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("The session with userId=[%s] has not yet been registered"),dstUserId.c_str());
	return m_sessions[dstUserId]->GetSubStream(subStreamId,priorityLevel);
CATCH_THROW()
}

void CSessionsMgr::OnSubStreamTaken(const tstring& dstUserId, unsigned int subStreamId, boost::shared_ptr<CAbstractStream> stream)
{
TRY_CATCH
		//Log.Add(_MESSAGE_,_T("CSessionsMgr::OnSubStreamTaken(%s,%d,0x%x)"),dstUserId.c_str(),subStreamId,stream.get());
	if(!m_owner)
		throw MCException("Owner has not been set");
	m_owner->OnSubStreamTaken(dstUserId,subStreamId,stream);
CATCH_THROW()
}

ESessionState CSessionsMgr::GetSessionState(const tstring& dstUserId)
{
TRY_CATCH
	CCritSection cs(&m_csSessions);
	if(m_sessions.find(dstUserId)==m_sessions.end())
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("The session with userId=[%s] has not yet been registered"),dstUserId.c_str());
	return m_sessions[dstUserId]->GetSessionState();
CATCH_THROW()
}

boost::shared_ptr<CMultiplexedSession> CSessionsMgr::GetSession(const tstring& dstUserId)
{
TRY_CATCH
		CCritSection cs(&m_csSessions);
	if(m_sessions.find(dstUserId)==m_sessions.end())
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("The session with userId=[%s] has not yet been registered"),dstUserId.c_str());
	return m_sessions[dstUserId];
CATCH_THROW()
}

void CSessionsMgr::OnConnecting(const tstring& fromUserId,const tstring& message)
{
TRY_CATCH
	if(!m_owner)
		throw MCException("Owner has not been set");
	m_owner->OnRemoteConnecting(fromUserId,message);
CATCH_THROW()
}