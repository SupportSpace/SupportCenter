/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRelayConnectSettings.cpp
///
///  Implements CRelayConnectSettings class, responsible for management of
///    relay server's settings
///
///  @author Dmitry Netrebenko @date 27.11.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CRelayConnectSettings.h>
#include <AidLib/CException/CException.h>
#include <AidLib/Logging/cLog.h>

CRelayConnectSettings::CRelayConnectSettings()
: m_strServerAddr( _T("") ), m_nServerPort( 0 ),
  m_strSrvUserId( _T("") ), m_strSrvPassword( _T("") ),
  m_strConnectId( _T("") ), m_strLocalPeerId( _T("") ), m_strRemotePeerId( _T("") ),
  m_bIsMaster( false )
{
TRY_CATCH

CATCH_THROW("CRelayConnectSettings::CRelayConnectSettings")
}

CRelayConnectSettings::~CRelayConnectSettings()
{
TRY_CATCH

CATCH_LOG("CRelayConnectSettings::~CRelayConnectSettings")
}

tstring CRelayConnectSettings::GetServerAddress() const
{
TRY_CATCH

	return m_strServerAddr;

CATCH_THROW("CRelayConnectSettings::GetServerAddress")
}

unsigned int CRelayConnectSettings::GetServerPort() const
{
TRY_CATCH

	return m_nServerPort;

CATCH_THROW("CRelayConnectSettings::GetServerPort")
}

tstring CRelayConnectSettings::GetSrvUserId() const
{
TRY_CATCH

	return m_strSrvUserId;

CATCH_THROW("CRelayConnectSettings::GetSrvUserId")
}

tstring CRelayConnectSettings::GetSrvPassword() const
{
TRY_CATCH

	return m_strSrvPassword;

CATCH_THROW("CRelayConnectSettings::GetSrvPassword")
}

tstring CRelayConnectSettings::GetConnectId() const
{
TRY_CATCH

	return m_strConnectId;

CATCH_THROW("CRelayConnectSettings::GetConnectId")
}

tstring CRelayConnectSettings::GetLocalPeerId() const
{
TRY_CATCH

	return m_strLocalPeerId;

CATCH_THROW("CRelayConnectSettings::GetLocalPeerId")
}

tstring CRelayConnectSettings::GetRemotePeerId() const
{
TRY_CATCH

	return m_strRemotePeerId;

CATCH_THROW("CRelayConnectSettings::GetRemotePeerId")
}

void CRelayConnectSettings::SetRelayServer( 
	const tstring& ServerAddr, const unsigned int& ServerPort, 
	const tstring& AuthUserId, const tstring& AuthPassword )
{
TRY_CATCH

	m_strServerAddr = ServerAddr;
	m_nServerPort = ServerPort;
	m_strSrvUserId = AuthUserId;
	m_strSrvPassword = AuthPassword;

CATCH_THROW("CRelayConnectSettings::SetRelayServer")
}

void CRelayConnectSettings::SetConnectionId( 
	const tstring& ConnectionId, const tstring& LocalPeerId, const tstring& RemotePeerId )
{
TRY_CATCH

	m_strConnectId = ConnectionId;
	m_strLocalPeerId = LocalPeerId;
	m_strRemotePeerId = RemotePeerId;

CATCH_THROW("CRelayConnectSettings::SetConnectionId")
}

bool CRelayConnectSettings::GetIsMaster() const
{
TRY_CATCH

	return m_bIsMaster;

CATCH_THROW("CRelayConnectSettings::GetIsMaster")
}

void CRelayConnectSettings::SetIsMaster( const bool master )
{
TRY_CATCH

	m_bIsMaster = master;

CATCH_THROW("CRelayConnectSettings::SetIsMaster")
}

