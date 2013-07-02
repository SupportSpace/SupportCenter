/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CUDPListener.cpp
///
///  Implements CUDPListener class, responsible for listening incoming
///    UDP messages from log server 
///
///  @author Dmitry Netrebenko @date 20.03.2007
///
////////////////////////////////////////////////////////////////////////

#include "CUDPListener.h"
#include <AidLib/CThread/CThreadLS.h>
#include <NetLog/SUDPListenerMsg.h>
#include <NWL/Streaming/CStreamException.h>

CUDPListener::CUDPListener(const tstring& name)
	:	m_name(name)
	,	CThread()
	,	m_socket(stUDP)
{
	// Create socket
	if(!m_socket.Create())
		throw MCSocketStreamException(_T("Can not create UDP socket."));
	// Bind socket
	if(!m_socket.Bind(NETLOG_UDP_PORT))
		throw MCSocketStreamException(_T("Can not bind UDP socket."));

	// Clear buffer
	memset(m_buffer, 0, NETLOG_UDP_BUFFER_SIZE);

	TCHAR buf[MAX_PATH];
	GetModuleFileName(NULL, buf, MAX_PATH);
	m_processName = buf;

	tstring::size_type index = m_processName.find_last_of(_T("\\"));
	if(-1 != index)
		m_processName.erase(0, index + 1);
}

CUDPListener::~CUDPListener()
{

}

void CUDPListener::Execute(void *Params)
{
	DISABLE_TRACE;
	SET_THREAD_LS;

	try
	{
		while(!Terminated())
		{
			// Wait for incoming messages
			if(!m_socket.ReadSelect(NETLOG_UDP_SELECT_TIME))
				continue;

			tstring remoteAddr;
			unsigned int remotePort;
			memset(m_buffer, 0, NETLOG_UDP_BUFFER_SIZE);

			// Receive message
			unsigned int received = m_socket.ReceiveFrom(remoteAddr, remotePort, m_buffer, NETLOG_UDP_BUFFER_SIZE);

			if(!received)
				continue;

			// Validating message
			tstring msg(m_buffer);
			if(msg == NETLOG_UDP_SERVER_REQUEST)
			{
				// Send response
				SendResponseTo(remoteAddr, remotePort);
			}
		}
	}
	catch(...)
	{
	}
}

void CUDPListener::SendResponseTo(const tstring& addr, const unsigned int port)
{
	try
	{
		// Calculate size of response message
		unsigned int size = NETLOG_UDP_MESSAGE_HEADER_SIZE + static_cast<unsigned int>(m_processName.length());
		// Allocate memory for message
		SPUDPListenerMsg msg(reinterpret_cast<SUDPListenerMsg*>(new char[size]));
		// Set size
		msg->m_size = size;
		// Set process id
		msg->m_process = GetCurrentProcessId();
		// Set port of TCPListener
		msg->m_tcpPort = m_tcpListenerPort;
		// Copy process name to m_data field
		memcpy(msg->m_data, m_processName.c_str(), m_processName.length());
		// Send message through UDP
		m_socket.SendTo(addr, port, reinterpret_cast<char*>(msg.get()), msg->m_size);
	}
	catch(...)
	{
	}
}

void CUDPListener::SetTCPListenerPort(unsigned int port)
{
	try
	{
		m_tcpListenerPort = port;
	}
	catch(...)
	{
	}
}
