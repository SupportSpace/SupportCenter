/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CResolutionServer.h
///
///  Declares CResolutionServer class, responsible for server part of 
///    "Resolution" functional test
///
///  @author Dmitry Netrebenko @date 11.06.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once
#include <NWL/Streaming/CSocketStream.h>
#include <windows.h>
#include <AidLib/CThread/CThread.h>
#include <RCEngine/RCEngine.h>
#include <boost/type_traits/remove_pointer.hpp>
#include "CFrame.h"
#include <AidLib/Loki/Singleton.h>

///  Shared pointer to HANDLE type
typedef boost::shared_ptr< boost::remove_pointer<HANDLE>::type > SPHandle;

///  CResolutionServer class, responsible for server part of 
///    "Resolution" functional test
///  Access through CSingleton
///  Base class - CThread from AidLib
///  Base class - CRCHost from RCEngine
class CResolutionServer
	:	public CThread
	,	public CRCHost
{
private:
///  Prevents making copies of CResolutionServer objects.
	CResolutionServer( const CResolutionServer& );
	CResolutionServer& operator=( const CResolutionServer& );

public:
///  Constructor
	CResolutionServer();
///  Destructor
	~CResolutionServer();

private:
///  Connection stream
	boost::shared_ptr<CSocketStream>	m_stream;
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

public:
///  Thread's entry point
///  @param Params - thread's parameters
	virtual void Execute( void* Params );

/// A virtual method that notifies session has started.
/// @param clientId corresponding cliend identifier
	virtual void NotifySessionStarted( const int clientId );

///  Draws current frame
	void DrawFrame();

private:
///  Stream's OnConnected event handler
	void ClientConnected( void* );

///  Changes screen resolution
	void ChangeResolution();
};

/// Should be used to CResolutionServer as single instance
#define RESOLUTIONSERVER_INSTANCE Loki::SingletonHolder<CResolutionServer, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
