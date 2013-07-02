/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CResponseWaitThread.cpp
///
///  Implements CResponseWaitThread class, responsible for waiting of response
///
///  @author Dmitry Netrebenko @date 06.12.2007
///
////////////////////////////////////////////////////////////////////////

#include "CResponseWaitThread.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CCritSection/CCritSection.h>

CResponseWaitThread::CResponseWaitThread(DWORD timeout, boost::function<void(void)> timeoutCallback)
	:	CThread()
	,	m_timeout(timeout)
	,	m_timeoutCallback(timeoutCallback)
{
TRY_CATCH
	m_event.reset(CreateEvent(NULL, TRUE, FALSE, NULL), CloseHandle);
CATCH_THROW()
}

CResponseWaitThread::~CResponseWaitThread()
{
TRY_CATCH
	Terminate();
	SetEvent(m_event.get());
CATCH_LOG()
}

void CResponseWaitThread::Execute(void *Params)
{
TRY_CATCH
	while(!Terminated())
	{
		DWORD res = WaitForSingleObject(m_event.get(), m_timeout);
		if(Terminated())
			break;
		if((WAIT_TIMEOUT == res) && m_timeoutCallback)
			m_timeoutCallback();
	}
CATCH_LOG()
}

unsigned int CResponseWaitThread::HandleRequest(unsigned int requestId)
{
TRY_CATCH
	CCritSection section(&m_section);
	std::map<unsigned int,tstring>::iterator index = m_requests.find(requestId);
	if(index != m_requests.end())
	{
		m_requests.erase(requestId);
	}
	else
	{
		Log.Add(_WARNING_, _T("Request (%d) is not found."), requestId);
	}
	unsigned int count = static_cast<unsigned int>(m_requests.size());
	if(!count)
	{
		Terminate();
		SetEvent(m_event.get());
	}
	return count;
CATCH_THROW()
}

void CResponseWaitThread::AddRequest(unsigned int requestId, const tstring& data)
{
TRY_CATCH
	CCritSection section(&m_section);
	m_requests[requestId] = data;
CATCH_THROW()
}

