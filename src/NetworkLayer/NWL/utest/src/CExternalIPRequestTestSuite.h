/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CExternalIPRequestTestSuite.h
///
///  Test suite for CExternalIPRequest
///
///  @author Dmitry Netrebenko @date 07.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Streaming/CExternalIPRequest.h>
#include <boostTestLib/boostTestLib.h>

namespace CExternalIPRequestTestSuite
{
	void TestReceiveAddr()
	{
		CExternalIPRequest request1(RELAY_SERVER, RELAY_TCP_PORT);
		CExternalIPRequest request2(RELAY_SERVER, RELAY_TCP_PORT);

		SPeerAddr addr1 = request1.GetExternalAddress(JABBER_USER_NAME_1, SRV_PASSWORD);
		SPeerAddr addr2 = request2.GetExternalAddress(JABBER_USER_NAME_2, SRV_PASSWORD);

		Log.Add(_MESSAGE_, Format(_T("Address1 = %s"), addr1.address).c_str());
		Log.Add(_MESSAGE_, Format(_T("Address2 = %s"), addr2.address).c_str());

		BOOST_CHECK( 0 == memcmp(addr1.address, addr2.address, PEER_ADDR_SIZE) );
	}
}

test_suite* getCExternalIPRequestTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CExternalIPRequestTestSuite" );
	suite->add( BOOST_TEST_CASE(CExternalIPRequestTestSuite::TestReceiveAddr) );
	return suite;
}
