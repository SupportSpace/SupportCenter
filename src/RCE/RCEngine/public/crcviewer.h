/////////////////////////////////////////////////////////////////////////
///
///  RCViewer.h
///
///  <TODO: insert file description here>
///
///  @remarks <TODO: insert remarks here>
///
///  @author Dmiry S. Golub @date 9/27/2006
///
////////////////////////////////////////////////////////////////////////

#pragma once
#include "Streaming/CShadowedStream.h"
#include <memory>
#include <windows.h>
#include "CRCHost.h"
#include "boost/shared_ptr.hpp"

#define INC_COLORS_DELAY 180000 /*180 sec. delay between raizing colors count in automatic mode*/
#define DEC_COLORS_DELAY 30000 /*30 sec. delay between lowering colors count in automatic mode*/

extern void (*frameRateTestCallback)(HDC hdc);

class VNCviewerApp32;
class CRCViewer;
class CVSocketStream;
class ClientConnection;

typedef enum EDisplayMode_
{
   SCALE_MODE=0,
   SCROLL_MODE,
   FULLSCREEN_MODE
} EDisplayMode;


#define WM_RECEIVE_SM WM_USER + 139
#define WM_UPDTHREAD_FINISHED WM_USER+140
#define WM_CHANGE_DISPLAY_MODE WM_USER+141
#define SESSION_START_TIMEOUT 20000

/// Viewer options set
typedef struct _SViewerOptions
{
	/// Use or not colors count value
	bool m_autoColors;
	/// Colors count
	/// 0 : Full colors
	/// 1 : 256 colors
	/// 2 : 64 colors
	/// 3 : 8 colors
	/// 4 : 8 Grey colors
	/// 5 : 4 colors
	/// 6 : 2 Grey colors
	int m_colorsCount;
	/// Preferred encoding
	int m_PreferredEncoding;
	/// Use custom zip/tight compression level
	bool m_useCompressLevel;
	/// Custom zip/tight compression level
	int m_compressLevel;
	/// Use custom jpeg compression
	bool m_enableJpegCompression;
	/// Custom jpeg compression
	int m_jpegQualityLevel;
} SViewerOptions;

struct rfbServerInitMsg;
union rfbClientToServerMsg;

/// outer interface for desctop capturing
class CRCViewer : CInstanceTracker
{
	friend class ClientConnection;
private:
	/// true, if mode from viewer side was selected
	bool m_modeFromViewerSideSelected;
	///static variable to shure, that global vnc native variables inited
	static bool globalStuffInited;
	/// vncViewer instance
	std::auto_ptr<VNCviewerApp32> m_vncViewer;

	/// Waits (blocks) for session start
	void WaitForBeginSession(); 
	static DWORD WaitForStopEvent(HANDLE,DWORD);
	/// internal window message notifiyng viewer to set view only mode
	static unsigned int m_msgSetViewOnly;
	/// internal window message notifiyng viewer to set visual pointer mode
	static unsigned int m_msgSetVisualPointer;
	/// internal window message notifiyng viewer to turn alpha blend capturing
	static unsigned int m_msgSetAlphaBlend;
	/// Server init msg
	std::auto_ptr<rfbServerInitMsg> m_serverInitMsg;
	std::auto_ptr<rfbClientToServerMsg> m_pixelFormat;
	/// Server display name
	TCHAR m_displayName[MAX_PATH];

	/// Update session mode from host handler
	/// @param mode mode to toggle state
	/// @param state new state for mode
	/// @remark on failure exception thrown
	void UpdateSessionMode(const ESessionMode mode, const bool state);
	/// Called from ClientConnection to setup rerverInitMsg
	/// @param msg server init msg
	void SetServerInitMsg(const rfbServerInitMsg *msg);
	/// Called from ClientConnection to setup server display name
	void SetDisplayName(const TCHAR* displayName);	
	/// Called from ClientConnection to set current pixel format
	/// @param msg Pixel formatm message
	void SetPixelFormat(const rfbClientToServerMsg* msg);

protected:
	/// Main stream for captrured data
	boost::shared_ptr<CShadowedStream> m_stream;
	/// auto detect options flag
	bool m_autoDetectOptions;
	/// custom options set
	SViewerOptions m_customOptions;
	/// View only state
	bool m_viewOnly;
	/// Visual pointer state
	bool m_visualPointer;
	/// Capture alpha blended and layered windows. false by default
	bool m_captureAlphaBlend;
	/// Hide wallpaper on host
	bool m_hideWallpaper;
	HWND m_hDesctop;	
	HWND m_hDesctopKeeper;
	EDisplayMode		m_displayMode;
	ESessionStopReason	m_stopReason;
	/// Criticals section for sync write operations
	CRITICAL_SECTION m_cs1;
	CRITICAL_SECTION m_cs2;
	/// true if client thread is killed
	bool m_klientThreadKilled;

	void SetDesctopHandle( HWND );

	/// Horisontal ScrollBar handle
	HWND m_hScrollBar;
	/// Vertical ScrollBar handle
	HWND m_vScrollBar;
public:
	/// initializes object instance
	/// @param stream reference to main stream
	CRCViewer(boost::shared_ptr<CAbstractStream> stream , HWND window );
	/// dtor
	virtual ~CRCViewer();

	/// Set custom options set for viewer
	/// @param customOptions options set
	virtual void SetCustomOptions(const SViewerOptions &customOptions);

	/// Start the remote control mechanism
	/// @remark main stream must be in active state @see m_stream
	/// otherwise exception will be thrown
	/// @remark method is nonblocking
	/// @remark on failure exception thrown
	virtual void Start();
	/// Stop the remote control mechanism
	/// @remark on failure exception thrown
	virtual void Stop();

	/// Set a shadow stream, which is used for session recording.
	/// can be called at any point before or after Start() has been invoked. 
	/// It can also be provided with a NULL stream, which signifies recording should stop.
	/// @param stream shadow stream to setup
	/// @remark stream object deletig lay on method caller
	/// @remark on failure exception thrown
	void SetShadowStream(boost::shared_ptr<CAbstractStream> stream);

	/// A virtual method that notifies session has started.
	virtual void NotifySessionStarted() {};
	/// A virtual method that notifies session has stopped and why 
	/// @param ReasonCode stop reason (Stop() called; Remote Stop() called; Stream signaled end-of-file; etc.)
	virtual void NotifySessionStopped(ESessionStopReason ReasonCode) {};

	/// Session mode changed callback
	/// @param mode mode to toggle state
	/// @param state new state for mode
	/// @remark on failure exception thrown
	virtual void NotifyModeChanged(const ESessionMode mode, const bool state) {};

	/// Initiate a redraw of the remote desktop.
	virtual void RedrawWindow();

	/// Toggle the boolean State of a specified session Mode.
	/// Can be invoked before or after Start() has been invoked.
	/// @param mode mode to toggle state
	/// @param state new state for mode
	/// @remark on failure exception thrown
	void SetSessionMode(const ESessionMode mode, const bool state);

	/// Toggle Display mode between Window Scaling, Window Scrolling and Full Screen.
	/// Can be invoked before Start() or during an active session.
	/// Window Scale Mode applies image scaling to the image of the remote desktop, so that it will fit the current window size (by stretching or shrinking it).
	/// Window Scrolling Mode displays the remote desktop as-is (1:1 pixel size).  If the remote desktop is larger than the window it is displayed in, horizontal and/or vertical scroll bars are added.
	/// Full Screen mode is identical to Window Scaling mode, but uses a full screen
	/// @param mode new display mode
	void SetDisplayMode(const EDisplayMode mode);

	/// Returns current display mode
	EDisplayMode GetDisplayMode();

	/// A virtual method that called after session is started, 
	/// or notifies remote desktop size is changed. Size is real remote desktop size.
	/// @param width new remote desktop width
	/// @param height new remote desktop height
	virtual void SetRemoteDesktopSize(const int width, const int height){};

	/// Turn on or off alpha blended and layered windows capturing
	/// @param captureAlplhaBlend set this to true if to turn alpha blend capturing on, false otherway
	/// @remark alphablended and layered windows capturing can decrease performance dramatically
	/// so by default it's turned off, and be started only manually as option
	/// @remark this will affect all clients, connected to host
	void SetCaptureAlphaBlend(bool captureAlplhaBlend);

	/// Turn on or off hiding wallpaper on host
	void SetHideWallpaper(const bool hideWallpaper);

	/// Send ctrl+alt+del sequence
	void SendCtrlAltDel();

	/// Set external vertical and horisontal scrollbars for SCROLL_MODE
	/// can be called only after session has started
	void SetScrollBars(HWND horScrollBar, HWND vertScrollBar);

	/// Called on titlebar minimize button click
	virtual void OnMinimize();

	/// Called on titlebar restore button click
	virtual void OnRestore();

	/// Restores previous display mode
	void RestorePrevDisplayMode();

	/// returns write critical section
	/// @return write critical section
	CRITICAL_SECTION* GetCS1()
	{
	TRY_CATCH
		return &m_cs1;
	CATCH_THROW("CRCViewer::GetCS1")
	}

	/// A virtual method that notifies session has broke.
	virtual void NotifySessionBroke() {};
	/// A virtual method that notifies session has restored.
	virtual void NotifySessionRestored() {};
	/// A virtual method that notifies display mode has changed
	/// @param mode new display mode
	virtual void NorifyDisplayModeChanged(EDisplayMode mode) {};
};