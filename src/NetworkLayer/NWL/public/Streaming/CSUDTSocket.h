/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSUDTSocket.h
///
///  Declares CSUDTSocket class, UDT socket wrapper
///
///  @author Dmitry Netrebenko @date 23.11.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "ESSocketTimeout.h"
#include <AidLib/Strings/tstring.h>
#include <winsock2.h>

///  UDT socket wrapper
///  @remarks
class CSUDTSocket
{
private:
/// Prevents making copies of CSUDTSocket objects.
	CSUDTSocket( const CSUDTSocket& );
	CSUDTSocket& operator=( const CSUDTSocket& );

public:
///  Constructor
///  @param   socket type
	CSUDTSocket( int = SOCK_STREAM );

///  Destructor
	~CSUDTSocket();

///  Creates UDT socket and attach it to this object
///  @param Use rendezvous connection setup method
///  @param windows socket object to attach UDT socket to
///    existing window socket
///  @return true, if operation completed successfully
///  @remarks
	bool Create( bool = false, SOCKET = INVALID_SOCKET );

///  Shutdows attached socket
///  @return true, if operation completed successfully
///  @remarks
	bool Shutdown();

///  Closes attached socket
///  @return true, if operation completed successfully
///  @remarks
  bool Close();

///  Binds socket to specified port
///  @param   Local port
///  @return true, if operation completed successfully
///  @remarks
	bool Bind( const unsigned int& = 0 );

///  Connects to specified remote address and port
///  @param   Remote Address
///  @param   Remote Port
///  @return true, if operation completed successfully
///  @remarks
	bool Connect( const tstring&, const unsigned int& );

///  Listen for incoming connections
///  @return true, if operation completed successfully
///  @remarks
	bool Listen();

///  Accepts incoming connection
///  @return reference to connected socket
///  @remarks
	CSUDTSocket* Accept();

///  Sets timeout value
///  @param   timeout in milliseconds
///  @param	  timeout type (default = receive && send)
///  @return true, if operation completed successfully
///  @remarks
	bool SetTimeout( unsigned int, ESSocketTimeout time = sstAll );

///  Check for incoming data
///  @param   time in milliseconds
///  @return true, if operation completed successfully
///  @remarks
	bool ReadSelect( const unsigned int );

///  Sends data
///  @param   buffer to send
///  @param   size of buffer
///  @return number of bytes sent
///  @remarks
///  @param   time in milliseconds
///  @return true, if operation completed successfully
///  @remarks
	bool WriteSelect( const unsigned int );
	
///  Sends data
///  @param   buffer to send
///  @param   size of buffer
///  @return number of bytes sent
///  @remarks
	unsigned int Send( const char*, const unsigned int& );
	
///  Sends message
///  @param   buffer to send
///  @param   size of buffer
///  @return number of bytes sent
///  @remarks
	unsigned int SendMsg( const char*, const unsigned int& );

//  Receives mesage
//  @param   buffer
//  @param   size of buffer
//  @return number of bytes received
//  @remarks
	unsigned int ReceiveMsg( char*, const unsigned int& );

///  Receives data
///  @param   buffer
///  @param   size of buffer
///  @return number of bytes received
	unsigned int Receive( char*, const unsigned int& );

///  Peeks at the incoming data
///  @param   buffer
///  @param   size of buffer
///  @return number of bytes received
	unsigned int Peek( char*, const unsigned int& );

///  Sends exactly number of bytes
///  @param   buffer to send
///  @param   size of buffer
///  @return true, if operation completed successfully
///  @remarks
	bool SendExact( const char*, const unsigned int& );

///  Receives exactly number of bytes
///  @param   buffer
///  @param   size of buffer
///  @return true, if operation completed successfully
///  @remarks
	bool ReceiveExact( char*, const unsigned int& );

/// Returns size of data buffer
/// @return size of data buffer
	int GetReadyDataCount();

/// Returns peer name
/// @return peer name
	tstring GetPeerName();

/// @returns local name
/// @return local name
	tstring GetSockName();

/// @returns peer port
/// @return peer port
	unsigned int GetPeerPort();

/// @returns local port
/// @return local port
	unsigned int GetSockPort();
private:
/// UDT socket
//	UDTSOCKET	m_sock;	
	int	m_sock;	
/// Socket type
	int m_SocketType;
};
