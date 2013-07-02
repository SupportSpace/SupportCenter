/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSocketSystem.cpp
///
///  Implements CSocketSystem class, responsible for initializing sockets
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CSocketSystem.h>
#include <winsock2.h>

CSocketSystem::CSocketSystem()
: m_bInited(false)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	
	wVersionRequested = MAKEWORD(2, 0);
	if ( WSAStartup( wVersionRequested, &wsaData ) )
		m_bInited = false;
	else
		m_bInited = true;
}

CSocketSystem::~CSocketSystem()
{
	if ( m_bInited )
		WSACleanup();
}

bool CSocketSystem::Initialized()
{
	return m_bInited;
}
