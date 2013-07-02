/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  RCHostAXCtrl.cpp
///
///  IRCHostAXCtrl,  ActiveX wrapper of CRCHost
///
///  @author "Archer Software" Solovyov K. @date 24.11.2006
///
////////////////////////////////////////////////////////////////////////
// RCHostAXCtrl.cpp : Implementation of CRCHostAXCtrl
#include "stdafx.h"
#include "RCHostAXCtrl.h"
#include <NWL/Streaming/CNetworkLayer.h>
#include <RCEngine/Streaming/coutstreamgzipped.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "AXmisc.h"

#include <NWL/Streaming/CIMStub.h>
#include <RCEngine/AXstuff/AXstuff.h>
// CRCHostAXCtrl
///
CRCHostAXCtrl::CRCHostAXCtrl()
	:	m_instanceTracker(_T("RCHost ActiveX")),
		m_msgFireEventOtherThreadsStarted(::RegisterWindowMessage(_T("CRC_AXCtrl::FireEventOtherThreadsStarted"))),//generate unique message code
		m_msgFireEventOtherThreadsStopped(::RegisterWindowMessage(_T("CRC_AXCtrl::FireEventOtherThreadsStopped"))),//generate unique message code
		m_msgFireEventOtherThreadsConnecting(::RegisterWindowMessage(_T("CRC_AXCtrl::FireEventOtherThreadsConnecting")))
{		
TRY_CATCH
		CSingleton<CCrtExeptionHook>::instance();
		REPORT_MODULE(RCUI_NAME);

		m_bWindowOnly=TRUE;//for fire event in other thread
		if(!m_msgFireEventOtherThreadsStarted||!m_msgFireEventOtherThreadsStopped||!m_msgFireEventOtherThreadsConnecting)
			throw MCException("Internal CRCHostAXCtrl::FireEventOtherThreads messages do not registered");
		m_fConnecting=false;

CATCH_LOG("CRCHostAXCtrl::CRCHostAXCtrl")
}

CRCHostAXCtrl::~CRCHostAXCtrl()
{
}

STDMETHODIMP CRCHostAXCtrl::StartClient(BSTR userId, BSTR password, BSTR peerId,BSTR relayServer,ULONG timeOut,LONG* id)
{	
TRY_CATCH
	USES_CONVERSION;
	if(m_fConnecting)
		MCException("Connection process has begun already, but it are not completed yet.");
	else
	{ 
		NWL_INSTANCE.SetRelayHost(OLE2T(relayServer));
		SetServerUserId(OLE2T(userId));
		SetServerPassword(OLE2T(password));
		*id=-2;//temporary id, that exist until connection process is completed. Then, NotifySessionStart pass valid id.
		m_fConnecting=true;
		Connect(OLE2T(userId),OLE2T(peerId),timeOut,true/*master role. Allways false for viewer*/,true/*async*/);
	}
CATCH_LOG_COMERROR("CRCHostAXCtrl::StartClient")
}
//TODO MAX I think critical section would not here.
void CRCHostAXCtrl::ConnectCompletion(boost::shared_ptr<CAbstractNetworkStream> stream)
{
TRY_CATCH
	//MessageBox("CRCHostAXCtrl::ConnectCompletion");
	m_fConnecting = false;
	if (stream.get())
	{
		CRCHost::StartClient(stream,0);
	} else
	{ 
#ifdef USEIMSTUB		
		CIMStub im(m_sourcePeerId);
		im.RemoveAllMyMessagesFromServer();
#endif
		Log.Add(_ERROR_,_T("CRCHostAXCtrl::ConnectCompletion: Failed to connect: %s"),m_error.c_str());
		if (NULL == m_Mediator)
			Log.Add(_ERROR_,_T("IMMediator isn't set. This could be the cause of connect failure"));
		PostMessage(m_msgFireEventOtherThreadsStopped,-2,CONNECTING_ERROR);
	}
CATCH_THROW("CRCHostAXCtrl::ConnectCompletion")
}

STDMETHODIMP CRCHostAXCtrl::StopClient(LONG id)
{	
TRY_CATCH
	if(m_fConnecting)
	{
		AbortConnect();	
		m_fConnecting = false;
	}
	else 
		CRCHost::StopClient(id);
CATCH_LOG_COMERROR("CRCHostAXCtrl::StopClient")
}

STDMETHODIMP CRCHostAXCtrl::SetSessionMode(LONG clientId,LONG mode,VARIANT_BOOL state)
{	
TRY_CATCH
	CRCHost::SetSessionMode(clientId,ESessionMode(mode),state != 0);
CATCH_LOG_COMERROR("CRCHostAXCtrl::SetSessionMode")
}


STDMETHODIMP CRCHostAXCtrl::SetSessionRecording(BSTR fileName, VARIANT_BOOL mode)
{	
TRY_CATCH
	if(!mode)//stop recording	
	{	SetShadowStream(boost::shared_ptr<CAbstractStream>(reinterpret_cast<CAbstractStream*>(NULL)));
		return S_OK;
	}
	USES_CONVERSION;
	TCHAR path[MAX_PATH];
	try
	{	GetRCRecFullFileName(path,OLE2T(fileName));
		COutStreamGZipped *fileStream = new COutStreamGZipped(path);
		SetShadowStream(boost::shared_ptr<CAbstractStream>(fileStream));	
	}
	catch(CStreamException &e)
	{	Log.Add(_MESSAGE_,e.What().c_str());
		NotifySessionStop(-1,OPENFILE_ERROR);
		return S_OK;
	}
CATCH_LOG_COMERROR("CRCHostAXCtrl::SetSessionRecording")
}

STDMETHODIMP CRCHostAXCtrl::ProtectWindow(VARIANT_BOOL mode)
{	
TRY_CATCH
		SetProtectedWindow((mode)?(HANDLE)m_hWnd:NULL);
CATCH_LOG_COMERROR("CRCHostAXCtrl::ProtectWindow")
}

STDMETHODIMP CRCHostAXCtrl::SetCaptureAlphaBlend(VARIANT_BOOL captureAlphaBlend)
{
TRY_CATCH
	CRCHost::SetCaptureAlphaBlend(captureAlphaBlend != 0);
CATCH_LOG_COMERROR("CRCHostAXCtrl::SetCaptureAlphaBlend")
}


void CRCHostAXCtrl::NotifyProgress(const int& percentCompleted, const tstring& status)
{
TRY_CATCH
	PostMessage(m_msgFireEventOtherThreadsConnecting,reinterpret_cast<WPARAM>(new CComBSTR(status.c_str())),percentCompleted);
CATCH_LOG("CRCHostAXCtrl::NotifyProgress")
}

//HRESULT CRCHostAXCtrl::OnDraw(ATL_DRAWINFO& di)
//{ 
//	RECT& rc = *(RECT*)di.prcBounds;
//	// Set Clip region to the rectangle specified by di.prcBounds
//	HRGN hRgnOld = NULL;
//	if(GetClipRgn(di.hdcDraw,hRgnOld)!=1)hRgnOld=NULL;
//	bool bSelectOldRgn=false;
//	HRGN hRgnNew=CreateRectRgn(rc.left,rc.top,rc.right,rc.bottom);
//	if (hRgnNew != NULL){ bSelectOldRgn = (SelectClipRgn(di.hdcDraw, hRgnNew) != ERROR);}
//	Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
//	SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
//	LPCTSTR pszText = _T("ATL 7.0 : RCHostAXCtrl");
//	TextOut(di.hdcDraw,(rc.left + rc.right)/2,(rc.top + rc.bottom)/2,pszText,lstrlen(pszText));
//	if (bSelectOldRgn)SelectClipRgn(di.hdcDraw, hRgnOld);
//	return S_OK;
//}
LRESULT CRCHostAXCtrl::FireEventOtherThreadsStarted(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
TRY_CATCH
	NotifySessionStart(static_cast<LONG>(wParam));
CATCH_LOG("CRCHostAXCtrl::FireEventOtherThreadsStarted")
	return 0;
}
LRESULT CRCHostAXCtrl::FireEventOtherThreadsStopped(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
TRY_CATCH
	NotifySessionStop(static_cast<LONG>(wParam),static_cast<LONG>(lParam));
CATCH_LOG("CRCHostAXCtrl::FireEventOtherThreadsStopped")
	return 0;
}

LRESULT CRCHostAXCtrl::FireEventOtherThreadsConnecting(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
TRY_CATCH
	NotifyConnecting(static_cast<LONG>(lParam),static_cast<BSTR>(*reinterpret_cast<CComBSTR*>(wParam)));
	delete reinterpret_cast<CComBSTR*>(wParam);
CATCH_LOG("CRCViewerAXCtrl::FireEventOtherThreadsConnecting")
	return 0;
}
void CRCHostAXCtrl::NotifySessionStarted(const int clientId)
{
TRY_CATCH
	m_fConnecting = false;
	if (IsWindow())
		::PostMessage(m_hWnd,m_msgFireEventOtherThreadsStarted,clientId,0);
CATCH_LOG("CRCHostAXCtrl::NotifySessionStarted")
}

void CRCHostAXCtrl::NotifySessionStopped(const int clientId,ESessionStopReason ReasonCode)
{
TRY_CATCH
	if (IsWindow())
		if (m_fConnecting)
			::PostMessage(m_hWnd,m_msgFireEventOtherThreadsStopped,-2,PROTOCOL_ERROR);
		else
			::PostMessage(m_hWnd,m_msgFireEventOtherThreadsStopped,clientId,ReasonCode);
	m_fConnecting = false;
CATCH_LOG("CRCHostAXCtrl::NotifySessionStopped")
}