// CoFAHost.cpp : Implementation of CCoFAHost

#include "stdafx.h"
#include "CCoFAHost.h"
#include <NWL/Streaming/CNetworkLayer.h>
#include <NWL/Streaming/CIMStub.h>
#include <AidLib/CSingleton/CSingleton.h>
#include <RCEngine/AXstuff/AXstuff.h>
#include <AidLib/COM/COMException.h>
#include <AidLib/CThread/CThreadLS.h>


CCoFAHost::CCoFAHost()
	:	CInstanceTracker(_T("CCoFAHost")),
		CThread(0, true)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CCoFAHost::CCoFAHost()"));

	REPORT_MODULE(FTUI_NAME);
	m_bWindowOnly=TRUE;//for fire event in other thread
CATCH_LOG()
}

CCoFAHost::~CCoFAHost()
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CCoFAHost::~CCoFAHost()"));

	PostThreadMessage(GetTid(), WM_QUIT, 0, 0);
	m_brokerEvents.Revoke();

//	if( m_server.get() )
//		m_server->Stop();

CATCH_LOG()
}


void CCoFAHost::LaunchHost()
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CCoFAHost::LaunchHost()"));

	m_server = boost::shared_ptr<CFileAccessServer>( new CFileAccessServer( m_stream.get() ) );
	m_server->SetActivityChangedHandler(boost::bind(&CCoFAHost::OnActivityNotification, this, _1));
	for( int i=0;i<8;++i )
		m_server->SetAuthorization( static_cast<TransferOpearation>(i) , /*(m_permission[ i ])?true:false*/ true );
	m_server->Start();
CATCH_THROW()
}

HRESULT CCoFAHost::IOleObject_Close(DWORD dwSaveOption)
{
	Log.Add(_MESSAGE_,_T("CCoFAHost::IOleObject_Close()"));

	if(m_server.get())
	{
		m_server->Stop();
		m_stream.reset();
	}
	return 0;
}

STDMETHODIMP CCoFAHost::Init(IUnknown *events)
{
TRY_CATCH_COM

	Log.Add(_MESSAGE_,_T("CCoFAHost::Init()"));

	HRESULT result;
	Log.Add(_MESSAGE_,_T("CCoFAHost::Init(0x%08x) m_dwRef=0x%x"),events,m_dwRef);
	// Attaching to broker events (which is actually not events, but broker pointer, having the same sence)
	//CComPtr<IUnknown> eventsUnkn;
	//eventsUnkn.Attach(reinterpret_cast<IUnknown*>(events));
	//if(S_OK != (result=eventsUnkn.QueryInterface(&m_brokerEvents)))
	//{
	//	SetLastError(result);
	//	MCException_Win(_T("QueryInterface(_IBrokerClientEvents) failed "));
	//}

	CComPtr<IUnknown> eventsUnkn(events);
	CComPtr<_IBrokerClientEvents> brokerEvents;
	if(S_OK != (result=eventsUnkn.QueryInterface(&brokerEvents)))
	{
		SetLastError(result);
		throw MCException_Win(_T("QueryInterface(_IBrokerClientEvents) failed "));
	}
	
	if(S_OK!=(result=m_brokerEvents.Attach(brokerEvents)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Broker object GIT registeration failed")),result);

	/// Requesting stream
	if((result=brokerEvents->GetSubStream(CComBSTR(BUSERIDPDV_AUTOSET), BSVCIDPDV_AUTOSET, FASSID_MAIN, FASSP_SERVICE))!=S_OK)
	{
		SetLastError(result);
		throw MCException_Win("Service substream obtaining failed");
	}

	/// Sending request to indicate - service is on
	if(NULL != brokerEvents.p)
		brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_SRV_STATE_CHANGED,ESM_SERVICE_STARTED/*on*/,CComBSTR(_T("File manager"))); //TODO: treat name carefully

	/// Starting notifications thread
	Start();

CATCH_LOG_COM
}

STDMETHODIMP CCoFAHost::HandleRequest(BSTR dstUserId,ULONG dstSvcId,BSTR srcUserId,ULONG srcSvcId,ULONG rId,ULONG rType,ULONG param,BSTR params)
{
TRY_CATCH_COM

	Log.Add(_MESSAGE_,_T("CCoFAHost::HandleRequest()"));

	USES_CONVERSION;
	switch(rType)
	{
		case BRT_PING:
			{
				Log.Add(_MESSAGE_,_T("CCoFAHost::HandleRequest(BRT_PING)"));
				if(!m_brokerEvents.m_dwCookie)
					throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
				HRESULT result;
				CComPtr<_IBrokerClientEvents> brokerEvents;
				if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
				if(!brokerEvents.p)
					throw MCException("_IBrokerClientEvents has not marshaled");
				brokerEvents->RequestSent(srcUserId,srcSvcId,srcUserId,dstSvcId,rId,rType|BRT_RESPONSE,param,params);//response on ping
			}
			break;
		case BRT_PING|BRT_RESPONSE:
			{
				Log.Add(_MESSAGE_,_T("CCoFAHost::HandleRequest(BRT_PING|BRT_RESPONSE)"));
			}
			break;
		case BRT_GET_SERVICE_INFO|BRT_RESPONSE:
			Log.Add(_MESSAGE_,_T("CCoFAHost::HandleRequest(BRT_GET_SERVICE_INFO|BRT_RESPONSE)"));

			if (NULL != m_server.get())
			{
				tstring sid = _T("unknown");
				tstring expertName = _T("unknown");
				tstring customerName = _T("unknown");
				USES_CONVERSION;
				std::vector<tstring> info = tokenize(_OLE2T(params), _T(";"));
				//params="[userId];;[UserName];;[remoteUserId];;[remoteUserName];;[sId]"
				if (info.size() != 5)
					Log.Add(_ERROR_,_T("Unknown BRT_GET_SERVICE_INFO response string format %s"),W2T(params));
				else
				{
					sid = info[4];
					expertName = Format(_T("%s[%s]"),info[3].c_str(),info[2].c_str());
					customerName = Format(_T("%s[%s]"),info[1].c_str(),info[0].c_str());
				}
				m_server->InitTransferLogging(sid, customerName, expertName);
			}
			break;
		case BRT_NWL_DISCONNECTED:
		case BRT_SERVICE_DESTROYED:
			Log.Add(_MESSAGE_,_T("CCoFAHost::HandleRequest(BRT_SERVICE_DESTROYED)"));
			TRY_CATCH
			HRESULT result;
			if(!m_brokerEvents.m_dwCookie)
				throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");

			CComPtr<_IBrokerClientEvents> brokerEvents;
			if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
				throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);

			/// Sending request to indicate - service is off
			if(NULL != brokerEvents.p)
				brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_SRV_STATE_CHANGED,ESM_SERVICE_STOPPED/*off*/,CComBSTR(_T("File manager"))); //TODO: treat name carefully
			CATCH_LOG()
			break;
		case BRT_STOP_SERVICE:
			Log.Add(_MESSAGE_,_T("CCoFAHost::HandleRequest(BRT_STOP_SERVICE|BRT_NWL_DISCONNECTED)"));
			{
			TRY_CATCH

				//m_server->Stop();
//				m_server->Stop();
//				m_stream->CancelReceiveOperation();
//				m_stream.reset();
				PostThreadMessage(GetTid(), WM_QUIT, 0, 0);
				Stop(false,3000);
				

				HRESULT result;
				if(!m_brokerEvents.m_dwCookie)
					throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");

				CComPtr<_IBrokerClientEvents> brokerEvents;
				if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);

				/// Resending request to client side
				if(NULL != brokerEvents.p)
					brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_STOP_SERVICE,param,params);

			CATCH_LOG()
			}
			break;
		default:
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Request type is unknown. HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),OLE2T(dstUserId),dstSvcId,OLE2T(srcUserId),srcSvcId,rId,rType,param,OLE2T(params));
	}
CATCH_LOG_COM
}

STDMETHODIMP CCoFAHost::SetSubStream(BSTR dstUserId, ULONG dstSvcId, ULONG streamId, ULONG pointer_shared_ptr_stream)
{
TRY_CATCH_COM
	Log.Add(_MESSAGE_,_T("CCoFAHost::SetSubStream() pointer_shared_ptr_stream=0x%08x"),pointer_shared_ptr_stream);

	USES_CONVERSION;
	switch(streamId)
	{
		case FASSID_MAIN:
			{
				Log.Add(_MESSAGE_,_T("CCoFAHost::SetSubStream(FASSID_MAIN)"));
				//boost::shared_ptr<CAbstractStream>* pstream = new boost::shared_ptr<CAbstractStream>(*reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream));
				m_stream = *reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream);
				/// Launching internal FAHost object
				LaunchHost();
				HRESULT result;
				if(!m_brokerEvents.m_dwCookie)
					throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
				CComPtr<_IBrokerClientEvents> brokerEvents;
				if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
				if(!brokerEvents.p)
					throw MCException("_IBrokerClientEvents has not marshaled");
				/// Requesting information for logger init
				brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_BROKER,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0,BRT_GET_SERVICE_INFO,0,CComBSTR(_T("")));
				break;
			}
		default:
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Sub stream id is unknown. SetSbuStream(%s,0x%x,0x%x,0x%08x)"),OLE2T(dstUserId),dstSvcId,streamId,pointer_shared_ptr_stream);
	}
CATCH_LOG_COM
}

void CCoFAHost::OnActivityNotification(const tstring description)
{
TRY_CATCH
	PTCHAR str = _tcsdup(description.c_str());
	if (FALSE == PostThreadMessage(GetTid(),WM_USER, reinterpret_cast<WPARAM>(str), 0))
		free(str);
CATCH_LOG()
}

void CCoFAHost::Execute(void*)
{
	Log.Add(_MESSAGE_,_T("CCoFAHost::Execute()"));

	CoInitializeEx(NULL,COINIT_MULTITHREADED);
	SET_THREAD_LS;

TRY_CATCH

	MSG msg;
	HRESULT result;
	if(!m_brokerEvents.m_dwCookie)
		throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");

	//CComPtr<_IBrokerClientEvents> brokerEvents;
	//if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
	//	throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);

	while(!Terminated() && GetMessage(&msg, NULL, 0, 0))
	{
		switch(msg.message)
		{
			case WM_USER:
				{
					Log.Add(_MESSAGE_,_T("CCoFAHost::Execute(WM_USER)"));

					CComPtr<_IBrokerClientEvents> brokerEvents;
					if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
						throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);

					boost::shared_ptr<TCHAR> str;
					str.reset(reinterpret_cast<PTCHAR>(msg.wParam), free);
					tstring activity(str.get());
					tstring params = activity.empty()?_T("File manager"):Format(_T("File manager;;%s"),activity.c_str());

					if(NULL != brokerEvents.p)
					{
						//TODO error handling
						brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_SRV_STATE_CHANGED,ESM_SERVICE_ACTIVITY_CHANGED,CComBSTR(params.c_str()));
					}
				}
				break;
			default:
				Log.Add(_WARNING_,_T("Unexpected message %d received"), msg.message);
		}
	}
CATCH_LOG()
	CoUninitialize();
}


