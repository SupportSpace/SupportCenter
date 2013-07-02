/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTLSAuthSettings.cpp
///
///  Implements CTLSAuthSettings class, responsible for GNUTLS authentication
///    settings
///
///  @author Dmitry Netrebenko @date 17.09.2007
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CTLSAuthSettings.h>
#include <AidLib/CException/CException.h>
#include <AidLib/Logging/cLog.h>

CTLSAuthSettings::CTLSAuthSettings()
	:	m_Suite()
	,	m_Credentials()
{
TRY_CATCH
CATCH_THROW()
}

CTLSAuthSettings::~CTLSAuthSettings()
{
TRY_CATCH
CATCH_LOG()
}

STLSSuite& CTLSAuthSettings::GetSuite()
{
TRY_CATCH
	return m_Suite;
CATCH_THROW()
}

void CTLSAuthSettings::SetSuite(const STLSSuite& suite)
{
TRY_CATCH
	m_Suite = suite;
CATCH_THROW()
}

STLSCredentials& CTLSAuthSettings::GetCredentials()
{
TRY_CATCH
	return m_Credentials;
CATCH_THROW()
}

void CTLSAuthSettings::SetCredentials(const STLSCredentials& credentials)
{
TRY_CATCH
	m_Credentials = credentials;
CATCH_THROW()
}
