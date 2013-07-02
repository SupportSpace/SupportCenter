/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNWLGarbageClient.cpp
///
///  Implements CNWLGarbageClient class, responsible for client part of 
///    "NWL Garbage" functional test
///
///  @author Dmitry Netrebenko @date 07.06.2007
///
////////////////////////////////////////////////////////////////////////

#include "CNWLGarbageClient.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "CSettings.h"
#include <AidLib/Strings/tstring.h>
#include "CGarbageStream.h"
#include <boost/bind.hpp>

///  Callback from viewer's draw method
void ViewerCallback(HDC hDC)
{
TRY_CATCH

	NWLGARGAGECLIENT_INSTANCE.ProcessFrame(hDC);

CATCH_LOG()
}

CNWLGarbageClient::CNWLGarbageClient()
	:	m_sessionActive(false)
{
TRY_CATCH
CATCH_THROW()
}

CNWLGarbageClient::~CNWLGarbageClient()
{
TRY_CATCH
CATCH_LOG()
}

void CNWLGarbageClient::StartViewer()
{
TRY_CATCH

	HWND hWnd = SETTINGS_INSTANCE.GetWindow();
	unsigned int port = SETTINGS_INSTANCE.GetRemotePort();
	unsigned int timeout = SETTINGS_INSTANCE.GetTimeout();
	tstring addr = SETTINGS_INSTANCE.GetRemoteAddr();

	/// Create connection stream
	if(SETTINGS_INSTANCE.GetSendGarbage())
	{
		if(SETTINGS_INSTANCE.GetUseGarbageThread())
			m_stream.reset(new CSocketStream());
		else
			m_stream.reset(new CGarbageStream(&m_sessionActive));
	}
	else
		m_stream.reset(new CSocketStream());

	/// Connect
	m_stream->Connect(addr, port, true);

	/// Receive server's window size
	int windowWidth = 0;
	int windowHeight = 0;
	m_stream->Receive(reinterpret_cast<char*>(&windowWidth), sizeof(int));
	m_stream->Receive(reinterpret_cast<char*>(&windowHeight), sizeof(int));

	/// Set window size
	SETTINGS_INSTANCE.SetWindowWidth(windowWidth);
	SETTINGS_INSTANCE.SetWindowHeight(windowHeight);

	/// Initialize frames
	m_firstFrame.reset(new CFrame());
	m_firstFrame->Init(true);
	m_secondFrame.reset(new CFrame());
	m_secondFrame->Init(false);
	
	/// Create viewer
	m_viewer.reset(new CTestViewer(m_stream, hWnd));

	///  Set session mode
	m_viewer->SetSessionMode(VIEW_ONLY, true);

	/// Set display mode
	m_viewer->SetDisplayMode(static_cast<EDisplayMode>(SETTINGS_INSTANCE.GetDisplayMode()));
	
	/// Start session
	m_viewer->Start();
	
CATCH_THROW()
}

void CNWLGarbageClient::ProcessFrame(HDC hdc)
{
TRY_CATCH

	SPFrame frame;
	if(m_firstFrame->Check(hdc))
		frame = m_firstFrame;
	else
	{
		if(m_secondFrame->Check(hdc))
			frame = m_secondFrame;
	}
	if(m_currentFrame.get() && frame.get() && m_waiter.get())
	{
		/// Reset session active event
		if(frame.get() != m_currentFrame.get())
			SetEvent(m_waiter->GetFrameEvent().get());
	}

	m_currentFrame = frame;

CATCH_THROW()
}

void CNWLGarbageClient::SessionStop(ESessionStopReason ReasonCode)
{
TRY_CATCH

	Log.Add(_MESSAGE_, _T("SessionStop reason: %d"), ReasonCode);

/*
	LOCAL_STOP		= 0,	/// Stop() called; 
	REMOTE_STOP		= 1,	/// Remote Stop() called; 
	STREAM_ERROR	= 2,	/// Stream signaled 
	PROTOCOL_ERROR	= 3,	/// Some protocol error
	CHANGE_DISPLAY_MODE,
	CONNECTING_ERROR,		///when connection has not.
	OPENFILE_ERROR			///open file failed
*/

	/// Set flag
	m_sessionActive = false;

	if(m_garbageThread.get())
		m_garbageThread->Terminate();

	/// Terminate waiter
	if(m_waiter.get())
	{
		m_waiter->Terminate();
		SetEvent(m_waiter->GetSessionEvent().get());
		SetEvent(m_waiter->GetFrameEvent().get());
	}

	/// Clear callback
	frameRateTestCallback = NULL;

	/// Store test results
	if(REMOTE_STOP == ReasonCode)
		Log.Add(_FTEST_SUITE_(_T("Garbage Test")), _T("Passed"));
	else
		Log.Add(_FTEST_SUITE_(_T("Garbage Test")), _T("Failed"));

	/// Close window in "autostart" mode
	HWND hWnd = SETTINGS_INSTANCE.GetWindow();
	if(hWnd && SETTINGS_INSTANCE.GetClientAutoStart())
		PostMessage(hWnd, WM_DESTROY, 0, 0);

CATCH_THROW()
}

void CNWLGarbageClient::SessionStart()
{
TRY_CATCH

	/// Set flag
	m_sessionActive = true;

	/// Set callback
	frameRateTestCallback = &ViewerCallback;

	/// Update client window
	HWND hWnd = SETTINGS_INSTANCE.GetWindow();
	SetWindowPos(
		hWnd, 
		NULL, 
		GetSystemMetrics(SM_CXSCREEN)/2, 
		0, 
		GetSystemMetrics(SM_CXSCREEN)/2,
		GetSystemMetrics(SM_CYSCREEN)/2,
		SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOZORDER );

	if(SETTINGS_INSTANCE.GetUseGarbageThread() && SETTINGS_INSTANCE.GetSendGarbage())
	{
		/// Create garbage thread
		m_garbageThread.reset(new CGarbageThread(m_stream, &m_sessionActive));
		m_garbageThread->Start();
	}

	/// Start waiter
	m_waiter.reset(
		new CFrameWaiter(
			boost::bind(&CNWLGarbageClient::OnSessionTimeout, this, _1),
			boost::bind(&CNWLGarbageClient::OnFrameTimeout, this, _1)
			)
		);

	/// Set session active event
	if(m_waiter.get())
		SetEvent(m_waiter->GetSessionEvent().get());

CATCH_THROW()
}

void CNWLGarbageClient::OnSessionTimeout(void*)
{
TRY_CATCH
CATCH_THROW()
}

void CNWLGarbageClient::OnFrameTimeout(void*)
{
TRY_CATCH

	m_viewer->Stop();

CATCH_THROW()
}

void CNWLGarbageClient::SessionBroke()
{
TRY_CATCH

	/// Clear flag
	m_sessionActive = false;

	if(m_waiter.get())
	{
		/// Reset event
		ResetEvent(m_waiter->GetSessionEvent().get());
		/// Set event
		SetEvent(m_waiter->GetFrameEvent().get());
	}

CATCH_THROW()
}

void CNWLGarbageClient::SessionRestored()
{
TRY_CATCH

	/// Set flag
	m_sessionActive = true;

	/// Set session active event
	if(m_waiter.get())
		SetEvent(m_waiter->GetSessionEvent().get());

CATCH_THROW()
}

