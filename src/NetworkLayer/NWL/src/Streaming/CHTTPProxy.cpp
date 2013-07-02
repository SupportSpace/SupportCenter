/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CHTTPProxy.cpp
///
///  Implements CHTTPProxy class for management of HTTP proxy's settings
///
///  @author Dmitry Netrebenko @date 24.11.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CHTTPProxy.h>
#include <AidLib/CException/CException.h>
#include <AidLib/Logging/cLog.h>

CHTTPProxy::CHTTPProxy()
: m_ProxySettings(), m_bConnectThroughProxy( false )
{
TRY_CATCH

CATCH_THROW("CHTTPProxy::CHTTPProxy")
}

CHTTPProxy::~CHTTPProxy()
{
TRY_CATCH

CATCH_LOG("CHTTPProxy::~CHTTPProxy")
}

SHTTPProxySettings& CHTTPProxy::GetProxySettings()
{
TRY_CATCH

	return m_ProxySettings;

CATCH_THROW("CHTTPProxy::GetProxySettings")
}
	
void CHTTPProxy::SetProxySettings( const SHTTPProxySettings& proxy )
{
TRY_CATCH

	m_ProxySettings = proxy;

CATCH_THROW("CHTTPProxy::SetProxySettings")
}

bool CHTTPProxy::GetConnectThroughProxy() const
{
TRY_CATCH

	return m_bConnectThroughProxy;

CATCH_THROW("CHTTPProxy::GetConnectThroughProxy")
}
	
void CHTTPProxy::SetConnectThroughProxy( const bool through_proxy )
{
TRY_CATCH

	m_bConnectThroughProxy = through_proxy;

CATCH_THROW("CHTTPProxy::SetConnectThroughProxy")
}

