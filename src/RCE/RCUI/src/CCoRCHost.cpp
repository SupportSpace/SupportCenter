/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCoRCHost.h
///
///  CCoRCHost object implementation (COM object wrapper of RCHost)
///
///  @author Kirill Solovyov @date 05.11.2007
///
////////////////////////////////////////////////////////////////////////

// CCoRCHost.cpp : Implementation of CCoRCHost

#include "stdafx.h"
#include "CCoRCHost.h"
#include <RCEngine/AXstuff/AXstuff.h>
#include <boost/bind.hpp>
#include <list>
#include <AidLib/CCritSection/CCritSectionWithMessageLoop.h>
// CCoRCHost

// Some important constants;
const int MAX_CLIENTS = 128;

#define WINDOW_KEY_TEXT _T("Session with")

inline HWND GetRootParent(HWND window)
{
TRY_CATCH
	HWND root = GetAncestor(window, GA_ROOT);
	return NULL!=root?root:window;
CATCH_THROW()
}

inline BOOL CALLBACK EnumWindows4Thread(HWND hWnd, LPARAM lParam)
{
TRY_CATCH

	std::set<DWORD> *newThreadList = reinterpret_cast<std::set<DWORD>*>(lParam);

	/// Checking match
	CWindow window(hWnd);
	boost::scoped_ptr<TCHAR> caption;
	unsigned int size = window.GetWindowTextLength()+1;
	caption.reset(new TCHAR[size]);
	window.GetWindowText(caption.get(), size);
	if (tstring::npos != tstring(caption.get()).find(WINDOW_KEY_TEXT))
	{
		DWORD threadId = window.GetWindowThreadID();
		Log.Add(_MESSAGE_,_T("Adding window (%X) caption(%s) thread(%X) to protected"),hWnd, caption.get(), threadId);
		newThreadList->insert(threadId);
	}

	/// Continue walking through windows
	EnumChildWindows(hWnd, EnumWindows4Thread, lParam);
	return TRUE;

CATCH_THROW()
}

void CRCHostImpl::FilterProtectedThreads(std::set<DWORD> &protectedThreads)
{
TRY_CATCH
	std::set<DWORD> newThreadList;
	for(std::set<DWORD>::iterator thread = protectedThreads.begin();
		protectedThreads.end() != thread;
		++thread)
	{
		Log.Add(_MESSAGE_,_T("Enumerating windows for thread (%X)"),*thread);
		EnumThreadWindows(*thread, EnumWindows4Thread, reinterpret_cast<LPARAM>(&newThreadList));
	}
	protectedThreads = newThreadList;
CATCH_THROW()
}

boost::signals::connection CRCHostImpl::SubscribeEventsListener(const eventListener listener)
{
TRY_CATCH
	CCritSection cs(&m_signalCS);
	return m_signal.connect(listener);
CATCH_THROW()
}

void CRCHostImpl::UnsubscribeEventListener(boost::signals::connection &connection)
{
TRY_CATCH
	//CCritSection cs(&m_signalCS);
	CCritSectionWithMessageLoop	cs(&m_signalCS);
	if (connection.connected())
		connection.disconnect();
CATCH_THROW()
}

void CRCHostImpl::NotifySessionStarted(const int clientId)
{
TRY_CATCH
	CCritSection cs(&m_signalCS);
	m_signal(HE_STARTED, clientId, 0);
CATCH_LOG()
}

void CRCHostImpl::NotifySessionStopped(const int clientId, ESessionStopReason ReasonCode)
{
TRY_CATCH
	CCritSection cs(&m_signalCS);
	m_signal(HE_STOPPED, clientId, 0);
CATCH_LOG()
}

CCoRCHost::CCoRCHost():
	CInstanceTracker(_T("CCoRCHost")),
	m_clientId(-1),
	m_svcStreamConnectedEvent(false/*ManualReset*/, false /*InitialState*/)
{
TRY_CATCH
CATCH_LOG()
}

CCoRCHost::~CCoRCHost(void)
{
TRY_CATCH
	RCHOST_INSTANCE.UnsubscribeEventListener(m_RCHostEnvetConnection);
CATCH_LOG()
}

void CCoRCHost::OnHostEvent(CRCHostImpl::EHostEvent eventType, const int param1, const int param2)
{
TRY_CATCH
	
	HRESULT result;
	if(!m_brokerEvents.m_dwCookie)
		throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");

	CComPtr<_IBrokerClientEvents> brokerEvents;
	if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);

	switch(eventType)
	{
		case CRCHostImpl::HE_STARTED:
			if(NULL != brokerEvents.p)
				//TODO error handling
				brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_SRV_STATE_CHANGED,ESM_SERVICE_STARTED/*on*/,CComBSTR(_T("Desktop Sharing"))); //TODO: change service name
			break;
		case CRCHostImpl::HE_STOPPED:
			if (m_clientId == param1)
				RCHOST_INSTANCE.UnsubscribeEventListener(m_RCHostEnvetConnection);
			if(NULL != brokerEvents.p)
				//TODO error handling
				brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_SRV_STATE_CHANGED,ESM_SERVICE_STOPPED/*off*/,CComBSTR(_T("Desktop Sharing"))); //TODO: change service name
			break;
		default:
			Log.Add(_WARNING_,_T("Unknown event type %d"), eventType);
	}
CATCH_LOG()
}

HRESULT CCoRCHost::FinalConstruct()
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CCoRCHost::FinalConstruct m_dwRef=0x%x"),m_dwRef);
CATCH_LOG_COMERROR()
}

void CCoRCHost::FinalRelease()
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CCoRCHost::FinalRelease+"));
	// Stopping client on internal host object, if such exists
	if(-1!=m_clientId)
	{
	TRY_CATCH
		RCHOST_INSTANCE.StopClient(m_clientId);
	CATCH_LOG()
	}
	// Reseting stream
	//m_svcStream.reset();
	// Waiting for internal thread termination
	if(_RUNNING == State || _PAUSED == State)
	{
		CThread::Terminate();
		m_svcStreamConnectedEvent.Set();
		// Reseting stream
		m_svcStream.reset();
		//WaitForSingleObject(hTerminatedEvent.get(),INFINITE);
		AtlWaitWithMessageLoop(hTerminatedEvent.get());
	}
	else
		// Reseting stream
		m_svcStream.reset();

	m_brokerEvents.Revoke();
	Log.Add(_MESSAGE_,_T("CCoRCHost::FinalRelease-"));
CATCH_LOG()
}

STDMETHODIMP CCoRCHost::Init(IUnknown *events)
{
TRY_CATCH

	HRESULT result;
	Log.Add(_MESSAGE_,_T("CCoRCHost::Init(0x%08x) m_dwRef=0x%x"),events,m_dwRef);
	// Attaching to broker events (which is actually not events, but broker pointer, having the same sence)
	//CComPtr<IUnknown> eventsUnkn;
	//eventsUnkn.Attach(reinterpret_cast<IUnknown*>(events));

	CComPtr<IUnknown> eventsUnkn(events);
	CComPtr<_IBrokerClientEvents>	brokerEvents;
	if(S_OK != (result=eventsUnkn.QueryInterface(&brokerEvents)))
	{
		SetLastError(result);
		throw MCException_Win(_T("QueryInterface(_IBrokerClientEvents) failed "));
	}
	
	if(S_OK!=(result=m_brokerEvents.Attach(brokerEvents)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Broker object GIT registeration failed")),result);

	// Starting internal thread (proper start of connection)
	CThread::Start();

CATCH_LOG_COMERROR()
}

STDMETHODIMP CCoRCHost::HandleRequest(BSTR dstUserId,ULONG dstSvcId,BSTR srcUserId,ULONG srcSvcId,ULONG rId,ULONG rType,ULONG param,BSTR params)
{
TRY_CATCH

	USES_CONVERSION;
	switch(rType)
	{
		//case BRT_SERVICE|BRT_RESPONSE:
		//	{
		//		if(BRR_DECLINED==param)
		//		{
		//			m_ui.SetUIStatus(UIS_PERMISSION_DENIED,_T("Customer declined."));
		//		}
		//		else if(BRR_APPROVED==param)
		//		{
		//			m_ui.SetUIStatus(UIS_PERMISSION_RECIEVED,_T("Customer approved."));
		//			if(!m_svcStream.get())
		//				CThread::Start();
		//			else
		//				InitiateRCConnectAndStart();
		//		}
		//		else
		//		{
		//			m_ui.SetUIStatus(UIS_PERMISSION_DENIED,Format(_T("Desktop sharing response is unknown (code=0x%x) %s"),param,OLE2T(params)));
		//			MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Responce type is unknown. HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),OLE2T(dstUserId),dstSvcId,OLE2T(srcUserId),srcSvcId,rId,rType,param,OLE2T(params)));
		//		}
		//	}
		//	break;
		case BRT_PING:
			{
				Log.Add(_MESSAGE_,_T("CCoRCHost::HandleRequest(BRT_PING)"));
				HRESULT result;
				if(!m_brokerEvents.m_dwCookie)
					throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
				CComPtr<_IBrokerClientEvents> brokerEvents;
				if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
				if(!brokerEvents.p)
					throw MCException("_IBrokerClientEvents has not marshaled");
				//TODO error hanling
				brokerEvents->RequestSent(srcUserId,srcSvcId,srcUserId,dstSvcId,rId,rType|BRT_RESPONSE,param,params);//response on ping
			}
			break;
		case BRT_PING|BRT_RESPONSE:
			{
				Log.Add(_MESSAGE_,_T("CCoRCHost::HandleRequest(BRT_PING|BRT_RESPONSE)"));
			}
			break;
		case BRT_STOP_SERVICE:
			{
				for(int i=0; i<MAX_CLIENTS; ++i)
					try
					{
						RCHOST_INSTANCE.StopClient(i);							
					}
					catch(CExceptionBase&)
					{
					}
			}
			break;
		default:
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Request type is unknown. HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),OLE2T(dstUserId),dstSvcId,OLE2T(srcUserId),srcSvcId,rId,rType,param,OLE2T(params));
	}
CATCH_LOG_COMERROR()
}

void CCoRCHost::HandleCoViewerCommand(ULONG buf)
{
TRY_CATCH
	switch(buf)
	{
		case RCC_START_REQ:
			{
				//TODO message box request here
				Log.Add(_MESSAGE_,_T("RC START REQUEST"));
				if(!m_svcStream.get())
					throw MCException("Service stream is NULL");
				ULONG buf=RCC_START_REQ_APPROVE;
				m_svcStream->Send(reinterpret_cast<char*>(&buf),sizeof(buf));
			}
			break;
		case RCC_START:
			InitiateRCConnectAndStart();
			break;
		default:
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Unknown service message type=0x%x"),buf));
	}
CATCH_THROW()
}

STDMETHODIMP CCoRCHost::SetSubStream(BSTR dstUserId, ULONG dstSvcId, ULONG streamId, ULONG pointer_shared_ptr_stream)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CCoRCHost::SetSubStream() pointer_shared_ptr_stream=0x%08x"),pointer_shared_ptr_stream);

	// We have following possibilities, why we're here:
	// 1. Stream for VNC received
	// 2. Stream for CoViewer/CoHost commands received
	
	USES_CONVERSION;
	switch(streamId)
	{
		case RCSSID_SERVICE:
			{
				Log.Add(_MESSAGE_,_T("CCoRCHost::SetSubStream(RCSSID_SERVICE)"));
				m_svcStream=*reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream);
				m_svcStreamConnectedEvent.Set();
			}
			break;
		case RCSSID_RC:
			{
				Log.Add(_MESSAGE_,_T("CCoRCHost::SetSubStream(RCSSID_RC)"));
				boost::shared_ptr<CAbstractStream>* pstream = new boost::shared_ptr<CAbstractStream>(*reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream));
				/// Subscribing on events
				m_RCHostEnvetConnection = RCHOST_INSTANCE.SubscribeEventsListener(boost::bind(&CCoRCHost::OnHostEvent, this, _1,_2,_3));
				RCHOST_INSTANCE.GetActivityMonitor().SetActivityChangedHandler(boost::bind(&CCoRCHost::OnHostActivity, this, _1));
				m_clientId=RCHOST_INSTANCE.StartClient(*reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream),0);
	
				/// Setting protected window
				TRY_CATCH
					Log.Add(_MESSAGE_,_T("Protecting process (%d) from input injection"), CProcessSingleton<COriginalIEPIDWrapper>::instance().GetPid());
					RCHOST_INSTANCE.SetProtectedProcess(CProcessSingleton<COriginalIEPIDWrapper>::instance().GetPid());
				CATCH_LOG()
				break;
			}
		default:
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Sub stream id is unknown. SetSbuStream(%s,0x%x,0x%x,0x%08x)"),OLE2T(dstUserId),dstSvcId,streamId,pointer_shared_ptr_stream);
	}
CATCH_LOG_COMERROR()
}

void CCoRCHost::InitiateRCConnectAndStart(void)
{
TRY_CATCH
	if(!m_brokerEvents.m_dwCookie)
		throw MCException("_IBrokerClientEvents is NULL. Was IBrokerClient::Init() called?");

	HRESULT result;
	CComPtr<_IBrokerClientEvents> brokerEvents;
	if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
	
	///TODO error handling
	brokerEvents->GetSubStream(CComBSTR(BUSERIDPDV_AUTOSET), BSVCIDPDV_AUTOSET, RCSSID_RC, RCSSP_RC);
CATCH_LOG()
}

void CCoRCHost::Execute(void *Params)
{
TRY_CATCH
	HRESULT result;
	CoInitializeEx(NULL,COINIT_MULTITHREADED);
	if(!m_brokerEvents.m_dwCookie)
		throw MCException("_IBrokerClientEvents is NULL. Was IBrokerClient::Init() called?");


	while(!Terminated())
	{
		Log.Add(_MESSAGE_,_T("GET SERVICE SUBSTREAM m_dwRef=0x%x"),m_dwRef);
		CComPtr<_IBrokerClientEvents> brokerEvents;
		if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);

		if(S_OK != (result=brokerEvents->GetSubStream(CComBSTR(BUSERIDPDV_AUTOSET), BSVCIDPDV_AUTOSET, RCSSID_SERVICE, RCSSP_RC)))
		{
			SetLastError(result);
			MLog_Exception(MCException_Win(_T("Service substream obtaining failed")))
			continue;
		}
		brokerEvents.Release();
		//TODO think about INFINITE
		if(WAIT_OBJECT_0!=WaitForSingleObject(m_svcStreamConnectedEvent,INFINITE))
			throw MCException_Win("WaitForSingleObject failed");
		Log.Add(_MESSAGE_,_T("SERVICE SUBSTREAM IS CONNECTED"));
		if(Terminated())
			return;
		if(!m_svcStream.get())
			throw MCStreamException("Service stream does not exist");
		try
		{
			while(!Terminated())
			{
				ULONG buf=0;
				m_svcStream->Receive(reinterpret_cast<char*>(&buf),sizeof(buf));
				try
				{
					HandleCoViewerCommand(buf);
				}
				catch(CExceptionBase& e)
				{
					MLog_Exception(e);
				}
			}
		}
		catch(CStreamException& e)
		{
			MLog_Exception(e);
		}
	}
CATCH_LOG()
}

void CCoRCHost::OnHostActivity(CActivityMonitor::EActivityType activityType)
{
TRY_CATCH

	if (0 == RCHOST_INSTANCE.GetClientsCount())
		return;

	tstring activity(_T("Unknown activity"));
	switch(activityType)
	{
		case CActivityMonitor::EAT_IDLE:
			activity = _T("");
			break;
		case CActivityMonitor::EAT_KEYBOARD:
			activity = _T("Expert is now typing");
			break;
		case CActivityMonitor::EAT_MOUSE:
			activity = _T("Expert is now moving mouse");
			break;
	};

	HRESULT result;
	if(!m_brokerEvents.m_dwCookie)
		throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");

	CComPtr<_IBrokerClientEvents> brokerEvents;
	if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);

	tstring params = activity.empty()?_T("Desktop Sharing"):Format(_T("Desktop Sharing;;%s"),activity.c_str());

	if(NULL != brokerEvents.p)
		//TODO error handling
		brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_SRV_STATE_CHANGED,ESM_SERVICE_ACTIVITY_CHANGED,CComBSTR(params.c_str()));

CATCH_LOG()
}
