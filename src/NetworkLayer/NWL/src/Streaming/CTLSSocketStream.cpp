/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTLSSocketStream.cpp
///
///  Implements CTLSSocketStream class, responsible for socket stream with TLS protection
///
///  @author Dmitry Netrebenko @date 11.10.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CTLSSocketStream.h>
#include <NWL/Streaming/CStreamException.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <boost/scoped_ptr.hpp>

CTLSSocketStream::CTLSSocketStream()
	:	CSecureSocketStream()
	,	CTLSAuthSettings()
	,	m_bConnectedThroughTLS( false )
	,	m_bTLSSessionInitialized( false )
	,	m_nTLSBufferSize(0)
{
TRY_CATCH

	int ret = 0;

	// Allocate memory for credentials
	ret = gnutls_psk_allocate_client_credentials( &m_TLSClientCredentials );
	if ( ret )
		throw MCStreamException( Format( _T("Allocating client credentials failed: %s"), gnutls_strerror( ret ) ) );

	ret = gnutls_psk_allocate_server_credentials( &m_TLSServerCredentials );
	if ( ret )
		throw MCStreamException( Format( _T("Allocating server credentials failed: %s"), gnutls_strerror( ret ) ) );

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
	ret = gnutls_dh_params_init( &m_DHPparams );
	if ( ret )
		throw MCStreamException( Format( _T("Initializing DH params failed: %s"), gnutls_strerror( ret ) ) );

	// Initialize critical section
	InitializeCriticalSection( &m_csHandshakeSection );

CATCH_THROW("CTLSSocketStream::CTLSSocketStream")
}

CTLSSocketStream::~CTLSSocketStream()
{
TRY_CATCH

	CloseTLSSession();
	
	// Free credentials
	gnutls_psk_free_client_credentials( m_TLSClientCredentials );
	gnutls_psk_free_server_credentials( m_TLSServerCredentials );

	// Free DH params
	gnutls_dh_params_deinit( m_DHPparams );

	// Close critical section
	DeleteCriticalSection( &m_csHandshakeSection );

CATCH_LOG("CTLSSocketStream::~CTLSSocketStream")
}

void CTLSSocketStream::ClientHandshake()
{
TRY_CATCH

	// Enter critical section
	CCritSection section( &m_csHandshakeSection );

	int ret = 0;

	// Check TLS connection
	if ( m_bConnectedThroughTLS )
		throw MCStreamException( _T("Protected connection already established") );

	// Initialize client session
	ret = gnutls_init( &m_TLSSession, GNUTLS_CLIENT );
	if ( ret )
		throw MCStreamException( Format( _T("Initializing session failed: %s"), gnutls_strerror( ret ) ) );
	m_bTLSSessionInitialized = true;

	// Set up session's params
	InitTLSSession();

	// Setup parameter
	m_TLSParam.Stream = this;	

	// Create client credentials structure
	unsigned int len = (unsigned int)m_Credentials.Key.length();
	m_ClientKey.size = len;
	m_ClientKey.data = (unsigned char*)malloc( len );
//	m_pKeyBuffer.reset( m_ClientKey.data );
	memcpy( m_ClientKey.data, m_Credentials.Key.c_str(), len );

	// Set client credentials
	ret = gnutls_psk_set_client_credentials( 
			m_TLSClientCredentials, 
			m_Credentials.UserID.c_str(), 
			&m_ClientKey, 
//			GNUTLS_PSK_KEY_HEX );
			GNUTLS_PSK_KEY_RAW );
	if ( ret )
		throw MCStreamException( Format( _T("Set client credentials failed: %s"), gnutls_strerror( ret ) ) );

	// Set credentials
	ret = gnutls_credentials_set ( m_TLSSession, GNUTLS_CRD_PSK, m_TLSClientCredentials );
	if ( ret )
		throw MCStreamException( Format( _T("Set credentials failed: %s"), gnutls_strerror( ret ) ) );

	// Set transtort functions, parameters
	gnutls_transport_set_ptr( m_TLSSession, &m_TLSParam );
	gnutls_transport_set_push_function( m_TLSSession, &CTLSSocketStream::SendToStream );
	gnutls_transport_set_pull_function( m_TLSSession, &CTLSSocketStream::ReceiveFromStream );
	
	// Create TLS connection
	m_bBlockTransportError = true;
	ret = gnutls_handshake( m_TLSSession );
	m_bBlockTransportError = false;
	if ( ret < 0 )
	{
		gnutls_bye( m_TLSSession, GNUTLS_SHUT_RDWR );
		throw MCStreamException( Format( _T("Creation protected connection failed: %s"), gnutls_strerror( ret ) ) );
	}

	m_bConnectedThroughTLS = true;

	m_nTLSBufferSize = (unsigned int)gnutls_record_get_max_size( m_TLSSession );
	if ( !m_nTLSBufferSize )
		m_nTLSBufferSize = DEFAULT_TLS_BUFFER_SIZE;

	return;

CATCH_LOG("CTLSSocketStream::ClientHandshake")

TRY_CATCH

	m_bBlockTransportError = false;
	// Close session
	CloseTLSSession();

	RaiseConnectErrorEvent( cerAuthFailed );

	throw MCStreamException( _T("ClientHandshake failed") );

CATCH_THROW("CTLSSocketStream::ClientHandshake")
}

void CTLSSocketStream::ServerHandshake()
{
TRY_CATCH

	// Enter critical section
	CCritSection section( &m_csHandshakeSection );

	int ret = 0;
	
	// Check TLS connection
	if ( m_bConnectedThroughTLS )
		throw MCStreamException( _T("Protected connection already established") );

	if ( KX_DHE_PSK == m_Suite.KeyExchange )
	{
		// Generate DH params
		ret = gnutls_dh_params_generate2( m_DHPparams, m_Suite.PrimeBits );
		if ( ret )
			throw MCStreamException( Format( _T("Generating DH params failed: %s"), gnutls_strerror( ret ) ) );

		// Set DH params
		gnutls_psk_set_server_dh_params( m_TLSServerCredentials, m_DHPparams );
	}

	// Initialize server session
	ret = gnutls_init( &m_TLSSession, GNUTLS_SERVER );
	if ( ret )
		throw MCStreamException( Format( _T("Initializing session failed: %s"), gnutls_strerror( ret ) ) );
	m_bTLSSessionInitialized = true;

	// Set up session's params
	InitTLSSession();

	// Setup parameter
	m_TLSParam.Stream = this;	

	// Set session parameter
	gnutls_session_set_ptr( m_TLSSession, &m_TLSParam );

	// Set credentials function
	gnutls_psk_set_server_credentials_function( m_TLSServerCredentials, &CTLSSocketStream::ServerCredentialsFunction );
	
	// Set credentials
	ret = gnutls_credentials_set( m_TLSSession, GNUTLS_CRD_PSK, m_TLSServerCredentials );
	if ( ret )
		throw MCStreamException( Format( _T("Set credentials failed: %s"), gnutls_strerror( ret ) ) );

	if ( KX_DHE_PSK == m_Suite.KeyExchange )
		// Set prime bits parameter
		gnutls_dh_set_prime_bits ( m_TLSSession, m_Suite.PrimeBits );

	// Set transtort functions, parameters
	gnutls_transport_set_ptr( m_TLSSession, &m_TLSParam );
	gnutls_transport_set_push_function( m_TLSSession, &CTLSSocketStream::SendToStream );
	gnutls_transport_set_pull_function( m_TLSSession, &CTLSSocketStream::ReceiveFromStream );
	
	// Create TLS connection
	m_bBlockTransportError = true;
	ret = gnutls_handshake( m_TLSSession );
	m_bBlockTransportError = false;
	if ( ret < 0 )
	{
		gnutls_bye( m_TLSSession, GNUTLS_SHUT_RDWR );
		throw MCStreamException( Format( _T("Creation protected connection failed: %s"), gnutls_strerror( ret ) ) );
	}

	m_bConnectedThroughTLS = true;

	m_nTLSBufferSize = (unsigned int)gnutls_record_get_max_size( m_TLSSession );
	if ( !m_nTLSBufferSize )
		m_nTLSBufferSize = DEFAULT_TLS_BUFFER_SIZE;

	return;

CATCH_LOG("CTLSSocketStream::ServerHandshake")

TRY_CATCH

	m_bBlockTransportError = false;

	// Close session
	CloseTLSSession();

	RaiseConnectErrorEvent( cerAuthFailed );

	throw MCStreamException( _T("ServerHandshake failed") );

CATCH_THROW("CTLSSocketStream::ServerHandshake")
}

ssize_t CTLSSocketStream::SendToStream( gnutls_transport_ptr_t ptr, const void* buf, size_t len )
{
TRY_CATCH

	PTLSParam param = (PTLSParam)ptr;
	CTLSSocketStream* stream = param->Stream;
	ssize_t ret = stream->BaseSendInternal( (char*)buf, len );

	return ret;

CATCH_THROW("CTLSSocketStream::SendToStream")
}

ssize_t CTLSSocketStream::ReceiveFromStream( gnutls_transport_ptr_t ptr, void* buf, size_t len )
{
TRY_CATCH

	PTLSParam param = (PTLSParam)ptr;
	CTLSSocketStream* stream = param->Stream;
	ssize_t ret = stream->BaseReceiveInternal( (char*)buf, len );

	return ret;

CATCH_THROW("CTLSSocketStream::ReceiveFromStream")
}

unsigned int CTLSSocketStream::ReceiveInternal( char* buf, const unsigned int& len )
{
TRY_CATCH

	unsigned int ret = 0;

	if ( !len )
		return 0;

	if ( m_bConnectedThroughTLS )
	{
		int n,length = len;
		char* dataPtr = buf;
		while (length > 0) 
		{
			n = gnutls_record_recv( m_TLSSession, dataPtr, length );
			
			if ( n <= 0 ) //( !n )FIX
			{
				RaiseDisconnectedEvent();
				throw MCSocketStreamException( _T("ReadError") );
			}
			dataPtr += n;
			length -= n;
		}
		return len;
	}
	else
		ret = BaseReceiveInternal( buf, len );

	if ( !ret )
	{
		RaiseDisconnectedEvent();
		throw MCSocketStreamException( _T("ReadError") );
	}

	return ret;

CATCH_THROW("CTLSSocketStream::ReceiveInternal")
}

unsigned int CTLSSocketStream::SendInternal( const char* buf, const unsigned int& len )
{
TRY_CATCH

	unsigned int ret = 0;

	if ( !len )
		return 0;

	if ( m_bConnectedThroughTLS )
	{
		if ( !m_nTLSBufferSize )
			throw MCSocketStreamException( _T("TLS buffer size == 0") );
		unsigned int rest_size = len;
		char* pbuf = const_cast<char*>(buf);
		do
		{
			unsigned int sz = min( rest_size, m_nTLSBufferSize );
			ret = gnutls_record_send( m_TLSSession, pbuf, sz );
			if ( ret <=0 ) // ( !ret ) FIX
				break;
			rest_size -= sz;
			pbuf += sz;
		} while ( rest_size );
	}
	else
		ret = BaseSendInternal( buf, len );

	if ( ret <= 0 ) //( !ret )FIX
	{
		RaiseDisconnectedEvent();
		throw MCSocketStreamException( _T("Failed to send") );
	}

	return ret;

CATCH_THROW("CTLSSocketStream::SendInternal")
}

unsigned int CTLSSocketStream::GetInBuffer( char* buf, const unsigned int& len )
{
TRY_CATCH
	if (m_ReceiveCancelled)
	{
		m_ReceiveCancelled = false;
		throw MCStreamException("Cancelled");
	}
	if (!HasInData()) return 0;
	int ret = gnutls_record_recv( m_TLSSession, buf, len );
	if ( ret <= 0 ) //( !ret )FIX
		throw MCSocketStreamException( _T("Failed to read") );
	return ret;
CATCH_THROW("CTLSSocketStream::GetInBuffer")
}


void CTLSSocketStream::InitTLSSession()
{
TRY_CATCH

	// Enable private extensions
	gnutls_handshake_set_private_extensions( m_TLSSession, 1 );

	// Set default prioroties
	gnutls_set_default_priority( m_TLSSession );

	// Set protocol
	gnutls_protocol_set_priority( m_TLSSession, m_ProtocolPriority );

	// Set MAC priority
	gnutls_mac_set_priority( m_TLSSession, m_MacPriority );

	// Set Key Exchange algorithm priority
	m_KeyExchangePriority[0] = m_Suite.KeyExchange;
	gnutls_kx_set_priority( m_TLSSession, m_KeyExchangePriority );

	// Set cipher priority
	m_CipherPriority[0] = m_Suite.Cipher;
	gnutls_cipher_set_priority( m_TLSSession, m_CipherPriority );

	// Set compression priority
	m_CompressionPriority[0] = m_Suite.Compression;
	gnutls_compression_set_priority( m_TLSSession, m_CompressionPriority );

CATCH_THROW("CTLSSocketStream::InitTLSSession")
}

void CTLSSocketStream::RaiseDisconnectedEvent()
{
TRY_CATCH

	CloseTLSSession();

	// Raise event from base class
	CSocketStream::RaiseDisconnectedEvent();

	m_bConnectedThroughTLS = false;

CATCH_THROW("CTLSSocketStream::RaiseDisconnectedEvent")
}

int CTLSSocketStream::ServerCredentialsFunction( gnutls_session_t session, const char* user, gnutls_datum_t* key )
{
TRY_CATCH

	PTLSParam param = (PTLSParam)gnutls_session_get_ptr( session );
	CTLSSocketStream* stream = param->Stream;
	return stream->GetUserCredentials( user, key );

CATCH_THROW("CTLSSocketStream::ServerCredentialsFunction")
}

int CTLSSocketStream::GetUserCredentials( const char* user, gnutls_datum_t* key )
{
TRY_CATCH

	tstring strUser( user );

	if ( !strUser.compare( m_Credentials.UserID ) )
	{
//		unsigned int len = (unsigned int)m_Credentials.Key.length();
//
//		gnutls_datum_t subkey;
//		subkey.size = len + 1;
//
//		subkey.data = (unsigned char*)malloc( subkey.size );
//		FillMemory( subkey.data, subkey.size, 0 );
//		memcpy( subkey.data, m_Credentials.Key.c_str(), len );
//		
//		key->size = len / 2;
//		key->data = (unsigned char*)malloc( key->size );
////		m_pKeyBuffer.reset( key->data );
//
//		int ret = gnutls_hex_decode( &subkey, (char*)key->data, &key->size );
//		
//		free( subkey.data );
//		
//		if ( ret < 0 )
//			throw MCStreamException( Format( _T("Decoding PSK failed: %s"), gnutls_strerror( ret ) ) );
	
		unsigned int len = (unsigned int)m_Credentials.Key.length();
		key->size = len;
		key->data = (unsigned char*)malloc( key->size );
		memcpy( key->data, m_Credentials.Key.c_str(), len );

		return 0;
	}
	else
		return -1;

CATCH_THROW("CTLSSocketStream::GetUserCredentials")
}

bool CTLSSocketStream::HasInData()
{
TRY_CATCH

	bool ret = CSocketStream::HasInData();

	if ( m_bConnectedThroughTLS )
		ret = ( gnutls_record_check_pending( m_TLSSession ) > 0 ) || ret;

	return ret;

CATCH_THROW("CTLSSocketStream::HasInData")
}

void CTLSSocketStream::CloseTLSSession()
{
TRY_CATCH

	// Enter critical section
	CCritSection section( &m_csHandshakeSection );

	m_nTLSBufferSize = 0;

	// Connected through TLS
	if ( m_bConnectedThroughTLS )
	{
		m_bConnectedThroughTLS = false; //avoiding recursion

		//TODO: This line of a code blocks connection termination if the remote peer does not respond
		// Close TLS connection
		//gnutls_bye( m_TLSSession, GNUTLS_SHUT_RDWR );
	}

	if ( m_bTLSSessionInitialized )
	{
		// Close session
		gnutls_deinit( m_TLSSession );
		m_bTLSSessionInitialized = false;
		m_TLSSession = NULL;
	}

CATCH_THROW("CTLSSocketStream::CloseTLSSession")
}

void CTLSSocketStream::InitSecureConnection(bool masterRole)
{
TRY_CATCH

	if(masterRole)
		ServerHandshake();
	else
		ClientHandshake();

CATCH_THROW("CTLSSocketStream::InitSecureConnection")
}

bool CTLSSocketStream::HasSecureConnection() const
{
TRY_CATCH

	return m_bConnectedThroughTLS;

CATCH_THROW("CTLSSocketStream::HasSecureConnection")
}

