/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSettings.h
///
///  Declares CSettings class, responsible for settings of frame rate test
///
///  @author Dmitry Netrebenko @date 14.05.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <AidLib/Strings/tstring.h>
#include <AidLib/Loki/Singleton.h>

#define DEFAULT_SUBSTREAM_ID		1
#define DEFAULT_SUBSTREAM_PRIORITY	1
#define DEFAULT_REPEATS				1
#define DEFAULT_REPEATS				1
#define DEFAULT_FILE				_T("frames.dat")
#define DEFAULT_PORT				3000
#define DEFAULT_TIMEOUT				30000
#define DEFAULT_ADDR				_T("127.0.0.1")
#define DEFAULT_RESULT				1048576
#define DEFAULT_RESULT_FILE			_T("results.csv")

///  CSettings class, responsible for settings of frame rate test
///  Access through CSingleton
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
///  Count of colunms in the grid
	int				m_colCount;
///  Count of rows in the grid
	int				m_rowCount;
///  Width of one block
	int				m_blockWidth;
///  Height of one block
	int				m_blockHeight;
///  Count of repeats
	int				m_repeats;
///  Name of the file with frames
	tstring			m_framesFileName;
///  Local port for CSocketStream
	unsigned int	m_localPort;
///  Connect timeout
	unsigned int	m_timeout;
///  Remote address
	tstring			m_remoteAddr;
///  Remote port
	unsigned int	m_remotePort;
///  Number of stored results
	int				m_resultEntriesCount;
///  Name of file to store results
	tstring			m_resultFileName;
///  Is creation bitmaps enabled
	bool			m_createFrameBitmaps;
///  Is creation control points enabled
	bool			m_createControlPoints;
///  Is "play only" mode
	bool			m_serverPlayOnly;
///  Starting viewer at application starting
	bool			m_clientAutoStart;
///  Is client analyses empty area
	bool			m_analyzeEmptySpace;
///  Client display mode
	int				m_displayMode;
///  Multiplexed stream usage
	bool			m_useMultiplexedStream;

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

///  Sets test grid resolution
	void SetGrid( int colCount, int rowCount );

///  Returns count of columns in the grid
	int GetColCount() const;

///  Returns count of rows in the grid
	int GetRowCount() const;

///  Returns width of the one block
	int GetBlockWidth() const;

///  Returns height of the one block
	int GetBlockHeight() const;

///  Sets count of test repeats
	void SetRepeats( int repeats );

///  Returns count of test repeats
	int GetRepeats() const;

///  Sets name of the file with frames
	void SetFramesFileName( const tstring& fileName );

///  Returns name of file with frames
	tstring GetFramesFileName() const;

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

///  Sets number of result entries
///  @param count - new number of entries
	void SetResultEntriesCount( const int count );

///  Returns number of result entries
	int GetResultEntriesCount() const;

///  Sets file name to store results
///  @param name - new file name
	void SetResultsFileName( const tstring& name );

///  Returns file name to store test results
	tstring GetResultsFileName();

///  Enables creation frame bitmaps
	void SetCreateFrameBitmaps( const bool enabled );

///  Get current state of the flag
	bool GetCreateFrameBitmaps() const;

///  Enables creation control points for frames
	void SetCreateControlPoints( const bool enabled );

///  Get current state of the flag
	bool GetCreateControlPoints() const;

///  Sets "play only" mode for server
	void SetServerPlayOnly( const bool playOnly );

///  Get "play only" server's mode
	bool GetServerPlayOnly() const;

///  Set "auto start" mode for client
	void SetClientAutoStart( const bool autoStart );

///  Get client's "auto start mode"
	bool GetClientAutoStart() const;

///  Enables analyses empty spaces
	void SetAnalyzeEmptySpace( const bool analyze );

///  Get current state of of the flag
	bool GetAnalyzeEmptySpace() const;

///  Sets new display mode for client
///  @param mode - new display mode
	void SetDisplayMode( const int mode );

///  Returns current display mode
	int GetDisplayMode() const;
	
///  Set if multiplex stream will be used
///  @param useMultiplexStream		use multiplex stream as transport layer
	void SetMultiplexMode( bool useMultiplexedStream);

///  Get if multiplex stream will be used as transport layer
	bool GetMultiplexMode() const;

};

/// Should be used to CSettings as single instance
#define SETTINGS_INSTANCE Loki::SingletonHolder<CSettings, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
