/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAbstractTest.cpp
///
///  Implements CAbstractTest class, abstract test
///
///  @author Dmitry Netrebenko @date 09.08.2007
///
////////////////////////////////////////////////////////////////////////

#include "CAbstractTest.h"
#include <AidLib/CException/CException.h>
#include "CGlooxMessenger.h"
#include <boost/bind.hpp>

CAbstractTest::CAbstractTest()
	:	m_terminated(false)
{
TRY_CATCH
CATCH_THROW()
}

CAbstractTest::~CAbstractTest()
{
TRY_CATCH
	Stop();
CATCH_LOG()
}

void CAbstractTest::Init(const STestParams& testParams, const SConnectParams& connectParams)
{
TRY_CATCH
	m_testParams = testParams;
	m_connectParams = connectParams;
	switch(m_testParams.m_clientType)
	{
	case ctGloox:
		m_messenger.reset(new CGlooxMessenger());
		break;
	case ctWebXml:
		throw MCException(_T("Not implemented"));
		break;
	case ctJajc:
		throw MCException(_T("Not implemented"));
		break;
	default:
		throw MCException(_T("Invalid client type"));
	}
	m_messenger->SetOnMessageEvent(boost::bind(&CAbstractTest::OnMessageReceived, this, _1));
CATCH_THROW()
}

void CAbstractTest::Start()
{
TRY_CATCH
	if(!m_messenger.get())
		throw MCException(_T("Messenger not created"));
	m_terminated = false;
	m_thread.reset(new boost::thread(boost::bind(&CAbstractTest::ThreadEntryPoint, this)));
CATCH_THROW()
}

void CAbstractTest::Stop()
{
TRY_CATCH
	m_terminated = true;
	if(m_messenger.get())
		m_messenger->DeInit();
	//if(m_thread.get())
	//	m_thread->join();
CATCH_THROW()
}

void CAbstractTest::RaiseOnComplete(bool successful)
{
TRY_CATCH
	if(m_onComplete)
		m_onComplete(successful);
CATCH_THROW()
}

void CAbstractTest::RaiseOnProgress(unsigned int percents)
{
TRY_CATCH
	if(m_onProgress)
		m_onProgress(percents);
CATCH_THROW()
}

void CAbstractTest::SetOnCompleteEvent(OnCompleteEvent handler)
{
TRY_CATCH
	m_onComplete = handler;
CATCH_THROW()
}

void CAbstractTest::SetOnProgressEvent(OnProgressEvent handler)
{
TRY_CATCH
	m_onProgress = handler;
CATCH_THROW()
}

STestParams& CAbstractTest::GetTestParams()
{
TRY_CATCH
	return m_testParams;
CATCH_THROW()
}

SConnectParams& CAbstractTest::GetConnectParams()
{
TRY_CATCH
	return m_connectParams;
CATCH_THROW()
}

void CAbstractTest::ThreadEntryPoint()
{
TRY_CATCH
	m_messenger->Init(m_testParams, m_connectParams);
	DoTest();
	return;
CATCH_LOG()
TRY_CATCH
	RaiseOnComplete(false);
CATCH_LOG()
}

bool CAbstractTest::Terminated() const
{
TRY_CATCH
	return m_terminated;
CATCH_THROW()
}

