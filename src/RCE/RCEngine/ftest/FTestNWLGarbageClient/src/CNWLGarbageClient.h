/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNWLGarbageClient.h
///
///  Declares CNWLGarbageClient class, responsible for client part of 
///    "NWL Garbage" functional test
///
///  @author Dmitry Netrebenko @date 07.06.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <winsock2.h>
#include <NWL/Streaming/CSocketStream.h>
#include <boost/shared_ptr.hpp>
#include "CTestViewer.h"
#include "CGarbageThread.h"
#include "CFrameWaiter.h"
#include "CFrame.h"
#include <AidLib/Loki/Singleton.h>

///  CNWLGarbageClient class, responsible for client part of 
///    "NWL Garbage" functional test
///  Access through singleton
class CNWLGarbageClient
{
private:
///  Prevents making copies of CNWLGarbageClient objects.
	CNWLGarbageClient( const CNWLGarbageClient& );
	CNWLGarbageClient& operator=( const CNWLGarbageClient& );

public:
///  Constructor
	CNWLGarbageClient();
///  Destructor
	~CNWLGarbageClient();

private:
///  Viewer
	boost::shared_ptr<CTestViewer>		m_viewer;
///  Connection stream
	boost::shared_ptr<CSocketStream>	m_stream;
///  Thread which sends garbage to stream
	SPGarbageThread						m_garbageThread;
///  Is session active
	bool								m_sessionActive;
///  Frames waiter thread
	SPFrameWaiter						m_waiter;
///  Frames
	SPFrame								m_firstFrame;
	SPFrame								m_secondFrame;
	SPFrame								m_currentFrame;

public:
///  Starts viewer
	void StartViewer();

///  Checks viewer's frame
///  @param hdc - DC of viewer's bitmap
	void ProcessFrame(HDC hdc);

///  Session stop calback
	void SessionStop(ESessionStopReason ReasonCode);

///  Session start callback
	void SessionStart();

///  Session broke callback
	void SessionBroke();

///  Session restored callback
	void SessionRestored();

private:
///  Event handler for session timeout
	void OnSessionTimeout( void* );

///  Event handler for frame timeout
	void OnFrameTimeout( void* );
	
};

/// Should be used to CNWLGarbageClient as single instance
#define NWLGARGAGECLIENT_INSTANCE Loki::SingletonHolder<CNWLGarbageClient, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
