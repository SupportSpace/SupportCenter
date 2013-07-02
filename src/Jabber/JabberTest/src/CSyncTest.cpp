/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSyncTest.cpp
///
///  Implements CSyncTest class, sync test
///
///  @author Dmitry Netrebenko @date 09.08.2007
///
////////////////////////////////////////////////////////////////////////

#include "CSyncTest.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CCritSection/CCritSection.h>

CSyncTest::CSyncTest()
{
TRY_CATCH
	m_event.reset( CreateEvent(NULL, FALSE, FALSE, NULL), CloseHandle );
	InitializeCriticalSection(&m_section);
CATCH_THROW()
}

CSyncTest::~CSyncTest()
{
TRY_CATCH
	DeleteCriticalSection(&m_section);
CATCH_LOG()
}
/*
void CSyncTest::Init(const STestParams& testParams, const SConnectParams& connectParams)
{
TRY_CATCH
	CAbstractTest::Init(testParams, connectParams);

	//TODO: initialization of sync test

CATCH_THROW()
}
*/

void CSyncTest::Stop()
{
TRY_CATCH
	SetEvent(m_event.get());
	CAbstractTest::Stop();
CATCH_THROW()
}

void CSyncTest::DoTest()
{
TRY_CATCH
	if(m_testParams.m_server)
		DoServerTest();
	else
		DoClientTest();
CATCH_THROW()
}

void CSyncTest::DoClientTest()
{
TRY_CATCH
	bool testSuccessful = true;
	RaiseOnProgress(0);
	for(unsigned int i = 0; i < m_testParams.m_msgCount; ++i)
	{
		if(m_terminated)
			throw MCException(_T("Test stopped"));
		{
			CCritSection section(&m_section);
			m_messages.clear();
		}
		for(unsigned int j = 0; j < m_testParams.m_bulkSize; ++j)
		{
			if(m_terminated)
				throw MCException(_T("Test stopped"));
			tstring msg = Format(_T("JabberTestMessage_%d_%d"), i, j);
			{
				CCritSection section(&m_section);
				m_messages.insert(msg);
			}
			m_messenger->Send(msg);
		}
		if(m_terminated)
			throw MCException(_T("Test stopped"));
		if(WAIT_TIMEOUT == WaitForSingleObject(m_event.get(), m_testParams.m_waitTimeout))
		{
			if(m_terminated)
				throw MCException(_T("Test stopped"));
			CCritSection section(&m_section);
			if(m_messages.size())
			{
				testSuccessful = false;
				Log.Add(_ERROR_, _T("The answer is not received for %d message(s)"), m_messages.size());
			}
		}
		if(i < m_testParams.m_msgCount - 1)
			Sleep(m_testParams.m_sendDelay);
		RaiseOnProgress((i + 1) / m_testParams.m_msgCount * 100);
	}
	RaiseOnComplete(testSuccessful);
CATCH_THROW()
}

void CSyncTest::DoServerTest()
{
TRY_CATCH
	RaiseOnProgress(0);
	unsigned int total = m_testParams.m_msgCount * m_testParams.m_bulkSize;
	unsigned int received = 0;
	for(unsigned int i = 0; i < total; ++i)
	{
		if(m_terminated)
			throw MCException(_T("Test stopped"));
		DWORD waitRes = WaitForSingleObject(m_event.get(), m_testParams.m_waitTimeout);
		if(m_terminated)
			throw MCException(_T("Test stopped"));
		if(WAIT_TIMEOUT == waitRes)
		{
			Log.Add(_WARNING_, _T("Message receiving timeout expiered"));
		}
		else
		{
			CCritSection section(&m_section);
			for(std::set<tstring>::iterator iter = m_messages.begin(); iter != m_messages.end(); ++iter )
			{
				m_messenger->Send(*iter);
				++received;
			}
			i += static_cast<unsigned int>(m_messages.size()) - 1;
			m_messages.clear();
		}
		if(m_terminated)
			throw MCException(_T("Test stopped"));
		RaiseOnProgress((i + 1) / total * 100);
	}
	RaiseOnComplete(received == total);
CATCH_THROW()
}

void CSyncTest::OnMessageReceived(const tstring& msg)
{
TRY_CATCH
	CCritSection section(&m_section);
	if(m_testParams.m_server)
	{
		m_messages.insert(msg);
		SetEvent(m_event.get());
	}
	else
	{
		m_messages.erase(msg);
		if(!m_messages.size())
			SetEvent(m_event.get());
	}
CATCH_THROW()
}

