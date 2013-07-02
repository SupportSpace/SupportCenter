/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CResolutionClient.h
///
///  Declares CResolutionClient class, responsible for client part of 
///    "Resolution" functional test
///
///  @author Dmitry Netrebenko @date 13.06.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once
#include <NWL/Streaming/CSocketStream.h>
#include <boost/shared_ptr.hpp>
#include "CTestViewer.h"
#include "CFrameWaiter.h"
#include "CFrame.h"
#include <windows.h>
#include <AidLib/Loki/Singleton.h>

///  CResolutionClient class, responsible for client part of 
///    "Resolution" functional test
///  Access through singleton
class CResolutionClient
{
private:
///  Prevents making copies of CResolutionClient objects.
	CResolutionClient( const CResolutionClient& );
	CResolutionClient& operator=( const CResolutionClient& );

public:
///  Constructor
	CResolutionClient();
///  Destructor
	~CResolutionClient();

private:
///  Viewer
	boost::shared_ptr<CTestViewer>		m_viewer;
///  Connection stream
	boost::shared_ptr<CSocketStream>	m_stream;
///  Frames waiter thread
	SPFrameWaiter						m_waiter;
///  Frames
	SPFrame								m_firstFrame;
	SPFrame								m_secondFrame;
	SPFrame								m_currentFrame;
///  Critical section to access frames
	CRITICAL_SECTION					m_section;

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

///  Event handler for changing resolution of remote desktop
///  @param width - new width
///  @param height - new height
	void OnResolutionChanged(const int width, const int height);

private:
///  Event handler for frame timeout
	void OnFrameTimeout( void* );
	
};

/// Should be used to CResolutionClient as single instance
#define RESOLUTIONCLIENT_INSTANCE Loki::SingletonHolder<CResolutionClient, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
