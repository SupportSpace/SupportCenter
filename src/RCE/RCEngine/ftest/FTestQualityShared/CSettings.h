/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSettings.h
///
///  Declares CSettings class, responsible for settings
///
///  @author Dmitry Netrebenko @date 18.07.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <AidLib/Strings/tstring.h>
#include <AidLib/Loki/Singleton.h>

#define DEFAULT_PORT		3000
#define DEFAULT_TIMEOUT		30000
#define DEFAULT_ADDR		_T("127.0.0.1")
#define DEFAULT_TEST_TIME	15000
#define DEFAULT_PERCENTAGE	1
#define DEFAULT_DEPTH		0
#define DEFAULT_IMAGE_FILE	_T("frame.bmp")

///  CSettings class, responsible for settings
///  Access through singleton
class CSettings
{
private:
///  Prevents making copies of CSettings objects.
	CSettings( const CSettings& );
	CSettings& operator=( const CSettings& );

public:
///  Constructor
	CSettings();
///  Destructor
	~CSettings();

private:
///  Handle of frame rate test window
	HWND			m_windowHandle;
///  DC of frame rate test window
	HDC				m_windowDC;
///  Window width
	int				m_windowWidth;
///  Window height
	int				m_windowHeight;
///  Local port for CSocketStream
	unsigned int	m_localPort;
///  Connect timeout
	unsigned int	m_timeout;
///  Remote address
	tstring			m_remoteAddr;
///  Remote port
	unsigned int	m_remotePort;
///  Starting viewer at application starting
	bool			m_clientAutoStart;
///  Client display mode
	int				m_displayMode;
///  Test time
	DWORD			m_testTime;
///  Allowed percentage of invalid pixels
	int				m_invalidPixelPercentage;
///  Viewer's color depth
	int				m_colorDepth;
///  Name of file with frame
	tstring			m_frameFile;

public:
///  Sets window's handle and gets window's properties
///  @param hwnd - new window handle
	void SetWindow( HWND hwnd );

///  Returns handle of the window
	HWND GetWindow() const;

///  Returns DC of test window
	HDC GetDC() const;

///  Sets new window width
///  @param width - new window width
	void SetWindowWidth( const int width );

///  Returns width of the window
	int GetWindowWidth() const;

///  Sets new window height
///  @param height - new window height
	void SetWindowHeight( const int height );

///  Returns height of the window
	int GetWindowHeight() const;

///  Sets new local port
///  @param port - local port
	void SetLocalPort( const unsigned int port );

///  Returns local port
	unsigned int GetLocalPort() const;

///  Sets new connection timeout
///  @param timeout - connect timeout
	void SetTimeout( const unsigned int timeout );

///  Returns connection timeout
	unsigned int GetTimeout() const;

///  Sets remote address
///  @param addr - remote address
	void SetRemoteAddr( const tstring& addr );

///  Returns remote address
	tstring GetRemoteAddr() const;

///  Sets remote port
///  @param port - new remote port
	void SetRemotePort( const unsigned int port );

///  Returns remote port
	unsigned int GetRemotePort() const;

///  Set "auto start" mode for client
	void SetClientAutoStart( const bool autoStart );

///  Get client's "auto start mode"
	bool GetClientAutoStart() const;

///  Sets new display mode for client
///  @param mode - new display mode
	void SetDisplayMode( const int mode );

///  Returns current display mode
	int GetDisplayMode() const;

///  Sets test time
///  @param testTime - new test time
	void SetTestTime( const DWORD testTime );

///  Rerurns test time
	DWORD GetTestTime() const;

///  Sets allowed percentage of invalid pixels
///  @param percent - new percentage
	void SetInvalidPixelPercentage( const int percent );

///  Rerurns allowed percentage of invalid pixels
	int GetInvalidPixelPercentage() const;

///  Sets color depth for viewer
///  @param depth - new depth
	void SetColorDepth( const int depth );

///  Rerurns color depth for viewer
	int GetColorDepth() const;

///  Sets file with frame
///  @param frameFile - name of the file with frame
	void SetFrameFile( const tstring& frameFile );

///  Returns file with frame
	tstring GetFrameFile() const;
};

/// Should be used to CSettings as single instance
#define SETTINGS_INSTANCE Loki::SingletonHolder<CSettings, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
