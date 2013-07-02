/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTestCollection.cpp
///
///  Implements CTestCollection class, responsible for collection of tests
///
///  @author Dmitry Netrebenko @date 20.02.2008
///
////////////////////////////////////////////////////////////////////////

#include "CTestCollection.h"
#include <AidLib/CException/CException.h>

CTestCollection::CTestCollection()
{
TRY_CATCH
	InitCollection();
CATCH_THROW()
}

CTestCollection::~CTestCollection()
{
TRY_CATCH
CATCH_LOG()
}

SettingsMap& CTestCollection::GetTestSettings()
{
TRY_CATCH
	return m_testSettings;
CATCH_THROW()
}

boost::shared_ptr<CTestSettings> CTestCollection::GetSettingsById(const ETestType id)
{
TRY_CATCH
	SettingsMap::iterator index = m_testSettings.find(id);
	if(m_testSettings.end() == index)
		throw MCException(_T("Unknown test id."));
	return index->second;
CATCH_THROW()
}

void CTestCollection::InitCollection()
{
TRY_CATCH
	boost::shared_ptr<CTestSettings> settings(new CTestSettings(ttRelayTest, RELAY_TEST_NAME));
	m_testSettings[ttRelayTest] = settings;

	settings.reset(new CTestSettings(ttNATTest, NAT_TEST_NAME));
	settings->GetSettings()->SetRelayPort(DEF_NAT_PORT);
	settings->GetSettings()->SetPoolSize(DEF_NAT_POOL_SIZE);
	settings->GetSettings()->SetPeersCount(DEF_NAT_PEERS_COUNT);
	m_testSettings[ttNATTest] = settings;

	settings.reset(new CTestSettings(ttExternalIPTest, EXTIP_TEST_NAME));
	settings->GetSettings()->SetPoolSize(DEF_EXTIP_POOL_SIZE);
	settings->GetSettings()->SetPeersCount(DEF_EXTIP_PEERS_COUNT);
	m_testSettings[ttExternalIPTest] = settings;

	settings.reset(new CTestSettings(ttOpenPortTest, OPENPORT_TEST_NAME));
	settings->GetSettings()->SetPoolSize(DEF_OPENPORT_POOL_SIZE);
	settings->GetSettings()->SetPeersCount(DEF_OPENPORT_PEERS_COUNT);
	m_testSettings[ttOpenPortTest] = settings;
CATCH_THROW()
}

