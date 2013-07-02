/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNATTraversingUDPNetworkStream.h
///
///  Declares CNATTraversingUDPNetworkStream class, responsible for NAT
///    traversal streams
///
///  @author Dmitry Netrebenko @date 24.11.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/NetworkLayer.h>
#include "CSSocket.h"
#include "CSUDTSocket.h"
#include "CAbstractServerNegotiatedNetworkStream.h"
#include "CStunConnectSettings.h"
#include "CTLSAuthSettings.h"
#include <NWL/TLS/TLSStructs.h>
#include "ESocketStreamState.h"
#include "CStunConnectThread.h"
#include <boost/shared_ptr.hpp>
#include <NWL/Statistic/SStunConnectRuntime.h>
#include <NWL/Statistic/SStatisticStunConnect.h>

/// CNATTraversingUDPNetworkStream class, responsible for NAT
///   traversal streams
/// Base class CAbstractServerNegotiatedNetworkStream - abstract class
///   for streams which negotiates using server
/// Base class CStunConnectSettings - settings for connection to STUN server
/// Base class CTLSAuthSettings responsible for GNUTLS authentication settings
class NWL_API CNATTraversingUDPNetworkStream
	:	public CAbstractServerNegotiatedNetworkStream
	,	public CStunConnectSettings
	,	public CTLSAuthSettings
{
private:
/// Prevents making copies of CNATTraversingUDPNetworkStream objects.
	CNATTraversingUDPNetworkStream( const CNATTraversingUDPNetworkStream& );
	CNATTraversingUDPNetworkStream& operator=( const CNATTraversingUDPNetworkStream& );

/// Transport callback parameter
	typedef struct
	{
		CNATTraversingUDPNetworkStream*	Stream;
	} TLSParam, *PTLSParam;
public:
/// Constructor
	CNATTraversingUDPNetworkStream();
/// Destructor
	virtual ~CNATTraversingUDPNetworkStream();
private:
/// Sockets
	CSSocket							m_sSocket;
	CSUDTSocket							m_sUDTSocket;
/// Stream state
	ESocketStreamState					m_StreamState;
/// Connection thread
	CStunConnectThread					m_ConnectThread;
/// Critical section for events handlers
	CRITICAL_SECTION					m_csConnectSection;
/// Event for connection
	HANDLE								m_hConnectEvent;
/// Connection error
	EConnectErrorReason					m_LastConnectError;
/// TLS connection established
	bool								m_bConnectedThroughTLS;
/// TLS session initialized
	bool								m_bTLSSessionInitialized;
/// TLS session
	gnutls_session_t					m_TLSSession;
/// TLS client credentials
	gnutls_psk_client_credentials_t		m_TLSClientCredentials;
/// TLS server credentials
	gnutls_psk_server_credentials_t		m_TLSServerCredentials;
/// TLS priorities
	int									m_KeyExchangePriority[2];
	int									m_CipherPriority[2];
	int									m_MacPriority[2];
	int									m_CompressionPriority[2];
	int									m_ProtocolPriority[2];
/// TLS transport layer parameter
	TLSParam							m_TLSParam;
/// TLS DH parameter
	gnutls_dh_params_t					m_DHPparams;
/// TLS key structure
	gnutls_datum_t						m_ClientKey;
/// Critical section for TLS handshake
	CRITICAL_SECTION					m_csHandshakeSection;
/// TLS buffer size
	unsigned int						m_nTLSBufferSize;
/// Runtime data for connection statistic
	SStunConnectRuntime					m_statistic;
protected:
/// Read operation cancelled
	bool								m_ReceiveCancelled;
	bool								m_bBlockTransportError;
public:
/// Connect to specified host/port
/// @param   do asynchronous connect
	virtual void Connect(const bool = false);
/// Returns true if connected, false othervay
/// @return true if connected, false othervay
	virtual bool Connected() const;
/// Disconnect
	virtual void Disconnect();
/// Cancel reading from the socket
	virtual void CancelReceiveOperation();
/// Checks data in the stream
/// @return returns amount of available data
	virtual bool HasInData();
/// Puts data to queue
/// @param   buffer with data
/// @param   number of bytes to put
	virtual void Send2Queue(const char*, const unsigned int&);
/// Extracts data from input buffer
/// @param   Pointer to buffer
/// @param   Buffer size
/// @return  Number of bytes
/// @remarks redefining in this class to improve performance
	virtual unsigned int GetInBuffer(char*, const unsigned int&);
/// Returns structure with statistic data for connection
	SStatisticStunConnect GetConnectStatistic() const;
private:
/// Starts thread to create socket connect
	void StartConnectThread();
/// Checks buffer
/// @param   Pointer to buffer
	void CheckBuffer(const char*);
/// "Connected" event handler
/// @param   pointer to parameter
	void OnSocketConnected(void*);
/// "Connect Error" event handler
/// @param   pointer to parameter
/// @param	  error reason
	void OnSocketConnectError(void*, EConnectErrorReason);
/// Closes connection
	void CloseConnect();
/// Inits sockets
	void InitSockets();
///  Checks stream state
	void CheckStream();
protected:
/// Get data from the stream
/// @param   buffer for data
/// @param   number of bytes to get
	virtual unsigned int ReceiveInternal(char*, const unsigned int&);
/// Put data to stream
/// @param   buffer with data
/// @param   number of bytes to put
	virtual unsigned int SendInternal(const char*, const unsigned int&);
/// Raises "Disconnected" event
	virtual void RaiseDisconnectedEvent();
/// Raises "Connect Error" event
/// @param Error reason
	virtual void RaiseConnectErrorEvent(EConnectErrorReason);
/// Sends protected data
/// @param   Pointer to parameter
/// @param   Pointer to buffer
/// @param   Size of buffer
/// @return number of sent bytes
	static ssize_t SendToStream(gnutls_transport_ptr_t, const void*, size_t);
/// Receives protected data
/// @param   Pointer to parameter
/// @param   Pointer to buffer
/// @param   Size of buffer
/// @return number of received bytes
	static ssize_t ReceiveFromStream(gnutls_transport_ptr_t, void*, size_t);
/// TLS server credentials function
/// @param   TLS session
/// @param   User name
/// @param   Secure key (out)
/// @return -1 if user not found, 0 if user found
	static int ServerCredentialsFunction(gnutls_session_t, const char*, gnutls_datum_t*);
private:
/// Initializes TLS session
	void InitTLSSession();
/// Receives from socket
/// @param   buffer for data
/// @param   number of bytes to get
	unsigned int SocketReceive(char*, const unsigned int&);
/// Sends through socket
/// @param   buffer with data
/// @param   number of bytes to put
	unsigned int SocketSend(const char*, const unsigned int&);
/// Obtains user's credentials
/// @param   User name
/// @param   Secure key (out)
/// @return -1 if user not found, 0 if user found
	int GetUserCredentials(const char*, gnutls_datum_t*);
/// Closes TLS session
	void CloseTLSSession();
/// TLS client side authentication
/// @param   credentials
	void ClientHandshake(const STLSCredentials&);
/// TLS server side authentication
/// @param   credentials
	void ServerHandshake(const STLSCredentials&);
/// Peer-To-Peer authentication
	void P2PAuthenticate();
/// Sends STUN connection statistic
	void SendStatistic();
};
