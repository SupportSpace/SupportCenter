/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  C_IRCViewerAXCtrlEvents.cpp
///
///  C_IRCViewerAXCtrlEvents object implementation. The object is events receiever of RCViewerAXCtrl events
///
///  @author Kirill Solovyov @date 05.10.2007
///
////////////////////////////////////////////////////////////////////////
// C_IRCViewerAXCtrlEvents.cpp : Implementation of C_IRCViewerAXCtrlEvents

#include "stdafx.h"
#include "C_IRCViewerAXCtrlEvents.h"


// C_IRCViewerAXCtrlEvents
C_IRCViewerAXCtrlEvents::C_IRCViewerAXCtrlEvents():m_owner(NULL)
{
}

HRESULT C_IRCViewerAXCtrlEvents::FinalConstruct()
{
	return S_OK;
}

void C_IRCViewerAXCtrlEvents::FinalRelease()
{
}

STDMETHODIMP C_IRCViewerAXCtrlEvents::NotifySessionStop(long reasonCode)
{
	if(m_owner)
	{
		CComPtr<IRCViewerAXCtrl> viewer;
		m_owner->m_feature.QueryControl(&viewer);
			if(reasonCode==0)
				viewer->SetUIStatus(6,CComBSTR(L"Desktop Sharing - Off"));
			else
				viewer->SetUIStatus(9,CComBSTR((tstring(_T("Connecting failed with reason = 0x"))+i2tstring(reasonCode,16)).c_str()));
		return S_OK;
	}
	else
		return E_NOTIMPL;
}
STDMETHODIMP C_IRCViewerAXCtrlEvents::NotifySessionStart(long connectType)
{
	if(m_owner)
	{
		CComPtr<IRCViewerAXCtrl> viewer;
		m_owner->m_feature.QueryControl(&viewer);
		//_this.SetDisplayMode(_this.m_displayMode);//set display mode
		//_this.SetCaptureAlphaBlend(_this.m_captureAlphaBlend);
		//if(_this.m_access!=2)//not a full control TODO: make safe machanism set access mode on host side in this. now not always. inner viewer setaccess carry out after host do this.
		//	_this.SetAccessMode(_this.m_access);//set session mode
		//_this.m_session.Activate(true);
		
		//crutch for CRCViewer window be stand on top
		RECT rect;
		BOOL handled=true;
		m_owner->m_feature.GetWindowRect(&rect);
		m_owner->OnSize(WM_SIZE,NULL,MAKELPARAM(rect.right-rect.left-1,rect.bottom-rect.top-1),handled);
		m_owner->OnSize(WM_SIZE,NULL,MAKELPARAM(rect.right-rect.left,rect.bottom-rect.top),handled);
		//crutch
		viewer->SetUIStatus(8,CComBSTR((tstring(_T("Desktop Sharing - On type=0x"))+i2tstring(connectType,16)).c_str()));
		return S_OK;
	}
	else
		return E_NOTIMPL;
}
STDMETHODIMP C_IRCViewerAXCtrlEvents::NotifyConnecting(long percentCompleted, BSTR status)
{
	if(m_owner)
	{
		CComPtr<IRCViewerAXCtrl> viewer;
		m_owner->m_feature.QueryControl(&viewer);
		viewer->SetUIStatus(7,CComBSTR((tstring(_T(" "))+i2tstring(percentCompleted,10)+tstring(_T(" "))).c_str())+status);
		return S_OK;
	}
	else
		return E_NOTIMPL;
}
STDMETHODIMP C_IRCViewerAXCtrlEvents::NotifyUIEvent(long eventType, long param)
{
		//m_owner->Request(CComBSTR(L"C_IRCViewerAXCtrlEvents::NotifyUIEvent(long eventType, long param)"),eventType);
	if(m_owner)
	{
		CComPtr<IRCViewerAXCtrl> viewer;
		m_owner->m_feature.QueryControl(&viewer);
		if(eventType==0)
			{
				//_this.m_access=param;
				//viewer->Start(m_owner->m_featureParams.user,m_owner->m_featureParams.passwd,m_owner->m_featureParams.remoteUser,m_owner->m_featureParams.relaySrv,30000);
				m_owner->SendRqstRemoteControl(ERCAccessMode(param));
			}
			else if(eventType==1)
			{
				viewer->Stop();
			}
			else if(eventType==2)
			{
				m_owner->SendRqstAccessMode(ERCAccessMode(param));
				//switch(param)
				//{
				//	case 0:	//view only
				//		viewer->SetSessionMode(0,true);
				//		viewer->SetSessionMode(1,false);
				//		break;
				//	case 1:	//view only + visual pointer
				//		viewer->SetSessionMode(0,true);
				//		viewer->SetSessionMode(1,true);
				//		break;
				//	case 2:	//full control
				//		viewer->SetSessionMode(0,false);
				//}
			}
		return S_OK;
	}
	else
		return E_NOTIMPL;
}




//C_IRCViewerAXCtrlEvents1::C_IRCViewerAXCtrlEvents1()
//{
//}
//void C_IRCViewerAXCtrlEvents1::EventAdvise(IUnknown* source)
//{
//	__hook(&_IRCViewerAXCtrlEvents::NotifyUIEvent,source,&C_IRCViewerAXCtrlEvents1::NotifyUIEvent);
//}
//void C_IRCViewerAXCtrlEvents1::EventUnadvise(IUnknown* source)
//{
//	__unhook(&_IRCViewerAXCtrlEvents::NotifyUIEvent,source,&C_IRCViewerAXCtrlEvents1::NotifyUIEvent);
//}
//
//STDMETHODIMP C_IRCViewerAXCtrlEvents1::NotifySessionStop(long reasonCode)
//{
//	return E_NOTIMPL;
//}
//STDMETHODIMP C_IRCViewerAXCtrlEvents1::NotifySessionStart(long connectType)
//{
//	return E_NOTIMPL;
//}
//STDMETHODIMP C_IRCViewerAXCtrlEvents1::NotifyConnecting(long percentCompleted, BSTR status)
//{
//	return E_NOTIMPL;
//}
//STDMETHODIMP C_IRCViewerAXCtrlEvents1::NotifyUIEvent(long eventType, long param)
//{
//	::MessageBox(NULL,_T("C_IRCViewerAXCtrlEvents1::NotifyUIEvent(long eventType, long param)"),NULL,0);
//	return E_NOTIMPL;
//}
