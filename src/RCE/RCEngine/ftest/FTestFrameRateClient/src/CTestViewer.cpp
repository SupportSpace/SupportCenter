/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTestViewer.cpp
///
///  Implements CTestViewer class, responsible for derived class from CRCViewer
///
///  @author Dmitry Netrebenko @date 16.05.2007
///
////////////////////////////////////////////////////////////////////////

#include "CTestViewer.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "CFrameRateClient.h"

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
	FRAMERATECLIENT_INSTANCE.SessionStart();

CATCH_THROW()
}

void CTestViewer::NotifySessionStopped(ESessionStopReason ReasonCode)
{
TRY_CATCH

	/// Stops session
	FRAMERATECLIENT_INSTANCE.SessionStop();

CATCH_THROW()
}

