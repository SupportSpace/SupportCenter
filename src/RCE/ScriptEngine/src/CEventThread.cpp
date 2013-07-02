/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CEventThread.cpp
///
///  Implements CEventThread class, responsible for event calling thread
///
///  @author Dmitry Netrebenko @date 15.01.2008
///
////////////////////////////////////////////////////////////////////////

#include "CEventThread.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CThread/CThreadLS.h>

CEventThread::CEventThread(EventFunc eventFunc, int eventId, unsigned int requestId, bool success, const tstring& errorString)
	:	m_eventFunc(eventFunc)
	,	m_eventId(eventId)
	,	m_requestId(requestId)
	,	m_success(success)
	,	m_errorString(errorString)
{
TRY_CATCH
	/// Check up event function
	if(!m_eventFunc)
		throw MCException(_T("Event function == NULL"));
	/// Create complete event
	m_completeEvent.reset(CreateEvent(NULL, TRUE, FALSE, NULL), CloseHandle);
CATCH_THROW()
}

CEventThread::~CEventThread()
{
TRY_CATCH
	/// Terminate thread
	Terminate();
	/// Wait for thread termination
	WaitForSingleObject(hTerminatedEvent.get(), INFINITE);
CATCH_LOG()
}

void CEventThread::Execute(void *Params)
{
	::CoInitialize(NULL);
TRY_CATCH
	SET_THREAD_LS;
	/// Call event function
	TRY_CATCH
		m_eventFunc(m_eventId, m_requestId, m_success, m_errorString);
	CATCH_LOG()
	/// Set complete event
	SetEvent(m_completeEvent.get());
CATCH_LOG()
	::CoUninitialize();
}

HANDLE CEventThread::GetCompleteEvent()
{
TRY_CATCH
	return m_completeEvent.get();
CATCH_THROW()
}
