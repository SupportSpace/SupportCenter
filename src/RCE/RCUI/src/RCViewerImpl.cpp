/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  RCViewerImpl.cpp
///
///  IRCViewerImpl,  Implementation of CRCViewer
///
///  @author "Archer Software" Solovyov K. @date 27.11.2006
///
////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include ".\rcviewerimpl.h"
#include "RCViewerAXCtrl.h"
#include "NWL\Streaming\CDirectNetworkStream.h"
#include "NWL\Streaming\CNATTraversingUDPNetworkStream.h"
#include "NWL\Streaming\CRelayedNetworkStream.h"
//.    Using relay.

CRCViewerImpl::~CRCViewerImpl(void)
{	TRY_CATCH
	CATCH_LOG("CRCViewerImpl::~CRCViewerImpl")
}

void CRCViewerImpl::NotifySessionStarted()
{	
TRY_CATCH
	//connect type obtaining
	CAbstractStream *ptr;
	ptr=this->m_stream->GetMainStream().get();
	int conType=ECT_UNKNOWN;
	if(dynamic_cast<CDirectNetworkStream*>(ptr))
		conType=ECT_DIRECT;
	else if(dynamic_cast<CNATTraversingUDPNetworkStream*>(ptr))
		conType=ECT_NAT;
	else if(dynamic_cast<CRelayedNetworkStream<>*>(ptr))
		conType=ECT_RELAY;
	if(ECT_UNKNOWN==conType)
		Log.Add(_WARNING_,_T("CRCViewerImpl::NotifySessionStarted() unknown connect type"));
	if (IsWindow(m_owner->m_hWnd))
		PostMessage(m_owner->m_hWnd,m_owner->m_msgFireEventOtherThreadsStarted,conType,0);//for round problem in other thread fire ActiveX event
CATCH_LOG("CRCViewerImpl::NotifySessionStarted");
}

void CRCViewerImpl::NotifySessionStopped(ESessionStopReason ReasonCode)
{	
TRY_CATCH
	if (IsWindow(m_owner->m_hWnd))
		PostMessage(m_owner->m_hWnd,m_owner->m_msgFireEventOtherThreadsStopped,ReasonCode,0);//for round problem in other thread fire ActiveX event
CATCH_LOG("CRCViewerImpl::NotifySessionStopped");
}

void CRCViewerImpl::NorifyDisplayModeChanged(EDisplayMode mode)
{
TRY_CATCH
	if (IsWindow(m_owner->m_hWnd))
		m_owner->m_ui.OnDisplayModeChanged(mode);
CATCH_LOG()
}

void CRCViewerImpl::SetRemoteDesktopSize(const int width, const int height)
{
TRY_CATCH
	if (IsWindow(m_owner->m_hWnd))
	{
		RECT rc;
		GetClientRect(m_owner->m_hWnd, &rc);
		LPARAM lParam = MAKELONG(rc.right - rc.left, rc.bottom -rc.top);
		PostMessage(m_owner->m_hWnd, WM_SIZE, 0, lParam);
	}
CATCH_LOG()
}

void CRCViewerImpl::OnMinimize()
{
TRY_CATCH
	CRCViewer::OnMinimize();
	m_minimizeSignal();
CATCH_LOG()
}

void CRCViewerImpl::OnRestore()
{
TRY_CATCH
	RestorePrevDisplayMode();
	///CRCViewer::OnRestore();
CATCH_LOG()
}

void CRCViewerImpl::SubscribeEventsListener(boost::function<void ()> eventListener)
{
TRY_CATCH
	m_minimizeSignal.connect(eventListener);
CATCH_THROW()
}