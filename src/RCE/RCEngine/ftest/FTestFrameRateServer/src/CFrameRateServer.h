/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFrameRateServer.h
///
///  Declares CFrameRateServer class, responsible for server part of 
///    frame rate test
///
///  @author Dmitry Netrebenko @date 14.05.2007
///
///  @modified Alexander Novak @date 11.10.2007
///
///  Added MultiplexedStream support
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <winsock2.h>
#include "CFrame.h"
#include "CFrameSequence.h"
#include <NWL/Streaming/CSocketStream.h>
#include <boost/shared_ptr.hpp>
#include <AidLib/CThread/CThread.h>
#include <RCEngine/RCEngine.h>
#include <RCEngine/CRCHost.h>
#include "CFrameRateStreamMultiplexer.h"
#include <AidLib/Loki/Singleton.h>

///  CFrameRateServer class, responsible for server part of frame rate test
///  Access through CSingleton
///  Base class - CThread from AidLib
///  Base class - CRCHost from RCEngine
class CFrameRateServer
	:	public CThread
	,	public CRCHost
{
private:
	boost::shared_ptr<CFrameRateStreamMultiplexer> m_multiplexer;
	boost::shared_ptr<CAbstractStream> m_transportStream;

///  Prevents making copies of CFrameRateServer objects.
	CFrameRateServer( const CFrameRateServer& );
	CFrameRateServer& operator=( const CFrameRateServer& );

public:
///  Constructor
	CFrameRateServer();
///  Destructor
	~CFrameRateServer();

private:
///  Sequence of the frames
	SPFrameSequence						m_frameSequence;
///  Critical section to access current frame
	CRITICAL_SECTION					m_section;
///  Current frame to draw
	SPFrame								m_currentFrame;
///  Connection stream
	boost::shared_ptr<CSocketStream>	m_stream;
/// Event for connection
	HANDLE								m_connectEvent;
/// Event for start session
	HANDLE								m_sessionEvent;

public:
///  Thread's entry point
///  @param Params - thread's parameters
	virtual void Execute( void* Params );

///  Returns current frame to draw
	SPFrame GetCurrentFrame();

///  Returns pointer to critical section object
	CRITICAL_SECTION* GetSection();

	/// A virtual method that notifies session has started.
	/// @param clientId corresponding cliend identifier
	virtual void NotifySessionStarted( const int clientId );

private:
///  Stream's OnConnected event handler
	void ClientConnected( void* );
};

/// Should be used to CFrameRateServer as single instance
#define FRAMERATESERVER_INSTANCE Loki::SingletonHolder<CFrameRateServer, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
