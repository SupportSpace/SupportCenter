/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAbstractTest.cpp
///
///  Implements CAbstractTest class, asbtract test functionality
///
///  @author Dmitry Netrebenko @date 27.02.2008
///
////////////////////////////////////////////////////////////////////////

#include "CAbstractTest.h"
#include <AidLib/CException/CException.h>
#include <boost/bind.hpp>
#include <windows.h>
#include "CPerformanceInterval.h"

CAbstractTest::CAbstractTest(boost::shared_ptr<CSettings> settings)
	:	m_settings(settings)
	,	m_terminated(false)
{
TRY_CATCH
	/// Check up settings
	if(!m_settings.get())
		throw MCException(_T("Settings not specified."));
	/// Init pool
	m_pool.reset(new boost::threadpool::pool(m_settings->GetPoolSize()));
CATCH_THROW()
}

CAbstractTest::~CAbstractTest()
{
TRY_CATCH
	/// Terminate all threads
	Stop();
	if(m_starterThread.get())
		m_starterThread->join();
CATCH_LOG()
}

void CAbstractTest::Start()
{
TRY_CATCH
	/// Start thread
	m_starterThread.reset(new boost::thread(boost::bind(&CAbstractTest::StarterThreadEntryPoint, this)));
CATCH_THROW()
}

void CAbstractTest::Stop()
{
TRY_CATCH
	m_terminated = true;
CATCH_THROW()
}

void CAbstractTest::SetEvents(boost::function<void (void)> onStartEvent, boost::function<void (std::list<tstring>)> onStopEvent, boost::function<void (const tstring&)> onInfoEvent)
{
TRY_CATCH
	m_startEvent = onStartEvent;
	m_stopEvent = onStopEvent;
	m_infoEvent = onInfoEvent;
CATCH_THROW()
}

void CAbstractTest::StartTestThread(boost::shared_ptr<SThreadInfo> threadInfo)
{
TRY_CATCH
	if(!threadInfo.get())
		return;
	if(!m_terminated)
		RunTestThread(threadInfo);
	else
	{
		OutputInfo(Format(_T("Connection '%s' cancelled"), threadInfo->m_connectId.c_str()));
		InterlockedIncrement(&m_statistic.m_cancelled);
	}
CATCH_LOG()
}

void CAbstractTest::StarterThreadEntryPoint()
{
TRY_CATCH
	m_terminated = false;
	m_statistic.Clear();
	/// Raise OnStart event
	if(m_startEvent)
		m_startEvent();
	OutputInfo(_T("Starting..."));
	TRY_CATCH
		OnTestStarted();
	CATCH_LOG()
	/// Create vector with threads info
	PrepareThreadsInfo();
	CPerformanceInterval timeInterval;
	timeInterval.Start();
	/// Start test threads
	for(int index = 0; !m_terminated && index < m_settings->GetPeersCount(); ++index)
	{
		boost::shared_ptr<SThreadInfo> threadInfo = m_threadsInfo[index];
		m_pool->schedule(boost::bind(&CAbstractTest::StartTestThread, this, threadInfo));
	}
	/// Wait for all threads
	m_pool->wait();
	timeInterval.Stop();
	m_statistic.m_testTime = timeInterval.GetSecExp3();
	TRY_CATCH
		/// Prepare test results
		PrepareResults();
		/// Adds general test infomation
		AddGeneralTestResults();
	CATCH_LOG()
	TRY_CATCH
		OnTestStopped();
	CATCH_LOG()
	/// Raise "OnStop" event
	if(m_stopEvent)
		m_stopEvent(m_results);
CATCH_LOG()
}

void CAbstractTest::PrepareThreadsInfo()
{
TRY_CATCH
	m_threadsInfo.clear();
	int count = m_settings->GetPeersCount();
	m_threadsInfo.resize(count);

	tstring localPeer;
	tstring remotePeer;
	tstring connect;
	for(int index = 0; index < count; ++index)
	{
		localPeer = Format(_T("peer_%d"), index + 1);
		bool master;
		if(index % 2)
		{
			remotePeer = Format(_T("peer_%d"), index);
			master = false;
		}
		else
		{
			remotePeer = Format(_T("peer_%d"), index + 2);
			master = true;
		}
		connect = Format(_T("conn_%d"), (int)(index/2) + 1);
		m_threadsInfo[index] = boost::shared_ptr<SThreadInfo>(new SThreadInfo(master, index, connect, localPeer, remotePeer));
	}
CATCH_THROW()
}

void CAbstractTest::OutputInfo(const tstring& info)
{
TRY_CATCH
	if(m_infoEvent)
		m_infoEvent(info);
CATCH_LOG()
}

void CAbstractTest::AddGeneralTestResults()
{
TRY_CATCH
	std::list<tstring> settingsInfo = m_settings->GetDescription();
	settingsInfo.push_back(Format(_T("Cancelled clients: %d"), m_statistic.m_cancelled));
	settingsInfo.push_back(Format(_T("Total test time: %I64d ms"), m_statistic.m_testTime));
	for(std::list<tstring>::iterator index = m_results.begin();
		index != m_results.end();
		++index)
	{
		settingsInfo.push_back(*index);
	}
	m_results = settingsInfo;
CATCH_THROW()
}

