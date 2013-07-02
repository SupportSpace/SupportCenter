/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStreamFactory.h
///
///  Steam factory
///
///  @author "Archer Software" Sogin M. @date 20.11.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once
#include <NWL/NetworkLayer.h>
#include "CAbstractNetworkStream.h"
#include <AidLib/CThread/CThread.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <set>
#include <map>
#include <AidLib/CThread/CThreadLS.h>
#include <NWL/Streaming/relay_messages.h>

class CDirectNetworkStream;
class CNATTraversingUDPNetworkStream;

// Order of LAN TCP delay. WAN delay doesn't needed, since in most cases
// for real IP's we'll have only ope IP's pair, or we'll haven't TCP connect ability
#define NET_DELAY_ORDER 100 /* 100 milliseconds */

/// Steam factory
class NWL_API CStreamFactory : public CInstanceTracker
{
	friend class CStarter;
private:
	/// Fabric timers map
	static std::map<UINT_PTR,CStreamFactory*> m_fabricTimersMap;
	/// Internal timer ID
	UINT_PTR m_timerId;
	/// Internal flag for async mode
	bool m_async;

	/// pointer to connected stream NULL, if there are no connected streams
	void *m_connectedStream;

	/// Timer event handler
	static void CALLBACK OnTimer(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);

	/// To allow ancessors be inherited from CThread
	/// creating this little thread starter stub
	class CStarter : public CThread
	{
	public:
		CStarter(CStreamFactory *factory) 
			:	CThread(), 
				m_factory(factory)
		{
			SetTerminateTimeout(INFINITE);
		};
	private:
		/// corresponding factory instance
		CStreamFactory *m_factory;
		/// Thread entry point
		virtual void Execute(void*)
		{
			SET_THREAD_LS;
			m_factory->StartThread(NULL);
		}
	};
	std::auto_ptr<CStarter> m_threadStarter;

	/// Critical section for stream event handlers
	CRITICAL_SECTION m_streamEventsCS;
	/// count of currently connecting streams
	int m_connectingStreamsCount;
	/// Stream connect event handlers
	void SomeStreamConnected(void*);
	void SomeStreamFailed2Connect(void*, EConnectErrorReason);

protected:
	tstring m_sourcePeerId;
	tstring m_destPeerId;
	tstring m_sessionId;
	int m_timeOut;
	bool m_masterRole;
	/// Error string
	tstring m_error;

	/// UserId for authentication on server
	tstring m_strServerUserId;

	tstring m_connectId;

private:
	/// critical section to prevent concurent connect calls
	CRITICAL_SECTION m_cs;

	/// Connect result
	boost::shared_ptr<CAbstractNetworkStream> m_stream;

	HANDLE m_hConnectEvent;

	/// connect aborted
	bool m_conenctAborted;

	/// Password for authentication on server
	tstring m_strServerPassword;

	/// internal purposes flag
	bool m_waitingForCompletion;

	/// Use this f-n to call connect completion from CStreamFactory
	void CallCompletion(boost::shared_ptr<CAbstractNetworkStream> stream, const tstring& error = tstring());

	/// throws exception if connect aborted
	/// and turns aborted flag off
	void CheckAborted();

	/// Try to connect directly
	/// @param remoteAddress remote ip address, having following format IP_OR_HOSTNAME[:PORT]
	/// If remote port not specified, default value from NWL_INSTANCE is used
	void DirectConnect(	const tstring& connectId, 
						const tstring& secret, 
						const tstring& remoteAddress,
						const int timeOut,
						boost::shared_ptr<CDirectNetworkStream>&);

	/// Try to connect directly through proxy
	void DirectProxyConnect(	const tstring& connectId, 
								const tstring& secret, 
								const tstring& remoteAddress,
								const int timeOut,
								boost::shared_ptr<CDirectNetworkStream>&);

	/// Try to connect through NAT traversal
	void NatTraversalConnect(	const tstring& connectId, 
								const tstring& secret, 
								const tstring& remoteAddress,
								const int timeOut,
								const bool masterRole,
								const tstring& destPeerId,
								const tstring& sourcePeerId,
								boost::shared_ptr<CNATTraversingUDPNetworkStream>&);

	/// Try to connect throung relay server
	/// @return inited stream if connect is successfull
	boost::shared_ptr<CAbstractNetworkStream> RelayedConnect(	const tstring& connectId, 
														const tstring& secret, 
														const tstring& remoteAddress,
														const int timeOut,
														const bool masterRole,
														const tstring& destPeerId,
														const tstring& sourcePeerId );

	/// Try to connect throung relay server through proxy
	/// @return inited stream if connect is successfull
	boost::shared_ptr<CAbstractNetworkStream> RelayedProxyConnect(	const tstring& connectId, 
															const tstring& secret, 
															const tstring& remoteAddress,
															const int timeOut,
															const bool masterRole,
															const tstring& destPeerId,
															const tstring& sourcePeerId );

	/// Returns local addresses set
	static std::set<tstring> GetLocalAddresses();

	/// Retrives addresses list from it's string representation
	static std::vector<tstring> GetRemoteAddresses(const tstring &addressesStr);

	/// Get external IP
	void GetExternalIp(SPeerAddr& extAddress, const int timeOut);

	/// Retrives locall addresses list (including external address)
	/// @return addresses list string, ready for sending to peer
	tstring RetriveLocalAddressesList(std::set<tstring> &localAddresses, const int TimeOut);

	/// Performs initial stuff exchange via jabber server
	void ExchngeInitialStuff(const tstring& sourcePeerId, 
							const tstring& destPeerId, 
							int timeOut, 
							bool masterRole,
							const tstring &addressStr, 
							std::vector<tstring> &remoteAddresses,
							tstring& connectId,
							tstring& secret,
							tstring& remoteAddress);
protected:

	/// Reports Connection speed statistics
	virtual void ReportConnectionSpeed();

	/// Reports statistics for the conencted stream
	virtual void ReportStat(boost::shared_ptr<CAbstractNetworkStream> stream);

	/// Sends a message to the specified destination peer (user ID).  
	/// @param peerId destination peer
	/// @param messageData Message data is an arbitrary string.
	virtual void SendMsg(const tstring& peerId, const tstring& messageData) = NULL;

	/// Handles an incoming message from a specified source peer (user ID).
	/// @param peerId source peer
	/// @param messageData Message data is an arbitrary string.
	virtual void HandleMsg(const tstring& peerId, tstring& messageData) = NULL;

	/// Returns event handle, which handles incomming IM messages
	/// @return event handle, which handles incomming IM messages
	virtual HANDLE GetHandleMsgEvent(); 

	/// A callback method that is invoked by the factory to notify the progress 
	/// and status of connection attempt.
	/// @param percentCompleted Percent Completed is an integer value in the range of 0-100 
	/// that represents the connection progress (in terms of steps/actions done).
	/// @param status Status is a text message that describes the current step.
	virtual void NotifyProgress(const int& percentCompleted, const tstring& status){};

	/// Attempt to establish a connection with the specified peer (user ID).
	/// @param sourcePeerId source peerId
	/// @param destPeerId destination peedId
	/// @param masterRole Master Role is a boolean value that determines the role of the peer; 
	/// each connection is established between one master and one slave peer.  
	/// The role only affects the process of negotiating connection parameters through Instant Messaging.
	/// @return Returns a new stream (AbstractNetworkStream) or failure indication.
	boost::shared_ptr<CAbstractNetworkStream> ConnectInternal(const tstring& sourcePeerId, const tstring& destPeerId, int timeOut, bool masterRole);

	/// Thread entry point
	void StartThread(void*);

	/// Initializes message exchanger
	/// @param sourcePeerId source peerId
	/// @param destPeerId destination peedId
	/// @param timeOut connection timeout
	/// @param masterRole peer's role
	virtual void InitExchanger(const tstring& sourcePeerId, const tstring& destPeerId, int timeOut, bool masterRole) {};

	/// Closes message exchanger
	/// @param masterRole peer's role
	virtual void CloseExchanger(bool masterRole) {};

	/// Aborts exchanger
	virtual void AbortExchanger() {};

public:
	/// Initializes object instance
	CStreamFactory();
	/// Destroys object instance
	virtual ~CStreamFactory();

	/// Attempt to establish a connection with the specified peer (user ID).
	/// @param sourcePeerId source peerId
	/// @param destPeerId destination peedId
	/// @param masterRole Master Role is a boolean value that determines the role of the peer; 
	/// each connection is established between one master and one slave peer.  
	/// The role only affects the process of negotiating connection parameters through Instant Messaging.
	/// @param async if set to true, then Connect returns immediately, and ConnectCompletion is called on connect complete
	/// @return Returns a new stream (AbstractNetworkStream) or failure indication.
	boost::shared_ptr<CAbstractNetworkStream> Connect(const tstring& sId,const tstring& sourcePeerId, const tstring& destPeerId, int timeOut, bool masterRole, bool async = false);

	/// Virtual metody, that notifyes connect is complete
	/// @param stream connected stream, null if not connected (NULL -> stream.get() == NULL)
	virtual void ConnectCompletion(boost::shared_ptr<CAbstractNetworkStream> stream) {};

	/// Aborts an on-going connection attempt.
	void AbortConnect();

	///  Returns userid for authentication on server
	///  @return userid
	tstring GetServerUserId() const;

	///  Setup userid for authentication on server
	///  @param   new userid
	void SetServerUserId( const tstring& );

	///  Returns password for authentication on server
	///  @return password
	tstring GetServerPassword() const;

	///  Setup password for authentication on server
	///  @param   new password
	void SetServerPassword( const tstring& );

};
