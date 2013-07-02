#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  COutFileStreamTimeStampedTestSuite.h
///
///  Test suite for COutFileStreamTimeStamped
///
///  @author Sogin Max @date 21.02.2007
///
////////////////////////////////////////////////////////////////////////
#include <RCEngine/Streaming/COutFileStreamTimeStamped.h>
#include <RCEngine/Streaming/CInFileStreamTimeStamped.h>
#include <queue>

namespace COutFileStreamTimeStampedTestSuite
{
	const int BLOCKS_COUNT = 100;
	const unsigned int TEST_INTERVAL = 50; /*Test time slice interval*/
	const char TEST_DATA[] = "May pease work!";

	/// Memory stream for test purposes
	class CMemStream : public CAbstractStream
	{
	public:
			bool HasInData()
			{
				return !m_queue.empty();
			}
	protected:
			// internal queue
			std::queue<char> m_queue;

			///  Abstract function to get data from the stream
			///  @param   buffer for data
			///  @param   number of bytes to get
			virtual unsigned int ReceiveInternal( char* buf, const unsigned int& len )
			{
			TRY_CATCH
				int res = min(len,m_queue.size());
				if (len > m_queue.size())
					throw MCStreamException("EOF");
				for(int i=0; i<res; ++i)
				{
					buf[i] = m_queue.front();
					m_queue.pop();
				}
				return res;
			CATCH_THROW()
			}

			///  Abstract function to put data to stream
			///  @param   buffer with data
			///  @param   number of bytes to put
			virtual unsigned int SendInternal( const char* buf, const unsigned int& len )
			{
			TRY_CATCH
				for(unsigned int i=0; i<len; ++i)
					m_queue.push(buf[i]);
			CATCH_THROW()
			}
	};

	/// Test InFileStreamTimeStamped implementation
	class CTestStream : public COutFileStreamTimeStamped
	{
	public:
			CMemStream m_memStream;
	protected:
		/// Really gets buffer from file
		/// this method can be redeffined to perform compressed input
		/// @param buf buffer for transfer
		/// @len length of buffer
		virtual void RealPut( const char* buf, const unsigned int &len )
		{
			m_memStream.Send(buf,len);
		}
	};

	/// Test InFileStreamTimeStamped implementation
	class CTestInStream : public CInFileStreamTimeStamped
	{
	protected:
		/// data storage (provided by out stream)
		CAbstractStream& m_stream;
	public:
		/// Initializing template
		CTestInStream(CAbstractStream& stream) : m_stream(stream)
		{}
	protected:
		/// Really gets buffer from file
		/// this method can be redeffined to perform compressed input
		/// @param buf buffer for transfer
		/// @len length of buffer
		virtual void RealGet( char* buf, const unsigned int &len )
		{
			return m_stream.Receive(buf,len);
		}
	};

	//SendInternal test case
	void TestSendInternal()
	{
		CTestStream stream;
		for(int i=0; i<BLOCKS_COUNT; ++i)
		{
			stream.Send(TEST_DATA,sizeof(TEST_DATA));
			Sleep(TEST_INTERVAL);
		}
		char buf[MAX_PATH];
		CTestInStream inStream(stream.m_memStream);
		for(int i=0; i<BLOCKS_COUNT; ++i)
		{
			inStream.Receive(buf,sizeof(TEST_DATA));
			BOOST_CHECK( memcmp(buf,TEST_DATA,sizeof(TEST_DATA)) == 0 );
		}
	}
};

test_suite* getCOutFileStreamTimeStampedTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "COutFileStreamTimeStampedTestSuite" );
	suite->add( BOOST_TEST_CASE(COutFileStreamTimeStampedTestSuite::TestSendInternal) );
	return suite;
}