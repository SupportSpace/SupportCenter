#include <winsock2.h>
#include "crcviewer.h"
#include "VNCviewerApp32.h"
#include <CTokenCatcher.h>
#include "rfb.h"
#include <AidLib/CTime/cTime.h>
#include "RCE_Resource.h"

void (*frameRateTestCallback)(HDC hdc) = NULL;

#pragma warning( disable: 4996 )//<func> was declared deprecated

unsigned int CRCViewer::m_msgSetAlphaBlend = RegisterWindowMessage(_T("CRCViewer::m_msgSetAlphaBlend"));
unsigned int CRCViewer::m_msgSetVisualPointer = RegisterWindowMessage(_T("CRCViewer::m_msgSetVisualPointer"));
unsigned int CRCViewer::m_msgSetViewOnly = RegisterWindowMessage(_T("CRCViewer::m_msgSetViewOnly"));

CRCViewer::CRCViewer(boost::shared_ptr<CAbstractStream> stream , HWND window )
	:	CInstanceTracker(_T("CRCViewer")),
		m_displayMode( SCALE_MODE ),
		m_hDesctopKeeper(0),m_hDesctop(0),
		m_autoDetectOptions(true),
		m_klientThreadKilled(false),
		m_visualPointer(false),
		m_viewOnly(false),
		m_captureAlphaBlend(false),
		m_modeFromViewerSideSelected(false),
		m_hScrollBar(NULL),
		m_vScrollBar(NULL),
		m_hideWallpaper(true)
{
TRY_CATCH

	InitializeCriticalSection(&m_cs1);
	InitializeCriticalSection(&m_cs2);
	m_hDesctop = window;
	m_stream = boost::shared_ptr<CShadowedStream>(new CShadowedStream(stream,CShadowedStream::INPUT));

CATCH_THROW()
};

CRCViewer::~CRCViewer()
{
TRY_CATCH

	if (m_vncViewer.get())
		Stop();
	DeleteCriticalSection(&m_cs1);
	DeleteCriticalSection(&m_cs2);

CATCH_LOG()
}

void CRCViewer::Stop()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("RCViewer: Stop called"));
	if (m_vncViewer.get())
	{
		m_vncViewer->GetConnection()->StopSession();

		/// Stopping thread
		//m_vncViewer->GetConnection()->KillThread();
		m_vncViewer.reset();

		/// Notifying stopped
		NotifySessionStopped(LOCAL_STOP);
	} else
		throw MCException("Already stopped");

CATCH_THROW()
}

void CRCViewer::Start()
{
TRY_CATCH

	CCritSection cs(&m_cs2);
	if (m_vncViewer.get()) 
		throw MCException("Already started");

	///Sending start command
	int len = strlen( START_COMMAND );
	char data[] =  START_COMMAND;
	m_stream->Send( data , len );

	m_vncViewer.reset(new VNCviewerApp32( m_hDesctop , "" ));  
	if (!m_autoDetectOptions)
	{
		/// Setting custom options
		m_vncViewer->m_options.autoDetect = false;
		m_vncViewer->m_options.m_Use8Bit = m_customOptions.m_colorsCount;
		m_vncViewer->m_options.m_PreferredEncoding = m_customOptions.m_PreferredEncoding;
		/*m_vncViewer->m_options.m_useCompressLevel = m_customOptions.m_useCompressLevel;
		m_vncViewer->m_options.m_compressLevel = m_customOptions.m_compressLevel;
		m_vncViewer->m_options.m_enableJpegCompression = m_customOptions.m_enableJpegCompression;
		m_vncViewer->m_options.m_jpegQualityLevel = m_customOptions.m_jpegQualityLevel;*/
	}
	m_vncViewer->NewConnection( m_stream , this );
	m_vncViewer->SetDisplayMode( m_displayMode );

CATCH_THROW()
};

void CRCViewer::SetShadowStream(boost::shared_ptr<CAbstractStream> stream)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("Setting shadowed stream for viewer"));
	CCritSection cs(&m_cs2);
	if (!m_vncViewer.get() || !stream.get())
		m_stream->SetShadowStream(stream);
	else
	/// If session allready started
	/// Adding headers to stream
	/// Adding start session command
	{	
		/// Sending request to server to restart compressing
		CCritSection cs(GetCS1());
		char code = rfbResetStreams;
		m_stream->Send(&code, 1);
		int len = strlen( START_COMMAND );
		char data[] =  START_COMMAND;
		stream->Send( data , len );

		/// Adding server init
		if (m_serverInitMsg.get())
		{
			stream->Send( reinterpret_cast<char*>(m_serverInitMsg.get()), sizeof(rfbServerInitMsg) );
			/// Sending server display name
			int nameLength = Swap32IfLE(m_serverInitMsg->nameLength);
			stream->Send( m_displayName, nameLength );
			/// Sending pixel format
			if (m_serverInitMsg.get())
			if (m_pixelFormat.get())
			{
				stream->Send( reinterpret_cast<char*>(m_pixelFormat.get()), sz_rfbSetPixelFormatMsg );
			}
			/// Setting shadow stream
			m_stream->SetShadowStream(stream);
		}
	}
CATCH_THROW()
};

void CRCViewer::RedrawWindow()
{
TRY_CATCH
	if (m_vncViewer.get())
	{
		PostMessage(m_vncViewer->GetConnection()->GetMainWindow(),WM_FULLSCREENUPDATED, 0, 0);
	}
CATCH_LOG()
};

void CRCViewer::WaitForBeginSession()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("Waiting for start command from server"));
	/// Waiting session start
	char code;
	tstring startToken(START_COMMAND);
	CTokenCatcher tockenCatcher(startToken.c_str(),startToken.length());
	cDate startTime;
	startTime.GetNow();
	while(true)
	{
		if (m_stream->GetMainStream()->HasInData())
		{
			m_stream->Receive(&code,1);
			if (tockenCatcher.Send(&code,1))
				break;
		}
		if (((cDate()).GetNow() > (cDate(startTime)).AddMilliSecs(SESSION_START_TIMEOUT)))
			throw MCStreamException("Wait for session start timed out. Stopping service...");
		Sleep(1);
	}	
	Log.Add(_MESSAGE_,_T("Server start command received"));

CATCH_THROW()
}

void CRCViewer::SetDisplayMode(const EDisplayMode mode_)
{
TRY_CATCH
	CCritSection cs(&m_cs2);
	m_displayMode = mode_;
	if( m_vncViewer.get() )
		m_vncViewer->SetDisplayMode( m_displayMode );
CATCH_THROW()
}

EDisplayMode CRCViewer::GetDisplayMode()
{
TRY_CATCH
	CCritSection cs(&m_cs2);
	if( m_vncViewer.get() )
		return m_displayMode = m_vncViewer->m_options.m_displayMode;
	else
	{
		return m_displayMode;
	}
CATCH_THROW()
}

void CRCViewer::SetDesctopHandle( HWND hWnd )
{
TRY_CATCH
	m_hDesctop = hWnd;
CATCH_THROW()
}

void CRCViewer::SetCustomOptions(const SViewerOptions &customOptions)
{
TRY_CATCH
	CCritSection cs(&m_cs2);
	if (m_vncViewer.get() && m_vncViewer->GetConnection())
	{
		/// If we already running, applying changes
		m_vncViewer->GetConnection()->m_opts.autoDetect = customOptions.m_autoColors;
		if (!customOptions.m_autoColors)
		{
			m_vncViewer->GetConnection()->m_newColors = customOptions.m_colorsCount;
			m_vncViewer->GetConnection()->m_pendingFormatChange = true;
			m_vncViewer->GetConnection()->m_pendingColorsRequest = true;
		} else
		{
			m_vncViewer->GetConnection()->m_nConfig = -1;
			m_vncViewer->GetConnection()->m_lLastChangeTime = 0;
		}
	} else
	{
		m_autoDetectOptions = customOptions.m_autoColors;
		m_customOptions = customOptions;
	}
CATCH_THROW()
}

void CRCViewer::SetSessionMode(const ESessionMode mode, const bool state)
{
TRY_CATCH
	CCritSection cs(&m_cs2);
	m_modeFromViewerSideSelected = true;
	switch(mode)
	{
		case VIEW_ONLY:
			{
				if (m_vncViewer.get())
				{
					PostMessage(m_vncViewer->GetConnection()->GetMainWindow(), m_msgSetViewOnly, state, 0);
				} else
				{
					m_viewOnly = state;
				}
			}
			break;
		case VISUAL_POINTER:
			{
				if (m_vncViewer.get())
				{
					PostMessage(m_vncViewer->GetConnection()->GetMainWindow(), m_msgSetVisualPointer, state, 0);
				} else
				{
					m_visualPointer = state;
				}
			}
			break;
		default:
			throw MCException(Format("Unknown mode %d",mode));
	}
CATCH_THROW()
}

void CRCViewer::UpdateSessionMode(const ESessionMode mode, const bool state)
{
TRY_CATCH
	/// Notifying mode changed
	NotifyModeChanged(mode,state);
	switch(mode)
	{
		case VIEW_ONLY:
			{
				m_viewOnly = state;
			}
			break;
		case VISUAL_POINTER:
			{
				m_visualPointer = state;
			}
			break;
		default:
			throw MCException(Format("Unknown mode %d",mode));
	}
CATCH_LOG()
}

void CRCViewer::SetCaptureAlphaBlend(bool captureAlplhaBlend)
{
TRY_CATCH
	CCritSection cs(&m_cs2);
	m_captureAlphaBlend = captureAlplhaBlend;
	if (m_vncViewer.get())
	{
		PostMessage(m_vncViewer->GetConnection()->GetMainWindow(), m_msgSetAlphaBlend, captureAlplhaBlend, 0);
	}
CATCH_THROW()
}

void CRCViewer::SetHideWallpaper(const bool hideWallpaper)
{
TRY_CATCH
	CCritSection cs(&m_cs2);
	m_hideWallpaper = hideWallpaper;
CATCH_THROW()
}

void CRCViewer::SetServerInitMsg(const rfbServerInitMsg *msg)
{
TRY_CATCH
	m_serverInitMsg.reset(new rfbServerInitMsg(*msg));
CATCH_THROW()
}

void CRCViewer::SetDisplayName(const TCHAR* displayName)
{
TRY_CATCH
	_tcscpy(m_displayName, displayName);
CATCH_THROW()
}

void CRCViewer::SetPixelFormat(const rfbClientToServerMsg* msg)
{
TRY_CATCH
	m_pixelFormat.reset(new rfbClientToServerMsg(*msg));
CATCH_THROW()
}

void CRCViewer::SendCtrlAltDel()
{
TRY_CATCH
	if (m_vncViewer.get() && m_vncViewer->GetConnection())
	{
		PostMessage(m_vncViewer->GetConnection()->GetOriginalWindow(), WM_SYSCOMMAND, MAKEWORD(ID_CONN_CTLALTDEL,0),0);
	}
CATCH_THROW()
}

void CRCViewer::SetScrollBars(HWND horScrollBar, HWND vertScrollBar)
{
TRY_CATCH
	m_hScrollBar = horScrollBar;
	m_vScrollBar = vertScrollBar;	
	if (NULL != m_vncViewer.get() && NULL != m_vncViewer->GetConnection())
	{
		m_vncViewer->GetConnection()->SetScrollBars(horScrollBar, vertScrollBar);
	}
CATCH_THROW()
}

void CRCViewer::OnMinimize()
{
TRY_CATCH
	if (NULL != m_vncViewer.get() && NULL != m_vncViewer->GetConnection())
	{
		m_vncViewer->GetConnection()->ChangeDisplayMode(SCALE_MODE);
	}
CATCH_THROW()
}

void CRCViewer::OnRestore()
{
TRY_CATCH
	if (NULL != m_vncViewer.get() && NULL != m_vncViewer->GetConnection())
	{
		m_vncViewer->GetConnection()->ChangeDisplayMode(SCROLL_MODE);
	}
CATCH_THROW()
}

void CRCViewer::RestorePrevDisplayMode()
{
TRY_CATCH
	if (NULL != m_vncViewer.get() && NULL != m_vncViewer->GetConnection())
	{
		m_vncViewer->GetConnection()->RestorePrevDisplayMode();
	}
CATCH_THROW()
}