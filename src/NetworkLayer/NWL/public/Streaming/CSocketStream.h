/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSocketStream.h
///
///  Declares CSocketStream class, responsible for CSSocket streams.
///
///  @author Dmitry Netrebenko @date 11.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Streaming/CSSocket.h>
#include "CAbstractStream.h"
#include "CConnectEvents.h"
#include "CSocketConnectThread.h"
#include "ESocketStreamState.h"
#include "SHTTPProxySettings.h"
#include <NWL/NetworkLayer.h>
#include <boost/shared_ptr.hpp>

#define SEND_TIMEOUT 5000 /*5 sec send timeout*/
#define BUF_FITTING_TIMEOUT 2500 /*	2,5 kb means that maximut transfer time with 1kb/sec speed
									will not exceed send timeout - 5 sec*/

/// forvard declaration
class CSSocket;
///  Thread shared pointer
typedef boost::shared_ptr<CSocketConnectThread> SPSocketConnectThread;

class NWL_API CSocketStream 
	:	public virtual CAbstractStream
	,	public virtual CConnectEvents
{
private:
/// Prevents making copies of CSocketStream objects.
	CSocketStream( const CSocketStream& );				
	CSocketStream& operator=( const CSocketStream& );	
	
public:
///  Constructor
	CSocketStream();

///  Constructor for connected socket
	CSocketStream( SPSocket );

///  Destructor
	virtual ~CSocketStream();

protected:
/// Read operation cancelled
	bool						m_ReceiveCancelled;
//private:
/// Remote host
	tstring						m_strHost;
/// Remote or Local port
	unsigned int				m_nPort;
/// Sockets
	SPSocket					m_sSocket;
	SPSocket					m_sDataSocket;
/// Stream state
	ESocketStreamState			m_StreamState;
/// Connection thread
	SPSocketConnectThread		m_ConnectThread;
/// Proxy settings
	SHTTPProxySettings			m_ProxySettings;
/// Stream should connect through http proxy
	bool						m_bConnectThroughProxy;

public:
///  Connect to specified host/port
///  @param   Host
///  @param   Port
///  @remarks
	void Connect( const tstring&, const unsigned int&, bool sync = false);

///  Disconnect
///  @remarks
	void Disconnect();

///  Accepts client connections
///  @param   Port
///  @remarks
	void Accept( const unsigned int& );

///  Cancel reading from the socket
///  @remarks
	void CancelReceiveOperation();

///  Checks data in the stream
///  @return returns amount of available data
	virtual bool HasInData();

///  Puts data to queue
///  @param   buffer with data
///  @param   number of bytes to put
///  @remarks
	virtual void Send2Queue( const char*, const unsigned int& );

protected:
///  Get data from the stream
///  @param   buffer for data
///  @param   number of bytes to get
///  @remarks
	virtual unsigned int ReceiveInternal( char*, const unsigned int& );

///  Gut data to stream
///  @param   buffer with data
///  @param   number of bytes to put
///  @remarks
	virtual unsigned int SendInternal( const char*, const unsigned int& );

///  Raises "Connected" event
///  @remarks
	virtual void RaiseConnectedEvent();

///  Raises "Disconnected" event
///  @remarks
	virtual void RaiseDisconnectedEvent();

///  Raises "Connect Error" event
///  @param Error reason
///  @remarks
	virtual void RaiseConnectErrorEvent( EConnectErrorReason );

private:
///  Starts thread to create socket connect
///  @remarks
	void StartConnectThread();

///  Checks buffer
///  @param   Pointer to buffer
///  @remarks
	void CheckBuffer( const char* );

///  Destroy internal objects
///  @remarks
	void FreeObjects();

///  "Connected" event handler
///  @param   pointer to parameter
///  @remarks
	void OnSocketConnected( void* );

///  "Connect Error" event handler
///  @param   pointer to parameter
///  @param	  error reason
///  @remarks
	void OnSocketConnectError( void*, EConnectErrorReason );

	bool m_sendTimeOutIsSet;
	bool m_useSendTimeout;
	inline void TriggerSendTimeout( unsigned int bufSize );

public:
///  Returns settings of http proxy
///  @return SHTTPProxySettings structure
///  @remarks
	SHTTPProxySettings& GetProxySettings();
	
///  Sets settings of http proxy
///  @param   new proxy settings
///  @remarks
	void SetProxySettings( const SHTTPProxySettings& );

///  Whether owe a stream will connect through a proxy
///  @return true, if stream needs connect through proxy
///  @remarks
	bool GetConnectThroughProxy() const;
	
///  Sets value for "Connect Through Proxy"
///  @param   new value
///  @remarks
	void SetConnectThroughProxy( const bool );

///  Closes stream
///  @remarks
	void Close();

protected:
	bool	m_bBlockTransportError;
};


inline void CSocketStream::TriggerSendTimeout( unsigned int bufSize )
{
	if (!m_useSendTimeout)
		return;
	if (bufSize > BUF_FITTING_TIMEOUT)
	{
		if (m_sendTimeOutIsSet)
		{
			/// Reseting timeout
			m_sDataSocket->SetTimeout(0, sstSend);
			m_sendTimeOutIsSet = false;
		}
	} else
	{
		if (!m_sendTimeOutIsSet)
		{
			/// Setting timeout
			m_sDataSocket->SetTimeout(SEND_TIMEOUT, sstSend);
			m_sendTimeOutIsSet = true;
		}
	}
}