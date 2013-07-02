/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CEventThreadsManager.cpp
///
///  Implements CEventThreadsManager class, responsible for management of 
///    event threads
///
///  @author Dmitry Netrebenko @date 15.01.2008
///
////////////////////////////////////////////////////////////////////////

#include "CEventThreadsManager.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CThread/CThreadLS.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <boost/scoped_array.hpp>

CEventThreadsManager::CEventThreadsManager()
{
TRY_CATCH
	/// Create event
	m_changeEvent.reset(CreateEvent(NULL, FALSE, FALSE, NULL), CloseHandle);
CATCH_THROW()
}

CEventThreadsManager::~CEventThreadsManager()
{
TRY_CATCH
	/// Terminate thread
	Terminate();
	/// Set changes event
	SetEvent(m_changeEvent.get());
	/// Wait for thread termination
	WaitForSingleObject(hTerminatedEvent.get(), INFINITE);
CATCH_LOG()
}

void CEventThreadsManager::Execute(void *Params)
{
TRY_CATCH
	SET_THREAD_LS;
	while(!Terminated())
	{
		/// Enter critical section
		CCritSection section(&m_section);
		/// Get count of registered threads
		DWORD count = static_cast<DWORD>(m_eventThreads.size() + 1);
		/// Allocate memory for array with handles
		boost::scoped_array<HANDLE> waitingEvents(new HANDLE[count]);
		/// Set first event
		waitingEvents[0] = m_changeEvent.get();
		/// Get threads and store complete events in array
		unsigned int idx = 1;
		for(std::map< HANDLE, boost::shared_ptr<CEventThread> >::iterator index = m_eventThreads.begin();
			index != m_eventThreads.end();
			++index, ++idx)
		{
			waitingEvents[idx] = index->first;
		}
		/// Leave critical section
		section.Unlock();
		/// Waiting for something happened
		DWORD result = WaitForMultipleObjects(count, waitingEvents.get(), FALSE, INFINITE);
		switch(result)
		{
		case WAIT_FAILED:	/// WaitForMultipleObjects failed
			throw MCException_Win("Failed to WaitForMultipleObjects");
		case WAIT_OBJECT_0:	/// Timeout happened of internal event signaled
		case WAIT_TIMEOUT:
			continue;
		default:			/// Some event signaled
			if((result - WAIT_OBJECT_0 < count) && (result - WAIT_OBJECT_0 >= 0))
			{
				/// Get signaled handle
				HANDLE eventHandle = waitingEvents[result - WAIT_OBJECT_0];
				/// Enter critical section
				section.Lock();
				/// Find event thread by event handle
				std::map< HANDLE, boost::shared_ptr<CEventThread> >::iterator index = m_eventThreads.find(eventHandle);
				if(index != m_eventThreads.end())
				{
					/// Destroy thread and remove it from map
					m_eventThreads.erase(index);
				}
				else
				{
					Log.Add(_WARNING_,_T("Cannot find event thread in map."));
				}
				/// Leave critical section
				section.Unlock();
			} 
			else
			{
				Log.WinError(_WARNING_,_T("Unexpected result %d is received from WaitForMultipleObjects"), result);
			}
		}
	}
CATCH_LOG()
}

void CEventThreadsManager::AddEvent(EventFunc eventFunc, int eventId, unsigned int requestId, bool success, const tstring& errorString)
{
TRY_CATCH
	/// Create event thread
	boost::shared_ptr<CEventThread> thread(new CEventThread(eventFunc, eventId, requestId, success, errorString));
	/// Get handle of complete event
	HANDLE eventHandle = thread->GetCompleteEvent();
	/// Enter critical section
	CCritSection section(&m_section);
	/// Check up count of waiting threads
	if(m_eventThreads.size() >= MAXIMUM_WAIT_OBJECTS - 1)
		throw MCException(_T("Cannot add thread to waiting map. Maximal count of waiting objects exceeded."));
	/// Add thread to map
	m_eventThreads[eventHandle] = thread;
	/// Set event
	SetEvent(m_changeEvent.get());
	/// Start thread
	thread->Start();
CATCH_THROW()
}
