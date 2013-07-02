/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CUDPListener.h
///
///  Declares CUDPListener class, responsible for listening incoming
///    UDP messages from log server 
///
///  @author Dmitry Netrebenko @date 20.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <winsock2.h>
#include <AidLib/CThread/CThread.h>
#include <AidLib/Strings/tstring.h>
#include <NWL/Streaming/CSSocket.h>
#include <NetLog/CNetLog.h>


///  Time in msecs for UDP socket select operation
#define NETLOG_UDP_SELECT_TIME 100
///  Buffer size for CUDPListener
#define NETLOG_UDP_BUFFER_SIZE 1024


///  CUDPListener class, responsible for listening incoming
///    UDP messages from log server
///  Base class - CThread
class CUDPListener : public CThread
{
private:
///  Prevents making copies of CUDPListener objects.
	CUDPListener( const CUDPListener& );
	CUDPListener& operator=( const CUDPListener& );

public:
///  Constructor
///  @param name - log name
	CUDPListener( const tstring& name );

///  Destructor
	~CUDPListener();

///  Thread's entry point
///  @param Params - thread parameters
	virtual void Execute( void *Params );

///  Set local port of TCPListener
	void SetTCPListenerPort( unsigned int port );

private:
///  Log name
	tstring			m_name;
///  UDP socket
	CSSocket		m_socket;
///  Internal buffer
	char			m_buffer[NETLOG_UDP_BUFFER_SIZE];
///  Procecess name
	tstring			m_processName;
///  CTCPListener port
	unsigned int	m_tcpListenerPort;

private:
///  Sends answer to log server
///  @param addr - remote address
///  @param port - remote port
	void SendResponseTo( const tstring& addr, const unsigned int port );

};
