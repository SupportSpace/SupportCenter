#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNetLog.h
///
///  Network log client part
///
///  @author Sogin Max @date 20.03.2007
///
////////////////////////////////////////////////////////////////////////

#include <AidLib/AidLib.h>
#include <NWL/NetworkLayer.h>

///  CUDPListener port
#define NETLOG_UDP_PORT 5905

///  CTCPListener port
#define NETLOG_TCP_PORT 5906

/// Broadcast datagramm from viewer
#define NETLOG_UDP_SERVER_REQUEST _T("Alive?")
