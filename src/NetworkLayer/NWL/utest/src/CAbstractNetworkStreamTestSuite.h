/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAbstractNetworkStreamTestSuite.h
///
///  Test suite for CAbstractNetworkStream
///
///  @author Dmitry Netrebenko @date 07.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Streaming/CAbstractNetworkStream.h>
#include <boostTestLib/boostTestLib.h>
#include <boost/bind.hpp>

namespace CAbstractNetworkStreamTestSuite
{
	/// Class for testing CAbstractNetworkStream
	class CTestAbstractNetworkStream : public CAbstractNetworkStream
	{
	public:
		CTestAbstractNetworkStream()
			:	CAbstractNetworkStream()
			,	m_connectEvent(false)
			,	m_disconnectEvent(false)
			,	m_connectErrorEvent(false)
			,	m_errorReason(cerNoError)
		{}

		bool m_connectEvent;
		bool m_disconnectEvent;
		bool m_connectErrorEvent;
		EConnectErrorReason m_errorReason;

		/// Stubs for raise events
		void TestRaiseConnectedEvent() { RaiseConnectedEvent(); };
		void TestRaiseDisconnectedEvent() { RaiseDisconnectedEvent(); };
		void TestRaiseConnectErrorEvent() { RaiseConnectErrorEvent(cerUnknown); };

		/// Events handlers
		void OnConnected( void* ) { m_connectEvent = true; };
		void OnDisconnected( void* ) { m_disconnectEvent = true; };
		void OnConnectError( void*, EConnectErrorReason reason ) { m_connectErrorEvent = true; m_errorReason = reason; };
	protected:
		/// Override abstract method
		virtual unsigned int ReceiveInternal( char*, const unsigned int& ) { return 0; };
		/// Override abstract method
		virtual unsigned int SendInternal( const char*, const unsigned int& ) { return 0; };
	public:
		/// Override abstract method
		virtual bool HasInData() { return false; };
		/// Override abstract method
		virtual void Connect( const bool = false ) {};
		/// Override abstract method
		virtual void Disconnect() {};
		/// Override abstract method
		virtual bool Connected() const { return false; };
	};


///<------------------------------------ Test cases ---------------------------------------------


	/// Set/Get connect timeout test case
	void TestTimeout()
	{
		CTestAbstractNetworkStream stream;
		unsigned int t1 = 3000;
		unsigned int t2 = 5000;

		BOOST_CHECK( DEFAULT_CONNECT_TIMEOUT == stream.GetConnectTimeout() );
		
		stream.SetConnectTimeout( t1 );
		BOOST_CHECK( t1 == stream.GetConnectTimeout() );

		stream.SetConnectTimeout( t2 );
		BOOST_CHECK( t1 != stream.GetConnectTimeout() );
	}

	/// Connection events test case
	void TestEvents()
	{
		CTestAbstractNetworkStream stream;

		BOOST_CHECK( false == stream.m_connectEvent );
		stream.TestRaiseConnectedEvent();
		BOOST_CHECK( false == stream.m_connectEvent );
		stream.SetConnectedEvent( boost::bind( &CTestAbstractNetworkStream::OnConnected, &stream, _1 ) );
		stream.TestRaiseConnectedEvent();
		BOOST_CHECK( true == stream.m_connectEvent );
		
		BOOST_CHECK( false == stream.m_disconnectEvent );
		stream.TestRaiseDisconnectedEvent();
		BOOST_CHECK( false == stream.m_disconnectEvent );
		stream.SetDisconnectedEvent( boost::bind( &CTestAbstractNetworkStream::OnDisconnected, &stream, _1 ) );
		stream.TestRaiseDisconnectedEvent();
		BOOST_CHECK( true == stream.m_disconnectEvent );
		
		BOOST_CHECK( false == stream.m_connectErrorEvent );
		BOOST_CHECK( cerNoError == stream.m_errorReason );
		stream.TestRaiseConnectErrorEvent();
		BOOST_CHECK( false == stream.m_connectErrorEvent );
		BOOST_CHECK( cerNoError == stream.m_errorReason );
		stream.SetConnectErrorEvent( boost::bind( &CTestAbstractNetworkStream::OnConnectError, &stream, _1, _2 ) );
		stream.TestRaiseConnectErrorEvent();
		BOOST_CHECK( true == stream.m_connectErrorEvent );
		BOOST_CHECK( cerUnknown == stream.m_errorReason );
	}
}

test_suite* getCAbstractNetworkStreamTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CAbstractNetworkStreamTestSuite" );
	suite->add( BOOST_TEST_CASE(CAbstractNetworkStreamTestSuite::TestTimeout) );
	suite->add( BOOST_TEST_CASE(CAbstractNetworkStreamTestSuite::TestEvents) );
	return suite;
}
