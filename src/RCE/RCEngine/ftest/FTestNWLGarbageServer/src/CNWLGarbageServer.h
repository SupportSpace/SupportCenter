/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNWLGarbageServer.h
///
///  Declares CNWLGarbageServer class, responsible for server part of 
///    "NWL Garbage" functional test
///
///  @author Dmitry Netrebenko @date 06.06.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <winsock2.h>
#include <AidLib/CThread/CThread.h>
#include <RCEngine/RCEngine.h>
#include <NWL/Streaming/CSocketStream.h>
#include "CGarbageThread.h"
#include <boost/type_traits/remove_pointer.hpp>
#include "CFrame.h"
#include <AidLib/Loki/Singleton.h>

///  Shared pointer to HANDLE type
typedef boost::shared_ptr< boost::remove_pointer<HANDLE>::type > SPHandle;

///  CNWLGarbageServer class, responsible for server part of 
///    "NWL Garbage" functional test
///  Access through CSingleton
///  Base class - CThread from AidLib
///  Base class - CRCHost from RCEngine
class CNWLGarbageServer
	:	public CThread
	,	public CRCHost
{
private:
///  Prevents making copies of CNWLGarbageServer objects.
	CNWLGarbageServer( const CNWLGarbageServer& );
	CNWLGarbageServer& operator=( const CNWLGarbageServer& );

public:
///  Constructor
	CNWLGarbageServer();
///  Destructor
	~CNWLGarbageServer();

private:
///  Connection stream
	boost::shared_ptr<CSocketStream>	m_stream;
///  Thread which sends garbage to stream
	SPGarbageThread						m_garbageThread;
///  Critical section to access current frame
	CRITICAL_SECTION					m_section;
///  Event fot connection
	SPHandle							m_connectEvent;
///  Event for start session
	SPHandle							m_sessionEvent;
///  Frames
	SPFrame								m_firstFrame;
	SPFrame								m_secondFrame;
	SPFrame								m_currentFrame;
///  Is session active
	bool								m_sessionActive;
///  Client Id
	int									m_clientId;

public:
///  Thread's entry point
///  @param Params - thread's parameters
	virtual void Execute( void* Params );

/// A virtual method that notifies session has started.
/// @param clientId corresponding cliend identifier
	virtual void NotifySessionStarted( const int clientId );

	/// A virtual method that notifies session has stopped and why 
	/// @param ReasonCode stop reason (Stop() called; Remote Stop() called; Stream signaled end-of-file; etc.)
	/// @param clientId corresponding cliend identifier
	virtual void NotifySessionStopped(const int clientId, ESessionStopReason ReasonCode);

	/// A virtual method that notifies session has broke.
	virtual void NotifySessionBroke();

	/// A virtual method that notifies session has restored.
	virtual void NotifySessionRestored();

///  Draws current frame
	void DrawFrame();

private:
///  Stream's OnConnected event handler
	void ClientConnected( void* );
};

/// Should be used to CNWLGarbageServer as single instance
#define NWLGARBAGESERVER_INSTANCE Loki::SingletonHolder<CNWLGarbageServer, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
