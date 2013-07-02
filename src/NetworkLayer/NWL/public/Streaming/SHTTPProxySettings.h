/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SHTTPProxySettings.h
///
///  Declares SHTTPProxySettings structure, HTTP proxy server's settings
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>
#include <NWL/NetworkLayer.h>

///  HTTP proxy server's settings
struct NWL_API SHTTPProxySettings
{
	tstring			ProxyURL;
	unsigned int	ProxyPort;

	SHTTPProxySettings() 
		: ProxyURL(_T("")), ProxyPort(0) 
	{};
	
	SHTTPProxySettings( const tstring& url, const unsigned int& port )
		: ProxyURL( url ), ProxyPort( port )
	{};

};
