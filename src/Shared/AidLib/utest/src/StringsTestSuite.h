#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  StringsTestSuite.h
///
///  Test suite for tstring.h methods
///
///  @author Sogin Max @date 09.02.2007
///
////////////////////////////////////////////////////////////////////////
#include <boostTestLib/boostTestLib.h>
#include <AidLib/Strings/tstring.h>

void TestFormat()
{
	BOOST_CHECK( Format("Test %d %s",1,"passed") == _T("Test 1 passed") );
}

void Testi2tstring()
{
	BOOST_CHECK( i2tstring(1) == _T("1") );
	BOOST_CHECK( i2tstring(2) != _T("1") );
}

void TestGetGUID()
{
	BOOST_CHECK( GetGUID() != GetGUID() );
}

void TestLowerCase()
{
	BOOST_CHECK( LowerCase(_T("HellO WorLd")) == _T("hello world"));
}

void TestUpperCase()
{
	BOOST_CHECK( UpperCase(_T("HellO WorLd")) == _T("HELLO WORLD"));
}

test_suite* getStringsTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "StringsTestSuite" );
	suite->add( BOOST_TEST_CASE(&TestFormat) );
	suite->add( BOOST_TEST_CASE(&TestGetGUID) );
	suite->add( BOOST_TEST_CASE(&TestLowerCase) );
	suite->add( BOOST_TEST_CASE(TestUpperCase) );
	return suite;
}