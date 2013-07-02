/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  RCViewerImpl.h
///
///  IRCViewerImpl,  Implementation of CRCViewer
///
///  @author "Archer Software" Solovyov K. @date 27.11.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include <RCEngine/crcviewer.h>
#include <AidLib/Logging/clog.h>

#ifndef BOOST_SIGNAL_USE_LIB 
	#define BOOST_SIGNALS_USE_LIB
#endif
#ifndef BOOST_SIGNALS_NO_LIB
	#define BOOST_SIGNALS_NO_LIB
#endif
#include <boost/signal.hpp>


/// connection type
typedef enum ECONNECTTYPE
{
	ECT_UNKNOWN=-1,  //Using unknown connection
	ECT_DIRECT=0,    //Using direct connection
	ECT_NAT=1,       //Using NAT
	ECT_RELAY=2      //Using relay
} EConnectType;


class CCoRCViewer;
/// Implementation of CRCViewer class (event - virtual function)
class CRCViewerImpl : public CRCViewer
{	
	/// Proper viewer ActiveX control
	CCoRCViewer *m_owner;

	/// Miniimze viewer signal
	boost::signal<void ()> m_minimizeSignal;

public:
	/// Initializes object instance
	/// @param stream reference to main stream
	/// @param window  container window handle.
	/// @param owner poiner to owner CCoRCViewer object
	CRCViewerImpl(boost::shared_ptr<CAbstractStream> stream,HWND window,CCoRCViewer* owner):CRCViewer(stream,window),m_owner(owner){}
	
	/// Destroys object instance
	virtual ~CRCViewerImpl(void);
	
	/// A virtual method that notifies that session has started.
	/// @remark Fire CCoRCViewer::NotifySessionStart event due to PostMessage() function
	virtual void NotifySessionStarted();
	
	/// A virtual method that notifies that session has stopped and why 
	/// @param ReasonCode stop reason (Stop() called; Remote Stop() called; Stream signaled end-of-file; etc.) LOCAL_STOP=0, REMOTE_STOP=1, STREAM_ERROR=2, PROTOCOL_ERROR=3, CHANGE_DISPLAY_MODE=4,CONNECTING_ERROR=5, OPENFILE_ERROR=6. See _ESessionStopReason definition for last version of reason code.
	/// @remark Fire CCoRCViewer::NotifySessionStop event due to PostMessage() function
	virtual void NotifySessionStopped(ESessionStopReason ReasonCode);
	
	/// A virtual method that notifies display mode has changed
	/// @param mode new display mode
	virtual void NorifyDisplayModeChanged(EDisplayMode mode);

	/// A virtual method that called after session is started, 
	/// or notifies remote desktop size is changed. Size is real remote desktop size.
	/// @param width new remote desktop width
	/// @param height new remote desktop height
	virtual void SetRemoteDesktopSize(const int width, const int height);

	/// Called on titlebar minimize button click
	virtual void OnMinimize();

	/// Called on titlebar restore button click
	virtual void OnRestore();

	/// Add minimize signgal listener delegate
	void SubscribeEventsListener(boost::function<void ()> eventListener);
};
