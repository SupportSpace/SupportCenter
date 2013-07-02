// UTestAidLib.cpp : Defines the entry point for the console application.
//
/// Setting up execution limit to 600 secs
#define LIMIT_EXECUTION_TIME 600000

#include <winsock2.h>
#include <boostTestLib/boostTestLib.h>
#include "CShadowedStreamTestSuite.h"
#include "CShadowedClientTestSuite.h"
#include "CInFileStreamTimeStampedTestSuite.h"
#include "COutFileStreamTimeStampedTestSuite.h"

#pragma comment(lib, "RCEngine.lib")
#pragma comment(lib, "NWL.lib")

/// Main test project entry point
test_suite* init_unit_test_suite( int, char* [] ) 
{
	test_suite* root = BOOST_TEST_SUITE( "root" );
	root->add( getCShadowedStreamTestSuite() );
	root->add( getCShadowedClientTestSuite() );
	root->add( getCInFileStreamTimeStampedTestSuite() );
	root->add( getCOutFileStreamTimeStampedTestSuite() );
	// root->add( ... );
    return root;
}