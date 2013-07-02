/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTLSSocketStream.h
///
///  Declares CTLSSocketStream class, responsible for socket stream with TLS protection
///
///  @author Dmitry Netrebenko @date 11.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CSecureSocketStream.h"
#include "CTLSAuthSettings.h"
#include <NWL/TLS/TLSStructs.h>
#include <NWL/NetworkLayer.h>

///  CTLSSocketStream class, responsible for socket stream with TLS protection
///  Base class CSecureSocketStream - socket stream class with abstract security connection
///  Base class CTLSAuthSettings - GNUTLS authentication settings
class NWL_API CTLSSocketStream 
	:	public CSecureSocketStream
	,	public CTLSAuthSettings
{
private:
/// Prevents making copies of CTLSSocketStream objects.
	CTLSSocketStream( const CTLSSocketStream& );
	CTLSSocketStream& operator=( const CTLSSocketStream& );

/// Transport callback parameter
	typedef struct
	{
		CTLSSocketStream*	Stream;
	} TLSParam, *PTLSParam;

public:
///  Constructor
	CTLSSocketStream();

///  Destructor
	virtual ~CTLSSocketStream();

protected:
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

/// TLS key buffer
//	std::auto_ptr<unsigned char>		m_pKeyBuffer;

private:
/// Critical section for TLS handshake
	CRITICAL_SECTION					m_csHandshakeSection;

/// TLS buffer size
	unsigned int						m_nTLSBufferSize;

public:
///  Checks data in the stream
///  @return returns amount of available data
	virtual bool HasInData();

///  Extracts data from input buffer
///  @param   Pointer to buffer
///  @param   Buffer size
///  @return  Number of bytes
///  @remarks redefining in this class to improve performance
	virtual unsigned int GetInBuffer( char*, const unsigned int& );

///  Initializes secure connection
///  @param masterRole - stream has master role
	virtual void InitSecureConnection( bool masterRole );

///  Returns true if secure connection is established
	virtual bool HasSecureConnection() const;

protected:
	
///  Sends protected data
///  @param   Pointer to parameter
///  @param   Pointer to buffer
///  @param   Size of buffer
///  @return number of sent bytes
///  @remarks
	static ssize_t SendToStream( gnutls_transport_ptr_t, const void*, size_t );
	
///  Receives protected data
///  @param   Pointer to parameter
///  @param   Pointer to buffer
///  @param   Size of buffer
///  @return number of received bytes
///  @remarks
	static ssize_t ReceiveFromStream( gnutls_transport_ptr_t, void*, size_t );

///  TLS server credentials function
///  @param   TLS session
///  @param   User name
///  @param   Secure key (out)
///  @return -1 if user not found, 0 if user found
///  @remarks
	static int ServerCredentialsFunction( gnutls_session_t, const char*, gnutls_datum_t* );
	
///  Gets data from the stream
///  @param   buffer for data
///  @param   number of bytes to get
///  @remarks
	virtual unsigned int ReceiveInternal( char*, const unsigned int& );

///  Puts data to stream
///  @param   buffer with data
///  @param   number of bytes to put
///  @remarks
	virtual unsigned int SendInternal( const char*, const unsigned int& );

///  Raises "Disconnected" event
///  @remarks
	virtual void RaiseDisconnectedEvent();

private:
///  Initializes TLS session
///  @remarks
	void InitTLSSession();

///  Calls ReceiveInternal for base class
///  @param   buffer for data
///  @param   number of bytes to get
///  @remarks
	inline unsigned int BaseReceiveInternal( char*, const unsigned int& );

///  Calls SendInternal for base class
///  @param   buffer with data
///  @param   number of bytes to put
///  @remarks
	inline unsigned int BaseSendInternal( const char*, const unsigned int& );

///  Obtains user's credentials
///  @param   User name
///  @param   Secure key (out)
///  @return -1 if user not found, 0 if user found
///  @remarks
	int GetUserCredentials( const char*, gnutls_datum_t* );

///  Closes TLS session
///  @remarks
	void CloseTLSSession();

///  TLS client side authentication
///  @remarks
	void ClientHandshake();

///  TLS server side authentication
///  @remarks
	void ServerHandshake();
};


inline unsigned int CTLSSocketStream::BaseReceiveInternal( char* buf, const unsigned int& len )
{
TRY_CATCH

	return CSocketStream::ReceiveInternal( buf, len );

CATCH_THROW("CTLSSocketStream::BaseReceiveInternal")
}

inline unsigned int CTLSSocketStream::BaseSendInternal( const char* buf, const unsigned int& len )
{
TRY_CATCH

	return CSocketStream::SendInternal( buf, len );

CATCH_THROW("CTLSSocketStream::BaseSendInternal")
}