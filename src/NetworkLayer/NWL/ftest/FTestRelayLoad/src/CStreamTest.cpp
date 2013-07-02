/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStreamTest.cpp
///
///  Implements CStreamTest class, test for sctream connection
///
///  @author Dmitry Netrebenko @date 27.02.2008
///
////////////////////////////////////////////////////////////////////////

#include "CStreamTest.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CCritSection/CCritSection.h>
#include "CPerformanceInterval.h"

CStreamTest::CStreamTest(boost::shared_ptr<CSettings> settings)
	:	CAbstractTest(settings)
{
TRY_CATCH
	m_statistic.Clear();
CATCH_THROW()
}

CStreamTest::~CStreamTest()
{
TRY_CATCH
CATCH_LOG()
}

void CStreamTest::RunTestThread(boost::shared_ptr<SThreadInfo> threadInfo)
{
TRY_CATCH
	InterlockedIncrement(&m_statistic.m_startedConnections);
	if(threadInfo->m_connectId == _T(""))
		throw MCException(_T("Invalid connection id"));

	boost::shared_ptr<CAbstractNetworkStream> stream = CreateStream(threadInfo);
	try
	{
		CPerformanceInterval timeInterval;
		timeInterval.Start();
		stream->Connect();
		timeInterval.Stop();
		InterlockedIncrement(&m_statistic.m_successConnections);
		SafeAdd64(&m_statistic.m_connectTime,timeInterval.GetSecExp3());
		OutputInfo(Format(_T("Peer '%s' connected to peer '%s' through connection '%s'"), threadInfo->m_localPeer.c_str(), threadInfo->m_remotePeer.c_str(), threadInfo->m_connectId.c_str()));
	}
	catch(...)
	{
		InterlockedIncrement(&m_statistic.m_failedConnections);
		OutputInfo(Format(_T("Peer '%s' has error at connection"), threadInfo->m_localPeer.c_str()));
		throw;
	}
	unsigned int blockSize = m_settings->GetBlockSize();
	unsigned int blocksCount = m_settings->GetBlocksCount();
	boost::scoped_array<char> buf(new char[blockSize]);
	TRY_CATCH
		try
		{
			for(unsigned int i = 0; i < blocksCount; ++i)
			{
				DWORD tm = GetTickCount();
				memcpy(buf.get(), &tm, sizeof(DWORD));
				stream->Send(buf.get(), blockSize);
				SafeAdd64(&m_statistic.m_sentBytes, blockSize);
			}
		}
		catch(...)
		{
			OutputInfo(Format(_T("Peer '%s' has error at sending data to peer '%s'"), threadInfo->m_localPeer.c_str(), threadInfo->m_remotePeer.c_str()));
			throw;
		}
		try
		{
			for(unsigned int i = 0; i < blocksCount; ++i)
			{
				stream->Receive(buf.get(), blockSize);
				SafeAdd64(&m_statistic.m_recvBytes, blockSize);
				DWORD tm;
				memcpy(&tm, buf.get(), sizeof(DWORD));
				SafeAdd64(&m_statistic.m_transferTime, GetTickCount() - tm);
			}
		}
		catch(...)
		{
			OutputInfo(Format(_T("Peer '%s' has error at recieving data from peer '%s'"), threadInfo->m_localPeer.c_str(), threadInfo->m_remotePeer.c_str()));
			throw;
		}
		stream->Disconnect();
		return;
	CATCH_LOG()
CATCH_THROW()
}

void CStreamTest::PrepareResults()
{
TRY_CATCH
	const double KBPERSEC = 1000.0/1024.0;
	m_results.push_back(Format(_T("Started connections: %d"), m_statistic.m_startedConnections));
	m_results.push_back(Format(_T("Successful connections: %d"), m_statistic.m_successConnections));
	m_results.push_back(Format(_T("Failed connections: %d"), m_statistic.m_failedConnections));
//	m_results.push_back(Format(_T("Total connect time: %I64d"), m_statistic.m_connectTime/* / m_settings->GetPoolSize()*/));
	if(m_statistic.m_successConnections)
	{
		double avgTime = m_statistic.m_connectTime * 1.0 / (m_statistic.m_successConnections /** m_settings->GetPoolSize()*/);
		m_results.push_back(Format(_T("Average connection time: %.2f ms"), avgTime));
	}
	m_results.push_back(Format(_T("Total bytes sent: %I64d"), m_statistic.m_sentBytes));
//	m_results.push_back(Format(_T("Total sent time: %I64d"), m_statistic.m_sentTime));
	m_results.push_back(Format(_T("Total bytes received: %I64d"), m_statistic.m_recvBytes));
//	m_results.push_back(Format(_T("Total receive time: %I64d"), m_statistic.m_recvTime));
	if(m_statistic.m_transferTime)
	{
		double avgTime = m_statistic.m_recvBytes * KBPERSEC / m_statistic.m_transferTime;
		m_results.push_back(Format(_T("Average transfer speed: %.2f kb/sec"), avgTime));
	}
CATCH_THROW()
}

void CStreamTest::SafeAdd64(__int64* data, __int64 value)
{
TRY_CATCH
	CCritSection section(&m_section);
	*data += value;
CATCH_THROW()
}

