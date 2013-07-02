/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStunConnectSettings.cpp
///
///  Implements CStunConnectSettings class, responsible for management of
///    properties of connection to STUN server
///
///  @author Dmitry Netrebenko @date 27.11.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CStunConnectSettings.h>
#include <AidLib/CException/CException.h>
#include <AidLib/Logging/cLog.h>

CStunConnectSettings::CStunConnectSettings()
: m_nBindRetryDelay( 5000 ), m_nBindMaxRetryCount( 3 ),
  m_nProbeRetryDelay( 5000 ), m_nProbeMaxRetryCount( 3 ),
  m_nAuthRetryDelay( 5000 ), m_nAuthMaxRetryCount( 3 ),
  m_nProbePortRange( 3 )
{
TRY_CATCH

CATCH_THROW("CStunConnectSettings::CStunConnectSettings")
}

CStunConnectSettings::~CStunConnectSettings()
{
TRY_CATCH

CATCH_LOG("CStunConnectSettings::~CStunConnectSettings")
}

unsigned int CStunConnectSettings::GetBindRetryDelay() const
{
TRY_CATCH

	return m_nBindRetryDelay;

CATCH_THROW("CStunConnectSettings::GetBindRetryDelay")
}

unsigned int CStunConnectSettings::GetBindMaxRetryCount() const
{
TRY_CATCH

	return m_nBindMaxRetryCount;

CATCH_THROW("CStunConnectSettings::GetBindMaxRetryCount")
}

unsigned int CStunConnectSettings::GetProbeRetryDelay() const
{
TRY_CATCH

	return m_nProbeRetryDelay;

CATCH_THROW("CStunConnectSettings::GetProbeRetryDelay")
}

unsigned int CStunConnectSettings::GetProbeMaxRetryCount() const
{
TRY_CATCH

	return m_nProbeMaxRetryCount;

CATCH_THROW("CStunConnectSettings::GetProbeMaxRetryCount")
}

unsigned int CStunConnectSettings::GetProbePortRange() const
{
TRY_CATCH

	return m_nProbePortRange;

CATCH_THROW("CStunConnectSettings::GetProbePortRange")
}

void CStunConnectSettings::SetBindRetry( 
	const unsigned int& RetryDelay, const unsigned int& MaxRetryCount )
{
TRY_CATCH

	m_nBindRetryDelay = RetryDelay;
	m_nBindMaxRetryCount = MaxRetryCount;

CATCH_THROW("CStunConnectSettings::SetBindRetry")
}

void CStunConnectSettings::SetProbeRetry( 
	const unsigned int& RetryDelay, const unsigned int& MaxRetryCount )
{
TRY_CATCH

	m_nProbeRetryDelay = RetryDelay;
	m_nProbeMaxRetryCount = MaxRetryCount;


CATCH_THROW("CStunConnectSettings::SetProbeRetry")
}

void CStunConnectSettings::SetProbePortRange( const unsigned int& PortRange )
{
TRY_CATCH

	m_nProbePortRange = PortRange;
	if( m_nProbePortRange < 1 )
		m_nProbePortRange = 1;

CATCH_THROW("CStunConnectSettings::SetProbePortRange")
}

unsigned int CStunConnectSettings::GetAuthRetryDelay() const
{
TRY_CATCH

	return m_nAuthRetryDelay;

CATCH_THROW("CStunConnectSettings::GetAuthRetryDelay")
}

unsigned int CStunConnectSettings::GetAuthMaxRetryCount() const
{
TRY_CATCH

	return m_nAuthMaxRetryCount;

CATCH_THROW("CStunConnectSettings::GetAuthMaxRetryCount")
}

void CStunConnectSettings::SetAuthRetry( 
	const unsigned int& RetryDelay, const unsigned int& MaxRetryCount )
{
TRY_CATCH

	m_nAuthRetryDelay = RetryDelay;
	m_nAuthMaxRetryCount = MaxRetryCount;

CATCH_THROW("CStunConnectSettings::SetBindRetry")
}

