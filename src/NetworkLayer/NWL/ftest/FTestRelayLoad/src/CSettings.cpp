/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSettings.cpp
///
///  Implements CSettings class, responsible for storing settings
///
///  @author Dmitry Netrebenko @date 21.02.2008
///
////////////////////////////////////////////////////////////////////////

#include "CSettings.h"
#include <AidLib/CException/CException.h>

CSettings::CSettings(void)
	:	m_host(DEF_RELAY_HOST)
	,	m_port(DEF_RELAY_PORT)
	,	m_user(DEF_USER_NAME)
	,	m_passwd(DEF_RELAY_PASSWD)
	,	m_poolSize(DEF_RELAY_POOL_SIZE)
	,	m_peersCount(DEF_RELAY_PEERS_COUNT)
	,	m_extPort(DEF_EXT_PORT)
	,	m_intPort(DEF_INT_PORT)
	,	m_blockSize(DEF_BLOCK_SIZE)
	,	m_blocksCount(DEF_BLOCKS_COUNT)
{
TRY_CATCH
CATCH_THROW()
}

CSettings::~CSettings(void)
{
TRY_CATCH
CATCH_LOG()
}

void CSettings::SetRelayHost(const tstring& host)
{
TRY_CATCH
	if(_T("") == host)
		throw MCException(_T("Empty host."));
	m_host = host;
CATCH_THROW()
}

tstring CSettings::GetRelayHost() const
{
TRY_CATCH
	return m_host;
CATCH_THROW()
}

void CSettings::SetRelayPort(const unsigned int port)
{
TRY_CATCH
	if(port < 1099)
		throw MCException(_T("Invalid relay port."));
	m_port = port;
CATCH_THROW()
}

unsigned int CSettings::GetRelayPort() const
{
TRY_CATCH
	return m_port;
CATCH_THROW()
}

void CSettings::SetUser(const tstring& user)
{
TRY_CATCH
	if(_T("") == user)
		throw MCException(_T("Empty user name."));
	m_user = user;
CATCH_THROW()
}

tstring CSettings::GetUser() const
{
TRY_CATCH
	return m_user;
CATCH_THROW()
}

void CSettings::SetPassword(const tstring& passwd)
{
TRY_CATCH
	if(_T("") == passwd)
		throw MCException(_T("Empty password."));
	m_passwd = passwd;
CATCH_THROW()
}

tstring CSettings::GetPassword() const
{
TRY_CATCH
	return m_passwd;
CATCH_THROW()
}

void CSettings::SetPoolSize(const int poolSize)
{
TRY_CATCH
	if(poolSize <= 0)
		throw MCException(_T("Invalid pool size."));
	m_poolSize = poolSize;
CATCH_THROW()
}

int CSettings::GetPoolSize() const
{
TRY_CATCH
	return m_poolSize;
CATCH_THROW()
}

void CSettings::SetPeersCount(const int peersCount)
{
TRY_CATCH
	if(peersCount <= 0)
		throw MCException(_T("Invalid number of connection."));
	m_peersCount = peersCount;
CATCH_THROW()
}

int CSettings::GetPeersCount() const
{
TRY_CATCH
	return m_peersCount;
CATCH_THROW()
}

void CSettings::SetExtPort(const unsigned int extPort)
{
TRY_CATCH
	if(extPort < 1099)
		throw MCException(_T("Invalid external port."));
	m_extPort = extPort;
CATCH_THROW()
}

unsigned int CSettings::GetExtPort() const
{
TRY_CATCH
	return m_extPort;
CATCH_THROW()
}

void CSettings::SetIntPort(const unsigned int intPort)
{
TRY_CATCH
	if(intPort < 1099)
		throw MCException(_T("Invalid internal port."));
	m_intPort = intPort;
CATCH_THROW()
}

unsigned int CSettings::GetIntPort() const
{
TRY_CATCH
	return m_intPort;
CATCH_THROW()
}

std::list<tstring> CSettings::GetDescription() const
{
TRY_CATCH
	std::list<tstring> strList;
	strList.push_back(Format(_T("Server: %s:%d"), m_host.c_str(), m_port));
	strList.push_back(Format(_T("Pool size: %d"), m_poolSize));
	strList.push_back(Format(_T("Number of clients: %d"), m_peersCount));
	return strList;
CATCH_THROW()
}

void CSettings::SetBlockSize(const unsigned int blockSize)
{
TRY_CATCH
	if(blockSize < sizeof(DWORD))
		throw MCException(_T("Invalid block size."));
	m_blockSize = blockSize;
CATCH_THROW()
}

unsigned int CSettings::GetBlockSize() const
{
TRY_CATCH
	return m_blockSize;
CATCH_THROW()
}

void CSettings::SetBlocksCount(const unsigned int blocksCount)
{
TRY_CATCH
	if(blocksCount < 1)
		throw MCException(_T("Invalid blocks count."));
	m_blocksCount = blocksCount;
CATCH_THROW()
}

unsigned int CSettings::GetBlocksCount() const
{
TRY_CATCH
	return m_blocksCount;
CATCH_THROW()
}

