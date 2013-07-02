/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStressTest.h
///
///  Declares CStressTest class, responsible for stress testing
///
///  @author Dmitry Netrebenko @date 04.10.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef BOOST_HAS_WINTHREADS 
	#define BOOST_HAS_WINTHREADS
#endif
#ifndef BOOST_THREAD_USE_LIB 
	#define BOOST_THREAD_USE_LIB
#endif
#ifndef BOOST_THREAD_NO_LIB
	#define BOOST_THREAD_NO_LIB
#endif

#include <winsock2.h>
#include <NWL/Streaming/CSocketSystem.h>
#include <NWL/TLS/CTLSSystem.h>
#include <AidLib/Strings/tstring.h>
#include <map>
#include <boost/threadpool.hpp>
#include <AidLib/CException/CException.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "CSettings.h"

/// Thread information structure
struct ThreadInfo
{
	int			thread_id;		/// Thread Id
	tstring		connect_id;		/// Connection Id
	tstring		local_peer_id;	/// Local peer name
	tstring		remote_peer_id;	/// Remote peer name
	bool		master;			/// Is master
/// Default constructor
	ThreadInfo()
		:	master(false)
		,	thread_id(0)
		,	connect_id(_T(""))
		,	local_peer_id(_T(""))
		,	remote_peer_id(_T(""))
	{};
/// Initial constructor
	ThreadInfo(const bool mstr, const int& id, const tstring& connect, const tstring& local, const tstring& remote)
		:	master(mstr)
		,	thread_id(id)
		,	connect_id(connect)
		,	local_peer_id(local)
		,	remote_peer_id(remote)
	{};
};

/// Containers for storing thread information
typedef std::pair<int, ThreadInfo> ThreadInfoEntry;
typedef std::map<int, ThreadInfo> ThreadsInfo;

///  CStressTest class, responsible for stress testing
class CStressTest
{
private:
/// Prevents making copies of CStressTest objects
	CStressTest(const CStressTest&);
	CStressTest& operator=(const CStressTest&);
public:
/// Constructor
/// @param - pool size
/// @param - peers count
	CStressTest(int, int);
/// Destructor
	~CStressTest();
/// Starts test	
	template<class T>
	void Start()
	{
	TRY_CATCH
		for(int index = 0; index < m_PeersCount; index++ )
			m_Pool.schedule( boost::bind( &CStressTest::ThreadEntryPoint<T>, this ) );
		m_Pool.wait();
	CATCH_THROW()
	}

private:
/// Count of threads
	int							m_ThreadsCount;
/// Count of peers
	int							m_PeersCount;
/// Threads pool
	boost::threadpool::pool		m_Pool;
/// Map with thread information
	ThreadsInfo					m_Info;
/// Critical section to access map
	CRITICAL_SECTION			m_InfoSection;
/// Thread Id
	int							m_ThreadId;

private:
/// Creates map with threads information
	ThreadInfo GetThreadInfo();
/// Entry point for connection thread
	template<class T>
	void ThreadEntryPoint()
	{
	TRY_CATCH

		ThreadInfo info = GetThreadInfo();
		if( info.connect_id == _T("") )
		{
			printf(Format("Invalid thread info for thread %d\n",info.thread_id).c_str());
			MCException(_T("Invalid thread info"));
		}

		//Stream stream;
		T stream;
		stream.SetIsMaster(info.master);
		CAbstractStream* as = static_cast<CAbstractStream*>(&stream);

		tstring userId = Format(_T("%s%d"), CSingleton<CSettings>::instance().m_user.c_str(), GetCurrentThreadId());

		stream.SetRelayServer( 
			CSingleton<CSettings>::instance().m_host, 
			CSingleton<CSettings>::instance().m_port, 
			userId, 
			CSingleton<CSettings>::instance().m_passwd);

		stream.SetConnectionId(info.connect_id, info.local_peer_id, info.remote_peer_id);
		stream.SetIsMaster(info.master);

		stream.GetCredentials().Key = _T("TestUser");
		stream.GetCredentials().UserID = _T("TestUser");
		stream.SetConnectTimeout(30000); 
		char Buf[MAX_PATH * 1024];
		TRY_CATCH
			stream.Connect();
			if (info.master)
			{
				for(int i=0; i<1; ++i)
					as->Send(Buf,MAX_PATH * 10);
			} 
			else
			{
				for(int i=0; i<1; ++i)
					as->Receive(Buf,MAX_PATH * 10);
			}
			printf(Format("Peer '%s' connected to peer '%s' through connection '%s'\n",
				info.local_peer_id.c_str(), info.remote_peer_id.c_str(), info.connect_id.c_str()).c_str());
			stream.Disconnect();
			return;
		CATCH_LOG()

			printf(Format("Peer '%s' has error in connection\n",info.local_peer_id.c_str()).c_str());

	CATCH_LOG()
	}
};
