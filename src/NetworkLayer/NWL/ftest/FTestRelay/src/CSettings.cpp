/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSettings.cpp
///
///  Implements CSettings class, responsible for storing settings
///
///  @author Dmitry Netrebenko @date 03.10.2007
///
////////////////////////////////////////////////////////////////////////

#include "CSettings.h"
#include <AidLib/CException/CException.h>

CSettings::CSettings()
	:	m_host(DEF_RELAY_HOST)
	,	m_port(DEF_RELAY_PORT)
	,	m_user(DEF_USER_NAME)
	,	m_passwd(DEF_RELAY_PASSWD)
	,	m_type(DEF_TEST_TYPE)
	,	m_poolSize(DEF_POOL_SIZE)
	,	m_peersCount(DEF_PEERS_COUNT)
	,	m_extPort(DEF_EXT_PORT)
{
TRY_CATCH
CATCH_THROW()
}

CSettings::~CSettings()
{
TRY_CATCH
CATCH_LOG()
}
