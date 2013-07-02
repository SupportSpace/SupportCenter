/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CQualityClient.h
///
///  Declares CQualityClient class, responsible for client part of 
///    "Picture Quality" functional test
///
///  @author Dmitry Netrebenko @date 18.07.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <winsock2.h>
#include <AidLib/CThread/CThread.h>
#include <NWL/Streaming/CSocketStream.h>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include "CTestViewer.h"
#include "SPTypes.h"
#include <AidLib/Loki/Singleton.h>

///  CResolutionClient class, responsible for client part of 
///    "Resolution" functional test
///  Base class - CThread from AidLib
///  Access through singleton
class CQualityClient
	:	public CThread
{
private:
///  Prevents making copies of CQualityClient objects.
	CQualityClient( const CQualityClient& );
	CQualityClient& operator=( const CQualityClient& );

public:
///  Constructor
	CQualityClient();
///  Destructor
	~CQualityClient();

private:
///  Viewer
	boost::shared_ptr<CTestViewer>		m_viewer;
///  Connection stream
	boost::shared_ptr<CSocketStream>	m_stream;
///  Critical section to access frames
	CRITICAL_SECTION					m_section;
///  DC of original bitmap
	SPDC								m_fileDC;
///  Original bitmap
	SPBitmap							m_fileBitmap;
///  Frame's DC
	SPDC								m_frameDC;
///  Frame's bitmap
	SPBitmap							m_frameBitmap;
///  Event for waiting results
	SPHandle							m_event;
///  Delta of color change
	unsigned char						m_delta;
///  Count of invalid pixels
	int									m_invalidPixels;

public:
///  Thread's entry point
///  @param Params - thread's parameters
	virtual void Execute( void* Params );

///  Starts viewer
	void StartViewer();

///  Stores viewer's frame
///  @param hdc - DC of viewer's bitmap
	void ProcessFrame(HDC hdc);

///  Session stop calback
	void SessionStop(ESessionStopReason ReasonCode);

///  Session start callback
	void SessionStart();

private:
///  Checks up test results (picture's quality)
	void CheckTestResults();

///  Calculates delta
///  @param origDepth - original colors
	void CalcDelta( const DWORD origDepth );

///  Load frame from file
	void PrepareOriginalFrame();

};

/// Should be used to CQualityClient as single instance
#define QUALITYCLIENT_INSTANCE Loki::SingletonHolder<CQualityClient, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
