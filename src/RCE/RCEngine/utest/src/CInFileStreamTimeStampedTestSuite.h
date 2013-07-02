#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CInFileStreamTimeStampedTestSuite.h
///
///  Test suite for CInFileStreamTimeStamped
///
///  @author Sogin Max @date 21.02.2007
///
////////////////////////////////////////////////////////////////////////
#include <RCEngine/Streaming/CInFileStreamTimeStamped.h>
#include <queue>

namespace CInFileStreamTimeStampedTestSuite
{

	const int BLOCKS_COUNT = 100;
	const int THREADS_COUNT = 50;
	const unsigned int TEST_INTERVAL = 50; /*Test time slice interval*/
	const char TEST_DATA[] = "Mir trud may!";

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
	class CTestStream : public CInFileStreamTimeStamped
	{
	protected:
		/// Template stream
		CMemStream m_template;
	public:
		/// Initializing template
		CTestStream()
		{
			//test time block
			int delay = TEST_INTERVAL;
			scoped_array<char> timeBlock;
			int size = sizeof(SBlock) + sizeof(delay) - 1;
			timeBlock.reset(new char[size]);
			SBlock* ptimeBlock = reinterpret_cast<SBlock*>(timeBlock.get());
			ptimeBlock->type = TIMESTAMP;
			ptimeBlock->size = size;
			memcpy(ptimeBlock->buf, &delay, sizeof(delay));

			/// test data block
			scoped_array<char> dataBlock;
			size = sizeof(SBlock) + sizeof(TEST_DATA) - 1;
			dataBlock.reset(new char[size]);
			SBlock* pdataBlock = reinterpret_cast<SBlock*>(dataBlock.get());
			pdataBlock->type = DATA;
			pdataBlock->size = size;
			memcpy(pdataBlock->buf, TEST_DATA, sizeof(TEST_DATA));

			//Forming test stream
			for(int i=0; i<BLOCKS_COUNT; ++i)
			{
				m_template.Send(timeBlock.get(),ptimeBlock->size);
				m_template.Send(dataBlock.get(),pdataBlock->size);
				if (i % 2 == 0)
					m_template.Send(timeBlock.get(),ptimeBlock->size);
			}
		}
	protected:
		/// Really gets buffer from file
		/// this method can be redeffined to perform compressed input
		/// @param buf buffer for transfer
		/// @len length of buffer
		virtual void RealGet( char* buf, const unsigned int &len )
		{
			return m_template.Receive(buf,len);
		}
	};

	/// Receive internal test case
	void TestReceiveInternal()
	{
	TRY_CATCH
		CTestStream stream;
		char buf[MAX_PATH];
		for(int i=0; i<BLOCKS_COUNT; ++i)
		{
			stream.Receive(buf,sizeof(TEST_DATA));
			BOOST_CHECK( memcmp(buf,TEST_DATA,sizeof(TEST_DATA)) == 0 );
		}

	CATCH_THROW()
	}

	/// Test END of file exception
	void TestEOF()
	{
	TRY_CATCH
		CTestStream stream;
		char buf[MAX_PATH];
		for(int i=0; i<BLOCKS_COUNT; ++i)
		{
			stream.Receive(buf,sizeof(TEST_DATA));
			BOOST_CHECK( memcmp(buf,TEST_DATA,sizeof(TEST_DATA)) == 0 );
		}
		BOOST_CHECK_THROW(stream.Receive(buf,1),CStreamException);

	CATCH_THROW()
	}
};

test_suite* getCInFileStreamTimeStampedTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CInFileStreamTimeStampedTestSuite" );
	suite->add( BOOST_TEST_CASE(CInFileStreamTimeStampedTestSuite::TestReceiveInternal) );
	suite->add( BOOST_TEST_CASE(CInFileStreamTimeStampedTestSuite::TestEOF) );
	return suite;
}