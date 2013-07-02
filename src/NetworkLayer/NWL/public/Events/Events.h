/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  Events.h
///
///  Declares event types
///
///  @author Dmitry Netrebenko @date 10.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <NWL/NetworkLayer.h>

typedef boost::function<void (void*)> NWL_API NotifyEvent;

enum EConnectErrorReason
{
	cerNoError		= 0,
	cerUnknown		= 1,	/// Unknown connect error
	cerWinError		= 2,	/// Windows error
	cerAuthFailed	= 3,	/// Authentication failed error
	cerTimeout		= 4,	/// Connect timeout
	cerCancelled	= 5,	/// Connect cancelled
	cerTriesPassed	= 6		/// All tries passed (for NAT traversal stream only)
};

typedef boost::function<void (void*, EConnectErrorReason)> NWL_API ConnectErrorEvent;

