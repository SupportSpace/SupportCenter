/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNetworkLog.h
///
///  Declares CNetworkLog class, responsible for sending log messages
///    to log server 
///
///  @author Dmitry Netrebenko @date 20.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <winsock2.h>
#include <AidLib/Strings/tstring.h>
#include <AidLib/Logging/cLog.h>
#include "CConnectedLogSvr.h"
#include <map>
#include "CLogTransportLayer.h"
#include <queue>
#include <list>


///  Map of log servers
typedef std::map<int,SPConnectedLogSvr> LogServers;
///  Log server entry
typedef std::pair<int,SPConnectedLogSvr> LogServer;

///  Environment variable name to store flag which indicates using of prestored messages
#define NETLOG_SHOW_PRESTORED_MESSAGES_ENV _T("NETLOG_SHOW_PRESTORED_MESSAGES")
///  Default value of flag which indicates using of prestored messages
#define NETLOG_DEFAULT_SHOW_PRESTORED_MESSAGES false
///  Size of the list for prestored messages
#define NETLOG_PRESTORED_MESSAGES_COUNT 1000

///  List of net log messages
typedef std::list<SPLogMsg> MsgList;

///  CNetworkLog class, responsible for sending log messages
///    to log servers
///  Base class - cLog from AidLib
class CNetworkLog : public cLog
{
private:
#ifndef _DYNAMIC_AID_
///  Prevents making copies of CNetworkLog objects.
	CNetworkLog( const CNetworkLog& );
	CNetworkLog& operator=( const CNetworkLog& );
#endif

	///  CNetLogCleaner class, responsible for cleaning
	///    list of log servers
	///  Base class - CThread
	class CNetLogCleaner : public CThread
	{
	private:
	///  Critical sectio
		CRITICAL_SECTION				m_cs;
	///  Queue of disconnected servers
		std::queue<SPConnectedLogSvr>	m_queue;
	public:
	///  Constructor
		CNetLogCleaner();
	///  Destructor
		~CNetLogCleaner();
	///  Adds server to queue
		void AddSvr(SPConnectedLogSvr svr);
	///  Thread's entry point
		void Execute(void*);
	private:
	///  Extract server from queue
		SPConnectedLogSvr GetSvr();
	};

public:
///  Constructor
///  @param name - log name
	CNetworkLog( const tstring& name );

///  Destructor
	~CNetworkLog();

///  Adds string to a log composed from NULL terminated
///  list of PTCHAR parametrs
	virtual void AddList( const cEventDesc &EventDesc, const TCHAR *Item, ... );

private:
///  Log name
	tstring				m_name;
///  Critical section to access log servers map
	CRITICAL_SECTION	m_svrSection;
///  Map of connected log servers
	LogServers			m_connectedLogSvrs;
///  Id for next connected log server
	int					m_nextSvrId;
///  Transport layer
	SPLogTransportLayer	m_transportLayer;
///  Thread to destroy disconnected servers
	CNetLogCleaner		m_cleaner;
///  List of prestored messages
	MsgList				m_prestoredMessages;
///  Critical section to access prestored messages list
	CRITICAL_SECTION	m_prestoredSection;
///  Flag which indicates using of prestored messages
	bool				m_showPrestoredMessages;
///  Size of prestored messages list
	unsigned int		m_prestoredMsgCount;

private:
///  Log server events handler
///  @param svrId - server id
///  @param svrEvent - event type
///  @param params - event parameters
	void OnSvrEvent( int svrId, EConnectedLogSvrEvent svrEvent, void* params );

///  Disconnects log server
///  @param svrId - server id
	void DisconnectServer( int svrId );

///  Overrides abstract method
	void AddString( const TCHAR* LogString, const eSeverity Severity ) {};

///  Adds net log message to prestored list
///  @param msg - net log message
	void AddMsgToPrestoredList( SPLogMsg msg );

///  Sends all prestored messages to net log server
///  @param svr - net log server
	void SendPrestoredMsgsToSvr( SPConnectedLogSvr svr );

public:
///  Adds new log server to map
	void AddConnectedSvr( SPConnectedLogSvr svr );

///  Returns next server id
	int GetNextSvrId();
};
