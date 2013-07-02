// C:\Users\Max\WORK\SupportSpace\Clients\Win32\RCE\FTUI\src\CCoFAViewer.cpp : Implementation of CCoFAViewer
#include "stdafx.h"
#include "CCoFAViewer.h"
#include <AidLib/CException/CException.h>
#include <RCEngine/AXstuff/AXstuff.h>
#include <NWL/Streaming/CNetworkLayer.h>
#include <AidLib/CSingleton/CSingleton.h>
#include <AidLib/CCrypto/CCrypto.h>
#include <NWL/TLS/CTLSSystem.h>
#include <AidLib/COM/COMException.h>


CTLSSystem TLSSystem;

HWND g_hwndDllDlg;
HHOOK g_hHook;

// Hack for processing dialog messages. Correct processing doesn't exist in main message loop.
LRESULT FAR PASCAL GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	LPMSG lpMsg = (LPMSG)lParam;

	if ( nCode >= 0 && PM_REMOVE == wParam )
	{
		if ( (lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST) )
		{
			HWND hwnd = lpMsg->hwnd;

			while ( hwnd && (hwnd != g_hwndDllDlg) )
				hwnd = GetParent(hwnd);

			if ( hwnd && IsDialogMessage(g_hwndDllDlg, lpMsg) )
			{
				lpMsg->message = WM_NULL;
				lpMsg->lParam  = 0;
				lpMsg->wParam  = 0;
			}
		}
	}
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

//Fint CCoFAViewer::m_wndClassOffset;

CCoFAViewer::CCoFAViewer()
	:	CInstanceTracker(_T("CCoFAViewer")),
		m_msgSessionStarted(::RegisterWindowMessage(_T("CCoFAViewer::m_msgSessionStarted"))),//generate unique message code
		m_msgSessionStopped(::RegisterWindowMessage(_T("CCoFAViewer::m_msgSessionStopped"))), //generate unique message code
		CRIdGen(100)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CCoFAViewer::CCoFAViewer()"));
	
	InitializeCriticalSection( &m_cs );
	REPORT_MODULE(CoFAViewerNAME);
	//m_wndClassOffset = reinterpret_cast<char*>(this) - reinterpret_cast<char*>(static_cast<CWindowImplBaseT<>* >(this)); 
	m_bWindowOnly = TRUE;
	
	//m_faClient.reset();	
	m_stream.reset();

CATCH_LOG()
}

CCoFAViewer::~CCoFAViewer()
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CCoFAViewer::~CCoFAViewer()"));

	m_brokerEvents.Revoke();

	TRACENAME
		
	//m_faClient.reset();	
	m_stream.reset();

	DeleteCriticalSection( &m_cs );

CATCH_LOG()
}

LRESULT CCoFAViewer::FireEventSessionStarted(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	
	Log.Add(_MESSAGE_,_T("CCoFAViewer::FireEventSessionStarted()"));

	TRACENAME
	
	InvokeEvent(1 /*NotifySessionStart*/); //	The same as call NotifySessionStart() but it's more safe
	m_commandManager->InitRemoteSide(m_stream);

CATCH_LOG()
	return 0;
}

LRESULT CCoFAViewer::FireEventSessionStopped(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CCoFAViewer::FireEventSessionStopped()"));

	TRACENAME

	InvokeEvent(2 /*NotifySessionStop*/, static_cast<int>(wParam)); // The same as call NotifySessionStop(wParam) but it's more safe

	m_commandManager->OnRemoteSideDisconnected(static_cast<ESessionStopReason>(wParam));
	m_stream.reset();

	tstring what;
	switch(static_cast<ESessionStopReason>(wParam))	//wParam ? lParam
    {
		case LOCAL_STOP:
			what = _T("LOCAL_STOP");
			break;
        case REMOTE_STOP:
			what = _T("REMOTE_STOP");
			break;
		case STREAM_ERROR:
			what = _T("STREAM_ERROR");
			break;
        case PROTOCOL_ERROR:
 			what = _T("PROTOCOL_ERROR");
			break;
        case CHANGE_DISPLAY_MODE:
			what = _T("CHANGE_DISPLAY_MODE");
			break;
        case CONNECTING_ERROR:
 			what = _T("CONNECTING_ERROR");
			break;
        case OPENFILE_ERROR:
			what = _T("OPENFILE_ERROR");
            break;

       default:
            throw MCException(Format("Unknown reason %d",wParam));
    }
	Log.Add(_MESSAGE_, "Session was stopped: %s", what.c_str());

CATCH_LOG()
	bHandled = TRUE;
	return 0;
}

LRESULT CCoFAViewer::OnDestroyDialog(UINT uMsg,WPARAM wParam,LPARAM lParam, BOOL&bHandled)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CCoFAViewer::OnDestroyDialog()"));

	if ( m_commandManager.get() )
		m_commandManager->DestroyManager();

	UnhookWindowsHookEx(g_hHook);
	g_hwndDllDlg	= NULL;
	g_hHook			= NULL;

CATCH_LOG()
	// Perform any dialog initialization
	return FALSE;
}

LRESULT CCoFAViewer::OnSizeDialog(UINT uMsg,WPARAM wParam,LPARAM lParam, BOOL&bHandled)
{
TRY_CATCH

CATCH_LOG()
	Log.Add(_MESSAGE_,_T("CCoFAViewer::OnSizeDialog"));

	// Perform any dialog initialization
	if (m_commandManager.get())
		m_commandManager->NotifyMainWindowPosChanged();

	return 0;
}

/// Requiret to handle dialog based com control
LRESULT CCoFAViewer::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&bHandled)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CCoFAViewer::OnInitDialog"));
	
	SetWindowLong(GWL_EXSTYLE,GetWindowLong(GWL_EXSTYLE)|WS_EX_CONTROLPARENT);
	
	/// Showing UI
	m_commandManager.reset(new CCommandManager);
	m_commandManager->InitManager(m_hWnd, m_commandManager);
	m_commandManager->SetRequestHandler(boost::bind(&CCoFAViewer::OnSendRequestBtn,this));
	
	// Set hook for correct dialog messages processing
	g_hwndDllDlg	= m_hWnd;
	g_hHook			= SetWindowsHookEx(WH_GETMESSAGE,GetMsgProc,NULL,GetCurrentThreadId());
		
CATCH_LOG()
	// Perform any dialog initialization

	return FALSE;
}

STDMETHODIMP CCoFAViewer::Init(IUnknown *events)
{
TRY_CATCH_COM

	Log.Add(_MESSAGE_,_T("BGN CCoFAViewer::Init(0x%08x)"),events);

	//CComPtr<IUnknown> eventsUnkn;
	//eventsUnkn.Attach(reinterpret_cast<IUnknown*>(events));
	//if((result=eventsUnkn.QueryInterface(&m_brokerEvents))!=S_OK)
	//	throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("_IBrokerClientEvents interface obtaining failed")),result);

	HRESULT result;
	CComPtr<IUnknown> eventsUnkn(events);
	CComPtr<_IBrokerClientEvents> brokerEvents;
	if(S_OK != (result=eventsUnkn.QueryInterface(&brokerEvents)))
	{
		SetLastError(result);
		throw MCException_Win(_T("QueryInterface(_IBrokerClientEvents) failed "));
	}
	
	if(S_OK!=(result=m_brokerEvents.Attach(brokerEvents)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Broker object GIT registeration failed")),result);

CATCH_LOG_COM
}

void CCoFAViewer::OnSendRequestBtn()
{
TRY_CATCH

	// Sending accept deny request
	//tstring& reqParams=Format(BRT_SERVICE_FATEXT, 0/*Permission?*/);
	tstring& reqParams=Format(BRT_SERVICE_FAFORMAT,BRT_SERVICE_FATEXT,BRT_SERVICE_APPROVE,BRT_SERVICE_DECLINE);
	if (NULL != m_commandManager.get())
	{
		m_commandManager->ShowStatusMessage(BRT_SERVICE_WAITING_APPROVE, EFMS_PERMISSION_REQUEST_SENT);
		m_commandManager->EnableRequestButton(FALSE);
	}

	if(!m_brokerEvents.m_dwCookie)
		throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
	HRESULT result;
	CComPtr<_IBrokerClientEvents> brokerEvents;
	if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
	if(!brokerEvents.p)
		throw MCException("_IBrokerClientEvents has not marshaled");

	brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_BROKER,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,RId(),BRT_SERVICE,BST_FAHOST,CComBSTR(reqParams.c_str()));
CATCH_LOG()
}

STDMETHODIMP CCoFAViewer::HandleRequest(BSTR dstUserId,ULONG dstSvcId,BSTR srcUserId,ULONG srcSvcId,ULONG rId,ULONG rType,ULONG param,BSTR params)
{
TRY_CATCH_COM

	Log.Add(_MESSAGE_,_T("CCoFAViewer::HandleRequest()"));

	USES_CONVERSION;
	switch(rType)
	{
		case BRT_PING:
			{
				Log.Add(_MESSAGE_,_T("CCoFAViewer::HandleRequest(BRT_PING)"));
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
				Log.Add(_MESSAGE_,_T("CCoFAViewer::HandleRequest(BRT_PING|BRT_RESPONSE)"));
			}
			break;
		case BRT_SERVICE|BRT_RESPONSE:
			{
				Log.Add(_MESSAGE_,_T("CCoFAViewer::HandleRequest(BRT_SERVICE|BRT_RESPONSE)"));

				if(BRR_BPFAILED==param)
				{
					if (NULL != m_commandManager.get())
					{
						m_commandManager->ShowStatusMessage(Format(BRT_SERVICE_BPFAILED,OLE2T(params)), EFMS_PERMISSION_DENIED);
						m_commandManager->EnableRequestButton();
					}
				}
				else if(BRR_ERROR==param)
				{
					if (NULL != m_commandManager.get())
					{
						m_commandManager->ShowStatusMessage(Format(_T("%s. Request handling failed"),BRT_SERVICE_DECLINED), EFMS_PERMISSION_DENIED);
						m_commandManager->EnableRequestButton();
					}
				}
				else if(BRR_DECLINED==param)
				{
					if (NULL != m_commandManager.get())
					{
						m_commandManager->ShowStatusMessage(BRT_SERVICE_DECLINED, EFMS_PERMISSION_DENIED);
						m_commandManager->EnableRequestButton();
					}
				}
				else if(BRR_APPROVED==param)
				{
					if (NULL != m_commandManager.get())
					{
						m_commandManager->ShowStatusMessage(BRT_SERVICE_APPROVED, EFMS_PERMISSION_RECEIVED);
						m_commandManager->EnableRequestButton();
					}
					if(!m_brokerEvents.m_dwCookie)
						throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
					HRESULT result;
					CComPtr<_IBrokerClientEvents> brokerEvents;
					if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
						throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
					if(!brokerEvents.p)
						throw MCException("_IBrokerClientEvents has not marshaled");

					if((result=brokerEvents->GetSubStream(CComBSTR(BUSERIDPDV_AUTOSET), BSVCIDPDV_AUTOSET, FASSID_MAIN, FASSP_SERVICE))!=S_OK)
						MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service substream obtaining failed")),result))
					/// Requesting information for logger init
					brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_BROKER,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,RId(),BRT_GET_SERVICE_INFO,0,CComBSTR(_T("")));
				}
				else if(BRR_BUSY==param)
				{
					if (NULL != m_commandManager.get())
					{
						//m_commandManager->ShowStatusMessage(BRT_SERVICE_BUSY);
						m_commandManager->ShowStatusMessage(_T("Another service is waiting for customer approval or installation. Please try again."), EFMS_PERMISSION_DENIED);
						m_commandManager->EnableRequestButton();
					}
				}
				else
				{
					// TODO: show progress: SCriptEngine response is unknown (code=0x%x) %s"),param,OLE2T(params)));
					if (NULL != m_commandManager.get())
					{
						m_commandManager->ShowStatusMessage(Format(_T("File Manager response is unknown (code=0x%x) %s"),param,OLE2T(params)));
						m_commandManager->EnableRequestButton();
					}
					MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Responce type is unknown. HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),OLE2T(dstUserId),dstSvcId,OLE2T(srcUserId),srcSvcId,rId,rType,param,OLE2T(params)));
				}
			}
			break;
		case BRT_INSTALLATION:
			Log.Add(_MESSAGE_,_T("CCoFAViewer::HandleRequest(BRT_INSTALLATION)"));

			if (NULL != m_commandManager.get())
				if(param)
					m_commandManager->ShowStatusMessage(Format(BRT_INSTALLATION_EXFORMAT,OLE2T(params),param), EFMS_INSTALLATION_PROGRESS);
				else
					m_commandManager->ShowStatusMessage(_OLE2T(params), EFMS_INSTALLATION_PROGRESS);
			break;
		case BRT_NWL_DISCONNECTED:
			{
				Log.Add(_MESSAGE_,_T("CCoFAViewer::HandleRequest(BRT_NWL_DISCONNECTED)"));

				PostMessage(m_msgSessionStopped, STREAM_ERROR, 0);
			}
			break;
		case BRT_GET_SERVICE_INFO|BRT_RESPONSE:
			Log.Add(_MESSAGE_,_T("CCoFAViewer::HandleRequest(BRT_GET_SERVICE_INFO|BRT_RESPONSE)"));

			if (NULL != m_commandManager.get())
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
					customerName = Format(_T("%s[%s]"),info[3].c_str(),info[2].c_str());
					expertName = Format(_T("%s[%s]"),info[1].c_str(),info[0].c_str());
				}
				m_commandManager->InitTransferLogging(sid, customerName, expertName);
			}
			break;
		case BRT_CONNECTION:
			Log.Add(_MESSAGE_,_T("CCoFAViewer::HandleRequest(BRT_CONNECTION)"));

			if (NULL != m_commandManager.get())
			{
				m_commandManager->ShowStatusMessage(_OLE2T(params), EFMS_PERMISSION_REQUEST_SENT);
				m_commandManager->EnableRequestButton(FALSE);
			}
			break;
		case BRT_STOP_SERVICE:
			{
				Log.Add(_MESSAGE_,_T("CCoFAViewer::HandleRequest(BRT_STOP_SERVICE)"));

				PostMessage(m_msgSessionStopped, REMOTE_STOP, 0);
				/// Reporting state to customer's JS
				if(!m_brokerEvents.m_dwCookie)
					throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
				HRESULT result;
				CComPtr<_IBrokerClientEvents> brokerEvents;
				if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
				if(!brokerEvents.p)
					throw MCException("_IBrokerClientEvents has not marshaled");
				brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,RId()/*rid*/,BRT_SRV_STATE_CHANGED,ESM_SERVICE_STOPPED/*off*/,CComBSTR(_T("File manager"))); //TODO: treat name carefully
			}
			break;
		default:
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Request type is unknown. HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),OLE2T(dstUserId),dstSvcId,OLE2T(srcUserId),srcSvcId,rId,rType,param,OLE2T(params));
	}
CATCH_LOG_COM
}

STDMETHODIMP CCoFAViewer::SetSubStream(BSTR dstUserId, ULONG dstSvcId, ULONG streamId, ULONG pointer_shared_ptr_stream)
{
TRY_CATCH_COM
	Log.Add(_MESSAGE_,_T("CCoFAViewer::SetSubStream() pointer_shared_ptr_stream=0x%08x"),pointer_shared_ptr_stream);

	USES_CONVERSION;
	switch(streamId)
	{
		case FASSID_MAIN:
			{
				Log.Add(_MESSAGE_,_T("CCoFAViewer::SetSubStream(SESSID_SE)"));
				boost::shared_ptr<CAbstractStream>* pstream = new boost::shared_ptr<CAbstractStream>(*reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream));
				m_stream = *reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream);
				PostMessage(m_msgSessionStarted,0,0);
				break;
			}
		default:
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Sub stream id is unknown. SetSbuStream(%s,0x%x,0x%x,0x%08x)"),OLE2T(dstUserId),dstSvcId,streamId,pointer_shared_ptr_stream);
	}
CATCH_LOG_COM
}


LRESULT CCoFAViewer::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	HRESULT result;
	if(!m_brokerEvents.m_dwCookie)
		throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
	CComPtr<_IBrokerClientEvents> brokerEvents;
	if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
	if(!brokerEvents.p)
		throw MCException("_IBrokerClientEvents has not marshaled");

	brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,RId(),BRT_SELECT_WIDGET,0,CComBSTR(_T("empty")));

	return MA_ACTIVATE;

CATCH_THROW()
}