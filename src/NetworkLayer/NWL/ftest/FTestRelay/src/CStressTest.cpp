/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStressTest.cpp
///
///  Implements CStressTest class, responsible for stress testing
///
///  @author Dmitry Netrebenko, Max Sogin @date 04.10.2007
///
////////////////////////////////////////////////////////////////////////

#include "CStressTest.h"
#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <boost/bind.hpp>

CStressTest::CStressTest(int threads_count, int peers_count)
	:	m_ThreadsCount(threads_count)
	,	m_Pool(threads_count)
	,	m_ThreadId(0)
	,	m_PeersCount(peers_count)
{
TRY_CATCH

	// Initialize critical section
	InitializeCriticalSection(
		&m_InfoSection);

	tstring strLocalPeer;
	tstring strRemotePeer;
	tstring strConnect;
	for( int index = 0; index < m_PeersCount; index++ )
	{
		strLocalPeer = Format(_T("peer_%d"), index + 1);
		bool master;
		if( !(index % 2) )
		{
			strRemotePeer = Format(_T("peer_%d"), index + 2);
			master = true;
		}
		else
		{
			strRemotePeer = Format(_T("peer_%d"), index);
			master = false;
		}
		strConnect = Format(_T("conn_%d"), (int)(index/2) + 1);
		m_Info.insert(ThreadInfoEntry(index + 1, ThreadInfo(master, index+1, strConnect, strLocalPeer, strRemotePeer)));
	}

CATCH_THROW()
}

CStressTest::~CStressTest()
{
TRY_CATCH

	// Close critical section
	DeleteCriticalSection(
		&m_InfoSection);

CATCH_LOG()
}

ThreadInfo CStressTest::GetThreadInfo()
{
TRY_CATCH
	
	CCritSection section( &m_InfoSection );

	m_ThreadId++;

	ThreadsInfo::iterator index = m_Info.find( m_ThreadId );

	if( index == m_Info.end() )
		return ThreadInfo();

	return index->second;

CATCH_THROW()
}
