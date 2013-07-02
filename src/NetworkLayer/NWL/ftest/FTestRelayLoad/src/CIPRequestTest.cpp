/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CIPRequestTest.cpp
///
///  Implements CIPRequestTest class, test for external ip service
///
///  @author Dmitry Netrebenko @date 04.03.2008
///
////////////////////////////////////////////////////////////////////////

#include <winsock2.h>
#include "CIPRequestTest.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CCritSection/CCritSection.h>
#include "CPerformanceInterval.h"
#include <NWL/Streaming/CExternalIPRequest.h>

CIPRequestTest::CIPRequestTest(boost::shared_ptr<CSettings> settings)
	:	CAbstractTest(settings)
{
TRY_CATCH
	m_statistic.Clear();
CATCH_THROW()
}

CIPRequestTest::~CIPRequestTest()
{
TRY_CATCH
CATCH_LOG()
}

void CIPRequestTest::RunTestThread(boost::shared_ptr<SThreadInfo> threadInfo)
{
TRY_CATCH
	CExternalIPRequest ipRequest(m_settings->GetRelayHost(), m_settings->GetRelayPort());
	tstring userId = Format(_T("%s%d"), m_settings->GetUser().c_str(), threadInfo->m_threadId);
	InterlockedIncrement(&m_statistic.m_startedRequests);
	CPerformanceInterval requestTime;
	try
	{
		requestTime.Start();
		SPeerAddr addr = ipRequest.GetExternalAddress(userId, m_settings->GetPassword());
		requestTime.Stop();
		OutputInfo(Format(_T("Peer '%s' has received the external IP address: %s:%d"), threadInfo->m_localPeer.c_str(), addr.address, addr.port));
		InterlockedIncrement(&m_statistic.m_successRequests);
		SafeAdd64(&m_statistic.m_requestTime, requestTime.GetSecExp3());
	}
	catch(...)
	{
		InterlockedIncrement(&m_statistic.m_failedRequests);
		OutputInfo(Format(_T("Peer '%s' has error during external IP request"), threadInfo->m_localPeer.c_str()));
		throw;
	}
CATCH_THROW()
}

void CIPRequestTest::PrepareResults()
{
TRY_CATCH
	m_results.push_back(Format(_T("Started requests: %d"), m_statistic.m_startedRequests));
	m_results.push_back(Format(_T("Successful requests: %d"), m_statistic.m_successRequests));
	m_results.push_back(Format(_T("Failed requests: %d"), m_statistic.m_failedRequests));
//	m_results.push_back(Format(_T("Total requests time: %I64d"), m_statistic.m_requestTime));
	if(m_statistic.m_successRequests)
	{
		double avgTime = m_statistic.m_requestTime * 1.0 / m_statistic.m_successRequests;
		m_results.push_back(Format(_T("Average request time: %.2f ms"), avgTime));
	}
CATCH_THROW()
}

void CIPRequestTest::SafeAdd64(__int64* data, __int64 value)
{
TRY_CATCH
	CCritSection section(&m_section);
	*data += value;
CATCH_THROW()
}
