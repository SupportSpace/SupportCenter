/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRSAEncoderTestSuite.h
///
///  Test suite for CRSAEncoder
///
///  @author Dmitry Netrebenko @date 19.09.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/CCrypto/CRSAEncoder.h>

#define BUF_SIZE 1024

namespace CRSAEncoderTestSuite
{
///<------------------------------------ Test cases ---------------------------------------------

	/// CRSAEncoderTestSuite::Encrypt/Decrypt test case
	void TestEncoding()
	{
		CRSAEncoder first, second;
		first.Create(2048);
		second.Create();
		char buffer[BUF_SIZE];

		memset(buffer, 0, BUF_SIZE);
		unsigned int bufsize = BUF_SIZE;
		unsigned int dataSize;
		first.GetLocalKey(NULL, &dataSize);
		first.GetLocalKey(reinterpret_cast<unsigned char*>(buffer), &dataSize);
		second.SetRemoteKey(reinterpret_cast<unsigned char*>(buffer), dataSize);

		memset(buffer, 0, BUF_SIZE);
		bufsize = BUF_SIZE;
		second.GetLocalKey(NULL, &dataSize);
		second.GetLocalKey(reinterpret_cast<unsigned char*>(buffer), &dataSize);
		first.SetRemoteKey(reinterpret_cast<unsigned char*>(buffer), dataSize);

		char* text = "Hello world !!!";
		memset(buffer, 0, BUF_SIZE);
		bufsize = BUF_SIZE;
		strcpy(buffer, text);
		dataSize = (unsigned int)strlen(text);

		unsigned int sz = dataSize;
		first.Encrypt(NULL, 0, &sz);
		first.Encrypt(reinterpret_cast<unsigned char*>(buffer), sz, &dataSize);
		second.Decrypt(reinterpret_cast<unsigned char*>(buffer), &dataSize);
		char* pbuf = buffer;
		pbuf += dataSize;
		*pbuf = 0;
		BOOST_CHECK( 0 == strcmp(text, buffer) );

		dataSize = (unsigned int)strlen(text);
		sz = dataSize;
		second.Encrypt(NULL, 0, &sz);
		second.Encrypt(reinterpret_cast<unsigned char*>(buffer), sz, &dataSize);
		first.Decrypt(reinterpret_cast<unsigned char*>(buffer), &dataSize);
		pbuf = buffer;
		pbuf += dataSize;
		*pbuf = 0;
		BOOST_CHECK( 0 == strcmp(text, buffer) );
	}
}

test_suite* getCRSAEncoderTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CRSAEncoderTestSuite" );
	suite->add( BOOST_TEST_CASE(CRSAEncoderTestSuite::TestEncoding) );
	return suite;
}
