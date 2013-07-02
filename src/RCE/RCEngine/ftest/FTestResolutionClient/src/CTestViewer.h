/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTestViewer.h
///
///  Declares CTestViewer class, responsible for derived class from CRCViewer
///
///  @author Dmitry Netrebenko @date 13.06.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <RCEngine/RCEngine.h>
#include <RCEngine/CRCViewer.h>
#include <NWL/Streaming/CAbstractStream.h>
#include <boost/shared_ptr.hpp>

///  CTestViewer class, responsible for derived class from CRCViewer
class CTestViewer
	:	public CRCViewer
{
public:
///  Constructor
///  @param stream - connected stream
///  @param hWnd - handle of window
	CTestViewer(boost::shared_ptr<CAbstractStream> stream, HWND hWnd);
///  Destructor
	~CTestViewer();
	/// A virtual method that notifies session has started.
	virtual void NotifySessionStarted();
	/// A virtual method that notifies session has stopped and why 
	/// @param ReasonCode stop reason (Stop() called; Remote Stop() called; Stream signaled end-of-file; etc.)
	virtual void NotifySessionStopped(ESessionStopReason ReasonCode);
	/// A virtual method that called after session is started, 
	/// or notifies remote desktop size is changed. Size is real remote desktop size.
	/// @param width new remote desktop width
	/// @param height new remote desktop height
	virtual void SetRemoteDesktopSize(const int width, const int height);
};
