/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTestViewer.cpp
///
///  Implements CTestViewer class, responsible for derived class from CRCViewer
///
///  @author Dmitry Netrebenko @date 07.06.2007
///
////////////////////////////////////////////////////////////////////////

#include "CTestViewer.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "CNWLGarbageClient.h"

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
	NWLGARGAGECLIENT_INSTANCE.SessionStart();

CATCH_THROW()
}

void CTestViewer::NotifySessionStopped(ESessionStopReason ReasonCode)
{
TRY_CATCH

	/// Stops session
	NWLGARGAGECLIENT_INSTANCE.SessionStop(ReasonCode);

CATCH_THROW()
}

void CTestViewer::NotifySessionBroke()
{
TRY_CATCH

	/// Session broke
	NWLGARGAGECLIENT_INSTANCE.SessionBroke();

CATCH_THROW()
}

void CTestViewer::NotifySessionRestored()
{
TRY_CATCH

	/// Session restored
	NWLGARGAGECLIENT_INSTANCE.SessionRestored();

CATCH_THROW()
}
