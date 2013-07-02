/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRelayedNetworkStream.h
///
///  Declares CRelayedNetworkStream class, stream to
///    connect with Relay server
///
///  @author Dmitry Netrebenko @date 09.10.2006
///  @modified Max Sogin @date 01.12.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <winsock2.h>
#include <NWL/NetworkLayer.h>
#include "CAbstractServerNegotiatedNetworkStream.h"
#include "CSocketStream.h"
#include "CHTTPProxy.h"
#include "CTLSSocketStream.h"
#include "relay_messages.h"
#include <AidLib/CCrypto/CCrypto.h>

/// Relayed network stream. Use to connect through external relay server
template<class NWL_API SecureStream = CTLSSocketStream>
class NWL_API CRelayedNetworkStream : 
	public CAbstractServerNegotiatedNetworkStream,
	public SecureStream
{
private:
	CRelayedNetworkStream(const CRelayedNetworkStream&);
	CRelayedNetworkStream& operator=(const CRelayedNetworkStream&);

public:
	/// initializes object instance
	CRelayedNetworkStream()
		:	CAbstractServerNegotiatedNetworkStream()
		,	SecureStream()
	{
	TRY_CATCH
	CATCH_THROW()
	}

	/// destroys object instance
	~CRelayedNetworkStream()
	{
	TRY_CATCH
	CATCH_LOG()
	}

	///  Connect to remote host
	///  @param   do asynchronous connect
	virtual void Connect( const bool = false )
	{
	TRY_CATCH
		SecureStream::SetProxySettings(m_ProxySettings);
		SecureStream::SetConnectThroughProxy(m_bConnectThroughProxy);
		Log.Add(_MESSAGE_,"Connecting...");
		SecureStream::Connect(m_strServerAddr, m_nServerPort, true /*sync connect*/);
		Log.Add(_MESSAGE_,"Connected");
		try
		{
			SRelayMessage msg;
			/// Sending auth request
			msg.type = rmtAuthRequest;
			strcpy_s(msg.data,MAX_MSG_SIZE,m_strSrvUserId.c_str());
			msg.size = static_cast<int>(RELAY_MSG_HEAD_SIZE + m_strSrvUserId.length() + 1);
			Log.Add(_MESSAGE_,"Sending auth request...");
			SecureStream::SendInternal(reinterpret_cast<char*>(&msg),msg.size);
			Log.Add(_MESSAGE_,"Receiving challenge request...");
			/// Retriving challenge request
			SecureStream::ReceiveInternal(reinterpret_cast<char*>(&msg), RELAY_MSG_HEAD_SIZE);
			if(!msg.size || msg.size != RELAY_MSG_HEAD_SIZE + RELAY_CHALLENGE_SIZE)
				throw MCStreamException("Invalid message size");
			if(msg.type != rmtChallengeRequest)
				throw MCStreamException("Invalid message type");
			SecureStream::ReceiveInternal(msg.data, msg.size - RELAY_MSG_HEAD_SIZE);
			Log.Add(_MESSAGE_,"Challenge request received");
			/// Calculating hash
			std::auto_ptr<char> buf;
			buf.reset(new char[RELAY_CHALLENGE_SIZE + m_strSrvPassword.length()]);
			memcpy(buf.get(),m_strSrvPassword.c_str(),m_strSrvPassword.length());
			memcpy(buf.get()+m_strSrvPassword.length(),msg.data,RELAY_CHALLENGE_SIZE);
			char hash[RELAY_HASH_SIZE];
			CRYPTO_INSTANCE.MakeHash(buf.get(),RELAY_CHALLENGE_SIZE + (unsigned int)m_strSrvPassword.length(), hash, RELAY_HASH_SIZE);
			/// Sending challenge response
			msg.type = rmtChallengeResopnce;
			msg.size = RELAY_MSG_HEAD_SIZE + sizeof(hash);
			memcpy(msg.data, hash, sizeof(hash));
			Log.Add(_MESSAGE_,"Sending challenge response...");
			SecureStream::SendInternal(reinterpret_cast<char*>(&msg),msg.size);
			Log.Add(_MESSAGE_,"Shallenge response sent");
			/// Is auth succeeded?
			msg.size = RELAY_MSG_HEAD_SIZE;
			try
			{
				Log.Add(_MESSAGE_,"Receiving auth reply...");
				SecureStream::ReceiveInternal(reinterpret_cast<char*>(&msg),msg.size);
				Log.Add(_MESSAGE_,"Auth reply received");
			}
			catch(CStreamException&)
			{
				throw MCStreamException("Relay auth failed");
			}
			if (msg.type != rmtAuthSuccessfull)
				throw MCStreamException("Relay auth failed");
			/// Auth succeeded, sending connection request
			msg.type = rmtConnectRequest;
			strcpy_s(msg.data, MAX_MSG_SIZE, m_strConnectId.c_str());
			msg.size = static_cast<int>(RELAY_MSG_HEAD_SIZE + m_strConnectId.length() + 1);
			Log.Add(_MESSAGE_,"Sending connect request....");
			SecureStream::SendInternal(reinterpret_cast<char*>(&msg),msg.size);
			/// Waiting for connected
			msg.size = RELAY_MSG_HEAD_SIZE;
			SecureStream::m_sDataSocket->SetTimeout(m_nConnectTimeout);
			Log.Add(_MESSAGE_,"Waiting connect....");
			if (msg.size != m_sDataSocket->Receive(reinterpret_cast<char*>(&msg),msg.size))
				//throw MCStreamException("Connect timeout expired");
				throw MCStreamException_ErrCode("Connect timeout expired",1/*timeout*/);
			SecureStream::m_sDataSocket->SetTimeout(0);
			if (msg.type != rmtConnectResopnce)
				throw MCStreamException("Relay server failed to connect peer");

			Log.Add(_MESSAGE_,"Connected. Performing auth");
			/// Initiating TLS connect
			SecureStream::InitSecureConnection(m_bIsMaster);

			SecureStream::m_sDataSocket->SetTimeout(1000, sstReceive);
		}
		catch(...)
		{
			SecureStream::Close();
			throw;
		}
	CATCH_THROW()
	}

	///  Disconnect from remote host
	virtual void Disconnect()
	{
	TRY_CATCH
		SecureStream::Disconnect();
	CATCH_THROW()
	}

	///  Abstract function to get data from the stream
	///  @param   buffer for data
	///  @param   number of bytes to get
	///  @remarks
	virtual unsigned int ReceiveInternal( char* buf, const unsigned int& len )
	{
	TRY_CATCH
		return SecureStream::ReceiveInternal(buf,len);
	CATCH_THROW()
	}

	///  Abstract function to put data to stream
	///  @param   buffer with data
	///  @param   number of bytes to put
	///  @remarks
	virtual unsigned int SendInternal( const char* buf, const unsigned int& len )
	{
	TRY_CATCH
		return SecureStream::SendInternal(buf,len);
	CATCH_THROW()
	}

	///  Checks data in the stream
	///  @return returns amount of available data
	virtual bool HasInData()
	{
	TRY_CATCH
		return SecureStream::HasInData();
	CATCH_THROW()
	}

	/// Returns true if connected, false othervay
	/// @return true if connected, false othervay
	virtual bool Connected() const
	{
	TRY_CATCH
		return SecureStream::HasSecureConnection();
	CATCH_THROW()
	}

	/// Raises "OnConnected" event
	void RaiseConnectedEvent()
	{
	TRY_CATCH
		SecureStream::RaiseConnectedEvent();
	CATCH_THROW()
	}

	/// Raises "OnDisconnected" event
	void RaiseDisconnectedEvent()
	{
	TRY_CATCH
		SecureStream::RaiseDisconnectedEvent();
	CATCH_THROW()
	}

	/// Raises "OnConnectError" event
	void RaiseConnectErrorEvent( EConnectErrorReason reason )
	{
	TRY_CATCH
		SecureStream::RaiseConnectErrorEvent(reason);
	CATCH_THROW()
	}

	///  Extracts data from input buffer
	///  @param   Pointer to buffer
	///  @param   Buffer size
	///  @return  Number of bytes
	virtual unsigned int GetInBuffer( char* buf, const unsigned int& len )
	{
	TRY_CATCH
		return SecureStream::GetInBuffer(buf, len);
	CATCH_THROW()
	}

	///  Puts data to queue
	///  @param   buffer with data
	///  @param   number of bytes to put
	virtual void Send2Queue( const char* buf, const unsigned int& len )
	{
	TRY_CATCH
		SecureStream::Send2Queue(buf, len);
	CATCH_THROW()
	}

	///  Cancel reading from the stream
	virtual void CancelReceiveOperation()
	{
	TRY_CATCH
		SecureStream::CancelReceiveOperation();
	CATCH_THROW()
	}

private:
	///  Initializes secure connection
	///  @param masterRole - stream has master role
	///  @remarks - hides inherited method
	void InitSecureConnection( bool masterRole ) {};

	///  Returns true if secure connection is established
	///  @remarks - hides inherited method
	bool HasSecureConnection() const { return false; };
};
