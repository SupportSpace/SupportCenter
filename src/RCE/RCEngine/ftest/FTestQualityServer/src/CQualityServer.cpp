/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CQualityServer.cpp
///
///  Implements CQualityServer class, responsible for server part of 
///    "Picture Quality" functional test
///
///  @author Dmitry Netrebenko @date 18.07.2007
///
////////////////////////////////////////////////////////////////////////

#include "CQualityServer.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "CSettings.h"
#include <AidLib/CCritSection/CCritSection.h>
#include "CHelper.h"
#include <boost/bind.hpp>

CQualityServer::CQualityServer()
	:	CThread()
	,	CRCHost()
{
TRY_CATCH

	/// Create stream
	m_stream.reset(new CSocketStream());

	/// Set OnConnected event handler
	m_stream->SetConnectedEvent(boost::bind(&CQualityServer::ClientConnected, this, _1));
	
	/// Create critical section
	InitializeCriticalSection(&m_section);

	/// Create event object
	m_event.reset( CreateEvent(NULL, TRUE, FALSE, NULL), CloseHandle );

CATCH_THROW()
}

CQualityServer::~CQualityServer()
{
TRY_CATCH

	/// Terminate thread
	Terminate();
	/// Delete critical section
	DeleteCriticalSection(&m_section);

CATCH_LOG()
}

void CQualityServer::Execute(void* Params)
{
TRY_CATCH

	CoInitialize(NULL);

	HWND hWnd = SETTINGS_INSTANCE.GetWindow();
	unsigned int port = SETTINGS_INSTANCE.GetLocalPort();
	unsigned int timeout = SETTINGS_INSTANCE.GetTimeout();
	int clientId = 0;

	PrepareFrame();

	ResetEvent(m_event.get());

	/// Accept stream
	m_stream->Accept(port);

	/// Check for timeout
	if(WAIT_TIMEOUT == WaitForSingleObject(m_event.get(), timeout))
		throw MCException(_T("Connect timeout expired"));

	if(!Terminated())
	{
		SendScreenInfo();

		/// Start client
		clientId = CRCHost::StartClient(m_stream, 0);

		/// Wait for session starting
		ResetEvent(m_event.get());
		if(WAIT_TIMEOUT == WaitForSingleObject(m_event.get(), timeout))
			throw MCException(_T("Session start timeout expired"));

		InvalidateRect(hWnd, NULL, FALSE);
	}

	return;

CATCH_LOG()

	StopServer();
}

void CQualityServer::NotifySessionStarted(const int clientId)
{
TRY_CATCH

	SetEvent(m_event.get());

CATCH_THROW()
}

void CQualityServer::NotifySessionStopped(const int clientId, ESessionStopReason ReasonCode )
{
TRY_CATCH

	StopServer();

CATCH_THROW()
}

void CQualityServer::DrawFrame()
{
TRY_CATCH

	/// Enter critical section
	CCritSection section(&m_section);

	if(!m_frameDC.get() || !m_frameBitmap.get())
		return;

	int width = SETTINGS_INSTANCE.GetWindowWidth();
	int height = SETTINGS_INSTANCE.GetWindowHeight();
	HDC dc = SETTINGS_INSTANCE.GetDC();

	/// Draw frame
	BitBlt(dc, 0, 0, width, height, m_frameDC.get(), 0, 0, SRCCOPY);

CATCH_THROW()
}

void CQualityServer::ClientConnected(void*)
{
TRY_CATCH

	SetEvent(m_event.get());

CATCH_THROW()
}

void CQualityServer::StopServer()
{
TRY_CATCH

	HWND hWnd = SETTINGS_INSTANCE.GetWindow();
	/// Close window
	if(hWnd)
		SendMessage(hWnd, WM_DESTROY, 0, 0);

CATCH_THROW()
}

void CQualityServer::PrepareFrame()
{
TRY_CATCH

	tstring fileName = SETTINGS_INSTANCE.GetFrameFile();
	Log.Add(_MESSAGE_, _T("Loading frame from %s ..."), fileName.c_str());
	
	SPBitmap fileBitmap((HBITMAP)LoadImage(NULL, fileName.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTCOLOR | LR_DEFAULTSIZE), DeleteObject);
	if(!fileBitmap.get())
		throw MCException_Win(_T("Can not load bitmap: %s"));

	HDC hdc = SETTINGS_INSTANCE.GetDC();

	/// Create DC
	SPDC fileDC(CreateCompatibleDC(hdc), DeleteDC);
	if(!fileDC.get())
		throw MCException_Win(_T("Can not create compatible DC for frame from file"));

	/// Select bitmap
	SelectObject(fileDC.get(), fileBitmap.get());

	/// Get bitmap's size
	BITMAP bitmap;
	memset(&bitmap, 0, sizeof(BITMAP));
	GetObject(fileBitmap.get(), sizeof(BITMAP), &bitmap);

	if(!bitmap.bmWidth || !bitmap.bmHeight)
		throw MCException_Win(_T("Loaded bitmap has invalid size."));

	Log.Add(_MESSAGE_, _T("Loaded bitmap size: %d x %d"), bitmap.bmWidth, bitmap.bmHeight);

	int windowWidth = SETTINGS_INSTANCE.GetWindowWidth();
	int windowHeight = SETTINGS_INSTANCE.GetWindowHeight();

	/// Enter critical section
	CCritSection section(&m_section);

	/// Create DC
	m_frameDC.reset(CreateCompatibleDC(hdc), DeleteDC);
	if(!m_frameDC.get())
		throw MCException_Win(_T("Can not create compatible DC"));

	/// Create bitmap
	m_frameBitmap.reset(CreateCompatibleBitmap(hdc, windowWidth, windowHeight), DeleteObject);
	if(!m_frameBitmap.get())
		throw MCException_Win(_T("Can not create bitmap"));

	/// Select bitmap
	SelectObject(m_frameDC.get(), m_frameBitmap.get());

	/// Copy bitmap
	if(!StretchBlt(m_frameDC.get(), 0, 0, windowWidth, windowHeight, fileDC.get(), 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY)) 
		throw MCException_Win(_T("Can not stretch bitmap"));

CATCH_THROW()
}

void CQualityServer::SendScreenInfo()
{
TRY_CATCH

	if(!m_stream.get())
		throw MCException_Win(_T("Stream doesn't created"));
	
	/// Enter critical section
	CCritSection section(&m_section);

	if(!m_frameDC.get() || !m_frameBitmap.get())
		throw MCException_Win(_T("Frame doesn't created"));


	SPDevMode mode(new DEVMODE());
	mode->dmSize = sizeof(DEVMODE);

	if(!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, mode.get()))
		throw MCException(_T("Can not obtain current display settings"));

	int windowWidth = SETTINGS_INSTANCE.GetWindowWidth();
	int windowHeight = SETTINGS_INSTANCE.GetWindowHeight();

	/// Send frame
	m_stream->Send(reinterpret_cast<char*>(&windowWidth), sizeof(int));
	m_stream->Send(reinterpret_cast<char*>(&windowHeight), sizeof(int));
	m_stream->Send(reinterpret_cast<char*>(&mode->dmBitsPerPel), sizeof(DWORD));

CATCH_THROW()
}
