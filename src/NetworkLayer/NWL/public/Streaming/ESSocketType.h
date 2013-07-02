/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  ESSocketType.h
///
///  socket types
///
///  @author "Archer Software" Sogin M. @date 20.11.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <winsock2.h>

/// Socket type
enum ESSocketType
{
	stTCP	= SOCK_STREAM,  /// TCP socket
	stUDP	= SOCK_DGRAM	/// UDP socket
};
