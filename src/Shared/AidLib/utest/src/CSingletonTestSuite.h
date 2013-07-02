#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSingletonTestSuite.h
///
///  Test suite for CSingleton template
///
///  @author Sogin Max @date 09.02.2007
///
////////////////////////////////////////////////////////////////////////
#include <boostTestLib/boostTestLib.h>
#include <AidLib/CSingleton/CSingleton.h>

namespace CSingletonTestSuite
{
	
	/// Stub class for testing CSingleton
	class Test : public tstring
	{
	public:
		Test& operator= (const TCHAR* tst)
		{
			tstring::operator=(tst);
			return *this;
		}
	};

	/// CSingleton::instance test case
	void TestInstance()
	{
		{
			CSingleton<Test>::instance() = _T("Test the world");
			BOOST_CHECK( CSingleton<Test>::instance() == tstring("Test the world") );
		}
		{
			BOOST_CHECK( CSingleton<Test>::instance() == tstring("Test the world") );
			BOOST_CHECK( CSingleton<Test>::instance() != _T("Hack the world") );
		}
	}
};

test_suite* getCSingletonTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CSingletonTestSuite" );
	suite->add( BOOST_TEST_CASE(&CSingletonTestSuite::TestInstance) );
	return suite;
}

