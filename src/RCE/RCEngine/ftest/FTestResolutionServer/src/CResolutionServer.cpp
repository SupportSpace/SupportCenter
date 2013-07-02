/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CResolutionServer.cpp
///
///  Implements CResolutionServer class, responsible for server part of 
///    "Resolution" functional test
///
///  @author Dmitry Netrebenko @date 11.06.2007
///
////////////////////////////////////////////////////////////////////////

#include "CResolutionServer.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "CSettings.h"
#include <AidLib/CCritSection/CCritSection.h>
#include "CHelper.h"
#include "CResolutionManager.h"

CResolutionServer::CResolutionServer()
	:	CThread()
	,	CRCHost()
{
TRY_CATCH

	/// Create stream
	m_stream.reset(new CSocketStream());

	/// Set OnConnected event handler
	m_stream->SetConnectedEvent(boost::bind(&CResolutionServer::ClientConnected, this, _1));
	
	/// Create critical section
	InitializeCriticalSection(&m_section);

	/// Create event objects
	m_connectEvent.reset( CreateEvent(NULL, TRUE, FALSE, NULL), CloseHandle );
	m_sessionEvent.reset( CreateEvent(NULL, TRUE, FALSE, NULL), CloseHandle );

CATCH_THROW()
}

CResolutionServer::~CResolutionServer()
{
TRY_CATCH

	/// Terminate thread
	Terminate();
	/// Delete critical section
	DeleteCriticalSection(&m_section);

CATCH_LOG()
}

void CResolutionServer::Execute(void* Params)
{
	HWND hWnd = NULL;

TRY_CATCH

	CoInitialize(NULL);

	/// Init frames
	m_firstFrame.reset(new CFrame());
	m_firstFrame->Init(true);
	m_secondFrame.reset(new CFrame());
	m_secondFrame->Init(false);

	/// Get settings
	hWnd = SETTINGS_INSTANCE.GetWindow();
	unsigned int port = SETTINGS_INSTANCE.GetLocalPort();
	unsigned int timeout = SETTINGS_INSTANCE.GetTimeout();
	int clientId = 0;

	ResetEvent(m_connectEvent.get());

	/// Accept stream
	m_stream->Accept(port);

	/// Check for timeout
	if(WAIT_TIMEOUT == WaitForSingleObject(m_connectEvent.get(), timeout))
		throw MCException(_T("Connect timeout expired"));

	if(!Terminated())
	{
		/// Start client
		clientId = CRCHost::StartClient(m_stream, 0);

		/// Wait for session starting
		ResetEvent(m_sessionEvent.get());
		if(WAIT_TIMEOUT == WaitForSingleObject(m_sessionEvent.get(), timeout))
			throw MCException(_T("Session start timeout expired"));
	}

	/// Get current time as time for next frame
	DWORD nextFrameTime = timeGetTime();
	DWORD currentTime = 0;
	DWORD exitTime = timeGetTime() + SETTINGS_INSTANCE.GetTestTime();
	DWORD resolutionTime = timeGetTime() + CHelper::GetRandom(5000, 10000);

	while(!Terminated())
	{
		currentTime = timeGetTime();
		if(currentTime > exitTime)
			break;
		if(currentTime > nextFrameTime)
		{
			/// Select next frame
			CCritSection section(&m_section);
			if(m_currentFrame.get() == m_firstFrame.get())
				m_currentFrame = m_secondFrame;
			else
				m_currentFrame = m_firstFrame;
			InvalidateRect(hWnd, NULL, FALSE);
			nextFrameTime += 500;
		}
		if(currentTime > resolutionTime)
		{
			ChangeResolution();
			resolutionTime = timeGetTime() + CHelper::GetRandom(3000, 7000);
		}
		Sleep(1);
	}

CATCH_LOG()
	
	/// Pause at finish
	Sleep(1000);

	/// Close window
	if(hWnd)
		SendMessage(hWnd, WM_DESTROY, 0, 0);

}

void CResolutionServer::ClientConnected(void*)
{
TRY_CATCH

	SetEvent(m_connectEvent.get());

CATCH_THROW()
}

void CResolutionServer::NotifySessionStarted(const int clientId)
{
TRY_CATCH

	SetEvent(m_sessionEvent.get());

CATCH_THROW()
}

void CResolutionServer::DrawFrame()
{
TRY_CATCH

	CCritSection section(&m_section);

	/// Draw current frame
	if(m_currentFrame.get())
		m_currentFrame->Draw();

CATCH_THROW()
}

void CResolutionServer::ChangeResolution()
{
TRY_CATCH

	SPDevMode mode = RESOLUTIONMANAGER_INSTANCE.GetNextMode();
	if(!mode.get())
		return;

	mode->dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY;
	LONG res = ChangeDisplaySettings(mode.get(),CDS_UPDATEREGISTRY);
	if(DISP_CHANGE_SUCCESSFUL != res)
	{
		Log.Add(_WARNING_, _T("Error at changing resolution to %d x %d x %d. Error code: %d"), 
			mode->dmPelsWidth, mode->dmPelsHeight, mode->dmBitsPerPel, res);
		return;
	}

	Log.Add(_MESSAGE_, _T("New screen resolution: %d x %d x %d"), mode->dmPelsWidth, mode->dmPelsHeight, mode->dmBitsPerPel);


	{
		CCritSection section(&m_section);

		SETTINGS_INSTANCE.SetWindowWidth(mode->dmPelsWidth);
		SETTINGS_INSTANCE.SetWindowHeight(mode->dmPelsHeight);

		m_firstFrame->Init(true);
		m_secondFrame->Init(false);
	}

	/// Update window
	HWND hWnd = SETTINGS_INSTANCE.GetWindow();
	SetWindowPos(
		hWnd, 
		NULL, 
		0, 
		0, 
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOZORDER );

CATCH_THROW()
}

