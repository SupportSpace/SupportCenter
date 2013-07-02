/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSettings.cpp
///
///  Implements CSettings class, responsible for settings of frame rate test
///
///  @author Dmitry Netrebenko @date 14.05.2007
///
////////////////////////////////////////////////////////////////////////

#include "CSettings.h"
#include <AidLib/CException/CException.h>

CSettings::CSettings()
	:	m_windowHandle(NULL)
	,	m_windowDC(NULL)
	,	m_windowWidth(0)
	,	m_windowHeight(0)
	,	m_colCount(0)
	,	m_rowCount(0)
	,	m_blockWidth(0)
	,	m_blockHeight(0)
	,	m_repeats(DEFAULT_REPEATS)
	,	m_framesFileName(DEFAULT_FILE)
	,	m_localPort(DEFAULT_PORT)
	,	m_timeout(DEFAULT_TIMEOUT)
	,	m_remoteAddr(DEFAULT_ADDR)
	,	m_remotePort(DEFAULT_PORT)
	,	m_resultEntriesCount(DEFAULT_RESULT)
	,	m_resultFileName(DEFAULT_RESULT_FILE)
	,	m_createFrameBitmaps(true)
	,	m_createControlPoints(false)
	,	m_serverPlayOnly(false)
	,	m_clientAutoStart(false)
	,	m_analyzeEmptySpace(false)
	,	m_displayMode(0)
	,	m_useMultiplexedStream(false)
{
TRY_CATCH
CATCH_THROW()
}

CSettings::~CSettings()
{
TRY_CATCH
CATCH_LOG()
}

void CSettings::SetWindow(HWND hwnd)
{
TRY_CATCH

	m_windowHandle = hwnd;
	m_windowDC = GetWindowDC(m_windowHandle);

	// Get window size
	RECT rect;
	GetWindowRect(m_windowHandle, &rect);
	m_windowWidth = rect.right - rect.left;
	m_windowHeight = rect.bottom - rect.top;

CATCH_THROW()
}

HWND CSettings::GetWindow() const
{
TRY_CATCH

	return m_windowHandle;

CATCH_THROW()
}

HDC CSettings::GetDC() const
{
TRY_CATCH

	return m_windowDC;

CATCH_THROW()
}

void CSettings::SetWindowWidth(const int width)
{
TRY_CATCH

	m_windowWidth = width;
	if(m_colCount)
		m_blockWidth = m_windowWidth / m_colCount;

CATCH_THROW()
}

int CSettings::GetWindowWidth() const
{
TRY_CATCH

	return m_windowWidth;

CATCH_THROW()
}

void CSettings::SetWindowHeight(const int height)
{
TRY_CATCH

	m_windowHeight = height;
	if(m_rowCount)
		m_blockHeight = m_windowHeight / m_rowCount;

CATCH_THROW()
}

int CSettings::GetWindowHeight() const
{
TRY_CATCH

	return m_windowHeight;

CATCH_THROW()
}

void CSettings::SetGrid(int colCount, int rowCount)
{
TRY_CATCH

	m_colCount = colCount;
	m_rowCount = rowCount;
	if(m_colCount)
		m_blockWidth = m_windowWidth / m_colCount;
	if(m_rowCount)
		m_blockHeight = m_windowHeight / m_rowCount;

CATCH_THROW()
}

int CSettings::GetColCount() const
{
TRY_CATCH

	return m_colCount;

CATCH_THROW()
}

int CSettings::GetRowCount() const
{
TRY_CATCH

	return m_rowCount;

CATCH_THROW()
}

int CSettings::GetBlockWidth() const
{
TRY_CATCH

	return m_blockWidth;

CATCH_THROW()
}

int CSettings::GetBlockHeight() const
{
TRY_CATCH

	return m_blockHeight;

CATCH_THROW()
}

void CSettings::SetRepeats(int repeats)
{
TRY_CATCH

	m_repeats = repeats;
	if(m_repeats <= 0)
		m_repeats = DEFAULT_REPEATS;

CATCH_THROW()
}

int CSettings::GetRepeats() const
{
TRY_CATCH

	return m_repeats;

CATCH_THROW()
}

void CSettings::SetFramesFileName(const tstring& fileName)
{
TRY_CATCH

	m_framesFileName = fileName;
	if(!m_framesFileName.compare(_T("")))
		m_framesFileName = DEFAULT_FILE;

CATCH_THROW()
}

tstring CSettings::GetFramesFileName() const
{
TRY_CATCH

	return m_framesFileName;

CATCH_THROW()
}

void CSettings::SetLocalPort(const unsigned int port)
{
TRY_CATCH

	m_localPort = port;
	if(m_localPort <= 1000)
		m_localPort = DEFAULT_PORT;

CATCH_THROW()
}

unsigned int CSettings::GetLocalPort() const
{
TRY_CATCH

	return m_localPort;

CATCH_THROW()
}

void CSettings::SetTimeout(const unsigned int timeout)
{
TRY_CATCH

	m_timeout = timeout;

CATCH_THROW()
}

unsigned int CSettings::GetTimeout() const
{
TRY_CATCH

	return m_timeout;

CATCH_THROW()
}

void CSettings::SetRemoteAddr(const tstring& addr)
{
TRY_CATCH

	m_remoteAddr = addr;

CATCH_THROW()
}

tstring CSettings::GetRemoteAddr() const
{
TRY_CATCH

	return m_remoteAddr;

CATCH_THROW()
}

void CSettings::SetRemotePort(const unsigned int port)
{
TRY_CATCH

	m_remotePort = port;

CATCH_THROW()
}

unsigned int CSettings::GetRemotePort() const
{
TRY_CATCH

	return m_remotePort;

CATCH_THROW()
}

void CSettings::SetResultEntriesCount(const int count)
{
TRY_CATCH

	m_resultEntriesCount = count;

CATCH_THROW()
}

int CSettings::GetResultEntriesCount() const
{
TRY_CATCH

	return m_resultEntriesCount;

CATCH_THROW()
}

void CSettings::SetResultsFileName(const tstring& name)
{
TRY_CATCH

	m_resultFileName = name;

CATCH_THROW()
}

tstring CSettings::GetResultsFileName()
{
TRY_CATCH

	return m_resultFileName;

CATCH_THROW()
}

void CSettings::SetCreateFrameBitmaps(const bool enabled)
{
TRY_CATCH

	m_createFrameBitmaps = enabled;

CATCH_THROW()
}

bool CSettings::GetCreateFrameBitmaps() const
{
TRY_CATCH

	return m_createFrameBitmaps;

CATCH_THROW()
}

void CSettings::SetCreateControlPoints(const bool enabled)
{
TRY_CATCH

	m_createControlPoints = enabled;

CATCH_THROW()
}

bool CSettings::GetCreateControlPoints() const
{
TRY_CATCH

	return m_createControlPoints;

CATCH_THROW()
}

void CSettings::SetServerPlayOnly(const bool playOnly)
{
TRY_CATCH

	m_serverPlayOnly = playOnly;

CATCH_THROW()
}

bool CSettings::GetServerPlayOnly() const
{
TRY_CATCH

	return m_serverPlayOnly;

CATCH_THROW()
}

void CSettings::SetClientAutoStart(const bool autoStart)
{
TRY_CATCH

	m_clientAutoStart = autoStart;

CATCH_THROW()
}

bool CSettings::GetClientAutoStart() const
{
TRY_CATCH

	return m_clientAutoStart;

CATCH_THROW()
}

void CSettings::SetAnalyzeEmptySpace(const bool analyze)
{
TRY_CATCH

	m_analyzeEmptySpace = analyze;

CATCH_THROW()
}

bool CSettings::GetAnalyzeEmptySpace() const
{
TRY_CATCH

	return m_analyzeEmptySpace;

CATCH_THROW()
}

void CSettings::SetDisplayMode(const int mode)
{
TRY_CATCH

	m_displayMode = mode;
	if((m_displayMode > 2) || (m_displayMode < 0))
		m_displayMode = 0;

CATCH_THROW()
}

int CSettings::GetDisplayMode() const
{
TRY_CATCH

	return m_displayMode;

CATCH_THROW()
}

void CSettings::SetMultiplexMode( bool useMultiplexedStream)
{
TRY_CATCH

	m_useMultiplexedStream = useMultiplexedStream;

CATCH_THROW()
}

bool CSettings::GetMultiplexMode() const
{
TRY_CATCH

	return m_useMultiplexedStream;

CATCH_THROW()
}
