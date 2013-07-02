/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTCPListener.cpp
///
///  Implements CTCPListener class, responsible for listening incoming
///    TCP connections from log server 
///
///  @author Dmitry Netrebenko @date 20.03.2007
///
////////////////////////////////////////////////////////////////////////

#include <NetLog/CNetLog.h>
#include "CTCPListener.h"
#include <NetLog/CNetworkLog.h>
#include <AidLib/CThread/CThreadLS.h>
#include <NetLog/CConnectedLogSvr.h>
#include <NWL/Streaming/CStreamException.h>

CTCPListener::CTCPListener(const tstring& name, CNetworkLog& netLog)
	:	m_name(name)
	,	CThread()
	,	m_socket(stTCP)
	,	m_netLog(netLog)
{
	if (!m_socketSystem.Initialized())
		throw MCSocketStreamException(_T("Failed to initialize socket system. "));

	// Create socket
	if(!m_socket.Create())
		throw MCSocketStreamException(_T("Can not create TCP socket."));
	// Bind socket
	if(!m_socket.Bind())
		throw MCSocketStreamException(_T("Can not bind TCP socket."));
	
	// Obtaining local port
	m_port = m_socket.GetSockPort();

	// Listen socket
	if(!m_socket.Listen())
		throw MCSocketStreamException(_T("Can not listen TCP socket."));
}

CTCPListener::~CTCPListener()
{
}

void CTCPListener::Execute(void *Params)
{
	DISABLE_TRACE;
	SET_THREAD_LS;
	try
	{
		while(!Terminated())
		{
			if(!m_socket.ReadSelect(NETLOG_TCP_SELECT_TIME))
				continue;
			SPSocket connectedSocket(m_socket.Accept());
			if(!connectedSocket.get())
				continue;
			OnSocketConnected(connectedSocket);
		}
	}
	catch(...)
	{
	}
}

void CTCPListener::OnSocketConnected(SPSocket sock)
{
	try
	{
		// Create CSocketStrean
		CSocketStream* sockStream = new CSocketStream(sock);
		SPAbstractStream stream(sockStream);

		// Create CConnectedLogSvr
		SPConnectedLogSvr svr(new CConnectedLogSvr(m_netLog.GetNextSvrId(), stream));

		// Bind OnDisconnected event
		sockStream->SetDisconnectedEvent(boost::bind(&CConnectedLogSvr::OnStreamDisconnected, svr.get(), _1));

		// Add server to map
		m_netLog.AddConnectedSvr(svr);
	}
	catch(...)
	{
	}
}

unsigned int CTCPListener::GetPort() const
{
	try
	{
		return m_port;
	}
	catch(...)
	{
		return 0;
	}
}
