/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLocalNetworkLogTransport.cpp
///
///  Implements CLocalNetworkLogTransport class, transport layer for local network
///
///  @author Dmitry Netrebenko @date 22.03.2007
///
////////////////////////////////////////////////////////////////////////

#include "CLocalNetworkLogTransport.h"

CLocalNetworkLogTransport::CLocalNetworkLogTransport(const tstring& name, CNetworkLog& netLog)
	:	CLogTransportLayer(name, netLog)
	,	m_tcpListener(name, netLog)
	,	m_udpListener(name)
{
	m_tcpListener.Start();
	m_udpListener.SetTCPListenerPort(m_tcpListener.GetPort());
	m_udpListener.Start();
}

CLocalNetworkLogTransport::~CLocalNetworkLogTransport()
{
	try
	{
		m_tcpListener.Terminate();
		m_udpListener.Terminate();
	}
	catch(...)
	{
	}
}
