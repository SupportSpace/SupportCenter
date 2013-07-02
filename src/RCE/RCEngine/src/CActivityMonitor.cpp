/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CActivityMonitor.cpp
///
///  classs for monitoring remote session activity
///
///  @author "Archer Software" Sogin M. @date 06.03.2008
///
////////////////////////////////////////////////////////////////////////
#include <RCEngine/CActivityMonitor.h>

CActivityMonitor::CActivityMonitor()
	:	m_currentActivity(EAT_IDLE),
		m_lastActivityTime(cDate().GetNow()),
		m_activityChangedHandler(NULL)

{
TRY_CATCH
	
	m_destroyingEvent.reset(CreateEvent(NULL, TRUE, FALSE, NULL),CloseHandle);
	if (NULL == m_destroyingEvent)
		throw MCException_Win("Failed to CreateEvent ");

	m_sendNewActivityEvent.reset(CreateEvent(NULL, FALSE, FALSE, NULL),CloseHandle);
	if (NULL == m_sendNewActivityEvent)
		throw MCException_Win("Failed to CreateEvent ");

	Start();

CATCH_THROW()
}

void CActivityMonitor::SetActivityChangedHandler(ActivityChangedHandler activityChangedHandler)
{
TRY_CATCH
	m_activityChangedHandler = activityChangedHandler;
CATCH_THROW()
}

void CActivityMonitor::Execute(void*)
{
TRY_CATCH
	DWORD result;
	HANDLE handles[2] = {m_destroyingEvent, m_sendNewActivityEvent};
	while(!Terminated())
	{
		result = WaitForMultipleObjects(2, handles, FALSE, ACTIVITY_IDLE_TIMEOUT/2);
		switch(result)
		{
			case WAIT_OBJECT_0:
				return;
			case WAIT_OBJECT_0 + 1:
					if (NULL != m_activityChangedHandler)
						m_activityChangedHandler(m_currentActivity);
				break;
			case WAIT_TIMEOUT:
				if (EAT_IDLE != m_currentActivity && 
					cDate(m_lastActivityTime).AddMilliSecs(ACTIVITY_IDLE_TIMEOUT) < cDate().GetNow())
				{
					m_lastActivityTime.GetNow();
					m_currentActivity = EAT_IDLE;
					if (NULL != m_activityChangedHandler)
						m_activityChangedHandler(m_currentActivity);
				}
				break;
			default:
				Log.WinError(_ERROR_,_T("WaitForSingleObject returned unexpected %X. Exiting activity monitoring thread "),result);
				return;
		}
	}
CATCH_LOG()
}
