/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSSocket.h
///
///  Declares CSSocket class, Windows socket wrapper
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/NetworkLayer.h>
#include "ESSocketType.h"
#include "ESSocketTimeout.h"
#include <AidLib/Strings/tstring.h>

///  Windows socket wrapper
///  @remarks
class NWL_API CSSocket
{
private:
/// Prevents making copies of CSSocket objects.
	CSSocket( const CSSocket& );
	CSSocket& operator=( const CSSocket& );

public:
	CSSocket( ESSocketType = stTCP );
	~CSSocket();

///  Creates windows socket and attach it to this object
///  @return true, if operation completed successfully
///  @remarks
	bool Create();

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
	CSSocket* Accept();

///  Sets timeout value
///  @param   timeout in milliseconds
///  @param	  timeout type (default = receive && send)
///  @return true, if operation completed successfully
///  @remarks
	inline bool SetTimeout( unsigned int, ESSocketTimeout time = sstAll );

///  Check for incoming data
///  @param   time in milliseconds
///  @return true, if operation completed successfully
///  @remarks
	inline bool ReadSelect( const unsigned int );

///  Sends data
///  @param   buffer to send
///  @param   size of buffer
///  @return number of bytes sent
///  @remarks
///  @param   time in milliseconds
///  @return true, if operation completed successfully
///  @remarks
	inline bool WriteSelect( const unsigned int );
	
///  Sends data
///  @param   buffer to send
///  @param   size of buffer
///  @return number of bytes sent
///  @remarks
	inline unsigned int Send( const char*, const unsigned int& );
	
///  Sends data
///  @param   remote host
///  @param   remote port
///  @param   buffer to send
///  @param   size of buffer
///  @return number of bytes sent
///  @remarks
	unsigned int SendTo( const tstring&, const unsigned int&, const char*, const unsigned int& );

///  Sends data
///  @param   remote port
///  @param   buffer to send
///  @param   size of buffer
///  @return number of bytes sent
	unsigned int SendBCast( const unsigned int&, const char*, const unsigned int& );

///  Switches broadcast socket option (only for udp sockets)
/// @param bCast new option value
	void SwitchBroadCastOption( const bool bCast );

//  Receives data
//  @param   remote host
//  @param   remote port
//  @param   buffer
//  @param   size of buffer
//  @return number of bytes received
//  @remarks
	int ReceiveFrom( tstring&, unsigned int&, char*, const unsigned int& );

///  Receives data
///  @param   buffer
///  @param   size of buffer
///  @return number of bytes received
	inline unsigned int Receive( char*, const unsigned int& );

///  Peeks at the incoming data
///  @param   buffer
///  @param   size of buffer
///  @return number of bytes received
	inline unsigned int Peek( char*, const unsigned int& );

///  Sends exactly number of bytes
///  @param   buffer to send
///  @param   size of buffer
///  @return true, if operation completed successfully
///  @remarks
	inline bool SendExact( const char*, const unsigned int& );

///  Receives exactly number of bytes
///  @param   buffer
///  @param   size of buffer
///  @return true, if operation completed successfully
///  @remarks
	inline bool ReceiveExact( char*, const unsigned int& );

/// Returns size of data buffer
/// @return size of data buffer
	inline int GetReadyDataCount();

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

///  SOCKET operator
///  @remarks
	inline operator SOCKET() const;

private:
/// Windows socket
	SOCKET	m_sock;	
/// Socket type
	ESSocketType m_SocketType;
};

inline CSSocket::operator SOCKET() const
{
	return m_sock;
}

inline bool CSSocket::SetTimeout( unsigned int msecs, ESSocketTimeout time )
{
	int timeout = msecs;
	
	if ( (sstReceive & time) == sstReceive )
	{
		if ( setsockopt( m_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout) ) == SOCKET_ERROR )
			return false;
	}
	if ( (sstSend & time) == sstSend )
	{
		if ( setsockopt( m_sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR )
			return false;
	}
	return true;
}

inline bool CSSocket::ReadSelect( const unsigned int msec )
{
 	fd_set fds;
 	FD_ZERO( &fds );
 	FD_SET( (unsigned long)m_sock, &fds );
	struct timeval tv;
 	tv.tv_sec = msec/1000;
 	tv.tv_usec = (msec % 1000) * 1000;
 	int rc = select( (int)m_sock + 1, &fds, 0, 0, &tv );
	return ( rc > 0 );
}

inline bool CSSocket::WriteSelect( const unsigned int msec )
{
 	fd_set fds;
 	FD_ZERO( &fds );
 	FD_SET( (unsigned long)m_sock, &fds );
	struct timeval tv;
 	tv.tv_sec = msec/1000;
 	tv.tv_usec = (msec % 1000) * 1000;
 	int rc = select( (int)m_sock + 1, 0, &fds, 0, &tv );
	return ( rc > 0 );
}

inline unsigned int CSSocket::Send( const char* buf, const unsigned int& len )
{
	return send( m_sock, buf, len, 0 );
}

inline unsigned int CSSocket::Receive( char* buf, const unsigned int& len )
{
	return recv( m_sock, buf, len, 0 );
}

inline int CSSocket::ReceiveFrom( tstring& remote_addr, 
	unsigned int& remote_port, char* buf, const unsigned int& len )
{
	int ret = 0;
	char addr_buf[4096];
	int size = 4096;
	struct in_addr address;

	ret = recvfrom( m_sock, buf, len, 0, (struct sockaddr*)addr_buf, &size );
	if (ret == SOCKET_ERROR)
		return SOCKET_ERROR;

	struct sockaddr_in *sockinfo = (struct sockaddr_in *)addr_buf;

	if( ret )
	{
		memcpy( &address, &sockinfo->sin_addr, sizeof(address) );
		char* nm = inet_ntoa(address);
		if ( !nm )
			remote_addr = "<unavailable>";
		else
			remote_addr = nm;
		remote_port = htons( sockinfo->sin_port );
	}
	else
	{
		remote_addr = "<unavailable>";
		remote_port = 0;
	}

	return ret;
}

inline unsigned int CSSocket::Peek( char* buf, const unsigned int& len )
{
	return recv( m_sock, buf, len, MSG_PEEK );
}

inline bool CSSocket::SendExact( const char* buf, const unsigned int& len )
{
	return ( Send( buf, len ) == len );
}

inline bool CSSocket::ReceiveExact( char* buf, const unsigned int& len )
{
	int num;
	unsigned int currlen = len;
    
	{
		while ( currlen > 0 )
		{
			num = Receive( buf, currlen );
			
			if ( num > 0 )
			{
				// Adjust the buffer position and size
				buf += num;
				currlen -= num;
			} 
			else 
			{
				if ( !num ) 
					return false;
				else 
				{
					if ( WSAEWOULDBLOCK != WSAGetLastError() )
						return false;
				}
			}
		}
	}

	return true;
}

inline int CSSocket::GetReadyDataCount()
{
	unsigned long ret;
	if (ioctlsocket(m_sock,FIONREAD,&ret)) 
		return SOCKET_ERROR;
	/*if (!ret)
	{
		char k;
		if (SOCKET_ERROR == recv(m_sock,&k,1,MSG_PEEK))
		{
			if (WSAETIMEDOUT != WSAGetLastError())
				return SOCKET_ERROR;
		}
	}*/
	return ret;
}

inline unsigned int CSSocket::SendTo( const tstring& address, const unsigned int& port, const char* buf, const unsigned int& len )
{
	struct sockaddr_in addr;
	memset( &addr, 0, sizeof(addr) );

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr( address.c_str() );
	addr.sin_port = htons(port);

	return sendto( m_sock, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr) );
}

inline unsigned int CSSocket::SendBCast( const unsigned int& port, const char* buf, const unsigned int& len )
{
	struct sockaddr_in addr;
	memset( &addr, 0, sizeof(addr) );

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_BROADCAST;
	addr.sin_port = htons(port);

	return sendto( m_sock, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr) );
}
