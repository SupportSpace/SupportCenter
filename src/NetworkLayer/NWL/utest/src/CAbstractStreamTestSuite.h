/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAbstractStreamTestSuite.h
///
///  Test suite for CAbstractStream
///
///  @author Dmitry Netrebenko @date 07.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Streaming/CAbstractStream.h>
#include <boostTestLib/boostTestLib.h>

namespace CAbstractStreamTestSuite
{
	/// Class for testing CAbstractStream
	class CTestAbstractStream : public CAbstractStream
	{
	public:
		CTestAbstractStream()
			:	CAbstractStream()
		{
			Clear();
		}
	public:
		char m_buffer[STREAM_QUEUE_SIZE];
		unsigned int m_count;
		unsigned int m_offset;
		void Clear()
		{
			memset(m_buffer, 0, STREAM_QUEUE_SIZE);
			m_count = 0;
			m_offset = 0;
		}
	protected:
		/// Override abstract method
		virtual unsigned int ReceiveInternal( char* buf, const unsigned int& count )
		{
			char* p = m_buffer + m_offset;
			memcpy(buf, p, count);
			m_offset += count;
			m_count -= count;
			return 0;
		}
		/// Override abstract method
		virtual unsigned int SendInternal( const char* buf, const unsigned int& count )
		{
			memcpy(m_buffer, buf, count);
			m_count = count;
			return 0;
		}
	public:
		/// Override abstract method
		virtual bool HasInData() 
		{ 
			return m_count > 0; 
		}
	};


///<------------------------------------ Test cases ---------------------------------------------


	/// CAbstractStream::Send() test case
	void TestSend()
	{
		CTestAbstractStream stream;
		char buf[STREAM_QUEUE_SIZE];
		memset(buf, 0, STREAM_QUEUE_SIZE);
		unsigned int len = static_cast<unsigned int>(strlen("Hello world !!!"));
		memcpy(buf, "Hello world !!!", len);

		stream.Send(buf, len);
		BOOST_CHECK( stream.m_count == len );
		BOOST_CHECK( 0 == memcmp(buf, stream.m_buffer, len) );
	}

	/// CAbstractStream::Receive() test case
	void TestReceive()
	{
		CTestAbstractStream stream;
		char buf[STREAM_QUEUE_SIZE];
		memset(buf, 0, STREAM_QUEUE_SIZE);
		unsigned int len = static_cast<unsigned int>(strlen("Hello world !!!"));

		memcpy(stream.m_buffer, "Hello world !!!", len);
		stream.m_count = len;

		stream.Receive(buf, len);
		BOOST_CHECK( stream.m_offset == len );
		BOOST_CHECK( 0 == stream.m_count );
		BOOST_CHECK( 0 == memcmp(buf, stream.m_buffer, len) );
	}

	/// CAbstractStream::Send2Queue() test case
	void TestSend2Queue()
	{
		CTestAbstractStream stream;
		char buf[STREAM_QUEUE_SIZE];
		memset(buf, 0, STREAM_QUEUE_SIZE);

		unsigned int len = static_cast<unsigned int>(strlen("Hello world !!!"));
		memcpy(buf, "Hello world !!!", len);

		stream.Send2Queue(buf, len);
		BOOST_CHECK( 0 == stream.m_count );
		BOOST_CHECK( 0 != memcmp(stream.m_buffer, buf, len) );

		memset(buf, '#', STREAM_QUEUE_SIZE);
		stream.Send2Queue(buf, STREAM_QUEUE_SIZE);
		memcpy(buf, "Hello world !!!", len);

		BOOST_CHECK( STREAM_QUEUE_SIZE == stream.m_count );
		BOOST_CHECK( 0 == memcmp(stream.m_buffer, buf, STREAM_QUEUE_SIZE) );
	}

	/// CAbstractStream::FlushQueue() test case
	void TestFlushQueue()
	{
		CTestAbstractStream stream;
		char buf[STREAM_QUEUE_SIZE];
		memset(buf, 0, STREAM_QUEUE_SIZE);
		unsigned int len = static_cast<unsigned int>(strlen("Hello world !!!"));
		memcpy(buf, "Hello world !!!", len);

		stream.Send2Queue(buf, len);
		BOOST_CHECK( 0 == stream.m_count );
		BOOST_CHECK( 0 != memcmp(stream.m_buffer, buf, len) );

		stream.FlushQueue();
		BOOST_CHECK( len == stream.m_count );
		BOOST_CHECK( 0 == memcmp(stream.m_buffer, buf, len) );
	}

	/// CAbstractStream::GetInBuffer() test case
	void TestGetInBuffer()
	{
		CTestAbstractStream stream;
		char buf[STREAM_QUEUE_SIZE];
		memset(buf, 0, STREAM_QUEUE_SIZE);
		unsigned int len = static_cast<unsigned int>(strlen("Hello world !!!"));

		memcpy(stream.m_buffer, "Hello world !!!", len);
		stream.m_count = len;

		unsigned int len2 = stream.GetInBuffer(buf, len);
		BOOST_CHECK( stream.m_offset == len );
		BOOST_CHECK( len2 == len );
		BOOST_CHECK( 0 == stream.m_count );
		BOOST_CHECK( 0 == memcmp(buf, stream.m_buffer, len) );
	}
}

test_suite* getCAbstractStreamTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CAbstractStreamTestSuite" );
	suite->add( BOOST_TEST_CASE(CAbstractStreamTestSuite::TestSend) );
	suite->add( BOOST_TEST_CASE(CAbstractStreamTestSuite::TestReceive) );
	suite->add( BOOST_TEST_CASE(CAbstractStreamTestSuite::TestSend2Queue) );
	suite->add( BOOST_TEST_CASE(CAbstractStreamTestSuite::TestFlushQueue) );
	suite->add( BOOST_TEST_CASE(CAbstractStreamTestSuite::TestGetInBuffer) );
	return suite;
}
