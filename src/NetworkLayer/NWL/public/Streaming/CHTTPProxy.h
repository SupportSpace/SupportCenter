/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CHTTPProxy.h
///
///  Declares CHTTPProxy class for management of HTTP proxy's settings
///
///  @author Dmitry Netrebenko @date 24.11.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "SHTTPProxySettings.h"
#include <NWL/NetworkLayer.h>

///  CHTTPProxy class for management of HTTP proxy's settings
///  @remarks
class NWL_API CHTTPProxy
{
public:
///  Constructor
	CHTTPProxy();

///  Destructor
	~CHTTPProxy();

protected:
/// Proxy settings (host, port)	
	SHTTPProxySettings		m_ProxySettings;

/// Stream should connect through http proxy
	bool					m_bConnectThroughProxy;

public:

///  Returns settings of http proxy
///  @return SHTTPProxySettings structure
///  @remarks
	SHTTPProxySettings& GetProxySettings();
	
///  Sets settings of http proxy
///  @param   new proxy settings
///  @remarks
	void SetProxySettings( const SHTTPProxySettings& );

///  Whether owe a stream will connect through a proxy
///  @return true, if stream needs connect through proxy
///  @remarks
	bool GetConnectThroughProxy() const;
	
///  Sets value for "Connect Through Proxy"
///  @param   new value
///  @remarks
	void SetConnectThroughProxy( const bool );

};
