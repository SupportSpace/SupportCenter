/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStunConnectThread.h
///
///  Declares CStunConnectThread class, responsible for connection to
///    remote peer through STUN server
///
///  @author Dmitry Netrebenko @date 27.11.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CSSocket.h"
#include "CSUDTSocket.h"
#include <AidLib/CThread/CThread.h>
#include "CRelayConnectSettings.h"
#include "CStunConnectSettings.h"
#include "relay_messages.h"
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <NWL/Events/Events.h>
#include <list>
#include <AidLib/cTime/cTime.h>
#include <NWL/Statistic/SStunConnectRuntime.h>

#pragma comment(lib,"Winmm.lib")

#define STUN_CLNT_BUFFER_SIZE 4096

/// List of peer addresses
typedef std::list<SPeerAddr> Addresses;

/// CStunConnectThread class, responsible for connection to
///   remote peer through STUN server
/// Base class CThread - base thread class
/// Base class CRelayConnectSettings - settings for connection to relay server
/// Base class CStunConnectSettings - settings for connection to STUN server
class CStunConnectThread
	:	public CThread
	,	public CRelayConnectSettings
	,	public CStunConnectSettings
{
private:
/// Prevents making copies of CStunConnectThread objects.
	CStunConnectThread( const CStunConnectThread& );
	CStunConnectThread& operator=( const CStunConnectThread& );
public:
/// Constructor
/// @param statistic - reference to runtime statistic data
	CStunConnectThread(SStunConnectRuntime& statistic);
/// Destructor
	~CStunConnectThread();
/// Thread's entry point
/// @param   Pointer to thread's parameters
	void Execute(void*);
/// Sets up sockets
/// @param   pointer to windows UDP socket
/// @param   pointer to UDT socket
	void SetSockets(CSSocket*, CSUDTSocket*);
/// Binds connection events
/// @param   "Connected" event
/// @param   "ConnectError" evant
	void BindEvents(NotifyEvent, ConnectErrorEvent);
private:
/// Local address
	tstring	m_strLocalAddr;
/// Local port
	unsigned int m_nLocalPort;
/// Remote address for message
	tstring	m_strRemoteAddr;
/// Remote port for message
	unsigned int m_nRemotePort;
/// Peer's address
	tstring	m_strPeerAddr;
/// Peer's port
	unsigned int m_nPeerPort;
/// Local buffer
	char m_Buffer[STUN_CLNT_BUFFER_SIZE];
/// Number of bytes in buffer
	unsigned int m_nBytesInBuffer;
/// Connection state
	unsigned int m_State;
/// Array to store challenge
	char m_Challenge[STUN_CHALLENGE_SIZE];
/// Array to store hash
	char m_Hash[STUN_HASH_SIZE];
/// UDP windows socket
	CSSocket* m_Sock;
/// UDT socket
	CSUDTSocket* m_UDTSock;
/// Connected event
	NotifyEvent	m_ConnectedEvent;
/// Connect error event
	ConnectErrorEvent m_ConnectErrorEvent;
/// List of local addresses
	Addresses m_LocalAddresses;
/// Count of auth requests
	unsigned int m_AuthTries;
/// Time of next auth request
	cDate m_NextAuthTime;
/// Count of bind requests
	unsigned int m_BindTries;
/// Time of next bind request
	cDate m_NextBindTime;
/// Count of probe requests
	unsigned int m_ProbeTries;
/// Time of next probe request
	cDate m_NextProbeTime;
/// List of remote addresses
	Addresses m_nRemoteAddresses;
/// Last connection error
	EConnectErrorReason m_LastConnectError;
/// Is thread is connected to peer
	bool m_bConnectedToPeer;
/// Runtime statistic data
	SStunConnectRuntime& m_statistic;
private:
/// Clears internal buffer
	void ClearBuffer();
/// Prepares socket and local addresses list
	void Prepare();
/// Extracts protocol message from internal buffer
/// @return shared pointer to extracted message
	SPStunMessage GetMessage();
/// Sends authentication request message
	void SendAuthRequest();
/// Sends bind request message
/// @param   add addresses list to request
	void SendBindRequest(bool);
/// Sends probe request messages to all addresses in list
	void SendProbeRequests();
/// Sends probe request to remote peer
/// @param   peer's address
/// @param   peer's port
	void SendProbeRequest(const tstring&, const unsigned int&);
/// Sends probe response message
/// @param   probe request message
	void SendProbeResponse(SStunMessage*);
/// Processes received message
/// @param   received message
	void ProcessStunMessage(SStunMessage*);
/// Processes authentication response message
/// @param   authentication response message
	void ProcessAuthResponse(SStunMessage*);
/// Processes authentication failed response message
/// @param   authentication failed response message
	void ProcessAuthFailedResponse(SStunMessage*);
/// Processes server busy response message
/// @param   server busy response message
	void ProcessServerBusyResponse(SStunMessage*);
/// Processes bind response message
/// @param   bind response message
	void ProcessBindResponse(SStunMessage*);
/// Processes probe request message
/// @param   probe request message
	void ProcessProbeRequest(SStunMessage*);
/// Processes probe response message
/// @param   probe response message
	void ProcessProbeResponse(SStunMessage*);
/// Connects to remote peer using UDT socket
	void ConnectThroughUDT();
/// Checks up connection state
/// @return true if while should be terminated
	bool CheckState();
};
