/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFrameRateClient.cpp
///
///  Implements CFrameRateClient class, responsible for client part of 
///    frame rate test
///
///  @author Dmitry Netrebenko @date 15.05.2007
///
///  @modified Alexander Novak @date 11.10.2007
///
///  Added MultiplexedStream support
///
////////////////////////////////////////////////////////////////////////

#include "CFrameRateClient.h"
#include <AidLib/Strings/tstring.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "CSettings.h"
#include <AidLib/CException/CException.h>


///  Callback from viewer's draw method
void ViewerCallback(HDC hDC)
{
TRY_CATCH

	FRAMERATECLIENT_INSTANCE.ProcessFrame(hDC);

CATCH_LOG()
}


CFrameRateClient::CFrameRateClient()
	:	m_startTime(0)
	,	m_sessionStarted(false)
	,	m_frevFrameTime(0)
{
TRY_CATCH
CATCH_THROW()
}

CFrameRateClient::~CFrameRateClient()
{
TRY_CATCH
CATCH_LOG()
}

void CFrameRateClient::StartViewer()
{
TRY_CATCH

	HWND hWnd = SETTINGS_INSTANCE.GetWindow();
	unsigned int port = SETTINGS_INSTANCE.GetRemotePort();
	unsigned int timeout = SETTINGS_INSTANCE.GetTimeout();
	tstring addr = SETTINGS_INSTANCE.GetRemoteAddr();

	/// Create results
	m_results.reset(new CTestResults());

	/// Create connection stream
	m_stream.reset(new CSocketStream());
	/// Connect
	m_stream->Connect(addr, port, true);
	
	if (SETTINGS_INSTANCE.GetMultiplexMode())
	{
		m_multiplexer = CFrameRateStreamMultiplexer::GetInstance<CFrameRateStreamMultiplexer>(m_stream);

		m_transportStream = m_multiplexer->GetSubStream(DEFAULT_SUBSTREAM_ID,
														DEFAULT_SUBSTREAM_PRIORITY,
														SETTINGS_INSTANCE.GetTimeout());
	}
	else
		m_transportStream = m_stream; 
	
	/// Receive server's window size
	int windowWidth = 0;
	int windowHeight = 0;
	m_transportStream->Receive(reinterpret_cast<char*>(&windowWidth), sizeof(int));
	m_transportStream->Receive(reinterpret_cast<char*>(&windowHeight), sizeof(int));

	/// Set window size
	SETTINGS_INSTANCE.SetWindowWidth(windowWidth);
	SETTINGS_INSTANCE.SetWindowHeight(windowHeight);

	/// Create frame sequence
	m_frameSequence.reset(new CFrameSequence());

	/// Load frames
	m_frameSequence->LoadFrames();

	/// Create viewer
	m_viewer.reset(new CTestViewer(m_transportStream, hWnd));

	///  Set session mode
	m_viewer->SetSessionMode(VIEW_ONLY, true);

	/// Set display mode
	m_viewer->SetDisplayMode(static_cast<EDisplayMode>(SETTINGS_INSTANCE.GetDisplayMode()));
	
	/// Start session
	m_viewer->Start();

CATCH_THROW()
}

void CFrameRateClient::ProcessFrame(HDC hdc)
{
TRY_CATCH

	if(!m_sessionStarted)
		return;
	
	int skipped = 0;

	/// Search frame and calculate FPS
	if(m_frameSequence->FindFrame(hdc, &skipped))
	{
		if(-1 == skipped)
			return;
		/// Get time of current frame
		DWORD frame_time = timeGetTime() - m_startTime;
		/// Get time between frames
		DWORD time_delta = frame_time - m_frevFrameTime;
		m_frevFrameTime = frame_time;
		if(!time_delta)
			return;
		/// FPS calculating
		double fps = (1000.0 / time_delta) * (1.0 / (1.0 + skipped));
		/// Store results
		if(m_results.get())
			m_results->AddEntry(frame_time, fps);
	}

CATCH_THROW()
}

void CFrameRateClient::SessionStop()
{
TRY_CATCH
	
	/// Set flag
	m_sessionStarted = false;

	/// Save results and destroy
	if(m_results.get())
	{
		m_results->SaveResults();
		m_results.reset();
	}
	if ( m_multiplexer.get() )
	{
		m_transportStream.reset();
		m_multiplexer.reset();
	}
	/// Close and destroy stream
	if(m_stream.get())
	{
		m_stream->Close();
		m_stream.reset();
	}

	/// Clear callback
	frameRateTestCallback = NULL;

	/// Close window in "autostart" mode
	HWND hWnd = SETTINGS_INSTANCE.GetWindow();
	if(hWnd && SETTINGS_INSTANCE.GetClientAutoStart())
		PostMessage(hWnd, WM_DESTROY, 0, 0);

CATCH_THROW()
}

void CFrameRateClient::SessionStart()
{
TRY_CATCH

	/// Store start time
	m_startTime = timeGetTime();

	/// Set flag
	m_sessionStarted = true;

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

CATCH_THROW()
}

