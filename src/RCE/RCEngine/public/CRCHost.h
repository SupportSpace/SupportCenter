/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRCHost.h
///
///  desctop capturing interface
///
///  @author "Archer Software" Sogin M. @date 20.09.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include "Streaming/CShadowedStream.h"
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <memory>
#include <set>
#include <list>
#include <RCEngine/CActivityMonitor.h>

/// Session modes enumeration
typedef enum _ESessionMode
{
	VIEW_ONLY=0,			/// Prevents input from the VCViewer from being injected into the local Windows
	VISUAL_POINTER		/// Causes mouse movement input from VCViewer to control a visual pointer (similar to a real mouse).  The visual pointer will have a different shape than the real mouse pointer, and it will not be able to control applications.  It will, however, have visual indications of remote Left and Right clicks.
						/// VISUAL-POINTER mode can only be enabled if  VIEW-ONLY mode is also enabled.

} ESessionMode;

class vncServer;
class vncClient;

/// Used to mark messages from VNCServer, to prevent them from ProtectedWindow
#define MESS_MARKER 12345

/// outer interface for desctop capturing
class CRCHost : public CInstanceTracker
{
	friend class vncServer;
	friend class vncClientThread;
private:
	/// Clients activity monitor
	CActivityMonitor m_activityMonitor;

	/// static variable to shure, that global vnc native variables inited
	static bool globalStuffInited;

	/// vncServer instance
	std::auto_ptr<vncServer> m_vncServer;

	/// Handle of protected window. NULL if there isn't protected window
	HANDLE m_hProtectedWindow;
	/// Set of threads to protect - retrived by protected window handle
	std::set<DWORD> m_protectedThreads;
	/// Handle of mouse hook for the protected window
	std::list<boost::shared_ptr<boost::remove_pointer<HANDLE>::type> > m_mouseHooks;

	/// Mouse hook fn
	static LRESULT MouseHook(int nCode, WPARAM wParam, LPARAM lParam);

	/// since shadow stream looks like another client to RCHost
	/// we have to store its id
	/// -1 if no shadow stream set
	int m_shadowStreamClientId;

protected:
	/// Returns stream for certain client
	/// if client not found, exception is thrown
	/// @param clientId id of client
	boost::shared_ptr<CAbstractStream> CRCHost::GetClientStream(const int clientId);

	/// Removes all threads, not related to protected windows
	/// Relation is defined by some descendant class
	/// in base class no filtering is performed
	/// Could be used to protect only specific windows within process - for example some
	/// IE tab page
	virtual void FilterProtectedThreads(std::set<DWORD> &protectedThreads) {};

public:
	/// initializes object instance
	CRCHost();

	/// dtor
	virtual ~CRCHost();

	/// Adds a stream to the RCHost, assigns a priority to it and immediately start its session.
	/// The priority is used to resolve conflicts between streams (such as conflicting keyboard/mouse operations).
	/// The caller may use this method to attach multiple streams to a single RCHost object.
	/// @param stream preinitialized stream
	/// @param priority client input priority
	/// @return This method allocates and returns a unique client ID.
	int StartClient(boost::shared_ptr<CAbstractStream> stream, const int priority);

	/// Stops a client and removes it from the RCHost
	/// @param client clientId client id
	/// @remark on failure exception thrown
	void StopClient(const int clientId);

	/// Set a protected window (referenced by Window handle) which cannot be accessed by the remote user.
	/// Can be called before or after Start().
	/// Can be called several times to set multiple protected windows.
	/// @param hWnd - handle of a window to protect
	/// @remark on failure exception thrown
	void SetProtectedWindow(HANDLE hWnd);

	/// The same as set proteced window but for process id
	/// @see SetProtectedWindow
	void SetProtectedProcess(const DWORD PID);

	/// Returns the handle of protected window NULL if no
	/// @seealso SetProtectedWindow
	/// @return the handle of protected window NULL if no
	HANDLE GetProtectedWindow() const
	{
		return m_hProtectedWindow;
	}

	/// Toggle the boolean State of a specified session Mode.
	/// Can be invoked before or after Start() has been invoked.
	/// @param mode mode to toggle state
	/// @param state new state for mode
	/// @param clientId client id to set mode
	/// @remark on failure exception thrown
	void SetSessionMode(const int clientId, const ESessionMode mode, const bool state);
	
	/// return the boolean State of a specified session Mode.
	/// Can be invoked before or after Start() has been invoked.
	/// @param mode mode to return state
	/// @param clientId client id to set mode
	/// @remark on failure exception thrown
	/// @return the boolean State of a specified session Mode
	bool GetSessionMode(const int clientId, const ESessionMode mode);

	/// Toggle the boolean State of a specified session Mode.
	/// Can be invoked before or after Start() has been invoked.
	/// @param mode mode to toggle state
	/// @param state new state for mode
	/// @param client client to set mode
	/// @remark on failure exception thrown
	void SetSessionMode(vncClient* client, const ESessionMode mode, const bool state);

	/// A virtual method that notifies session has started.
	/// @param clientId corresponding cliend identifier
	virtual void NotifySessionStarted(const int clientId) {};

	/// A virtual method that notifies session has stopped and why 
	/// @param ReasonCode stop reason (Stop() called; Remote Stop() called; Stream signaled end-of-file; etc.)
	/// @param clientId corresponding cliend identifier
	virtual void NotifySessionStopped(const int clientId, ESessionStopReason ReasonCode) {};

	/// returns true is viewOnly mode is turened on
	/// @param clientId client id to get viewOnly mode flag
	/// @return true is viewOnly mode is turened on
	bool ViewOnly(const int clientId) const;

	/// returns true if visualPointer is turned on
	/// @param clientId client id to get VisualPointer flag
	/// @return true if visualPointer is turned on
	bool VisualPointer(const int clientId) const;

	/// Hides visual pointer
	/// For example when session is stopped
	void HideVisualPointer();

	/// Shows visual pointer if visual pointer mode is turned on
	/// For example when session is stopped
	void ShowVisualPointer();

	/// Set a shadow stream, which is used for session recording.
	/// SetShadowStream() can be called at any point before or after Start() has been invoked. 
	/// It can also be provided with a NULL stream, which signifies recording should stop.
	/// RCHost must make sure that the recorded stream can be played back by RCViewer, 
	/// regardless of the state in which recording started / ended.
	/// For example, all dynamically negotiated session parameters that affect parsing of 
	/// VNC traffic should always be recorded in an appropriate header.
	/// @param stream shadow stream to set. if NULL then no shadowing will performed
	/// @remark stream object deleting lay on method caller
	void SetShadowStream(boost::shared_ptr<CAbstractStream> stream);

	/// Turn on or off alpha blended and layered windows capturing
	/// @param captureAlplhaBlend set this to true if to turn alpha blend capturing on, false otherway
	/// @remark alphablended and layered windows capturing can decrease performance dramatically
	/// so by default it's turned off, and be started only manually as option
	void SetCaptureAlphaBlend(bool captureAlplhaBlend);

	/// Returns connected clients count
	unsigned int GetClientsCount();

	/// Returns true if alpha blended and layered windows capturing is turned on
	/// Return true if alpha blended and layered windows capturing is turned on
	/// @see SetCaptureAlphaBlend
	bool GetCaptureAlphaBlend();

	/// A virtual method that notifies session has broke.
	virtual void NotifySessionBroke() {};
	/// A virtual method that notifies session has restored.
	virtual void NotifySessionRestored() {};
	
	/// @modified Alexander Novak @date 05.11.2007 Added the method to support layered windows clipping
	/// Hide window with layered style
	/// @param hwnd			Window's handle
	/// @param showWindow	If it's true then window will be hidden, false otherwise
	void HideLayeredWindow(HWND hwnd, bool showWindow=false);

	/// Returns client activity monitor
	inline CActivityMonitor& GetActivityMonitor()
	{
		return m_activityMonitor;
	}
};