/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CServicesMgr.cpp
///
///  CServicesMgr object implementation. Service manager
///
///  @author Kirill Solovyov @date 12.11.2007
///
////////////////////////////////////////////////////////////////////////
#include "CServicesMgr.h"
#include "BrokersTypes.h"

CServicesMgr::CServicesMgr(CRequestsMgr *owner):
	m_owner(owner),
	m_lastSvcId(0),
	CInstanceTracker(_T("CServicesMgr"))
{
TRY_CATCH
CATCH_LOG()
}

CServicesMgr::~CServicesMgr(void)
{
TRY_CATCH
	//m_owner=NULL;
	m_services.clear();
CATCH_LOG()
}

unsigned long CServicesMgr::CreateToolService(const tstring& remoteUserId,unsigned long remoteSvcId, const tstring& userId, unsigned long svcId,EBrokerServicesTypes svcType, IDispatch* host)
{
TRY_CATCH
	unsigned long newSvcId;
	boost::shared_ptr<CService> service;
	{
		CCritSection cs(&m_csServices);
		if(svcId!=BSVCIDPDV_AUTOSET)
		{
			if(svcId>=BSVCIDPDV_BROKER)
				throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Service identifier svcId=0x%x is predefined value"),svcId));
			if(m_services.find(svcId)!=m_services.end())
				throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Service with svcId=0x%x has existed already"),svcId));
			m_lastSvcId=svcId;
		}
		else
		{
			
			unsigned long i;
			for(i=m_lastSvcId+1; m_services.find(i)!=m_services.end()&&i!=m_lastSvcId; ++i)
			{
				if(i==BSVCIDPDV_BROKER)
					i=0;
			}
			if(i==m_lastSvcId)
				throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Free svcIds does not exist more"));
			m_lastSvcId=i;
		}
		m_services[m_lastSvcId].reset(new CService(this));

		boost::shared_ptr<CService> svc=m_services[m_lastSvcId];
		svc->m_params.m_userId=userId;
		svc->m_params.m_remoteUserId=remoteUserId;
		svc->m_params.m_svcId=m_lastSvcId;
		svc->m_params.m_remoteSvcId=remoteSvcId;

		newSvcId=m_lastSvcId;
		service=m_services[newSvcId];
	}
	if(host)
		service->CreateToolService(svcType,host);
	else
		service->CreateToolService(svcType);
	return newSvcId;
CATCH_THROW()
}

void CServicesMgr::DestroyToolService(unsigned long svcId)
{
TRY_CATCH
	CCritSection cs(&m_csServices);
	if(svcId>=BSVCIDPDV_BROKER)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Service identifier svcId=0x%x is predefined value"),svcId));
	if(m_services.find(svcId)==m_services.end())
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Service with svcId=0x%x does not exist"),svcId));
	boost::shared_ptr<CService> svc=m_services[svcId];
	m_services.erase(svcId);

	cs.Unlock();
	//m_services.erase(svcId);
CATCH_THROW()
}


boost::shared_ptr<CService> CServicesMgr::GetService(unsigned long svcId)
{
TRY_CATCH
	CCritSection cs(&m_csServices);
	if(svcId>=BSVCIDPDV_BROKER)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Service identifier svcId=0x%x is predefined value"),svcId));
	if(m_services.find(svcId)==m_services.end())
		//return boost::shared_ptr<CService>(reinterpret_cast<CService*>(NULL));
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Service with svcId=0x%x does not exist"),svcId));
	return m_services[svcId];
CATCH_THROW()
}

bool CServicesMgr::IsSrvWithTypeExist(EBrokerServicesTypes svcType)
{
TRY_CATCH
	CCritSection cs(&m_csServices);
	std::map<unsigned long,boost::shared_ptr<CService> >::iterator i;
	for(i=m_services.begin();i!=m_services.end();++i)
	{
			if(i->second->GetSvcType()==svcType)
				return true;
	}
	return false;
CATCH_THROW()
}

void CServicesMgr::HandleRequest(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params)
{
TRY_CATCH
	CCritSection cs(&m_csServices);
	if(BSVCIDPDV_BROADCAST == dstSvcId)
	{
		for(std::map<unsigned long,boost::shared_ptr<CService> >::iterator service = m_services.begin();
			m_services.end() != service;
			++service)
		{
			boost::shared_ptr<CService> svc=service->second;
			cs.Unlock();
			TRY_CATCH
				svc->HandleRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
			CATCH_LOG()
		}
	} else
	if(dstSvcId>=BSVCIDPDV_BROKER)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Service identifier dstSvcId=0x%x is predefined value. HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),dstSvcId,dstUserId.c_str(),dstSvcId,srcUserId.c_str(),srcSvcId,rId,rType,param,params.c_str());
	else
	if(m_services.find(dstSvcId)==m_services.end())
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Service does not exist. HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),dstUserId.c_str(),dstSvcId,srcUserId.c_str(),srcSvcId,rId,rType,param,params.c_str());
	else
	{
		boost::shared_ptr<CService> svc=m_services[dstSvcId];
		cs.Unlock();
		svc->HandleRequest(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
	}
CATCH_THROW()
}

void CServicesMgr::RequestSent(unsigned long svcId, const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params)
{
TRY_CATCH
	if(!m_owner)
		throw MCException("Owner don't set");
	m_owner->OnSvcRequest(svcId,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
CATCH_THROW()
}

void CServicesMgr::SetSubStream(unsigned long svcId, const tstring& dstUserId, unsigned long dstSvcId, unsigned long streamId, boost::shared_ptr<CAbstractStream> stream)
{
TRY_CATCH
	CCritSection cs(&m_csServices);
	if(dstSvcId>=BSVCIDPDV_BROKER)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Service identifier svcId=0x%x is predefined value. SetSubStream(0x%x,%s,0x%x,0x%x,0x%08x)"),svcId,dstUserId.c_str(),dstSvcId,streamId,stream.get());
	if(m_services.find(svcId)==m_services.end())
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Service does not exist. SetSubStream(0x%x,%s,0x%x,0x%x,0x%08x)"),svcId,dstUserId.c_str(),dstSvcId,streamId,stream.get());
	boost::shared_ptr<CService> svc=m_services[svcId];
	cs.Unlock();
	svc->SetSubStream(dstUserId,dstSvcId,streamId,stream);
CATCH_THROW()
}

void CServicesMgr::GetSubStream(unsigned long svcId, const tstring& dstUserId, unsigned long dstSvcId, unsigned long streamId, unsigned long priorityLevel)
{
TRY_CATCH
	if(!m_owner)
		throw MCException("Owner don't set");
	//Log.Add(_MESSAGE_,_T("CServicesMgr::OnGetSubStream(0x%x,%s,0x%x,0x%x,%d)"),svcId,dstUserId.c_str(),dstSvcId,streamId,priorityLevel);
	m_owner->OnGetSubStream(svcId,dstUserId,dstSvcId,streamId,priorityLevel);
CATCH_THROW()
}
