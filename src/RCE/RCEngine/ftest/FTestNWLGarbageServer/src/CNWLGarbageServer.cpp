/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNWLGarbageServer.cpp
///
///  Implements CNWLGarbageServer class, responsible for server part of 
///    "NWL Garbage" functional test
///
///  @author Dmitry Netrebenko @date 06.06.2007
///
////////////////////////////////////////////////////////////////////////

#include "CNWLGarbageServer.h"
#include <AidLib/CException/CException.h>
#include "CGarbageStream.h"
#include <boost/bind.hpp>
#include <AidLib/CSingleton/CSingleton.h>
#include "CSettings.h"
#include <AidLib/CCritSection/CCritSection.h>

CNWLGarbageServer::CNWLGarbageServer()
	:	CThread()
	,	CRCHost()
	,	m_sessionActive(false)
	,	m_clientId(0)
{
TRY_CATCH

	/// Create connection stream
	if(SETTINGS_INSTANCE.GetSendGarbage())
	{
		if(SETTINGS_INSTANCE.GetUseGarbageThread())
			m_stream.reset(new CSocketStream());
		else
			m_stream.reset(new CGarbageStream(&m_sessionActive));
//			m_stream.reset(new CSocketStream());
	}
	else
		m_stream.reset(new CSocketStream());

	/// Set OnConnected event handler
	m_stream->SetConnectedEvent(boost::bind(&CNWLGarbageServer::ClientConnected, this, _1));
	
	/// Create critical section
	InitializeCriticalSection(&m_section);

	/// Create event objects
	m_connectEvent.reset( CreateEvent(NULL, TRUE, FALSE, NULL), CloseHandle );
	m_sessionEvent.reset( CreateEvent(NULL, TRUE, FALSE, NULL), CloseHandle );

CATCH_THROW()
}

CNWLGarbageServer::~CNWLGarbageServer()
{
TRY_CATCH

	/// Terminate thread
	Terminate();
	/// Delete critical section
	DeleteCriticalSection(&m_section);

CATCH_LOG()
}

void CNWLGarbageServer::Execute(void* Params)
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
	int windowWidth = SETTINGS_INSTANCE.GetWindowWidth();
	int windowHeight = SETTINGS_INSTANCE.GetWindowHeight();

	RECT rect;
	rect.left = 0;
	rect.right = windowWidth;
	rect.top = 0;
	rect.bottom = windowHeight;
	unsigned int port = SETTINGS_INSTANCE.GetLocalPort();
	unsigned int timeout = SETTINGS_INSTANCE.GetTimeout();
	int clientId = 0;

	ResetEvent(m_connectEvent.get());

	/// Accept stream
	m_stream->Accept(port);

	/// Check for timeout
	if(WAIT_TIMEOUT == WaitForSingleObject(m_connectEvent.get(), timeout))
		throw MCException(_T("Connect timeout expired"));

	/// Send window size
	m_stream->Send(reinterpret_cast<char*>(&windowWidth), sizeof(int));
	m_stream->Send(reinterpret_cast<char*>(&windowHeight), sizeof(int));

	if(!Terminated())
	{
		/// Start client
		clientId = CRCHost::StartClient(m_stream, 0);

		/// Wait for session starting
		ResetEvent(m_sessionEvent.get());
		if(WAIT_TIMEOUT == WaitForSingleObject(m_sessionEvent.get(), timeout))
			throw MCException(_T("Session start timeout expired"));
	}
	if(SETTINGS_INSTANCE.GetUseGarbageThread() && SETTINGS_INSTANCE.GetSendGarbage())
	{
		/// Create garbage thread
		m_garbageThread.reset(new CGarbageThread(m_stream, &m_sessionActive));
		m_garbageThread->Start();
	}

	/// Get current time as time for next frame
	DWORD nextFrameTime = timeGetTime();
	DWORD currentTime = 0;
	DWORD exitTime = timeGetTime() + SETTINGS_INSTANCE.GetTestTime();

	while(!Terminated())
	{
		currentTime = timeGetTime();
		if(currentTime > nextFrameTime)
		{
			/// Select next frame
			CCritSection section(&m_section);
			if(m_currentFrame.get() == m_firstFrame.get())
				m_currentFrame = m_secondFrame;
			else
				m_currentFrame = m_firstFrame;
			InvalidateRect(hWnd, &rect, FALSE);
			nextFrameTime += 500;
		}
		if(currentTime > exitTime)
		{
			if(!m_sessionActive)
			{
				/// Wait for session restoration
				ResetEvent(m_sessionEvent.get());
				WaitForSingleObject(m_sessionEvent.get(), 30000);
			}
			break;
		}
		Sleep(1);
	}

CATCH_LOG()
	
	/// Pause at finish
	Sleep(1000);

	if(m_garbageThread.get())
		m_garbageThread->Terminate();

	if(m_sessionActive)
		StopClient(m_clientId);

/*
	/// Close stream
	if(m_stream.get())
		m_stream->Close();
*/

	/// Close window
	if(hWnd)
		SendMessage(hWnd, WM_DESTROY, 0, 0);
}

void CNWLGarbageServer::ClientConnected(void*)
{
TRY_CATCH

	SetEvent(m_connectEvent.get());

CATCH_THROW()
}

void CNWLGarbageServer::NotifySessionStarted(const int clientId)
{
TRY_CATCH

	SetEvent(m_sessionEvent.get());
	m_sessionActive = true;
	m_clientId = clientId;

CATCH_THROW()
}

void CNWLGarbageServer::DrawFrame()
{
TRY_CATCH

	CCritSection section(&m_section);

	/// Draw current frame
	if(m_currentFrame.get())
		m_currentFrame->Draw();

CATCH_THROW()
}

void CNWLGarbageServer::NotifySessionStopped(const int clientId, ESessionStopReason ReasonCode)
{
TRY_CATCH

	m_sessionActive = false;

CATCH_THROW()
}

void CNWLGarbageServer::NotifySessionBroke()
{
TRY_CATCH

	m_sessionActive = false;

CATCH_THROW()
}

void CNWLGarbageServer::NotifySessionRestored()
{
TRY_CATCH

	m_sessionActive = true;
	SetEvent(m_sessionEvent.get());

CATCH_THROW()
}
