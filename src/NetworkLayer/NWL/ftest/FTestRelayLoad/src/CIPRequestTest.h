/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CIPRequestTest.h
///
///  Declares CIPRequestTest class, test for external ip service
///
///  @author Dmitry Netrebenko @date 04.03.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CAbstractTest.h"
#include "CSettings.h"
#include <AidLib/CCritSection/CCritSectionObject.h>
#include "SIPRequestTestStatistic.h"

/// CIPRequestTest class, test for external ip service
class CIPRequestTest
	:	public CAbstractTest
{
private:
/// Prevents making copies of CIPRequestTest objects
	CIPRequestTest(const CIPRequestTest&);
	CIPRequestTest& operator=(const CIPRequestTest&);
public:
/// Constructor
/// @param settings - pointer to settings
	CIPRequestTest(boost::shared_ptr<CSettings> settings);
/// Destructor
	~CIPRequestTest();
protected:
/// Starts test thread
/// @param threadInfo - pointer to thread info
	virtual void RunTestThread(boost::shared_ptr<SThreadInfo> threadInfo);
/// Prepare test result
	virtual void PrepareResults();
private:
/// Test statistic
	SIPRequestTestStatistic		m_statistic;
	CCritSectionSimpleObject	m_section;
	void SafeAdd64(__int64* data, __int64 value);
};
