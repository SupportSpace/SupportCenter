/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  EConnectType.h
///
///  Declares EConnectType enumeration - types of NWL connections
///
///  @author Dmitry Netrebenko @date 11.10.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/NetworkLayer.h>

enum NWL_API EConnectType
{
	conDirect			= 0,	/// Direct connection
	conDirectProxy		= 1,	/// Deirect connection through http proxy
	conNat				= 2,	/// Connect through STUN server
	conRelay			= 3,	/// Connect through relay server
	conRelayProxy		= 4		/// Connect through relay server and http proxy
};

