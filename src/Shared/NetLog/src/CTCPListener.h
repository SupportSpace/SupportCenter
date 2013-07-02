/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTCPListener.h
///
///  Declares CTCPListener class, responsible for listening incoming
///    TCP connections from log server 
///
///  @author Dmitry Netrebenko @date 20.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <winsock2.h>
#include <AidLib/CThread/CThread.h>
#include <AidLib/Strings/tstring.h>
#include <NWL/Streaming/CSSocket.h>
#include <NetLog/CConnectedLogSvr.h>
#include <NWL/Streaming/CSocketStream.h>
#include <NWL/Streaming/CSocketSystem.h>

///  Forward declaration
class CNetworkLog;

///  Time in msecs for TCP socket select operation
#define NETLOG_TCP_SELECT_TIME 100

///  CTCPListener class, responsible for listening incoming
///    TCP connections from log server 
///  Base class - CThread
class CTCPListener : public CThread
{
private:
	/// Socket system for listener
	CSocketSystem m_socketSystem;

///  Prevents making copies of CTCPListener objects.
	CTCPListener( const CTCPListener& );
	CTCPListener& operator=( const CTCPListener& );

public:
///  Constructor
///  @param name - log name
///  @param netLog - reference to CNetworkLog
	CTCPListener( const tstring& name, CNetworkLog& netLog );

///  Destructor
	~CTCPListener();

///  Thread's entry point
///  @param Params - thread parameters
	virtual void Execute( void *Params );

///  Returns local TCP port
	unsigned int GetPort() const;

private:
///  Log name
	tstring				m_name;
///  TCP socket
	CSSocket			m_socket;
///  Reference to CNetworkLog
	CNetworkLog&		m_netLog;
///  Local TCP port
	unsigned int		m_port;

private:
///  Adds new log server to the map
///  @param sock - connected socket
	void OnSocketConnected( SPSocket sock );
};
