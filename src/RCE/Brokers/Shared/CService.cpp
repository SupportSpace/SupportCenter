/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CService.cpp
///
///  CService object implementation. The object implement wrapper for Service COM object (creation, events and others)
///
///  @author Kirill Solovyov @date 12.11.2007
///
////////////////////////////////////////////////////////////////////////
#include "CService.h"
#include <AidLib/Strings/tstring.h>
//#include <AidLib/CException/CException.h>
#include <AidLib/Com/ComException.h>
#include <RCEngine/AXstuff/AXstuff.h>

//-------------------------------------------------------------------------------------------------------------------------

C_IBrokerClientEvents::C_IBrokerClientEvents():
	m_owner(NULL),
	CInstanceTracker(_T("C_IBrokerClientEvents"))
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("C_IBrokerClientEvents::C_IBrokerClientEvents()"));
CATCH_LOG()
}

C_IBrokerClientEvents::~C_IBrokerClientEvents(void)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("C_IBrokerClientEvents::~C_IBrokerClientEvents()"));
CATCH_LOG()
}


HRESULT C_IBrokerClientEvents::FinalConstruct()
{
TRY_CATCH
CATCH_LOG()
	return S_OK;
}

void C_IBrokerClientEvents::FinalRelease()
{
TRY_CATCH
CATCH_THROW()
}
STDMETHODIMP C_IBrokerClientEvents::RequestSent(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params)
{
TRY_CATCH_COM
	if(!m_owner)
		throw MCException("Owner doesn't set");
	USES_CONVERSION;
	TCHAR *p=_OLE2T(dstUserId);
	tstring s(p);
	m_owner->RequestSent(_OLE2T(dstUserId),dstSvcId,_OLE2T(srcUserId),srcSvcId,rId,rType,param,_OLE2T(params));
CATCH_LOG_COM
}

STDMETHODIMP C_IBrokerClientEvents::GetSubStream(BSTR dstUserId, ULONG dstSvcId, ULONG streamId, ULONG priorityLevel)
{
TRY_CATCH_COM
	if(!m_owner)
		throw MCException("Owner doesn't set");
	USES_CONVERSION;
	//TODO remove log
	Log.Add(_MESSAGE_,_T("C_IBrokerClientEvents::OnGetSubStream(%s,0x%x,0x%x,%d)"),_OLE2T(dstUserId),dstSvcId,streamId,priorityLevel);
	m_owner->GetSubStream(_OLE2T(dstUserId),dstSvcId,streamId,priorityLevel);
CATCH_LOG_COM
}

//-------------------------------------------------------------------------------------------------------------------------

CService::CService(CServicesMgr *owner):
	m_owner(owner),
	CInstanceTracker(_T("CService"))
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CService::CService()"));
CATCH_LOG()
}

CService::~CService(void)
{
TRY_CATCH
	DestroyToolService();
	Log.Add(_MESSAGE_,_T("CService::~CService()"));
CATCH_LOG()
}

void CService::CreateToolService(EBrokerServicesTypes svcType, IDispatch* host)
{
TRY_CATCH
	HRESULT result;
	//first variant
	if(!host)
		throw MCException("The \"host\" DHTML object can't have NULL value");
	CComPtr<IDispatch> hostDisp;
	hostDisp.Attach(host);
	hostDisp.p->AddRef();
	CComPtr<IHTMLElement> hostElement;
	if((result=hostDisp.QueryInterface(&hostElement))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("The \"host\" DHTML object is invalid")),result);
	CComPtr<IDispatch> documentDisp;
	if((result=hostElement->get_document(&documentDisp))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Document DHTML object from \"host\" object obtaining failed")),result);
	CComPtr<IHTMLDocument3> document3;
	if((result=documentDisp.QueryInterface(&document3))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IHTMLDocument3 interface from \"host\" object obtaining failed")),result);
	BSTR uniqueID;
	if((result=document3->get_uniqueID(&uniqueID))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Unique ID for new DHTML object obtaining failed")),result);
	CComBSTR uniqueIDRemover(uniqueID);
	//uniqueIDRemover.Attach(uniqueID);
	if(AVAILABLE_SERVICES.find(svcType)==AVAILABLE_SERVICES.end())
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Service with type=0x%x inaccessible"),svcType));
	USES_CONVERSION;
	//tstring sss=OLE2T(uniqueID);
	//tstring ssss=AVAILABLE_SERVICES[svcType].m_GUID;
	tstring objectStr(Format(_T("<OBJECT ID=\"%s\" CLASSID=\"CLSID:%s\" width=\"99%%\" height=\"100%%\"></OBJECT>"),
	                        OLE2T(uniqueID),
	                        AVAILABLE_SERVICES[svcType].m_GUID.c_str()));
	if((result=hostElement->put_innerHTML(CComBSTR(objectStr.c_str())))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Object tag inserting failed, string=\"%s\""),objectStr.c_str()),result);
	CComPtr<IHTMLElement> objectElement;
	if((result=document3->getElementById(uniqueIDRemover,&objectElement))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("New object not found. It may be because Service is not created")),result);
	CComPtr<IHTMLObjectElement> object;
	if((result=objectElement.QueryInterface(&object))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IHTMLObjectElement obtaining failed from new Object")),result);
	CComPtr<IDispatch> serviceDisp;
	if((result=object->get_object(&serviceDisp))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("ActiveX object pointer obtaining failed. It may be because Service is not created")),result);
	CComPtr<IBrokerClient> service;
	if((result=serviceDisp.QueryInterface(&service))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClient interface obtaining failed")),result);

	if(S_OK!=(result=m_service.Attach(service)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service object GIT registeration failed")),result);
	if(S_OK!=(result=m_hostElement.Attach(hostElement)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Host object (HTML element) GIT registeration failed")),result);

	CComObject<C_IBrokerClientEvents>::CreateInstance(&m_events);
	m_events.p->AddRef();
	m_events->m_owner=this;
	//m_events->DispEventAdvise(m_service);

	//IUnknown* eventsUnkn;
	//if((result=m_events->QueryInterface(&eventsUnkn))!=S_OK)
	//	throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service's IUnknown interface obtaining failed")),result);
	//
	//m_params.m_svcType=svcType;

	////m_service->Init(eventsUnkn);// AddRef in previous QueryInterface() call
	//if((result=service->Init(reinterpret_cast<ULONG>(eventsUnkn))!=S_OK))// AddRef in previous QueryInterface() call
	//	throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service COM object Init() call failed")),result);
	
	CComPtr<IUnknown> eventsUnkn;
	if((result=m_events->QueryInterface(&eventsUnkn))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service's IUnknown interface obtaining failed")),result);
	m_params.m_svcType=svcType;
	if((result=service->Init(eventsUnkn)!=S_OK))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service COM object Init() call failed")),result);

CATCH_THROW()
}

void CService::CreateToolService(EBrokerServicesTypes svcType)
{
TRY_CATCH
	HRESULT result;
	if(AVAILABLE_SERVICES.find(svcType)==AVAILABLE_SERVICES.end())
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Service with type=0x%x inaccessible"),svcType));
	USES_CONVERSION;
	
	CLSID clsid;
	if((result=CLSIDFromString(T2OLE((_T("{")+AVAILABLE_SERVICES[svcType].m_GUID+_T("}")).c_str()),&clsid))!=NOERROR)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("CLSID obtain failed for service with type=0x%x"),svcType),result);
	
	CComPtr<IBrokerClient> service;
	if((result=service.CoCreateInstance(clsid,NULL,CLSCTX_INPROC_SERVER))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Service with type=0x%x creation failed"),svcType),result);
	//if((result=m_serviceUnkn.QueryInterface(&m_service))!=S_OK)
	//	throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClient interface obtaining failed")),result);

	//CComPtr<IDispatch> serviceDisp;
	//if(S_OK!=(result=service.QueryInterface(&serviceDisp)))
	//	throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service IDispatch obtaining failed")),result);

	if(S_OK!=(result=m_service.Attach(service)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service object GIT registeration failed")),result);

	{
		CComPtr<IBrokerClient> service;
		HRESULT result;
		if(S_OK!=(result=m_service.CopyTo(&service)))
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClient obtaining failed in service")),result);
	}



	//{
	//	ULONG addRef,release;
	//	addRef=service.p->AddRef();
	//	release=service.p->Release();
	//	Log.Add(_MESSAGE_, _T("CoCreateInstance() addRef=0x%x release=0x%x"),addRef,release);
	//}

	CComObject<C_IBrokerClientEvents>::CreateInstance(&m_events);
	m_events.p->AddRef();
	m_events->m_owner=this;
	////m_events->DispEventAdvise(m_service);
	//IUnknown* eventsUnkn;
	//if((result=m_events->QueryInterface(&eventsUnkn))!=S_OK)
	//	throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service's IUnknown interface obtaining failed")),result);

	//m_params.m_svcType=svcType;
	////Log.Add(_MESSAGE_,_T("CService::CreateToolService() before m_service->Init(0x%08x) m_events.p=0x%08x"), eventsUnkn, m_events.p);
	////m_service->Init(eventsUnkn);// AddRef in previous QueryInterface() call

	//if((result=service->Init(reinterpret_cast<ULONG>(eventsUnkn))!=S_OK))// AddRef in previous QueryInterface() call
	//	throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service COM object Init() call failed")),result);


	CComPtr<IUnknown> eventsUnkn;
	if((result=m_events->QueryInterface(&eventsUnkn))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service's IUnknown interface obtaining failed")),result);
	m_params.m_svcType=svcType;
	if((result=service->Init(eventsUnkn)!=S_OK))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service COM object Init() call failed")),result);


CATCH_THROW()
}

void CService::DestroyToolService()
{
TRY_CATCH

	m_service.Revoke();

	//if(m_service.p)
	//{
	//	ULONG before=m_service.p->AddRef();
	//	ULONG after=m_service.p->Release();
	//	Log.Add(_MESSAGE_,_T("CService::DestroyToolService()::m_service.p->AddRef()=%d m_service.p->Release()=%d"),before,after);
	//}

	if(m_events.p)
		m_events->m_owner=NULL;
	m_events.Release();

	if(0!=m_hostElement.GetCookie())
	{
		HRESULT result;
		CComPtr<IHTMLElement> hostElement;
		if(S_OK!=(result=m_hostElement.CopyTo(&hostElement)))
			MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IHTMLElement obtaining failed in service host DHTML element object")),result))
		else
			hostElement->put_innerHTML(CComBSTR(L" "));
	}

	m_hostElement.Revoke();
CATCH_THROW()
}

EBrokerServicesTypes CService::GetSvcType()
{
TRY_CATCH
	return m_params.m_svcType;
CATCH_THROW()
}


void CService::HandleRequest(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params)
{
TRY_CATCH
	if(!m_service.m_dwCookie)
		throw MCException("Service COM object has not created");
	HRESULT result;
	CComPtr<IBrokerClient> service;
	if(S_OK!=(result=m_service.CopyTo(&service)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClient obtaining failed in service")),result);
	if((result=service->HandleRequest(CComBSTR(dstUserId.c_str()),dstSvcId,CComBSTR(srcUserId.c_str()),srcSvcId,rId,rType,param,CComBSTR(params.c_str())))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service COM object HandleRequest() call failed")),result);
CATCH_THROW()
}

void CService::RequestSent(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params)
{
TRY_CATCH
	if(!m_owner)
		throw MCException("Owner don't set");
	m_owner->RequestSent(m_params.m_svcId,dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
CATCH_THROW()
}

void CService::SetSubStream(const tstring& dstUserId, unsigned long dstSvcId, unsigned long streamId, boost::shared_ptr<CAbstractStream> stream)
{
TRY_CATCH
	if(!m_service.m_dwCookie)
		throw MCException("Service COM object has not created");
	HRESULT result;
	CComPtr<IBrokerClient> service;
	if(S_OK!=(result=m_service.CopyTo(&service)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClient obtaining failed in service")),result);
	if((result=service->SetSubStream(CComBSTR(dstUserId.c_str()),dstSvcId,streamId,reinterpret_cast<ULONG>(&stream)))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service COM object SetSubStream() call failed")),result);
CATCH_THROW()
}

void CService::GetSubStream(const tstring& dstUserId, unsigned long dstSvcId, unsigned long streamId, unsigned long priorityLevel)
{
TRY_CATCH
	if(!m_owner)
		throw MCException("Owner don't set");
	Log.Add(_MESSAGE_,_T("CService::OnGetSubStream(%s,0x%x,0x%x,%d)"),dstUserId.c_str(),dstSvcId,streamId,priorityLevel);
	m_owner->GetSubStream(m_params.m_svcId,dstUserId,dstSvcId,streamId,priorityLevel);
CATCH_THROW()
}

