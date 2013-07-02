/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSocketProxyClientConnectThread.h
///
///  Declares CSocketProxyClientConnectThread class, responsible for socket 
///    client connection through HTTP proxy
///
///  @author Dmitry Netrebenko @date 16.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CSocketClientConnectThread.h"
#include "SHTTPProxySettings.h"

///  CSocketProxyClientConnectThread class, responsible for socket 
///    client connection through HTTP proxy
///  Base class CSocketClientConnectThread, responsible for client connection 
class CSocketProxyClientConnectThread :
	public CSocketClientConnectThread
{
private:
/// Prevents making copies of CSocketProxyClientConnectThread objects.
	CSocketProxyClientConnectThread( const CSocketProxyClientConnectThread& );				
	CSocketProxyClientConnectThread& operator=( const CSocketProxyClientConnectThread& );	

public:
///  Constructor
	CSocketProxyClientConnectThread();

///  Thread's entry point
///  @param   Pointer to thread's parameters
	void Execute(void*);

private:
/// Proxy settings
	SHTTPProxySettings	m_ProxySettings;

public:
///  Returns settings of http proxy
///  @return SHTTPProxySettings structure
///  @remarks
	SHTTPProxySettings& GetProxySettings();
	
///  Sets settings of http proxy
///  @param   new proxy settings
///  @remarks
	void SetProxySettings( const SHTTPProxySettings& );

private:
///  Connect to remote host through HTTP proxy
///  @param   sync call
///  @remarks
	void ConnectThroughProxy(bool sync_call);

};
