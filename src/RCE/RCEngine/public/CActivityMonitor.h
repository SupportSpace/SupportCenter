#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CActivityMonitor.h
///
///  classs for monitoring remote session activity
///
///  @author "Archer Software" Sogin M. @date 06.03.2008
///
////////////////////////////////////////////////////////////////////////
#include <AidLib/CException/CException.h>
#include <AidLib/CTime/CTime.h>
#include <AidLib/CThread/CThread.h>
#include <AidLib/CScopedTracker/CScopedTracker.h>
#include <boost/function.hpp>

/// Timeout, while going to idle state
#define ACTIVITY_IDLE_TIMEOUT 3000

/// classs for monitoring remote session activity
class CActivityMonitor : protected CThread
{
public:
	/// Possible activities
	typedef enum _EActivityType
	{
		EAT_IDLE,
		EAT_KEYBOARD,
		EAT_MOUSE
	} EActivityType;

	/// Activity has changed delegate type
	typedef boost::function<void(const EActivityType)> ActivityChangedHandler;

	/// ctor
	CActivityMonitor();

	/// Set new handler for activity changed event
	void SetActivityChangedHandler(ActivityChangedHandler activityChangedHandler);

	/// Registers new activity
	inline void SetNewActivity(const EActivityType activity)
	{
		if (activity != m_currentActivity)
		{
			m_currentActivity = activity;
			SetEvent(m_sendNewActivityEvent);
		}
		m_lastActivityTime.GetNow();
	}

protected:
	CScopedTracker<HANDLE> m_destroyingEvent;
	CScopedTracker<HANDLE> m_sendNewActivityEvent;

	/// current activity
	EActivityType m_currentActivity;

	/// last time, when activity was registered
	cDate m_lastActivityTime;

	///  Activity has changed delegate
	ActivityChangedHandler m_activityChangedHandler;

	/// Thread entry point
	virtual void Execute(void*);
};