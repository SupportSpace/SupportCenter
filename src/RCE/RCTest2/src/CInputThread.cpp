#include "CInputThread.h"
#include <AidLib/CException/CException.h>

CInputThread::CInputThread()
	:	m_driverHandle(NULL)
	,	m_window(NULL)
	,	m_msg(0)
{
TRY_CATCH
CATCH_THROW()
}

CInputThread::~CInputThread()
{
TRY_CATCH
CATCH_LOG()
}

void CInputThread::Execute(void *Params)
{
TRY_CATCH
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	while(!Terminated())
		GetAndProcessInput();
CATCH_LOG()
}

void CInputThread::Init(HANDLE driverHandle, HWND window, unsigned int msg)
{
TRY_CATCH
	m_driverHandle = driverHandle;
	m_window = window;
	m_msg = msg;
CATCH_THROW()
}

