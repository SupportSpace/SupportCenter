/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CConnectedLogSvr.h
///
///  Declares CConnectedLogSvr class, responsible for transportation
///    log messages to separated log server 
///
///  @author Dmitry Netrebenko @date 20.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <winsock2.h>
#include <AidLib/CThread/CThread.h>
#include <AidLib/Strings/tstring.h>
#include <NWL/Streaming/CSSocket.h>
#include <queue>
#include "SLogMsg.h"
#include <NWL/Streaming/CAbstractStream.h>
#include <NetLog/SServerMessage.h>
#include <boost/function.hpp>
#include "EConnectedLogSvrEvent.h"
#include <AidLib/Logging/cLog.h>

///  Log server event
typedef boost::function<void (int, EConnectedLogSvrEvent, void*)> LogSvrEvent;
///  Queue of the log messages
typedef std::queue<SPLogMsg> MsgQueue;
///  Shared pointer to CAbstractStream
typedef boost::shared_ptr<CAbstractStream> SPAbstractStream;


///  CConnectedLogSvr class, responsible for transportation
///    log messages to separated log server 
///  Base class - CThread
class CConnectedLogSvr : public CThread
{
private:
///  Prevents making copies of CTCPListener objects.
	CConnectedLogSvr( const CConnectedLogSvr& );
	CConnectedLogSvr& operator=( const CConnectedLogSvr& );

public:
///  Constructor
///  @param id - server id
///  @param stream - connected abstract stream
	CConnectedLogSvr( const int id, SPAbstractStream stream );

///  Destructor
	~CConnectedLogSvr();

///  Thread's entry point
///  @param Params - thread parameters
	virtual void Execute( void *Params );

private:
///  Delayed messages queue
	MsgQueue			m_delayedMessages;
///  Is sending delayed
	bool				m_delayedSend;
///  Critical section to access queue
	CRITICAL_SECTION	m_section;
///  Connected log server id
	const int			m_svrId;
///  Abstract stream
	SPAbstractStream	m_stream;
///  Log server event
	LogSvrEvent			m_event;

public:
///  Adds log message to queue
	void AddLogMsg( SPLogMsg msg );

///  Returns log server event handler
	LogSvrEvent GetLogSvrEvent() const;

///  Sets new log server event handler
///  @param handler - new event handler
	void SetLogSvrEvent( LogSvrEvent handler );

///  On stream disconnected event handler
	void OnStreamDisconnected( void* );

///  Returns server id
	int GetSvrId() const;

private:

///  Raises disconnected event
	void RaiseDisconnectedEvent();

///  Processes server message
	void OnServerMessage( SPServerMessage msg );

///  Changes Delayed mode
///  @param mode - new mode
	void OnChangeDelayedMode( const bool mode );

///  Sends delayed log to server
	void OnGetDelayedLog();

///  Changes verbosity level
///  @param verbosity - new verbosity level
	void OnChangeVerbosity( eVerbosity verbosity );
};

///  Shared pointer to CConnectedLogSvr
typedef boost::shared_ptr<CConnectedLogSvr> SPConnectedLogSvr;
