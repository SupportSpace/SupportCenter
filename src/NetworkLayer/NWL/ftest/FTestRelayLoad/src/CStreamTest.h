/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStreamTest.h
///
///  Declares CStreamTest class, test for stream connection
///
///  @author Dmitry Netrebenko @date 27.02.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CAbstractTest.h"
#include "CSettings.h"
#include <NWL/Streaming/CAbstractNetworkStream.h>
#include <boost/shared_ptr.hpp>
#include "SStreamTestStatistic.h"
#include <AidLib/CCritSection/CCritSectionObject.h>

/// CStreamTest class, test for stream connection
class CStreamTest
	:	public CAbstractTest
{
private:
/// Prevents making copies of CStreamTest objects
	CStreamTest(const CStreamTest&);
	CStreamTest& operator=(const CStreamTest&);
public:
/// Constructor
/// @param settings - pointer to settings
	CStreamTest(boost::shared_ptr<CSettings> settings);
/// Destructor
	~CStreamTest();
protected:
/// Starts test thread
/// @param threadInfo - pointer to thread info
	virtual void RunTestThread(boost::shared_ptr<SThreadInfo> threadInfo);
/// Prepare test result
	virtual void PrepareResults();
/// Abstract method to create scream
	virtual boost::shared_ptr<CAbstractNetworkStream> CreateStream(boost::shared_ptr<SThreadInfo> threadInfo) = NULL;
private:
/// Test statistic information
	SStreamTestStatistic		m_statistic;
	CCritSectionSimpleObject	m_section;
	void SafeAdd64(__int64* data, __int64 value);
};
