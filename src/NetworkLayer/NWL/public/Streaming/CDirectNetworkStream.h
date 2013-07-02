/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CDirectNetworkStream.h
///
///  Declares CDirectNetworkStream class, responsible for direct end-to-end stream
///
///  @author Dmitry Netrebenko @date 10.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once
#include <NWL/Streaming/CTLSSocketStream.h>
#include <NWL/NetworkLayer.h>
#include "CAbstractNetworkStream.h"
#include "CTLSAuthSettings.h"
#include "EDirectStreamType.h"
#include "CHTTPProxy.h"
#include <boost/shared_ptr.hpp>

typedef boost::shared_ptr<CTLSSocketStream> SPTLSSocketStream;

///  CDirectNetworkStream class, responsible for direct end-to-end stream
///  Base class CAbstractNetworkStream Responsible for abstract network streams
///  Base class CHTTPProxy responsible for management of HTTP proxy's settings
///  Base class CTLSAuthSettings responsible for GNUTLS authentication settings
///  @remarks
class NWL_API CDirectNetworkStream
	:	public CAbstractNetworkStream
	,	public CHTTPProxy
	,	public CTLSAuthSettings
{
private:
/// Prevents making copies of CDirectNetworkStream objects.
	CDirectNetworkStream( const CDirectNetworkStream& );
	CDirectNetworkStream& operator=( const CDirectNetworkStream& );

public:
///  Constructor
	CDirectNetworkStream();

///  Destructor
	~CDirectNetworkStream();

private:
/// Streams
	SPTLSSocketStream		m_pServerStream;
	SPTLSSocketStream		m_pClientStream;

/// Active (connected) stream
	SPTLSSocketStream		m_pActiveStream;

/// Stream type (server, client, etc)
	EDirectStreamType		m_DirectStream;

/// Critical section for events handlers
	CRITICAL_SECTION		m_csConnectSection;

/// Event for connection
	HANDLE					m_hConnectEvent;

	unsigned int			m_nErrorReports;
	bool					m_bAuthFailed;

	EConnectErrorReason		m_LastConnectError;

/// Remote port
	unsigned int			m_nRemotePort;

/// Remote host
	tstring					m_strRemoteAddr;

/// Local port
	unsigned int			m_nLocalPort;

public:
///  Establish connection
///  @param   do asynchronous connect
///  @remarks
	virtual void Connect( const bool = false );

///  Close connection
///  @remarks
	virtual void Disconnect();

///  Cancel reading from the stream
///  @remarks
	virtual void CancelReceiveOperation();

///  Checks data in the stream
///  @return returns amount of available data
	virtual bool HasInData();

///  Sets up local address
///  @param   local port
///  @remarks
	void SetLocalAddr( const unsigned int& );

///  Sets up remote address and port
///  @param   remote address
///  @param   remote port
///  @remarks
	void SetRemoteAddr( const tstring&, const unsigned int& );

///  Returns remote port
///  @return remote port
///  @remarks
	unsigned int GetRemotePort() const;

///  Returns renote address
///  @return remote address
///  @remarks
	tstring GetRemoteAddr() const;

///  Returns local port
///  @return local port
///  @remarks
	unsigned int GetLocalPort() const;

private:
///  Initializes streams settings
///  @remarks
	void InitStreams();

///  Removes streams
///  @remarks
	void DestroyStreams();

///  Peer-To-Peer authentication
///  @remarks
	void P2PAuthenticate();

///  Checks stream's state
///  @remarks
	void CheckStream();

///  "Connected" event handler
///  @param   pointer to parameter
///  @remarks
	void OnConnected( void* );

///  "Connect Error" event handler
///  @param   pointer to parameter
///  @param   error reason
///  @remarks
	void OnConnectError( void*, EConnectErrorReason );

///  "Disconnected" event handler
///  @param   pointer to parameter
///  @remarks
	void OnDisconnected( void* );

	unsigned int ReceiveInternal( char*, const unsigned int& );
	unsigned int SendInternal( const char*, const unsigned int& );

public:
/// Returns true if connected, false othervay
/// @return true if connected, false othervay
	virtual bool Connected() const;

///  Extracts data from input buffer
///  @param   Pointer to buffer
///  @param   Buffer size
///  @return  Number of bytes
///  @remarks redefining in this class to improve performance
	virtual unsigned int GetInBuffer( char*, const unsigned int& );


///  Returns stream type
///  @return type of the stream
///  @remarks
	EDirectStreamType GetStreamType() const;	

protected:
///  Raises "Connect Error" event
///  @param Error reason
///  @remarks
	void RaiseConnectErrorEvent( EConnectErrorReason );
};
