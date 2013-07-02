/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFrameWaiter.cpp
///
///  Implements CFrameWaiter class, responsible for waiting of changing frames 
///
///  @author Dmitry Netrebenko @date 13.06.2007
///
////////////////////////////////////////////////////////////////////////

#include "CFrameWaiter.h"
#include <AidLib/CException/CException.h>

CFrameWaiter::CFrameWaiter(WaiterEvent frameEventHandler)
	:	CThread()
	,	m_frameEventHandler(frameEventHandler)
{
TRY_CATCH

	/// Create event object
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
		/// Wait for changing frame
		if(WAIT_TIMEOUT == WaitForSingleObject(m_frameEvent.get(), 10000))
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

SPHandle CFrameWaiter::GetFrameEvent() const
{
TRY_CATCH

	return m_frameEvent;

CATCH_THROW()
}
