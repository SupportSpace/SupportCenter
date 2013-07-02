/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLocalNetworkLogTransport.h
///
///  Declares CLocalNetworkLogTransport class, transport layer for local network
///
///  @author Dmitry Netrebenko @date 22.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NetLog/CLogTransportLayer.h>
#include "CTCPListener.h"
#include "CUDPListener.h"

///  CLocalNetworkLogTransport class, transport layer for local network
///  Base class - CLogTransportLayer
class CLocalNetworkLogTransport : public CLogTransportLayer
{
private:
///  Prevents making copies of CLocalNetworkLogTransport objects.
	CLocalNetworkLogTransport( const CLocalNetworkLogTransport& );
	CLocalNetworkLogTransport& operator=( const CLocalNetworkLogTransport& );

public:
///  Constructor
///  @param name - log name
///  @param netLog - reference to CNetworkLog
	CLocalNetworkLogTransport( const tstring& name, CNetworkLog& netLog );

///  Destructor
	~CLocalNetworkLogTransport();

private:
///  Listeners
	CTCPListener	m_tcpListener;
	CUDPListener	m_udpListener;
};
