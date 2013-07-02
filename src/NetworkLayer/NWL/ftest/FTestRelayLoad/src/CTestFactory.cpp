/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTestFactory.cpp
///
///  Implements CTestFactory class, test factory
///
///  @author Dmitry Netrebenko @date 22.02.2008
///
////////////////////////////////////////////////////////////////////////

#include "CTestFactory.h"
#include <AidLib/CException/CException.h>
#include "CRelayedStreamTest.h"
#include "CNATTraversingStreamTest.h"
#include "CIPRequestTest.h"
#include "COpenPortTest.h"

CTestFactory::CTestFactory()
{
TRY_CATCH
CATCH_THROW()
}

CTestFactory::~CTestFactory()
{
TRY_CATCH
CATCH_LOG()
}

boost::shared_ptr<CAbstractTest> CTestFactory::CreateTest(boost::shared_ptr<CTestSettings> testSettings)
{
TRY_CATCH
	if(!testSettings.get())
		throw MCException(_T("Empty test setting."));
	boost::shared_ptr<CSettings> settings = testSettings->GetSettings();
	switch(testSettings->GetId())
	{
	case ttRelayTest:
		{
			return boost::shared_ptr<CAbstractTest>(new CRelayedStreamTest(settings));
		}
		break;
	case ttNATTest:
		{
			return boost::shared_ptr<CAbstractTest>(new CNATTraversingStreamTest(settings));
		}
		break;
	case ttExternalIPTest:
		{
			return boost::shared_ptr<CAbstractTest>(new CIPRequestTest(settings));
		}
		break;
	case ttOpenPortTest:
		{
			return boost::shared_ptr<CAbstractTest>(new COpenPortTest(settings));
		}
		break;
	default:
		throw MCException(_T("Unknown test type."));
	}
CATCH_THROW()
}
