/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CChildWatcher.cpp
///
///  Implements CChildWatcher class, responsible for terminating child processes
///
///  @author Dmitry Netrebenko @date 16.04.2008
///
////////////////////////////////////////////////////////////////////////

#include "CChildWatcher.h"
#include <AidLib/CException/CException.h>
#include <AidLib/Utils/Utils.h>

CChildWatcher::CChildWatcher()
	:	m_terminationEnabled(false)
	,	m_started(false)
{
TRY_CATCH
CATCH_THROW()
}

CChildWatcher::~CChildWatcher()
{
TRY_CATCH
	/// Terminate thread
	Terminate();
	/// Wait for thread termination
	WaitForSingleObject(hTerminatedEvent.get(), INFINITE);
CATCH_LOG()
}

void CChildWatcher::Execute(void *Params)
{
TRY_CATCH
	m_started = true;
	DWORD processId = GetCurrentProcessId();
	DWORD count = 0;
	while(!Terminated())
	{
		if(m_terminationEnabled)
		{
			DWORD processes = TerminateChildProcesses(processId, m_exceptionProcesses);
			if(processes)
				count = 0;
			else
			{
				count++;
				if(count == TERMINATE_TRIES)
				{
					m_terminationEnabled = false;
					count = 0;
				}
			}
		}
		for(DWORD i = 0; i < TERMINATE_WAIT_TIMEOUT; i += TERMINATE_WAIT_STEP)
		{
			if(Terminated())
				break;
			Sleep(TERMINATE_WAIT_STEP);
		}
	}
CATCH_LOG()
}

void CChildWatcher::EnableTermination(const bool enabled)
{
TRY_CATCH
	m_terminationEnabled = enabled;
	if(m_terminationEnabled && !m_started)
		Start();
CATCH_THROW()
}

void CChildWatcher::AddExceptionProcess(const DWORD processId)
{
TRY_CATCH
	m_exceptionProcesses.insert(processId);
CATCH_THROW()
}

