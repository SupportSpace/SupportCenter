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

#pragma once
#include <NWL/NetworkLayer.h>
#include <AidLib/Strings/tstring.h>
#include "SHTTPProxySettings.h"
#include "CStunConnectSettings.h"
#include <windows.h>


/// NWL defaults
/// default relay host
#define DEFAULT_RELAY_HOST "192.168.0.20"
//#define DEFAULT_RELAY_HOST "213.8.114.131"

/// default relay UDP port
#define DEFAULT_RELAY_UDP_PORT 5902

/// default relay TCP port
#define DEFAULT_RELAY_TCP_PORT 5904
//#define DEFAULT_RELAY_TCP_PORT 5900

/// default port for direct stream
#define DEFAULT_DIRECT_STREAM_PORT 5910
#define DIRECT_STREAM_KEY_NAME _T("DirectStreamPort")
#define DIRECT_STREAM_PORT_MODE_KEY_NAME _T("DirectStreamPortMode")

/// Port for IM stub
#define IM_PORT 5903

/// Default delay for auth requests
#define DEFAULT_STUN_AUTH_DELAY 1000

/// Default max count for auth requests
#define DEFAULT_STUN_AUTH_MAX_COUNT 3

/// Default delay for bind requests
#define DEFAULT_STUN_BIND_DELAY 3000

/// Default max count for bind requests
#define DEFAULT_STUN_BIND_MAX_COUNT 15

/// Default delay for probe requests
#define DEFAULT_STUN_PROBE_DELAY 3000

/// Default max count for probe requests
#define DEFAULT_STUN_PROBE_MAX_COUNT 3

/// Default port range for probe requests
#define DEFAULT_STUN_PROBE_PORT_RANGE 3

/// Default interval for instant messaging send/receive msg retries
#define DEFAULT_IM_RETRY_INTERVAL 1000

/// Path to NWL registry keys
#define NWL_REGISTRY_PATH _T("SOFTWARE\\SupportSpace\\NWL")

/// Default password for authentication on relay server
#define DEFAULT_RELAY_PASSWD _T("A7B3F3CA-0096-4d02-8936-31B2392F973F")

/// Port selection mode
typedef enum _EPortSelectionMode
{
	EPSM_DEFAULT,	// Use default direct stream port
	EPSM_CUSTOM,	// Use custom direct stream port
	EPSM_AUTO		// Autoconfigure direct stream port if connecting from CStreamFactory, or use default elseway
} EPortSelectionMode;
/// Default timeout for checking port availability by server 
#define DEFAULT_TIMEOUT_FOR_PORT_LISTENING		5000


/// NWL class (NWL setting etc)
class NWL_API CNetworkLayer :
	public CStunConnectSettings
{
protected:
	/// relay server host address
	tstring m_relayHost;
	
	/// UDP port on relay for NAT traversal
	int m_relayUDPPort;

	/// TCP port on relay for relaying
	int m_relayTCPPort;

	/// Port for direct stream
	int m_directStreamPort;

	/// Proxy settings
	SHTTPProxySettings m_proxySettings;

	/// IM Stun Stub port
	int m_IMStubPort;

	/// true if we should use proxy
	bool m_useProxy;

	/// interval for instant messaging send/receive msg retries
	int m_IMRetryInterval;

	/// password for authentication on relay server
	tstring m_relayPasswd;
	
	/// timeout for checking port availability by server
	int m_timeout_port_availability;

	/// Use send timeout within socket stream
	bool m_useSendTimeout;

	/// Port selection mode for direct streams
	EPortSelectionMode m_directStreamPortMode;

	/// Saves value to registry
	static void SaveValue(const tstring& valueName, const int value);

	/// Restores value from registry
	/// throws exception, if can't be read (not saved before for example)
	static void RestoreValue(const tstring &valueName, int &value);

	/// Removes value from registry
	static void ResetValue(const tstring &valueName);
public:
	CNetworkLayer();

	/// destroys object instance
	virtual ~CNetworkLayer();

	/// Set interval for instant messaging send/receive msg retries
	/// @param IMRetryInterval new interval value
	void SetIMRetryInterval(const int IMRetryInterval);

	/// Returns retry interval for instant messaging send/receive msg retries
	/// @return retry interval for instant messaging send/receive msg retries
	int GetIMRetryInterval() const;

	/// set relay host
	/// @param relayHost new relay host address
	void SetRelayHost(const tstring& relayHost);

	/// returns relay host address
	/// @return relay host address
	tstring GetRelayHost() const;

	/// Setup relay port for NAT traversal
	/// @param relayUDPPort new relay UDP port
	void SetRelayUDPPort(const int relayUDPPort);

	/// Returns relay port for NAT traversal
	/// @return relay port for NAT traversal
	int GetRelayUDPPort() const;

	/// Setup relay port for relaying
	/// @param relayTCPPort new relay TCP port
	void SetRelayTCPPort(const int relayTCPPort);

	/// Returns relay port for relaing
	/// @return relay port for relaing
	int GetRelayTCPPort() const;

	/// Set port for IM stub port
	/// @param IMStubPort new IM stub port
	void SetIMStubPort(const int IMStubPort);

	/// Returns IM stub port
	/// @return IM stub port
	int GetIMStubPort() const;

	/// Setup port for direct stream
	/// @param directStreamPort new port for direct stream
	/// @param permanently save value to registry as well. Next NWL init will load this value
	void SetCustomDirectStreamPort(const int directStreamPort, const bool permanently = false);

	/// Setup port for direct stream
	/// @param portMode new port mode for direct stream
	/// @param permanently save value to registry as well. Next NWL init will load this value
	void SetDirectStreamPortMode(const EPortSelectionMode portMode, const bool permanently = false);

	/// Returns current value for direct stream port mode
	EPortSelectionMode GetDirectStreamPortMode() const;

	/// Set default value for direct stream port
	/// Removes permanent value for DirectStream Port. (Removes key from registry)
	void Reset2DefaultDirectStreamPort();

	/// Returns custom port for direct stream
	/// @return port for direct stream
	int GetCustomDirectStreamPort() const;

	/// Returns default direct stream port
	int GetDefaultDirectStreamPort() const;

	/// Return port for direct stream (depending on m_directStreamPortMode)
	int GetDirectStreamPort() const;

	/// Setup proxy settings
	/// @param proxySettings new proxy settings set
	void SetProxySettings(const SHTTPProxySettings &proxySettings);

	/// Returns use proxy flag
	/// @return use proxy flag
	bool GetUseProxy() const;

	/// Setup use proxy flag
	/// @param useProxy proxy usage flag
	void SetUseProxy(const bool useProxy);

	/// Returns proxy settings
	/// @return current proxy settings
	SHTTPProxySettings GetProxySettings() const;

	/// Returns password for authentication on relay server
	/// @return password string
	tstring GetRelayPasswd() const;

	/// Sets new relay password
	/// @param passwd - new password 
	void SetRelayPasswd(const tstring& passwd);

	/// Returns timeout for checking port availability by server
	/// @return timeout
	int GetTimeoutPortListen() const;

	/// Sets timeout for checking port availability by server
	/// @param timeout - new timeout 
	void SetTimeoutPortListen(const int timeout);

	/// Sets useSendTimeout flag value
	void SetUseSendTimeout(const bool useSendTimeout);

	/// Returns useSendTimeout flag value
	bool GetUseSendTimeout() const;
};

#define NWL_INSTANCE CProcessSingleton<CNetworkLayer>::instance()
