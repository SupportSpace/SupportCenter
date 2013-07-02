/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTestViewer.cpp
///
///  Implements CTestViewer class, responsible for derived class from CRCViewer
///
///  @author Dmitry Netrebenko @date 18.07.2007
///
////////////////////////////////////////////////////////////////////////

#include "CTestViewer.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "CQualityClient.h"

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
	QUALITYCLIENT_INSTANCE.SessionStart();

CATCH_THROW()
}

void CTestViewer::NotifySessionStopped(ESessionStopReason ReasonCode)
{
TRY_CATCH

	/// Stops session
	QUALITYCLIENT_INSTANCE.SessionStop(ReasonCode);

CATCH_THROW()
}
