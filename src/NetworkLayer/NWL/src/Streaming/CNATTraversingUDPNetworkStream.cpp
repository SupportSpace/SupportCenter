/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNATTraversingUDPNetworkStream.cpp
///
///  Implements CNATTraversingUDPNetworkStream class, responsible for NAT
///    traversal streams
///
///  @author Dmitry Netrebenko @date 24.11.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CNATTraversingUDPNetworkStream.h>
#include <AidLib/Logging/cLog.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <udt.h>
#include <NWL/Streaming/CFirewallConfigurator.h>
#include <AidLib/CSingleton/CSingleton.h>
#include <NWL/Statistic/CStatisticClient.h>
#include <AidLib/CScopedTracker/CFinalFunction.h>

CNATTraversingUDPNetworkStream::CNATTraversingUDPNetworkStream()
	:	CAbstractServerNegotiatedNetworkStream()
	,	CStunConnectSettings()
	,	CTLSAuthSettings()
	,	m_sSocket(stUDP)
	,	m_sUDTSocket(stTCP)
	,	m_StreamState(ssDisconnected)
	,	m_LastConnectError(cerNoError)
	,	m_ReceiveCancelled(false)
	,	m_bBlockTransportError(false)
	,	m_bConnectedThroughTLS(false)
	,	m_bTLSSessionInitialized(false)
	,	m_nTLSBufferSize(0)
	,	m_statistic()
	,	m_ConnectThread(m_statistic)
{
TRY_CATCH
	// Initialize critical section
	InitializeCriticalSection(
		&m_csConnectSection);
	// Create event object
	m_hConnectEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL);
	if(!m_hConnectEvent)
		throw MCStreamException(_T("Event creation failed"));
	int ret = 0;
	// Allocate memory for credentials
	ret = gnutls_psk_allocate_client_credentials(&m_TLSClientCredentials);
	if(ret)
		throw MCStreamException(Format(_T("Allocating client credentials failed: %s"), gnutls_strerror(ret)));
	ret = gnutls_psk_allocate_server_credentials(&m_TLSServerCredentials);
	if(ret)
		throw MCStreamException(Format(_T("Allocating server credentials failed: %s"), gnutls_strerror(ret)));
	// Initialize priorities
	m_ProtocolPriority[0] = GNUTLS_TLS1_1;
	m_ProtocolPriority[1] = 0;

	m_MacPriority[0] = GNUTLS_MAC_SHA1;
	m_MacPriority[1] = 0;

	m_CompressionPriority[0] = 0;
	m_CompressionPriority[1] = 0;

	m_CipherPriority[0] = 0;
	m_CipherPriority[1] = 0;

	m_KeyExchangePriority[0] = 0;
	m_KeyExchangePriority[1] = 0;

	// Initialize DH params
	ret = gnutls_dh_params_init(&m_DHPparams);
	if(ret)
		throw MCStreamException(Format(_T("Initializing DH params failed: %s"), gnutls_strerror(ret)));

	// Initialize critical section
	InitializeCriticalSection(&m_csHandshakeSection);

	TRY_CATCH
		// Configure firewall for incoming connections
		FIREWALL_CONFIGURATOR_INSTANCE.AllowIncoming();
	CATCH_LOG()
CATCH_THROW()
}

CNATTraversingUDPNetworkStream::~CNATTraversingUDPNetworkStream()
{
TRY_CATCH
	// Terminate thread
	DWORD ThreadId = m_ConnectThread.GetTid();
	m_ConnectThread.Terminate();
	// Close sockets
	m_sUDTSocket.Close();
	m_sSocket.Close();
	// Wait for thread termination
	if((GetCurrentThreadId() != ThreadId) && ThreadId)
		WaitForSingleObject(m_ConnectThread.hTerminatedEvent.get(), INFINITE);
	// Close critical section
	DeleteCriticalSection(
		&m_csConnectSection);
	// Destroy event
	CloseHandle(m_hConnectEvent);
	// Close TLS session
	CloseTLSSession();
	// Free credentials
	gnutls_psk_free_client_credentials(m_TLSClientCredentials);
	gnutls_psk_free_server_credentials(m_TLSServerCredentials);
	// Free DH params
	gnutls_dh_params_deinit(m_DHPparams);
	// Close critical section
	DeleteCriticalSection(&m_csHandshakeSection);
CATCH_LOG()
}

void CNATTraversingUDPNetworkStream::Connect(const bool AsyncConnect)
{
TRY_CATCH
	// Check state
	if(ssDisconnected != m_StreamState)
		throw MCStreamException(_T("Connection already established"));
	// Reset last error
	m_LastConnectError = cerNoError;
	m_ReceiveCancelled = false;
	// Init statistic structure
	memset(&m_statistic, 0, sizeof(SStunConnectRuntime));
	// Store timeout
	m_statistic.m_timeout = m_nConnectTimeout;
	// Store time for starting connection
	m_statistic.m_total.m_startTime = timeGetTime();
	// Init sockets
	InitSockets();
	// Start connection thread
	StartConnectThread();
	m_StreamState = ssConnecting;
	// Reset event
	ResetEvent(m_hConnectEvent);
	if(AsyncConnect)
	{
		// Do nothing
	} 
	else
	{
		if(WAIT_TIMEOUT == WaitForSingleObject(m_hConnectEvent, m_nConnectTimeout))
		{

/// This block of code creates deadlock at TLS handshake if other side skips TLS authentication
		//// Enter critical section
		//CCritSection section( &m_csConnectSection );

		//if ( ssDisconnected != m_StreamState )
		//{
		//	
		//	RaiseConnectErrorEvent( cerTimeout );

		//	CloseConnect();

		//	m_StreamState = ssDisconnected;
		//}

			// Send connection statistic at leaving scope
			CFinalFunction final(boost::bind(&CNATTraversingUDPNetworkStream::SendStatistic, this));

			RaiseConnectErrorEvent(cerTimeout);
			CloseConnect();
			m_StreamState = ssDisconnected;
		}
	}

	switch(m_LastConnectError)
	{
	case cerUnknown:
		throw MCStreamException(_T("Unknown connection error"));
	case cerWinError:
		throw MCStreamException(_T("WinError occured"));
	case cerAuthFailed:
		throw MCStreamException(_T("Authentication failed"));
	case cerTimeout:
		throw MCStreamException(_T("Connection timeout expired"));
	case cerCancelled:
		throw MCStreamException(_T("Connect cancelled"));
	case cerTriesPassed:
		throw MCStreamException(_T("All connection tries are passed"));
	}
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::Disconnect()
{
TRY_CATCH
	CheckStream();
	RaiseDisconnectedEvent();
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::StartConnectThread()
{
TRY_CATCH
	// Set master/slave state
	m_ConnectThread.SetIsMaster(m_bIsMaster);
	// Set sockets
	m_ConnectThread.SetSockets(&m_sSocket, &m_sUDTSocket);
	// Bind events
	m_ConnectThread.BindEvents(
		boost::bind(&CNATTraversingUDPNetworkStream::OnSocketConnected, this, _1),
		boost::bind(&CNATTraversingUDPNetworkStream::OnSocketConnectError, this, _1, _2)
		);
	// Set relay server's settings
	m_ConnectThread.SetRelayServer( 
		m_strServerAddr, m_nServerPort, m_strSrvUserId, m_strSrvPassword);
	// Set connection
	m_ConnectThread.SetConnectionId(m_strConnectId, m_strLocalPeerId, m_strRemotePeerId);
	// Set auth retries
	m_ConnectThread.SetAuthRetry(m_nAuthRetryDelay, m_nAuthMaxRetryCount);
	// Set bind retries
	m_ConnectThread.SetBindRetry(m_nBindRetryDelay, m_nBindMaxRetryCount);
	// Set probe retries
	m_ConnectThread.SetProbeRetry(m_nProbeRetryDelay, m_nProbeMaxRetryCount);
	// Set probe port range
	m_ConnectThread.SetProbePortRange(m_nProbePortRange);
	// Start thread
	m_ConnectThread.Start();
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::CheckBuffer(const char* buf)
{
TRY_CATCH
	// Check buffer
	if(!buf) 
		throw MCStreamException(_T("buf == NULL"));
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::OnSocketConnected(void*)
{
TRY_CATCH
	// Enter critical section
	CCritSection section(&m_csConnectSection);
	if(ssDisconnected == m_StreamState)
		return;
	// Send connection statistic at leaving scope
	CFinalFunction final(boost::bind(&CNATTraversingUDPNetworkStream::SendStatistic, this));
	// Store time for starting TLS authentication
	m_statistic.m_tls.m_startTime = timeGetTime();
	// TLS authentication
	P2PAuthenticate();
	// Store time for end of TLS authentication
	m_statistic.m_tls.m_endTime = timeGetTime();
	m_StreamState = ssConnected;
	// Store time for end of connection
	m_statistic.m_total.m_endTime = timeGetTime();
	RaiseConnectedEvent();
	SetEvent(m_hConnectEvent);
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::OnSocketConnectError(void*, EConnectErrorReason reason)
{
TRY_CATCH
	// Store time for end of connection
	m_statistic.m_total.m_endTime = timeGetTime();
	// Enter critical section
	CCritSection section(&m_csConnectSection);
	if(ssDisconnected == m_StreamState)
		return;
	// Send connection statistic at leaving scope
	CFinalFunction final(boost::bind(&CNATTraversingUDPNetworkStream::SendStatistic, this));
	RaiseConnectErrorEvent(reason);
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::CloseConnect()
{
TRY_CATCH
	m_ConnectThread.Terminate();
	// Close sockets
	m_sUDTSocket.Close();
	m_sSocket.Close();
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::InitSockets()
{
TRY_CATCH
	if(!m_sSocket.Create())
		throw MCSocketStreamException(_T("m_sSocket.Create failed"));
	// Set read timeout
	if(!m_sSocket.SetTimeout(1000, sstReceive))
		throw MCSocketStreamException(_T("m_sSocket.SetTimeout failed"));
	// Create "rendezvous" UDT socket
	if(!m_sUDTSocket.Create(true/*, (SOCKET)m_sSocket*/))
		throw MCSocketStreamException(_T("m_sUDTSocket.Create() failed"));
	// Set read timeout
	if(!m_sUDTSocket.SetTimeout(1000, sstReceive))
		throw MCSocketStreamException(_T("m_sUDTSocket.SetTimeout failed"));
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::CheckStream()
{
TRY_CATCH
	if(ssConnected != m_StreamState)
		throw MCStreamException(_T("Connection not established"));
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::CancelReceiveOperation()
{
TRY_CATCH
	CheckStream();
	m_sUDTSocket.ReadSelect(100);
	// Set "Cancelled" flag
	m_ReceiveCancelled = true;
CATCH_THROW()
}

bool CNATTraversingUDPNetworkStream::HasInData()
{
TRY_CATCH
	// Check connection
	CheckStream();
	bool ret;
	// Check data in socket
	ret = m_sUDTSocket.ReadSelect(0);
	// Check data in TLS buffer
	if(m_bConnectedThroughTLS)
		ret = (gnutls_record_check_pending( m_TLSSession ) > 0) || ret;
	return ret;
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::Send2Queue(const char* pbuffer, const unsigned int& bufsize)
{
TRY_CATCH
	// Check buffer
	CheckBuffer(pbuffer);
	// Check state
	CheckStream();
	// Send to queue
	CAbstractStream::Send2Queue(pbuffer, bufsize);
CATCH_THROW()
}

unsigned int CNATTraversingUDPNetworkStream::ReceiveInternal(char* pbuffer, const unsigned int& bufsize)
{
TRY_CATCH
	// Check buffer
	CheckBuffer(pbuffer);
	// Check connection
	CheckStream();
	if(!bufsize)
		return 0;
	unsigned int ret = 0;
	if(m_bConnectedThroughTLS)
	{
		int n,length = bufsize;
		char* dataPtr = pbuffer;
		while(length > 0) 
		{
			n = gnutls_record_recv(m_TLSSession, dataPtr, length);
			if(n <= 0) //(!n) FIX
			{
				RaiseDisconnectedEvent();
				throw MCSocketStreamException(_T("ReadError"));
			}
			dataPtr += n;
			length -= n;
		}
		return bufsize;
	}
	else
		ret = SocketReceive(pbuffer, bufsize);

	if(!ret)
	{
		RaiseDisconnectedEvent();
		throw MCSocketStreamException(_T("ReadError"));
	}
	return ret;
CATCH_THROW()
}

unsigned int CNATTraversingUDPNetworkStream::SendInternal(const char* pbuffer, const unsigned int& bufsize)
{
TRY_CATCH
	// Check buffer
	CheckBuffer(pbuffer);
	// Check state
	CheckStream();
	unsigned int ret = 0;
	if(!bufsize )
		return 0;
	if(m_bConnectedThroughTLS)
	{
		if(!m_nTLSBufferSize)
			throw MCSocketStreamException(_T("TLS buffer size == 0"));
		unsigned int rest_size = bufsize;
		char* pbuf = const_cast<char*>(pbuffer);
		do
		{
			unsigned int sz = min(rest_size, m_nTLSBufferSize);
			ret = gnutls_record_send(m_TLSSession, pbuf, sz);
			if(ret <= 0) // (!ret) FIX
				break;
			rest_size -= sz;
			pbuf += sz;
		} while(rest_size);
	}
	else
		ret = SocketSend(pbuffer, bufsize);

	if(ret <= 0) //(!ret)FIX
	{
		RaiseDisconnectedEvent();
		throw MCSocketStreamException(_T("Failed to send"));
	}
	return ret;
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::RaiseDisconnectedEvent()
{
TRY_CATCH
	m_StreamState = ssDisconnecting;
	// Close session
	CloseTLSSession();
	CloseConnect();
	m_StreamState = ssDisconnected;
	if(m_DisconnectedEvent)
		m_DisconnectedEvent(this);
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::RaiseConnectErrorEvent(EConnectErrorReason reason)
{
TRY_CATCH
	m_LastConnectError = reason;
	CloseConnect();
	m_StreamState = ssDisconnected;
	if(m_ConnectErrorEvent)
		m_ConnectErrorEvent(this, reason);
	SetEvent(m_hConnectEvent);
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::InitTLSSession()
{
TRY_CATCH
	// Enable private extensions
	gnutls_handshake_set_private_extensions(m_TLSSession, 1);
	// Set default prioroties
	gnutls_set_default_priority(m_TLSSession);
	// Set protocol
	gnutls_protocol_set_priority(m_TLSSession, m_ProtocolPriority);
	// Set MAC priority
	gnutls_mac_set_priority(m_TLSSession, m_MacPriority);
	// Set Key Exchange algorithm priority
	m_KeyExchangePriority[0] = m_Suite.KeyExchange;
	gnutls_kx_set_priority(m_TLSSession, m_KeyExchangePriority);
	// Set cipher priority
	m_CipherPriority[0] = m_Suite.Cipher;
	gnutls_cipher_set_priority(m_TLSSession, m_CipherPriority);
	// Set compression priority
	m_CompressionPriority[0] = m_Suite.Compression;
	gnutls_compression_set_priority(m_TLSSession, m_CompressionPriority);
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::CloseTLSSession()
{
TRY_CATCH
	m_nTLSBufferSize = 0;
	// Connected through TLS
	if(m_bConnectedThroughTLS)
	{
		m_bConnectedThroughTLS = false; //avoiding recursion
		//TODO: This line of a code blocks connection termination if the remote peer does not respond
		// Close TLS connection
		//gnutls_bye( m_TLSSession, GNUTLS_SHUT_RDWR );
	}
	if(m_bTLSSessionInitialized)
	{
		// Close session
		gnutls_deinit(m_TLSSession);
		m_bTLSSessionInitialized = false;
		m_TLSSession = NULL;
	}
CATCH_THROW()
}

unsigned int CNATTraversingUDPNetworkStream::SocketReceive(char* pbuffer, const unsigned int& bufsize)
{
TRY_CATCH
	if(m_ReceiveCancelled)
	{
		m_ReceiveCancelled = false;
		throw MCStreamException(_T("Cancelled"));
	}
	int result;
	unsigned int i = 0;
	for(i = 0; i < bufsize;)
	{
		result = m_sUDTSocket.Receive(pbuffer + i, bufsize - i);
		if (result > 0)
		{
			i += result;
		} 
		else 
		if(!UDT::getlasterror().getErrorCode())
		{
			//Something ununderstandable from UDT
			if(m_ReceiveCancelled)
			{
				m_ReceiveCancelled = false;
				throw MCStreamException(_T("Cancelled"));
			}
			if(ssDisconnecting == m_StreamState)
				break;
		} 
		else
		if(!m_bBlockTransportError)
		{
			RaiseDisconnectedEvent();
			throw MCStreamException(Format(_T("ReadError: %d - %s"), 
				UDT::getlasterror().getErrorCode(), UDT::getlasterror().getErrorMessage()));
		}
		else
		{
			break;
		}
	}
	if(m_ReceiveCancelled)
	{
		m_ReceiveCancelled = false;
		throw MCStreamException(_T("Cancelled"));
	}
	return i;
CATCH_THROW()
}

unsigned int CNATTraversingUDPNetworkStream::SocketSend(const char* pbuffer, const unsigned int& bufsize)
{
TRY_CATCH
	if(!bufsize) 
		return 0;
	unsigned int ret = m_sUDTSocket.Send(pbuffer, bufsize);
	if(!ret && !m_bBlockTransportError)
	{
		RaiseDisconnectedEvent();
		throw MCSocketStreamException(_T("Failed to send"));
	}
	return ret;
CATCH_THROW()
}

int CNATTraversingUDPNetworkStream::GetUserCredentials(const char* user, gnutls_datum_t* key)
{
TRY_CATCH
	tstring strUser(user);
	if(!strUser.compare(m_Credentials.UserID))
	{
		unsigned int len = (unsigned int)m_Credentials.Key.length();
		key->size = len;
		key->data = (unsigned char*)malloc(key->size);
		memcpy(key->data, m_Credentials.Key.c_str(), len);
		return 0;
	}
	else
		return -1;
CATCH_THROW()
}

int CNATTraversingUDPNetworkStream::ServerCredentialsFunction(gnutls_session_t session, const char* user, gnutls_datum_t* key)
{
TRY_CATCH
	PTLSParam param = (PTLSParam)gnutls_session_get_ptr(session);
	CNATTraversingUDPNetworkStream* stream = param->Stream;
	return stream->GetUserCredentials(user, key);
CATCH_THROW()
}

ssize_t CNATTraversingUDPNetworkStream::ReceiveFromStream(gnutls_transport_ptr_t ptr, void* buf, size_t len)
{
TRY_CATCH
	PTLSParam param = (PTLSParam)ptr;
	CNATTraversingUDPNetworkStream* stream = param->Stream;
	ssize_t ret = stream->SocketReceive((char*)buf, len);
	return ret;
CATCH_THROW()
}

ssize_t CNATTraversingUDPNetworkStream::SendToStream(gnutls_transport_ptr_t ptr, const void* buf, size_t len)
{
TRY_CATCH
	PTLSParam param = (PTLSParam)ptr;
	CNATTraversingUDPNetworkStream* stream = param->Stream;
	ssize_t ret = stream->SocketSend((char*)buf, len);
	return ret;
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::ClientHandshake(const STLSCredentials& cred)
{
TRY_CATCH
	// Enter critical section
	CCritSection section(&m_csHandshakeSection);
	int ret = 0;
	// Check TLS connection
	if(m_bConnectedThroughTLS)
		throw MCStreamException(_T("Protected connection already established"));
	m_Credentials = cred;
	// Initialize client session
	ret = gnutls_init(&m_TLSSession, GNUTLS_CLIENT);
	if(ret)
		throw MCStreamException(Format(_T("Initializing session failed: %s"), gnutls_strerror(ret)));
	m_bTLSSessionInitialized = true;
	// Set up session's params
	InitTLSSession();
	// Setup parameter
	m_TLSParam.Stream = this;	
	// Create client credentials structure
	unsigned int len = (unsigned int)m_Credentials.Key.length();
	m_ClientKey.size = len;
	m_ClientKey.data = (unsigned char*)malloc(len);
	memcpy(m_ClientKey.data, m_Credentials.Key.c_str(), len);
	// Set client credentials
	ret = gnutls_psk_set_client_credentials( 
			m_TLSClientCredentials, 
			m_Credentials.UserID.c_str(), 
			&m_ClientKey, 
			GNUTLS_PSK_KEY_RAW);
	if(ret)
		throw MCStreamException(Format(_T("Set client credentials failed: %s"), gnutls_strerror(ret)));
	// Set credentials
	ret = gnutls_credentials_set(m_TLSSession, GNUTLS_CRD_PSK, m_TLSClientCredentials);
	if(ret)
		throw MCStreamException(Format(_T("Set credentials failed: %s"), gnutls_strerror(ret)));
	// Set transtort functions, parameters
	gnutls_transport_set_ptr(m_TLSSession, &m_TLSParam);
	gnutls_transport_set_push_function(m_TLSSession, &CNATTraversingUDPNetworkStream::SendToStream);
	gnutls_transport_set_pull_function(m_TLSSession, &CNATTraversingUDPNetworkStream::ReceiveFromStream);
	// Create TLS connection
	m_bBlockTransportError = true;
	ret = gnutls_handshake(m_TLSSession);
	m_bBlockTransportError = false;
	if(ret < 0)
	{
		gnutls_bye(m_TLSSession, GNUTLS_SHUT_RDWR);
		throw MCStreamException(Format(_T("Creation protected connection failed: %s"), gnutls_strerror(ret)));
	}
	m_bConnectedThroughTLS = true;
	m_nTLSBufferSize = (unsigned int)gnutls_record_get_max_size(m_TLSSession);
	if(!m_nTLSBufferSize)
		m_nTLSBufferSize = DEFAULT_TLS_BUFFER_SIZE;
	return;
CATCH_LOG()

TRY_CATCH
	m_bBlockTransportError = false;
	// Close session
	CloseTLSSession();
	RaiseConnectErrorEvent(cerAuthFailed);
	throw MCStreamException(_T("ClientHandshake failed"));
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::ServerHandshake(const STLSCredentials& cred)
{
TRY_CATCH
	// Enter critical section
	CCritSection section(&m_csHandshakeSection);
	int ret = 0;
	// Check TLS connection
	if(m_bConnectedThroughTLS)
		throw MCStreamException(_T("Protected connection already established"));
	// Store credentials
	m_Credentials = cred;
	if(KX_DHE_PSK == m_Suite.KeyExchange)
	{
		// Generate DH params
		ret = gnutls_dh_params_generate2(m_DHPparams, m_Suite.PrimeBits);
		if(ret)
			throw MCStreamException(Format(_T("Generating DH params failed: %s"), gnutls_strerror(ret)));
		// Set DH params
		gnutls_psk_set_server_dh_params(m_TLSServerCredentials, m_DHPparams);
	}
	// Initialize server session
	ret = gnutls_init(&m_TLSSession, GNUTLS_SERVER);
	if(ret)
		throw MCStreamException(Format(_T("Initializing session failed: %s"), gnutls_strerror(ret)));
	m_bTLSSessionInitialized = true;
	// Set up session's params
	InitTLSSession();
	// Setup parameter
	m_TLSParam.Stream = this;	
	// Set session parameter
	gnutls_session_set_ptr(m_TLSSession, &m_TLSParam);
	// Set credentials function
	gnutls_psk_set_server_credentials_function(m_TLSServerCredentials, &CNATTraversingUDPNetworkStream::ServerCredentialsFunction);
	// Set credentials
	ret = gnutls_credentials_set(m_TLSSession, GNUTLS_CRD_PSK, m_TLSServerCredentials);
	if(ret)
		throw MCStreamException(Format(_T("Set credentials failed: %s"), gnutls_strerror(ret)));
	if(KX_DHE_PSK == m_Suite.KeyExchange)
		// Set prime bits parameter
		gnutls_dh_set_prime_bits(m_TLSSession, m_Suite.PrimeBits);
	// Set transtort functions, parameters
	gnutls_transport_set_ptr(m_TLSSession, &m_TLSParam);
	gnutls_transport_set_push_function(m_TLSSession, &CNATTraversingUDPNetworkStream::SendToStream);
	gnutls_transport_set_pull_function(m_TLSSession, &CNATTraversingUDPNetworkStream::ReceiveFromStream);
	// Create TLS connection
	m_bBlockTransportError = true;
	ret = gnutls_handshake(m_TLSSession);
	m_bBlockTransportError = false;
	if(ret < 0)
	{
		gnutls_bye(m_TLSSession, GNUTLS_SHUT_RDWR);
		throw MCStreamException(Format(_T("Creation protected connection failed: %s"), gnutls_strerror(ret)));
	}
	m_bConnectedThroughTLS = true;
	m_nTLSBufferSize = (unsigned int)gnutls_record_get_max_size(m_TLSSession);
	if(!m_nTLSBufferSize)
		m_nTLSBufferSize = DEFAULT_TLS_BUFFER_SIZE;
	return;
CATCH_LOG()

TRY_CATCH
	m_bBlockTransportError = false;
	// Close session
	CloseTLSSession();
	RaiseConnectErrorEvent(cerAuthFailed);
	throw MCStreamException(_T("ServerHandshake failed"));
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::P2PAuthenticate()
{
TRY_CATCH
	// Check connection
	if((ssDisconnected == m_StreamState) || (ssDisconnecting == m_StreamState))
		throw MCStreamException(_T("Socket connection not established"));
	if(m_bIsMaster)
		ServerHandshake(m_Credentials);
	else
		ClientHandshake(m_Credentials);
CATCH_THROW()
}

unsigned int CNATTraversingUDPNetworkStream::GetInBuffer(char* buf, const unsigned int& len)
{
TRY_CATCH
	// Check connection
	CheckStream();
	if(m_ReceiveCancelled)
	{
		m_ReceiveCancelled = false;
		throw MCStreamException("Cancelled");
	}
	if(!HasInData()) return 0;
	int ret = gnutls_record_recv(m_TLSSession, buf, len);
	if(ret <= 0) // ( !ret )FIX
	{
		RaiseDisconnectedEvent();
		throw MCSocketStreamException(_T("Failed to read"));
	}
	return ret;
CATCH_THROW()
}

bool CNATTraversingUDPNetworkStream::Connected() const
{
TRY_CATCH
	return m_bConnectedThroughTLS;
CATCH_THROW()
}

SStatisticStunConnect CNATTraversingUDPNetworkStream::GetConnectStatistic() const
{
TRY_CATCH
	return m_statistic;
CATCH_THROW()
}

void CNATTraversingUDPNetworkStream::SendStatistic()
{
TRY_CATCH
	// Send connection statistic
	SStatisticStunConnect stunInfo = m_statistic;
	CStatisticClient::CreateStatisticMessage(_T(""), m_strLocalPeerId, m_strConnectId, -1, sttSTUNConnect, &stunInfo, sizeof(SStatisticStunConnect));
CATCH_LOG()
}

