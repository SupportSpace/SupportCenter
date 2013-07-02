/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CMessenger.cpp
///
///  Implements CMessenger class, base messenger
///
///  @author Dmitry Netrebenko @date 09.08.2007
///
////////////////////////////////////////////////////////////////////////

#include "CMessenger.h"
#include <AidLib/CException/CException.h>

CMessenger::CMessenger()
{
TRY_CATCH
CATCH_THROW()
}

CMessenger::~CMessenger()
{
TRY_CATCH
CATCH_LOG()
}

void CMessenger::Init(const STestParams& testParams, const SConnectParams& connectParams)
{
TRY_CATCH
	m_testParams = testParams;
	m_connectParams = connectParams;
CATCH_THROW()
}

void CMessenger::RaiseOnMessage(const tstring& msg)
{
TRY_CATCH
	if(m_onMessage)
		m_onMessage(msg);
CATCH_THROW()
}

void CMessenger::SetOnMessageEvent(OnMessageEvent handler)
{
TRY_CATCH
	m_onMessage = handler;
CATCH_THROW()
}

STestParams& CMessenger::GetTestParams()
{
TRY_CATCH
	return m_testParams;
CATCH_THROW()
}

SConnectParams& CMessenger::GetConnectParams()
{
TRY_CATCH
	return m_connectParams;
CATCH_THROW()
}
