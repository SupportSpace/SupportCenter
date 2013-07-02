#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CShadowedClientTestSuite.h
///
///  Test suite for CShadowedClient
///
///  @author Sogin Max @date 21.02.2007
///
////////////////////////////////////////////////////////////////////////
#include "./../../src/RCHost/CShadowedClient.h"
#include "./../../src/CTokenCatcher.h"
#include <boost/scoped_array.hpp>
#include <boost/threadpool.hpp>

namespace CShadowedClientTestSuite
{
	const int THREAD_COUNT=10;
	const int CALLS_COUNT=100;
	const char* TEST_STRING = "this is test string for send";
	const int READ_TEST_COUNT=10;

	//Test stream class
	class CTestStream : public CAbstractStream
	{
	public:
			bool m_hasInData;
			tstring m_receivedData;

			///  Checks data in the stream
			///  @return returns amount of available data
			virtual bool HasInData()
			{
				return m_hasInData;
			}

			/// ctor
			CTestStream()
				:	m_hasInData(false),
					m_receivedData()
			{
			}
	private:
			///  Abstract function definition to get data from the stream
			///  @param   buffer for data
			///  @param   number of bytes to get
			///  @remarks
			virtual unsigned int ReceiveInternal( char* buf, const unsigned int& size )
			{
				int res = min(size,m_receivedData.length() + 1);
				memcpy(buf,m_receivedData.c_str(),res);
				return res;
			}

			///  Abstract function definition to put data to stream
			///  @param   buffer with data
			///  @param   number of bytes to put
			///  @remarks
			virtual unsigned int SendInternal( const char* buf, const unsigned int& size )
			{
				BOOST_CHECK( strcmp(buf,TEST_STRING) == 0 );
				return size;
			}
	};

	/// TestSendInternal single call
	void TestSendOnce()
	{
		static CShadowedClient client(boost::shared_ptr<CAbstractStream>(new CTestStream()));
		boost::scoped_array<char> tmp;
		tmp.reset(_strdup(TEST_STRING));
		client.Send(tmp.get(),strlen(tmp.get()));
	}

	/// Send internal test case
	void TestSendInternal()
	{
		boost::threadpool::pool threadPool(THREAD_COUNT);
		for(int i=0; i<=CALLS_COUNT; ++i)
		{
			threadPool.schedule(boost::bind(&TestSendOnce));
		}
	}

	typedef unsigned char CARD8;
	typedef unsigned short CARD16;
	typedef unsigned int CARD32;
	#define Swap16IfLE(s) \
    ((CARD16) ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff)))
	typedef struct 
	{
		CARD8 shared;
	} rfbClientInitMsg;
	#define sz_rfbClientInitMsg 1
	typedef struct 
	{
		CARD8 bitsPerPixel;		/* 8,16,32 only */
		CARD8 depth;		/* 8 to 32 */
		CARD8 bigEndian;		/* True if multi-byte pixels are interpreted
					   as big endian, or if single-bit-per-pixel
					   has most significant bit of the byte
					   corresponding to first (leftmost) pixel. Of
					   course this is meaningless for 8 bits/pix */
		CARD8 trueColour;		/* If false then we need a "colour map" to
					   convert pixels to RGB.  If true, xxxMax and
					   xxxShift specify bits used for red, green
					   and blue */
		/* the following fields are only meaningful if trueColour is true */
		CARD16 redMax;		/* maximum red value (= 2^n - 1 where n is the
					   number of bits used for red). Note this
					   value is always in big endian order. */
		CARD16 greenMax;		/* similar for green */
		CARD16 blueMax;		/* and blue */
		CARD8 redShift;		/* number of shifts needed to get the red
				   value in a pixel to the least significant
				   bit. To find the red value from a given
				   pixel, do the following:
				   1) Swap pixel value according to bigEndian
				      (e.g. if bigEndian is false and host byte
				      order is big endian, then swap).
				   2) Shift right by redShift.
				   3) AND with redMax (in host byte order).
				   4) You now have the red value between 0 and
				      redMax. */
	    CARD8 greenShift;		/* similar for green */
	    CARD8 blueShift;		/* and blue */
		CARD8 pad1;
	    CARD16 pad2;
	} rfbPixelFormat;
	#define sz_rfbPixelFormat 16
	typedef struct 
	{
		CARD8 type;			/* always rfbSetPixelFormat */
		CARD8 pad1;
		CARD16 pad2;
		rfbPixelFormat format;
	} rfbSetPixelFormatMsg;
	#define sz_rfbSetPixelFormatMsg (sz_rfbPixelFormat + 4)
	#define rfbSetPixelFormat 0

	/*-----------------------------------------------------------------------------
	 * SetEncodings - tell the RFB server which encoding types we accept.  Send them
	 * in order of preference, if we have any.  We may always receive raw
	 * encoding, even if we don't specify it here.
	 */
	typedef struct {
		CARD8 type;			/* always rfbSetEncodings */
		CARD8 pad;
		CARD16 nEncodings;
		/* followed by nEncodings * CARD32 encoding types */
	} rfbSetEncodingsMsg;
	#define sz_rfbSetEncodingsMsg 4
	#define rfbSetEncodings 2

	/*-----------------------------------------------------------------------------
	 * FramebufferUpdateRequest - request for a framebuffer update.  If incremental
	 * is true then the client just wants the changes since the last update.  If
	 * false then it wants the whole of the specified rectangle.
	 */
	typedef struct {
		CARD8 type;			/* always rfbFramebufferUpdateRequest */
		CARD8 incremental;
		CARD16 x;
		CARD16 y;
		CARD16 w;
		CARD16 h;
	} rfbFramebufferUpdateRequestMsg;
	#define sz_rfbFramebufferUpdateRequestMsg 10
	#define rfbFramebufferUpdateRequest 3

	/// Receive internal test case
	void TestReceiveInternal()
	{
		/// Testing session start
		CShadowedClient client(boost::shared_ptr<CAbstractStream>(new CTestStream()));
		char code;
		tstring startToken(START_COMMAND);
		CTokenCatcher tockenCatcher(startToken.c_str(),startToken.length());
		bool started(false);
		for(unsigned int i=0; i<strlen(START_COMMAND); ++i)
		{
			client.Receive(&code,1);
			if (tockenCatcher.Send(&code,1))
			{
				started = true;
				break;
			}
		}
		BOOST_CHECK(started);

		// Read the client's initialisation message
		rfbClientInitMsg client_ini;
		client.Receive((char *)&client_ini, sz_rfbClientInitMsg);

		// Read pixel format
		rfbSetPixelFormatMsg pixelFormatMsg;
		client.Receive(reinterpret_cast<char*>(&pixelFormatMsg), sz_rfbSetPixelFormatMsg);
		BOOST_CHECK( pixelFormatMsg.type == rfbSetPixelFormat );

		/// Read encodings
		rfbSetEncodingsMsg setEncodingsMsg;
		client.Receive(reinterpret_cast<char*>(&setEncodingsMsg), sz_rfbSetEncodingsMsg);
		BOOST_CHECK( setEncodingsMsg.type == rfbSetEncodings );
		CARD32 enc;
		for(int i=0; i<Swap16IfLE(setEncodingsMsg.nEncodings); ++i)
			client.Receive(reinterpret_cast<char*>(&enc), sizeof(enc));

		/// Read framebuffer update requests
		for(int i=0; i<READ_TEST_COUNT; ++i)
		{
			rfbFramebufferUpdateRequestMsg updateRequest;
			client.Receive(reinterpret_cast<char*>(&updateRequest), sz_rfbFramebufferUpdateRequestMsg);
			BOOST_CHECK( updateRequest.type == rfbFramebufferUpdateRequest );
		}
	}

};

test_suite* getCShadowedClientTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CShadowedClientTestSuite" );
	suite->add( BOOST_TEST_CASE(CShadowedClientTestSuite::TestSendInternal) );
	suite->add( BOOST_TEST_CASE(CShadowedClientTestSuite::TestReceiveInternal) );
	return suite;
}


