// UTestAidLib.cpp : Defines the entry point for the console application.
//

#include <boostTestLib/boostTestLib.h>
#include "StringsTestSuite.h"
// #include ...
#include "CSingletonTestSuite.h"
#include "cLogTestSuite.h"
#include "cFileLogTestSuite.h"
#include "CThreadTestSuite.h"
#include "CRSAEncoderTestSuite.h"
#include "ThreadUtilTestSuite.h"


/// Main test project entry point
test_suite* init_unit_test_suite( int, char* [] ) 
{
	test_suite* root = BOOST_TEST_SUITE( "root" );
	root->add( getStringsTestSuite() );
	// root->add( ... );
	root->add( getCSingletonTestSuite() );
	root->add( getcLogTestSuite() );
	root->add( getcFileLogTestSuite() );
	root->add( getCThreadTestSuite() );
	root->add( getCRSAEncoderTestSuite() );
	root->add( getThreadUtilTestSuiteTestSuite() );
    return root;
}