/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CExternalIPRequest.h
///
///  cass for requesting external peer IP
///
///  @author "Archer Software" Dmitry Netrebenko @date 20.11.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once

#include <NWL/NetworkLayer.h>
#include "CSSocket.h"
#include <NWL/Events/Events.h>
#include "relay_messages.h"

/// cass for requesting external peer IP
class NWL_API CExternalIPRequest
{
private:
	CExternalIPRequest( const CExternalIPRequest& );
	CExternalIPRequest& operator=( const CExternalIPRequest& );
	HANDLE m_hConnectEvent;
	EConnectErrorReason m_lastConnectError;
	CRITICAL_SECTION m_cs;

public:
	/// Initializes object instance
	/// @param ServerAddr relay server address
	/// @raram ServerPort relay server port
	CExternalIPRequest( const tstring& ServerAddr, const unsigned int& ServerPort );

	/// destroys object instance
	~CExternalIPRequest();

	/// Returns external ip address
	/// @param userId - user name for authentiocation on server
	/// @param passwd - password for authentication on server
	/// @return external ip address
	SPeerAddr GetExternalAddress( const tstring& userId, const tstring& passwd );

	/// Returns external ip address through proxy
	/// @param userId - user name for authentiocation on server
	/// @param passwd - password for authentication on server
	/// @param timeOut connect timeout
	/// @return external ip address
	SPeerAddr GetExternalAddressThroughProxy(const tstring& userId, const tstring& passwd, const int timeOut);

private:
	CSSocket m_sock;
	tstring m_strServerAddr;
	unsigned int m_nServerPort;

	void OnConnected( void* param );
	void OnConnectError( void* param, EConnectErrorReason reason );
};
