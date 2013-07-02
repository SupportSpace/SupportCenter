/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFrameRateClient.h
///
///  Declares CFrameRateClient class, responsible for client part of 
///    frame rate test
///
///  @author Dmitry Netrebenko @date 15.05.2007
///
///  @modified Alexander Novak @date 11.10.2007
///
///  Added MultiplexedStream support
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <winsock2.h>
#include <NWL/Streaming/CSocketStream.h>
#include <boost/shared_ptr.hpp>
#include "CTestResults.h"
#include "CFrameSequence.h"
#include "CTestViewer.h"
#include "CFrameRateStreamMultiplexer.h"
#include <AidLib/Loki/Singleton.h>

///  CFrameRateClient class, responsible for client part of frame rate test
///  Access through CSingleton
class CFrameRateClient
{
private:
	boost::shared_ptr<CFrameRateStreamMultiplexer> m_multiplexer;
	boost::shared_ptr<CAbstractStream> m_transportStream;

///  Prevents making copies of CFrameRateClient objects.
	CFrameRateClient( const CFrameRateClient& );
	CFrameRateClient& operator=( const CFrameRateClient& );

public:
///  Constructor
	CFrameRateClient();
///  Destructor
	~CFrameRateClient();

private:
///  Viewer
	boost::shared_ptr<CTestViewer>		m_viewer;
///  Connection stream
	boost::shared_ptr<CSocketStream>	m_stream;
///  Results storage
	SPTestResults						m_results;
///  Frames sequence
	SPFrameSequence						m_frameSequence;
///  Session start time
	DWORD								m_startTime;
///  Is session started
	bool								m_sessionStarted;
///  Time of previous frame
	DWORD								m_frevFrameTime;

public:
///  Starts viewer
	void StartViewer();

///  Checks viewer's frame
///  @param hdc - DC of viewer's bitmap
	void ProcessFrame(HDC hdc);

///  Session stop calback
	void SessionStop();

///  Session start callback
	void SessionStart();

};

/// Should be used to CFrameRateClient as single instance
#define FRAMERATECLIENT_INSTANCE Loki::SingletonHolder<CFrameRateClient, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
