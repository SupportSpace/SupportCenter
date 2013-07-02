/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRequestsMgr.cpp
///
///  CRequestsMgr object implementation. Requests manager
///8
///  @author Kirill Solovyov @date 14.11.2007
///
////////////////////////////////////////////////////////////////////////
#include "CRequestsMgr.h"

CRequestsMgr::CRequestsMgr(void):
	m_currentRId(0),
	CInstanceTracker(_T("CRequestsMgr"))
{
TRY_CATCH
	m_sessionsMgr.reset(new CSessionsMgr(this));
	m_servicesMgr.reset(new CServicesMgr(this));
CATCH_LOG()
}

CRequestsMgr::~CRequestsMgr(void)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CRequestsMgr::~CRequestsMgr()+"));
	CBlockerBlock b(&m_b);
	m_servicesMgr.reset();
	m_sessionsMgr.reset();
	Log.Add(_MESSAGE_,_T("CRequestsMgr::~CRequestsMgr()-"));
CATCH_LOG()
}

void CRequestsMgr::OnRequest(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params)
{
TRY_CATCH
	CBlockerUse b(&m_b);
	
	Log.Add(_MESSAGE_,_T("CRequestsMgr::OnRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),dstUserId.c_str(),dstSvcId,srcUserId.c_str(),srcSvcId,rId,rType,param, params.substr(0, MSG_BUF_SIZE - 100).c_str());
	if (BRT_START_WATCHDOG==rType)
	{
		/// Starting watchdog timer
		Log.Add(_MESSAGE_,_T("Watchdog request from pid(%d) received"),param);
		CSingleton<CSuicideProcessWatchDog>::instance().AddClient(param);
		CProcessSingleton<COriginalIEPIDWrapper>::instance().SetPid(param);
		return;
	}
	if(dstUserId!=m_params.m_userId)
	{
		if(BRT_SERVICE==rType)
		{
			//MessageBox(NULL,_T("blah"),NULL,0);
			//throw MCException("AL;DFALKJFL;AKJDFLKASJDFLKJSDLA;KFJDA;LKDFJL;AKJDF;LKAJFD;L");
			CCritSection cs(&m_csSvcReqWaiting);
			if(m_svcReqWaiting.find(dstUserId)!=m_svcReqWaiting.end())
				OnRequest(srcUserId,srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,rId,BRT_SERVICE|BRT_RESPONSE,BRR_BUSY,_T("One request has been sent to unconnected remote user already"));
			else if(SSMS_ENQUEUE==m_sessionsMgr->SendRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params))
			//if(SSMS_SENT==m_sessionsMgr->SendRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params))
			{
				//SendRequestJS(dstUserId,BSVCIDPDV_JS,srcUserId,srcSvcId,rId,rType,param,params);
				tstring& reqParams=Format(BRT_SERVICE_FIRST_FORMAT,BRT_SERVICE_APPROVE,BRT_SERVICE_DECLINE);
				SendRequestJS(dstUserId,BSVCIDPDV_JS,srcUserId,srcSvcId,BRIDPDV_SERVICE_FIRST,rType,param,reqParams);
				//m_svcReqWaiting.insert(dstUserId);//lock others serivce requests on this user id
				m_svcReqWaiting[dstUserId]=srcSvcId;//lock others serivce requests on this user id
			}
		}
		else
			m_sessionsMgr->SendRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
	}
	else
	{
		if(BSVCIDPDV_BROKER==dstSvcId)
		{
			//Log.Add(_MESSAGE_,_T("CRequestsMgr::OnRequest(BSVCIDPDV_BROKER)"));
			if(BRT_PING==rType)
			{
				//Log.Add(_MESSAGE_,_T("CRequestsMgr::OnRequest(BSVCIDPDV_BROKER, BRT_PING)"));
				m_sessionsMgr->SendRequest(srcUserId,srcSvcId,srcUserId,dstSvcId,rId,rType|BRT_RESPONSE,param,params);//response on ping
			}
			else if((BRT_PING|BRT_RESPONSE)==rType)
			{
				//Log.Add(_MESSAGE_,_T("CRequestsMgr::OnRequest(BSVCIDPDV_BROKER, BRT_PING|BRT_RESPONSE)"));
			}
			else if(BRT_SERVICE==rType)
			{
				//Log.Add(_MESSAGE_,_T("CRequestsMgr::OnRequest(BSVCIDPDV_BROKER,BRT_SERVICE,svc type=0x%x)"),param);
				{
					CCritSection cs(&m_csSvcReqFromJS);
					if(m_svcReqFromJS.find(srcUserId)!=m_svcReqFromJS.end()&&
						 m_svcReqFromJS[srcUserId].m_dstUserId==srcUserId&&
						 m_svcReqFromJS[srcUserId].m_dstSvcId==srcSvcId&&
						 m_svcReqFromJS[srcUserId].m_rId==rId)
					{
						// session start request - start service without of message box
						m_svcReqFromJS.erase(srcUserId);
						cs.Unlock();
						BRT_SERVICE_Handler(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
					}
					else
					{
						cs.Unlock();
						unsigned long mbRId=RId();
						{
							CCritSection cs(&m_csRequestsPool);
							m_requestsPool[mbRId]=SRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
						}
						OnRequest(m_params.m_userId,BSVCIDPDV_JS,m_params.m_userId,BSVCIDPDV_BROKER,mbRId,BRT_MESSAGEBOX,2/*two buttons*/,params);
						OnRequest(srcUserId,srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,mbRId,BRT_INSTALLATION,0/*0%*/,BRT_SERVICE_WAITING_APPROVE_APPLICATION);
					}
				}
			}
			else if((BRT_SERVICE|BRT_RESPONSE)==rType)
			{
				//Log.Add(_MESSAGE_,_T("CRequestsMgr::OnRequest(BSVCIDPDV_BROKER,BRT_SERVICE|BRT_RESPONSE, param=0x%x params=[%s])"),param,params.c_str());
				unsigned long lSvcId,rSvcId;
				lSvcId=_tstol(params.c_str());
				tstring::size_type iRSvcId=params.find(_T(";;"));
				if(iRSvcId==tstring::npos||iRSvcId+2>=params.size())
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Params value is not recognized; params=[%s]"),params.c_str());
				rSvcId=_tstol(params.c_str()+iRSvcId+2);
				boost::shared_ptr<CService> svc=m_servicesMgr->GetService(lSvcId);
				//if(!svc.get())
				//	throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Service with svcId=0x%x does not exist"),lSvcId));
				svc->m_params.m_remoteSvcId=rSvcId;
				//TODO remove
				//m_sessionsMgr->GetSubStream(srcUserId,SUBSTREAMID_AUTOSET,1);

				//CreateToolService(srcUserId,15,m_params.m_userId,23,BST_RCVIEWER,NULL);
			}
			else if(BRT_SUBSTREAM==rType)
			{
				//Log.Add(_MESSAGE_,_T("CRequestsMgr::OnRequest(BSVCIDPDV_BROKER,BRT_SUBSTREAM, param=0x%x params=[%s])"),param,params.c_str());
				unsigned long _dstSvcId,_srcSvcId,_svcSubStreamId,_subStreamId;
				_dstSvcId=_tstol(params.c_str());
				
				tstring::size_type iRSvcId=params.find(_T(";;"));
				if(iRSvcId==tstring::npos||iRSvcId+2>=params.size())
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Params value is not recognized; params=[%s]"),params.c_str());
				_srcSvcId=_tstol(params.c_str()+iRSvcId+2);
				
				iRSvcId=params.find(_T(";;"),iRSvcId+2);
				if(iRSvcId==tstring::npos||iRSvcId+2>=params.size())
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Params value is not recognized; params=[%s]"),params.c_str());
				_svcSubStreamId=_tstol(params.c_str()+iRSvcId+2);
				
				iRSvcId=params.find(_T(";;"),iRSvcId+2);
				if(iRSvcId==tstring::npos||iRSvcId+2>=params.size())
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Params value is not recognized; params=[%s]"),params.c_str());
				_subStreamId=_tstol(params.c_str()+iRSvcId+2);
				//Log.Add(_MESSAGE_,_T("CRequestsMgr::OnRequest(BSVCIDPDV_BROKER,BRT_SUBSTREAM, [0x%x,0x%x,0x%x,%d]"),_dstSvcId,_srcSvcId,_svcSubStreamId,_subStreamId);
				innerGetSubStream(srcUserId,_dstSvcId,_srcSvcId,_svcSubStreamId,_subStreamId,0/*no matter*/);
			}
			else if((BRT_MESSAGEBOX|BRT_RESPONSE)==rType)
			{
				//Log.Add(_MESSAGE_,_T("CRequestsMgr::OnRequest(BRT_MESSAGEBOX|BRT_RESPONSE)"));
				SRequest req;
				{
					CCritSection cs(&m_csRequestsPool);
					if(m_requestsPool.find(rId)==m_requestsPool.end())
						throw MCException("BRT_MESSAGEBOX|BRT_RESPONSE request doesn't exist in request pool");
					req=m_requestsPool[rId];
					m_requestsPool.erase(rId);
				}
				if(0/*approve button*/==param)
				{
					TRY_CATCH
						//create service
						BRT_SERVICE_Handler(req.m_dstUserId,req.m_dstSvcId,req.m_srcUserId,req.m_srcSvcId,req.m_rId,req.m_rType,req.m_param,req.m_params);
						return;
					CATCH_LOG()
					OnRequest(req.m_srcUserId,req.m_srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,req.m_rId,req.m_rType|BRT_RESPONSE,BRR_ERROR,
					          Format(_T("%d;;%s;;%d;;%s"),0/*reserved*/,req.m_dstUserId.c_str(),req.m_dstSvcId,req.m_params.c_str()));
				}
				else if(BRR_BUSY==param)//handler busy - only one question on customer's side
				{
					m_sessionsMgr->SendRequest(req.m_srcUserId,req.m_srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,req.m_rId,BRT_SERVICE|BRT_RESPONSE,BRR_BUSY,_T(""));
				}
				else
				{
					// send decline response
					m_sessionsMgr->SendRequest(req.m_srcUserId,req.m_srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,req.m_rId,BRT_SERVICE|BRT_RESPONSE,BRR_DECLINED,_T(""));
				}
			}
			else if(BRT_SERVICE_DESTROYED==rType)
			{
				unsigned long svcId;
				svcId=_tstol(params.c_str());
				//SendRequestJS(dstUserId,BSVCIDPDV_JS,srcUserId,BSVCIDPDV_BROKER,RId(),BRT_SERVICE_DESTROYED,svcId,i2tstring(param));
				SendRequestJS(dstUserId,BSVCIDPDV_JS,srcUserId,BSVCIDPDV_BROKER,RId(),BRT_SERVICE_DESTROYED,param,params);
				//boost::shared_ptr<CService> svc=m_servicesMgr->GetService(svcId);//exception if it's not found
				//OnRequest(m_params.m_userId,svcId,m_params.m_userId,BSVCIDPDV_BROKER,RId(),BRT_SERVICE_DESTROYED,svcId,i2tstring(param));
				OnRequest(dstUserId,param,srcUserId,BSVCIDPDV_BROKER,RId(),BRT_SERVICE_DESTROYED,param,params);
				//m_servicesMgr->DestroyToolService(svcId);

				std::vector<SRequest> deletedRequests;
				{
					CCritSection cs(&m_csRequestsPool);
					std::map<unsigned int,SRequest>::iterator i=m_requestsPool.begin();
					while(i!=m_requestsPool.end())
					{
						if(i->second.m_srcUserId==srcUserId&&i->second.m_srcSvcId==svcId)//request sent by destroyed service
						{
							deletedRequests.push_back(i->second);
							unsigned int key=i->first;
							++i;
							m_requestsPool.erase(key);
						}
						else
							++i;
					}
				}
				for(std::vector<SRequest>::iterator i=deletedRequests.begin();i!=deletedRequests.end();++i)
				{
					SendRequestJS(i->m_dstUserId,BSVCIDPDV_JS,m_params.m_userId,BSVCIDPDV_BROKER,RId(),BRT_SERVICE_DESTROYED,
					              BSVCIDPDV_BROKER,i2tstring(BSVCIDPDV_BROKER));
				}
			}
			else if(BRT_GET_SERVICE_INFO==rType)
			{
				//TODO check if that user local
				boost::shared_ptr<CService> svc;
				svc=m_servicesMgr->GetService(srcSvcId);
				boost::shared_ptr<CMultiplexedSession> session;
				session=m_sessionsMgr->GetSession(svc->m_params.m_remoteUserId);
				m_servicesMgr->HandleRequest(srcUserId,srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,rId,BRT_GET_SERVICE_INFO|BRT_RESPONSE,0/*reserved*/,
				                             Format(_T("%s;;%s;;%s;;%s;;%s"),svc->m_params.m_userId.c_str(),
				                                                             session->m_params.m_userName.c_str(),
				                                                             svc->m_params.m_remoteUserId.c_str(),
				                                                             session->m_params.m_remoteUserName.c_str(),
				                                                             session->m_params.m_sId.c_str()));
			}
			else if((BRT_GET_SESSION_INFO|BRT_RESPONSE)==rType)
			{
				std::vector<tstring> info = tokenize(params.c_str(), _T(";"));
				if (info.size() != 4)
					Log.Add(_ERROR_,_T("Unknown BRT_SET_SESSION_INFO request string format [%s]"),params.c_str());
				else
				{
					tstring& userId=info[0];
					tstring& userName=info[1];
					tstring& remoteUserId=info[2];
					tstring& remoteUserName=info[3];
					boost::shared_ptr<CMultiplexedSession> session;
					session=m_sessionsMgr->GetSession(remoteUserId);
					session->m_params.m_userName=userName;
					session->m_params.m_remoteUserName=remoteUserName;
				}
			}
		}
		else if(BSVCIDPDV_JS==dstSvcId)
		{
			//Log.Add(_MESSAGE_,_T("CRequestsMgr::OnRequest(request to JS) has not implement"));
			SendRequestJS(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
		}
		else if(BSVCIDPDV_BROKER>dstSvcId || BSVCIDPDV_BROADCAST==dstSvcId)
			m_servicesMgr->HandleRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
		else 
			Log.Add(_WARNING_,_T("CRequestsMgr::OnRequest(). Request with dstSvcId=0x%x has not been handled becaus it is predefined value."),dstSvcId);
	}
CATCH_THROW()
}


void CRequestsMgr::OnRemoteRequest(const tstring& fromUserId, const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params)
{
//TRY_CATCH
//	{
//		CCritSection cs(&m_csSvcReqWaiting);
//		m_svcReqWaiting.erase(fromUserId);
//	}
//	OnRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
//CATCH_THROW()
TRY_CATCH
	TRY_CATCH
		{
			CCritSection cs(&m_csSvcReqWaiting);
			m_svcReqWaiting.erase(fromUserId);
		}
		OnRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
		return;
	CATCH_LOG()

	if(BRT_RESPONSE_FAILED==rType)
		throw MCException("BRT_RESPONSE_FAILED request handling WAS FAILED");
	else if((BRT_RESPONSE_FAILED|BRT_RESPONSE)==rType)
		throw MCException("BRT_RESPONSE_FAILED|BRT_RESPONSE (illegal) request handling WAS FAILED");
	else if(rType&BRT_RESPONSE)
	{
		//handling of response was failed
		OnRequest(srcUserId,srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,rId,BRT_RESPONSE_FAILED,0/*reserved*/,
		          Format(_T("%s;;%d;;%d;;%d;;%s"),dstUserId.c_str(),dstSvcId,rType,param,params.c_str()));
	}
	else
	{
		//handling of request was failed
		OnRequest(srcUserId,srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,rId,rType|BRT_RESPONSE,BRR_ERROR,
		          Format(_T("%d;;%s;;%d;;%s"),0/*reserved*/,dstUserId.c_str(),dstSvcId,params.c_str()));
	}
CATCH_THROW()
}

void CRequestsMgr::OnJSRequest(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params)
{
TRY_CATCH
	CBlockerUse b(&m_b);
	// first service request
	if((BRT_SERVICE|BRT_RESPONSE)==rType)
	{
		if(BRR_APPROVED==param)
		{
			{
				CCritSection cs(&m_csSvcReqFromJS);
				if(m_svcReqFromJS.find(dstUserId)!=m_svcReqFromJS.end())
					Log.Add(_MESSAGE_,_T("BRT_SERVICE|BRT_RESPONSE request from JS has existed already! old request=[%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s]"),
																m_svcReqFromJS[dstUserId].m_dstUserId.c_str(),
																m_svcReqFromJS[dstUserId].m_dstSvcId,
																m_svcReqFromJS[dstUserId].m_srcUserId.c_str(),
																m_svcReqFromJS[dstUserId].m_srcSvcId,
																m_svcReqFromJS[dstUserId].m_rId,
																m_svcReqFromJS[dstUserId].m_rType,
																m_svcReqFromJS[dstUserId].m_param,
																m_svcReqFromJS[dstUserId].m_params.c_str());
				m_svcReqFromJS[dstUserId]=SRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
			}
			//m_sessionsMgr->SendRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,BRT_PING,0,_T(" "));
			m_sessionsMgr->SendRequest(dstUserId,BSVCIDPDV_BROKER,srcUserId,BSVCIDPDV_BROKER,RId(),BRT_PING,0,_T(" "));
		}
		else if(BRR_DECLINED==param||BRR_BPFAILED==param)
		{
			{
				CCritSection cs(&m_csSvcReqWaiting);
				m_svcReqWaiting.erase(srcUserId);
				m_sessionsMgr->Disconnect(srcUserId);

			}
			OnRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
		}
	}
	else
		OnRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
CATCH_THROW()
}

void CRequestsMgr::OnSvcRequest(unsigned long svcId, const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params)
{
TRY_CATCH
	CBlockerUse b(&m_b);
	//Log.Add(_MESSAGE_,_T("CRequestsMgr::OnSvcRequest(%s)"),params.c_str());
	boost::shared_ptr<CService> svc=m_servicesMgr->GetService(svcId);
	tstring _dstUserId;
	if(dstUserId==BUSERIDPDV_AUTOSET)
		_dstUserId=svc->m_params.m_remoteUserId;
	else if(dstUserId==BUSERIDPDV_LOCAL)
		_dstUserId=svc->m_params.m_userId;
	else
		_dstUserId=dstUserId;
	unsigned long _dstSvcId=(dstSvcId==BSVCIDPDV_AUTOSET)?svc->m_params.m_remoteSvcId:dstSvcId;
	tstring _srcUserId=(srcUserId==BUSERIDPDV_AUTOSET)?svc->m_params.m_userId:srcUserId;
	unsigned long _srcSvcId=(srcSvcId==BSVCIDPDV_AUTOSET)?svc->m_params.m_svcId:srcSvcId;

	OnRequest(_dstUserId,_dstSvcId,_srcUserId,_srcSvcId,rId,rType,param,params);
CATCH_THROW()
}
void CRequestsMgr::OnSubStreamTaken(const tstring& dstUserId, unsigned int subStreamId, boost::shared_ptr<CAbstractStream> stream)
{
TRY_CATCH
	CBlockerUse b(&m_b);
	SSubStreamQueryParams query;
	Log.Add(_MESSAGE_,_T("CRequestsMgr::OnSubStreamTaken(%s,0x%x,0x%08x)"),dstUserId.c_str(),subStreamId,stream.get());
	{
		CCritSection cs(&m_csSubStreamQueries);
		std::vector<SSubStreamQueryParams>::iterator i;
		for(i=m_subStreamQueries.begin();i!=m_subStreamQueries.end();++i)
		{
			if( i->m_dstUserId==dstUserId&&i->m_subStreamId==subStreamId)
				break;//found
		}
		if(m_subStreamQueries.end()==i)
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Query is not found for connected sub stream userId=[%s] subStreamId=[0x%x]"),dstUserId.c_str(),subStreamId);
		query=*i;
		m_subStreamQueries.erase(i);
	}
	m_servicesMgr->SetSubStream(query.m_srcSvcId,query.m_dstUserId,query.m_dstSvcId,query.m_svcSubStreamId,stream);
CATCH_THROW()
}

void CRequestsMgr::OnGetSubStream(unsigned long svcId, const tstring& dstUserId, unsigned long dstSvcId, unsigned long svcSubStreamId, unsigned long priorityLevel)
{
TRY_CATCH
	CBlockerUse b(&m_b);
	//Log.Add(_MESSAGE_,_T("CRequestsMgr::OnGetSubStream(0x%x,%s,0x%x,0x%x,%d)"),svcId,dstUserId.c_str(),dstSvcId,svcSubStreamId,priorityLevel);
	boost::shared_ptr<CService> svc=m_servicesMgr->GetService(svcId);
	tstring _dstUserId=(dstUserId==BUSERIDPDV_AUTOSET)?svc->m_params.m_remoteUserId:dstUserId;
	unsigned long _dstSvcId=(dstSvcId==BSVCIDPDV_AUTOSET)?svc->m_params.m_remoteSvcId:dstSvcId;
	innerGetSubStream(_dstUserId,_dstSvcId,svcId,svcSubStreamId,SUBSTREAMID_AUTOSET,priorityLevel);
CATCH_THROW()
}
void CRequestsMgr::innerGetSubStream(const tstring& dstUserId, unsigned long dstSvcId, unsigned long srcSvcId, unsigned long svcSubStreamId, unsigned long subStreamId,unsigned long priorityLevel)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CRequestsMgr::innerGetSubStream(%s,0x%x,0x%x,0x%x,0x%x,%d)"),dstUserId.c_str(),dstSvcId,srcSvcId,svcSubStreamId,subStreamId,priorityLevel);
	CCritSection cs(&m_csSubStreamQueries);
	CBlockerUse b(&m_b);
	const bool master=m_sessionsMgr->IsMasterRole(dstUserId);//master role
	std::vector<SSubStreamQueryParams>::iterator i;
	for(i=m_subStreamQueries.begin();i!=m_subStreamQueries.end();++i)
	{
		if( i->m_dstUserId==dstUserId&&
		    i->m_dstSvcId==dstSvcId&&
		    i->m_srcSvcId==srcSvcId&&
		    i->m_svcSubStreamId==svcSubStreamId)
			break;//found
	}
	const bool found=m_subStreamQueries.end()!=i;//the query is found
	std::vector<SSubStreamQueryParams>::iterator& query=i;

	Log.Add(_MESSAGE_,_T("CRequestsMgr::innerGetSubStream() master=%d found=%d"),master,found);
	
	//handling
	if(SUBSTREAMID_AUTOSET==subStreamId)//the query from service of local side
	{
		if(master)
		{
			if(found)
				throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Sub stream query exist already [%s,0x%x,0x%x,0x%x,0x%x,%d]"),dstUserId.c_str(),dstSvcId,srcSvcId,svcSubStreamId,subStreamId,priorityLevel);
			unsigned long streamId=m_sessionsMgr->GetSubStream(dstUserId,SUBSTREAMID_AUTOSET,priorityLevel);
			m_subStreamQueries.push_back(SSubStreamQueryParams(dstUserId,dstSvcId,srcSvcId,svcSubStreamId,streamId,priorityLevel));
			m_sessionsMgr->SendRequest(dstUserId,BSVCIDPDV_BROKER,m_params.m_userId,BSVCIDPDV_BROKER,0/*no matter*/,BRT_SUBSTREAM,0/*no matter*/,Format(_T("%d;;%d;;%d;;%d"),srcSvcId,dstSvcId,svcSubStreamId,streamId));
		}
		else
		{
			if(found)
			{
				if(SUBSTREAMID_AUTOSET==query->m_subStreamId)
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Sub stream query exist already [%s,0x%x,0x%x,0x%x,0x%x,%d]"),dstUserId.c_str(),dstSvcId,srcSvcId,svcSubStreamId,subStreamId,priorityLevel);
				unsigned long streamId=m_sessionsMgr->GetSubStream(dstUserId,query->m_subStreamId,priorityLevel);
				query->m_subStreamId=streamId;
			}
			else
			{
				m_subStreamQueries.push_back(SSubStreamQueryParams(dstUserId,dstSvcId,srcSvcId,svcSubStreamId,SUBSTREAMID_AUTOSET,priorityLevel));
			}
		}
	}
	else //the query from service of remote side - handle of BRT_SUBSTREAM request which sent by side with masterRole
	{
		if(found)
		{
			unsigned long streamId=m_sessionsMgr->GetSubStream(query->m_dstUserId,subStreamId,query->m_priorityLevel);
			query->m_subStreamId=streamId;
		}
		else
		{
			m_subStreamQueries.push_back(SSubStreamQueryParams(dstUserId,dstSvcId,srcSvcId,svcSubStreamId,subStreamId,priorityLevel));
		}
	}
	//TODO romove loging
	tstring queries=_T("Queries in m_subStreamQueries:\n");
	std::vector<SSubStreamQueryParams>::iterator j;
	for(j=m_subStreamQueries.begin();j!=m_subStreamQueries.end();++j)
	{
		queries+=Format(_T("[%s,0x%x,0x%x,0x%x,0x%x,%d]\n"),j->m_dstUserId.c_str(),j->m_dstSvcId,j->m_srcSvcId,j->m_svcSubStreamId,j->m_subStreamId,j->m_priorityLevel);
	}
	Log.Add(_MESSAGE_,_T("%s"),queries.c_str());
CATCH_THROW()
}

void CRequestsMgr::BRT_SERVICE_Handler(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params)
{
TRY_CATCH
	CBlockerUse b(&m_b);
	unsigned long newSvcId=CreateToolService(srcUserId,srcSvcId,m_params.m_userId,BSVCIDPDV_AUTOSET,EBrokerServicesTypes(param),NULL/*COM object*/);
	//m_servicesMgr->GetService(CreateToolService(srcUserId,srcSvcId,m_params.m_userId,BSVCIDPDV_AUTOSET,EBrokerServicesTypes(param),NULL/*COM object*/))->DestroyToolService();
	m_sessionsMgr->SendRequest(srcUserId,BSVCIDPDV_BROKER,m_params.m_userId,dstSvcId,rId,rType|BRT_RESPONSE,BRR_APPROVED,i2tstring(srcSvcId)+_T(";;")+i2tstring(newSvcId));//response on service request to broker for remote user id set
	m_sessionsMgr->SendRequest(srcUserId,srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,rId,rType|BRT_RESPONSE,BRR_APPROVED,i2tstring(srcSvcId)+_T(";;")+i2tstring(newSvcId));//response on service request
CATCH_LOG()
}

unsigned long CRequestsMgr::CreateToolService(const tstring& remoteUserId,unsigned long remoteSvcId, const tstring& userId, unsigned long svcId,EBrokerServicesTypes svcType, IDispatch* host)
{
TRY_CATCH
	CBlockerUse b(&m_b);
	unsigned long newSvcId;
	newSvcId=m_servicesMgr->CreateToolService(remoteUserId,remoteSvcId,userId,svcId,svcType,host);
	return newSvcId;
CATCH_THROW()
}

void CRequestsMgr::DestroyToolService(unsigned long svcId)
{
TRY_CATCH
	CBlockerUse b(&m_b);
	{
		//firs service request removing
		CCritSection cs(&m_csSvcReqWaiting);
		for(std::map<tstring,unsigned long>::iterator i=m_svcReqWaiting.begin();i!=m_svcReqWaiting.end();)
		{
			if(i->second==svcId)
			{
				tstring userId=i->first;
				++i;
				m_svcReqWaiting.erase(userId);
				m_sessionsMgr->Disconnect(userId);
				TRY_CATCH
					boost::shared_ptr<CService> svc=m_servicesMgr->GetService(svcId);
					SendRequestJS(svc->m_params.m_remoteUserId,BSVCIDPDV_JS,m_params.m_userId,BSVCIDPDV_BROKER,RId(),BRT_SERVICE_DESTROYED,
					              svc->m_params.m_remoteSvcId,i2tstring(svcId));
				CATCH_LOG()
				break;
			}
			++i;
		}
	}
	{
		boost::shared_ptr<CService> svc=m_servicesMgr->GetService(svcId);
		if(SS_CONNECTED==m_sessionsMgr->GetSessionState(svc->m_params.m_remoteUserId))
			OnRequest(svc->m_params.m_remoteUserId,BSVCIDPDV_BROKER,m_params.m_userId,BSVCIDPDV_BROKER,RId(),BRT_SERVICE_DESTROYED,
			          svc->m_params.m_remoteSvcId,i2tstring(svcId));
	}
	//OnRequest(m_params.m_userId,svcId,m_params.m_userId,BSVCIDPDV_BROKER,RId(),BRT_SERVICE_DESTROYED,svcId,_T(""));
	m_servicesMgr->DestroyToolService(svcId);
CATCH_THROW()
}

void CRequestsMgr::Init(const tstring& userId, const tstring& passwd)
{
TRY_CATCH
	if(userId!=m_params.m_userId)
		MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("User name change old=[%s] new=[%s]"),m_params.m_userId.c_str(),userId.c_str()));
	m_params.m_userId=userId;
	m_params.m_passwd=passwd;
CATCH_THROW()
}

void CRequestsMgr::InitSession(const tstring& relaySrv, const tstring& sId, const tstring& user, const tstring& passwd, const tstring& remoteUser, unsigned int timeOut, bool masterRole, bool change)
{
TRY_CATCH
	CBlockerUse b(&m_b);
	m_sessionsMgr->RegSession(relaySrv,sId,user,passwd,remoteUser,timeOut,masterRole,change);
	SendRequestJS(user,BSVCIDPDV_JS,user,BSVCIDPDV_BROKER,RId(),BRT_GET_SESSION_INFO,0,_T(""));
CATCH_LOG()
}

unsigned long CRequestsMgr::RId(void)
{
TRY_CATCH
	return InterlockedIncrement(reinterpret_cast<long*>(&m_currentRId));
CATCH_THROW()
}

void CRequestsMgr::OnNWLDisconnect(const tstring &remoteUID)
{
TRY_CATCH
	if (NULL != m_sessionsMgr.get())
	{
		OnRequest(	m_params.m_userId, 
					BSVCIDPDV_BROADCAST,
					m_params.m_userId,
					BSVCIDPDV_BROKER,
					RId(),
					BRT_NWL_DISCONNECTED,
					0,
					remoteUID);
	}
CATCH_LOG()
}

void CRequestsMgr::OnRemoteConnecting(const tstring& fromUserId,const tstring& message)
{
TRY_CATCH
	CBlockerUse b(&m_b);
	CCritSection cs(&m_csSvcReqWaiting);
	if(m_svcReqWaiting.find(fromUserId)!=m_svcReqWaiting.end())
	{
		unsigned long svcId=m_svcReqWaiting[fromUserId];
		cs.Unlock();
		OnRequest(m_params.m_userId,svcId,m_params.m_userId,BSVCIDPDV_BROKER,RId(),BRT_CONNECTION,0/*no matter*/,message);
	}
CATCH_THROW()
}