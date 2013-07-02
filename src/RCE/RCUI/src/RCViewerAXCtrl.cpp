/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  RCViewerAXCtrl.cpp
///
///  IRCViewerAXCtrl,  ActiveX wrapper of CRCViewer
///
///  @author "Archer Software" Solovyov K. @date 23.11.2006
///
////////////////////////////////////////////////////////////////////////
// RCViewerAXCtrl.cpp : Implementation of CCoRCViewer

#include "stdafx.h"
#include "RCViewerAXCtrl.h"
//#include "CFactoryConnectDlg.h"
#include <NWL/Streaming/CNetworkLayer.h>
#include <NWL/Streaming/CStreamException.h>
#include <RCEngine/Streaming/coutstreamgzipped.h>
#include "AXmisc.h"
#include <AidLib/CSingleton/CSingleton.h>
#include <AidLib/Com/ComException.h>
#include <NWL/Streaming/CSocketSystem.h>
#include <NWL/TLS/CTLSSystem.h>
CSocketSystem sockSystem;
CTLSSystem tlsSystem;

#include <NWL/Streaming/CIMStub.h>
#include <RCEngine/AXstuff/AXstuff.h>


//TODO	what do with this.
#define rfbEncodingRaw 0
#define rfbEncodingCopyRect 1
#define rfbEncodingRRE 2
#define rfbEncodingCoRRE 4
#define rfbEncodingHextile 5
#define rfbEncodingZlib    6
#define rfbEncodingTight   7
#define rfbEncodingZlibHex 8
#define rfbEncodingUltra	9
#define rfbEncodingZRLE 16
#define rfbEncodingCache					0xFFFF0000
#define rfbEncodingCacheEnable				0xFFFF0001
#define rfbEncodingXOR_Zlib					0xFFFF0002
#define rfbEncodingXORMonoColor_Zlib		0xFFFF0003
#define rfbEncodingXORMultiColor_Zlib		0xFFFF0004
#define rfbEncodingSolidColor				0xFFFF0005
#define rfbEncodingXOREnable				0xFFFF0006
#define rfbEncodingCacheZip					0xFFFF0007
#define rfbEncodingSolMonoZip				0xFFFF0008
#define rfbEncodingUltraZip					0xFFFF0009
//TODO


CCoRCViewer::~CCoRCViewer()
{
TRY_CATCH
	m_terminateEvent.Set();
	TRY_CATCH
		if (m_spViewer)
			m_spViewer->Stop();
	CATCH_LOG()
	if(_RUNNING == State||_PAUSED == State)
	{
		CThread::Terminate();
		m_svcStream.reset();
		m_svcStreamConnectedEvent.Set();
		//WaitForSingleObject(hTerminatedEvent.get(),INFINITE);
		AtlWaitWithMessageLoop(hTerminatedEvent.get());
	} else
			m_svcStream.reset();
	{
		CCritSection cs(&m_cs);
	}
	DeleteCriticalSection(&m_cs);
CATCH_LOG("CCoRCViewer::~CCoRCViewer")
}

int CCoRCViewer::m_wndClassOffset;

// CCoRCViewer
CCoRCViewer::CCoRCViewer()
	:	m_instanceTracker(_T("RCViewer ActiveX")),
		m_msgFireEventOtherThreadsStarted(::RegisterWindowMessage(_T("CRC_AXCtrl::FireEventOtherThreadsStarted"))),//generate unique message code
		m_msgFireEventOtherThreadsStopped(::RegisterWindowMessage(_T("CRC_AXCtrl::FireEventOtherThreadsStopped"))),//generate unique message code
		m_msgFireEventOtherThreadsStartService(::RegisterWindowMessage(_T("CRC_AXCtrl::FireEventOtherThreadsStartService"))),
		m_captureAlphaBlend(false),
		m_ui(this),
		m_svcStreamConnectedEvent(false/*ManualReset*/, false /*InitialState*/),
		m_terminateEvent(false/*ManualReset*/, false/*InitialState*/),
		m_maximumAllowedPermission(RCAM_VIEW_ONLY),
		m_requestedPermission(RCAM_VIEW_ONLY),
		m_hideWallpaper(true),
		CRIdGen(100)
{
TRY_CATCH
	CSingleton<CCrtExeptionHook>::instance(); 
	REPORT_MODULE(RCUI_NAME);

	InitializeCriticalSection(&m_cs);
	m_bWindowOnly = TRUE;
	//#ifdef _DEBUG
	//	m_name.Format(_T("RCViewerAXCtrl(%d)"),::GetTickCount());::Sleep(1);//TODO
	//#endif
	if(!m_msgFireEventOtherThreadsStarted||!m_msgFireEventOtherThreadsStopped)
		throw MCException("Internal CRC_AXCtrl::FireEventOtherThreads message do not registered");

	m_wndClassOffset = 0;
	//m_wndClassOffset = reinterpret_cast<char*>(this) - reinterpret_cast<char*>(static_cast<CWindowImplBaseT<>* >(this)); 
	Log.Add(_MESSAGE_,_T("RCViewer ActiveX created"));

CATCH_LOG("CCoRCViewer::CCoRCViewer");
}

LRESULT CCoRCViewer::FireEventOtherThreadsStartService(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	boost::shared_ptr<CAbstractStream> stream;
	stream = *reinterpret_cast<boost::shared_ptr<CAbstractStream>* >(wParam);
	delete reinterpret_cast<boost::shared_ptr<CAbstractStream>* >(wParam);

	m_spViewer.reset(new CRCViewerImpl(stream,m_ui.GetViewerHostWnd(),this));
	m_spViewer->SubscribeEventsListener(boost::bind(&CCoRCViewer::OnViewerMinimize,this));
	m_spViewer->SetCustomOptions(m_opts);//set optins,if it was changed.
	m_spViewer->SetCaptureAlphaBlend(m_captureAlphaBlend);
	m_spViewer->SetHideWallpaper(m_hideWallpaper);
	/// Setting up scroll bars
	m_spViewer->SetScrollBars(m_ui.GetHorSrcrollBar(), m_ui.GetVertScrollBar());

	m_maximumAllowedPermission = m_requestedPermission;
	/// Setting up permission
	switch(m_requestedPermission)
	{
		case RCAM_VIEW_ONLY:
			m_spViewer->SetSessionMode(VIEW_ONLY, true);
			m_spViewer->SetSessionMode(VISUAL_POINTER, false);
			break;
		case RCAM_VISUAL_POINTER:
			m_spViewer->SetSessionMode(VIEW_ONLY, true);
			m_spViewer->SetSessionMode(VISUAL_POINTER, true);
			break;
		default:
			m_spViewer->SetSessionMode(VIEW_ONLY, false);
	}

	/// Starting viewer
	m_spViewer->Start();
	return 0;
CATCH_LOG("CCoRCViewer::FireEventOtherThreadsStartService")
	// Start failed, so stopping session
	PostMessage(m_msgFireEventOtherThreadsStopped,PROTOCOL_ERROR,0);
	return 0;
}

LRESULT CCoRCViewer::FireEventOtherThreadsStarted(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
TRY_CATCH
	m_ui.OnSessionStarted();
CATCH_LOG("CCoRCViewer::FireEventOtherThreadsStarted")
	return 0;
}

LRESULT CCoRCViewer::FireEventOtherThreadsStopped(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	m_spstream.reset();
	m_spViewer.reset(); //This will unlock host in case of viewer crash
	m_ui.OnSessionStopped(static_cast<ESessionStopReason>(wParam));
CATCH_LOG("CCoRCViewer::FireEventOtherThreadsStopped")
	bHandled = TRUE;
	return TRUE;
}

void CCoRCViewer::SendCtrlAltDel()
{
TRY_CATCH
	if(m_spViewer.get())
		m_spViewer->SendCtrlAltDel();
CATCH_LOG()
}

STDMETHODIMP CCoRCViewer::SetDisplayMode(LONG displayMode)
{
TRY_CATCH
	if(!m_spViewer.get())
		throw MCException("Connection has been not established yet.");
	m_spViewer->SetDisplayMode(EDisplayMode_(displayMode));
CATCH_LOG_COMERROR("CCoRCViewer::SetDisplayMode")
}

STDMETHODIMP CCoRCViewer::SetSessionOpts(LONG colorDepth, LONG encoding, VARIANT_BOOL useCompressLevel, LONG compressLevel, VARIANT_BOOL jpegCompress, LONG jpegQualityLevel)
{	
TRY_CATCH
	m_opts.m_colorsCount=colorDepth;
	//+map encoding
	switch(encoding)
	{	case 0: m_opts.m_PreferredEncoding=rfbEncodingZRLE;break;
		case 1: m_opts.m_PreferredEncoding=rfbEncodingXOR_Zlib;break;
		case 2: m_opts.m_PreferredEncoding=rfbEncodingZlibHex;break;
		case 3: m_opts.m_PreferredEncoding=rfbEncodingHextile;break;
		case 4: m_opts.m_PreferredEncoding=rfbEncodingRRE;break;
		//case 5: m_opts.m_PreferredEncoding=rfbEncodingCoRRE;break;
		case 5: m_opts.m_PreferredEncoding=rfbEncodingRaw;break;
		//case 7: m_opts.m_PreferredEncoding=rfbEncodingUltra;break;
		case 6: m_opts.m_PreferredEncoding=rfbEncodingTight;break;
	}
	//-map encoding
	m_opts.m_useCompressLevel=useCompressLevel;
	m_opts.m_compressLevel=compressLevel;
	m_opts.m_enableJpegCompression=jpegCompress;
	m_opts.m_jpegQualityLevel=jpegQualityLevel;
	//::MessageBox(NULL,"CCoRCViewer::SetSessionOpts",NULL,0);
	//if(!m_spViewer.get())return S_OK;//TODO how catch E_UNEXPECTED in js?;
	//m_spViewer->SetCustomOptions(opts);
CATCH_LOG_COMERROR("CCoRCViewer::SetSessionOpts")
}

STDMETHODIMP CCoRCViewer::SetSessionMode(LONG mode, VARIANT_BOOL state)
{	
TRY_CATCH
	if(!m_spViewer.get())
		throw MCException("Connection has been not established yet.");
	m_spViewer->SetSessionMode(ESessionMode(mode),state);
CATCH_LOG_COMERROR("CCoRCViewer::SetSessionMode")
}

STDMETHODIMP CCoRCViewer::SetSessionRecording(BSTR fileName, VARIANT_BOOL mode)
{	
TRY_CATCH
	if(!mode)//stop recording	
	{	
		m_spViewer->SetShadowStream(boost::shared_ptr<CAbstractStream>(reinterpret_cast<CAbstractStream*>(NULL)));
		return S_OK;
	}
	USES_CONVERSION;
	TCHAR path[MAX_PATH];
	try
	{	
		GetRCRecFullFileName(path,OLE2T(fileName));
		COutStreamGZipped *fileStream = new COutStreamGZipped(path);
		m_spViewer->SetShadowStream(boost::shared_ptr<CAbstractStream>(fileStream));	
		
	}
	catch(CStreamException &e)
	{	Log.Add(_MESSAGE_,e.What().c_str());
		//NotifySessionStop(OPENFILE_ERROR);
		InvokeEvent(1 /*NotifySessionStop*/, OPENFILE_ERROR);//TODO for handling error in js
		return S_OK;
	}
CATCH_LOG_COMERROR("CCoRCViewer::SetSessionRecording")
}

STDMETHODIMP CCoRCViewer::StartPlayback(BSTR fileName)
{
TRY_CATCH
	USES_CONVERSION;
	TCHAR path[MAX_PATH];
	try
	{	GetRCRecFullFileName(path,OLE2T(fileName));
		m_spstream.reset(new CInStreamGZipped(path));
		//m_spViewer.reset(new CRCViewerImpl(boost::shared_ptr<CAbstractStream>(m_spstream),m_hWnd,this));
		m_spViewer.reset(new CRCViewerImpl(m_spstream,m_hWnd,this));
	}
	catch(CStreamException &e)
	{	Log.Add(_MESSAGE_,e.What().c_str());
		//NotifySessionStop(OPENFILE_ERROR);
		InvokeEvent(1 /*NotifySessionStop*/, OPENFILE_ERROR);//TODO for handling error in js
		return S_OK;
	}
	m_spViewer->Start();
CATCH_LOG_COMERROR("CCoRCViewer::StartPlayback")
}

STDMETHODIMP CCoRCViewer::SetDelayFactor(DOUBLE delayFactor)
{
TRY_CATCH
	//CInFileStreamTimeStamped* stream = dynamic_cast<CInFileStreamTimeStamped*>(m_spstream.get());
	//if(!stream)throw MCException("This isn't replay session");
	//TODO m_spstream are used only for playback
	if(!m_spstream.get())
		throw MCException("The replay session is not start");
	m_spstream->SetDelayFactor(delayFactor);
	//MessageBox(Format(_T("%f"),m_spstream->m_delayFactor).c_str()); 
CATCH_LOG_COMERROR("CCoRCViewer::SetDelayFactor")
}

STDMETHODIMP CCoRCViewer::SetPlaybackMode(LONG mode)
{	
TRY_CATCH
	if(!m_spstream.get())throw MCException("The replay session is not start");
	switch(mode)
	{	case 0: m_spstream->Start();break;
		case 1:	m_spstream->Stop();break;
		case 2: m_spstream->Pause();break;
		default :	throw MCException("The mode parameter is invalid");
	}
	//MessageBox(Format(_T("%x"),m_spstream->m_state).c_str());
CATCH_LOG_COMERROR("CCoRCViewer::SetPlaybackMode")
}				

STDMETHODIMP CCoRCViewer::SetCaptureAlphaBlend(VARIANT_BOOL captureAlphaBlend)
{
TRY_CATCH
	m_captureAlphaBlend = captureAlphaBlend != 0;
	if(!m_spViewer.get())
		throw MCException("Connection has been not established yet.");
		m_spViewer->SetCaptureAlphaBlend(m_captureAlphaBlend);
CATCH_LOG_COMERROR("CCoRCViewer::SetCaptureAlphaBlend")
}

void CCoRCViewer::SetRemoteDesktopSize(const int width, const int height)
{
TRY_CATCH
	m_cx = width;
	m_cy = height;
CATCH_LOG("CCoRCViewer::SetRemoteDesktopSize")
}

LRESULT CALLBACK CCoRCViewer::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
TRY_CATCH

	CCoRCViewer* pThis = (CCoRCViewer*)(((DWORD)hWnd) + m_wndClassOffset);	/// Brutal hack
																					/// TODO: think how bypass this
																					/// the reason why dynamic_cast isn't used
																					/// is Runtime Type Info disabled
	if (uMsg == pThis->m_msgFireEventOtherThreadsStopped) ///Proper hack
	{
		//LRESULT lRes;
		BOOL handled;
		return pThis->FireEventOtherThreadsStopped(uMsg, wParam, lParam, handled);
		//ProcessWindowMessage(pThis->m_hWnd, uMsg, wParam, lParam, lRes, 0);
	} else
	{
		return CWindowImplBaseT<>::WindowProc(hWnd, uMsg, wParam, lParam);
	}

CATCH_LOG("CCoRCViewer::WindowProc")
	return FALSE;
}
LRESULT CCoRCViewer::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	m_ui.CreateUI(m_hWnd);
CATCH_LOG()
	return 1;  // Let the system set the focus
}

LRESULT CCoRCViewer::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
TRY_CATCH
	m_ui.Size(wParam,lParam);
CATCH_LOG()
	return 0;
}

STDMETHODIMP CCoRCViewer::SetUIStatus(LONG status, BSTR message)
{
TRY_CATCH
	USES_CONVERSION;
	m_ui.SetExtendedUIStatus(static_cast<EDSStateEx>(status),OLE2A(message));
CATCH_LOG_COMERROR("CCoRCViewer::SetCaptureAlphaBlend")
}

STDMETHODIMP CCoRCViewer::Init(BSTR peerId)
{
TRY_CATCH
	USES_CONVERSION;
	m_ui.OnViewerInit(tstring(OLE2T(peerId)));
CATCH_LOG_COMERROR("CCoRCViewer::SetCaptureAlphaBlend")
}

LRESULT CCoRCViewer::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
TRY_CATCH
	m_ui.DestroyUI();
	m_terminateEvent.Set();
	m_svcStreamConnectedEvent.Set();
CATCH_LOG()
	return 0;
}

void CCoRCViewer::ApplyOptions()
{
TRY_CATCH
	if (NULL != m_spViewer.get())
		m_spViewer->SetCustomOptions(m_opts);
CATCH_LOG()
}

STDMETHODIMP CCoRCViewer::Init(IUnknown *events)
{
TRY_CATCH
	HRESULT result;
	Log.Add(_MESSAGE_,_T("CCoRCViewer::Init(0x%08x)"),events);

	CComPtr<IUnknown> eventsUnkn(events);
	CComPtr<_IBrokerClientEvents>	brokerEvents;
	if(S_OK != (result=eventsUnkn.QueryInterface(&brokerEvents)))
	{
		SetLastError(result);
		throw MCException_Win(_T("QueryInterface(_IBrokerClientEvents) failed "));
	}
	
	if(S_OK!=(result=m_brokerEvents.Attach(brokerEvents)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Broker object GIT registeration failed")),result);

CATCH_LOG_COMERROR()
}

STDMETHODIMP CCoRCViewer::HandleRequest(BSTR dstUserId,ULONG dstSvcId,BSTR srcUserId,ULONG srcSvcId,ULONG rId,ULONG rType,ULONG param,BSTR params)
{
TRY_CATCH
	//// Add your function implementation here.
	////RequestSent(sId,svcId,rId,rType,param,params);
	////m_brokerEvents->RequestSent(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
	//USES_CONVERSION;
	//Log.Add(_MESSAGE_,_T("CCoRCViewer::HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),OLE2T(dstUserId),dstSvcId,OLE2T(srcUserId),srcSvcId,rId,rType,param,OLE2T(params));
	//if((BRT_SERVICE|BRT_RESPONSE)==rType)
	//{
	//	//ping
	//	m_brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,rId,BRT_PING,param,params);
	//	m_brokerEvents->GetSubStream(CComBSTR(BUSERIDPDV_AUTOSET), BSVCIDPDV_AUTOSET, 0/*service stream*/, 1);
	//}
	USES_CONVERSION;
	switch(rType)
	{
		case BRT_PING:
			{
				Log.Add(_MESSAGE_,_T("CCoRCViewer::HandleRequest(BRT_PING)"));
				HRESULT result;
				if(!m_brokerEvents.m_dwCookie)
					throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
				CComPtr<_IBrokerClientEvents> brokerEvents;
				if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
				if(!brokerEvents.p)
					throw MCException("_IBrokerClientEvents has not marshaled");
				if(S_OK!=brokerEvents->RequestSent(srcUserId,srcSvcId,srcUserId,dstSvcId,rId,rType|BRT_RESPONSE,param,params))//response on ping
					MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),TStringFromErrorInfo()))
			}
			break;
		case BRT_PING|BRT_RESPONSE:
			{
				Log.Add(_MESSAGE_,_T("CCoRCHost::HandleRequest(BRT_PING|BRT_RESPONSE)"));
			}
			break;
		case BRT_SERVICE|BRT_RESPONSE:
			{
				if(BRR_BPFAILED==param)
				{
					m_ui.SetExtendedUIStatus(EDSS_PERMISSION_DENIED, Format(BRT_SERVICE_BPFAILED,OLE2T(params)));
				}
				else if(BRR_ERROR==param)
				{
					m_ui.SetExtendedUIStatus(EDSS_PERMISSION_DENIED, Format(_T("%s. Request handling failed"),BRT_SERVICE_DECLINED));
				}
				else if(BRR_DECLINED==param)
				{
					m_ui.SetExtendedUIStatus(EDSS_PERMISSION_DENIED, BRT_SERVICE_DECLINED);
				}
				else if(BRR_APPROVED==param)
				{
					m_ui.SetExtendedUIStatus(EDSS_PERMISSION_RECEIVED,BRT_SERVICE_APPROVED);
					if(!m_svcStream.get())
						CThread::Start();
					else
						InitiateRCConnectAndStart();
				}
				else if(BRR_BUSY==param)
				{
					m_ui.SetExtendedUIStatus(EDSS_PERMISSION_DENIED,BRT_SERVICE_BUSY);
				}
				else
				{
					m_ui.SetExtendedUIStatus(EDSS_PERMISSION_DENIED,Format(_T("Desktop sharing response is unknown (code=0x%x) %s"),param,OLE2T(params)));
					MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Responce type is unknown. HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),OLE2T(dstUserId),dstSvcId,OLE2T(srcUserId),srcSvcId,rId,rType,param,OLE2T(params)));
				}
			}
			break;
		case BRT_MESSAGEBOX|BRT_RESPONSE:
			{
				switch(rId)
				{
					case RID_PERMISSION:
						if (NULL == m_svcStream.get())
						{
							Log.Add(_ERROR_,_T("BRT_MESSAGEBOX|BRT_RESPONSE while NULL == m_svcStream.get() received. Scipping..."));
						} else if (param == 0)
						{
							// Accept pressed
							if (NULL == m_spViewer.get())
							{
								// Accept pressedd
								m_ui.SetExtendedUIStatus(EDSS_PERMISSION_RECEIVED, BRT_SERVICE_APPROVED); //TODO: add perm name
								/// We already have stream, and RCHost on remote side as well as well as permission grant
								/// Just asking RCHost to start
								ULONG buf=RCC_START_REQ;
								m_svcStream->Send(reinterpret_cast<char*>(&buf),sizeof(buf));
							} else
							{								
								m_ui.SetExtendedUIStatus(EDSS_PERMISSION_RECEIVED, BRT_SERVICE_APPROVED); //TODO: add perm name
								m_maximumAllowedPermission = m_requestedPermission;
								HandleUIMediatorCommand(UIE_PERMISSION_REQUEST, m_requestedPermission);
							}
						}
						else if(BRR_BUSY==param)
						{
							m_ui.SetExtendedUIStatus(EDSS_PERMISSION_DENIED,BRT_SERVICE_BUSY);
						}
						else
						{
							// Deny pressed
							m_ui.SetExtendedUIStatus(EDSS_PERMISSION_DENIED, BRT_SERVICE_DECLINED); //TODO: add perm name
						}
						break;
					default:
						Log.Add(_WARNING_,_T("MessageBox response with unknown request id %d"),rId);
				}
			}
			break;
		case BRT_INSTALLATION:
			if(param)
				m_ui.SetExtendedUIStatus(EDSS_INSTALLATION_PROGRESS, Format(BRT_INSTALLATION_EXFORMAT,OLE2T(params),param));
			else
				m_ui.SetExtendedUIStatus(EDSS_INSTALLATION_PROGRESS,_OLE2T(params));
			break;
		case BRT_CONNECTION:
			m_ui.SetExtendedUIStatus(EDSS_PERMISSION_REQUEST_SENT,_OLE2T(params));
			break;
		default:
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Request type is unknown. HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),OLE2T(dstUserId),dstSvcId,OLE2T(srcUserId),srcSvcId,rId,rType,param,OLE2T(params));
	}
CATCH_LOG_COMERROR()
}

void CCoRCViewer::HandleCoHostCommand(ULONG buf)
{
TRY_CATCH
	switch(buf)
	{
		case RCC_START_REQ_DECLINE:
			m_ui.SetExtendedUIStatus(EDSS_PERMISSION_DENIED, BRT_SERVICE_DECLINED);
			break;
		case RCC_START_REQ_APPROVE:
			m_ui.SetExtendedUIStatus(EDSS_PERMISSION_RECEIVED, BRT_SERVICE_APPROVED);
			InitiateRCConnectAndStart();
			break;
		default:
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Unknown service message type=0x%x"),buf));
	}
CATCH_THROW()
}

STDMETHODIMP CCoRCViewer::SetSubStream(BSTR dstUserId, ULONG dstSvcId, ULONG streamId, ULONG pointer_shared_ptr_stream)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CCoRCViewer::SetSubStream() pointer_shared_ptr_stream=0x%08x"),pointer_shared_ptr_stream);
	USES_CONVERSION;
	switch(streamId)
	{
		case RCSSID_SERVICE:
			{
				/// Service stream received, unblocking corresponding event
				Log.Add(_MESSAGE_,_T("CCoRCViewer::SetSubStream(RCSSID_SERVICE)"));
				m_svcStream=*reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream);
				m_svcStreamConnectedEvent.Set();
			}
			break;
		case RCSSID_RC:
			{
				/// Viewer stream received, initiating viewer start
				Log.Add(_MESSAGE_,_T("CCoRCViewer::SetSubStream(RCSSID_RC)"));
				boost::shared_ptr<CAbstractStream>* pstream = new boost::shared_ptr<CAbstractStream>(*reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream));
				/// Since viewer should start in GUI thread, doing this through PostMessage mechanism
				PostMessage(m_msgFireEventOtherThreadsStartService, reinterpret_cast<WPARAM>(pstream),0);
			}
			break;
		default:
			throw	CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Sub stream id is unknown. SetSbuStream(%s,0x%x,0x%x,0x%08x)"),OLE2T(dstUserId),dstSvcId,streamId,pointer_shared_ptr_stream);
	}
CATCH_LOG_COMERROR()
}

void CCoRCViewer::HandleUIMediatorCommand(ERCViewerUIEvents eventType, LONG param)
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

	switch(eventType)
	{
		case UIE_SESSION_START:
			if(m_svcStream.get())
			{

				m_maximumAllowedPermission = static_cast<ERCAccessMode>(-1);
				m_requestedPermission = static_cast<ERCAccessMode>(param);
				TCHAR *access;
				if(RCAM_VIEW_ONLY==param)
					access=BRT_SERVICE_RCVIEWONLY;
				else if(RCAM_VISUAL_POINTER==param)
					access=BRT_SERVICE_RCVISUALPOINTER;
				else if(RCAM_FULL_CONTROL==param)
					access=BRT_SERVICE_RCFULLCONTROL;
				else 
					access=_T(" ");
				TCHAR* wallpaper;
				if(m_hideWallpaper)
					wallpaper = BRT_SERVICE_RCWALLPAPEROFF;
				else
					wallpaper = _T("");
				tstring& reqParams=Format(BRT_SERVICE_RCFORMAT,BRT_SERVICE_RCTEXT,access,wallpaper,BRT_SERVICE_APPROVE,BRT_SERVICE_DECLINE);
				//tstring& reqParams=Format(_T("[Caption text];;[Message text];;[approve button text];;[decline button text]"));
				if(S_OK!=brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,RID_PERMISSION,BRT_MESSAGEBOX,2/*buttons count*/,CComBSTR(reqParams.c_str())))
				{
					MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),TStringFromErrorInfo()))
					//TODO set error UI state
				}
				else
					m_ui.SetExtendedUIStatus(EDSS_PERMISSION_REQUEST_SENT, BRT_SERVICE_WAITING_APPROVE);
			}
			else
			{
				/// We haven't created host on remote side, 
				/// So asking broker to show accept/deny request on remote side, and init service
				/// And start waiting for BRT_SERVICE|BRT_RESPONSE
				m_requestedPermission = static_cast<ERCAccessMode>(param);
				TCHAR *access;
				if(RCAM_VIEW_ONLY==param)
					access=BRT_SERVICE_RCVIEWONLY;
				else if(RCAM_VISUAL_POINTER==param)
					access=BRT_SERVICE_RCVISUALPOINTER;
				else if(RCAM_FULL_CONTROL==param)
					access=BRT_SERVICE_RCFULLCONTROL;
				else 
					access=_T(" ");
				TCHAR* wallpaper;
				if(m_hideWallpaper)
					wallpaper = BRT_SERVICE_RCWALLPAPEROFF;
				else
					wallpaper = _T("");
				tstring& reqParams=Format(BRT_SERVICE_RCFORMAT,BRT_SERVICE_RCTEXT,access,wallpaper,BRT_SERVICE_APPROVE,BRT_SERVICE_DECLINE);
				//m_ui.SetExtendedUIStatus(EDSS_PERMISSION_REQUEST_SENT, BRT_SERVICE_WAITING_APPROVE);
				//brokerEvents->RequestSent(CComBSTR(),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,RID_PERMISSION,BRT_MESSAGEBOX,2/*buttons count*/,CComBSTR(reqParams.c_str()));
				if(S_OK!=brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_BROKER,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,RId(),BRT_SERVICE,BST_RCHOST,CComBSTR(reqParams.c_str())))
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("RequestSent(service) failed.\n"))+TStringFromErrorInfo());
				m_ui.SetExtendedUIStatus(EDSS_PERMISSION_REQUEST_SENT, BRT_SERVICE_WAITING_APPROVE);
			}
			break;
		case UIE_SESSION_STOP:
			if(!m_spViewer.get())
				throw MCException("RC session has not started");
			m_spViewer->Stop();
			break;
		case UIE_PERMISSION_REQUEST:
			m_requestedPermission = static_cast<ERCAccessMode>(param);
			if (m_spViewer.get())
			{
				// We already in session, so should handle permission request interactively
				if (m_requestedPermission <= m_maximumAllowedPermission)
				{
					// User already approved needed permission, just setting it on
					switch(m_requestedPermission)
					{
						case RCAM_VIEW_ONLY:
							m_spViewer->SetSessionMode(VIEW_ONLY, true);
							m_spViewer->SetSessionMode(VISUAL_POINTER, false);
							break;
						case RCAM_VISUAL_POINTER:
							m_spViewer->SetSessionMode(VIEW_ONLY, true);
							m_spViewer->SetSessionMode(VISUAL_POINTER, true);
							break;
						default:
							m_spViewer->SetSessionMode(VIEW_ONLY, false);
					}
				} else
				{
					TCHAR *access;
					if(RCAM_VIEW_ONLY==param)
						access=BRT_SERVICE_RCVIEWONLY;
					else if(RCAM_VISUAL_POINTER==param)
						access=BRT_SERVICE_RCVISUALPOINTER;
					else if(RCAM_FULL_CONTROL==param)
						access=BRT_SERVICE_RCFULLCONTROL;
					else 
						access=_T(" ");
					tstring& reqParams=Format(BRT_SERVICE_RCFORMAT,BRT_SERVICE_RCTEXT,access,_T(""),BRT_SERVICE_APPROVE,BRT_SERVICE_DECLINE);
					//tstring& reqParams=Format(_T("[Caption text];;[Message text];;[approve button text];;[decline button text]"));
					if(S_OK!=brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,RID_PERMISSION,BRT_MESSAGEBOX,2/*buttons count*/,CComBSTR(reqParams.c_str())))
						throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("RequestSent(permissions) failed.\n"))+TStringFromErrorInfo());
					m_ui.SetExtendedUIStatus(EDSS_PERMISSION_REQUEST_SENT,_T("Waiting for customer approval for new permission"));
				}
			}
			break;
		default:
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),0,_T("Unknown UI event type=0x%x param=0x%x"),eventType,param);
	}
CATCH_LOG()
}

void CCoRCViewer::InitiateRCConnectAndStart(void)
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
	if(!m_svcStream.get())
		throw MCException("Service doesn't exist");
	ULONG buf=RCC_START;
	//m_svcStream->Send(reinterpret_cast<char*>(&buf),sizeof(buf));
	if(S_OK!=brokerEvents->GetSubStream(CComBSTR(BUSERIDPDV_AUTOSET), BSVCIDPDV_AUTOSET, RCSSID_RC, RCSSP_RC))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),TStringFromErrorInfo());
	m_svcStream->Send(reinterpret_cast<char*>(&buf),sizeof(buf));
CATCH_LOG()
}

void CCoRCViewer::Execute(void *Params)
{
TRY_CATCH
	//CoInitialize(NULL);
	CoInitializeEx(NULL,COINIT_MULTITHREADED);
	if(!m_brokerEvents.m_dwCookie)
			throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");

	bool firstRC=true;
	//while(!Terminated())
	do
	{
		Log.Add(_MESSAGE_,_T("GET SERVICE SUBSTREAM"));
		HRESULT result;
		CComPtr<_IBrokerClientEvents> brokerEvents;
		if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
		if(!brokerEvents.p)
			throw MCException("_IBrokerClientEvents has not marshaled");
		
		if((result=brokerEvents->GetSubStream(CComBSTR(BUSERIDPDV_AUTOSET), BSVCIDPDV_AUTOSET, RCSSID_SERVICE, RCSSP_RC))!=S_OK)
		{
			//MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service substream obtaining failed")),result))
			MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),TStringFromErrorInfo()))
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
			if(firstRC)
			{
				firstRC=false;
				InitiateRCConnectAndStart();
			}
			while(!Terminated())
			{
				ULONG buf=0;
				m_svcStream->Receive(reinterpret_cast<char*>(&buf),sizeof(buf));
				try
				{
					HandleCoHostCommand(buf);
				}
				catch(CExceptionBase& e)
				{
					MLog_Exception(e);
					DWORD result = WaitForSingleObject(m_terminateEvent, RECONNECT_TIMEOUT);
					switch(result)
					{
						case WAIT_OBJECT_0:
							return;
						case WAIT_TIMEOUT:
							Log.Add(_MESSAGE_,_T("RCViewer: going to reconnect now"));
							break;
						default:
							Log.WinError(_ERROR_,_T("RCViewer: Waiting reconnect timeout failed, exiting service thread"));
							return;
					}
				}
			}
		}
		catch(CStreamException& e)
		{
			MLog_Exception(e);
		}
	}
	while(false);

	m_svcStream.reset();
	m_maximumAllowedPermission = RCAM_VIEW_ONLY;

CATCH_LOG()
}

void CCoRCViewer::OnViewerMinimize()
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

	brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,RId(),BRT_MINIMIZE_WIDGET,0,CComBSTR(_T("empty")));
CATCH_THROW()
}

LRESULT CCoRCViewer::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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