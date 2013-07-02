/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CDirectNetworkStreamTestSuite.h
///
///  Test suite for CDirectNetworkStream
///
///  @author Dmitry Netrebenko @date 07.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Streaming/CDirectNetworkStream.h>
#include <NWL/Streaming/CSSocket.h>
#include <boostTestLib/boostTestLib.h>
#include <boost/threadpool.hpp>
#include <AidLib/CCritSection/CCritSection.h>

namespace CDirectNetworkStreamTestSuite
{

	tstring GetLocalAddress()
	{
		char name_buf[MAX_PATH];
		memset( name_buf, 0, MAX_PATH );

		tstring addr = _T("");

		// Get host name
		if ( !gethostname( name_buf, MAX_PATH ) )
		{
			// Get host info
			hostent* host = gethostbyname( name_buf );

			if ( host )
			{
				char** addrs = host->h_addr_list;
				if ( *addrs )
				{
					// Get address
					char* str_addr = inet_ntoa( *((struct in_addr *)*addrs) );
					addr = str_addr;
				}
			}
		}
		return addr;
	}

	// Class for testing CDirectNetworkStream
	class CTestDirectNetworkStream
	{
	public:
		CTestDirectNetworkStream()
			:	m_stream1()
			,	m_stream2()
			,	m_port1(0)
			,	m_port2(0)
			,	m_timeout(30000)
			,	m_connectEvent1(false)
			,	m_connectErrorEvent1(false)
			,	m_connectEvent2(false)
			,	m_connectErrorEvent2(false)
			,	m_received(false)
			,	m_threadPool(50)
			,	m_addr(GetLocalAddress())
		{
			m_stream1.SetConnectedEvent( boost::bind( &CTestDirectNetworkStream::OnConnected1, this, _1 ) );
			m_stream1.SetDisconnectedEvent( boost::bind( &CTestDirectNetworkStream::OnDisconnected1, this, _1 ) );
			m_stream1.SetConnectErrorEvent( boost::bind( &CTestDirectNetworkStream::OnConnectError1, this, _1, _2 ) );
			m_stream2.SetConnectedEvent( boost::bind( &CTestDirectNetworkStream::OnConnected2, this, _1 ) );
			m_stream2.SetDisconnectedEvent( boost::bind( &CTestDirectNetworkStream::OnDisconnected2, this, _1 ) );
			m_stream2.SetConnectErrorEvent( boost::bind( &CTestDirectNetworkStream::OnConnectError2, this, _1, _2 ) );

			InitializeCriticalSection(&m_receiveSection);
		}
		~CTestDirectNetworkStream()
		{
			DeleteCriticalSection(&m_receiveSection);
		}
	private:
		boost::threadpool::pool m_threadPool;
		CRITICAL_SECTION m_receiveSection;
	public:
		CDirectNetworkStream m_stream1;
		CDirectNetworkStream m_stream2;
		unsigned int m_port1;
		unsigned int m_port2;
		unsigned int m_timeout;

		bool m_connectEvent1;
		bool m_connectErrorEvent1;
		bool m_connectEvent2;
		bool m_connectErrorEvent2;

		bool m_received;
		int m_receivedCount;

		tstring m_addr;


		void OnConnected1( void* ) { m_connectEvent1 = true; };
		void OnDisconnected1( void* ) { m_connectEvent1 = false; };
		void OnConnectError1( void*, EConnectErrorReason ) { m_connectErrorEvent1 = true; };
		void OnConnected2( void* ) { m_connectEvent2 = true; };
		void OnDisconnected2( void* ) { m_connectEvent2 = false; };
		void OnConnectError2( void*, EConnectErrorReason ) { m_connectErrorEvent2 = true; };

		void DoStreamConnect(CDirectNetworkStream* stream, bool async) 
		{ 
			try
			{
				stream->Connect(async); 
			}
			catch(...)
			{
			}
		};
		void Connect(bool async, bool proxy = false)
		{
			m_stream1.SetRemoteAddr(m_addr, m_port2);
			m_stream1.SetLocalAddr(m_port1);
			m_stream1.GetCredentials().UserID = _T("ThisIsUserID");
			m_stream1.GetCredentials().Key = _T("ThisIsSecretKey");
			m_stream1.SetConnectTimeout(m_timeout);
			if(proxy)
			{
				m_stream1.SetConnectThroughProxy(true);
				m_stream1.GetProxySettings().ProxyURL = PROXY_URL;
				m_stream1.GetProxySettings().ProxyPort = PROXY_PORT;
			}
			m_threadPool.schedule( boost::bind( &CTestDirectNetworkStream::DoStreamConnect, this, &m_stream1, async ) );

			m_stream2.SetRemoteAddr(m_addr, m_port1);
			m_stream2.SetLocalAddr(m_port2);
			m_stream2.GetCredentials().UserID = _T("ThisIsUserID");
			m_stream2.GetCredentials().Key = _T("ThisIsSecretKey");
			m_stream2.SetConnectTimeout(m_timeout);
			if(proxy)
			{
				m_stream2.SetConnectThroughProxy(true);
				m_stream2.GetProxySettings().ProxyURL = PROXY_URL;
				m_stream2.GetProxySettings().ProxyPort = PROXY_PORT;
			}

			/// TODO: check without speep (may be bug)
			Sleep(5000);
			m_threadPool.schedule( boost::bind( &CTestDirectNetworkStream::DoStreamConnect, this, &m_stream2, async ) );
		}

		void Disconnect()
		{
			try
			{
				int buf;
				m_stream1.Disconnect();
				m_stream2.Receive(reinterpret_cast<char*>(&buf), 4);
			}
			catch(...)
			{
			}
		}

		void DoStreamReceiveForCancel(CDirectNetworkStream* stream)
		{
			m_received = false;
			char buf[STREAM_QUEUE_SIZE];
			try
			{
				stream->Receive(buf, STREAM_QUEUE_SIZE);
			}
			catch(...)
			{
			}
			m_received = true;
		}
		void ReceiveForCancel()
		{
			m_threadPool.schedule( boost::bind( &CTestDirectNetworkStream::DoStreamReceiveForCancel, this, &m_stream1 ) );
		}

		void DoAsyncSend(CDirectNetworkStream* stream)
		{
			unsigned int len = static_cast<unsigned int>(strlen("Hello world !!!"));
			stream->Send("Hello world !!!", len);
		}

		void DoAsyncReceive(CDirectNetworkStream* stream)
		{
			CCritSection section(&m_receiveSection);
			char buf[STREAM_QUEUE_SIZE];
			memset(buf, 0, STREAM_QUEUE_SIZE);
			unsigned int len = static_cast<unsigned int>(strlen("Hello world !!!"));
//			Log.Add(_MESSAGE_, _T("Reciving..."));
			stream->Receive(buf, len);
			m_receivedCount++;
			BOOST_CHECK( 0 == memcmp(buf, "Hello world !!!", len) );
//			Log.Add(_MESSAGE_, Format(_T("Recived: %s"), buf).c_str());
		}

		void DoAsyncSendReceive()
		{
			m_receivedCount = 0;
			for(int i = 0; i < 10000; ++i)
			{
				m_threadPool.schedule( boost::bind( &CTestDirectNetworkStream::DoAsyncSend, this, &m_stream1 ) );
				m_threadPool.schedule( boost::bind( &CTestDirectNetworkStream::DoAsyncReceive, this, &m_stream1 ) );
				m_threadPool.schedule( boost::bind( &CTestDirectNetworkStream::DoAsyncSend, this, &m_stream2 ) );
				m_threadPool.schedule( boost::bind( &CTestDirectNetworkStream::DoAsyncReceive, this, &m_stream2 ) );
			}
		}

	};


///<------------------------------------ Test cases ---------------------------------------------


	/// Proxy server settings test case
	void TestProxySettings()
	{
		CDirectNetworkStream stream;
		SHTTPProxySettings sets(_T("ProxyServerAddress"), 8080);
		stream.SetProxySettings(sets);
		BOOST_CHECK( sets.ProxyURL == stream.GetProxySettings().ProxyURL );
		BOOST_CHECK( sets.ProxyPort == stream.GetProxySettings().ProxyPort );
		sets.ProxyURL = _T("New URL");
		stream.GetProxySettings().ProxyPort = 8081;
		BOOST_CHECK( sets.ProxyURL != stream.GetProxySettings().ProxyURL );
		BOOST_CHECK( sets.ProxyPort != stream.GetProxySettings().ProxyPort );

		BOOST_CHECK( false == stream.GetConnectThroughProxy() );
		stream.SetConnectThroughProxy(true);
		BOOST_CHECK( true == stream.GetConnectThroughProxy() );
	}

	/// Remote address test case
	void TestRemoteAddr()
	{
		CDirectNetworkStream stream;
		BOOST_CHECK( _T("") == stream.GetRemoteAddr() );
		BOOST_CHECK( 0 == stream.GetRemotePort() );

		stream.SetRemoteAddr(_T("RemoteAddress"), 123);
		BOOST_CHECK( _T("RemoteAddress") == stream.GetRemoteAddr() );
		BOOST_CHECK( 123 == stream.GetRemotePort() );

		stream.SetRemoteAddr(_T("NewRemoteAddress"), 321);
		BOOST_CHECK( _T("RemoteAddress") != stream.GetRemoteAddr() );
		BOOST_CHECK( 123 != stream.GetRemotePort() );
	}

	/// Local address test case
	void TestLocalAddr()
	{
		CDirectNetworkStream stream;
		BOOST_CHECK( 0 == stream.GetLocalPort() );

		stream.SetLocalAddr(123);
		BOOST_CHECK( 123 == stream.GetLocalPort() );

		stream.SetLocalAddr(321);
		BOOST_CHECK( 123 != stream.GetLocalPort() );
	}

	/// Connect/Disconnect test case
	void TestSyncConnectDisconnect()
	{
		CTestDirectNetworkStream stream;
		stream.m_port1 = DIRECT_PORT_1;
		stream.m_port2 = DIRECT_PORT_2;
		stream.m_timeout = 15000;
		stream.Connect(false);
		
		WAIT_FOR((stream.m_connectEvent1 && stream.m_connectEvent2), 15000)

		BOOST_CHECK( stream.m_connectEvent1 );
		BOOST_CHECK( stream.m_connectEvent2 );
		BOOST_CHECK( stream.m_stream1.Connected() );
		BOOST_CHECK( stream.m_stream2.Connected() );

		if(stream.m_connectEvent1 && stream.m_connectEvent2 && stream.m_stream1.Connected() && stream.m_stream2.Connected() )
		{
			stream.Disconnect();

			WAIT_FOR((!stream.m_connectEvent1 && !stream.m_connectEvent2), 15000)

			BOOST_CHECK( !stream.m_connectEvent1 );
			BOOST_CHECK( !stream.m_connectEvent2 );
			BOOST_CHECK( !stream.m_stream1.Connected() );
			BOOST_CHECK( !stream.m_stream2.Connected() );
		}
	}

	/// Connect/Disconnect test case
	void TestAsyncConnectDisconnect()
	{
		CTestDirectNetworkStream stream;
		stream.m_port1 = DIRECT_PORT_1;
		stream.m_port2 = DIRECT_PORT_2;
		stream.m_timeout = 15000;
		stream.Connect(true);
		
		WAIT_FOR((stream.m_connectEvent1 && stream.m_connectEvent2), 15000)

		BOOST_CHECK( stream.m_connectEvent1 );
		BOOST_CHECK( stream.m_connectEvent2 );
		BOOST_CHECK( stream.m_stream1.Connected() );
		BOOST_CHECK( stream.m_stream2.Connected() );

		if(stream.m_connectEvent1 && stream.m_connectEvent2 && stream.m_stream1.Connected() && stream.m_stream2.Connected() )
		{
			stream.Disconnect();

			WAIT_FOR((!stream.m_connectEvent1 && !stream.m_connectEvent2), 15000)

			BOOST_CHECK( !stream.m_connectEvent1 );
			BOOST_CHECK( !stream.m_connectEvent2 );
			BOOST_CHECK( !stream.m_stream1.Connected() );
			BOOST_CHECK( !stream.m_stream2.Connected() );
		}
	}

	/// Connect/Disconnect test case
	void TestProxyConnectDisconnect()
	{
		CTestDirectNetworkStream stream;
		stream.m_port1 = DIRECT_PORT_1;
		stream.m_port2 = DIRECT_PORT_2;
		stream.m_timeout = 15000;
		stream.Connect(true, true);
		
		WAIT_FOR((stream.m_connectEvent1 && stream.m_connectEvent2), 15000)

		BOOST_CHECK( stream.m_connectEvent1 );
		BOOST_CHECK( stream.m_connectEvent2 );
		BOOST_CHECK( stream.m_stream1.Connected() );
		BOOST_CHECK( stream.m_stream2.Connected() );

		if(stream.m_connectEvent1 && stream.m_connectEvent2 && stream.m_stream1.Connected() && stream.m_stream2.Connected() )
		{
			stream.Disconnect();

			WAIT_FOR((!stream.m_connectEvent1 && !stream.m_connectEvent2), 15000)

			BOOST_CHECK( !stream.m_connectEvent1 );
			BOOST_CHECK( !stream.m_connectEvent2 );
			BOOST_CHECK( !stream.m_stream1.Connected() );
			BOOST_CHECK( !stream.m_stream2.Connected() );
		}
	}

	/// Send/Rceive test case
	void TestSendReceive()
	{
		CTestDirectNetworkStream test_stream;
		test_stream.m_port1 = DIRECT_PORT_1;
		test_stream.m_port2 = DIRECT_PORT_2;
		test_stream.m_timeout = 30000;
		test_stream.Connect(true);

		WAIT_FOR((test_stream.m_connectEvent1 && test_stream.m_connectEvent2), 15000)
		
		BOOST_CHECK( test_stream.m_connectEvent1 );
		BOOST_CHECK( test_stream.m_connectEvent2 );
		BOOST_CHECK( test_stream.m_stream1.Connected() );
		BOOST_CHECK( test_stream.m_stream2.Connected() );

		if(test_stream.m_connectEvent1 && test_stream.m_connectEvent2 && test_stream.m_stream1.Connected() && test_stream.m_stream2.Connected() )
		{
			char buf1[STREAM_QUEUE_SIZE];
			memset(buf1, 0, STREAM_QUEUE_SIZE);
			unsigned int len = static_cast<unsigned int>(strlen("Hello world !!!"));
			memcpy(buf1, "Hello world !!!", len);
			char buf2[STREAM_QUEUE_SIZE];
			memset(buf2, 0, STREAM_QUEUE_SIZE);
			test_stream.m_stream1.Send(buf1, len);
			test_stream.m_stream2.Receive(buf2, len);

			BOOST_CHECK( 0 == memcmp(buf1, buf2, len) );

			test_stream.Disconnect();

			WAIT_FOR((!test_stream.m_connectEvent1 && !test_stream.m_connectEvent2), 15000)

			BOOST_CHECK( !test_stream.m_connectEvent1 );
			BOOST_CHECK( !test_stream.m_connectEvent2 );
			BOOST_CHECK( !test_stream.m_stream1.Connected() );
			BOOST_CHECK( !test_stream.m_stream2.Connected() );
		}
	}

	/// GetInBuffer test case
	void TestGetInBuffer()
	{
		CTestDirectNetworkStream test_stream;
		test_stream.m_port1 = DIRECT_PORT_1;
		test_stream.m_port2 = DIRECT_PORT_2;
		test_stream.m_timeout = 30000;
		test_stream.Connect(true);

		WAIT_FOR((test_stream.m_connectEvent1 && test_stream.m_connectEvent2), 15000)
		
		BOOST_CHECK( test_stream.m_connectEvent1 );
		BOOST_CHECK( test_stream.m_connectEvent2 );
		BOOST_CHECK( test_stream.m_stream1.Connected() );
		BOOST_CHECK( test_stream.m_stream2.Connected() );

		if(test_stream.m_connectEvent1 && test_stream.m_connectEvent2 && test_stream.m_stream1.Connected() && test_stream.m_stream2.Connected() )
		{
			char buf1[STREAM_QUEUE_SIZE];
			memset(buf1, 0, STREAM_QUEUE_SIZE);
			unsigned int len = static_cast<unsigned int>(strlen("Hello world !!!"));
			memcpy(buf1, "Hello world !!!", len);
			char buf2[STREAM_QUEUE_SIZE];
			memset(buf2, 0, STREAM_QUEUE_SIZE);

			test_stream.m_stream1.Send(buf1, len);
			WAIT_FOR((test_stream.m_stream2.HasInData()), 15000)
				test_stream.m_stream2.GetInBuffer(buf2, len);

			BOOST_CHECK( 0 == memcmp(buf1, buf2, len) );

			test_stream.Disconnect();

			WAIT_FOR((!test_stream.m_connectEvent1 && !test_stream.m_connectEvent2), 15000)

			BOOST_CHECK( !test_stream.m_connectEvent1 );
			BOOST_CHECK( !test_stream.m_connectEvent2 );
			BOOST_CHECK( !test_stream.m_stream1.Connected() );
			BOOST_CHECK( !test_stream.m_stream2.Connected() );
		}
	}

	/// CancelReceiveOperation test case
	void TestCancelReceive()
	{
		CTestDirectNetworkStream test_stream;
		test_stream.m_port1 = DIRECT_PORT_1;
		test_stream.m_port2 = DIRECT_PORT_2;
		test_stream.m_timeout = 30000;
		test_stream.Connect(true);

		WAIT_FOR((test_stream.m_connectEvent1 && test_stream.m_connectEvent2), 15000)

		BOOST_CHECK( test_stream.m_connectEvent1 );
		BOOST_CHECK( test_stream.m_connectEvent2 );
		BOOST_CHECK( test_stream.m_stream1.Connected() );
		BOOST_CHECK( test_stream.m_stream2.Connected() );
		

		if(test_stream.m_connectEvent1 && test_stream.m_connectEvent2 && test_stream.m_stream1.Connected() && test_stream.m_stream2.Connected() )
		{
			test_stream.ReceiveForCancel();
			Sleep(3000);
			BOOST_CHECK( !test_stream.m_received );
			test_stream.m_stream1.CancelReceiveOperation();
			Sleep(3000);
			BOOST_CHECK( test_stream.m_received );

			test_stream.Disconnect();
			WAIT_FOR((!test_stream.m_connectEvent1 && !test_stream.m_connectEvent2), 15000)

			BOOST_CHECK( !test_stream.m_connectEvent1 );
			BOOST_CHECK( !test_stream.m_connectEvent2 );
			BOOST_CHECK( !test_stream.m_stream1.Connected() );
			BOOST_CHECK( !test_stream.m_stream2.Connected() );
		}
	}

	/// Async send/receive test case
	void TestAsyncSendReceive()
	{
		CTestDirectNetworkStream test_stream;
		test_stream.m_port1 = DIRECT_PORT_1;
		test_stream.m_port2 = DIRECT_PORT_2;
		test_stream.m_timeout = 30000;
		test_stream.Connect(true);

		WAIT_FOR((test_stream.m_connectEvent1 && test_stream.m_connectEvent2), 15000)
		
		BOOST_CHECK( test_stream.m_connectEvent1 );
		BOOST_CHECK( test_stream.m_connectEvent2 );
		BOOST_CHECK( test_stream.m_stream1.Connected() );
		BOOST_CHECK( test_stream.m_stream2.Connected() );

		if(test_stream.m_connectEvent1 && test_stream.m_connectEvent2 && test_stream.m_stream1.Connected() && test_stream.m_stream2.Connected() )
		{
			test_stream.DoAsyncSendReceive();
			WAIT_FOR((test_stream.m_receivedCount >= 20000), 180000)
			Log.Add(_MESSAGE_, Format(_T("%d messages are received"), test_stream.m_receivedCount).c_str());
		}
	}
}

test_suite* getCDirectNetworkStreamTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CDirectNetworkStreamTestSuite" );
	suite->add( BOOST_TEST_CASE(&CDirectNetworkStreamTestSuite::TestProxySettings) );
	suite->add( BOOST_TEST_CASE(&CDirectNetworkStreamTestSuite::TestRemoteAddr) );
	suite->add( BOOST_TEST_CASE(&CDirectNetworkStreamTestSuite::TestLocalAddr) );
	suite->add( BOOST_TEST_CASE(&CDirectNetworkStreamTestSuite::TestSyncConnectDisconnect) );
	suite->add( BOOST_TEST_CASE(&CDirectNetworkStreamTestSuite::TestAsyncConnectDisconnect) );
	suite->add( BOOST_TEST_CASE(&CDirectNetworkStreamTestSuite::TestProxyConnectDisconnect) );
	suite->add( BOOST_TEST_CASE(&CDirectNetworkStreamTestSuite::TestSendReceive) );
	suite->add( BOOST_TEST_CASE(&CDirectNetworkStreamTestSuite::TestGetInBuffer) );
	suite->add( BOOST_TEST_CASE(&CDirectNetworkStreamTestSuite::TestCancelReceive) );
	suite->add( BOOST_TEST_CASE(&CDirectNetworkStreamTestSuite::TestAsyncSendReceive) );
	return suite;
}
