/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFrameWaiter.cpp
///
///  Implements CFrameWaiter class, responsible for waiting of changing frames 
///
///  @author Dmitry Netrebenko @date 11.06.2007
///
////////////////////////////////////////////////////////////////////////

#include "CFrameWaiter.h"
#include <AidLib/CException/CException.h>

CFrameWaiter::CFrameWaiter(WaiterEvent sessionEventHandler, WaiterEvent frameEventHandler)
	:	CThread()
	,	m_sessionEventHandler(sessionEventHandler)
	,	m_frameEventHandler(frameEventHandler)
{
TRY_CATCH

	/// Create event objects
	m_sessionEvent.reset( CreateEvent(NULL, TRUE, FALSE, NULL), CloseHandle );
	m_frameEvent.reset( CreateEvent(NULL, TRUE, FALSE, NULL), CloseHandle );

	/// Start thread
	Start();

CATCH_THROW()
}

CFrameWaiter::~CFrameWaiter()
{
TRY_CATCH

	/// Terminate thread
	Terminate();

CATCH_LOG()
}

void CFrameWaiter::Execute(void* Params)
{
TRY_CATCH

	while(!Terminated())
	{
		/// Wait for activating of session
		if(WAIT_TIMEOUT == WaitForSingleObject(m_sessionEvent.get(), 30000))
		{
			/// Raise timeout event
			if(m_sessionEventHandler)
				m_sessionEventHandler(NULL);
			break;
		}
		/// Check termination
		if(Terminated())
			break;
		/// Wait for changing frame
		if(WAIT_TIMEOUT == WaitForSingleObject(m_frameEvent.get(), 5000))
		{
			/// Raise timeout event
			if(m_frameEventHandler)
				m_frameEventHandler(NULL);
			break;
		}
		/// Reset event
		if(!Terminated())
			ResetEvent(m_frameEvent.get());
	}

CATCH_LOG()
}

SPHandle CFrameWaiter::GetSessionEvent() const
{
TRY_CATCH

	return m_sessionEvent;

CATCH_THROW()
}

SPHandle CFrameWaiter::GetFrameEvent() const
{
TRY_CATCH

	return m_frameEvent;

CATCH_THROW()
}

