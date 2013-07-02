/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CQualityClient.cpp
///
///  Implements CQualityClient class, responsible for client part of 
///    "Picture Quality" functional test
///
///  @author Dmitry Netrebenko @date 18.07.2007
///
////////////////////////////////////////////////////////////////////////

#include "CQualityClient.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "CSettings.h"
#include <AidLib/CCritSection/CCritSection.h>

///  Callback from viewer's draw method
void ViewerCallback(HDC hDC)
{
TRY_CATCH

	QUALITYCLIENT_INSTANCE.ProcessFrame(hDC);

CATCH_LOG()
}

CQualityClient::CQualityClient()
	:	CThread()
{
TRY_CATCH

	/// Create critical section
	InitializeCriticalSection(&m_section);

	/// Create event object
	m_event.reset( CreateEvent(NULL, TRUE, FALSE, NULL), CloseHandle );

CATCH_THROW()
}

CQualityClient::~CQualityClient()
{
TRY_CATCH

	/// Terminate waiter's thread
	Terminate();
	SetEvent(m_event.get());

	/// Delete critical section
	DeleteCriticalSection(&m_section);

CATCH_LOG()
}

void CQualityClient::Execute(void* Params)
{
TRY_CATCH

	ResetEvent(m_event.get());
	
	DWORD testTime = SETTINGS_INSTANCE.GetTestTime();

	if(WAIT_TIMEOUT == WaitForSingleObject(m_event.get(), testTime))
		CheckTestResults();

CATCH_LOG()
}

void CQualityClient::StartViewer()
{
TRY_CATCH

	m_invalidPixels = 0;

	HWND hWnd = SETTINGS_INSTANCE.GetWindow();
	unsigned int port = SETTINGS_INSTANCE.GetRemotePort();
	unsigned int timeout = SETTINGS_INSTANCE.GetTimeout();
	tstring addr = SETTINGS_INSTANCE.GetRemoteAddr();
	HDC hdc = SETTINGS_INSTANCE.GetDC();

	m_stream.reset(new CSocketStream());

	Log.Add(_MESSAGE_, _T("Connecting to %s:%d"), addr.c_str(), port);

	/// Connect
	m_stream->Connect(addr, port, true);

	/// Receive server's window size
	int windowWidth = 0;
	int windowHeight = 0;
	m_stream->Receive(reinterpret_cast<char*>(&windowWidth), sizeof(int));
	m_stream->Receive(reinterpret_cast<char*>(&windowHeight), sizeof(int));

	Log.Add(_MESSAGE_, _T("Server window size: %d x %d"), windowWidth, windowHeight);

	DWORD depth = 0;
	m_stream->Receive(reinterpret_cast<char*>(&depth), sizeof(DWORD));

	CalcDelta(depth);

	/// Set window size
	SETTINGS_INSTANCE.SetWindowWidth(windowWidth);
	SETTINGS_INSTANCE.SetWindowHeight(windowHeight);

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

	/// Loads frame
	PrepareOriginalFrame();

	/// Create viewer
	m_viewer.reset(new CTestViewer(m_stream, hWnd));

	///  Set session mode
	m_viewer->SetSessionMode(VIEW_ONLY, true);

	/// Set display mode
	m_viewer->SetDisplayMode(static_cast<EDisplayMode>(SETTINGS_INSTANCE.GetDisplayMode()));
	
	/// Set optiions
	SViewerOptions options;
	options.m_colorsCount = SETTINGS_INSTANCE.GetColorDepth();
	m_viewer->SetCustomOptions(options);

	/// Start session
	m_viewer->Start();
	
CATCH_THROW()
}

void CQualityClient::ProcessFrame(HDC hdc)
{
TRY_CATCH

	/// Enter critical section
	CCritSection section(&m_section);

	if(!m_frameDC.get() || !m_frameBitmap.get())
		return;

	int width = SETTINGS_INSTANCE.GetWindowWidth();
	int height = SETTINGS_INSTANCE.GetWindowHeight();

	/// Store frame's bitmap
	BitBlt(m_frameDC.get(), 0, 0, width, height, hdc, 0, 0, SRCCOPY);

CATCH_THROW()
}

void CQualityClient::SessionStop(ESessionStopReason ReasonCode)
{
TRY_CATCH

	/// Clear callback
	frameRateTestCallback = NULL;

	SetEvent(m_event.get());

	/// Store test results
	if(LOCAL_STOP == ReasonCode)
	{
		int totalPixels = SETTINGS_INSTANCE.GetWindowWidth() * SETTINGS_INSTANCE.GetWindowHeight();
		double invalidPerc = (m_invalidPixels * 100.0) / totalPixels;
		int allowed = SETTINGS_INSTANCE.GetInvalidPixelPercentage();

		if(invalidPerc <= allowed)
			Log.Add(_FTEST_SUITE_(_T("Quality Test")), _T("Passed. Invalid pixels: %9.2f %% (%d from %d)"), invalidPerc, m_invalidPixels, totalPixels);
		else
			Log.Add(_FTEST_SUITE_(_T("Quality Test")), _T("Failed. Invalid pixels: %9.2f %% (%d from %d)"), invalidPerc, m_invalidPixels, totalPixels);
	}
	else
		Log.Add(_FTEST_SUITE_(_T("Quality Test")), _T("Failed (premature break of session)"));

	/// Close window in "autostart" mode
	HWND hWnd = SETTINGS_INSTANCE.GetWindow();
	if(hWnd && SETTINGS_INSTANCE.GetClientAutoStart())
		PostMessage(hWnd, WM_DESTROY, 0, 0);

CATCH_THROW()
}

void CQualityClient::SessionStart()
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
	Start();

CATCH_THROW()
}

void CQualityClient::CheckTestResults()
{
TRY_CATCH

	int width = SETTINGS_INSTANCE.GetWindowWidth();
	int height = SETTINGS_INSTANCE.GetWindowHeight();

	int totalPixels = width * height;
	int invalidPixels = 0;

	for(int j = 0; j < height; ++j)
	{
		for(int i = 0; i < width; ++i)
		{
			COLORREF color = GetPixel(m_frameDC.get(), i, j);
			BYTE rValue = GetRValue(color);
			BYTE gValue = GetGValue(color);
			BYTE bValue = GetBValue(color);
			COLORREF origColor = GetPixel(m_fileDC.get(), i, j);
			BYTE origRValue = GetRValue(origColor);
			BYTE origGValue = GetGValue(origColor);
			BYTE origBValue = GetBValue(origColor);

//Log.Add(_MESSAGE_, _T("Change color from (%d, %d, %d) ---> (%d, %d, %d)"), origRValue, origGValue, origBValue, rValue, gValue, bValue);

			if((rValue >= origRValue + m_delta) || (rValue <= origRValue - m_delta))
			{
				invalidPixels++;
				continue;
			}
			if((gValue >= origGValue + m_delta) || (gValue <= origGValue - m_delta))
			{
				invalidPixels++;
				continue;
			}
			if((bValue >= origBValue + m_delta) || (bValue <= origBValue - m_delta))
			{
				invalidPixels++;
				continue;
			}
		}
	}

	m_invalidPixels = invalidPixels;

	m_viewer->Stop();

CATCH_THROW()
}

void CQualityClient::CalcDelta(const DWORD origDepth)
{
TRY_CATCH

	DWORD depth = 16;
	DWORD serverDepth = origDepth;
	if(serverDepth > 24)
		serverDepth = 24;

	switch(SETTINGS_INSTANCE.GetColorDepth())
	{
	case 0: // Full colors
		depth = 16;
		break;
	case 1: // 256 colors
		depth = 8;
		break;
	case 2: // 64 colors
		depth = 6;
		break;
	case 3: // 8 colors
		depth = 3;
		break;
	case 4: //8 Grey colors
		depth = 3;
		break;
	case 5: // 4 colors
		depth = 2;
		break;
	case 6: // 2 Grey colors
		depth = 1;
		break;
	}

	DWORD deltaBitsPerColor = 0;

	if(depth >= serverDepth)
		m_delta = 1;
	else
	{
		deltaBitsPerColor = (serverDepth - depth) / 3;
		m_delta = 1 << deltaBitsPerColor;
	}

	Log.Add(_MESSAGE_, _T("Calculated delta: %d (%d bits); original color depth - %d bits; viewer color depth - %d bits"), m_delta, deltaBitsPerColor, serverDepth, depth);

CATCH_THROW()
}

void CQualityClient::PrepareOriginalFrame()
{
TRY_CATCH

	tstring fileName = SETTINGS_INSTANCE.GetFrameFile();
	
	SPBitmap fileBitmap((HBITMAP)LoadImage(NULL, fileName.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTCOLOR | LR_DEFAULTSIZE), DeleteObject);
	if(!fileBitmap.get())
		throw MCException_Win(_T("Can not load bitmap"));

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

	/// Create DC
	m_fileDC.reset(CreateCompatibleDC(hdc), DeleteDC);
	if(!m_fileDC.get())
		throw MCException_Win(_T("Can not create compatible DC"));

	/// Create bitmap
	m_fileBitmap.reset(CreateCompatibleBitmap(hdc, windowWidth, windowHeight), DeleteObject);
	if(!m_fileBitmap.get())
		throw MCException_Win(_T("Can not create bitmap"));

	/// Select bitmap
	SelectObject(m_fileDC.get(), m_fileBitmap.get());

	/// Copy bitmap
	if(!StretchBlt(m_fileDC.get(), 0, 0, windowWidth, windowHeight, fileDC.get(), 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY)) 
		throw MCException_Win(_T("Can not stretch bitmap"));

CATCH_THROW()
}

