//===========================================================================
// Archer Software.
//                                   CThread.h
//
//---------------------------------------------------------------------------
// base class for threads
//---------------------------------------------------------------------------
//
// Version : 01.00
// By      : Max Sogin
// Date    : 7/19/05 05:00:56 PM
//===========================================================================

#ifndef	CTHREAD_H
#define	CTHREAD_H

#include <AidLib/CException/CException.h>
#include <AidLib/Logging/cLog.h>
#include <AidLib/AidLib.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/shared_ptr.hpp>

//Default timeout for starting threads
#define DEFAULT_START_TIMEOUT 3000

//===========================================================================
// @{CSEH}
//								CThread
//
//---------------------------------------------------------------------------
// Description		: Base class for thread
//===========================================================================
class AIDLIB_API CThread
{
protected:
	typedef enum _eState
	{
		_TERMINATED,	//Thread isn't running
		_PAUSED,		//Thread is paused
		_RUNNING,		//Thread is running now
		_TERMINATING	//Thread wait for terminate
	} eState;

	//State of a therad;
	eState State;

	//Threads handle
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > hThread;

public:
	//This event get signaled state wright after thread starts
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > hStartedEvent;

	//This event get signaled state wright after thread terminated
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > hTerminatedEvent;

	// Set this value to true if message queue should be created on thread start
	// by default setted to false
	bool CreateMessageQueue;

private:
	// Terminate timeout in milliseconds
	DWORD dwTerminateTimeout;

public:
	//===========================================================================
	//
	// @{FUN}                               SetTerminateTimeout()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Set new termination timeout in milliseconds
	//===========================================================================
	void SetTerminateTimeout(const DWORD dwNewTimeout)
	{
		dwTerminateTimeout = dwNewTimeout;	
	}

	//===========================================================================
	//
	// @{FUN}                               GetTerminateTimeout()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Returns termination timeout in milliseconds
	// Return type	: DWORD 
	//===========================================================================
	DWORD GetTerminateTimeout() const
	{
		return dwTerminateTimeout;
	}

	/// Accessor to termination timeout.
	__declspec( property( get=GetTerminateTimeout, put=SetTerminateTimeout )) DWORD TerminateTimeout;

private:

	//Thread identifier
	unsigned int Tid;
public:

	//===========================================================================
	//
	// @{FUN}                               GetTid()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Returns thread identifier
	// Return type	: unsigned int 
	// Errors		: If can't return thread id 0 returned
	//===========================================================================
	unsigned int GetTid() const ;
private:

	//User defined entry point for thread
	//pfEntryPoint EntryPoint;
	//User defined clas for entry point
	//void* EntryObject;

	//User params to thread
	void *Param;

	//===========================================================================
	//
	// @{FUN}                          Starter()
	//
	//---------------------------------------------------------------------------
	// Effects		: Real entry point to thread
	// Arguments	: void *ThreadObject must be pointer to CThread based object
	// Errors		: Errors logged by means of cLog class
	//===========================================================================	
	unsigned int static __stdcall Starter(void *ThreadObject) ;

public:
	//===========================================================================
	//
	// @{FUN}                          Execute()
	//
	//---------------------------------------------------------------------------
	// Effects		: Default entry point to thread
	// Arguments	: void *Params, that users pass to thread
	// Errors		: On critical errors exception throwed
	// Comment		: throwed exceptions catched and logged by Starter function
	//===========================================================================	
	virtual void Execute(void *Params) {};


	//===========================================================================
	//
	// @{FUN}                          CThread()
	//
	//---------------------------------------------------------------------------
	// Effects		: Creates thread object
	// Arguments	: void *Param - eny param, to pass in entry point
	// Arguments	: const bool createMessageQueue - CreateMessageQueue initial value
	// Errors		: On errors exception thrown
	//===========================================================================	
	CThread(void *_Param = NULL, const bool createMessageQueue = false) ;
	
	//===========================================================================
	//
	// @{FUN}                          ~CThread()
	//
	//---------------------------------------------------------------------------
	// Effects		: Stops (forced) the thread and destroys the object
	// Errors		: On errors exception thrown
	//===========================================================================	
	virtual ~CThread()
	{
	TRY_CATCH
		if(_PAUSED == State)
		{
			// Resume thread
			Start();
		}
		//Stop(false, dwTerminateTimeout);		
		Terminate();
		WaitForSingleObject(hTerminatedEvent.get(),dwTerminateTimeout);
	CATCH_LOG("CThread::~CThread")
	}


	virtual void Terminate()
	{
		switch(State)
		{
		case _PAUSED:
			// Resume thread
			Start();
			break;
		case _TERMINATED:
			return;
		}
		State=_TERMINATING;
	}

	//===========================================================================
	//
	// @{FUN}                          Start()
	//
	//---------------------------------------------------------------------------
	// Effects		: Starts a thread. If it was paused resumes it
	// Errors		: On errors exception thrown
	//===========================================================================	
	void Start() ;

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
	void Stop(bool Forced, DWORD TimeOut=INFINITE) ;

	//===========================================================================
	//
	// @{FUN}                          Suspend()
	//
	//---------------------------------------------------------------------------
	// Effects		: Pauses thread execution
	// Errors		: On errors exception thrown
	//===========================================================================	
	void Suspend() ;

	//===========================================================================
	//
	// @{FUN}                          Terminated()
	//
	//---------------------------------------------------------------------------
	// Return type	: bool
	// Return velue	: True if thread terminated
	//===========================================================================	
	inline bool Terminated() throw()
	{
		return (_TERMINATED == State) || (_TERMINATING == State);
	}

	//===========================================================================
	//
	// @{FUN}                          SetCreateMessageQueue()
	//
	//---------------------------------------------------------------------------
	// Effects		: Set new value for CreateMessageQueue flag
	//	Arguments	: bool createMessageQueue new flag value
	//===========================================================================	
	void SetCreateMessageQueue(const bool createMessageQueue)
	{
		CreateMessageQueue = createMessageQueue;
	}
};

#endif // CTHREAD_H

