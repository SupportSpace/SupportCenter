/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSettings.h
///
///  Declares CSettings class, responsible for storing settings
///
///  @author Dmitry Netrebenko @date 03.10.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>


#define DEF_RELAY_HOST _T("192.168.0.66")
#define DEF_RELAY_PORT 5904
#define DEF_RELAY_PASSWD _T("A7B3F3CA-0096-4d02-8936-31B2392F973F")
#define DEF_TEST_TYPE ttRelayedStreamTest
#define DEF_POOL_SIZE 500
#define DEF_PEERS_COUNT 100000
#define DEF_EXT_PORT 7799
#define DEF_USER_NAME _T("TestUser")

/// Test type enumeration
enum ETestType
{
	ttRelayedStreamTest		= 0,
	ttNATStreamTest			= 1,
	ttConnectTest			= 2
};

///  CSettings class, responsible for storing settings
class CSettings
{
private:
/// Prevents making copies of CSettings objects
	CSettings(const CSettings&);
	CSettings& operator=(const CSettings&);

public:
/// Constructor
	CSettings();
/// Destructor
	~CSettings();

public:
/// Relay's address
	tstring			m_host;
/// Relay's port
	unsigned int	m_port;
/// Relay's user
	tstring			m_user;
/// Relay's password
	tstring			m_passwd;
/// Test type
	ETestType		m_type;
/// Size of thread pool
	int				m_poolSize;
/// Count of peers to connect
	int				m_peersCount;
/// Checking port
	unsigned int	m_extPort;
};
