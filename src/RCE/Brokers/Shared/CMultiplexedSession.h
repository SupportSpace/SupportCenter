/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CMultiplexedSession.h
///
///  CMultiplexedSession object declaration. (Session (connection) object)
///
///  @author Kirill Solovyov @date 05.11.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once

#include <atlbase.h>
#include <atlwin.h>
#include <AidLib/Strings/tstring.h>
#include <NWL/Streaming/CStreamFactoryRelayedImpl.h>
#include <NWL/Streaming/CStreamException.h>
#include <NWL/Multiplexer/CStreamMultiplexerBase.h>
//#include <NWL/Multiplexer/CSubStream.h>
#include <atlsync.h>
#include <queue>

#include <AidLib/Logging/CInstanceTracker.h>

//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO
//TODO session id in CStreamFactory
//TODO GUID user id in CStreamFactory Getting external IP
//TODO review libs
//TODO
//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO//TODO




class CMultiplexedSession;
struct SData;
enum ESessionSentMessageState;
enum ESessionState;
#include "CSessionsMgr.h"
#include <NWL/Statistic/CMeasurement.h>


#pragma comment (lib, "Winmm.lib")
#pragma comment (lib, "Version.lib") ///TODO: if driver will be removed remove this dependency

#pragma comment(lib, "wininet.lib")

#pragma comment(lib, "nwl.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libjpeg.lib")
#pragma comment(lib, "Xregion.lib")
#pragma comment(lib, "udt.lib")
#ifdef _DYNAMIC_NWL_
	#pragma comment(lib, "w32gnutls.lib")
#else
	#pragma comment(lib, "gnutls.lib")
#endif
#pragma comment(lib, "aidlib.lib")
#pragma comment(lib, "miniUPnP.lib")

#pragma comment(lib, "ws2_32.lib")



#pragma pack(push)
#pragma pack(1)
/// the structure for safe array of row data
struct SData
{
	/// size data in bytes
	unsigned long m_size;
	/// beginnig of data
	char m_data[0];
	SData(){}
private:
	SData(const SData&);
	SData& operator=(const SData&);
};
#pragma pack(pop)

/// state of sent message
enum ESessionSentMessageState
{
	/// message sent to the stream
	SSMS_SENT=0,
	/// message enqueue of offline messages
	SSMS_ENQUEUE,
	/// message enqueue of offline messages and session is being connected
	SSMS_ENQUEUE_CONNECTING
};

/// state of session
enum ESessionState
{
	SS_CONNECTED=0,    //session is connected
	SS_DISCONNECTED,   //session is disconnected
	SS_CONNECTING,     //session is connecting
	SS_RESERVED1
};

/// auto set sub stream identifier
#define SUBSTREAMID_AUTOSET 0L



//------------------------------------------------------------------------------------------------------------------------------------
// default value of alive messages period equal 10s
#define DEFAULT_ALIVEPERIOD	10000L
							
/// The class is thread of live messages
class CLiveThread: 
	public CThread,
	public CInstanceTracker
{
protected:
	/// The method send live message with m_liveTime period
	virtual void Execute(void *Params);
	/// Live timer, it is period sending live messages in ms
	DWORD m_livePeriod;
	/// wake up event is used for wakeup thread for terminate
	//boost::shared_ptr<boost::remove_pointer<HANDLE>::type> m_wakeEvent;
	CEvent m_wakeEvent;
public:
	CLiveThread(void *_Param = NULL, const bool createMessageQueue = false,DWORD livePeriod=DEFAULT_ALIVEPERIOD);
	virtual ~CLiveThread();
};

//------------------------------------------------------------------------------------------------------------------------------------
/// CStreamMultiplexerBase implementation for events handling
class CStreamMultiplexerBaseImpl: 
	public CStreamMultiplexerBase,
	public CInstanceTracker
{
	friend class CStreamMultiplexerBase;
	CStreamMultiplexerBaseImpl(boost::shared_ptr<CAbstractStream> transportStream, CMultiplexedSession* owner=NULL);
public:
	CMultiplexedSession *m_owner;



	/// Called if substream was created on the other side
	/// @param serviceID			Service identifier
	virtual int OnSubStreamConnected(unsigned int serviceID);

	/// Called if substream was deleted on the other side
	/// @param serviceID			Service identifier
	virtual int OnSubStreamDisconnected(unsigned int serviceID);

	/// Called if transport stream was broken
	/// @param transportStream		Thr broken transport stream
	virtual int OnConnectionBroke(boost::shared_ptr<CAbstractStream> transportStream);

};

//------------------------------------------------------------------------------------------------------------------------------------
/// The session class (CSession reserved)
class CMultiplexedSession:
	protected CStreamFactoryRelayedImpl,
	protected CThread
{
	friend class CStreamMultiplexerBaseImpl;
	friend class CLiveThread;
protected:
	/// the owner object
	CSessionsMgr* m_owner;

	/// the inner state set by call Connect() reset by Disconect(). is used for reconnect
	bool m_innerState;

	///// Session parameters
	//struct
	//{
	//	/// relay server address
	//	tstring m_relaySrv;
	//	/// session identifier
	//	tstring m_sId;
	//	/// user name
	//	tstring m_user;
	//	/// pass word
	//	tstring m_passwd;
	//	///	remote user name
	//	tstring m_remoteUser;
	//	/// 
	//	unsigned int m_timeOut;
	//	/// 
	//	bool m_masterRole;
	//} m_params;

	/// Virtual method, that notifyes connection has completed
	/// @param stream connected stream, null if not connected (NULL -> stream.get() == NULL)
	virtual void ConnectCompletion(boost::shared_ptr<CAbstractNetworkStream> stream);

	/// Transport stream
	boost::shared_ptr<CAbstractNetworkStream> m_transportStream;

	/// Stream Multiplexer
	boost::shared_ptr<CStreamMultiplexerBaseImpl> m_muxStream;

	/// service stream. it is used for Send and Receive data
	boost::shared_ptr<CAbstractStream> m_svcStream;

	/// the service stream connected event. set when service stream is connected
	CEvent m_svcStreamConnectedEvent;

	/// The method is Receive thread entry point
	virtual void Execute(void *Params);

	/// Initializes message exchanger. The method is blank, initialStream is connected separately. Factory use this initialStream.
	/// @param sourcePeerId source peerId
	/// @param destPeerId destination peedId
	/// @param timeOut connection timeout
	/// @param masterRole peer's role
	virtual void InitExchanger(const tstring& sourcePeerId, const tstring& destPeerId, int timeOut, bool masterRole);

	/// the queue of message which sent by disconnected state
	std::queue < boost::shared_ptr<SData> > m_offlineMessages;
	CCriticalSection m_csOfflineMessages;

	/// the map of asked stream but it is not connected streams
	std::map<unsigned int,boost::shared_ptr<CAbstractStream> > m_askedStreams;

	/// last created substream identifier. used by new substream identifier generation
	unsigned int m_lastSubStreamId;

	/// The method send data to remote side
	/// @param data data structure to sending
	void innerSend(boost::shared_ptr<SData> data);

	/// Reports statistics for the conencted stream
//	virtual void ReportStat(boost::shared_ptr<CAbstractNetworkStream> stream){/*::Beep(100,2000);*/}

	///TODO two essences are protected by this CS
	CCriticalSection m_cs;

public:
	/// Session parameters
	struct
	{
		/// relay server address
		tstring m_relaySrv;
		/// session identifier
		tstring m_sId;
		/// user name
		tstring m_user;
		/// pass word
		tstring m_passwd;
		///	remote user name
		tstring m_remoteUser;
		/// 
		unsigned int m_timeOut;
		/// 
		bool m_masterRole;
		/// user name
		tstring m_userName;
		/// remote user name
		tstring m_remoteUserName;
	} m_params;

	/// ctor
	CMultiplexedSession(CSessionsMgr* owner=NULL);
	/// dtor
	virtual ~CMultiplexedSession(void);

	/// Init new object by connection parameters
	/// @param relaySrv Relay server address 
	/// @param sId session identifier
	/// @param user user name
	/// @param passwd user pass word
	/// @param remoteUser remote user name
	/// @param timeOut time of connection
	/// @param masterRole 
	void Init(const tstring& relaySrv, const tstring& sId, const tstring& user, const tstring& passwd, const tstring& remoteUser, unsigned int timeOut, bool masterRole);

	/// The method initialized new session (start thread, Execute())
	void Connect(void);
	
	/// The method disconnect connection
	void Disconnect(void);
	
	/// The method send data to remote side
	/// @param data data structure to sending
	/// @return state of the message
	ESessionSentMessageState Send(boost::shared_ptr<SData> data);

	/// The method is called by data retrieved from remote side
	/// @param data received data structure
	void OnReceived(boost::shared_ptr<SData> data);

	/// The method is called when connection progress change its state
	/// @param message text describe connection status
	void OnConnecting(const tstring& message);


	/// The method retrieve (master/slave) session's side role
	/// @return true if it is master role and false if slave
	bool IsMasterRole(void);


	/// The method ask sub stream (asynchronous).
	/// @param subStreamId Service identifier
	/// @param priorityLevel Priority level for the substream
	/// @return real stream identifier
	unsigned int GetSubStream(unsigned int subStreamId, unsigned int priorityLevel);

	/// The method is called when sub stream is connected
	/// @param subStreamId Service identifier
	void OnSubStreamTaken(unsigned int subStreamId);

	/// The method return state of session
	ESessionState GetSessionState(void);

	/// Reports Connection speed statistics
	void ReportConnectionSpeed();

};
