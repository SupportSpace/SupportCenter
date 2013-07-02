/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSettings.cpp
///
///  Implements CSettings class, responsible for settings
///
///  @author Dmitry Netrebenko @date 07.06.2007
///
////////////////////////////////////////////////////////////////////////

#include "CSettings.h"
#include <AidLib/CException/CException.h>

CSettings::CSettings()
	:	m_windowHandle(NULL)
	,	m_windowDC(NULL)
	,	m_windowWidth(0)
	,	m_windowHeight(0)
	,	m_colCount(DEFAULT_COLS)
	,	m_rowCount(DEFAULT_ROWS)
	,	m_localPort(DEFAULT_PORT)
	,	m_timeout(DEFAULT_TIMEOUT)
	,	m_remoteAddr(DEFAULT_ADDR)
	,	m_remotePort(DEFAULT_PORT)
	,	m_clientAutoStart(false)
	,	m_displayMode(0)
	,	m_testTime(DEFAULT_TEST_TIME)
	,	m_useGarbageThread(false)
	,	m_sendGarbage(false)
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

CATCH_THROW()
}

int CSettings::GetWindowHeight() const
{
TRY_CATCH

	return m_windowHeight;

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

void CSettings::SetTestTime(const DWORD testTime)
{
TRY_CATCH

	m_testTime = testTime;

CATCH_THROW()
}

DWORD CSettings::GetTestTime() const
{
TRY_CATCH

	return m_testTime;

CATCH_THROW()
}

void CSettings::SetUseGarbageThread(const bool useThread)
{
TRY_CATCH

	m_useGarbageThread = useThread;

CATCH_THROW()
}

bool CSettings::GetUseGarbageThread() const
{
TRY_CATCH

	return m_useGarbageThread;

CATCH_THROW()
}

void CSettings::SetSendGarbage(const bool sendGarbage)
{
TRY_CATCH

	m_sendGarbage = sendGarbage;

CATCH_THROW()
}

bool CSettings::GetSendGarbage() const
{
TRY_CATCH

	return m_sendGarbage;

CATCH_THROW()
}

