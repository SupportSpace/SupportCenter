/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTestViewer.cpp
///
///  Implements CTestViewer class, responsible for derived class from CRCViewer
///
///  @author Dmitry Netrebenko @date 13.06.2007
///
////////////////////////////////////////////////////////////////////////
#include "CResolutionClient.h"
#include "CTestViewer.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CSingleton/CSingleton.h>

CTestViewer::CTestViewer(boost::shared_ptr<CAbstractStream> stream, HWND hWnd)
	:	CRCViewer(stream, hWnd)
{
TRY_CATCH
CATCH_THROW()
}

CTestViewer::~CTestViewer()
{
TRY_CATCH
CATCH_LOG()
}

void CTestViewer::NotifySessionStarted()
{
TRY_CATCH

	/// Starts session
	RESOLUTIONCLIENT_INSTANCE.SessionStart();

CATCH_THROW()
}

void CTestViewer::NotifySessionStopped(ESessionStopReason ReasonCode)
{
TRY_CATCH

	/// Stops session
	RESOLUTIONCLIENT_INSTANCE.SessionStop(ReasonCode);

CATCH_THROW()
}

void CTestViewer::SetRemoteDesktopSize(const int width, const int height)
{
TRY_CATCH

	/// Change resolution
	RESOLUTIONCLIENT_INSTANCE.OnResolutionChanged(width, height);

CATCH_THROW()
}

