/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSettings.h
///
///  Declares CSettings class, responsible for storing settings
///
///  @author Dmitry Netrebenko @date 21.02.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>
#include <list>

#define DEF_RELAY_HOST _T("192.168.0.66")
#define DEF_RELAY_PORT 5904
#define DEF_NAT_PORT 5902
#define DEF_RELAY_PASSWD _T("A7B3F3CA-0096-4d02-8936-31B2392F973F")
#define DEF_RELAY_POOL_SIZE 200
#define DEF_RELAY_PEERS_COUNT 100000
#define DEF_NAT_POOL_SIZE 50
#define DEF_NAT_PEERS_COUNT 10000
#define DEF_EXTIP_POOL_SIZE 100
#define DEF_EXTIP_PEERS_COUNT 10000
#define DEF_OPENPORT_POOL_SIZE 100
#define DEF_OPENPORT_PEERS_COUNT 10000
#define DEF_EXT_PORT 7799
#define DEF_INT_PORT 7799
#define DEF_USER_NAME _T("TestUser")
#define DEF_BLOCK_SIZE 1024
#define DEF_BLOCKS_COUNT 10


/// CSettings class, responsible for storing settings
class CSettings
{
public:
/// Constructor
	CSettings();
/// Destructor
	~CSettings();
private:
/// Relay's address
	tstring			m_host;
/// Relay's port
	unsigned int	m_port;
/// Relay's user
	tstring			m_user;
/// Relay's password
	tstring			m_passwd;
/// Size of thread pool
	int				m_poolSize;
/// Count of peers to connect
	int				m_peersCount;
/// Checking external port
	unsigned int	m_extPort;
/// Checking internal port
	unsigned int	m_intPort;
/// Size of data block to send/receive
	unsigned int	m_blockSize;
/// Count of blocks to send/receive
	unsigned int	m_blocksCount;
public:
/// Setters and getters for fields
	void SetRelayHost(const tstring& host);
	tstring GetRelayHost() const;
	void SetRelayPort(const unsigned int port);
	unsigned int GetRelayPort() const;
	void SetUser(const tstring& user);
	tstring GetUser() const;
	void SetPassword(const tstring& passwd);
	tstring GetPassword() const;
	void SetPoolSize(const int poolSize);
	int GetPoolSize() const;
	void SetPeersCount(const int peersCount);
	int GetPeersCount() const;
	void SetExtPort(const unsigned int extPort);
	unsigned int GetExtPort() const;
	void SetIntPort(const unsigned int intPort);
	unsigned int GetIntPort() const;
	void SetBlockSize(const unsigned int blockSize);
	unsigned int GetBlockSize() const;
	void SetBlocksCount(const unsigned int blocksCount);
	unsigned int GetBlocksCount() const;
/// Returns list of strings with settings descriptions
	std::list<tstring> GetDescription() const;
};
