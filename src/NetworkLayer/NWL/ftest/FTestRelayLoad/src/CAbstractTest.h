/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAbstractTest.h
///
///  Declares CAbstractTest class, asbtract test functionality
///
///  @author Dmitry Netrebenko @date 27.02.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/shared_ptr.hpp>
#include "CSettings.h"
#include <boost/thread.hpp>
#include <boost/threadpool.hpp>
#include <boost/function.hpp>
#include <list>
#include <AidLib/Strings/tstring.h>
#include "SThreadInfo.h"
#include <vector>
#include "STestGeneralStatistic.h"


#pragma comment(lib,"Winmm.lib")

/// CAbstractTest class, asbtract test functionality
class CAbstractTest
{
private:
/// Prevents making copies of CAbstractTest objects
	CAbstractTest(const CAbstractTest&);
	CAbstractTest& operator=(const CAbstractTest&);
public:
/// Constructor
/// @param settings - pointer to settings
	CAbstractTest(boost::shared_ptr<CSettings> settings);
/// Destructor
	~CAbstractTest();
/// Starts test
	void Start();
/// Stops test
	void Stop();
/// Sets up callbacks for test events
	void SetEvents(boost::function<void (void)> onStartEvent, boost::function<void (std::list<tstring>)> onStopEvent, boost::function<void (const tstring&)> onInfoEvent);
protected:
/// Settings
	boost::shared_ptr<CSettings>					m_settings;
/// Test results
	std::list<tstring>								m_results;
/// Abstract method for starting test thread
/// @param threadInfo - pointer to thread info
	virtual void RunTestThread(boost::shared_ptr<SThreadInfo> threadInfo) = NULL;
/// Abstract method to prepare test result
	virtual void PrepareResults() = NULL;
/// Output info message
	void OutputInfo(const tstring& info);
/// Virtual methods for running routines befor and after test
	virtual void OnTestStarted() {};
	virtual void OnTestStopped() {};
private:
/// Pool for test threads
	boost::shared_ptr<boost::threadpool::pool>		m_pool;
/// Thread for starting test
	boost::shared_ptr<boost::thread>				m_starterThread;
/// Functions for test events
	boost::function<void (void)>					m_startEvent;
	boost::function<void (std::list<tstring>)>		m_stopEvent;
	boost::function<void (const tstring&)>			m_infoEvent;
/// Termination flag
	bool											m_terminated;
/// Vector with information of threads
	std::vector< boost::shared_ptr<SThreadInfo> >	m_threadsInfo;
/// Test statistic
	STestGeneralStatistic							m_statistic;
/// Checks up test state and starts test thread
/// @param threadInfo - pointer to thread info
	void StartTestThread(boost::shared_ptr<SThreadInfo> threadInfo);
/// Entry point for starting thread
	void StarterThreadEntryPoint();
/// Creates vector with threads information
	void PrepareThreadsInfo();
/// Adds test general information
	void AddGeneralTestResults();
};
