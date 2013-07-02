/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLocalNetworkComLogTransport.h
///
///  Declares CLocalNetworkComLogTransport class, transport layer for local 
///    network which uses additional COM object for UDP listening
///
///  @author Dmitry Netrebenko @date 30.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NetLog/CLogTransportLayer.h>
#include "CTCPListener.h"
#include <NWL/Streaming/CSSocket.h>
#include "CComUDPListenerEventHandler.h"

///  CLocalNetworkComLogTransport class, transport layer for local 
///    network which uses additional COM object for UDP listening
///  Base class - CLogTransportLayer
///  Base class - CComUDPListenerEventHandler
class CLocalNetworkComLogTransport 
	:	public CLogTransportLayer
	,	public CComUDPListenerEventHandler
{
private:
///  Prevents making copies of CLocalNetworkComLogTransport objects.
	CLocalNetworkComLogTransport( const CLocalNetworkComLogTransport& );
	CLocalNetworkComLogTransport& operator=( const CLocalNetworkComLogTransport& );

public:
///  Constructor
///  @param name - log name
///  @param netLog - reference to CNetworkLog
	CLocalNetworkComLogTransport( const tstring& name, CNetworkLog& netLog );

///  Constructor
///  @param comListener - reference to ComUDPListener object
///  @param name - log name
///  @param netLog - reference to CNetworkLog
	CLocalNetworkComLogTransport( IComUDPListenerPtr comListener, const tstring& name, CNetworkLog& netLog );

///  Destructor
	~CLocalNetworkComLogTransport();

private:
///  Listeners
	CTCPListener		m_tcpListener;
///  UDP socket
	CSSocket			m_socket;
///  Procecess name
	tstring				m_processName;
///  Current preocess id
	unsigned int		m_processId;
///  CTCPListener port
	unsigned int		m_tcpListenerPort;
///  COM UDP lestener
	IComUDPListenerPtr	m_comUDPListener;

private:
///  Sends answer to log server
///  @param addr - remote address
///  @param port - remote port
	void SendResponseTo( const tstring& addr, const unsigned int port );

///  Initializes listener, socket and additional data
	void InitTransport();

///  DatagramReceived event handler
///  @param addr - remote address
///  @param port - remote port
	HRESULT __stdcall OnDatagramReceived ( BSTR addr, long port );

};
