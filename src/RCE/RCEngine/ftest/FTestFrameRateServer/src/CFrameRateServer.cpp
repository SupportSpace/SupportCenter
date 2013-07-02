/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFrameRateServer.cpp
///
///  Implements CFrameRateServer class, responsible for server part of 
///    frame rate test
///
///  @author Dmitry Netrebenko @date 14.05.2007
///
///  @modified Alexander Novak @date 11.10.2007
///
///  Added MultiplexedStream support
///
////////////////////////////////////////////////////////////////////////

#include "CFrameRateServer.h"
#include <AidLib/CSingleton/CSingleton.h>
#include "CSettings.h"
#include <AidLib/CCritSection/CCritSection.h>
#include <boost/bind.hpp>
#include <AidLib/CException/CException.h>

CFrameRateServer::CFrameRateServer()
	:	CThread()
	,	CRCHost()
	,	m_connectEvent(NULL)
	,	m_sessionEvent(NULL)
{
TRY_CATCH

	/// Create frame sequence
	m_frameSequence.reset(new CFrameSequence());

	/// Create connection stream
	m_stream.reset(new CSocketStream());

	/// Set OnConnected event handler
	m_stream->SetConnectedEvent(boost::bind(&CFrameRateServer::ClientConnected, this, _1));

	/// Initialize critical section
	InitializeCriticalSection(&m_section);

	/// Create event object
	m_connectEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	/// Create event object
	m_sessionEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

CATCH_THROW()
}

CFrameRateServer::~CFrameRateServer()
{
TRY_CATCH

	/// Terminate thread
	Terminate();

	/// Delete critical section
	DeleteCriticalSection(&m_section);
	/// Destroy event
	CloseHandle(m_connectEvent);
	/// Destroy event
	CloseHandle(m_sessionEvent);

CATCH_LOG()
}

void CFrameRateServer::Execute(void*)
{
	HWND hWnd = NULL;
	bool viewOnly = false;
	int clientId = 0;

TRY_CATCH

	CoInitialize(NULL);

	/// Load frames
	m_frameSequence->LoadFrames();

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
	viewOnly = SETTINGS_INSTANCE.GetServerPlayOnly();

	if(!viewOnly)
	{
		ResetEvent(m_connectEvent);

		/// Accept stream
		m_stream->Accept(port);

		/// Check for timeout
		if(WAIT_TIMEOUT == WaitForSingleObject(m_connectEvent, timeout))
			throw MCException(_T("Connect timeout expired"));

		if (SETTINGS_INSTANCE.GetMultiplexMode())
		{
			m_multiplexer = CFrameRateStreamMultiplexer::GetInstance<CFrameRateStreamMultiplexer>(m_stream);

			m_transportStream = m_multiplexer->GetSubStream(DEFAULT_SUBSTREAM_ID,
															DEFAULT_SUBSTREAM_PRIORITY,
															SETTINGS_INSTANCE.GetTimeout());
		}
		else
			m_transportStream = m_stream;

		/// Send window size
		m_transportStream->Send(reinterpret_cast<char*>(&windowWidth), sizeof(int));
		m_transportStream->Send(reinterpret_cast<char*>(&windowHeight), sizeof(int));

		if(!Terminated())
		{
			/// Start client
			clientId = CRCHost::StartClient(m_transportStream, 0);

			/// Wait for session starting
			ResetEvent(m_sessionEvent);
			if(WAIT_TIMEOUT == WaitForSingleObject(m_sessionEvent, timeout))
				throw MCException(_T("Session start timeout expired"));
		}
	}
	/// Get current time as time for next frame
	DWORD nextFrameTime = timeGetTime();
	DWORD currentTime = 0;

	while(!Terminated())
	{
		currentTime = timeGetTime();
		if(currentTime > nextFrameTime)
		{
			/// Select next frame
			CCritSection section(&m_section);
			m_currentFrame = m_frameSequence->GetNextFrame();
			if(!m_currentFrame.get())
				break;
			InvalidateRect(hWnd, m_currentFrame->GetBoundRect().get(), FALSE);
			nextFrameTime += 30;
		}
		Sleep(1);
	}

CATCH_LOG()
	
	/// Pause at finish
	Sleep(1000);

	/// Close stream
	if(m_stream.get() && !viewOnly)
	{
		CRCHost::StopClient(clientId);

		if ( m_multiplexer.get() )
			m_transportStream.reset();
		
		m_stream->Close();
	}
	/// Close window
	if (hWnd)
		SendMessage(hWnd, WM_DESTROY, 0, 0);
}

SPFrame CFrameRateServer::GetCurrentFrame()
{
TRY_CATCH

	return m_currentFrame;

CATCH_THROW()
}

CRITICAL_SECTION* CFrameRateServer::GetSection()
{
TRY_CATCH

	return &m_section;

CATCH_THROW()
}

void CFrameRateServer::ClientConnected(void*)
{
TRY_CATCH

	SetEvent(m_connectEvent);

CATCH_THROW()
}

void CFrameRateServer::NotifySessionStarted(const int clientId)
{
TRY_CATCH

	SetEvent(m_sessionEvent);

CATCH_THROW()
}
