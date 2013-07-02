//===========================================================================
// Archer Software.
//                                 CThread.cpp
//
//---------------------------------------------------------------------------
// base class for threads
//---------------------------------------------------------------------------
//
// Version : 01.00
// By      : Max Sogin
// Date    : 7/19/05 05:01:12 PM
//===========================================================================
#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif //_DEBUG

#include <AidLib/CThread/CThread.h>
#include <process.h>
#include <AidLib/CThread/CThreadLS.h>

//===========================================================================
//
// @{FUN}                          Starter()
//
//---------------------------------------------------------------------------
// Effects		: Real entry point to thread
// Arguments	: void *ThreadObject must be pointer to CThread based object
// Errors		: Errors logged by means of cLog class
//===========================================================================	
unsigned int __stdcall CThread::Starter(void *_ThreadObject)
{
	//CThread* ThreadObject = dynamic_cast<CThread*>((CThread*)_ThreadObject);
	CThread* ThreadObject = static_cast<CThread*>(_ThreadObject);
	SET_THREAD_LS;
TRY_CATCH
	if (!ThreadObject) throw MCException("Bad argument void *_ThreadObject");

	//Creating message queue for thread
	if (ThreadObject->CreateMessageQueue)
	{
		MSG msg;
		PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	}
	
	//Getting thread id
	ThreadObject->Tid = GetCurrentThreadId();

	//Setting event, indicating, that we have started
	ResetEvent(ThreadObject->hTerminatedEvent.get());
	SetEvent(ThreadObject->hStartedEvent.get());
	
	//Starting thread
	ThreadObject->Execute(ThreadObject->Param);
CATCH_LOG("CThread::Starter");
	
	//Here we do all terminate work	
TRY_CATCH
	//Setting events
	ResetEvent(ThreadObject->hStartedEvent.get());
	ThreadObject->State=_TERMINATED;
	SetEvent(ThreadObject->hTerminatedEvent.get());
CATCH_LOG("CThread::Starter");

	//Explicitly to terminating a thread
	//NOTE Very bad practic to call _endthread explicitly because this function will called by CRT after thread finishing
	//_endthread();
	return 0;
}


//===========================================================================
//
// @{FUN}                          CThread()
//
//---------------------------------------------------------------------------
// Effects		: Creates thread object
// Arguments	: void *Param - eny param, to pass in entry point
// Errors		: On errors exception thrown
//===========================================================================	
CThread::CThread(void *_Param, const bool createMessageQueue) 
	:	State(_TERMINATED)
	,	dwTerminateTimeout(INFINITE)
	,	Param(_Param)
	,	CreateMessageQueue(createMessageQueue)
{
TRY_CATCH
	hStartedEvent.reset(CreateEvent(NULL,TRUE,FALSE,NULL), CloseHandle);
	hTerminatedEvent.reset(CreateEvent(NULL,TRUE,TRUE,NULL), CloseHandle);
CATCH_THROW("CThread::CThread")
}


//===========================================================================
//
// @{FUN}                          Start()
//
//---------------------------------------------------------------------------
// Effects		: Starts a thread. If it was paused resumes it
// Errors		: On errors exception thrown
//===========================================================================	
void CThread::Start()
{
TRY_CATCH
	switch(State)
	{
		case _TERMINATED:
			unsigned threadId;
			hThread.reset();
			{
				HANDLE hNewThread=(HANDLE)_beginthreadex(NULL,0,&Starter,this,0,&threadId);
				if (hNewThread == (HANDLE)-1)
				{
					throw MCException_Win("Failed to start _beginthread error");
				}
				hThread.reset(hNewThread, CloseHandle);
			}
			State=_RUNNING;
			WaitForSingleObject(hStartedEvent.get(),DEFAULT_START_TIMEOUT);
			//Should we Handle this ^^^^^^ ????
			break;
		case _PAUSED:
			if (0xFFFFFFFF == ResumeThread(hThread.get()))
			{
				throw MCException_Win("Failed to start ResumeThread error");
			}
			State=_RUNNING;
			break;
		default:
			throw MCException("Cant start thread with such state");
	}
CATCH_THROW("CThread::Start")
}


//===========================================================================
//
// @{FUN}                          Stop()
//
//---------------------------------------------------------------------------
// Effects		: Stops a thread. 
//	Arguments	: bool Forced if true, than do not wait till thread stops
//	Arguments	: DWORD TimeOut=INFINITE time to wait, while thread stops
//				:	default - INFINITE
// Errors		: On errors exception thrown
//===========================================================================	
void CThread::Stop(bool Forced, DWORD TimeOut) 
{
TRY_CATCH
	switch(State)
	{
		case _PAUSED:
			Start();
		case _TERMINATING:
		case _RUNNING:
			if (Forced)
			{
				//Terminating thread in rude way
				//if (!TerminateThread(hThread,0))
				//{
				//	throw MCException_Win("Failed to terminate thred");
				//}
				Log.Add(_WARNING_,_T("CThread::Stop. Terminated forced was called. Doing nothing in this implementation"));
				//Setting evens by ourself
				SetEvent(hTerminatedEvent.get());
				ResetEvent(hStartedEvent.get());

				//Closing thread's handle
				hThread.reset();
				//Terminated successfully
				State=_TERMINATED;
				break;
			} else
			{
				State=_TERMINATED;
				if (WAIT_OBJECT_0 != WaitForSingleObject(hTerminatedEvent.get(),TimeOut))
				{
					//Wait for thread terminate themself failed
					Log.Add(_WARNING_,_T("CThread::Stop: Wait for thread terminating failed. Terminate forced"));
					Stop(true);
					break;
				} else
				{
					//Terminated successfully
					//Do not need to close thread's handle
					//As we start if with _beginthread
					State=_TERMINATED;
					break;
				}
			}
	}
CATCH_THROW("CThread::Stop")
}


//===========================================================================
//
// @{FUN}                          Suspend()
//
//---------------------------------------------------------------------------
// Effects		: Pauses thread execution
// Errors		: On errors exception thrown
//===========================================================================	
void CThread::Suspend() 
{
TRY_CATCH
	// Try to suspend a thread
	if ( 0xFFFFFFFF == SuspendThread(hThread.get()) )
	{
		throw MCException_Win("Error while suspending thread");
	}
	State=_PAUSED;
CATCH_THROW("CThread::Suspend")
}


//===========================================================================
//
// @{FUN}                               GetTid()
//
//---------------------------------------------------------------------------
// Effects 		: Returns thread identifier
// Return type	: unsigned int 
// Errors		: If can't return thread id 0 returned
//===========================================================================
unsigned int CThread::GetTid() const 
{
TRY_CATCH
	return Tid;
CATCH_THROW("CThread::GetTid")
}