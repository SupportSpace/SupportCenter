/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  COpenPortTest.cpp
///
///  Implements COpenPortTest class, test for open port service
///
///  @author Dmitry Netrebenko @date 06.03.2008
///
////////////////////////////////////////////////////////////////////////

#include <winsock2.h>
#include "COpenPortTest.h"
#include <AidLib/CException/CException.h>
#include <NWL/Streaming/CSSocket.h>
#include <NWL/Streaming/relay_messages.h>
#include <NWL/Streaming/CSocketStream.h>
#include <AidLib/CCrypto/CCrypto.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "CPerformanceInterval.h"

COpenPortTest::COpenPortTest(boost::shared_ptr<CSettings> settings)
	:	CAbstractTest(settings)
	,	m_stubTerminated(false)
{
TRY_CATCH
	m_statistic.Clear();
CATCH_THROW()
}

COpenPortTest::~COpenPortTest()
{
TRY_CATCH
CATCH_LOG()
}

void COpenPortTest::RunTestThread(boost::shared_ptr<SThreadInfo> threadInfo)
{
TRY_CATCH
	boost::scoped_ptr<CSocketStream> stream(new CSocketStream());
	SRelayMessage msg;
	tstring userId = Format(_T("%s%d"), m_settings->GetUser().c_str(), threadInfo->m_threadId);
	tstring password = m_settings->GetPassword();

	InterlockedIncrement(&m_statistic.m_startedRequests);
	CPerformanceInterval requestTime;
	requestTime.Start();

	try
	{
		CPerformanceInterval connectTime;
		connectTime.Start();
		stream->Connect(m_settings->GetRelayHost(), m_settings->GetRelayPort(), true);
		connectTime.Stop();
		SafeAdd64(&m_statistic.m_connectTime, connectTime.GetSecExp3());
	}
	catch(...)
	{
		InterlockedIncrement(&m_statistic.m_failedRequests);
		InterlockedIncrement(&m_statistic.m_failedConnect);
		OutputInfo(Format(_T("Peer '%s' has error at checking external port"), threadInfo->m_localPeer.c_str()));
		throw;
	}

	try
	{
		CPerformanceInterval authTime;
		authTime.Start();

		msg.type = rmtAuthRequest;
		msg.size = static_cast<int>(RELAY_MSG_HEAD_SIZE + (userId.length() + 1)*sizeof(TCHAR));
		_tcscpy_s(msg.data,MAX_MSG_SIZE,userId.c_str()); 
		stream->Send(reinterpret_cast<char*>(&msg),msg.size);
		stream->Receive(reinterpret_cast<char*>(&msg), RELAY_MSG_HEAD_SIZE);
		if (!msg.size || msg.size != RELAY_MSG_HEAD_SIZE + RELAY_CHALLENGE_SIZE)
			throw MCStreamException(_T("Invalid message size"));
		if (msg.type != rmtChallengeRequest)
			throw MCStreamException(_T("Invalid message type"));
		stream->Receive(msg.data,msg.size - RELAY_MSG_HEAD_SIZE);
		// Calculating hash
		std::auto_ptr<TCHAR> buffer;
		buffer.reset(new TCHAR[RELAY_CHALLENGE_SIZE + password.length()]);
		memcpy(buffer.get(),password.c_str(),password.length()*sizeof(TCHAR));
		memcpy(buffer.get()+password.length(),msg.data,RELAY_CHALLENGE_SIZE);
		char hash[RELAY_HASH_SIZE];
		CSingleton<CCrypto>::instance().MakeHash(	buffer.get(),
													(int)(RELAY_CHALLENGE_SIZE + password.length())*sizeof(TCHAR),
													hash,
													RELAY_HASH_SIZE);

		msg.type = rmtChallengeResponceNoSession;
		msg.size = RELAY_MSG_HEAD_SIZE + RELAY_HASH_SIZE;
		memcpy(msg.data,hash,RELAY_HASH_SIZE);
		stream->Send(reinterpret_cast<char*>(&msg),msg.size);

		// Validate authentication
		msg.size = RELAY_MSG_HEAD_SIZE;
		stream->Receive(reinterpret_cast<char*>(&msg),msg.size);
		if (msg.type != rmtAuthSuccessfull)
			throw MCStreamException(_T("Relay authentication failed"));

		authTime.Stop();
		SafeAdd64(&m_statistic.m_authTime, authTime.GetSecExp3());
	}
	catch(...)
	{
		InterlockedIncrement(&m_statistic.m_failedAuth);
		InterlockedIncrement(&m_statistic.m_failedRequests);
		OutputInfo(Format(_T("Peer '%s' has error at checking external port"), threadInfo->m_localPeer.c_str()));
		throw;
	}

	try
	{
		CPerformanceInterval checkPortTime;
		checkPortTime.Start();

		msg.type = rmtCheckPort;
		msg.size = RELAY_MSG_HEAD_SIZE + RELAY_PORTCHECK_SIZE;
		*(reinterpret_cast<int*>(msg.data)) = m_settings->GetExtPort();
		stream->Send(reinterpret_cast<char*>(&msg),msg.size);

		stream->Receive(reinterpret_cast<char*>(&msg), RELAY_MSG_HEAD_SIZE);
		if (!msg.size || msg.size != RELAY_MSG_HEAD_SIZE + RELAY_PORTCHECK_SIZE)
			throw MCStreamException("Invalid message size");
		if (msg.type != rmtPortOpened)
			throw MCStreamException("Invalid message type");
		stream->Receive(msg.data,msg.size - RELAY_MSG_HEAD_SIZE);
		//Check received and sent data on an equality
		if ( static_cast<int>(m_settings->GetExtPort()) != *(reinterpret_cast<int*>(msg.data)) )
			throw MCStreamException("Wrong answer from server side");

		checkPortTime.Stop();
		SafeAdd64(&m_statistic.m_portCheckTime, checkPortTime.GetSecExp3());
	}
	catch(...)
	{
		InterlockedIncrement(&m_statistic.m_failedOpenPort);
		InterlockedIncrement(&m_statistic.m_failedRequests);
		OutputInfo(Format(_T("Peer '%s' has error at checking external port"), threadInfo->m_localPeer.c_str()));
		throw;
	}

	requestTime.Stop();
	InterlockedIncrement(&m_statistic.m_successRequests);
	SafeAdd64(&m_statistic.m_requestTime, requestTime.GetSecExp3());
	OutputInfo(Format(_T("Peer '%s' has checked external port successfully"), threadInfo->m_localPeer.c_str()));

CATCH_THROW()
}

void COpenPortTest::PrepareResults()
{
TRY_CATCH
	m_results.push_back(Format(_T("Started requests: %d"), m_statistic.m_startedRequests));
	m_results.push_back(Format(_T("Successful requests: %d"), m_statistic.m_successRequests));
	m_results.push_back(Format(_T("Failed requests: %d"), m_statistic.m_failedRequests));

	m_results.push_back(Format(_T("Connect failures: %d"), m_statistic.m_failedConnect));
	m_results.push_back(Format(_T("Authentication failures: %d"), m_statistic.m_failedAuth));
	m_results.push_back(Format(_T("Open port checking failures: %d"), m_statistic.m_failedOpenPort));


	m_results.push_back(Format(_T("Checking requests from server: %d"), m_statistic.m_serverRequests));
//	m_results.push_back(Format(_T("Total requests time: %I64d"), m_statistic.m_requestTime));
	if(m_statistic.m_successRequests)
	{
		double avgTime = m_statistic.m_requestTime * 1.0 / m_statistic.m_successRequests;
		m_results.push_back(Format(_T("Average request time: %.2f ms"), avgTime));

		avgTime = m_statistic.m_connectTime * 1.0 / m_statistic.m_successRequests;
		m_results.push_back(Format(_T("Average connect time: %.2f ms"), avgTime));

		avgTime = m_statistic.m_authTime * 1.0 / m_statistic.m_successRequests;
		m_results.push_back(Format(_T("Average auth time: %.2f ms"), avgTime));

		avgTime = m_statistic.m_portCheckTime * 1.0 / m_statistic.m_successRequests;
		m_results.push_back(Format(_T("Average port checking time: %.2f ms"), avgTime));
	}
CATCH_THROW()
}

void COpenPortTest::OnTestStarted()
{
TRY_CATCH
	m_stubTerminated = false;
	m_stubThread.reset(new boost::thread(boost::bind(&COpenPortTest::StubEntryPoint, this)));
CATCH_THROW()
}

void COpenPortTest::OnTestStopped()
{
TRY_CATCH
	m_stubTerminated = true;
	if(m_stubThread.get())
		m_stubThread->join();
CATCH_THROW()
}

void COpenPortTest::StubEntryPoint()
{
TRY_CATCH
	boost::shared_ptr<CSSocket> socket(new CSSocket());
	if(!socket->Create())
		throw MCException_Win("Socket create failed");
	if(!socket->Bind(m_settings->GetIntPort()))
		throw MCException_Win("Bind failed");
	if(!socket->Listen())
		throw MCException_Win("Listen failed");
	while(!m_stubTerminated)
	{
		if(socket->ReadSelect(100))
		{
			boost::scoped_ptr<CSSocket> sock(socket->Accept());
			if(sock.get())
			{
				SRelayMessage msg;
				try
				{
					sock->Receive(reinterpret_cast<char*>(&msg), RELAY_MSG_HEAD_SIZE);
					if (	msg.size &&
						msg.size == RELAY_MSG_HEAD_SIZE + RELAY_PORTCHECK_SIZE &&
						msg.type == rmtCheckPort )
					{
						InterlockedIncrement(&m_statistic.m_serverRequests);
						sock->Receive(msg.data,msg.size - RELAY_MSG_HEAD_SIZE);
						msg.type=rmtPortOpened;
						sock->Send(reinterpret_cast<char*>(&msg),msg.size);
					}
				}
				catch(...)
				{
				}
			}
		}
	}
CATCH_LOG()
}

void COpenPortTest::SafeAdd64(__int64* data, __int64 value)
{
TRY_CATCH
	CCritSection section(&m_section);
	*data += value;
CATCH_THROW()
}

