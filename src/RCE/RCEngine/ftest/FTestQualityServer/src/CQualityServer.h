/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CQualityServer.h
///
///  Declares CQualityServer class, responsible for server part of 
///    "Picture Quality" functional test
///
///  @author Dmitry Netrebenko @date 18.07.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <winsock2.h>
#include <AidLib/CThread/CThread.h>
#include <RCEngine/RCEngine.h>
#include <NWL/Streaming/CSocketStream.h>
#include <boost/type_traits/remove_pointer.hpp>
#include "SPTypes.h"
#include <AidLib/Loki/Singleton.h>

///  CQualityServer class, responsible for server part of 
///    "Picture Quality" functional test
///  Access through CSingleton
///  Base class - CThread from AidLib
///  Base class - CRCHost from RCEngine
class CQualityServer
	:	public CThread
	,	public CRCHost
{
private:
///  Prevents making copies of CQualityServer objects.
	CQualityServer( const CQualityServer& );
	CQualityServer& operator=( const CQualityServer& );

public:
///  Constructor
	CQualityServer();
///  Destructor
	~CQualityServer();

private:
///  Connection stream
	boost::shared_ptr<CSocketStream>	m_stream;
///  Critical section to access current frame
	CRITICAL_SECTION					m_section;
///  Event fot connection
	SPHandle							m_event;
///  Frame's DC
	SPDC								m_frameDC;
///  Frame's bitmap
	SPBitmap							m_frameBitmap;

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
	virtual void NotifySessionStopped( const int clientId, ESessionStopReason ReasonCode );

///  Draws current frame
	void DrawFrame();

private:
///  Stream's OnConnected event handler
	void ClientConnected( void* );

///  Stops test
	void StopServer();

///  Prepares frame's bitmap
	void PrepareFrame();

/// Sends information about screen to viewer
	void SendScreenInfo();

};

/// Should be used to CQualityServer as single instance
#define QUALITYSERVER_INSTANCE Loki::SingletonHolder<CQualityServer, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
