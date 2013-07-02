/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTestCollection.h
///
///  Declares CTestCollection class, responsible for collection of tests
///
///  @author Dmitry Netrebenko @date 20.02.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CTestSettings.h"
#include <map>
#include <boost/shared_ptr.hpp>
#include "ETestType.h"
#include <AidLib/CSingleton/CSingleton.h>

#define RELAY_TEST_NAME		_T("Relay service test")
#define NAT_TEST_NAME		_T("NAT traversing service test")
#define EXTIP_TEST_NAME		_T("External IP service test")
#define OPENPORT_TEST_NAME	_T("Open port service test")

typedef std::map< ETestType,boost::shared_ptr<CTestSettings> > SettingsMap;

/// CTestCollection class, responsible for collection of tests
class CTestCollection
{
private:
/// Prevents making copies of CTestCollection objects
	CTestCollection(const CTestCollection&);
	CTestCollection& operator=(const CTestCollection&);
public:
/// Constructor
	CTestCollection();
/// Destructor
	~CTestCollection();
/// Returns reference to map with settings
	SettingsMap& GetTestSettings();
/// Returns test settings by test id
	boost::shared_ptr<CTestSettings> GetSettingsById(const ETestType id);
/// Returns default test
	ETestType DefaultTest()
	{
		return ttRelayTest;
	}
private:
/// Map of settings
	SettingsMap	m_testSettings;
/// Initializes collection
	void InitCollection();
};

#define TESTS_INSTANCE CSingleton<CTestCollection>::instance()
