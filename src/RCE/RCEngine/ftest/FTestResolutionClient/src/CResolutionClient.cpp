/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CResolutionClient.cpp
///
///  Implements CResolutionClient class, responsible for client part of 
///    "Resolution" functional test
///
///  @author Dmitry Netrebenko @date 13.06.2007
///
////////////////////////////////////////////////////////////////////////

#include "CResolutionClient.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "CSettings.h"
#include <AidLib/Strings/tstring.h>
#include <boost/bind.hpp>
#include <AidLib/CCritSection/CCritSection.h>

///  Callback from viewer's draw method
void ViewerCallback(HDC hDC)
{
TRY_CATCH

	RESOLUTIONCLIENT_INSTANCE.ProcessFrame(hDC);

CATCH_LOG()
}


CResolutionClient::CResolutionClient()
{
TRY_CATCH

	InitializeCriticalSection(&m_section);

CATCH_THROW()
}

CResolutionClient::~CResolutionClient()
{
TRY_CATCH

	DeleteCriticalSection(&m_section);

CATCH_LOG()
}

void CResolutionClient::StartViewer()
{
TRY_CATCH

	HWND hWnd = SETTINGS_INSTANCE.GetWindow();
	unsigned int port = SETTINGS_INSTANCE.GetRemotePort();
	unsigned int timeout = SETTINGS_INSTANCE.GetTimeout();
	tstring addr = SETTINGS_INSTANCE.GetRemoteAddr();

	/// Create stream
	m_stream.reset(new CSocketStream());

	/// Connect
	m_stream->Connect(addr, port, true);

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

void CResolutionClient::ProcessFrame(HDC hdc)
{
TRY_CATCH

	/// Enter critical section
	CCritSection section(&m_section);

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

void CResolutionClient::SessionStop(ESessionStopReason ReasonCode)
{
TRY_CATCH

	Log.Add(_MESSAGE_, _T("SessionStop reason: %d"), ReasonCode);

	/// Terminate waiter
	if(m_waiter.get())
	{
		m_waiter->Terminate();
		SetEvent(m_waiter->GetFrameEvent().get());
	}

	/// Clear callback
	frameRateTestCallback = NULL;

	/// Store test results
	if(REMOTE_STOP == ReasonCode)
		Log.Add(_FTEST_SUITE_(_T("Resolution Test")), _T("Passed"));
	else
		Log.Add(_FTEST_SUITE_(_T("Resolution Test")), _T("Failed"));

	/// Close window in "autostart" mode
	HWND hWnd = SETTINGS_INSTANCE.GetWindow();
	if(hWnd && SETTINGS_INSTANCE.GetClientAutoStart())
		PostMessage(hWnd, WM_DESTROY, 0, 0);

CATCH_THROW()
}

void CResolutionClient::SessionStart()
{
TRY_CATCH

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

	/// Start waiter
	m_waiter.reset(
		new CFrameWaiter(
			boost::bind(&CResolutionClient::OnFrameTimeout, this, _1)
			)
		);

CATCH_THROW()
}

void CResolutionClient::OnFrameTimeout(void*)
{
TRY_CATCH

	Log.Add(_ERROR_,_T("CResolutionClient: Frame receive timed out"));
	//m_viewer->Stop();

CATCH_THROW()
}

void CResolutionClient::OnResolutionChanged(const int width, const int height)
{
TRY_CATCH

	Log.Add(_MESSAGE_, _T("New host resolution: %d x %d"), width, height);

	/// Enter critical section
	CCritSection section(&m_section);

	/// Store new window size
	SETTINGS_INSTANCE.SetWindowWidth(width);		
	SETTINGS_INSTANCE.SetWindowHeight(height);

	/// Init frames
	m_firstFrame->Init(true);
	m_secondFrame->Init(false);

CATCH_THROW()
}

