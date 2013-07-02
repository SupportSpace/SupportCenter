/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTLSAuthSettingsTestSuite.h
///
///  Test suite for CTLSAuthSettings
///
///  @author Dmitry Netrebenko @date 19.09.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Streaming/CTLSAuthSettings.h>

namespace CTLSAuthSettingsTestSuite
{

///<------------------------------------ Test cases ---------------------------------------------

	/// Stream suite test case
	void TestAuthSuite()
	{
		CTLSAuthSettings stream;
		STLSSuite suite;
		suite.KeyExchange = KX_DHE_PSK;
		suite.Cipher = CPH_3DES;
		suite.Compression = PRS_LZO;
		suite.PrimeBits = PB_4096;
		stream.SetSuite( suite );
		BOOST_CHECK( suite.KeyExchange == stream.GetSuite().KeyExchange );
		BOOST_CHECK( suite.Cipher == stream.GetSuite().Cipher );
		BOOST_CHECK( suite.Compression == stream.GetSuite().Compression );
		BOOST_CHECK( suite.PrimeBits == stream.GetSuite().PrimeBits );

		stream.GetSuite().KeyExchange = KX_PSK;
		suite.Cipher = CPH_AES_128;
		stream.GetSuite().Compression = PRS_ZLIB;
		suite.PrimeBits = PB_1024;
		BOOST_CHECK( suite.KeyExchange != stream.GetSuite().KeyExchange );
		BOOST_CHECK( suite.Cipher != stream.GetSuite().Cipher );
		BOOST_CHECK( suite.Compression != stream.GetSuite().Compression );
		BOOST_CHECK( suite.PrimeBits != stream.GetSuite().PrimeBits );
	}

	/// Credentials test case
	void TestAuthCredentials()
	{
		CTLSAuthSettings stream;
		STLSCredentials credentials;
		credentials.UserID = _T("ThisIsUserName");
		credentials.Key = _T("ThisIsKey");
		stream.SetCredentials(credentials);
		BOOST_CHECK( credentials.UserID == stream.GetCredentials().UserID );
		BOOST_CHECK( credentials.Key == stream.GetCredentials().Key );

		credentials.UserID = _T("NewUserID");
		stream.GetCredentials().Key = _T("NewKey");
		BOOST_CHECK( credentials.UserID != stream.GetCredentials().UserID );
		BOOST_CHECK( credentials.Key != stream.GetCredentials().Key );
	}
}

test_suite* getCTLSAuthSettingsTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CTLSAuthSettingsTestSuite" );
	suite->add( BOOST_TEST_CASE(CTLSAuthSettingsTestSuite::TestAuthSuite) );
	suite->add( BOOST_TEST_CASE(CTLSAuthSettingsTestSuite::TestAuthCredentials) );
	return suite;
}
