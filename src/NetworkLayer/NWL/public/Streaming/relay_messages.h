/////////////////////////////////////////////////////////////////////////
///
///  Archer Software, Ltd
///
///  relay_messages.h
///
///  Peer-relay-peer protocol messages
///
///  @author Dmitry Netrebenko (dim_netr@archer-soft.com) @date 20.11.2006
///
////////////////////////////////////////////////////////////////////////

#ifndef RELAY_MESSAGES_H
#define RELAY_MESSAGES_H

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

/// Relay message type
enum ERelayMessageType
{
	rmtAuthRequest					= 0,	/// Authenticate request
	rmtChallengeRequest				= 1,	/// Challenge request
	rmtChallengeResopnce			= 2,	/// Challenge response
	rmtAuthSuccessfull				= 3,	/// Auth suceeded
	rmtConnectRequest				= 4,	/// Connection request
	rmtConnectResopnce				= 5,	/// Connection response
	rmtExternalIPRequest			= 6,	/// Request for own external ip
	rmtServerError					= 7,	/// Some error occurred. See msg data for details
	rmtChallengeResponceNoSession	= 8,	/// Challenge response for ExternalIPRequest and for checking opened port
	rmtCheckPort					= 9,	/// Check port
	rmtPortOpened					= 10,	/// Port opened
	rmtDiagnosticRequest			= 11	/// Diagnostic information request
};

#define RELAY_HASH_SIZE 20
#define RELAY_PORTCHECK_SIZE 4
#define MAX_MSG_SIZE 1024
#define RELAY_MSG_HEAD_SIZE (sizeof(SRelayMessage) - MAX_MSG_SIZE)
#pragma pack(push)
#pragma pack(1)
/// Relay messaggge structure
struct SRelayMessage
{
	int size;						/// Message size
	ERelayMessageType type;		/// Type of a message @see ERelayMessageType for details
	char data[MAX_MSG_SIZE]; 		/// Message data
};
#pragma pack(pop)

/// Size of challenge for relay
#define RELAY_CHALLENGE_SIZE 32



#define PEER_ADDR_SIZE 16

#define PEER_ADDR_HEAD_SIZE sizeof(SPeerAddr)
#pragma pack(push)
#pragma pack(1)
/// Peer address structure
struct SPeerAddr
{
	unsigned int port;				/// Port
	char address[PEER_ADDR_SIZE];	/// IP address
};
#pragma pack(pop)

/// Stun server's message type
enum EStunMessageType
{
	smtAuthRequest			= 0,	/// Authentication request
	smtAuthResponse			= 1,	/// Authentication response
	smtAuthFailed			= 2,	/// Authentication failed response
	smtServerBusy			= 3,	/// Server busy
	smtBindRequest			= 4,	/// Bind request
	smtBindResponse			= 5,	/// Bind response
	smtProbeRequest			= 6,	/// Probe request
	smtProbeResponse		= 7,	/// Probe response
	smtData					= 8,	/// Data
	smtStatAuthRequest		= 9,	/// Authentication request for statistics message
	smtPeerStatistic		= 10,	/// Statistics message
	smtPeerLog				= 11	/// Logging message
};


#define STUN_USERID_SIZE 64
#define STUN_CONNID_SIZE 64
#define STUN_PEERID_SIZE 64
#define STUN_HASH_SIZE 20
#define STUN_CHALLENGE_SIZE 32

#pragma pack(push)
#pragma pack(1)
/// Stun server's authenticator structure
struct SStunAuthenticator
{
	char				user_id[STUN_USERID_SIZE];		/// User Id
	char				hash[STUN_HASH_SIZE];			/// Hash
};
#pragma pack(pop)



#define STUN_MSG_HEAD_SIZE (sizeof(SStunMessage) - 1)

#pragma pack(push)
#pragma pack(1)
/// Stun server's message structure
struct SStunMessage
{
	unsigned int		ident;							/// Message identificator
	EStunMessageType	msg_type;						/// Message type
	SPeerAddr			src_addr;						/// Source address
	SPeerAddr			dest_addr;						/// Destination address
	char				connect_id[STUN_CONNID_SIZE];	/// Connection Id
	char				peer_id[STUN_PEERID_SIZE];		/// Peer Id
	SStunAuthenticator	auth;							/// Authenticator
	unsigned int		data_size;						/// Number of records in "data"
	unsigned int		size;							/// Structure size in bytes
	char				data[1];						/// Data
};
#pragma pack(pop)

/// Pointers to SStunMessage
typedef boost::shared_ptr<SStunMessage> SPStunMessage;
typedef boost::scoped_ptr<SStunMessage> SCPStunMessage;

#endif
