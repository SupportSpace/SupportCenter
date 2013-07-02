/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAbstractTest.h
///
///  Declares CAbstractTest class, abstract test
///
///  @author Dmitry Netrebenko @date 09.08.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "STestParams.h"
#include "SConnectParams.h"
#include "TestEvents.h"
#include <boost/shared_ptr.hpp>
#include "CMessenger.h"
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <AidLib/Strings/tstring.h>

///  CAbstractTest class, abstract test
class CAbstractTest
{
private:
/// Prevents making copies of CAbstractTest objects.
	CAbstractTest(const CAbstractTest&);
	CAbstractTest& operator=(const CAbstractTest&);

public:
/// Constructor
	CAbstractTest();

/// Destructor
	virtual ~CAbstractTest();

/// Initializes internal data
/// @param testParams - structure with test parameters
/// @param connectParams - structure with connection parameters
	virtual void Init(const STestParams& testParams, const SConnectParams& connectParams);

/// Starts test
	void Start();

/// Stops test
	virtual void Stop();

/// Sets up "OnComplete" event handler
/// @param handler - event handler
	void SetOnCompleteEvent(OnCompleteEvent handler);

/// Sets up "OnProgress" event handler
/// @param handler - event handler
	void SetOnProgressEvent(OnProgressEvent handler);

/// Returns test parameters structure
	STestParams& GetTestParams();

/// Returns connection parameters structure
	SConnectParams& GetConnectParams();

/// Returns state
	bool Terminated() const;

private:
/// On test completed event
	OnCompleteEvent		m_onComplete;

/// On progress event
	OnProgressEvent		m_onProgress;

/// Separated thread to running test
	boost::shared_ptr<boost::thread>	m_thread;

protected:
/// Tests parameters
	STestParams							m_testParams;

/// Connection parameters
	SConnectParams						m_connectParams;

/// Pointer to messenger
	boost::shared_ptr<CMessenger>		m_messenger;

/// Termination flag
	bool								m_terminated;

protected:
/// Raises "OnComplete" event
/// @param successful - test result
	void RaiseOnComplete(bool successful);

/// Raises "OnProgress" event
/// @param percents - completed percentage
	void RaiseOnProgress(unsigned int percents);

/// Executes test routines
	virtual void DoTest() = NULL;

/// "OnMessage" event handler
/// @param msg - received message
	virtual void OnMessageReceived(const tstring& msg) = NULL;

private:
/// Test thread's entry point
	void ThreadEntryPoint();

};
