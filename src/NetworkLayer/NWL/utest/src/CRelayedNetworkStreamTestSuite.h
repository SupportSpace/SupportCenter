/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRelayedNetworkStreamTestSuite.h
///
///  Test suite for CRelayedNetworkStream
///
///  @author Dmitry Netrebenko @date 07.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Streaming/CRelayedNetworkStream.h>
#include <boostTestLib/boostTestLib.h>
#include <boost/threadpool.hpp>
#include <NWL/Streaming/CNetworkLayer.h>
#include <AidLib/CSingleton/CSingleton.h>
#include <NWL/Streaming/CTLSSocketStream.h>
#include <NWL/Streaming/CRSASocketStream.h>

namespace CRelayedNetworkStreamTestSuite
{
	/// Class for testing CRelayedNetworkStream
	template<class T>
	class CTestRelayedNetworkStream
	{
	public:
		CTestRelayedNetworkStream() 
			:	m_stream1()
			,	m_stream2()
			,	m_threadPool(50)
			,	m_timeout(30000)
			,	m_connectEvent1(false)
			,	m_connectErrorEvent1(false)
			,	m_connectEvent2(false)
			,	m_connectErrorEvent2(false)
		{
			m_stream1.SetIsMaster(true);
			m_stream1.SetConnectedEvent( boost::bind( &CTestRelayedNetworkStream::OnConnected1, this, _1 ) );
			m_stream1.SetDisconnectedEvent( boost::bind( &CTestRelayedNetworkStream::OnDisconnected1, this, _1 ) );
			m_stream1.SetConnectErrorEvent( boost::bind( &CTestRelayedNetworkStream::OnConnectError1, this, _1, _2 ) );
			m_stream2.SetIsMaster(false);
			m_stream2.SetConnectedEvent( boost::bind( &CTestRelayedNetworkStream::OnConnected2, this, _1 ) );
			m_stream2.SetDisconnectedEvent( boost::bind( &CTestRelayedNetworkStream::OnDisconnected2, this, _1 ) );
			m_stream2.SetConnectErrorEvent( boost::bind( &CTestRelayedNetworkStream::OnConnectError2, this, _1, _2 ) );

			InitializeCriticalSection(&m_receiveSection);
		};
		~CTestRelayedNetworkStream()
		{
			DeleteCriticalSection(&m_receiveSection);
		}

	private:
		boost::threadpool::pool m_threadPool;
		CRITICAL_SECTION m_receiveSection;
	public:
		CRelayedNetworkStream<T> m_stream1;
		CRelayedNetworkStream<T> m_stream2;
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

		void DoStreamConnect(CRelayedNetworkStream<T>* stream) 
		{ 
			try
			{
				stream->Connect(); 
			}
			catch(...)
			{
			}
		};
		void Connect(int index, bool proxy = false)
		{
			tstring connectId = Format(_T("Connect_%d"), index);
			tstring peer1 = Format(_T("Peer_%d_1"), index);
			tstring peer2 = Format(_T("Peer_%d_2"), index);

			NWL_INSTANCE.SetRelayHost(RELAY_SERVER);

			CTLSSocketStream* tlsStream1 = dynamic_cast<CTLSSocketStream*>(&m_stream1);
			if(tlsStream1)
			{
				tlsStream1->GetCredentials().UserID = _T("ThisIsUserID");
				tlsStream1->GetCredentials().Key = _T("ThisIsSecretKey");
			}

			m_stream1.SetConnectTimeout(m_timeout);
			m_stream1.SetRelayServer(RELAY_SERVER, RELAY_TCP_PORT, JABBER_USER_NAME_1, SRV_PASSWORD);
			m_stream1.SetConnectionId(connectId, peer1, peer2);
			if(proxy)
			{
				m_stream1.SetConnectThroughProxy(true);
				m_stream1.GetProxySettings().ProxyURL = PROXY_URL;
				m_stream1.GetProxySettings().ProxyPort = PROXY_PORT;
			}
			m_threadPool.schedule( boost::bind( &CTestRelayedNetworkStream::DoStreamConnect, this, &m_stream1 ) );

			CTLSSocketStream* tlsStream2 = dynamic_cast<CTLSSocketStream*>(&m_stream2);
			if(tlsStream2)
			{
				tlsStream2->GetCredentials().UserID = _T("ThisIsUserID");
				tlsStream2->GetCredentials().Key = _T("ThisIsSecretKey");
			}

			m_stream2.SetConnectTimeout(m_timeout);
			m_stream2.SetRelayServer(RELAY_SERVER, RELAY_TCP_PORT, JABBER_USER_NAME_2, SRV_PASSWORD);
			m_stream2.SetConnectionId(connectId, peer2, peer1);
			if(proxy)
			{
				m_stream2.SetConnectThroughProxy(true);
				m_stream2.GetProxySettings().ProxyURL = PROXY_URL;
				m_stream2.GetProxySettings().ProxyPort = PROXY_PORT;
			}

			Sleep(5000);
			m_threadPool.schedule( boost::bind( &CTestRelayedNetworkStream::DoStreamConnect, this, &m_stream2 ) );
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

		void DoStreamReceiveForCancel(CRelayedNetworkStream<T>* stream)
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
			m_threadPool.schedule( boost::bind( &CTestRelayedNetworkStream::DoStreamReceiveForCancel, this, &m_stream1 ) );
		}

		void DoAsyncSend(CRelayedNetworkStream<T>* stream)
		{
			unsigned int len = static_cast<unsigned int>(strlen("Hello world !!!"));
			stream->Send("Hello world !!!", len);
		}

		void DoAsyncReceive(CRelayedNetworkStream<T>* stream)
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
			for(int i = 0; i < 5000; ++i)
			{
				m_threadPool.schedule( boost::bind( &CTestRelayedNetworkStream::DoAsyncSend, this, &m_stream1 ) );
				m_threadPool.schedule( boost::bind( &CTestRelayedNetworkStream::DoAsyncReceive, this, &m_stream1 ) );
				m_threadPool.schedule( boost::bind( &CTestRelayedNetworkStream::DoAsyncSend, this, &m_stream2 ) );
				m_threadPool.schedule( boost::bind( &CTestRelayedNetworkStream::DoAsyncReceive, this, &m_stream2 ) );
			}
		}

	};

	int GetConnectIndex()
	{
		static int idx = 0;
		return ++idx;
	}

	/// Relay connect settings test case
	template<class T>
	void TestRelayConnectSettings()
	{
		CRelayedNetworkStream<T> stream;

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

	/// Connect/Disconnect test case
	template<class T>
	void TestConnectDisconnect()
	{
		CTestRelayedNetworkStream<T> stream;
		stream.m_timeout = 15000;
		stream.Connect(GetConnectIndex());

		WAIT_FOR((stream.m_stream1.Connected() && stream.m_stream2.Connected()), 15000)

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
	template<class T>
	void TestProxyConnectDisconnect()
	{
		CTestRelayedNetworkStream<T> stream;
		stream.m_timeout = 15000;
		stream.Connect(GetConnectIndex(), true);

		WAIT_FOR((stream.m_stream1.Connected() && stream.m_stream2.Connected()), 15000)

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
	template<class T>
	void TestSendReceive()
	{
		CTestRelayedNetworkStream<T> test_stream;
		test_stream.m_timeout = 15000;
		test_stream.Connect(GetConnectIndex());

		WAIT_FOR((test_stream.m_stream1.Connected() && test_stream.m_stream2.Connected()), 15000)

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
	template<class T>
	void TestGetInBuffer()
	{
		CTestRelayedNetworkStream<T> test_stream;
		test_stream.m_timeout = 15000;
		test_stream.Connect(GetConnectIndex());
		
		WAIT_FOR((test_stream.m_stream1.Connected() && test_stream.m_stream2.Connected()), 15000)

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
	template<class T>
	void TestCancelReceive()
	{
		CTestRelayedNetworkStream<T> test_stream;
		test_stream.m_timeout = 15000;
		test_stream.Connect(GetConnectIndex());

		WAIT_FOR((test_stream.m_stream1.Connected() && test_stream.m_stream2.Connected()), 15000)

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
	template<class T>
	void TestAsyncSendReceive()
	{
		CTestRelayedNetworkStream<T> test_stream;
		test_stream.m_timeout = 15000;
		test_stream.Connect(GetConnectIndex());

		WAIT_FOR((test_stream.m_stream1.Connected() && test_stream.m_stream2.Connected()), 15000)

		BOOST_CHECK( test_stream.m_stream1.Connected() );
		BOOST_CHECK( test_stream.m_stream2.Connected());
		BOOST_CHECK( test_stream.m_connectEvent1 );
		BOOST_CHECK( test_stream.m_connectEvent2 );

		if(test_stream.m_connectEvent1 && test_stream.m_connectEvent2 && test_stream.m_stream1.Connected() && test_stream.m_stream2.Connected() )
		{
			test_stream.DoAsyncSendReceive();
			WAIT_FOR((test_stream.m_receivedCount >= 10000), 180000)
			Log.Add(_MESSAGE_, Format(_T("%d messages are received"), test_stream.m_receivedCount).c_str());

			test_stream.Disconnect();

			WAIT_FOR((!test_stream.m_connectEvent1 && !test_stream.m_connectEvent2), 15000)
			
			BOOST_CHECK( !test_stream.m_stream1.Connected() );
			BOOST_CHECK( !test_stream.m_stream2.Connected());
			BOOST_CHECK( !test_stream.m_connectEvent1 );
			BOOST_CHECK( !test_stream.m_connectEvent2 );
		}
	}
};

test_suite* getCRelayedNetworkStreamTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CRelayedNetworkStreamTestSuite" );
	
	suite->add( BOOST_TEST_CASE(&CRelayedNetworkStreamTestSuite::TestRelayConnectSettings<CTLSSocketStream>) );
	suite->add( BOOST_TEST_CASE(&CRelayedNetworkStreamTestSuite::TestConnectDisconnect<CTLSSocketStream>) );
	suite->add( BOOST_TEST_CASE(&CRelayedNetworkStreamTestSuite::TestProxyConnectDisconnect<CTLSSocketStream>) );
	suite->add( BOOST_TEST_CASE(&CRelayedNetworkStreamTestSuite::TestSendReceive<CTLSSocketStream>) );
	suite->add( BOOST_TEST_CASE(&CRelayedNetworkStreamTestSuite::TestGetInBuffer<CTLSSocketStream>) );
	suite->add( BOOST_TEST_CASE(&CRelayedNetworkStreamTestSuite::TestCancelReceive<CTLSSocketStream>) );
	suite->add( BOOST_TEST_CASE(&CRelayedNetworkStreamTestSuite::TestAsyncSendReceive<CTLSSocketStream>) );

	suite->add( BOOST_TEST_CASE(&CRelayedNetworkStreamTestSuite::TestRelayConnectSettings<CRSASocketStream>) );
	suite->add( BOOST_TEST_CASE(&CRelayedNetworkStreamTestSuite::TestConnectDisconnect<CRSASocketStream>) );
	suite->add( BOOST_TEST_CASE(&CRelayedNetworkStreamTestSuite::TestProxyConnectDisconnect<CRSASocketStream>) );
	suite->add( BOOST_TEST_CASE(&CRelayedNetworkStreamTestSuite::TestSendReceive<CRSASocketStream>) );
	suite->add( BOOST_TEST_CASE(&CRelayedNetworkStreamTestSuite::TestGetInBuffer<CRSASocketStream>) );
	suite->add( BOOST_TEST_CASE(&CRelayedNetworkStreamTestSuite::TestCancelReceive<CRSASocketStream>) );
	suite->add( BOOST_TEST_CASE(&CRelayedNetworkStreamTestSuite::TestAsyncSendReceive<CRSASocketStream>) );

	return suite;
}
