/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  COpenPortRequestTestSuite.h
///
///  Test suite for COpenPortRequest
///
///  @author Alexander Novak @date 02.07.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Streaming/COpenPortRequest.h>
#include <boostTestLib/boostTestLib.h>

namespace COpenPortRequestTestSuite
{
	void TestCheckExternalPortSuccess()
	{
		COpenPortRequest request(RELAY_SERVER, RELAY_TCP_PORT);
		request.CheckPortAvailability(JABBER_USER_NAME_1, SRV_PASSWORD, PORTMAPPING_PORT, PORTMAPPING_PORT);
		BOOST_CHECK( TRUE );
	}

	void TestCheckExternalPortFailure()
	{
		COpenPortRequest request(RELAY_SERVER, RELAY_TCP_PORT);
		BOOST_CHECK_THROW( 	request.CheckPortAvailability(JABBER_USER_NAME_1, SRV_PASSWORD, PORTMAPPING_PORT, PORTMAPPING_PORT + 1),
							CStreamException );
	}

	void TestCheckExternalPortAuthFailed()
	{
		COpenPortRequest request(RELAY_SERVER, RELAY_TCP_PORT);
		BOOST_CHECK_THROW( 	request.CheckPortAvailability(JABBER_USER_NAME_1, _T("Some incorrect password"), PORTMAPPING_PORT, PORTMAPPING_PORT),
							CStreamException );
	}

}

test_suite* getCOpenPortRequestTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "COpenPortRequestTestSuite" );
	suite->add( BOOST_TEST_CASE(COpenPortRequestTestSuite::TestCheckExternalPortSuccess) );
	suite->add( BOOST_TEST_CASE(COpenPortRequestTestSuite::TestCheckExternalPortFailure) );
	suite->add( BOOST_TEST_CASE(COpenPortRequestTestSuite::TestCheckExternalPortAuthFailed) );
	return suite;
}
