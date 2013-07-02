/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  EStatisticMessageType.h
///
///  Declares EStatisticMessageType - enumeration for types of statistic
///    messages
///
///  @author Dmitry Netrebenko @date 11.10.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/NetworkLayer.h>

enum NWL_API EStatisticMessageType
{
	sttExpertRequestFeature				= 0,	/// Expert requested Feature
	sttCustomerApprovedFeature			= 1,	/// Customer approved Feature
	sttCustomerDeclinedFeature			= 2,	/// Customer declined Feature
	sttCustomerInstallFailed			= 3,	/// Customer’s side feature Installation failed
	sttCustomerUpdateFailed				= 4,	/// Customer’s side feature update failed
	sttNWLInitFailed					= 5,	/// Network Layer Initialization failed
	sttFeatureTerminatedByExpert		= 6,	/// Feature termination
	sttFeatureTerminatedByCustomer		= 7,	/// Feature termination
	sttFeatureTerminatedOnError			= 8,	/// Feature termination
	sttCustomerFeatureFirtsInit			= 9,	/// Customer first time Feature initialization
	sttRouterInfo						= 10,	/// UpnP Routers Information
	sttConnectType						= 11,	/// Feature NWL connection type
	sttFeatureSessionLength				= 12,	/// Feature Session length
	sttRCAvgSpeed						= 13,	/// Remote Control average speed
	sttRecordingEnabled					= 14,	/// Recording feature enabled
	sttToolsVersion						= 15,	/// Version number of Tools on Customer’s/Expert’s side
	sttSTUNConnect						= 16,	/// STUN connection statistic
	sttPingPongData						= 17    /// Keep a Live statistics measurement
};
  
