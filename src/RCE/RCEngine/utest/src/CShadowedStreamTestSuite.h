#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CShadowedStreamTestSuite.h
///
///  Test suite for CShadowedStream
///
///  @author Sogin Max @date 21.02.2007
///
////////////////////////////////////////////////////////////////////////
#include <RCEngine/Streaming/CShadowedStream.h>
#include <boost/scoped_array.hpp>

namespace CShadowedStreamTestSuite
{
	//Test stream class
	class CTestStream : public CAbstractStream
	{
	public:
			bool m_hasInData;
			tstring m_sent;
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
					m_sent(),
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
				boost::scoped_array<TCHAR> str;
				str.reset(new TCHAR[size + 1]);
				memset(str.get(),0,size+1);
				memcpy(str.get(),buf,size);
				m_sent += str.get();
				return size;
			}
	};

	/// fixture for shadowed stream
	class CShadowedStreamFixture
	{
	public:
		boost::shared_ptr<CTestStream> m_mainStream;
		boost::shared_ptr<CTestStream> m_shadowStream;
		CShadowedStream m_stream;

		CShadowedStreamFixture(CShadowedStream::EShadowType type)
			:	
				m_mainStream(new CTestStream()),
				m_shadowStream(new CTestStream()),
				m_stream(m_mainStream,type)
		{
			m_stream.SetShadowStream(m_shadowStream);
		}		
	};

	/// CShadowedStream::Send test case
	void TestSend()
	{
		tstring testString(_T("Test string"));

		CShadowedStreamFixture f1(CShadowedStream::OUTPUT);
		f1.m_stream.Send(testString.c_str(),testString.length());
		BOOST_CHECK( testString == f1.m_mainStream->m_sent && testString == f1.m_shadowStream->m_sent );

		CShadowedStreamFixture f2(CShadowedStream::BOTH);
		f2.m_stream.Send(testString.c_str(),testString.length());
		BOOST_CHECK( testString == f2.m_mainStream->m_sent && testString == f2.m_shadowStream->m_sent );

		CShadowedStreamFixture f3(CShadowedStream::INPUT);
		f3.m_stream.Send(testString.c_str(),testString.length());
		BOOST_CHECK( testString == f3.m_mainStream->m_sent && testString != f3.m_shadowStream->m_sent );
	}

	/// CShadowedStream::Receive test case
	void TestReceive()
	{
		tstring testString(_T("Test string"));
		TCHAR buf[MAX_PATH];
		CShadowedStreamFixture f1(CShadowedStream::OUTPUT);
		f1.m_mainStream->m_receivedData = testString; //Setting up fake received data for main stream
		f1.m_stream.Receive(buf,MAX_PATH);
		BOOST_CHECK( testString == buf && testString != f1.m_shadowStream->m_sent );

		CShadowedStreamFixture f2(CShadowedStream::INPUT);
		f2.m_mainStream->m_receivedData = testString; //Setting up fake received data for main stream
		f2.m_stream.Receive(buf,MAX_PATH);
		BOOST_CHECK( testString == buf && testString == f2.m_shadowStream->m_sent );

		CShadowedStreamFixture f3(CShadowedStream::BOTH);
		f3.m_mainStream->m_receivedData = testString; //Setting up fake received data for main stream
		f3.m_stream.Receive(buf,MAX_PATH);
		BOOST_CHECK( testString == buf && testString == f3.m_shadowStream->m_sent );	
	}

	/// CShadowedStream::H test case
	void TestHasInData()
	{
		CShadowedStreamFixture f1(CShadowedStream::OUTPUT);
		f1.m_mainStream->m_hasInData = true;
		BOOST_CHECK(f1.m_stream.HasInData());
		BOOST_CHECK(!f1.m_shadowStream->HasInData());
	}
}

test_suite* getCShadowedStreamTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CShadowedStreamTestSuite" );
	suite->add( BOOST_TEST_CASE(CShadowedStreamTestSuite::TestSend) );
	suite->add( BOOST_TEST_CASE(CShadowedStreamTestSuite::TestReceive) );
	suite->add( BOOST_TEST_CASE(CShadowedStreamTestSuite::TestHasInData) );
	return suite;
}


