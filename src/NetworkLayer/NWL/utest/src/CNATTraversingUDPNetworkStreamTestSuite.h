/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNATTraversingUDPNetworkStreamTestSuite.h
///
///  Test suite for CNATTraversingUDPNetworkStream
///
///  @author Dmitry Netrebenko @date 07.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Streaming/CNATTraversingUDPNetworkStream.h>
#include <boostTestLib/boostTestLib.h>
#include <boost/threadpool.hpp>

namespace CNATTraversingUDPNetworkStreamTestSuite
{
	/// Class for testing CNATTraversingUDPNetworkStream
	class CTestNATTraversingUDPNetworkStream
	{
	public:
		CTestNATTraversingUDPNetworkStream() 
			:	m_stream1()
			,	m_stream2()
			,	m_threadPool(10)
			,	m_timeout(30000)
			,	m_connectEvent1(false)
			,	m_connectErrorEvent1(false)
			,	m_connectEvent2(false)
			,	m_connectErrorEvent2(false)
		{
			m_stream1.SetConnectedEvent( boost::bind( &CTestNATTraversingUDPNetworkStream::OnConnected1, this, _1 ) );
			m_stream1.SetDisconnectedEvent( boost::bind( &CTestNATTraversingUDPNetworkStream::OnDisconnected1, this, _1 ) );
			m_stream1.SetConnectErrorEvent( boost::bind( &CTestNATTraversingUDPNetworkStream::OnConnectError1, this, _1, _2 ) );
			m_stream2.SetConnectedEvent( boost::bind( &CTestNATTraversingUDPNetworkStream::OnConnected2, this, _1 ) );
			m_stream2.SetDisconnectedEvent( boost::bind( &CTestNATTraversingUDPNetworkStream::OnDisconnected2, this, _1 ) );
			m_stream2.SetConnectErrorEvent( boost::bind( &CTestNATTraversingUDPNetworkStream::OnConnectError2, this, _1, _2 ) );
			InitializeCriticalSection(&m_receiveSection);
		};
		~CTestNATTraversingUDPNetworkStream()
		{
			DeleteCriticalSection(&m_receiveSection);
		}

	private:
		boost::threadpool::pool m_threadPool;
		CRITICAL_SECTION m_receiveSection;
	public:
		CNATTraversingUDPNetworkStream m_stream1;
		CNATTraversingUDPNetworkStream m_stream2;
		unsigned int m_timeout;

		bool m_connectEvent1;
		bool m_connectErrorEvent1;
		bool m_connectEvent2;
		bool m_connectErrorEvent2;
		bool m_received;
		int m_receivedCount;

		void OnConnected1( void* ) { m_connectEvent1 = true; };
		void OnDisconnected1( void* ) { m_connectEvent1 = false; };
		void OnConnectError1( void*, EConnectErrorReason ) { m_connectErrorEvent1 = true; };
		void OnConnected2( void* ) { m_connectEvent2 = true; };
		void OnDisconnected2( void* ) { m_connectEvent2 = false; };
		void OnConnectError2( void*, EConnectErrorReason ) { m_connectErrorEvent2 = true; };

		void DoStreamConnect(CNATTraversingUDPNetworkStream* stream, bool async) 
		{ 
			try
			{
				stream->Connect(async); 
			}
			catch(...)
			{
			}
		};
		void Connect(bool async, int index)
		{
			tstring connectId = Format(_T("Connect_%d"), index);
			tstring peer1 = Format(_T("Peer_%d_1"), index);
			tstring peer2 = Format(_T("Peer_%d_2"), index);

			m_stream1.GetCredentials().UserID = _T("ThisIsUserID");
			m_stream1.GetCredentials().Key = _T("ThisIsSecretKey");
			m_stream1.SetConnectTimeout(m_timeout);
			m_stream1.SetRelayServer(RELAY_SERVER, RELAY_UDP_PORT, JABBER_USER_NAME_1, SRV_PASSWORD);
			m_stream1.SetConnectionId(connectId, peer1, peer2);
			m_stream1.SetIsMaster(true);
			m_stream1.SetBindRetry(1000, 5);
			m_stream1.SetProbeRetry(1000, 3);
			m_stream1.SetProbePortRange(3);
			m_stream1.SetAuthRetry(1000, 3);
			m_threadPool.schedule( boost::bind( &CTestNATTraversingUDPNetworkStream::DoStreamConnect, this, &m_stream1, async ) );

			m_stream2.GetCredentials().UserID = _T("ThisIsUserID");
			m_stream2.GetCredentials().Key = _T("ThisIsSecretKey");
			m_stream2.SetConnectTimeout(m_timeout);
			m_stream2.SetRelayServer(RELAY_SERVER, RELAY_UDP_PORT, JABBER_USER_NAME_2, SRV_PASSWORD);
			m_stream2.SetConnectionId(connectId, peer2, peer1);
			m_stream2.SetIsMaster(false);
			m_stream2.SetBindRetry(1000, 5);
			m_stream2.SetProbeRetry(1000, 3);
			m_stream2.SetProbePortRange(3);
			m_stream2.SetAuthRetry(1000, 3);

			m_threadPool.schedule( boost::bind( &CTestNATTraversingUDPNetworkStream::DoStreamConnect, this, &m_stream2, async ) );
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

		void DoStreamReceiveForCancel(CNATTraversingUDPNetworkStream* stream)
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
			m_threadPool.schedule( boost::bind( &CTestNATTraversingUDPNetworkStream::DoStreamReceiveForCancel, this, &m_stream1 ) );
		}

		void DoAsyncSend(CNATTraversingUDPNetworkStream* stream)
		{
			unsigned int len = static_cast<unsigned int>(strlen("Hello world !!!"));
			stream->Send("Hello world !!!", len);
		}

		void DoAsyncReceive(CNATTraversingUDPNetworkStream* stream)
		{
			CCritSection section(&m_receiveSection);
			TRY_CATCH
				char buf[STREAM_QUEUE_SIZE];
				memset(buf, 0, STREAM_QUEUE_SIZE);
				unsigned int len = static_cast<unsigned int>(strlen("Hello world !!!"));
//				Log.Add(_MESSAGE_, _T("Reciving..."));
				stream->Receive(buf, len);
				m_receivedCount++;
				BOOST_CHECK( 0 == memcmp(buf, "Hello world !!!", len) );
//				Log.Add(_MESSAGE_, Format(_T("Recived: %s"), buf).c_str());
			CATCH_LOG()
		}

		void DoAsyncSendReceive()
		{
			m_receivedCount = 0;
			for(int i = 0; i < 10000; ++i)
			{
				m_threadPool.schedule( boost::bind( &CTestNATTraversingUDPNetworkStream::DoAsyncSend, this, &m_stream1 ) );
				m_threadPool.schedule( boost::bind( &CTestNATTraversingUDPNetworkStream::DoAsyncReceive, this, &m_stream2 ) );
				m_threadPool.schedule( boost::bind( &CTestNATTraversingUDPNetworkStream::DoAsyncSend, this, &m_stream2 ) );
				m_threadPool.schedule( boost::bind( &CTestNATTraversingUDPNetworkStream::DoAsyncReceive, this, &m_stream1 ) );
			}
		}
	};

	/// Relay connect settings test case
	void TestRelayConnectSettings()
	{
		CNATTraversingUDPNetworkStream stream;

		BOOST_CHECK( _T("") == stream.GetServerAddress() );
		BOOST_CHECK( 0 == stream.GetServerPort() );
		BOOST_CHECK( _T("") == stream.GetSrvUserId() );
		BOOST_CHECK( _T("") == stream.GetSrvPassword() );
		BOOST_CHECK( _T("") == stream.GetConnectId() );
		BOOST_CHECK( _T("") == stream.GetLocalPeerId() );
		BOOST_CHECK( _T("") == stream.GetRemotePeerId() );
		BOOST_CHECK( false == stream.GetIsMaster() );

		stream.SetRelayServer(_T("address"), 25000, _T("UserName"), _T("UserPassword"));
		stream.SetConnectionId(_T("ConnectId"), _T("LocalPeer"), _T("RemotePeer"));
		stream.SetIsMaster(true);

		BOOST_CHECK( _T("address") == stream.GetServerAddress() );
		BOOST_CHECK( 25000 == stream.GetServerPort() );
		BOOST_CHECK( _T("UserName") == stream.GetSrvUserId() );
		BOOST_CHECK( _T("UserPassword") == stream.GetSrvPassword() );
		BOOST_CHECK( _T("ConnectId") == stream.GetConnectId() );
		BOOST_CHECK( _T("LocalPeer") == stream.GetLocalPeerId() );
		BOOST_CHECK( _T("RemotePeer") == stream.GetRemotePeerId() );
		BOOST_CHECK( true == stream.GetIsMaster() );
	}

	/// Stun connect settings test case
	void TestStunConnectSettings()
	{
		CNATTraversingUDPNetworkStream stream;
		
		BOOST_CHECK( 5000 == stream.GetBindRetryDelay() );
		BOOST_CHECK( 3 == stream.GetBindMaxRetryCount() );
		BOOST_CHECK( 5000 == stream.GetProbeRetryDelay() );
		BOOST_CHECK( 3 == stream.GetProbeMaxRetryCount() );
		BOOST_CHECK( 3 == stream.GetProbePortRange() );
		BOOST_CHECK( 5000 == stream.GetAuthRetryDelay() );
		BOOST_CHECK( 3 == stream.GetAuthMaxRetryCount() );

		stream.SetBindRetry(1000, 1);
		stream.SetProbeRetry(2000, 2);
		stream.SetProbePortRange(4);
		stream.SetAuthRetry(3000, 5);

		BOOST_CHECK( 1000 == stream.GetBindRetryDelay() );
		BOOST_CHECK( 1 == stream.GetBindMaxRetryCount() );
		BOOST_CHECK( 2000 == stream.GetProbeRetryDelay() );
		BOOST_CHECK( 2 == stream.GetProbeMaxRetryCount() );
		BOOST_CHECK( 4 == stream.GetProbePortRange() );
		BOOST_CHECK( 3000 == stream.GetAuthRetryDelay() );
		BOOST_CHECK( 5 == stream.GetAuthMaxRetryCount() );
	}

	/// Connect/Disconnect test case
	void TestSyncConnectDisconnect()
	{
		CTestNATTraversingUDPNetworkStream stream;
		stream.m_timeout = 15000;
		stream.Connect(false,1);

		WAIT_FOR((stream.m_connectEvent1 && stream.m_connectEvent2), 15000)

		BOOST_CHECK( stream.m_stream1.Connected() );
		BOOST_CHECK( stream.m_stream2.Connected() );
		BOOST_CHECK( stream.m_connectEvent1 );
		BOOST_CHECK( stream.m_connectEvent2 );

		if(stream.m_connectEvent1 && stream.m_connectEvent2 && stream.m_stream1.Connected() && stream.m_stream2.Connected() )
		{
			stream.Disconnect();

			WAIT_FOR((!stream.m_connectEvent1 && !stream.m_connectEvent2), 15000)

			BOOST_CHECK( !stream.m_stream1.Connected() );
			BOOST_CHECK( !stream.m_stream2.Connected() );
			BOOST_CHECK( !stream.m_connectEvent1 );
			BOOST_CHECK( !stream.m_connectEvent2 );
		}
	}

	/// Connect/Disconnect test case
	void TestAsyncConnectDisconnect()
	{
		CTestNATTraversingUDPNetworkStream stream;
		stream.m_timeout = 15000;
		stream.Connect(true,2);

		WAIT_FOR((stream.m_connectEvent1 && stream.m_connectEvent2), 15000)

		BOOST_CHECK( stream.m_stream1.Connected() );
		BOOST_CHECK( stream.m_stream2.Connected() );
		BOOST_CHECK( stream.m_connectEvent1 );
		BOOST_CHECK( stream.m_connectEvent2 );

		if(stream.m_connectEvent1 && stream.m_connectEvent2 && stream.m_stream1.Connected() && stream.m_stream2.Connected() )
		{
			stream.Disconnect();

			WAIT_FOR((!stream.m_connectEvent1 && !stream.m_connectEvent2), 15000)

			BOOST_CHECK( !stream.m_stream1.Connected() );
			BOOST_CHECK( !stream.m_stream2.Connected() );
			BOOST_CHECK( !stream.m_connectEvent1 );
			BOOST_CHECK( !stream.m_connectEvent2 );
		}
	}

	/// Send/Rceive test case
	void TestSendReceive()
	{
		CTestNATTraversingUDPNetworkStream test_stream;
		test_stream.m_timeout = 15000;
		test_stream.Connect(true,3);

		WAIT_FOR((test_stream.m_connectEvent1 && test_stream.m_connectEvent2), 15000)

		BOOST_CHECK( test_stream.m_stream1.Connected() );
		BOOST_CHECK( test_stream.m_stream2.Connected() );
		BOOST_CHECK( test_stream.m_connectEvent1 );
		BOOST_CHECK( test_stream.m_connectEvent2 );

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

			BOOST_CHECK( !test_stream.m_stream1.Connected() );
			BOOST_CHECK( !test_stream.m_stream2.Connected());
			BOOST_CHECK( !test_stream.m_connectEvent1 );
			BOOST_CHECK( !test_stream.m_connectEvent2 );
		}
	}

	/// GetInBuffer test case
	void TestGetInBuffer()
	{
		CTestNATTraversingUDPNetworkStream test_stream;
		test_stream.m_timeout = 15000;
		test_stream.Connect(true,4);
		
		WAIT_FOR((test_stream.m_connectEvent1 && test_stream.m_connectEvent2), 15000)

		BOOST_CHECK( test_stream.m_stream1.Connected() );
		BOOST_CHECK( test_stream.m_stream2.Connected());
		BOOST_CHECK( test_stream.m_connectEvent1 );
		BOOST_CHECK( test_stream.m_connectEvent2 );

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

			BOOST_CHECK( !test_stream.m_stream1.Connected() );
			BOOST_CHECK( !test_stream.m_stream2.Connected());
			BOOST_CHECK( !test_stream.m_connectEvent1 );
			BOOST_CHECK( !test_stream.m_connectEvent2 );
		}
	}

	/// CancelReceiveOperation test case
	void TestCancelReceive()
	{
		CTestNATTraversingUDPNetworkStream test_stream;
		test_stream.m_timeout = 15000;
		test_stream.Connect(true,5);

		WAIT_FOR((test_stream.m_connectEvent1 && test_stream.m_connectEvent2), 15000)

		BOOST_CHECK( test_stream.m_stream1.Connected() );
		BOOST_CHECK( test_stream.m_stream2.Connected());
		BOOST_CHECK( test_stream.m_connectEvent1 );
		BOOST_CHECK( test_stream.m_connectEvent2 );

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
			BOOST_CHECK( !test_stream.m_stream1.Connected() );
			BOOST_CHECK( !test_stream.m_stream2.Connected());
			BOOST_CHECK( !test_stream.m_connectEvent1 );
			BOOST_CHECK( !test_stream.m_connectEvent2 );
		}
	}

	/// Async send/receive test case
	void TestAsyncSendReceive()
	{
		CTestNATTraversingUDPNetworkStream test_stream;
		test_stream.m_timeout = 15000;
		test_stream.Connect(true,6);

		WAIT_FOR((test_stream.m_connectEvent1 && test_stream.m_connectEvent2), 15000)
		
		BOOST_CHECK( test_stream.m_stream1.Connected() );
		BOOST_CHECK( test_stream.m_stream2.Connected() );
		BOOST_CHECK( test_stream.m_connectEvent1 );
		BOOST_CHECK( test_stream.m_connectEvent2 );

		if(test_stream.m_connectEvent1 && test_stream.m_connectEvent2 && test_stream.m_stream1.Connected() && test_stream.m_stream2.Connected() )
		{
			test_stream.DoAsyncSendReceive();
			WAIT_FOR((test_stream.m_receivedCount >= 20000), 180000)
			Log.Add(_MESSAGE_, Format(_T("%d messages are received"), test_stream.m_receivedCount).c_str());
		}
	}
}

test_suite* getCNATTraversingUDPNetworkStreamTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CNATTraversingUDPNetworkStreamTestSuite" );
	suite->add( BOOST_TEST_CASE(&CNATTraversingUDPNetworkStreamTestSuite::TestRelayConnectSettings) );
	suite->add( BOOST_TEST_CASE(&CNATTraversingUDPNetworkStreamTestSuite::TestStunConnectSettings) );
	suite->add( BOOST_TEST_CASE(&CNATTraversingUDPNetworkStreamTestSuite::TestSyncConnectDisconnect) );
	suite->add( BOOST_TEST_CASE(&CNATTraversingUDPNetworkStreamTestSuite::TestAsyncConnectDisconnect) );
	suite->add( BOOST_TEST_CASE(&CNATTraversingUDPNetworkStreamTestSuite::TestSendReceive) );
	suite->add( BOOST_TEST_CASE(&CNATTraversingUDPNetworkStreamTestSuite::TestGetInBuffer) );
	suite->add( BOOST_TEST_CASE(&CNATTraversingUDPNetworkStreamTestSuite::TestCancelReceive) );
	suite->add( BOOST_TEST_CASE(&CNATTraversingUDPNetworkStreamTestSuite::TestAsyncSendReceive) );
	return suite;
}
