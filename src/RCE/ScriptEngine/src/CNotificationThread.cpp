/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNotificationThread.cpp
///
///  Implements CNotificationThread class, responsible for thread which will send
///    notifications to remote side through infrastructure
///
///  @author Dmitry Netrebenko @date 12.03.2008
///
////////////////////////////////////////////////////////////////////////

#include "CNotificationThread.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CThread/CThreadLS.h>
#include "stdafx.h"

CNotificationThread::CNotificationThread()
	:	CThread(NULL, true)
	,	m_scriptName(_T(""))
{
TRY_CATCH

CATCH_THROW()
}

CNotificationThread::~CNotificationThread()
{
TRY_CATCH
	Terminate();
	/// Wait for thread termination
	WaitForSingleObject(hTerminatedEvent.get(), INFINITE);
	m_brokerEvents.Revoke();
CATCH_LOG()
}

void CNotificationThread::Execute(void *Params)
{
	CoInitializeEx(NULL,COINIT_MULTITHREADED);
	SET_THREAD_LS;
TRY_CATCH
	MSG msg;
	HRESULT result;
	if(!m_brokerEvents.m_dwCookie)
		throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");

	CComPtr<_IBrokerClientEvents> brokerEvents;
	if(S_OK != (result = m_brokerEvents.CopyTo(&brokerEvents)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")), result);

	while(!Terminated() && GetMessage(&msg, NULL, 0, 0))
	{
		switch(msg.message)
		{
			case WM_USER:
				{
					EServiceStateMessages state = static_cast<EServiceStateMessages>(msg.lParam);
					tstring params(_T(""));
					switch (state)
					{
					case ESM_SERVICE_ACTIVITY_CHANGED:
						{
							boost::shared_ptr<TCHAR> str;
							str.reset(reinterpret_cast<PTCHAR>(msg.wParam), free);
							tstring activity(str.get());
							params = activity.empty()?m_scriptName:Format(m_scriptName + tstring(_T(";;%s")),activity.c_str());
						}
						break;
					default:
						params = m_scriptName;
					}
					if(NULL == brokerEvents.p)
						MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("_IBrokerClientEvents has not marshaled")),result));

					if(ESM_SERVICE_STARTED == state)
						brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_SET_SCRIPTNAME,0,CComBSTR(m_scriptName.c_str()));
					else
						brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_AUTOSET),/*BSVCIDPDV_JS*/BSVCIDPDV_AUTOSET,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_SRV_STATE_CHANGED,state,CComBSTR(params.c_str()));
				}
				break;
			default:
				Log.Add(_WARNING_,_T("Unexpected message %d received"), msg.message);
		}
	}
CATCH_LOG()
	CoUninitialize();
}

void CNotificationThread::Init(const tstring& scriptName, CComGITPtr<_IBrokerClientEvents> brokerEvents)
{
TRY_CATCH
	if(scriptName.empty())
		throw MCException(_T("Script name is empty."));
	if(!brokerEvents.m_dwCookie)
		throw MCException(_T("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first"));
	m_scriptName = scriptName;
	m_brokerEvents = brokerEvents;
CATCH_THROW()
}

void CNotificationThread::NotifyActivity(const tstring& activity)
{
TRY_CATCH
	PTCHAR str = _tcsdup(activity.c_str());
	if(FALSE == PostThreadMessage(GetTid(),WM_USER, reinterpret_cast<WPARAM>(str), ESM_SERVICE_ACTIVITY_CHANGED))
		free(str);
CATCH_THROW()
}

void CNotificationThread::NotifyServiceStart()
{
TRY_CATCH
	PostThreadMessage(GetTid(),WM_USER, 0, ESM_SERVICE_STARTED);
CATCH_THROW()
}

void CNotificationThread::NotifyServiceStop()
{
TRY_CATCH
	PostThreadMessage(GetTid(),WM_USER, 0, ESM_SERVICE_STOPPED);
CATCH_THROW()
}
