/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRelayConnectSettings.h
///
///  Declares CRelayConnectSettings class, responsible for management of
///    relay server's settings
///
///  @author Dmitry Netrebenko @date 27.11.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>
#include <NWL/NetworkLayer.h>

///  CRelayConnectSettings class, responsible for management of
///    relay server's settings
///  @remarks
class NWL_API CRelayConnectSettings
{
public:
///  Constructor
	CRelayConnectSettings();

///  Destructor
	~CRelayConnectSettings();

protected:
///  Relay server's address
	tstring			m_strServerAddr;
///  Relay server's port
	unsigned int	m_nServerPort;
///  User Id for authentication on relay server
	tstring			m_strSrvUserId;
///  Password for authentication on relay server
	tstring			m_strSrvPassword;
///  Connection Id
	tstring			m_strConnectId;
///  Local PeerId
	tstring			m_strLocalPeerId;
///  Remote PeerId
	tstring			m_strRemotePeerId;
///  Master/Slave peer flag
	bool			m_bIsMaster;

public:
///  Returns relay server's address
///  @return relay server's address
///  @remarks
	tstring GetServerAddress() const;

///  Returns relay server's port
///  @return relay server's port
///  @remarks
	unsigned int GetServerPort() const;

///  Returns user id for authentication on relay server
///  @return user id for authentication on relay server
///  @remarks
	tstring GetSrvUserId() const;

///  Returns password for authentication on relay server
///  @return password for authentication on relay server
///  @remarks
	tstring GetSrvPassword() const;

///  Returns connection id
///  @return connection id
///  @remarks
	tstring GetConnectId() const;

///  Returns local peer id
///  @return peer id
///  @remarks
	tstring GetLocalPeerId() const;

///  Returns remote peer id
///  @return peer id
///  @remarks
	tstring GetRemotePeerId() const;

///  Chech master/slave state
///  @return true if this is master peer
///  @remarks
	bool GetIsMaster() const;

///  Initializes relay server's parameters
///  @param   Server's address
///  @param   Server's port
///  @param   User Id
///  @param   Password
///  @remarks
	void SetRelayServer( const tstring&, const unsigned int&, const tstring&, const tstring& );

///  Initializes connection id and peer id
///  @param   Connection Id
///  @param   Local Peer Id
///  @param   Remote Peer Id
///  @remarks
///  @see
	void SetConnectionId( const tstring&, const tstring&, const tstring& );

///  Initializes master/slave state
///  @param   master/slave state
///  @remarks
	void SetIsMaster( const bool );
};
