/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  COpenPortTest.h
///
///  Declares COpenPortTest class, test for open port service
///
///  @author Dmitry Netrebenko @date 06.03.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CAbstractTest.h"
#include "CSettings.h"
#include <boost/shared_ptr.hpp>
#include <AidLib/CCritSection/CCritSectionObject.h>
#include <boost/thread.hpp>
#include "SOpenPortTestStatistic.h"

/// COpenPortTest class, test for open port service
class COpenPortTest
	:	public CAbstractTest
{
private:
/// Prevents making copies of COpenPortTest objects
	COpenPortTest(const COpenPortTest&);
	COpenPortTest& operator=(const COpenPortTest&);
public:
/// Constructor
/// @param settings - pointer to settings
	COpenPortTest(boost::shared_ptr<CSettings> settings);
/// Destructor
	~COpenPortTest();
protected:
/// Starts test thread
/// @param threadInfo - pointer to thread info
	virtual void RunTestThread(boost::shared_ptr<SThreadInfo> threadInfo);
/// Prepare test result
	virtual void PrepareResults();
/// Virtual methods for running routines befor and after test
	virtual void OnTestStarted();
	virtual void OnTestStopped();
private:
/// Stub thread
	boost::shared_ptr<boost::thread>	m_stubThread;
/// Is stub thread terminated
	bool								m_stubTerminated;
/// Test statistic information
	SOpenPortTestStatistic				m_statistic;
	CCritSectionSimpleObject			m_section;
	void SafeAdd64(__int64* data, __int64 value);
/// Entry point for stub
	void StubEntryPoint();
};
