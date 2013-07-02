/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SConnectParams.h
///
///  Declares SConnectParams structure, responsible for set of connection
///    parameters
///
///  @author Dmitry Netrebenko @date 09.08.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>

#define DEFAULT_USER_NAME		_T("utest1")
#define DEFAULT_USER_PASSWD		_T("utest1")
#define DEFAULT_REMOTE_USER		_T("utest2")
#define DEFAULT_SERVER_ADDR		_T("192.168.0.66")
#define DEFAULT_SERVER_PORT		5222
#define DEFAULT_SERVER_NAME		_T("edem")
#define DEFAULT_RESOURCE_NAME	_T("jabber_test")
#define DEFAULT_CONNECT_TIME	30000

///  SConnectParams structure, responsible for set of connection
///    parameters
struct SConnectParams
{
	tstring			m_userName;			/// jabber user name
	tstring			m_userPasswd;		/// jabber password
	tstring			m_remoteUserName;	/// remote user name
	tstring			m_serverAddr;		/// jabber server address
	unsigned int	m_serverPort;		/// jabber server port
	tstring			m_serverName;		/// jabber server name
	tstring			m_resource;			/// jabber resource name
	unsigned int	m_connectTimeout;	/// jabber connection timeout

	/// Default constructor
	SConnectParams()
		:	m_userName(DEFAULT_USER_NAME)
		,	m_userPasswd(DEFAULT_USER_PASSWD)
		,	m_remoteUserName(DEFAULT_REMOTE_USER)
		,	m_serverAddr(DEFAULT_SERVER_ADDR)
		,	m_serverPort(DEFAULT_SERVER_PORT)
		,	m_serverName(DEFAULT_SERVER_NAME)
		,	m_resource(DEFAULT_RESOURCE_NAME)
		,	m_connectTimeout(DEFAULT_CONNECT_TIME)
	{};
};
