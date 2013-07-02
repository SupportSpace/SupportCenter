/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSSocket.cpp
///
///  Implements CSSocket class, socket wrapper
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CSSocket.h>

CSSocket::CSSocket( ESSocketType sock_type )
:m_sock(SOCKET_ERROR), m_SocketType( sock_type )
{
}

CSSocket::~CSSocket()
{
	Close();
}

bool CSSocket::Create()
{
	const int one = 1;

	// Check socket
	if ( m_sock != INVALID_SOCKET )
		Close();

	// Create the socket
	m_sock = socket( AF_INET, m_SocketType, 0 );
	if ( m_sock == INVALID_SOCKET )
		return false;

	if ( stTCP == m_SocketType )
	{
		// Set socket options:
		if ( setsockopt( m_sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&one, sizeof(one) ) )
			return false;

		if ( setsockopt( m_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(one) ) )
			return false;
	}

	return true;
}

bool CSSocket::Shutdown()
{
	if ( m_sock != INVALID_SOCKET )
	  shutdown( m_sock, SD_BOTH );
	return true;
}

bool CSSocket::Close()
{
	if ( m_sock != INVALID_SOCKET )
    {
		shutdown( m_sock, SD_BOTH );
		closesocket( m_sock );
		m_sock = INVALID_SOCKET;
    }
	return true;
}

bool CSSocket::Bind( const unsigned int& port )
{
	struct sockaddr_in addr;

	// Check that the socket is open
	if ( m_sock == INVALID_SOCKET )
		return false;

	// Set up the address to bind the socket to
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Do the binding
	if ( bind( m_sock, (struct sockaddr *)&addr, sizeof(addr) ) < 0 )
		return false;

	return true;
}

bool CSSocket::Connect( const tstring& address, const unsigned int& port )
{
	// Check the socket
	if ( m_sock == INVALID_SOCKET )
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
	if ( connect( m_sock, (struct sockaddr *)&addr, sizeof(addr) ) )
		return false;

	return true;
}

bool CSSocket::Listen()
{
	// Check socket
	if ( m_sock == INVALID_SOCKET )
		return false;

	// Set it to listen
	if ( listen( m_sock, 5 ) == INVALID_SOCKET )
		return false;

	return true;
}

CSSocket* CSSocket::Accept()
{
	const int one = 1;

	SOCKET new_socket_id;
	CSSocket* new_socket;

	// Check this socket
	if ( m_sock == INVALID_SOCKET )
		return NULL;

	// Accept an incoming connection
	if ( ( new_socket_id = accept( m_sock, NULL, 0 ) ) == INVALID_SOCKET )
		return NULL;

	// Create a new CSSocket
	new_socket = new CSSocket( m_SocketType );
	if ( new_socket )
		new_socket->m_sock = new_socket_id;
	else
    {
		shutdown( new_socket_id, SD_BOTH );
		closesocket( new_socket_id );
    }

	// Attempt to set the new socket's options
	if ( stTCP == m_SocketType )
		setsockopt( new_socket->m_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(one) );

	return new_socket;
}
void CSSocket::SwitchBroadCastOption( const bool bCast )
{
	BOOL bOptVal(bCast);
	setsockopt( m_sock, SOL_SOCKET, SO_BROADCAST, (char*)&bOptVal, sizeof(bOptVal));
}

tstring CSSocket::GetPeerName()
{
	if ( m_sock == INVALID_SOCKET )
		return _T("<unavailable>");

	struct sockaddr_in sockinfo;
	struct in_addr address;
	int sockinfosize = sizeof(sockinfo);
	tstring name;

	getpeername( m_sock, (struct sockaddr*)&sockinfo, &sockinfosize );
	memcpy( &address, &sockinfo.sin_addr, sizeof(address) );
	char* nm = inet_ntoa(address);
	if ( !nm )
		name = "<unavailable>";
	else
		name = nm;
	return name;
}

tstring CSSocket::GetSockName()
{
	if ( m_sock == INVALID_SOCKET )
		return _T("<unavailable>");

	struct sockaddr_in sockinfo;
	struct in_addr address;
	int sockinfosize = sizeof(sockinfo);
	tstring name;

	getsockname( m_sock, (struct sockaddr*)&sockinfo, &sockinfosize );
	memcpy( &address, &sockinfo.sin_addr, sizeof(address) );
	char* nm = inet_ntoa(address);
	if ( !nm )
		name = "<unavailable>";
	else
		name = nm;
	return name;
}

unsigned int CSSocket::GetPeerPort()
{
	if ( m_sock == INVALID_SOCKET )
		return 0;

	struct sockaddr_in sockinfo;
	int sockinfosize = sizeof(sockinfo);

	getpeername( m_sock, (struct sockaddr*)&sockinfo, &sockinfosize );
	return htons( sockinfo.sin_port );
}

unsigned int CSSocket::GetSockPort()
{
	if ( m_sock == INVALID_SOCKET )
		return 0;

	struct sockaddr_in sockinfo;
	int sockinfosize = sizeof(sockinfo);

	getsockname( m_sock, (struct sockaddr*)&sockinfo, &sockinfosize );
	return htons( sockinfo.sin_port );
}
