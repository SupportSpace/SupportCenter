/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSUDTSocket.cpp
///
///  Implements CSUDTSocket class, UDT socket wrapper
///
///  @author Dmitry Netrebenko @date 23.11.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CSUDTSocket.h>
#include <udt.h>

CSUDTSocket::CSUDTSocket( int sock_type )
:m_sock( UDT::INVALID_SOCK ), m_SocketType( sock_type )
{
}

CSUDTSocket::~CSUDTSocket()
{
	Close();
}

bool CSUDTSocket::Create(bool rendezvous, SOCKET sock)
{
	const int one = 1;

	// Check socket
	if ( m_sock != UDT::INVALID_SOCK )
		Close();

	// Create the socket
	m_sock = UDT::socket( AF_INET, m_SocketType, 0, sock );
	if ( m_sock == UDT::INVALID_SOCK )
		return false;

	// Set rendezvous connection
	if ( UDT::setsockopt( m_sock, 0, UDT_RENDEZVOUS, &rendezvous, sizeof(rendezvous) ) )
		return false;

	return true;
}

bool CSUDTSocket::Shutdown()
{
	if ( m_sock != UDT::INVALID_SOCK )
	  UDT::shutdown( m_sock, 2 );
	return true;
}

bool CSUDTSocket::Close()
{
	if ( m_sock != UDT::INVALID_SOCK )
    {
		UDT::shutdown( m_sock, 2 );
		UDT::close( m_sock );
		m_sock = UDT::INVALID_SOCK;
    }
	return true;
}

bool CSUDTSocket::Bind( const unsigned int& port )
{
	struct sockaddr_in addr;

	// Check that the socket is open
	if ( m_sock == UDT::INVALID_SOCK )
		return false;

	// Set up the address to bind the socket to
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Do the binding
	if ( UDT::ERROR == UDT::bind( m_sock, (struct sockaddr *)&addr, sizeof(addr) ) )
		return false;

	return true;
}

bool CSUDTSocket::Connect( const tstring& address, const unsigned int& port )
{
	// Check the socket
	if ( m_sock == UDT::INVALID_SOCK )
		return false;

	// Create an address structure and clear it
	struct sockaddr_in addr;
	memset( &addr, 0, sizeof(addr) );

	// Fill in the address if possible
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr( address.c_str() );

	// Check IP address validation
	if ( -1 == addr.sin_addr.s_addr )
    {
		// No, so get the actual IP address of the host name specified
		struct hostent *pHost;
		pHost = gethostbyname( address.c_str() );
		if ( pHost )
		{
			if ( !pHost->h_addr )
				return false;
			addr.sin_addr.s_addr = ((struct in_addr *)pHost->h_addr)->s_addr;
		}
		else
			return false;
    }

	// Set the port number in the correct format
	addr.sin_port = htons(port);

	// Connect the socket
	if ( UDT::ERROR == UDT::connect( m_sock, (struct sockaddr *)&addr, sizeof(addr) )  )
		return false;

	return true;
}

bool CSUDTSocket::Listen()
{
	// Check socket
	if ( m_sock == UDT::INVALID_SOCK )
		return false;

	// Set it to listen
	if ( UDT::ERROR == UDT::listen( m_sock, 5 ) )
		return false;

	return true;
}

CSUDTSocket* CSUDTSocket::Accept()
{
	const int one = 1;

	UDTSOCKET new_socket_id;
	CSUDTSocket* new_socket;

	// Check this socket
	if ( m_sock == UDT::INVALID_SOCK )
		return NULL;

	// Accept an incoming connection
	if ( ( new_socket_id = UDT::accept( m_sock, NULL, 0 ) ) == UDT::INVALID_SOCK )
		return NULL;

	// Create a new CSUDTSocket
	new_socket = new CSUDTSocket( m_SocketType );
	if ( new_socket )
		new_socket->m_sock = new_socket_id;
	else
    {
		UDT::shutdown( new_socket_id, 2 );
		UDT::close( new_socket_id );
    }

	return new_socket;
}

bool CSUDTSocket::SetTimeout( unsigned int msecs, ESSocketTimeout time )
{
	int timeout = msecs;
	
	if ( (sstReceive & time) == sstReceive )
	{
		if ( UDT::setsockopt( m_sock, SOL_SOCKET, UDT_RCVTIMEO, (char*)&timeout, sizeof(timeout) ) == UDT::ERROR )
			return false;
	}
	if ( (sstSend & time) == sstSend )
	{
		if ( UDT::setsockopt( m_sock, SOL_SOCKET, UDT_SNDTIMEO, (char*)&timeout, sizeof(timeout)) == UDT::ERROR )
			return false;
	}
	return true;
}

bool CSUDTSocket::ReadSelect( const unsigned int msec )
{
	UDT::UDSET fds;
 	UD_ZERO( &fds );
 	UD_SET( (unsigned long)m_sock, &fds );
	struct timeval tv;
 	tv.tv_sec = msec/1000;
 	tv.tv_usec = (msec % 1000) * 1000;
 	int rc = UDT::select( (int)m_sock + 1, &fds, 0, 0, &tv );
	return ( rc > 0 );
}

bool CSUDTSocket::WriteSelect( const unsigned int msec )
{
 	UDT::UDSET fds;
 	UD_ZERO( &fds );
 	UD_SET( (unsigned long)m_sock, &fds );
	struct timeval tv;
 	tv.tv_sec = msec/1000;
 	tv.tv_usec = (msec % 1000) * 1000;
 	int rc = UDT::select( (int)m_sock + 1, 0, &fds, 0, &tv );
	return ( rc > 0 );
}

unsigned int CSUDTSocket::Send( const char* buf, const unsigned int& len )
{
	return UDT::send( m_sock, buf, len, 0 );
}

unsigned int CSUDTSocket::Receive( char* buf, const unsigned int& len )
{
	return UDT::recv( m_sock, buf, len, 0 );
}

unsigned int CSUDTSocket::SendMsg( const char* buf, const unsigned int& len )
{
	return UDT::sendmsg( m_sock, buf, len, -1/*, bool inorder = false*/ );
}

unsigned int CSUDTSocket::ReceiveMsg( char* buf, const unsigned int& len )
{
	return UDT::recvmsg( m_sock, buf, len );
}

unsigned int CSUDTSocket::Peek( char* buf, const unsigned int& len )
{
	return UDT::recv( m_sock, buf, len, MSG_PEEK );
}

bool CSUDTSocket::SendExact( const char* buf, const unsigned int& len )
{
	return ( Send( buf, len ) == len );
}

bool CSUDTSocket::ReceiveExact( char* buf, const unsigned int& len )
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

int CSUDTSocket::GetReadyDataCount()
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

tstring CSUDTSocket::GetPeerName()
{
	if ( m_sock == UDT::INVALID_SOCK )
		return _T("<unavailable>");

	struct sockaddr_in sockinfo;
	struct in_addr address;
	int sockinfosize = sizeof(sockinfo);
	tstring name;

	UDT::getpeername( m_sock, (struct sockaddr*)&sockinfo, &sockinfosize );
	memcpy( &address, &sockinfo.sin_addr, sizeof(address) );
	char* nm = inet_ntoa(address);
	if ( !nm )
		name = "<unavailable>";
	else
		name = nm;
	return name;
}

tstring CSUDTSocket::GetSockName()
{
	if ( m_sock == UDT::INVALID_SOCK )
		return _T("<unavailable>");

	struct sockaddr_in sockinfo;
	struct in_addr address;
	int sockinfosize = sizeof(sockinfo);
	tstring name;

	UDT::getsockname( m_sock, (struct sockaddr*)&sockinfo, &sockinfosize );
	memcpy( &address, &sockinfo.sin_addr, sizeof(address) );
	char* nm = inet_ntoa(address);
	if ( !nm )
		name = "<unavailable>";
	else
		name = nm;
	return name;
}

unsigned int CSUDTSocket::GetPeerPort()
{
	if ( m_sock == UDT::INVALID_SOCK )
		return 0;

	struct sockaddr_in sockinfo;
	int sockinfosize = sizeof(sockinfo);

	UDT::getpeername( m_sock, (struct sockaddr*)&sockinfo, &sockinfosize );
	return htons( sockinfo.sin_port );
}

unsigned int CSUDTSocket::GetSockPort()
{
	if ( m_sock == UDT::INVALID_SOCK )
		return 0;

	struct sockaddr_in sockinfo;
	int sockinfosize = sizeof(sockinfo);

	UDT::getsockname( m_sock, (struct sockaddr*)&sockinfo, &sockinfosize );
	return htons( sockinfo.sin_port );
}