/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTestFactory.h
///
///  Declares CTestFactory class, test factory
///
///  @author Dmitry Netrebenko @date 22.02.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/shared_ptr.hpp>
#include "CAbstractTest.h"
#include "CTestSettings.h"
#include <AidLib/CSingleton/CSingleton.h>

/// CTestFactory class, test factory
class CTestFactory
{
private:
/// Prevents making copies of CTestFactory objects
	CTestFactory(const CTestFactory&);
	CTestFactory& operator=(const CTestFactory&);
public:
/// Constructor
	CTestFactory();
/// Destructor
	~CTestFactory();
/// Creates test by settings
	boost::shared_ptr<CAbstractTest> CreateTest(boost::shared_ptr<CTestSettings> testSettings);
};

#define TESTFACTORY_INSTANCE CSingleton<CTestFactory>::instance()
