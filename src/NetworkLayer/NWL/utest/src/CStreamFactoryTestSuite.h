/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStreamFactoryTestSuite.h
///
///  Test suite for CStreamFactory
///
///  @author Dmitry Netrebenko @date 07.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Streaming/CStreamFactory.h>
#include <NWL/Streaming/CStreamFactoryRelayedImpl.h>
#include <boostTestLib/boostTestLib.h>
#include <boost/threadpool.hpp>
#include <NWL/Streaming/CIMStub.h>
#include <NWL/Streaming/CNetworkLayer.h>
#include <AidLib/CSingleton/CSingleton.h>
#include <AidLib/CCritSection/CCritSection.h>

namespace CStreamFactoryTestSuite
{

	///  Map entry
	typedef std::pair<tstring, tstring> MsgMapEntry;
	///  Messages map
	typedef std::map<tstring, tstring> MsgMap;
	/// Message map stub
	class CPeerMsgs
	{
	public:
		CPeerMsgs() 
		{
			InitializeCriticalSection(&m_MapSection);
		};
		~CPeerMsgs() 
		{
			DeleteCriticalSection(&m_MapSection);
		};
	private:
		MsgMap m_MessagesMap;
		CRITICAL_SECTION m_MapSection;
	public:
		void ClearMap()
		{
			CCritSection section(&m_MapSection);
			m_MessagesMap.clear();
		}
		void SendMsg(const tstring& peer, const tstring& msg)
		{
			CCritSection section(&m_MapSection);
			m_MessagesMap[peer] = msg;
		}
		void HandleMsg(const tstring& peer, tstring& msg)
		{
			CCritSection section(&m_MapSection);
			msg = m_MessagesMap[peer];
		}
	};


	/// Class for testing CStreamFactory
	class CTestStreamFactory : public CStreamFactory
	{
	public:
		CTestStreamFactory()
			:	CStreamFactory()
			,	m_connected(false)
			,	m_srcPeer(_T(""))
			,	m_destPeer(_T(""))
			,	m_isMaster(false)
			,	m_threadPool(1)
		{}
		~CTestStreamFactory() 
		{ 
			m_threadPool.wait(); 
		};
	private:
		boost::threadpool::pool m_threadPool;
	protected:
		virtual void SendMsg(const tstring& peerId, const tstring& messageData) 
		{
			CSingleton<CPeerMsgs>::instance().SendMsg(peerId, messageData);
			//Log.Add(_MESSAGE_, Format(_T("SendMsg. Peer: %s, data: %s"), peerId.c_str(), messageData.c_str()).c_str());
		};
		virtual void HandleMsg(const tstring& peerId, tstring& messageData) 
		{
			messageData = _T("");
			int i = 0;
			while((_T("") == messageData) && (i < 30000))
			{
				CSingleton<CPeerMsgs>::instance().HandleMsg(m_srcPeer, messageData);
				i += 100;
				if(_T("") == messageData)
					Sleep(100);
			}
			//Log.Add(_MESSAGE_, Format(_T("HandleMsg. Peer: %s, data: %s"), m_srcPeer.c_str(), messageData.c_str()).c_str());
		};
	public:
		bool m_connected;
		tstring m_srcPeer;
		tstring m_destPeer;
		tstring m_sessionId;
		bool m_isMaster;
		boost::shared_ptr<CAbstractNetworkStream> m_stream;
		virtual void ConnectCompletion(boost::shared_ptr<CAbstractNetworkStream> stream) 
		{
			if(stream.get())
			{
				m_connected = true;
				m_stream = stream;
				m_stream->SetDisconnectedEvent(boost::bind(&CTestStreamFactory::OnStreamDisconnected, this, _1));
			}
		};
		void DoFactoryConnect(bool async)
		{
			boost::shared_ptr<CAbstractNetworkStream> stream = Connect(m_sessionId,m_srcPeer, m_destPeer, 30000, m_isMaster, async);
			if(!async && stream.get())
			{
				m_connected = true;
				m_stream = stream;
				m_stream->SetDisconnectedEvent(boost::bind(&CTestStreamFactory::OnStreamDisconnected, this, _1));
			}
		}
		void DoConnect(bool async = false)
		{
			m_threadPool.schedule( boost::bind( &CTestStreamFactory::DoFactoryConnect, this, async ) );
		}
		void OnStreamDisconnected(void*)
		{
			m_connected = false;
		}
	};


	/// Class for testing CStreamFactoryRelayedImpl
	class CTestStreamFactoryRelayedImpl : public CStreamFactoryRelayedImpl
	{
	public:
		CTestStreamFactoryRelayedImpl()
			:	CStreamFactoryRelayedImpl()
			,	m_connected(false)
			,	m_srcPeer(_T(""))
			,	m_destPeer(_T(""))
			,	m_isMaster(false)
			,	m_threadPool(1)
		{}
		~CTestStreamFactoryRelayedImpl() 
		{
			m_threadPool.wait();
		};
	private:
		boost::threadpool::pool m_threadPool;
	public:
		bool m_connected;
		tstring m_srcPeer;
		tstring m_destPeer;
		tstring m_sessionId;
		bool m_isMaster;
		boost::shared_ptr<CAbstractNetworkStream> m_stream;
		virtual void ConnectCompletion(boost::shared_ptr<CAbstractNetworkStream> stream) 
		{ 
			if(stream.get())
			{
				m_connected = true;
				m_stream = stream;
				m_stream->SetDisconnectedEvent(boost::bind(&CTestStreamFactoryRelayedImpl::OnStreamDisconnected, this, _1));
			}
		};
		void DoFactoryConnect(bool async)
		{
			boost::shared_ptr<CAbstractNetworkStream> stream = Connect(m_sessionId,m_srcPeer, m_destPeer, 30000, m_isMaster, async);
			if(!async && stream.get())
			{
				m_connected = true;
				m_stream = stream;
				m_stream->SetDisconnectedEvent(boost::bind(&CTestStreamFactoryRelayedImpl::OnStreamDisconnected, this, _1));
			}
		}
		void DoConnect(bool async = false)
		{
			m_threadPool.schedule( boost::bind( &CTestStreamFactoryRelayedImpl::DoFactoryConnect, this, async ) );
		}
		void OnStreamDisconnected(void*)
		{
			m_connected = false;
		}
	};

	/// Class for testing CStreamFactory
	template<class T>
	class CTestFactory
	{
	public:
		CTestFactory()
			:	m_stream1()
			,	m_stream2()
		{}
		~CTestFactory() {};
	public:
		T m_stream1;
		T m_stream2;
		void Connect(int index, bool async = false)
		{
			tstring peer1 = Format(_T("Peer_%d_1"), index);
			tstring peer2 = Format(_T("Peer_%d_2"), index);

			NWL_INSTANCE.SetRelayHost(RELAY_SERVER);
			CSingleton<CPeerMsgs>::instance().ClearMap();

			m_stream1.m_srcPeer = peer1;
			m_stream1.m_destPeer = peer2;
			m_stream1.m_sessionId = index;
			m_stream1.m_isMaster = true;
			m_stream1.SetServerUserId(JABBER_USER_NAME_1);
			m_stream1.SetServerPassword(SRV_PASSWORD);
			m_stream1.DoConnect(async);

			m_stream2.m_srcPeer = peer2;
			m_stream2.m_destPeer = peer1;
			m_stream2.m_sessionId = index;
			m_stream2.m_isMaster = false;
			m_stream2.SetServerUserId(JABBER_USER_NAME_2);
			m_stream2.SetServerPassword(SRV_PASSWORD);
			m_stream2.DoConnect(async);

		}
		void Disconnect()
		{
			if(m_stream1.m_stream.get())
				m_stream1.m_stream->Disconnect();
			if(m_stream2.m_stream.get())
				m_stream2.m_stream->Disconnect();
		}
	};

	int GetConnectIndex()
	{
		static int index = 0;
		return ++index;
	}

	/// Connect test case
	template<class T>
	void TestSyncConnect()
	{
		/// initing singleton
		CSingleton<CPeerMsgs>::instance();

		CTestFactory<T> factory;
		factory.Connect(GetConnectIndex(), false);

		WAIT_FOR((factory.m_stream1.m_connected && factory.m_stream2.m_connected), 30000)

		BOOST_CHECK( factory.m_stream1.m_connected );
		BOOST_CHECK( factory.m_stream2.m_connected );
		BOOST_CHECK( NULL != factory.m_stream1.m_stream.get() );
		BOOST_CHECK( NULL != factory.m_stream2.m_stream.get() );

		factory.Disconnect();

		WAIT_FOR((!factory.m_stream1.m_connected && !factory.m_stream2.m_connected), 30000)

		BOOST_CHECK( !factory.m_stream1.m_connected );
		BOOST_CHECK( !factory.m_stream2.m_connected );
	}

	/// Connect test case
	template<class T>
	void TestAsyncConnect()
	{
		CTestFactory<T> factory;
		factory.Connect(GetConnectIndex(), true);

		WAIT_FOR((factory.m_stream1.m_connected && factory.m_stream2.m_connected), 30000)

		BOOST_CHECK( factory.m_stream1.m_connected );
		BOOST_CHECK( factory.m_stream2.m_connected );
		BOOST_CHECK( NULL != factory.m_stream1.m_stream.get() );
		BOOST_CHECK( NULL != factory.m_stream2.m_stream.get() );

		factory.Disconnect();

		WAIT_FOR((!factory.m_stream1.m_connected && !factory.m_stream2.m_connected), 30000)

		BOOST_CHECK( !factory.m_stream1.m_connected );
		BOOST_CHECK( !factory.m_stream2.m_connected );
	}

	/// Send/Receive test case
	template<class T>
	void TestSendReceive()
	{
		CTestFactory<T> factory;
		factory.Connect(GetConnectIndex(), true);

		WAIT_FOR((factory.m_stream1.m_connected && factory.m_stream2.m_connected), 30000)

		BOOST_CHECK( factory.m_stream1.m_connected );
		BOOST_CHECK( factory.m_stream2.m_connected );
		BOOST_CHECK( NULL != factory.m_stream1.m_stream.get() );
		BOOST_CHECK( NULL != factory.m_stream2.m_stream.get() );

		char buf1[STREAM_QUEUE_SIZE];
		memset(buf1, 0, STREAM_QUEUE_SIZE);
		char buf2[STREAM_QUEUE_SIZE];
		memset(buf2, 0, STREAM_QUEUE_SIZE);
		unsigned int len = static_cast<unsigned int>(strlen("Hello world !!!"));
		memcpy(buf1, "Hello world !!!", len);

		factory.m_stream1.m_stream->Send(buf1, len);
		factory.m_stream2.m_stream->Receive(buf2, len);
		BOOST_CHECK( 0 == memcmp(buf1, buf2, len) );

		factory.Disconnect();

		WAIT_FOR((!factory.m_stream1.m_connected && !factory.m_stream2.m_connected), 30000)

		BOOST_CHECK( !factory.m_stream1.m_connected );
		BOOST_CHECK( !factory.m_stream2.m_connected );
	}
}

test_suite* getCStreamFactoryTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CStreamFactoryTestSuite" );

	suite->add( BOOST_TEST_CASE(&CStreamFactoryTestSuite::TestSyncConnect<CStreamFactoryTestSuite::CTestStreamFactoryRelayedImpl>) );
	suite->add( BOOST_TEST_CASE(&CStreamFactoryTestSuite::TestAsyncConnect<CStreamFactoryTestSuite::CTestStreamFactoryRelayedImpl>) );
	suite->add( BOOST_TEST_CASE(&CStreamFactoryTestSuite::TestSendReceive<CStreamFactoryTestSuite::CTestStreamFactoryRelayedImpl>) );

	suite->add( BOOST_TEST_CASE(&CStreamFactoryTestSuite::TestSyncConnect<CStreamFactoryTestSuite::CTestStreamFactory>) );
	suite->add( BOOST_TEST_CASE(&CStreamFactoryTestSuite::TestAsyncConnect<CStreamFactoryTestSuite::CTestStreamFactory>) );
	suite->add( BOOST_TEST_CASE(&CStreamFactoryTestSuite::TestSendReceive<CStreamFactoryTestSuite::CTestStreamFactory>) );

	return suite;
}
