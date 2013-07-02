/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNetworkLayer.h
///
///  NWL class (NWL setting etc)
///
///  @author "Archer Software" Sogin M. @date 20.11.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CNetworkLayer.h>
#include <AidLib/Logging/cLog.h>
#include <NWL/Streaming/CStreamException.h>
#include "WinInet.h"
#include <NWL/Locale/locale.h>
#include <boost/shared_ptr.hpp>
#include <boost\type_traits\remove_pointer.hpp>

void CNetworkLayer::SaveValue(const tstring& valueName, const int value)
{
TRY_CATCH
	tstring subKey = Format(_T("%s"), NWL_REGISTRY_PATH);
	HKEY hKey;
	DWORD disposition;
	DWORD res;
	if (ERROR_SUCCESS != (res = RegCreateKeyEx(	HKEY_CURRENT_USER, 
												subKey.c_str(),
												0, /*reserved*/
												NULL /*class*/, 
												REG_OPTION_NON_VOLATILE,
												KEY_SET_VALUE,
												NULL,
												&hKey,
												&disposition)))
	{
		SetLastError(res);
		throw MCException_Win("Failed to RegCreateKeyEx");
	}
	boost::shared_ptr<boost::remove_pointer<HKEY>::type> key(hKey,::RegCloseKey);
	if ( ERROR_SUCCESS != (res = RegSetValueEx(	hKey,
												valueName.c_str(),
												0,
												REG_DWORD,
												(BYTE*)&value,
												sizeof(value))))
	{
		SetLastError(res);
		throw MCException_Win("Failed to RegSetValueEx");
	}
	
CATCH_THROW()
}

void CNetworkLayer::RestoreValue(const tstring &valueName, int &value)
{
TRY_CATCH
	tstring subKey = Format(_T("%s"), NWL_REGISTRY_PATH );
	DWORD res,type(RRF_RT_REG_DWORD),size(sizeof(value));
	HKEY hKey;
	DWORD disposition;
	if (ERROR_SUCCESS != (res = RegCreateKeyEx(	HKEY_CURRENT_USER, 
												subKey.c_str(),
												0, /*reserved*/
												NULL /*class*/, 
												REG_OPTION_NON_VOLATILE,
												KEY_QUERY_VALUE,
												NULL,
												&hKey,
												&disposition)))
	{
		SetLastError(res);
		throw MCException_Win("Failed to RegCreateKeyEx");
	}
	boost::shared_ptr<boost::remove_pointer<HKEY>::type> key(hKey,::RegCloseKey);
	if ( ERROR_SUCCESS != (res = RegQueryValueEx(	key.get(),
													valueName.c_str(),
													NULL, /*reserved*/
													&type,
													(LPBYTE)&value,
													&size )))
	{
		SetLastError(res);
		throw MCException_Win("Failed to RegQueryValueEx");
	}
CATCH_THROW()
}

void CNetworkLayer::ResetValue(const tstring &valueName)
{
TRY_CATCH
	tstring subKey = Format(_T("%s"), NWL_REGISTRY_PATH);
	HKEY hKey;
	DWORD disposition;
	DWORD res;
	if (ERROR_SUCCESS != (res = RegCreateKeyEx(	HKEY_CURRENT_USER, 
												subKey.c_str(),
												0, /*reserved*/
												NULL /*class*/, 
												REG_OPTION_NON_VOLATILE,
												KEY_SET_VALUE,
												NULL,
												&hKey,
												&disposition)))
	{
		SetLastError(res);
		throw MCException_Win("Failed to RegCreateKeyEx");
	}
	boost::shared_ptr<boost::remove_pointer<HKEY>::type> key(hKey,::RegCloseKey);
	if ( ERROR_SUCCESS != (res = RegDeleteValue(	hKey,
													valueName.c_str() )))
	{
		SetLastError(res);
		throw MCException_Win("Failed to RegDeleteValue");
	}

CATCH_THROW()
}

CNetworkLayer::CNetworkLayer() 
	:	CStunConnectSettings(),
		m_relayHost(_T(DEFAULT_RELAY_HOST)),
		m_relayUDPPort(DEFAULT_RELAY_UDP_PORT),
		m_relayTCPPort(DEFAULT_RELAY_TCP_PORT),
		m_directStreamPort(DEFAULT_DIRECT_STREAM_PORT),
		m_IMStubPort(IM_PORT),
		m_IMRetryInterval(DEFAULT_IM_RETRY_INTERVAL),
		m_useProxy(false),
		m_relayPasswd(DEFAULT_RELAY_PASSWD),
		m_timeout_port_availability(DEFAULT_TIMEOUT_FOR_PORT_LISTENING),
		m_directStreamPortMode(EPSM_DEFAULT),
		m_useSendTimeout(false)
{
TRY_CATCH

	InitLocale();

	m_nBindRetryDelay		= DEFAULT_STUN_BIND_DELAY;
	m_nBindMaxRetryCount	= DEFAULT_STUN_BIND_MAX_COUNT;
	m_nProbeRetryDelay		= DEFAULT_STUN_PROBE_DELAY;
	m_nProbeMaxRetryCount	= DEFAULT_STUN_PROBE_MAX_COUNT;
	m_nProbePortRange		= DEFAULT_STUN_PROBE_PORT_RANGE;
	m_nAuthRetryDelay		= DEFAULT_STUN_AUTH_DELAY;
	m_nAuthMaxRetryCount	= DEFAULT_STUN_AUTH_MAX_COUNT;

	/// Setting up proxy server
	std::auto_ptr<INTERNET_PROXY_INFO> proxyInfo;
	proxyInfo.reset(reinterpret_cast<INTERNET_PROXY_INFO*>(new char[MAX_PATH]));
	DWORD size = MAX_PATH;
	if (!InternetQueryOption(NULL,INTERNET_OPTION_PROXY,proxyInfo.get(), &size))
	{
		Log.WinError(_ERROR_,"CNetworkLayer::CNetworkLayer: Failed to retrive proxy settings: ");
	} else
	{
		if (proxyInfo->dwAccessType == INTERNET_OPEN_TYPE_PROXY)
		{
			tstring proxy = proxyInfo->lpszProxy;
			size_t pos;
			tstring prefix(_T("http="));
			if ((pos = proxy.find(prefix)) != tstring::npos)
			{
				proxy.erase(0,pos+prefix.length());
			}
			if ((pos = proxy.find(_T(" "))) != tstring::npos)
			{
				proxy.erase(pos,proxy.length()-pos);
			}
			if ((pos = proxy.find(_T(":"))) != tstring::npos)
			{
				int port = atoi(proxy.substr(pos+1,proxy.length()-pos-1).c_str());
				proxy.erase(pos,proxy.length()-pos);
				if (port && proxy.length())
				{
					SetUseProxy(true);
					SetProxySettings(SHTTPProxySettings(proxy,port));
				}
			}
		}
	}

	// Restoring persistent values
	try
	{
		RestoreValue(DIRECT_STREAM_KEY_NAME, m_directStreamPort);
	}
	catch(CExceptionBase&)
	{
		// It's ok. Just wasn't saved before
	}
	try
	{
		int portMode;
		RestoreValue(DIRECT_STREAM_PORT_MODE_KEY_NAME, portMode);
		m_directStreamPortMode = static_cast<EPortSelectionMode>(portMode);
	}
	catch(CExceptionBase&)
	{
		// It's ok. Just wasn't saved before
	}

CATCH_LOG("CNetworkLayer::CNetworkLayer")
}

CNetworkLayer::~CNetworkLayer()
{
TRY_CATCH
CATCH_LOG("CNetworkLayer::~CNetworkLayer")
}

void CNetworkLayer::SetIMStubPort(const int IMStubPort)
{
TRY_CATCH
	m_IMStubPort = IMStubPort;
CATCH_THROW("CNetworkLayer::SetIMStubPort")
}

int CNetworkLayer::GetIMStubPort() const
{
TRY_CATCH
	return m_IMStubPort;
CATCH_THROW("CNetworkLayer::GetIMStubPort")
}

void CNetworkLayer::SetRelayHost(const tstring& relayHost)
{
TRY_CATCH
	m_relayHost = relayHost;
CATCH_THROW("CNetworkLayer::SetRelayHost")
}

tstring CNetworkLayer::GetRelayHost() const
{
TRY_CATCH
	return m_relayHost;
CATCH_THROW("CNetworkLayer::GetRelayHost")
}

void CNetworkLayer::SetRelayUDPPort(const int relayUDPPort)
{
TRY_CATCH
	m_relayUDPPort = relayUDPPort;
CATCH_THROW("CNetworkLayer::SetRelayUDPPort")
}

int CNetworkLayer::GetRelayUDPPort() const
{
TRY_CATCH
	return m_relayUDPPort;
CATCH_THROW("CNetworkLayer::GetRelayUDPPort")
}


void CNetworkLayer::SetRelayTCPPort(const int relayTCPPort)
{
TRY_CATCH
	m_relayTCPPort = relayTCPPort;
CATCH_THROW("CNetworkLayer::SetRelayTCPPort")
}

int CNetworkLayer::GetRelayTCPPort() const
{
TRY_CATCH
	return m_relayTCPPort;
CATCH_THROW("CNetworkLayer::GetRelayTCPPort")
}

void CNetworkLayer::SetDirectStreamPortMode(const EPortSelectionMode portMode, const bool permanently)
{
TRY_CATCH
	m_directStreamPortMode = portMode;
	if (permanently)
	{
		SaveValue(DIRECT_STREAM_PORT_MODE_KEY_NAME, static_cast<int>(portMode));
	}
CATCH_THROW("CNetworkLayer::SetDirectStreamPortMode")
}

EPortSelectionMode CNetworkLayer::GetDirectStreamPortMode() const
{
TRY_CATCH
	return m_directStreamPortMode;
CATCH_THROW("CNetworkLayer::GetDirectStreamPortMode")
}

void CNetworkLayer::SetCustomDirectStreamPort(const int directStreamPort, const bool permanently)
{
TRY_CATCH
	m_directStreamPort = directStreamPort;
	if (permanently)
	{
		SaveValue(DIRECT_STREAM_KEY_NAME,directStreamPort);
	}
CATCH_THROW("CNetworkLayer::SetCustomDirectStreamPort")
}

void CNetworkLayer::Reset2DefaultDirectStreamPort()
{
TRY_CATCH
	ResetValue(DIRECT_STREAM_KEY_NAME);
	m_directStreamPort = DEFAULT_DIRECT_STREAM_PORT;
	SetDirectStreamPortMode(EPSM_DEFAULT);
CATCH_THROW()
}

int CNetworkLayer::GetCustomDirectStreamPort() const
{
TRY_CATCH
	return m_directStreamPort;
CATCH_THROW("CNetworkLayer::GetCustomDirectStreamPort")
}

int CNetworkLayer::GetDefaultDirectStreamPort() const
{
TRY_CATCH
	return DEFAULT_DIRECT_STREAM_PORT;
CATCH_THROW("CNetworkLayer::GetDefaultDirectStreamPort")
}

int CNetworkLayer::GetDirectStreamPort() const
{
TRY_CATCH
	switch(GetDirectStreamPortMode())
	{
		case EPSM_CUSTOM:
			return GetCustomDirectStreamPort();
		case EPSM_AUTO: //TODO: implement auto port configuration
		case EPSM_DEFAULT:
		default:
			return GetDefaultDirectStreamPort();
	}
CATCH_THROW("CNetworkLayer::GetDirectStreamPort")
}


void CNetworkLayer::SetProxySettings(const SHTTPProxySettings &proxySettings)
{
TRY_CATCH
	m_proxySettings = proxySettings;
CATCH_THROW("CNetworkLayer::SetProxySettings")
}

SHTTPProxySettings CNetworkLayer::GetProxySettings() const
{
TRY_CATCH
	return m_proxySettings;
CATCH_THROW("CNetworkLayer::GetProxySettings")
}

bool CNetworkLayer::GetUseProxy() const
{
TRY_CATCH
	return m_useProxy;
CATCH_THROW("CNetworkLayer::GetUseProxy")
}

void CNetworkLayer::SetUseProxy(const bool useProxy)
{
TRY_CATCH
	m_useProxy = useProxy;
CATCH_THROW("CNetworkLayer::SetUseProxy")
}

void CNetworkLayer::SetIMRetryInterval(const int IMRetryInterval)
{
TRY_CATCH
	m_IMRetryInterval = IMRetryInterval;
CATCH_THROW("CNetworkLayer::SetIMRetryInterval")
}

int CNetworkLayer::GetIMRetryInterval() const
{
TRY_CATCH
	return m_IMRetryInterval;
CATCH_THROW("CNetworkLayer::GetIMRetryInterval")
}

tstring CNetworkLayer::GetRelayPasswd() const
{
TRY_CATCH
	return m_relayPasswd;
CATCH_THROW("CNetworkLayer::GetRelayPasswd")
}

void CNetworkLayer::SetRelayPasswd(const tstring& passwd)
{
TRY_CATCH
	m_relayPasswd = passwd;
CATCH_THROW("CNetworkLayer::SetRelayPasswd")
}

int CNetworkLayer::GetTimeoutPortListen() const
{
TRY_CATCH
	return m_timeout_port_availability;
CATCH_THROW()
}

void CNetworkLayer::SetTimeoutPortListen(const int timeout)
{
TRY_CATCH
	m_timeout_port_availability=timeout;
CATCH_THROW()
}

void CNetworkLayer::SetUseSendTimeout(const bool useSendTimeout)
{
TRY_CATCH
	m_useSendTimeout = useSendTimeout;
CATCH_THROW()
}

bool CNetworkLayer::GetUseSendTimeout() const
{
TRY_CATCH
	return m_useSendTimeout;
CATCH_THROW()
}