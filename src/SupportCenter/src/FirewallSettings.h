#pragma once

#include <NWL/Streaming/CNetworkLayer.h>
#include "AidLib\Strings\tstring.h"
#include <set>

class CFirewallSettings
{
public:
	CFirewallSettings(void);
	~CFirewallSettings(void);

	void ConfiguringPortOnRuterThroughUPnP(const int internalPort);

	void TestPortAvailabilityWithRelayD(
		tstring jabberUserName,
		tstring relayServerAddress,
		unsigned short relayServerPort,
		const int internalPort);

	/// Returns local addresses set
	static std::set<tstring> GetLocalAddresses();
		 
	int  GetDefaultDirectStreamPort() const;
	int  GetCustomDirectStreamPort() const;

	EPortSelectionMode GetDirectStreamPortMode() const;

	void SetCustomDirectStreamPort(const int directStreamPort);
	void SetDirectStreamPortMode(EPortSelectionMode portMode);

};
