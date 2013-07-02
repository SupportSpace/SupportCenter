//  Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
//
//  Copyright (C) 2000-2002 Const Kaplinsky. All Rights Reserved.
//
//  Copyright (C) 2002 RealVNC Ltd. All Rights Reserved.
//
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// If the source code for the VNC system is not available from the place
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


// Many thanks to Randy Brown <rgb@inven.com> for providing the 3-button
// emulation code.

// This is the main source for a ClientConnection object.
// It handles almost everything to do with a connection to a server.
// The decoding of specific rectangle encodings is done in separate files.


#define _WIN32_WINDOWS 0x0410
#define WINVER 0x0400

#include "stdhdrs.h"
#include "vncviewer.h"
#include <AidLib/CThread/CThreadLs.h>
#include <CTokenCatcher.h>

#ifdef UNDER_CE
#include "omnithreadce.h"
#define SD_BOTH 0x02
#else
#endif

#include "ClientConnection.h"
#include "LowLevelHook.h"

#include "Exception.h"

#include <rdr/FdInStream.h>
#include <rdr/ZlibInStream.h>
#include <rdr/Exception.h>

#include <rfb/dh.h>

#include <NWL/Streaming/CAbstractStream.h>
#include <CRCViewer.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/shared_ptr.hpp>
#include "RCEngine.h"
#include "util.h"

#pragma warning( disable: 4996 )//<func> was declared deprecated


// Sends the number of bytes specified from the buffer
inline void ClientConnection::WriteExact(char *buf, int bytes)
{
	if (0 == bytes)
	{
		Log.Add(_WARNING_,_T("Empty send requested"));
		return;
	}
	m_sentRecently = true;
	//m_stream->Send( buf , bytes );
	//m_stream->Send2Queue( buf , bytes );
	m_asyncStream.FastSend(buf, bytes);
}

#define INITIALNETBUFSIZE			4096
#define MAX_ENCODINGS				(LASTENCODING+10)
#define VWR_WND_CLASS_NAME			_T("RCViewer")
#define VWR_WND_CLASS_NAME_VIEWER	_T("RCViewerWindow")
#define VNCMDI_CLASS_NAME			_T("VNCMDI_Window")
#define SESSION_MRU_KEY_NAME		_T("Software\\ORL\\RCViewer\\MRU")


unsigned int ClientConnection::m_msgSendInitialStuff = RegisterWindowMessage("ClientConnection::m_msgSendInitialStuff");
unsigned int ClientConnection::m_msgSendRetryStuff = RegisterWindowMessage("ClientConnection::m_msgSendRetryStuff");
unsigned int ClientConnection::m_msgSendStopSession = RegisterWindowMessage("ClientConnection::m_msgSendStopSession");
extern bool g_passwordfailed;

/*
 * Macro to compare pixel formats.
 */
#define PF_EQ(x,y)							\
	((x.bitsPerPixel == y.bitsPerPixel) &&				\
	 (x.depth == y.depth) &&					\
	 ((x.bigEndian == y.bigEndian) || (x.bitsPerPixel == 8)) &&	\
	 (x.trueColour == y.trueColour) &&				\
	 (!x.trueColour || ((x.redMax == y.redMax) &&			\
			    (x.greenMax == y.greenMax) &&		\
			    (x.blueMax == y.blueMax) &&			\
			    (x.redShift == y.redShift) &&		\
			    (x.greenShift == y.greenShift) &&		\
			    (x.blueShift == y.blueShift))))

const rfbPixelFormat vnc8bitFormat			= {8,8,0,1,7,7,3,0,3,6, 0, 0}; // 256 colors
const rfbPixelFormat vnc8bitFormat_64		= {8,6,0,1,3,3,3,4,2,0, 0, 0} ;	// 64 colors
const rfbPixelFormat vnc8bitFormat_8		= {8,3,0,1,1,1,1,2,1,0, 0, 0} ;	// 8 colors
const rfbPixelFormat vnc8bitFormat_8Grey	= {8,8,0,1,7,7,3,0,3,6, 1, 0} ;	// 8 colors-Dark Scale
const rfbPixelFormat vnc8bitFormat_4Grey	= {8,6,0,1,3,3,3,4,2,0, 1, 0} ;	// 4 colors-Grey Scale
const rfbPixelFormat vnc8bitFormat_2Grey	= {8,3,0,1,1,1,1,2,1,0, 1, 0} ;	// 2 colors-Grey Scale

const rfbPixelFormat vnc16bitFormat			= {16,16,0,1,63,31,31,0,6,11, 0, 0};

extern HWND currentHWND;
extern char sz_L1[64];
extern char sz_L2[64];
extern char sz_L3[64];
extern char sz_L4[64];
extern char sz_L5[64];
extern char sz_L6[64];
extern char sz_L7[64];
extern char sz_L8[64];
extern char sz_L9[64];
extern char sz_L10[64];
extern char sz_L11[64];
extern char sz_L12[64];
extern char sz_L13[64];
extern char sz_L14[64];
extern char sz_L15[64];
extern char sz_L16[64];
extern char sz_L17[64];
extern char sz_L18[64];
extern char sz_L19[64];
extern char sz_L20[64];
extern char sz_L21[64];
extern char sz_L22[64];
extern char sz_L23[64];
extern char sz_L24[64];
extern char sz_L25[64];
extern char sz_L26[64];
extern char sz_L27[64];
extern char sz_L28[64];
extern char sz_L29[64];
extern char sz_L30[64];
extern char sz_L31[64];
extern char sz_L32[64];
extern char sz_L33[64];
extern char sz_L34[64];
extern char sz_L35[64];
extern char sz_L36[64];
extern char sz_L37[64];
extern char sz_L38[64];
extern char sz_L39[64];
extern char sz_L40[64];
extern char sz_L41[64];
extern char sz_L42[64];
extern char sz_L43[64];
extern char sz_L44[64];
extern char sz_L45[64];
extern char sz_L46[64];
extern char sz_L47[64];
extern char sz_L48[64];
extern char sz_L49[64];
extern char sz_L50[64];
extern char sz_L51[64];
extern char sz_L52[64];
extern char sz_L53[64];
extern char sz_L54[64];
extern char sz_L55[64];
extern char sz_L56[64];
extern char sz_L57[64];
extern char sz_L58[64];
extern char sz_L59[64];
extern char sz_L60[64];
extern char sz_L61[64];
extern char sz_L62[64];
extern char sz_L63[64];
extern char sz_L64[64];
extern char sz_L65[64];
extern char sz_L66[64];
extern char sz_L67[64];
extern char sz_L68[64];
extern char sz_L69[64];
extern char sz_L70[64];
extern char sz_L71[64];
extern char sz_L72[64];
extern char sz_L73[64];
extern char sz_L74[64];
extern char sz_L75[64];
extern char sz_L76[64];
extern char sz_L77[64];
extern char sz_L78[64];
extern char sz_L79[64];
extern char sz_L80[64];
extern char sz_L81[64];
extern char sz_L82[64];
extern char sz_L83[64];
extern char sz_L84[64];
extern char sz_L85[64];
extern char sz_L86[64];
extern char sz_L87[64];
extern char sz_L88[64];
extern char sz_L89[64];
extern char sz_L90[64];
extern char sz_L91[64];
extern char sz_L92[64];

extern char sz_F1[64];
extern char sz_F5[128];
extern char sz_F6[64];
extern bool command_line;

// *************************************************************************
//  A Client connection involves two threads - the main one which sets up
//  connections and processes window messages and inputs, and a 
//  client-specific one which receives, decodes and draws output data 
//  from the remote server.
//  This first section contains bits which are generally called by the main
//  program thread.
// *************************************************************************

bool ClientConnection::m_focused=false;
int ClientConnection::m_fullScreen=0;
ClientConnection::ClientConnection(VNCviewerApp *pApp, boost::shared_ptr<CAbstractStream> stream, CRCViewer* vptr,HWND hDesctop)
	:	
		m_integrator(DEF_INTEG_FILTER_LENGTH),
		m_recvRecently(true),
		m_sentRecently(true),
		m_asyncStream(stream),
		m_inited(false),
		fis(0), 
		zis(0),
		m_stream( stream ),
		m_ptrViewer( vptr ),
		m_pendingColorsRequest(false),
		m_keysSentCount(0),
		m_altSentIndex(-1),
		m_horScrollBar(NULL),
		m_vertScrollBar(NULL),
		m_vertScrollBarParent(NULL)
{
TRY_CATCH

	for(int i=0; i<DEF_INTEG_FILTER_LENGTH; ++i)
		m_integrator.Next(20000 /*20 MBps*/);

	m_aliveTimer.reset(0, NULL);
	m_streamTimer.reset(0, NULL);

	Init(pApp);
	if (m_opts.autoDetect)
	{
		m_opts.m_Use8Bit = rfbPFFullColors; //rfbPF256Colors;
		m_opts.m_fEnableCache = true; // sf@2002
	}
	m_serverInitiated = true;
	struct sockaddr_in svraddr;
	int sasize = sizeof(svraddr);
	_tcscpy(m_host,sz_L1);
	m_hwndMain = hDesctop;
	//CreateFullScreenWnd();

CATCH_THROW("ClientConnection::ClientConnection")
}

void ClientConnection::KillInternalTimer(UINT_PTR timerId)
{
TRY_CATCH
	if (0 == KillTimer(m_hwnd, timerId))
		Log.WinError(_WARNING_,_T("Failed to kill %d timer"),timerId);
CATCH_THROW()
}

void ClientConnection::SetScrollBars(HWND horScrollBar, HWND vertScrollBar)
{
TRY_CATCH
	m_horScrollBar = horScrollBar;
	m_vertScrollBar = vertScrollBar;
	if (NULL != m_vertScrollBar && NULL == m_vertScrollBarParent)
	{
		m_vertScrollBarParent = GetParent(m_vertScrollBar);
		SetParent(m_vertScrollBar, GetMainWindow());
	}
CATCH_THROW()
}

void ClientConnection::Init(VNCviewerApp *pApp)
{
TRY_CATCH

	Pressed_Cancel=false;
	saved_set=false;
	m_hwnd = 0;
	m_desktopName = NULL;
	m_port = -1;
	m_proxyport = -1;
	//	m_proxy = 0;
	m_serverInitiated = false;
	m_netbuf = NULL;
	m_netbufsize = 0;
	m_zlibbuf = NULL;
	m_zlibbufsize = 0;
	m_decompStreamInited = false;
	m_hwndNextViewer = NULL;	
	m_pApp = pApp;
	m_dormant = false;
	m_hBitmapDC = NULL;
	m_hBitmap = NULL;
	m_hCacheBitmapDC = NULL;
	m_hCacheBitmap = NULL;
	m_hPalette = NULL;
	m_encPasswd[0] = '\0';
	m_clearPasswd[0] = '\0'; // Modif sf@2002
	// static window
	m_BytesSend=0;
	m_BytesRead=0;

	// We take the initial conn options from the application defaults
	m_opts = m_pApp->m_options;
	//m_opts = m_ptrViewer->m_
	//ZeroMemory( &m_opts , sizeof( m_opts ) );

	m_bKillThread = false;
	m_threadStarted = true;
	m_running = false;
	m_restarting = false;
	m_pendingFormatChange = false;

	// sf@2002 - v1.1.2 - Data Stream Modification Plugin handling
	m_nTO = 1;
	//m_pDSMPlugin = new CDSMPlugin();
	m_fUsePlugin = false;
	m_fUseProxy = false;
	m_pNetRectBuf = NULL;
	m_fReadFromNetRectBuf = false;  // 
	m_nNetRectBufOffset = 0;
	m_nReadSize = 0;
	m_nNetRectBufSize = 0;
	m_pZRLENetRectBuf = NULL;
	m_fReadFromZRLENetRectBuf = false;  // 
	m_nZRLENetRectBufOffset = 0;
	m_nZRLEReadSize = 0;
	m_nZRLENetRectBufSize = 0;

	// ZlibHex
	m_decompStreamInited = false;
	m_decompStreamRaw.total_in = ZLIBHEX_DECOMP_UNINITED;
	m_decompStreamEncoded.total_in = ZLIBHEX_DECOMP_UNINITED;

	// Initialise a few fields that will be properly set when the
	// connection has been negotiated
	m_fullwinwidth = m_fullwinheight = 0;
	m_si.framebufferWidth = m_si.framebufferHeight = 0;

	m_hScrollPos = 0; m_vScrollPos = 0;

	m_waitingOnEmulateTimer = false;
	m_emulatingMiddleButton = false;

	oldPointerX = oldPointerY = oldButtonMask = 0;

	// Create a buffer for various network operations
	CheckBufferSize(INITIALNETBUFSIZE);

	kbitsPerSecond = 0;
	m_lLastChangeTime = 0; // 0 because we want the first encoding switching to occur quickly
	// (in Auto mode, ZRLE is used: pointless over a LAN)

	m_fScalingDone = false;

	zis = new rdr::ZlibInStream;

	// tight cusorhandling
	prevCursorSet = false;
	rcCursorX = 0;
	rcCursorY = 0;

	// Modif sf@2002 - FileTransfer
	m_filezipbuf = NULL;
	m_filezipbufsize = 0;
	m_filechunkbuf = NULL;
	m_filechunkbufsize = 0;

	// Modif sf@2002 - Scaling
	m_pendingScaleChange = false;
	m_pendingCacheInit = false;
	m_nServerScale = 1;

	//ms logon
	m_ms_logon=false;

	// sf@2002 - FileTransfer on server
	m_fServerKnowsFileTransfer = false;

	// Auto Mode
	m_nConfig = 0;

	// sf@2002 - Options window flag
	m_fOptionsOpen = false;

	// Tight encoding
	for (int i = 0; i < 4; i++)
	m_tightZlibStreamActive[i] = false;

	m_hwnd=NULL;
	m_hbands=NULL;
	m_hwndTB=NULL;
	m_hwndTBwin=NULL;
	m_hwndMain=NULL;
	m_hwndStatus=NULL;
	m_TrafficMonitor=NULL;
	m_logo_wnd=NULL;
	m_button_wnd=NULL;
	// m_ToolbarEnable=true;
	m_remote_mouse_disable=false;
	m_SWselect=false;

	EncodingStatusWindow = -1;
	OldEncodingStatusWindow = -2;

	m_nStatusTimer = 0;
	skipprompt2=true;
	// UltraFast
	m_hmemdc=NULL;
	m_DIBbits=NULL;
	m_membitmap=NULL;
	m_BigToolbar=false;
	strcpy(m_proxyhost,"");
	KillEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	newtick=0;
	oldtick=0;

	m_zipbuf=NULL;
	m_filezipbuf=NULL;
	m_filechunkbuf=NULL;
	m_zlibbuf=NULL; 
	rcSource=NULL;
	rcMask=NULL;

CATCH_THROW("ClientConnection::Init")
}

// 
// Run() creates the connection if necessary, does the initial negotiations
// and then starts the thread running which does the output (update) processing.
// If Run throws an Exception, the caller must delete the ClientConnection object.
//
LRESULT CALLBACK KeyWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

void ClientConnection::Run()
{
TRY_CATCH


	HWND parentWindow = m_hwndMain; /*GetParent(mainWindow)*/;
	m_oldWndProc = (WNDPROC)GetWindowLong( parentWindow, GWL_WNDPROC );
	//SetWindowLong( parentWindow , GWL_WNDPROC , (LONG)KeyWndProc );
	SetWindowLong( parentWindow , GWL_WNDPROC , (LONG)ClientConnection::WndProc );
	SetWindowLong(parentWindow, GWL_USERDATA, (LONG) this);
	
	if( m_opts.m_displayMode == FULLSCREEN_MODE )
		m_opts.m_FullScreen = true;

	if( m_opts.m_displayMode == SCALE_MODE )
	{
		m_opts.m_fAutoScaling = true;
		m_opts.m_scaling = true;
		m_fScalingDone = false;
	}
	//m_opts.m_Use8Bit = 1;

	SetSocketOptions();

	GTGBS_CreateDisplay();
	// Set up windows etc 
	CreateDisplay();
	m_pApp->SetDisplayHandle( m_hwnd );	
  
	// This starts the worker thread.
	// The rest of the processing continues in run_undetached.
	LowLevelHook::Initialize(m_hwndMain);

	/// Setting scroll bars
	SetScrollBars(m_ptrViewer->m_hScrollBar, m_ptrViewer->m_vScrollBar);

	//start_undetached();
	m_thread.reset(new boost::thread( boost::bind( &ClientConnection::run_undetached, this )));

	//TODO: remove
	EndDialog(m_hwndStatus,0);

CATCH_THROW("ClientConnection::Run")
}

////////////////////////////////////////////////////////
#include <commctrl.h>
#include <shellapi.h>
#include <lmaccess.h>
#include <lmat.h>
#include <lmalert.h>

//////////////////////////////////////////////////////////


void ClientConnection::CreateDisplay() 
{
TRY_CATCH
#ifdef _WIN32_WCE
	const DWORD winstyle =  WS_CHILD;
#else
	const DWORD winstyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
#endif
	RECT Rmain;
	RECT Rtb;
	GetClientRect(m_hwndMain,&Rmain);
	GetClientRect(m_hwndTBwin,&Rtb);

	WNDCLASS wndclass;

	wndclass.style			= 0;
	wndclass.lpfnWndProc	= ClientConnection::WndProchwnd;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= m_pApp->m_instance;
	wndclass.hIcon			= NULL;
	wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground	=   NULL;
	wndclass.lpszMenuName	= (const TCHAR *) NULL;
	wndclass.lpszClassName	= VWR_WND_CLASS_NAME_VIEWER;

	TRY_CATCH
		RegisterClassForced(wndclass);
	CATCH_LOG()

	m_hwnd = CreateWindowEx(	WS_EX_TOOLWINDOW,
								VWR_WND_CLASS_NAME_VIEWER,
								_T("RCViewer"),
								winstyle ,
								0,
								0,
								Rmain.right - Rmain.left,		// x-size
								Rmain.bottom - Rmain.top,		// y-size
								m_hwndMain,
								NULL,							// Menu handle
								m_pApp->m_instance,
								NULL);

	ShowWindow(m_hwnd, SW_SHOW);

	// record which client created this window
	SetWindowLong(m_hwnd, GWL_USERDATA, (LONG) this);

	// Create a memory DC which we'll use for drawing to
	// the local framebuffer
	m_hBitmapDC = CreateCompatibleDC(NULL);
	m_hCacheBitmapDC = CreateCompatibleDC(NULL);

	// Set a suitable palette up
	if (GetDeviceCaps(m_hBitmapDC, RASTERCAPS) & RC_PALETTE) 
	{
		BYTE buf[sizeof(LOGPALETTE)+216*sizeof(PALETTEENTRY)];
		LOGPALETTE *plp = (LOGPALETTE *) buf;
		int pepos = 0;
		for (int r = 5; r >= 0; r--) {
			for (int g = 5; g >= 0; g--) {
				for (int b = 5; b >= 0; b--) {
					plp->palPalEntry[pepos].peRed   = r * 255 / 5; 	
					plp->palPalEntry[pepos].peGreen = g * 255 / 5;
					plp->palPalEntry[pepos].peBlue  = b * 255 / 5;
					plp->palPalEntry[pepos].peFlags  = NULL;
					pepos++;
				}
			}
		}
		plp->palVersion = 0x300;
		plp->palNumEntries = 216;
		m_hPalette = CreatePalette(plp);
	}

	DrawMenuBar(m_hwndMain);
	TheAccelKeys.SetWindowHandle(m_opts.m_NoHotKeys ? 0 : m_hwndMain);

	// Set up clipboard watching
#ifndef _WIN32_WCE
	// We want to know when the clipboard changes, so
	// insert ourselves in the viewer chain. But doing
	// this will cause us to be notified immediately of
	// the current state.
	// We don't want to send that.
	m_initialClipboardSeen = false;
	m_hwndNextViewer = SetClipboardViewer(m_hwnd); 	
#endif

	//Added by: Lars Werner (http://lars.werner.no)
	if(TitleBar.GetSafeHwnd()==NULL) 
		TitleBar.Create( GetModuleHandle( 0 )/*m_pApp->m_instance*/  , m_hwndMain);

	if (0 == SetTimer(m_hwnd, IDT_STREAM_TIMER , FLUSH_STREAM_INTERVAL, NULL))
		Log.WinError(_ERROR_,"Failed to set IDT_STREAM_TIMER timer ");
	else
		m_streamTimer.reset(IDT_STREAM_TIMER, boost::bind(&ClientConnection::KillInternalTimer, this, _1));

	if (0 == SetTimer(m_hwnd, IDT_ALIVE_TIMER, ALIVE_MESSAGES_INTERVAL, NULL))
		Log.WinError(_ERROR_,"Failed to set IDT_ALIVE_TIMER timer ");
	else
		m_aliveTimer.reset(IDT_ALIVE_TIMER, boost::bind(&ClientConnection::KillInternalTimer, this, _1));

CATCH_THROW("ClientConnection::CreateDisplay")
}

void ClientConnection::SetSocketOptions() 
{
TRY_CATCH

	// Disable Nagle's algorithm
	BOOL nodelayval = TRUE;
	fis = new rdr::FdInStream( m_stream.get() );
	fis->SetDSMMode(false);

CATCH_THROW("ClientConnection::SetSocketOptions")
}


void ClientConnection::NegotiateProtocolVersion()
{
TRY_CATCH

	rfbProtocolVersionMsg pv;

   /* if the connection is immediately closed, don't report anything, so
       that pmw's monitor can make test connections */

    try
	{
		ReadExact(pv, sz_rfbProtocolVersionMsg);
	}
	catch (Exception &c)
	{
		//vnclog.Print(0, _T("Error reading protocol version: %s\n"), c.m_info);
		Log.Add(_ERROR_, "Error reading protocol version: %s", c.m_info);

		if (m_fUsePlugin)
			throw WarningException("Connection failed - Error reading Protocol Version\r\n\n\r"
									"Possible causes:\r\r"
									"- You've forgotten to select a DSMPlugin and the Server uses a DSMPlugin\r\n"
									"- The selected DSMPlugin is not compatible with the one running on the Server\r\n"
									"- The selected DSMPlugin is not correctly configured (also possibly on the Server)\r\n"
									"- The password you've possibly entered is incorrect\r\n"
									);
		else
			throw WarningException("Connection failed - Error reading Protocol Version\r\n\n\r"
									"Possible causes:\r\r"
									"- You've forgotten to select a DSMPlugin and the Server uses a DSMPlugin\r\n"
									"- Viewer and Server are not compatible (they use different RFB protocoles)\r\n"
									"- Bad connection\r\n"
									);

		throw QuietException(c.m_info);
	}

    pv[sz_rfbProtocolVersionMsg] = 0;

	// XXX This is a hack.  Under CE we just return to the server the
	// version number it gives us without parsing it.  
	// Too much hassle replacing sscanf for now. Fix this!
#ifdef UNDER_CE
	m_majorVersion = rfbProtocolMajorVersion;
	m_minorVersion = rfbProtocolMinorVersion;
#else
    if (sscanf(pv,rfbProtocolVersionFormat,&m_majorVersion,&m_minorVersion) != 2)
	{
		if (m_fUsePlugin)
			throw WarningException("Connection failed - Invalid protocol !\r\n\r\n"
									"Possible causes:\r\r"
									"- You've forgotten to select a DSMPlugin and the Server uses a DSMPlugin\r\n"
									"- The selected DSMPlugin is not compatible with the one running on the Server\r\n"
									"- The selected DSMPlugin is not correctly configured (also possibly on the Server)\r\n"
									"- The password you've possibly entered is incorrect\r\n"
									);
		else
			throw WarningException("Connection failed - Invalid protocol !\r\n\r\n"
									"Possible causes:\r\r"
									"- You've forgotten to select a DSMPlugin and the Server uses a DSMPlugin\r\n"
									"- Viewer and Server are not compatible (they use different RFB protocoles)\r\n"
									);
    }

    //vnclog.Print(0, _T("RFB server supports protocol version %d.%d\n"), m_majorVersion,m_minorVersion);

	// UltraVNC specific functionnalities
	// - ms logon
	// - FileTransfer (TODO: change Minor version in next eSVNC release so it's compatible with Ultra)
	// Minor = 4 means that server supports FileTransfer and requires ms logon
	// Minor = 6 means that server support FileTransfer and requires normal VNC logon
	if (m_minorVersion == 4)
	{
		m_ms_logon = true;
		m_fServerKnowsFileTransfer = true;
	}
	if (m_minorVersion == 6) // 6 because 5 already used in TightVNC viewer for some reason
	{
		m_ms_logon = false;
		m_fServerKnowsFileTransfer = true;
	}
	// Added for SC so we can do something before actual data transfer start
	if (m_minorVersion == 14 || m_minorVersion == 16)
	{
		m_fServerKnowsFileTransfer = true;
	}
    else if ((m_majorVersion == 3) && (m_minorVersion < 3)) 
	{		
        /* if server is 3.2 we can't use the new authentication */
		//vnclog.Print(0, _T("Can't use IDEA authentication\n"));
		Log.Add(_ERROR_, "Can't use IDEA authentication");
        /* This will be reported later if authentication is requested*/
    } 
	else 
	{
        /* any other server version, just tell the server what we want */
		m_majorVersion = rfbProtocolMajorVersion;
		m_minorVersion = rfbProtocolMinorVersion; // always 4 for Ultra Viewer
    }

    sprintf(pv,rfbProtocolVersionFormat, m_majorVersion, m_minorVersion);
#endif

    WriteExact(pv, sz_rfbProtocolVersionMsg);
	if (m_minorVersion == 14 || m_minorVersion == 16)
	{
		int size;
		ReadExact((char *)&size,sizeof(int));
		char mytext[1024]; //10k
		ReadExact(mytext,size);
		mytext[size]=0;

		int returnvalue=MessageBox(NULL,   mytext,"Accept Incoming SC connection", MB_YESNO |  MB_TOPMOST);
		if (returnvalue==IDNO) 
		{
			int nummer=0;
			WriteExact((char *)&nummer,sizeof(int));
			throw WarningException("You refused connection.....");
		}
		else
		{
			int nummer=1;
			WriteExact((char *)&nummer,sizeof(int));

		}
		
	}
CATCH_THROW("ClientConnection::NegotiateProtocolVersion")
}

void ClientConnection::SendClientInit()
{
TRY_CATCH

    rfbClientInitMsg ci;
	ci.shared = m_opts.m_Shared;
    WriteExact((char *)&ci, sz_rfbClientInitMsg); // sf@2002 - RSM Plugin

CATCH_THROW("ClientConnection::SendClientInit")
}

void ClientConnection::ReadServerInit()
{
TRY_CATCH

	m_stream->Receive((char *)&m_si, sz_rfbServerInitMsg);
	m_ptrViewer->SetServerInitMsg(&m_si);
    m_si.framebufferWidth = Swap16IfLE(m_si.framebufferWidth);
    m_si.framebufferHeight = Swap16IfLE(m_si.framebufferHeight);
	m_ptrViewer->SetRemoteDesktopSize(m_si.framebufferWidth, m_si.framebufferHeight);
    m_si.format.redMax = Swap16IfLE(m_si.format.redMax);
    m_si.format.greenMax = Swap16IfLE(m_si.format.greenMax);
    m_si.format.blueMax = Swap16IfLE(m_si.format.blueMax);
    m_si.nameLength = Swap32IfLE(m_si.nameLength);
	
    m_desktopName = new TCHAR[m_si.nameLength + 4 + 256];

#ifdef UNDER_CE
    char *deskNameBuf = new char[m_si.nameLength + 4];

	//ReadString(deskNameBuf, m_si.nameLength);
	m_stream->Receive(deskNameBuf, m_si.nameLength);
	if (m_si.nameLength)
		deskNameBuf[m_si.nameLength] = '\0';
    
	MultiByteToWideChar( CP_ACP,   MB_PRECOMPOSED, 
			     deskNameBuf, m_si.nameLength,
			     m_desktopName, m_si.nameLength+1);
    delete deskNameBuf;
#else
    //ReadString(m_desktopName, m_si.nameLength);
	m_stream->Receive(m_desktopName, m_si.nameLength);
	if (m_si.nameLength)
		m_desktopName[m_si.nameLength] = '\0';
#endif
	strcat(m_desktopName, " ");
	//SetWindowText(m_hwndMain, m_desktopName);	See http://srv-dev/jira/browse/STL-221
	m_ptrViewer->SetDisplayName(m_desktopName);
	SizeWindow();

CATCH_THROW("ClientConnection::ReadServerInit")
}


void ClientConnection::SizeWindow()
{
TRY_CATCH

	//TODO: should be optimized, since it's called even after window moving, not only resize

	if( !m_si.framebufferHeight && !m_si.framebufferWidth )
		return;
	
	//LONG st=0;
	//st = GetWindowLong(m_hwnd, GWL_STYLE) & WS_HSCROLL;
	//st = GetWindowLong(m_hwndMain, GWL_STYLE) & WS_HSCROLL;

	// Find how large the desktop work area is
	RECT workrect={0};
	//SystemParametersInfo(SPI_GETWORKAREA, 0, &workrect, 0);
	GetClientRect( m_hwnd  , &workrect );
	/*GetWindowRect( m_hwnd  , &workrect );*/
	int workwidth = workrect.right -  workrect.left;
	int workheight = workrect.bottom - workrect.top;
	//vnclog.Print(2, _T("Screen work area is %d x %d\n"), workwidth, workheight);

	// sf@2003 - AutoScaling 
	if (m_opts.m_fAutoScaling && !m_fScalingDone)
	{
		// We save the scales values coming from options
		m_opts.m_scaling = true;
		m_opts.m_saved_scale_num = m_opts.m_scale_num;
		m_opts.m_saved_scale_den = m_opts.m_scale_den;
		m_opts.m_saved_scaling = m_opts.m_scaling;

		NONCLIENTMETRICS ncm = {0};
		ncm.cbSize = sizeof(ncm);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0); 
		int TitleBarHeight = ncm.iCaptionHeight + 10;
		int MenuBarHeight = ncm.iMenuHeight + 1;
		
		int nLocalHeight = workheight; 
		int nLocalWidth = workwidth;
		//nLocalHeight -= TitleBarHeight;
		//if (m_opts.m_ShowToolbar)
		//nLocalHeight -= (m_TBr.bottom); // Always take toolbar into account in calculation
		//m_opts.m_scale_num = (int)((nLocalHeight * 100) / m_si.framebufferHeight);
		m_opts.m_scale_num = (float(nLocalWidth * 100) / max(1,m_si.framebufferWidth));
		m_opts.m_scale_num_y = (float(nLocalHeight * 100) / max(1,m_si.framebufferHeight));
		m_opts.m_dx = (float(nLocalWidth * 100) / float(max(1,m_si.framebufferWidth)));
		m_opts.m_dy = (float(nLocalHeight * 100) / float(max(1,m_si.framebufferHeight)));

		//m_opts.m_scale_num = 100;
		m_opts.m_scale_den = 100;
		m_opts.m_scaling = true; 
		m_fScalingDone = true;
	}
	
	if (!m_opts.m_fAutoScaling && m_fScalingDone)
	{
		// Restore scale values to the original options values
		m_opts.m_scale_num = m_opts.m_saved_scale_num;
		m_opts.m_scale_den = m_opts.m_saved_scale_den;
		m_opts.m_scaling = m_opts.m_saved_scaling;
		m_fScalingDone = false;
	}

	// Size the window.
	// Let's find out how big a window would be needed to display the
	// whole desktop (assuming no scrollbars).

	RECT fullwinrect;

	if (m_opts.m_scaling)
	{
		int height = int(float(m_si.framebufferHeight * m_opts.m_dy ) / float(m_opts.m_scale_den));
		int width = int(float(m_si.framebufferWidth * m_opts.m_scale_num) / float(m_opts.m_scale_den));
/*
		SetRect(&fullwinrect, 0, 0,
				m_si.framebufferWidth * m_opts.m_scale_num / m_opts.m_scale_den,
				m_si.framebufferHeight * m_opts.m_scale_num_y / m_opts.m_scale_den);
*/

		SetRect(&fullwinrect, 0, 0,	width,height);

	}
	else 
		SetRect(&fullwinrect, 0, 0, m_si.framebufferWidth, m_si.framebufferHeight);
		//SetRect(&fullwinrect, 0, 0,	width,height);

	AdjustWindowRectEx(	&fullwinrect, 
						GetWindowLong(m_hwnd, GWL_STYLE) & ~WS_VSCROLL & ~WS_HSCROLL, 
						FALSE, GetWindowLong(m_hwnd, GWL_EXSTYLE));


	m_fullwinwidth = fullwinrect.right - fullwinrect.left;
	m_fullwinheight = (fullwinrect.bottom - fullwinrect.top);

	m_winwidth  = min(m_fullwinwidth,  workwidth);
	m_winheight = min(m_fullwinheight, workheight);

	AdjustWindowRectEx(&fullwinrect, 
						GetWindowLong(m_hwndMain, GWL_STYLE) & ~WS_VSCROLL & ~WS_HSCROLL, 
						FALSE, GetWindowLong(m_hwndMain, GWL_EXSTYLE));

	m_fullwinwidth = fullwinrect.right - fullwinrect.left;
	m_fullwinheight = (fullwinrect.bottom - fullwinrect.top);

	//m_winwidth  = min(m_fullwinwidth+16,  workwidth);
	m_winwidth  = min(m_fullwinwidth,  workwidth);
	//m_winheight = min(m_fullwinheight+m_TBr.bottom + m_TBr.top+16 , workheight);
	if (m_opts.m_ShowToolbar)
		m_winheight = min(m_fullwinheight + m_TBr.bottom + m_TBr.top , workheight);
	else
		m_winheight = min(m_fullwinheight, workheight);

	SetForegroundWindow(m_hwndMain);

CATCH_THROW("ClientConnection::SizeWindow")
}

// We keep a local copy of the whole screen.  This is not strictly necessary
// for VNC, but makes scrolling & deiconifying much smoother.

void ClientConnection::CreateLocalFramebuffer()
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_bitmapdcMutex);
	
	// We create a bitmap which has the same pixel characteristics as
	// the local display, in the hope that blitting will be faster.

	TempDC hdc(m_hwnd);

	if (m_hBitmap != NULL)
		DeleteObject(m_hBitmap);

	m_hBitmap = CreateCompatibleBitmap(hdc, m_si.framebufferWidth, m_si.framebufferHeight);
	if (m_hBitmap == NULL)
		throw MCException("failed to CreateCompatibleBitmap");
	// Select this bitmap into the DC with an appropriate palette
	ObjectSelector b(m_hBitmapDC, m_hBitmap);
	PaletteSelector p(m_hBitmapDC, m_hPalette);
	// Modif RDV@2002 - Cache Encoding
	// Modif sf@2002
	if (m_opts.m_fEnableCache)
	{
		if (m_hCacheBitmap != NULL) DeleteObject(m_hCacheBitmap);
		m_hCacheBitmap = CreateCompatibleBitmap(m_hBitmapDC, m_si.framebufferWidth, m_si.framebufferHeight);
		//vnclog.Print(0, _T("Cache: Cache buffer bitmap creation\n"));
	}
	
	RECT rect;

	SetRect(&rect, 0,0, m_si.framebufferWidth, m_si.framebufferHeight);
	COLORREF bgcol = RGB(0, 0, 50);
	FillSolidRect(&rect, bgcol);
	
	COLORREF oldbgcol  = SetBkColor(  m_hBitmapDC, bgcol);
	COLORREF oldtxtcol = SetTextColor(m_hBitmapDC, RGB(255,255,255));
	rect.right = m_si.framebufferWidth / 2;
	rect.bottom = m_si.framebufferHeight / 2;
	
	DrawText (m_hBitmapDC, sz_L62, -1, &rect,
		DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	SetBkColor(  m_hBitmapDC, oldbgcol);
	SetTextColor(m_hBitmapDC, oldtxtcol);
	InvalidateRect(m_hwnd, NULL, FALSE);

CATCH_THROW("ClientConnection::CreateLocalFramebuffer")
}

rfbPixelFormat ClientConnection::SelectPixelFormat(int ColorsCount)
{
TRY_CATCH

	rfbPixelFormat m_myFormat(ClientConnection::m_myFormat);
	// Have we requested a reduction to 8-bit?
	if (ColorsCount)
	{		
		switch (ColorsCount)
		{
		case rfbPF256Colors:
			m_myFormat = vnc8bitFormat;
			break;
		case rfbPF64Colors:
			m_myFormat = vnc8bitFormat_64;
			break;
		case rfbPF8Colors:
			m_myFormat = vnc8bitFormat_8;
			break;
		case rfbPF8GreyColors:
			m_myFormat = vnc8bitFormat_8Grey;
			break;
		case rfbPF4GreyColors:
			m_myFormat = vnc8bitFormat_4Grey;
			break;
		case rfbPF2GreyColors:
			m_myFormat = vnc8bitFormat_2Grey;
			break;
		}
		//vnclog.Print(2, _T("Requesting 8-bit truecolour\n"));  
		// We don't support colormaps so we'll ask the server to convert
	}
	else if (true || !m_si.format.trueColour) //TODO: check if should be done on server side
	{
		// We'll just request a standard 16-bit truecolor
		//vnclog.Print(2, _T("Requesting 16-bit truecolour\n"));
		m_myFormat = vnc16bitFormat;
	}
	else
	{

		// Normally we just use the sever's format suggestion
		m_myFormat = m_si.format;
		m_myFormat.bigEndian = 0; // except always little endian

		// It's silly requesting more bits than our current display has, but
		// in fact it doesn't usually amount to much on the network.
		// Windows doesn't support 8-bit truecolour.
		// If our display is palette-based, we want more than 8 bit anyway,
		// unless we're going to start doing palette stuff at the server.
		// So the main use would be a 24-bit true-colour desktop being viewed
		// on a 16-bit true-colour display, and unless you have lots of images
		// and hence lots of raw-encoded stuff, the size of the pixel is not
		// going to make much difference.
		//   We therefore don't bother with any restrictions, but here's the
		// start of the code if we wanted to do it.
		
	}
	// The endian will be set before sending
	return m_myFormat;

CATCH_THROW("ClientConnection::SelectPixelFormat")
}

void ClientConnection::SetupPixelFormat() 
{
TRY_CATCH

	m_myFormat = SelectPixelFormat(m_opts.m_Use8Bit);

CATCH_THROW("ClientConnection::SetupPixelFormat")
}


void ClientConnection::SetFormatAndEncodings(bool sendColorsFormat)
{
TRY_CATCH

	if (sendColorsFormat)
	{
		// Set pixel format to myFormat
		rfbSetPixelFormatMsg spf;
		spf.type = rfbSetPixelFormat;
		spf.format = m_myFormat;
		spf.format.redMax = Swap16IfLE(spf.format.redMax);
		spf.format.greenMax = Swap16IfLE(spf.format.greenMax);
		spf.format.blueMax = Swap16IfLE(spf.format.blueMax);
		WriteExact((char *)&spf, sz_rfbSetPixelFormatMsg, rfbSetPixelFormat);
	}

    // The number of bytes required to hold at least one pixel.
	m_minPixelBytes = (m_myFormat.bitsPerPixel + 7) >> 3;

	// Set encodings
    char buf[sz_rfbSetEncodingsMsg + MAX_ENCODINGS * 4];
    rfbSetEncodingsMsg *se = (rfbSetEncodingsMsg *)buf;
    CARD32 *encs = (CARD32 *)(&buf[sz_rfbSetEncodingsMsg]);
    int len = 0;
	
    se->type = rfbSetEncodings;
    se->nEncodings = 0;

	bool useCompressLevel = false;
	int i = 0;
	// Send the preferred encoding first, and change it if the
	// preferred encoding is not actually usable.
	Log.Add(_MESSAGE_,_T("Sending new preffered encoding - %d"), m_opts.m_PreferredEncoding);
	for (i = LASTENCODING; i >= rfbEncodingRaw; i--)
	{
		if (m_opts.m_PreferredEncoding == i) {
			if (m_opts.m_UseEnc[i])
			{
				encs[se->nEncodings++] = Swap32IfLE(i);
	  			if ( i == rfbEncodingZlib ||
					 i == rfbEncodingTight ||
					 i == rfbEncodingZlibHex
			   )
				{
					useCompressLevel = true;
				}
			}
			else 
			{
				m_opts.m_PreferredEncoding--;
			}
		}
	}

	// Now we go through and put in all the other encodings in order.
	// We do rather assume that the most recent encoding is the most
	// desirable!
	for (i = LASTENCODING; i >= rfbEncodingRaw; i--)
	{
		if ( (m_opts.m_PreferredEncoding != i) &&
			 (m_opts.m_UseEnc[i]))
		{
			encs[se->nEncodings++] = Swap32IfLE(i);
			if ( i == rfbEncodingZlib ||
				 i == rfbEncodingTight ||
				 i == rfbEncodingZlibHex
				)
			{
				useCompressLevel = true;
			}
		}
	}

	// Tight - Request desired compression level if applicable
	if ( useCompressLevel && m_opts.m_useCompressLevel &&
		 m_opts.m_compressLevel >= 0 &&
		 m_opts.m_compressLevel <= 9) {
		encs[se->nEncodings++] = Swap32IfLE( rfbEncodingCompressLevel0 +
											 m_opts.m_compressLevel );
	}

	// Tight - Request cursor shape updates if enabled by user
	if (m_opts.m_requestShapeUpdates) {
		encs[se->nEncodings++] = Swap32IfLE(rfbEncodingXCursor);
		encs[se->nEncodings++] = Swap32IfLE(rfbEncodingRichCursor);
		if (!m_opts.m_ignoreShapeUpdates)
			encs[se->nEncodings++] = Swap32IfLE(rfbEncodingPointerPos); // marscha PointerPos
	}

	// Tight - Request JPEG quality level if JPEG compression was enabled by user
	if ( m_opts.m_enableJpegCompression &&
		 m_opts.m_jpegQualityLevel >= 0 &&
		 m_opts.m_jpegQualityLevel <= 9) {
		encs[se->nEncodings++] = Swap32IfLE( rfbEncodingQualityLevel0 +
											 m_opts.m_jpegQualityLevel );
	}

    // Modif rdv@2002
	//Tell the server that we support the special Zlibencoding
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingXOREnable);

	// Tight - LastRect - SINGLE WINDOW
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingLastRect);
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingNewFBSize);

	// Modif sf@2002
	if (m_opts.m_fEnableCache)
	{
		encs[se->nEncodings++] = Swap32IfLE(rfbEncodingCacheEnable);
		// vnclog.Print(0, _T("Cache: Enable Cache sent to Server\n"));
	}

      // sf@2002 - DSM Plugin
	int nEncodings = se->nEncodings;
	se->nEncodings = Swap16IfLE(se->nEncodings);
	len = sz_rfbSetEncodingsMsg + nEncodings * sizeof(CARD32);
	WriteExact((char *)buf, len);
	/*WriteExact((char *)buf, sz_rfbSetEncodingsMsg, rfbSetEncodings);
	for (int x = 0; x < nEncodings; x++)
	{ 
		WriteExact((char *)&encs[x], sizeof(CARD32));
	}*/
CATCH_THROW("ClientConnection::SetFormatAndEncodings")
}


void ClientConnection::Createdib()
{
TRY_CATCH

	boost::recursive_mutex::scoped_lock l(m_bitmapdcMutex);
	TempDC hdc(m_hwnd);
	BitmapInfo bi;
	UINT iUsage;
    memset(&bi, 0, sizeof(bi));
	
	iUsage = m_myFormat.trueColour ? DIB_RGB_COLORS : DIB_PAL_COLORS;
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biBitCount = m_myFormat.bitsPerPixel;
    bi.bmiHeader.biSizeImage = (m_myFormat.bitsPerPixel / 8) * m_si.framebufferWidth * m_si.framebufferHeight;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biWidth = m_si.framebufferWidth;
    bi.bmiHeader.biHeight = -m_si.framebufferHeight;
    bi.bmiHeader.biCompression = (m_myFormat.bitsPerPixel > 8) ? BI_BITFIELDS : BI_RGB;
    bi.mask.red = m_myFormat.redMax << m_myFormat.redShift;
    bi.mask.green = m_myFormat.greenMax << m_myFormat.greenShift;
    bi.mask.blue = m_myFormat.blueMax << m_myFormat.blueShift;

	if (m_hmemdc != NULL) {DeleteDC(m_hmemdc);m_hmemdc = NULL;m_DIBbits=NULL;}
	if (m_membitmap != NULL) {DeleteObject(m_membitmap);m_membitmap= NULL;}
//	m_hmemdc = CreateCompatibleDC(hdc);
	m_hmemdc = CreateCompatibleDC(m_hBitmapDC);
	m_membitmap = CreateDIBSection(m_hmemdc, (BITMAPINFO*)&bi.bmiHeader, iUsage, &m_DIBbits, NULL, 0);

	ObjectSelector bb(m_hmemdc, m_membitmap);

	int i;
	if (m_myFormat.bitsPerPixel==8 && m_myFormat.trueColour)
	{
		struct Colour {
		int r, g, b;
		};
		Colour rgbQ[256];
        /*UINT num_entries;
		num_entries =GetPaletteEntries(m_hPalette, 0, 0, NULL);              
        size_t pal_size = sizeof(LOGPALETTE) +(num_entries - 1) * sizeof(PALETTEENTRY);
        LOGPALETTE* pLogPal =(LOGPALETTE*) new unsigned char[pal_size];
        UINT num_got = GetPaletteEntries( m_hPalette, 0, num_entries, pLogPal->palPalEntry);
          for (UINT i=0; i<num_got; ++i)
          {
            rgbQ[i].rgbRed = pLogPal->palPalEntry[i].peRed;
            rgbQ[i].rgbGreen = pLogPal->palPalEntry[i].peGreen;
            rgbQ[i].rgbBlue = pLogPal-> palPalEntry[i].peBlue;
          }

         delete [] pLogPal;*/

		 for (i=0; i < (1<<(m_myFormat.depth)); i++) {
			rgbQ[i].b = ((((i >> m_myFormat.blueShift) & m_myFormat.blueMax) * 65535) + m_myFormat.blueMax/2) / m_myFormat.blueMax;
			rgbQ[i].g = ((((i >> m_myFormat.greenShift) & m_myFormat.greenMax) * 65535) + m_myFormat.greenMax/2) / m_myFormat.greenMax;
			rgbQ[i].r = ((((i >> m_myFormat.redShift) & m_myFormat.redMax) * 65535) + m_myFormat.redMax/2) / m_myFormat.redMax;
		 }

	for (i=0; i<256; i++)
	{
		bi.color[i].rgbRed      = rgbQ[i].r >> 8;
		bi.color[i].rgbGreen    = rgbQ[i].g >> 8;
		bi.color[i].rgbBlue     = rgbQ[i].b >> 8;
		bi.color[i].rgbReserved = 0;
	}
	SetDIBColorTable(m_hmemdc, 0, 256, bi.color);
	}

CATCH_THROW("ClientConnection::Createdib")
}
// Closing down the connection.
// Close the socket, kill the thread.
void ClientConnection::KillThread()
{
TRY_CATCH

	m_bKillThread = true;
	m_running = false;
	m_restarting = false;

	TRY_CATCH
		m_stream->CancelReceiveOperation();
	CATCH_LOG()
	/// Unblock read operations
	WaitForSingleObject(KillEvent, 100000);

CATCH_THROW("ClientConnection::KillThread")
}


ClientConnection::~ClientConnection()
{
TRY_CATCH

	TurnOffFullScreen();

	if (m_pNetRectBuf != NULL)
	delete [] m_pNetRectBuf;
	LowLevelHook::Release();

	if (zis)
	delete zis;

	if (fis)
	delete fis;

	if (m_pZRLENetRectBuf != NULL)
		delete [] m_pZRLENetRectBuf;

	if (m_desktopName != NULL) delete [] m_desktopName;
		delete [] m_netbuf;

	if (m_hCacheBitmapDC != NULL)
		DeleteDC(m_hCacheBitmapDC);
	if (m_hCacheBitmapDC != NULL)
		DeleteObject(m_hCacheBitmapDC);
	if (m_hCacheBitmap != NULL)
		DeleteObject(m_hCacheBitmap);

	if (m_hBitmapDC != NULL)
		DeleteDC(m_hBitmapDC);
	if (m_hBitmapDC != NULL)
		DeleteObject(m_hBitmapDC);
	if (m_hBitmap != NULL)
		DeleteObject(m_hBitmap);

	if (m_hPalette != NULL)
		DeleteObject(m_hPalette);
	//UltraFast
	if (m_hmemdc != NULL) 
	{
		DeleteDC(m_hmemdc);
		m_hmemdc = NULL;
		m_DIBbits=NULL;
	}
	if (m_membitmap != NULL) 
	{
		DeleteObject(m_membitmap);
		m_membitmap = NULL;
	}
	//	if (flash) delete flash;
	if (m_zipbuf!=NULL)
		delete [] m_zipbuf;
	if (m_filezipbuf!=NULL)
		delete [] m_filezipbuf;
	if (m_filechunkbuf!=NULL)
		delete [] m_filechunkbuf;
	if (m_zlibbuf!=NULL)
		delete [] m_zlibbuf;
	if (m_hwndTBwin!= 0)
		DestroyWindow(m_hwndTBwin);
	if (rcSource!=NULL)
		delete[] rcSource;
	if (rcMask!=NULL)
		delete[] rcMask;

	CloseHandle(KillEvent);
	UnregisterClass(VWR_WND_CLASS_NAME_VIEWER, GetModuleHandle(NULL));
	UnregisterClass(VNCMDI_CLASS_NAME, GetModuleHandle(NULL));

CATCH_LOG("ClientConnection::~ClientConnection")
}

// You can specify a dx & dy outside the limits; the return value will
// tell you whether it actually scrolled.
bool ClientConnection::ScrollScreen(int dx, int dy) 
{
TRY_CATCH

	dx = max(dx, -m_hScrollPos);
	dx = min(dx, m_hScrollMax-(m_cliwidth)-m_hScrollPos);
	dy = max(dy, -m_vScrollPos);
	dy = min(dy, m_vScrollMax-(m_cliheight)-m_vScrollPos);
	if (dx || dy)
	{
		m_hScrollPos += dx;
		m_vScrollPos += dy;
		RECT clirect;
		RECT Rtb;
		GetClientRect(m_hwndMain, &clirect);
		if (NULL != m_vertScrollBar)
		{
			RECT rc;
			GetWindowRect(m_vertScrollBar,&rc);
			clirect.right -= (rc.right - rc.left);
		}
		Rtb.top=0;
		Rtb.bottom=0;
		
		clirect.top += Rtb.top;
		clirect.bottom += Rtb.bottom;
		ScrollWindowEx(m_hwnd, -dx, -dy, NULL, &clirect, NULL, NULL,  SW_INVALIDATE);
		UpdateScrollbars();
		UpdateWindow(m_hwnd);
		
		return true;
	}
	return false;

CATCH_THROW("ClientConnection::ScrollScreen")
}



// ProcessPointerEvent handles the delicate case of emulating 3 buttons
// on a two button mouse, then passes events off to SubProcessPointerEvent.
inline void ClientConnection::ProcessPointerEvent(int x, int y, DWORD keyflags, UINT msg) 
{
TRY_CATCH

	if (!m_inited)
		return;

	if (m_opts.m_Emul3Buttons) {
		// XXX To be done:
		// If this is a left or right press, the user may be 
		// about to press the other button to emulate a middle press.
		// We need to start a timer, and if it expires without any
		// further presses, then we send the button press. 
		// If a press of the other button, or any release, comes in
		// before timer has expired, we cancel timer & take different action.
	  if (m_waitingOnEmulateTimer)
	    {
	      if (msg == WM_LBUTTONUP || msg == WM_RBUTTONUP ||
		  abs(x - m_emulateButtonPressedX) > m_opts.m_Emul3Fuzz ||
		  abs(y - m_emulateButtonPressedY) > m_opts.m_Emul3Fuzz)
		{
		  // if button released or we moved too far then cancel.
		  // First let the remote know where the button was down
		  SubProcessPointerEvent(
					 m_emulateButtonPressedX, 
					 m_emulateButtonPressedY, 
					 m_emulateKeyFlags);
		  // Then tell it where we are now
		  SubProcessPointerEvent(x, y, keyflags);
		}
	      else if (
		       (msg == WM_LBUTTONDOWN && (m_emulateKeyFlags & MK_RBUTTON))
		       || (msg == WM_RBUTTONDOWN && (m_emulateKeyFlags & MK_LBUTTON)))
		{
		  // Triggered an emulate; remove left and right buttons, put
		  // in middle one.
		  DWORD emulatekeys = keyflags & ~(MK_LBUTTON|MK_RBUTTON);
		  emulatekeys |= MK_MBUTTON;
		  SubProcessPointerEvent(x, y, emulatekeys);
		  
		  m_emulatingMiddleButton = true;
		}
	      else
		{
		  // handle movement normally & don't kill timer.
		  // just remove the pressed button from the mask.
		  DWORD keymask = m_emulateKeyFlags & (MK_LBUTTON|MK_RBUTTON);
		  DWORD emulatekeys = keyflags & ~keymask;
		  SubProcessPointerEvent(x, y, emulatekeys);
		  return;
		}
	      
	      // if we reached here, we don't need the timer anymore.
	      KillTimer(m_hwnd, m_emulate3ButtonsTimer);
	      m_waitingOnEmulateTimer = false;
	    }
	  else if (m_emulatingMiddleButton)
	    {
	      if ((keyflags & MK_LBUTTON) == 0 && (keyflags & MK_RBUTTON) == 0)
		{
		  // We finish emulation only when both buttons come back up.
		  m_emulatingMiddleButton = false;
		  SubProcessPointerEvent(x, y, keyflags);
		}
	      else
		{
		  // keep emulating.
		  DWORD emulatekeys = keyflags & ~(MK_LBUTTON|MK_RBUTTON);
		  emulatekeys |= MK_MBUTTON;
		  SubProcessPointerEvent(x, y, emulatekeys);
		}
	    }
	  else
	    {
	      // Start considering emulation if we've pressed a button
	      // and the other isn't pressed.
	      if ( (msg == WM_LBUTTONDOWN && !(keyflags & MK_RBUTTON))
		   || (msg == WM_RBUTTONDOWN && !(keyflags & MK_LBUTTON)))
		{
		  // Start timer for emulation.
		  m_emulate3ButtonsTimer = 
		    SetTimer(
			     m_hwnd, 
			     IDT_EMULATE3BUTTONSTIMER, 
			     m_opts.m_Emul3Timeout, 
			     NULL);
		  
		  if (!m_emulate3ButtonsTimer)
		    {
			  Log.Add(_ERROR_,_T("Failed to create timer for emulating 3 buttons"));
		      PostMessage(m_hwndMain, WM_CLOSE, 0, 0);
		      return;
		    }
		  
		  m_waitingOnEmulateTimer = true;
		  
		  // Note that we don't send the event here; we're batching it for
		  // later.
		  m_emulateKeyFlags = keyflags;
		  m_emulateButtonPressedX = x;
		  m_emulateButtonPressedY = y;
		}
	      else
		{
		  // just send event noramlly
		  SubProcessPointerEvent(x, y, keyflags);
		}
	    }
 	}
	else
	  {
	    SubProcessPointerEvent(x, y, keyflags);
	  }

CATCH_THROW("ClientConnection::ProcessPointerEvent")
}

// SubProcessPointerEvent takes windows positions and flags and converts 
// them into VNC ones.

inline void ClientConnection::SubProcessPointerEvent(int x, int y, DWORD keyflags)
{
TRY_CATCH

	int mask;
	if (m_opts.m_SwapMouse) 
	{
		mask = ( ((keyflags & MK_LBUTTON) ? rfbButton1Mask : 0) |
				 ((keyflags & MK_MBUTTON) ? rfbButton3Mask : 0) |
				 ((keyflags & MK_RBUTTON) ? rfbButton2Mask : 0) );
	} 
	else 
	{
		mask = ( ((keyflags & MK_LBUTTON) ? rfbButton1Mask : 0) |
				 ((keyflags & MK_MBUTTON) ? rfbButton2Mask : 0) |
				 ((keyflags & MK_RBUTTON) ? rfbButton3Mask : 0) );
	}

	if ((short)HIWORD(keyflags) > 0) 
	{
		mask |= rfbButton4Mask;
	} else 
		if ((short)HIWORD(keyflags) < 0) 
		{
			mask |= rfbButton5Mask;
		}

	try 
	{
		int x_scaled;
		int y_scaled;
		switch(m_opts.m_displayMode)
		{
			case SCALE_MODE:
				if (m_opts.m_scaleUpEnabled || 
					m_opts.m_scale_den/m_opts.m_dx > 1.0 || 
					m_opts.m_scale_den/m_opts.m_dy > 1.0)
				{
					x_scaled =	x * m_opts.m_scale_den / m_opts.m_scale_num;
					y_scaled =	y * m_opts.m_scale_den / m_opts.m_scale_num_y;
				} else
				{
					RECT rc;
					GetClientRect(m_hwnd, &rc);
					int dx = ((rc.right-rc.left) - m_si.framebufferWidth)/2;
					int dy = ((rc.bottom-rc.top) - m_si.framebufferHeight)/2;
					x_scaled = x - dx;
					y_scaled = y - dy;
				}
				break;
			case SCROLL_MODE:
				x_scaled =	x + m_hScrollPos;
				y_scaled =	y + m_vScrollPos;
				break;
			default:
				if (m_opts.m_scaling)
				{
					if (m_opts.m_scaleUpEnabled || 
						m_opts.m_scale_den/m_opts.m_dx > 1.0 || 
						m_opts.m_scale_den/m_opts.m_dy > 1.0)
					{
						x_scaled =	x * m_opts.m_scale_den / m_opts.m_scale_num;
						y_scaled =	y * m_opts.m_scale_den / m_opts.m_scale_num_y;
					} else
					{
						RECT rc;
						GetClientRect(m_hwnd, &rc);
						int dx = ((rc.right-rc.left) - m_si.framebufferWidth)/2;
						int dy = ((rc.bottom-rc.top) - m_si.framebufferHeight)/2;
						x_scaled = x - dx;
						y_scaled = y - dy;
					}
				} else
				{
					x_scaled =	x + m_hScrollPos;
					y_scaled =	y + m_vScrollPos;
				}
				break;
		}
		SendPointerEvent(x_scaled, y_scaled, mask);
		if ((short)HIWORD(keyflags) != 0) 
		{
			// Immediately send a "button-up" after mouse wheel event.
			mask &= !(rfbButton4Mask | rfbButton5Mask);
			SendPointerEvent(x_scaled, y_scaled, mask);
		}
	} catch (Exception &e) 
	{
		e.Report();
		PostMessage(m_hwndMain, WM_CLOSE, 0, 0);
	}

CATCH_THROW("ClientConnection::SubProcessPointerEvent")
}

inline void ClientConnection::ProcessMouseWheel(int delta)
{
TRY_CATCH

	if (!m_inited)
		return;
	int wheelMask = rfbWheelUpMask;
	if (delta < 0) 
	{
		wheelMask = rfbWheelDownMask;
		delta = -delta;
	}
	while (delta > 0) 
	{
		SendPointerEvent(oldPointerX, oldPointerY, oldButtonMask | wheelMask);
		SendPointerEvent(oldPointerX, oldPointerY, oldButtonMask & ~wheelMask);
		delta -= 120;
	}
CATCH_THROW("ClientConnection::ProcessMouseWheel")
}

inline void ClientConnection::SendPointerEvent(int x, int y, int buttonMask)
{
TRY_CATCH
	rfbPointerEventMsg pe;
	oldPointerX = x;
	oldPointerY = y;
	oldButtonMask = buttonMask;
	pe.type = rfbPointerEvent;
	pe.buttonMask = buttonMask;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	// tight cursor handling
	if (m_ptrViewer && !m_ptrViewer->m_viewOnly)
		SoftCursorMove(x, y);
	pe.x = Swap16IfLE(x);
	pe.y = Swap16IfLE(y);
	if (m_nConfig != 0 && m_nConfig != 1)
		m_asyncStream.FastSend2Queue((char *)&pe, sz_rfbPointerEventMsg); // Better on slow connections
	else
		WriteExact((char *)&pe, sz_rfbPointerEventMsg); // Better on fast connections*/
CATCH_THROW("ClientConnection::SendPointerEvent")
}

//
// ProcessKeyEvent
//
// Normally a single Windows key event will map onto a single RFB
// key message, but this is not always the case.  Much of the stuff
// here is to handle AltGr (=Ctrl-Alt) on international keyboards.
// Example cases:
//
//    We want Ctrl-F to be sent as:
//      Ctrl-Down, F-Down, F-Up, Ctrl-Up.
//    because there is no keysym for ctrl-f, and because the ctrl
//    will already have been sent by the time we get the F.
//
//    On German keyboards, @ is produced using AltGr-Q, which is
//    Ctrl-Alt-Q.  But @ is a valid keysym in its own right, and when
//    a German user types this combination, he doesn't mean Ctrl-@.
//    So for this we will send, in total:
//
//      Ctrl-Down, Alt-Down,   
//                 (when we get the AltGr pressed)
//
//      Alt-Up, Ctrl-Up, @-Down, Ctrl-Down, Alt-Down 
//                 (when we discover that this is @ being pressed)
//
//      Alt-Up, Ctrl-Up, @-Up, Ctrl-Down, Alt-Down
//                 (when we discover that this is @ being released)
//
//      Alt-Up, Ctrl-Up
//                 (when the AltGr is released)

inline void ClientConnection::ProcessKeyEvent(int virtkey, DWORD keyData)
{
TRY_CATCH

    bool down = ((keyData & 0x80000000l) == 0);

    // if virtkey found in mapping table, send X equivalent
    // else
    //   try to convert directly to ascii
    //   if result is in range supported by X keysyms,
    //      raise any modifiers, send it, then restore mods
    //   else
    //      calculate what the ascii would be without mods
    //      send that

#ifdef _DEBUG
#ifdef UNDER_CE
	char *keyname="";
#else
    char keyname[32];
    if (GetKeyNameText(  keyData,keyname, 31)) {
//        vnclog.Print(4, _T("Process key: %s (keyData %04x): virtkey %04x "), keyname, keyData,virtkey);
//		if (virtkey==0x00dd) 
//			vnclog.Print(4, _T("Process key: %s (keyData %04x): virtkey %04x "), keyname, keyData,virtkey);
    };
#endif
#endif

	try {
		KeyActionSpec kas = m_keymap.PCtoX(virtkey, keyData);
		
		if (kas.releaseModifiers & KEYMAP_LCONTROL) 
		{
			SendKeyEvent(XK_Control_L, false );
			//vnclog.Print(5, _T("fake L Ctrl raised\n"));
		}
		if (kas.releaseModifiers & KEYMAP_LALT) 
		{
			SendKeyEvent(XK_Alt_L, false );
			//vnclog.Print(5, _T("fake L Alt raised\n"));
		}
		if (kas.releaseModifiers & KEYMAP_RCONTROL) 
		{
			SendKeyEvent(XK_Control_R, false );
			//vnclog.Print(5, _T("fake R Ctrl raised\n"));
		}
		if (kas.releaseModifiers & KEYMAP_RALT) 
		{
			SendKeyEvent(XK_Alt_R, false );
			//vnclog.Print(5, _T("fake R Alt raised\n"));
		}
		
		for (int i = 0; kas.keycodes[i] != XK_VoidSymbol && i < MaxKeysPerKey; i++) {
			SendKeyEvent(kas.keycodes[i], down );
			//vnclog.Print(4, _T("Sent keysym %04x (%s)\n"), 
			//	kas.keycodes[i], down ? _T("press") : _T("release"));
		}
		
		if (kas.releaseModifiers & KEYMAP_RALT) 
		{
			SendKeyEvent(XK_Alt_R, true );
			//vnclog.Print(5, _T("fake R Alt pressed\n"));
		}
		if (kas.releaseModifiers & KEYMAP_RCONTROL) {
			SendKeyEvent(XK_Control_R, true );
			//vnclog.Print(5, _T("fake R Ctrl pressed\n"));
		}
		if (kas.releaseModifiers & KEYMAP_LALT) {
			SendKeyEvent(XK_Alt_L, false );
			//vnclog.Print(5, _T("fake L Alt pressed\n"));
		}
		if (kas.releaseModifiers & KEYMAP_LCONTROL) {
			SendKeyEvent(XK_Control_L, false );
			//vnclog.Print(5, _T("fake L Ctrl pressed\n"));
		}
	} catch (Exception &e) {
		e.Report();
		PostMessage(m_hwndMain, WM_CLOSE, 0, 0);
	}

CATCH_THROW("ClientConnection::ProcessKeyEvent")
}

//
// SendKeyEvent
//

inline void ClientConnection::SendKeyEvent(CARD32 key, bool down)
{
TRY_CATCH
	
	if (!m_inited)
		return;
	++m_keysSentCount;
	rfbKeyEventMsg ke;
    ke.type = rfbKeyEvent;
    ke.down = down ? 1 : 0;
    ke.key = Swap32IfLE(key);
    WriteExact((char *)&ke, sz_rfbKeyEventMsg, rfbKeyEvent);

CATCH_THROW("ClientConnection::SendKeyEvent")
}

#ifndef UNDER_CE

void ClientConnection::SendClientCutText(char *str, int len)
{
TRY_CATCH

	if (!m_inited)
		return;
	rfbClientCutTextMsg cct;
	cct.type = rfbClientCutText;
	cct.length = Swap32IfLE(len);
	WriteExact((char *)&cct, sz_rfbClientCutTextMsg, rfbClientCutText);
	WriteExact(str, len);
	
CATCH_THROW("ClientConnection::SendClientCutText")
}
#endif

// Copy any updated areas from the bitmap onto the screen.
inline void ClientConnection::DoBlit() 
{
TRY_CATCH

 	if ( NULL == m_hBitmap || (!m_running && !m_restarting) ) 
	{
		ValidateRect(m_hwnd,NULL);
		return;
	}
				
	// No other threads can use bitmap DC
	boost::recursive_mutex::scoped_lock l(m_bitmapdcMutex);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hwnd, &ps);

	// Select and realize hPalette
	PaletteSelector p(hdc, m_hPalette);
	ObjectSelector b(m_hBitmapDC, m_hBitmap);
	
	if (m_opts.m_delay) 
	{
		// Display the area to be updated for debugging purposes
		COLORREF oldbgcol = SetBkColor(hdc, RGB(0,0,0));
		::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, NULL, 0, NULL);
		SetBkColor(hdc,oldbgcol);
		::Sleep(m_pApp->m_options.m_delay);
	}
	
	if (m_opts.m_scaling)
	{
		int n = m_opts.m_scale_num;
		int d = m_opts.m_scale_den;
		int ny = m_opts.m_scale_num_y;
		float dx = m_opts.m_scale_den / m_opts.m_dx;
		float dy = m_opts.m_scale_den / m_opts.m_dy;
		
		SetBrushOrgEx(hdc, 0,0, NULL);

		if (m_opts.m_scaleUpEnabled || 
			d/m_opts.m_dx > 1.0 || 
			d/m_opts.m_dy > 1.0)
		{
			SetStretchBltMode(hdc, HALFTONE);
			if(UltraFast && m_hmemdc)
			{
				StretchBlt(
					hdc,
					ps.rcPaint.left,
					ps.rcPaint.top,
					ps.rcPaint.right-ps.rcPaint.left,
					ps.rcPaint.bottom-ps.rcPaint.top,
					m_hmemdc,
					int(ps.rcPaint.left * dx),
					int(ps.rcPaint.top * dy),
					int((ps.rcPaint.right-ps.rcPaint.left) * dx),
					int((ps.rcPaint.bottom-ps.rcPaint.top) * dy),
					SRCCOPY);
			}
			else
			{
				StretchBlt(
					hdc,
					ps.rcPaint.left,
					ps.rcPaint.top,
					ps.rcPaint.right-ps.rcPaint.left,
					ps.rcPaint.bottom-ps.rcPaint.top,
					m_hBitmapDC, 
					int(ps.rcPaint.left * dx),
					int(ps.rcPaint.top * dy),
					int((ps.rcPaint.right - ps.rcPaint.left) * dx),
					int((ps.rcPaint.bottom - ps.rcPaint.top) * dy),
					SRCCOPY);
			}
		} else
		{
			RECT rc;
			GetClientRect(m_hwnd, &rc);
			//int dx = ((ps.rcPaint.right-ps.rcPaint.left) - m_si.framebufferWidth)/2;
			//int dy = ((ps.rcPaint.bottom-ps.rcPaint.top) - m_si.framebufferHeight)/2;
			int dx = ((rc.right-rc.left) - m_si.framebufferWidth)/2;
			int dy = ((rc.bottom-rc.top) - m_si.framebufferHeight)/2;
			if(UltraFast && m_hmemdc)
			{
				BitBlt(
					hdc, 
					ps.rcPaint.left + dx, 
					ps.rcPaint.top + dy, 
					ps.rcPaint.right-ps.rcPaint.left, 
					ps.rcPaint.bottom-ps.rcPaint.top, 
					m_hmemdc, 
					ps.rcPaint.left, 
					ps.rcPaint.top,
					SRCCOPY);
			}
			else
			{
				BitBlt(
					hdc, 
					ps.rcPaint.left + dx, 
					ps.rcPaint.top + dy, 
					ps.rcPaint.right-ps.rcPaint.left, 
					ps.rcPaint.bottom-ps.rcPaint.top, 
					m_hBitmapDC, 
					ps.rcPaint.left, 
					ps.rcPaint.top, 
					SRCCOPY);
			}
			/// Filling non covered by remote area
			HBRUSH hBrush = FULLSCREEN_MODE == m_opts.m_displayMode?CreateSolidBrush(RGB(0,0,0)):(HBRUSH) (COLOR_MENU+1);
			//RECT rc;
			rc.left = 0;
			rc.top = 0;
			rc.right = dx,
			rc.bottom = ps.rcPaint.bottom;
			FillRect(hdc, &rc, hBrush);

			rc.right = ps.rcPaint.right;
			rc.bottom = dy;
			FillRect(hdc, &rc, hBrush);

			GetClientRect(m_hwnd, &rc);
			rc.left = rc.right - dx;
			FillRect(hdc, &rc, hBrush);

			GetClientRect(m_hwnd, &rc);
			rc.top = rc.bottom - dy;
			FillRect(hdc, &rc, hBrush);
			if (FULLSCREEN_MODE == m_opts.m_displayMode)
				DeleteObject(hBrush);
		}
	}
	else
	{
		if (UltraFast && m_hmemdc)
		{
			ObjectSelector bb(m_hmemdc, m_membitmap);
			BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, 
			ps.rcPaint.right-ps.rcPaint.left, ps.rcPaint.bottom-ps.rcPaint.top, 
			m_hmemdc, ps.rcPaint.left+m_hScrollPos, ps.rcPaint.top+m_vScrollPos, SRCCOPY);
		}
		else
		{
			if (!BitBlt(
					hdc,
					ps.rcPaint.left,
					ps.rcPaint.top , 
					ps.rcPaint.right-ps.rcPaint.left,
					ps.rcPaint.bottom-ps.rcPaint.top , 
					m_hBitmapDC,
					ps.rcPaint.left+m_hScrollPos,
					//ps.rcPaint.top +m_vScrollPos- (m_TBr.bottom - m_TBr.top)  , SRCCOPY)) 
					ps.rcPaint.top +m_vScrollPos,
					SRCCOPY)) 
			{
				Log.WinError(_ERROR_,"Blit error ");
			}
		}
		//BITMAP bmp;
		//GetObject(m_hBitmap, sizeof(bmp), &bmp);
		int cx = m_si.framebufferWidth;
		int cy = m_si.framebufferHeight;
		RECT rc(ps.rcPaint);
		rc.left = cx;
		rc.top = cy;
		FillRect(hdc, &rc, (HBRUSH) (COLOR_WINDOW+1));
		rc.left = 0;
		rc.top = cy;
		FillRect(hdc, &rc, (HBRUSH) (COLOR_WINDOW+1));
		rc.left = cx;
		rc.top = 0;
		FillRect(hdc, &rc, (HBRUSH) (COLOR_WINDOW+1));
	}
	EndPaint(m_hwnd, &ps);

	/// Measuring framerate
	if(frameRateTestCallback)
		(*frameRateTestCallback)(m_hBitmapDC);

CATCH_THROW("ClientConnection::DoBlit")
}

inline void ClientConnection::UpdateScrollbars() 
{
TRY_CATCH

	// We don't update the actual scrollbar info in full-screen mode
	// because it causes them to flicker.
	bool setInfo = !InFullScreenMode();

	SCROLLINFO scri;
	scri.cbSize = sizeof(scri);
	scri.fMask = SIF_ALL;
	scri.nMin = 0;
	scri.nMax = m_hScrollMax; 
	//scri.nMax = m_si.framebufferWidth; 
	scri.nPage= m_cliwidth;
	scri.nPos = m_hScrollPos; 

	RECT rect;
	GetClientRect(m_hwnd, &rect);
	if (setInfo)
	{
		SetScrollInfo(NULL==m_horScrollBar?m_hwndMain:m_horScrollBar, NULL==m_horScrollBar?SB_HORZ:SB_CTL, &scri, TRUE);
		if (NULL != m_horScrollBar)
			ShowScrollBar(m_horScrollBar, SB_CTL, m_opts.m_displayMode == SCROLL_MODE && (rect.right - rect.left) < m_si.framebufferWidth);
	}
	
	scri.cbSize = sizeof(scri);
	scri.fMask = SIF_ALL;
	scri.nMin = 0;

	scri.nMax = m_vScrollMax ;
	//scri.nMax = m_si.framebufferHeight;
	scri.nPage= m_cliheight;
	scri.nPos = m_vScrollPos; 
	
	if (setInfo) 
	{
		SetScrollInfo(NULL==m_vertScrollBar?m_hwndMain:m_vertScrollBar, NULL==m_vertScrollBar?SB_VERT:SB_CTL, &scri, TRUE);
		if (NULL != m_vertScrollBar)
			ShowScrollBar(m_vertScrollBar, SB_CTL, m_opts.m_displayMode == SCROLL_MODE && (rect.bottom - rect.top) < m_si.framebufferHeight);
	}

CATCH_THROW("ClientConnection::UpdateScrollbars")
}


void ClientConnection::ShowConnInfo()
{
TRY_CATCH

	TCHAR buf[2048];
#ifndef UNDER_CE
	char kbdname[9];
	GetKeyboardLayoutName(kbdname);
#else
	TCHAR *kbdname = _T("(n/a)");
#endif
	TCHAR num[16];
	_stprintf(
		buf,
		_T("Connected to: %s\n\r\n\r")
		_T("Host: %s  Port: %d\n\r")
		_T("%s %s  %s\n\r\n\r")
		_T("Desktop geometry: %d x %d x %d\n\r")
		_T("Using depth: %d\n\r")
		_T("Line speed estimate: %d kbit/s\n")
		_T("Current protocol version: %d.%d\n\r\n\r")
		_T("Current keyboard name: %s\n\r\n\r")
		_T("Using Plugin : %s - %s\n\r\n\r"), // sf@2002 - v1.1.2
		m_desktopName, m_host, m_port,
		strcmp(m_proxyhost,"") ? m_proxyhost : "", 
		strcmp(m_proxyhost,"") ? "Port" : "", 
		strcmp(m_proxyhost,"") ? itoa(m_proxyport, num, 10) : "", 
		m_si.framebufferWidth, m_si.framebufferHeight,
                m_si.format.depth,
		m_myFormat.depth, kbitsPerSecond,
		m_majorVersion, m_minorVersion,
		kbdname,"","");
	MessageBox(NULL, buf, _T("VNC connection info"), MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND | MB_TOPMOST);

CATCH_THROW("ClientConnection::ShowConnInfo")
}

// ********************************************************************
//  Methods after this point are generally called by the worker thread.
//  They finish the initialisation, then chiefly read data from the server.
// ********************************************************************
void ClientConnection::run_undetached()
{
	SET_THREAD_LS;

TRY_CATCH

	DoMainLoop();

	if (m_ptrViewer)
		m_ptrViewer->m_klientThreadKilled = true;
	
CATCH_LOG("ClientConnection::run_undetached")
}

void ClientConnection::DoMainLoop()
{
	int sessionRestarts = 0;
TRY_CATCH
	for(sessionRestarts = 0; sessionRestarts < MAX_SESSION_RESTARTS && !m_bKillThread; ++sessionRestarts)
	{
		try
		{
			//if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
			//Log.WinError(_WARNING_,_T("Failed to increase viewer thread priority"));
			m_inited = false;

			//Waiting for session start
			m_ptrViewer->WaitForBeginSession();
			
			if (sessionRestarts != 0)
			{
				/// Session restart
				Log.Add(_MESSAGE_,_T("Start command from host received. Restarting session"));

				m_ptrViewer->NotifySessionRestored();

				SendMessage(GetMainWindow(), m_msgSendInitialStuff, 0 /*do not send view only etc*/, 0);
			}
			else
			{
				/// First session start
				CCritSection cs(m_ptrViewer->GetCS1());
				SendClientInit(); //TODO: remove this. No more needed. (Remove will brake backward compatibility
				LONG st=0;
				st = GetWindowLong(m_hwndMain, GWL_STYLE) & WS_HSCROLL;
				ReadServerInit();

				CreateLocalFramebuffer();
				SetupPixelFormat();
				Createdib();
				SetFormatAndEncodings();
				SizeWindow();

				//SetFocus( GetParent(m_hwnd) );
				BOOL stop_reason = 0;
				m_threadStarted = true;

				/// Sending initial stuff
				SendMessage(GetMainWindow(), m_msgSendInitialStuff, 1 /*send view only etc*/, 0);

				/// Notifying session started
				if ( m_ptrViewer )
				{
					m_ptrViewer->NotifySessionStarted();
				}
				m_stream->SetCS1(m_ptrViewer->GetCS1());

				if (m_opts.m_displayMode != FULLSCREEN_MODE)
					TitleBar.DisplayWindow(FALSE, TRUE);

				/// To display cursor
				SoftCursorMove(0, 0);
			}

			if (!InFullScreenMode()) 
				SizeWindow();

			m_running = true;
			m_restarting = false;
			UpdateWindow(m_hwnd);

			PostMessage(m_hwndMain, WM_SIZE, 0, 0);

			// sf@2002 - Attempt to speed up the thing
			//if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
			//	Log.WinError(_WARNING_,_T("Failed to increase viewer thread priority"));
			rdr::U8 msgType;
			while (!m_bKillThread) 
			{
				// sf@2002 - DSM Plugin
				msgType = fis->readU8();
				m_nTO = 1; // Read the rest of the rfb message (normal case)
				m_recvRecently = true;
				{
					CCritSection cs(m_ptrViewer->GetCS1());
					switch (msgType)
					{
					case rfbAliveMsg:
						{
							Log.Add(_MESSAGE_,_T("Alive message received"));
							break;
						}
					case rfbSetPixelFormatClient:
						{
							int nTO = 1; // Type offset
							rfbClientToServerMsg msg;
							ReadExact(((char *) &msg)+nTO, sz_rfbSetPixelFormatMsg-nTO);
							msg.type = rfbSetPixelFormatClient;
							// Swap the relevant bits.
							m_ptrViewer->SetPixelFormat(&msg);
							msg.spf.format.redMax = Swap16IfLE(msg.spf.format.redMax);
							msg.spf.format.greenMax = Swap16IfLE(msg.spf.format.greenMax);
							msg.spf.format.blueMax = Swap16IfLE(msg.spf.format.blueMax);
							m_myFormat = msg.spf.format;
							break;
						}
					case rfbResetStreams:
						if (zis)
						{
							delete zis;
							zis = new rdr::ZlibInStream();
						}
						break;
					case rfbFramebufferUpdate:
						ReadScreenUpdate();
						RedrawWindow( m_hwnd , 0 , 0 , TRUE );
						break;

					case rfbSetColourMapEntries:
						throw MCException("rfbSetColourMapEntries read but not supported");
						break;

					case rfbBell:
						ReadBell();
						break;

					case rfbServerCutText:
						ReadServerCutText();
						break;

						// Modif sf@2002 - Server Scaling
						// Server Scaled screen buffer size has changed, so we resize
						// the viewer window
					case rfbResizeFrameBuffer:
						{
							//cMsgBoxLog().Add(_MESSAGE_,_T("rfbResizeFrameBuffer"));
							rfbResizeFrameBufferMsg rsmsg;
							ReadExact(((char*)&rsmsg) + m_nTO, sz_rfbResizeFrameBufferMsg - m_nTO);

							m_si.framebufferWidth = Swap16IfLE(rsmsg.framebufferWidth);
							m_si.framebufferHeight = Swap16IfLE(rsmsg.framebufferHeigth);

							ClearCache();
							CreateLocalFramebuffer();
							// SendFullFramebufferUpdateRequest();
							Createdib();
							m_pendingScaleChange = true;
							m_pendingFormatChange = true;
							//SendAppropriateFramebufferUpdateRequest();
							SendMessage(GetMainWindow(),WM_REGIONUPDATED,0,0);

							SizeWindow();
							InvalidateRect(m_hwnd, NULL, TRUE);
							break;
						}
					case rfbStopSession:
						{
							if (m_bKillThread)
								break;
							/// Sending stop to RCHost
							char buf = rfbStopSession;
							WriteExact( &buf , sizeof( buf ) );
							SetWindowLong( m_hwndMain , GWL_WNDPROC , (LONG)m_oldWndProc );
							m_bKillThread = true;
							SetEvent(KillEvent);
							/// Notifying session stopped
							m_ptrViewer->NotifySessionStopped(REMOTE_STOP);
							PostMessage( m_hwnd , WM_CLOSE , 0 , 0 );
							return; //This will brake all loops
						}
					case rfbSetVisualPointer:
						{
							char state;
							ReadExact(&state,1);
							if (m_ptrViewer)
								m_ptrViewer->UpdateSessionMode(VISUAL_POINTER, state!=0);
							break;
						}
					case rfbSetViewOnly:
						{
							char state;
							ReadExact(&state,1);
							if (m_ptrViewer)
								m_ptrViewer->UpdateSessionMode(VIEW_ONLY, state!=0);
							break;
						}
					case rfbRetrySession:
						throw MCException("Message 'Try session restart' received");
						break;
					default:
						Log.Add(_ERROR_, _T("Unknown message type x%02x\n"), msgType );
						throw MCException("Unknown message type received");
						break;
					}
					// yield();
					//Sleep(1);
				}
			} //while
		}
		catch (WarningException& e)
		{
			tstring what = Format(_T("ClientConenction::DoMainLoop WarningException( %s )"),e.m_info);
			MLog_Exception(MCStreamException(what.c_str()));
			TryRestartSession();
		}
		catch (QuietException& e)
		{
			tstring what = Format(_T("ClientConenction::DoMainLoop QuietException( %s )"),e.m_info);
			MLog_Exception(MCStreamException(what.c_str()));
			TryRestartSession();
		}
		catch (rdr::Exception& e)
		{
			tstring what = Format(_T("ClientConenction::DoMainLoop rdr::Exception( %s )"),e.str());
			MLog_Exception(MCStreamException(what.c_str()));
			TryRestartSession();
		}
		catch( CStreamException& e )
		{
			/// Stopping session only on stream exception
			MLog_Exception(CStreamException(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__))));
			break; //Breacking loop
		} 
		catch( CExceptionBase& e)
		{
			MLog_Exception(CExceptionBase(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__))));
			TryRestartSession();
		}
		/// try
	} /// for
CATCH_LOG()
TRY_CATCH
	if (sessionRestarts >= MAX_SESSION_RESTARTS)
		Log.Add(_ERROR_,_T("Maximum session retry count (%d) exceeded, closing session"),sessionRestarts);
	TurnOffFullScreen();
	if (!m_bKillThread)
		m_ptrViewer->NotifySessionStopped(PROTOCOL_ERROR);
	SetWindowLong( m_hwndMain , GWL_WNDPROC , (LONG)m_oldWndProc );
	SetEvent(KillEvent);
	PostMessage( m_hwnd , WM_CLOSE, 0 , 0 );
	m_bKillThread = true;
CATCH_LOG()
	return;
}

void ClientConnection::TryRestartSession()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("Try to restart broken session"));

	m_ptrViewer->NotifySessionBroke();

	m_running = false;
	m_restarting = true;

	if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL))
		Log.WinError(_WARNING_,_T("Failed to decrease viewer thread priority"));

	/// Creating error bitmap
	ObjectSelector b(m_hBitmapDC, m_hBitmap);
	tstring text = _T("Protocol broke, try to reconnect...");
	int cx = m_si.framebufferWidth;
	int cy = m_si.framebufferHeight;
	SIZE sz;
	RECT rc;
	rc.top = 0;
	rc.left = 0;
	rc.right = cx;
	rc.bottom = cy;
	boost::shared_ptr<boost::remove_pointer<HBRUSH>::type > hBrush(CreateSolidBrush(RGB(0,0,0)), DeleteObject);
	hBrush.reset();
	if (hBrush.get() != NULL)
	{
		FillRect(m_hBitmapDC, &rc, hBrush.get());
	} else
	{
		FillRect(m_hBitmapDC, &rc, (HBRUSH) (COLOR_BTNTEXT+1));
	}
	if (GetTextExtentPoint32( m_hBitmapDC, text.c_str(), text.length(), &sz ) != FALSE)
	{
		rc.left = (cx - sz.cx)/2;
		rc.top = (cy - sz.cy)/2;
	} else
		Log.WinError(_WARNING_,"Failed to GetTextExtentPoint32 ");
	if (SetTextColor(m_hBitmapDC, RGB(255,255,255)) == CLR_INVALID)
		Log.WinError(_WARNING_,_T("Failed to SetTextColor "));
	if (SetBkColor(m_hBitmapDC, RGB(0,0,0)) == CLR_INVALID)
		Log.WinError(_WARNING_,_T("Failed to SetBkColor"));
	TextOut(m_hBitmapDC, rc.left, rc.top, text.c_str(), text.length());
	InvalidateRect(GetMainWindow(), NULL, TRUE);

	/// Reseting encoder streams
	if (zis)
	{
		delete zis;
		zis = new rdr::ZlibInStream();
	}
	if (fis)
	{
		delete fis;
		fis = new rdr::FdInStream( m_stream.get() );
		fis->SetDSMMode(false);
	}

	if (m_restarting)
	{
		/// Sending initial stuff
		DWORD result;
		if (FALSE == SendMessageTimeout(GetMainWindow(), m_msgSendRetryStuff, 0, 0, SMTO_NORMAL, WAIT_RESTART_TIMEOUT, &result))
		{
			m_restarting = false;
			Log.WinError(_ERROR_,_T("Send restart message wait failed. Terminating DS session "));
		}
	}

	/// Now we'll return to loop withing DoMainLoop

CATCH_THROW()
}


//
// Requesting screen updates from the server
//

inline void ClientConnection::SendFramebufferUpdateRequest(int x, int y, int w, int h, bool incremental)
{
TRY_CATCH

	rfbFramebufferUpdateRequestMsg fur;
	fur.type = rfbFramebufferUpdateRequest;
	fur.incremental = incremental ? 1 : 0;
	fur.x = Swap16IfLE(x);
	fur.y = Swap16IfLE(y);
	fur.w = Swap16IfLE(w);
	fur.h = Swap16IfLE(h);

	WriteExact((char *)&fur, sz_rfbFramebufferUpdateRequestMsg, rfbFramebufferUpdateRequest);

CATCH_THROW("ClientConnection::SendFramebufferUpdateRequest")
}

inline void ClientConnection::SendIncrementalFramebufferUpdateRequest()
{
TRY_CATCH
    SendFramebufferUpdateRequest(0, 0, m_si.framebufferWidth, m_si.framebufferHeight, true);
CATCH_THROW("ClientConnection::SendIncrementalFramebufferUpdateRequest")
}

void ClientConnection::SendFullFramebufferUpdateRequest()
{
TRY_CATCH
    SendFramebufferUpdateRequest(0, 0, m_si.framebufferWidth, m_si.framebufferHeight, false);
CATCH_THROW("ClientConnection::SendFullFramebufferUpdateRequest")
}


void ClientConnection::SendAppropriateFramebufferUpdateRequest()
{
TRY_CATCH

	if (m_pendingFormatChange) 
	{
		/// Requesting new pixel format

		// Cache init/reinit - A SetFormatAndEncoding() implies a cache reinit on server side
		// Cache enabled, so it's going to be reallocated/reinited on server side
		if (m_opts.m_fEnableCache)
		{
			// create viewer cache buffer if necessary
			if (m_hCacheBitmap == NULL)
			{
				m_hCacheBitmapDC = CreateCompatibleDC(m_hBitmapDC);
				m_hCacheBitmap = CreateCompatibleBitmap(m_hBitmapDC, m_si.framebufferWidth, m_si.framebufferHeight);
			}
			ClearCache(); // Clear the cache
			m_pendingCacheInit = true; // Order full update to synchronize both sides caches
		}
		else // No cache requested - The cache on the other side is to be cleared/deleted
			 // Todo: fix the cache switching pb when viewer has been started without cache
		{
			/* causes balck rects after cache off/on
			// Delete local cache
			DeleteDC(m_hCacheBitmapDC);
			if (m_hCacheBitmap != NULL) DeleteObject(m_hCacheBitmap);
			if (m_hCacheBitmapDC != NULL) DeleteObject(m_hCacheBitmapDC);			
			m_hCacheBitmap = NULL;
			m_pendingCacheInit = false;
			*/
		}
		
		//rfbPixelFormat oldFormat = m_myFormat;
		//SetupPixelFormat();
		if (m_pendingColorsRequest)
		{
			m_pendingColorsRequest = false;
			//if (PF_EQ(m_myFormat, oldFormat))
			{
				rfbPixelFormat newPixFormat = SelectPixelFormat(m_newColors);
				rfbSetPixelFormatMsg spf;
			    spf.type = rfbSetPixelFormat;
				spf.format = newPixFormat;
				spf.format.redMax = Swap16IfLE(spf.format.redMax);
				spf.format.greenMax = Swap16IfLE(spf.format.greenMax);
				spf.format.blueMax = Swap16IfLE(spf.format.blueMax);
				WriteExact((char *)&spf, sz_rfbSetPixelFormatMsg, rfbSetPixelFormat);
				// tight cursor handling
				SoftCursorFree();
				Createdib();
				SetFormatAndEncodings(false /*only encodings*/);
				m_pendingFormatChange = false;
			}
		}
		else
		{
			// tight cursor handling
			SoftCursorFree();
			Createdib();
			SetFormatAndEncodings();
			m_pendingFormatChange = false;
		}

		// If the pixel format has changed, or cache, or scale request whole screen
		//if (!PF_EQ(m_myFormat, oldFormat) || m_pendingCacheInit || m_pendingScaleChange)
		if (m_pendingCacheInit || m_pendingScaleChange)
		{
			SendFullFramebufferUpdateRequest();	
		}
		else
		{
			SendIncrementalFramebufferUpdateRequest();
		}
		m_pendingScaleChange = false;
		m_pendingCacheInit = false;
	}
	else 
	{
		if (!m_dormant)
			SendIncrementalFramebufferUpdateRequest();
	}

CATCH_THROW("ClientConnection::SendAppropriateFramebufferUpdateRequest")
}

bool ClientConnection::SendServerScale(int nScale)
{
TRY_CATCH

    rfbSetScaleMsg ssc;
    int len = 0;

    ssc.type = rfbSetScale;
    ssc.scale = /*(unsigned short)*/ nScale;

    WriteExact((char*)&ssc, sz_rfbSetScaleMsg, rfbSetScale);

    return true;

CATCH_THROW("ClientConnection::SendServerScale")
}

bool ClientConnection::SendServerInput(BOOL enabled)
{
TRY_CATCH

    rfbSetServerInputMsg sim;
    int len = 0;
    sim.type = rfbSetServerInput;
    sim.status = enabled;
    WriteExact((char*)&sim, sz_rfbSetServerInputMsg, rfbSetServerInput);
    return true;

CATCH_THROW("ClientConnection::SendServerInput")
}

bool ClientConnection::SendSW(int x, int y)
{
TRY_CATCH

    rfbSetSWMsg sw;
    int len = 0;
	if (x==9999 && y==9999)
	{
		sw.type = rfbSetSW;
		sw.x = Swap16IfLE(1);
		sw.y = Swap16IfLE(1);
	}
	else
	{
		int x_scaled =
			(x + m_hScrollPos) * m_opts.m_scale_den / m_opts.m_scale_num;
		int y_scaled =
			(y + m_vScrollPos) * m_opts.m_scale_den / m_opts.m_scale_num;
		
		sw.type = rfbSetSW;
		sw.x = Swap16IfLE(x_scaled);
		sw.y = Swap16IfLE(y_scaled);
	}
    WriteExact((char*)&sw, sz_rfbSetSWMsg, rfbSetSW);
	m_SWselect=false;
    return true;

CATCH_THROW("ClientConnection::SendSW")
}

inline void ClientConnection::OnSpeedMeasured(const unsigned int _kbitsPerSecond)
{
TRY_CATCH

	unsigned int kbitsPerSecond = m_integrator.Next(_kbitsPerSecond);

	// We only change the preferred encoding if FileTransfer is not running and if
	// the last encoding change occured more than 30s ago
	int timeDelta = timeGetTime() - m_lLastChangeTime;
	if (m_opts.autoDetect 
		&&
		timeDelta > DEC_COLORS_DELAY)
	{
		int nOldServerScale = m_nServerScale;
		// If connection speed > 1Mbits/s - All to the max
		/* 
		if (kbitsPerSecond > 2000 && (m_nConfig != 7))
		{
		m_nConfig = 1;
		m_opts.m_PreferredEncoding = rfbEncodingUltra;
		m_opts.m_Use8Bit = false; // Max colors
		m_opts.m_fEnableCache = false;
		m_pendingFormatChange = true;
		m_lLastChangeTime = timeGetTime();
		}*/
		if (kbitsPerSecond > 1000 && (m_nConfig != 1))
		{
			if (m_nConfig > 1 && timeDelta < INC_COLORS_DELAY)
				return;
			m_nConfig = 1;
			m_opts.m_PreferredEncoding = rfbEncodingZlibHex; //rfbEncodingZRLE; /*rfbEncodingHextile*/  //TODO: check if ZRLE really better
			//m_opts.m_Use8Bit = rfbPFFullColors; // Max colors
			m_newColors = rfbPFFullColors;
			//m_opts.m_fEnableCache = false;
			m_pendingFormatChange = true;
			m_pendingColorsRequest = true;
			m_lLastChangeTime = timeGetTime();
			Log.Add(_MESSAGE_,_T("Switching to FullColors / ZlibHex %d kbut/s"),kbitsPerSecond);
		}
		// Medium connection speed 
		else if (kbitsPerSecond < 512 && kbitsPerSecond > 256 && (m_nConfig != 2))
		{
			if (m_nConfig > 2 && timeDelta < INC_COLORS_DELAY)
				return;
			m_nConfig = 2;
			m_opts.m_PreferredEncoding = rfbEncodingZlibHex; //rfbEncodingTight;//rfbEncodingZRLE; 
			//m_opts.m_Use8Bit = rfbPF256Colors; 
			m_newColors = rfbPF256Colors;
			//m_opts.m_compressLevel = 5;
			//m_opts.m_fEnableCache = false;
			m_pendingFormatChange = true;
			m_pendingColorsRequest = true;
			m_lLastChangeTime = timeGetTime();
			Log.Add(_MESSAGE_,_T("Switching to 256Colors / ZlibHex compression; speed %d kbit/s"),kbitsPerSecond);
		}
		// Modem (including cable modem) connection speed 
		else if (kbitsPerSecond < 256 && kbitsPerSecond > 80 && (m_nConfig != 3))
		{
			if (m_nConfig > 3 && timeDelta < INC_COLORS_DELAY)
				return;
			m_nConfig = 3;
			m_opts.m_PreferredEncoding = rfbEncodingTight;
			//m_opts.m_Use8Bit = rfbPF64Colors; 
			m_newColors = rfbPF64Colors;
			//m_opts.m_compressLevel = 9;
			//m_opts.m_fEnableCache = false;
			m_pendingFormatChange = true;
			m_pendingColorsRequest = true;
			m_lLastChangeTime = timeGetTime();
			Log.Add(_MESSAGE_,_T("Switching to 64Colors / Jpeg (Tight) compression; speed %d kbit/s"),kbitsPerSecond);
		}
		// Slow Modem connection speed 
		// Not sure it's a good thing in Auto mode...because in some cases
		// (CTRL-ALT-DEL, initial screen loading, connection short hangups...)
		// the speed can be momentary VERY slow. The fast fuzzy/normal modes switching
		// can be quite disturbing and useless in these situations.
		//else if (kbitsPerSecond < 128 && kbitsPerSecond > 5 && (m_nConfig != 4))
		else if (kbitsPerSecond < 80 && kbitsPerSecond > 0 && (m_nConfig != 4))
		{
			m_nConfig = 4;
			m_opts.m_PreferredEncoding = rfbEncodingTight;
			//m_opts.m_compressLevel = 9;
			//m_opts.m_Use8Bit = rfbPF8GreyColors;//rfbPF8Colors; 
			m_newColors = rfbPF8GreyColors;
			// m_opts.m_scaling = true;
			// m_opts.m_scale_num = 2;
			// m_opts.m_scale_den = 1;
			// m_nServerScale = 2;
			// m_opts.m_nServerScale = 2;
			//m_opts.m_fEnableCache = false;
			m_pendingFormatChange = true;
			m_pendingColorsRequest = true;
			m_lLastChangeTime = timeGetTime();
			Log.Add(_MESSAGE_,_T("Switching to 8GreyColors / Jpeg (Tight) compression; speed %d kbit/s"),kbitsPerSecond);
		}
	}
CATCH_THROW()
}


// A ScreenUpdate message has been received
inline void ClientConnection::ReadScreenUpdate()
{
try
{
	HDC hdcX,hdcBits;
	bool fTimingAlreadyStopped = false;
	fis->startTiming();
	rfbFramebufferUpdateMsg sut;
	ReadExact(((char *) &sut)+m_nTO, sz_rfbFramebufferUpdateMsg-m_nTO);
	sut.nRects = Swap16IfLE(sut.nRects);
	HRGN UpdateRegion=CreateRectRgn(0,0,0,0);
	bool Recover_from_sync=false;
		
	for (UINT u=0; u < sut.nRects; ++u)
	{
		rfbFramebufferUpdateRectHeader surh;
		ReadExact((char *) &surh, sz_rfbFramebufferUpdateRectHeader);
		surh.r.x = Swap16IfLE(surh.r.x);
		surh.r.y = Swap16IfLE(surh.r.y);
		surh.r.w = Swap16IfLE(surh.r.w);
		surh.r.h = Swap16IfLE(surh.r.h);
		surh.encoding = Swap32IfLE(surh.encoding);
		
		// Tight - If lastrect we must quit this loop (nRects = 0xFFFF)
		if (surh.encoding == rfbEncodingLastRect)
			break;
		
		if (surh.encoding == rfbEncodingNewFBSize)
		{
			ReadNewFBSize(&surh);
			break;
		}
		
		// Tight cursor handling
		if ( surh.encoding == rfbEncodingXCursor ||
			surh.encoding == rfbEncodingRichCursor )
		{
			ReadCursorShape(&surh);
			continue;
		}
		
		if (surh.encoding == rfbEncodingPointerPos) 
		{
			if (m_ptrViewer && m_ptrViewer->m_viewOnly)
				ReadCursorPos(&surh);
			continue;
		}

		if (surh.encoding !=rfbEncodingNewFBSize && surh.encoding != rfbEncodingCacheZip && surh.encoding != rfbEncodingSolMonoZip && surh.encoding !=rfbEncodingUltraZip)
			SoftCursorLockArea(surh.r.x, surh.r.y, surh.r.w, surh.r.h);
		
		
		RECT cacherect;
		if (m_opts.m_fEnableCache)
		{
			cacherect.left=surh.r.x;
			cacherect.right=surh.r.x+surh.r.w;
			cacherect.top=surh.r.y;
			cacherect.bottom=surh.r.y+surh.r.h;
		}

		if (m_TrafficMonitor)
		{
			hdcX = GetDC(m_TrafficMonitor);
			hdcBits = CreateCompatibleDC(hdcX);
			SelectObject(hdcBits,m_bitmapBACK);
			BitBlt(hdcX,4,2,22,20,hdcBits,0,0,SRCCOPY);
			DeleteDC(hdcBits);
			ReleaseDC(m_TrafficMonitor,hdcX);
		}

		//vnclog.Print(0, _T("encoding %d\n"), surh.encoding);
		if (surh.encoding==rfbEncodingUltra || surh.encoding==rfbEncodingUltraZip)
		{
			UltraFast=true;
		}
		else
		{
			UltraFast=false;
		}

		switch (surh.encoding)
		{
		case rfbEncodingHextile:
			SaveArea(cacherect);
			ReadHextileRect(&surh);
			EncodingStatusWindow=rfbEncodingHextile;
			break;
		case rfbEncodingUltra:
			ReadUltraRect(&surh);
			EncodingStatusWindow=rfbEncodingUltra;
			break;
		case rfbEncodingUltraZip:
			ReadUltraZip(&surh,&UpdateRegion);
			break;
		case rfbEncodingRaw:
			SaveArea(cacherect);
			ReadRawRect(&surh);
			EncodingStatusWindow=rfbEncodingRaw;
			break;
		case rfbEncodingCopyRect:
			ReadCopyRect(&surh);
			break;
		case rfbEncodingCache:
			ReadCacheRect(&surh);
			break;
		case rfbEncodingCacheZip:
			ReadCacheZip(&surh,&UpdateRegion);
			break;
		case rfbEncodingSolMonoZip:
			ReadSolMonoZip(&surh,&UpdateRegion);
			break;
		case rfbEncodingRRE:
			SaveArea(cacherect);
			ReadRRERect(&surh);
			EncodingStatusWindow=rfbEncodingRRE;
			break;
		case rfbEncodingCoRRE:
			SaveArea(cacherect);
			ReadCoRRERect(&surh);
			EncodingStatusWindow=rfbEncodingCoRRE;
			break;
		case rfbEncodingZlib:
			SaveArea(cacherect);
			ReadZlibRect(&surh,0);
			EncodingStatusWindow=rfbEncodingZlib;
			break;
		case rfbEncodingZlibHex:
			SaveArea(cacherect);
			ReadZlibHexRect(&surh);
			EncodingStatusWindow=rfbEncodingZlibHex;
			break;
		case rfbEncodingXOR_Zlib:
			SaveArea(cacherect);
			ReadZlibRect(&surh,1);
			break;
		case rfbEncodingXORMultiColor_Zlib:
			SaveArea(cacherect);
			ReadZlibRect(&surh,2);
			break;
		case rfbEncodingXORMonoColor_Zlib:
			SaveArea(cacherect);
			ReadZlibRect(&surh,3);
			break;
		case rfbEncodingSolidColor:
			SaveArea(cacherect);
			ReadSolidRect(&surh);
			break;
		case rfbEncodingZRLE:
			SaveArea(cacherect);
			zrleDecode(surh.r.x, surh.r.y, surh.r.w, surh.r.h);
			EncodingStatusWindow=rfbEncodingZRLE;
			break;
		case rfbEncodingTight:
			SaveArea(cacherect);
			ReadTightRect(&surh);
			EncodingStatusWindow=rfbEncodingTight;
			break;
		default:
			// Unknown encoding surh.encoding - not supported
			// Try to empty buffer...
			// so next update should be back in sync
			throw MCException(Format(_T("Unknown encoding surh.encoding (%d) - not supported."),surh.encoding));
			///TODO: remove all relative to Recover_from_sync
			Log.Add(_WARNING_,_T("Unknown encoding surh.encoding (%d) - not supported. Try to empty buffer... so next update should be back in sync"), surh.encoding);
			BYTE * buffer;
			int i=0;
			while (TRUE)
			{
				int aantal=fis->Check_if_buffer_has_data();
				if (aantal>0) buffer = new BYTE [aantal];
				if (aantal>0)
				{
					i=0;
					ReadExact(((char *) buffer), aantal);
					delete [] buffer;
					Sleep(5);
				}
				else if (aantal==0)
				{
					if (i==5) break;
					Sleep(200);
					i++;
				}
				else break;   
			}
			//vnclog.Print(0, _T("Buffer cleared, sync should be back OK..Continue \n"));
			Recover_from_sync=true;
			break;
		}

		if (Recover_from_sync) 
		{
		    Recover_from_sync=false;
			break;
		}
		
		if (surh.encoding !=rfbEncodingNewFBSize && surh.encoding != rfbEncodingCacheZip && surh.encoding != rfbEncodingSolMonoZip && surh.encoding != rfbEncodingUltraZip)
		{
			RECT rect;
			rect.left   = surh.r.x;
			rect.top    = surh.r.y;
			rect.right  = surh.r.x + surh.r.w ;
			rect.bottom = surh.r.y + surh.r.h; 
			InvalidateScreenRect(&rect); 			
		}
		else if (surh.encoding !=rfbEncodingNewFBSize)
		{
			InvalidateRgn(m_hwnd, UpdateRegion, FALSE);
			HRGN tempregion=CreateRectRgn(0,0,0,0);
			CombineRgn(UpdateRegion,UpdateRegion,tempregion,RGN_AND);
			DeleteObject(tempregion);
		}

		if (m_TrafficMonitor)
		{
			hdcX = GetDC(m_TrafficMonitor);
			hdcBits = CreateCompatibleDC(hdcX);
			SelectObject(hdcBits,m_bitmapNONE);
			BitBlt(hdcX,4,2,22,20,hdcBits,0,0,SRCCOPY);
			DeleteDC(hdcBits);
			ReleaseDC(m_TrafficMonitor,hdcX);
		}

		SoftCursorUnlockScreen();
	}

	if (!fTimingAlreadyStopped)
	{
		fis->stopTiming();
		kbitsPerSecond = fis->kbitsPerSecond();
	}
	OnSpeedMeasured(kbitsPerSecond);

	// Inform the other thread that an update is needed.
	PostMessage(m_hwnd, WM_REGIONUPDATED, NULL, NULL);
	DeleteObject(UpdateRegion);
}
catch(rdr::Exception&)
{
	throw;
}
catch(QuietException&)
{
	throw;
}
catch(WarningException&)
{
	throw;
CATCH_THROW("ClientConnection::ReadScreenUpdate")
}	



void ClientConnection::SetDormant(bool newstate)
{
TRY_CATCH

	m_dormant = newstate;
	if (!m_dormant)
		SendIncrementalFramebufferUpdateRequest();

CATCH_THROW("ClientConnection::SetDormant")
}

// The server has copied some text to the clipboard - put it 
// in the local clipboard too.
void ClientConnection::ReadServerCutText() 
{
TRY_CATCH

	rfbServerCutTextMsg sctm;
	//vnclog.Print(6, _T("Read remote clipboard change\n"));
	ReadExact(((char *) &sctm)+m_nTO, sz_rfbServerCutTextMsg-m_nTO);
	unsigned int len = Swap32IfLE(sctm.length);
	std::auto_ptr<char> buf;
	buf.reset(new char[len+1]);
	if (!buf.get())
		throw MCException("Failed to new char[len]");
    if (len == 0) 
	{
		buf.get()[0] = '\0';
	} else 
	{
		ReadString(buf.get(), len);
	}
	UpdateLocalClipboard(buf.get(), len);

CATCH_THROW("ClientConnection::ReadServerCutText")
}

void ClientConnection::ReadBell() 
{
TRY_CATCH
	rfbBellMsg bm;
	ReadExact(((char *) &bm)+m_nTO, sz_rfbBellMsg-m_nTO);

	#ifdef UNDER_CE
	MessageBeep( MB_OK );
	#else

	if (! ::PlaySound("RCViewerBell", NULL, 
		SND_APPLICATION | SND_ALIAS | SND_NODEFAULT | SND_ASYNC) ) {
		::Beep(440, 125);
	}
	#endif
	if (m_opts.m_DeiconifyOnBell) {
		if (IsIconic(m_hwnd)) {
			SetDormant(false);
			ShowWindow(m_hwnd, SW_SHOWNORMAL);
		}
	}
	Log.Add(_MESSAGE_, _T("Bell!\n"));
CATCH_THROW("ClientConnection::ReadBell")
}


// General utilities -------------------------------------------------

// Reads the number of bytes specified into the buffer given
inline void ClientConnection::ReadExact(char *inbuf, int wanted)
{
TRY_CATCH
	try
	{
		//boost::recursive_mutex::scoped_lock l(m_readMutex);
		fis->readBytes(inbuf, wanted);
	}
	catch (rdr::Exception& e)
	{
		Log.Add(_ERROR_,"rdr::Exception (2): %s\n",e.str());
		throw QuietException(e.str());
	}
CATCH_THROW("ClientConnection::ReadExact")
}

// Read the number of bytes and return them zero terminated in the buffer 
void ClientConnection::ReadString(char *buf, int length)
{
TRY_CATCH
	if (length > 0)
		ReadExact(buf, length);
	buf[length] = '\0';
CATCH_THROW("ClientConnection::ReadString");
}

// Makes sure netbuf is at least as big as the specified size.
// Note that netbuf itself may change as a result of this call.
// Throws an exception on failure.
void ClientConnection::CheckBufferSize(int bufsize)
{
TRY_CATCH

	if (m_netbufsize > bufsize) return;
	char *newbuf = new char[bufsize+256];
	if (newbuf == NULL) {
		throw ErrorException(sz_L70);
	}
	// Only if we're successful...
	boost::recursive_mutex::scoped_lock l(m_bufferMutex);
	if (m_netbuf != NULL)
		delete [] m_netbuf;
	m_netbuf = newbuf;
	m_netbufsize=bufsize + 256;

CATCH_THROW("ClientConnection::CheckBufferSize")
}


// Makes sure zipbuf is at least as big as the specified size.
// Note that zlibbuf itself may change as a result of this call.
// Throws an exception on failure.
// sf@2002
void ClientConnection::CheckZipBufferSize(int bufsize)
{
TRY_CATCH

	unsigned char *newbuf;
	if (m_zipbufsize > bufsize) return;
	boost::recursive_mutex::scoped_lock l(m_ZipBufferMutex);

	newbuf = (unsigned char *)new char[bufsize + 256];
	if (newbuf == NULL) {
		throw ErrorException(sz_L71);
	}

	// Only if we're successful...
	if (m_zipbuf != NULL)
		delete [] m_zipbuf;
	m_zipbuf = newbuf;
	m_zipbufsize = bufsize + 256;

CATCH_THROW("ClientConnection::CheckZipBufferSize")
}

void ClientConnection::CheckFileZipBufferSize(int bufsize)
{
TRY_CATCH

	unsigned char *newbuf;
	if (m_filezipbufsize > bufsize) return;
	boost::recursive_mutex::scoped_lock l(m_FileZipBufferMutex);

	newbuf = (unsigned char *)new char[bufsize + 256];
	if (newbuf == NULL) {
		throw ErrorException(sz_L71);
	}

	// Only if we're successful...
	if (m_filezipbuf != NULL)
		delete [] m_filezipbuf;
	m_filezipbuf = newbuf;
	m_filezipbufsize = bufsize + 256;

CATCH_THROW("ClientConnection::CheckFileZipBufferSize")
}

void ClientConnection::CheckFileChunkBufferSize(int bufsize)
{
TRY_CATCH

	unsigned char *newbuf;
	if (m_filechunkbufsize > bufsize) return;
	boost::recursive_mutex::scoped_lock l(m_FileChunkBufferMutex);

	newbuf = (unsigned char *)new char[bufsize + 256];
	if (newbuf == NULL) {
		throw ErrorException(sz_L71);
	}


	if (m_filechunkbuf != NULL)
		delete [] m_filechunkbuf;
	m_filechunkbuf = newbuf;
	m_filechunkbufsize = bufsize + 256;

CATCH_THROW("ClientConnection::CheckFileChunkBufferSize")
}

// Processing NewFBSize pseudo-rectangle. Create new framebuffer of
// the size specified in pfburh->r.w and pfburh->r.h, and change the
// window size correspondingly.
//
void ClientConnection::ReadNewFBSize(rfbFramebufferUpdateRectHeader *pfburh)
{
TRY_CATCH

	m_si.framebufferWidth = pfburh->r.w;
	m_si.framebufferHeight = pfburh->r.h;
	ClearCache();
	CreateLocalFramebuffer();
	SendMessage(GetMainWindow(),WM_FULLSCREENUPDATED,0,0);
	Createdib();
	m_pendingScaleChange = true;
	m_pendingFormatChange = true;
	SendMessage(GetMainWindow(),WM_REGIONUPDATED,0,0);
	SizeWindow();
	InvalidateRect(m_hwnd, NULL, TRUE);
	ChangeDisplayMode(m_opts.m_displayMode);
	m_ptrViewer->SetRemoteDesktopSize(m_si.framebufferWidth, m_si.framebufferHeight);
	
CATCH_THROW("ClientConnection::ReadNewFBSize")
}


// sf@2002 - DSMPlugin 
// Ensures that the temporary "alignement" buffer in large enough 
/*inline void ClientConnection::CheckNetRectBufferSize(int nBufSize)
{
	CCritSection cs(m_ptrViewer->GetCS1());
	if (m_nNetRectBufSize > nBufSize) return;

	boost::recursive_mutex::scoped_lock l(m_NetRectBufferMutex);

	BYTE *newbuf = new BYTE[nBufSize + 256];
	if (newbuf == NULL) 
	{
		// Error
	}
	if (m_pNetRectBuf != NULL)
		delete [] m_pNetRectBuf;

	m_pNetRectBuf = newbuf;
	m_nNetRectBufSize = nBufSize + 256;
}*/


//
// Ensures that the temporary "alignement" buffer in large enough 
//
inline void ClientConnection::CheckZRLENetRectBufferSize(int nBufSize)
{
	if (m_nZRLENetRectBufSize > nBufSize) return;

	boost::recursive_mutex::scoped_lock l(m_ZRLENetRectBufferMutex);

	BYTE *newbuf = new BYTE[nBufSize + 256];
	if (newbuf == NULL) 
	{
		// Error
	}
	if (m_pZRLENetRectBuf != NULL)
		delete [] m_pZRLENetRectBuf;

	m_pZRLENetRectBuf = newbuf;
	m_nZRLENetRectBufSize = nBufSize + 256;
}



//
// Format file size so it is user friendly to read
// 
void ClientConnection::GetFriendlySizeString(__int64 Size, char* szText)
{
	szText[0] = '\0';
	if( Size > (1024*1024*1024) )
	{
		__int64 lRest = (Size % (1024*1024*1024));
		Size /= (1024*1024*1024);
		wsprintf(szText,"%u.%4.4lu Gb", (unsigned long)Size, (unsigned long)((__int64)(lRest) * 10000 / 1024 / 1024 / 1024));
	}
	else if( Size > (1024*1024) )
	{
		unsigned long lRest = (Size % (1024*1024));
		Size /= (1024*1024);
		wsprintf(szText,"%u.%3.3lu Mb", (unsigned long)Size, (unsigned long)((__int64)(lRest) * 1000 / 1024 / 1024));
	}
	else if ( Size > 1024 )
	{
		unsigned long lRest = Size % (1024);
		Size /= 1024;
		wsprintf(szText,"%u.%2.2lu Kb", (unsigned long)Size, lRest * 100 / 1024);
	}
	else
	{
		wsprintf(szText,"%u bytes", (unsigned long)Size);
	}
}


////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void ClientConnection::GTGBS_ScrollToolbar(int dx, int dy)
{

/*	dx = max(dx, -m_hScrollPos);
	dx = min(dx, m_hScrollMax-(m_cliwidth)-m_hScrollPos);
	dy = max(dy, -m_vScrollPos);
	dy = min(dy, m_vScrollMax-(m_cliheight)-m_vScrollPos);
	if (dx || dy) {
		RECT clirect;
		GetClientRect(m_hwndTBwin, &clirect);
		ScrollWindowEx(m_hwndTBwin, dx, dy, NULL, &clirect, NULL, NULL, SW_ERASE );
		DoBlit();
	}
*/
}

void ClientConnection::GTGBS_CreateDisplay()
{
TRY_CATCH
	// Das eigendliche HauptFenster erstellen,
	// welches das VNC-Fenster und die Toolbar enthält
	//if( mainWindow )
	//{
		//m_hwndMain = mainWindow;
		//LONG st = GetWindowLong( m_hwndMain , GWL_STYLE ) & WS_HSCROLL;
		//SetWindowLong( m_hwndMain , GWL_STYLE , WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |WS_MINIMIZEBOX |WS_MAXIMIZEBOX | WS_THICKFRAME | WS_VSCROLL | WS_HSCROLL );
	//return;
	//}

	if( m_hwndMain )
		return;

	WNDCLASS wndclass;

	wndclass.style			= 0;
	wndclass.lpfnWndProc	= ClientConnection::WndProc;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= m_pApp->m_instance;
	wndclass.hIcon			= NULL;
	switch (m_opts.m_localCursor) 
	{
		case NOCURSOR:
			wndclass.hCursor		= LoadCursor(m_pApp->m_instance, MAKEINTRESOURCE(IDC_NOCURSOR));
			break;
		case NORMALCURSOR:
			wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
			break;
		case DOTCURSOR:
		default:
			wndclass.hCursor		= LoadCursor(m_pApp->m_instance, MAKEINTRESOURCE(IDC_DOTCURSOR));
	}
	wndclass.hbrBackground	= (HBRUSH) GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName	= (const TCHAR *) NULL;
	wndclass.lpszClassName	= VNCMDI_CLASS_NAME;

	TRY_CATCH
		RegisterClassForced(wndclass);
	CATCH_LOG()


#ifdef _WIN32_WCE
	const DWORD winstyle = WS_VSCROLL | WS_HSCROLL | WS_CAPTION | WS_SYSMENU;
#else
	const DWORD winstyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |  WS_MINIMIZEBOX |WS_MAXIMIZEBOX | WS_THICKFRAME /*| WS_VSCROLL | WS_HSCROLL*/;
#endif

	m_hwndMain = CreateWindowEx(	WS_EX_TOOLWINDOW,
									VNCMDI_CLASS_NAME,
									_T("RCViewer"),
									winstyle,
									CW_USEDEFAULT,
									CW_USEDEFAULT,
									//CW_USEDEFAULT,
									//CW_USEDEFAULT,
									320,200,
									NULL,                // Parent handle
									NULL,                // Menu handle
									m_pApp->m_instance,
									NULL);

	SetWindowLong(m_hwndMain, GWL_USERDATA, (LONG) this);

CATCH_THROW()
}


//
// Process windows messages (Host window)
//
LRESULT CALLBACK ClientConnection::WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
TRY_CATCH

	// This is a static method, so we don't know which instantiation we're 
	// dealing with.  But we've stored a 'pseudo-this' in the window data
	ClientConnection *_this = (ClientConnection *) GetWindowLong(hwnd, GWL_USERDATA);
	
	if (_this == NULL)
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	{
		{
			switch (iMsg)
			{
			case WM_GETDLGCODE: //Do not know who sends it. I really try to find, but for our window it causes ugly beeps
				//return DLGC_WANTALLKEYS;
				//return 0;
				return DLGC_WANTCHARS;
			case WM_SYSCOMMAND:
				{
					switch (LOWORD(wParam))
					{	
					case ID_VIEWONLYTOGGLE: 
						// Toggle view only mode
						_this->m_opts.m_ViewOnly = !_this->m_opts.m_ViewOnly;
						// Todo update menu state
						return 0;
						
					case ID_REQUEST_REFRESH: 
						// Request a full-screen update
						_this->SendFullFramebufferUpdateRequest();
						return 0;
	
					case ID_VK_LWINDOWN:
						if (_this->m_opts.m_ViewOnly) return 0;
						_this->SendKeyEvent(XK_Super_L, true);
						return 0;
					case ID_VK_LWINUP:
						if (_this->m_opts.m_ViewOnly) return 0;
						_this->SendKeyEvent(XK_Super_L, false);
						return 0;
					case ID_VK_RWINDOWN:
						if (_this->m_opts.m_ViewOnly) return 0;
						_this->SendKeyEvent(XK_Super_R, true);
						return 0;
					case ID_VK_RWINUP:
						if (_this->m_opts.m_ViewOnly) return 0;
						_this->SendKeyEvent(XK_Super_R, false);
						return 0;
					case ID_VK_APPSDOWN:
						if (_this->m_opts.m_ViewOnly) return 0;
						_this->SendKeyEvent(XK_Menu, true);
						return 0;
					case ID_VK_APPSUP:
						if (_this->m_opts.m_ViewOnly) return 0;
						_this->SendKeyEvent(XK_Menu, false);
						return 0;

						
					// Send START Button
					case ID_CONN_CTLESC:
						if (_this->m_opts.m_ViewOnly) return 0;
						_this->SendKeyEvent(XK_Control_L,true);
						_this->SendKeyEvent(XK_Escape,true);
						_this->SendKeyEvent(XK_Control_L,false);
						_this->SendKeyEvent(XK_Escape,false);
						return 0;
						
					// Send Ctrl-Alt-Del
					case ID_CONN_CTLALTDEL:
						{
							char code = rfbSendCAD;
							_this->WriteExact( &code, 1 );
						}
						/*if (_this->m_opts.m_ViewOnly) return 0;
						_this->SendKeyEvent(XK_Control_L, true);
						_this->SendKeyEvent(XK_Alt_L,     true);
						_this->SendKeyEvent(XK_Delete,    true);
						_this->SendKeyEvent(XK_Delete,    false);
						_this->SendKeyEvent(XK_Alt_L,     false);
						_this->SendKeyEvent(XK_Control_L, false);*/
						return 0;
						
					case ID_CONN_CTLDOWN:
						if (_this->m_opts.m_ViewOnly) return 0;
						_this->SendKeyEvent(XK_Control_L, true);
						return 0;
						
					case ID_CONN_CTLUP:
						if (_this->m_opts.m_ViewOnly) return 0;
						_this->SendKeyEvent(XK_Control_L, false);
						return 0;
						
					case ID_CONN_ALTDOWN:
						if (_this->m_keysSentCount == _this->m_altSentIndex + 2) /*2 since alt up + alt down*/
						{
							::Beep(440,50);
							SetFocus(NULL);
							_this->m_altSentIndex = -1;
						} else
						{
							_this->m_altSentIndex = _this->m_keysSentCount;
							if (_this->m_opts.m_ViewOnly)
							{
								// Killing focus for view only
								::Beep(440,50);
								SetFocus(NULL);
								return 0;
							}
							_this->SendKeyEvent(XK_Alt_L, true);
						}
						return 0;
						
					case ID_CONN_ALTUP:
						if (_this->m_opts.m_ViewOnly) return 0;
						_this->SendKeyEvent(XK_Alt_L, false);
						return 0;
							
					} // end switch lowparam syscommand
					
					break;

				}//end case wm_syscommand
	
				
#ifndef UNDER_CE

#endif
				case WM_QUERYOPEN:
					_this->SetDormant(false);
					return true;


				case WM_SETFOCUS:

					TheAccelKeys.SetWindowHandle(_this->m_opts.m_NoHotKeys ? 0 : hwnd);
					m_focused = true;
					LowLevelHook::g_hwndVNCViewer = hwnd;
					return 0;			

				case WM_KILLFOCUS:
					m_focused = false;
					if (!_this->m_running) return 0;
					if ( _this->m_opts.m_ViewOnly) return 0;
					_this->SendKeyEvent(XK_Alt_L,     false);
					_this->SendKeyEvent(XK_Control_L, false);
					_this->SendKeyEvent(XK_Shift_L,   false);
					_this->SendKeyEvent(XK_Alt_R,     false);
					_this->SendKeyEvent(XK_Control_R, false);
					_this->SendKeyEvent(XK_Shift_R,   false);
					return 0;
					
				case WM_USER + WM_KEYDOWN:
				case WM_USER + WM_KEYUP:
				case WM_USER + WM_SYSKEYDOWN:
				case WM_USER + WM_SYSKEYUP:
				case WM_KEYDOWN:
				case WM_KEYUP:
				case WM_SYSKEYDOWN:
				case WM_SYSKEYUP:

					{
						if (!_this->m_running) return 0;
						if ( _this->m_opts.m_ViewOnly) return 0;
						_this->ProcessKeyEvent((int) wParam, (DWORD) lParam);
						return 0;
					}

				case WM_DEADCHAR:
				case WM_SYSDEADCHAR:
					return 0;
					
				case WM_WINDOWPOSCHANGED:
				case WM_SIZE:
					{
						// Calculate window dimensions
						RECT rect;
						RECT Rtb;
						GetWindowRect(hwnd, &rect);
						_this->m_winwidth = rect.right - rect.left;
						_this->m_winheight = rect.bottom - rect.top ;
						_this->m_opts.m_ShowToolbar = false;
						if( SCALE_MODE == _this->m_opts.m_displayMode )
						{
							_this->m_fScalingDone = false;
							GetClientRect(hwnd, &rect);
							int cx = rect.right - rect.left;
							int cy = rect.bottom - rect.top;
							float fx = float(cx) / max(1,_this->m_si.framebufferWidth);
							float fy = float(cy) / max(1,_this->m_si.framebufferHeight);
							if (fx > fy)
							{
								cx = _this->m_si.framebufferWidth * fy;
							} else
							{
								cy = _this->m_si.framebufferHeight * fx;
							}
							//SetWindowPos( _this->m_hwnd , HWND_TOP , ((rect.right - rect.left)-cx)/2,((rect.bottom - rect.top)-cy)/2, cx , cy , SWP_FRAMECHANGED );
							if (cx>0 && cy>0)
								SetWindowPos( _this->m_hwnd, _this->m_hwndMain , ((rect.right - rect.left)-cx)/2,((rect.bottom - rect.top)-cy)/2, cx , cy , SWP_FRAMECHANGED | SWP_NOZORDER );
							_this->SizeWindow();
							RedrawWindow( _this->m_hwnd , 0 , 0 , TRUE );
							break;
						}
						if( SCROLL_MODE == _this->m_opts.m_displayMode )
						{
							GetClientRect(hwnd, &rect);
							//SetWindowPos( _this->m_hwnd , HWND_TOP , 0,0, rect.right - rect.left , rect.bottom - rect.top , SWP_FRAMECHANGED );
							SetWindowPos( _this->m_hwnd , _this->m_hwndMain , 0,0, rect.right - rect.left , rect.bottom - rect.top , SWP_FRAMECHANGED | SWP_NOZORDER );
							RedrawWindow( _this->m_hwnd , 0 , 0 , TRUE );
						}

						Rtb.top=0;Rtb.bottom=0;	
						
						// If the current window size would be large enough to hold the
						// whole screen without scrollbars, or if we're full-screen,
						// we turn them off.  Under CE, the scroll bars are unchangeable.
		
#ifndef UNDER_CE
						if (_this->InFullScreenMode() ||
							_this->m_winwidth  >= _this->m_fullwinwidth  &&
							_this->m_winheight >= (_this->m_fullwinheight + ((Rtb.bottom - Rtb.top) )) ) 
						{
							//_this->m_winheight >= _this->m_fullwinheight  ) {
							// added by Dmitry 20.10.2006
							if( SCROLL_MODE == _this->m_opts.m_displayMode )
							{
								if (NULL == _this->m_horScrollBar)
									ShowScrollBar(hwnd, SB_HORZ, FALSE);
								/*else
									ShowScrollBar(_this->m_horScrollBar, SB_CTL, FALSE);*/

								if (NULL == _this->m_vertScrollBar)
									ShowScrollBar(hwnd, SB_VERT, FALSE);
								/*else
									ShowScrollBar(_this->m_vertScrollBar, SB_CTL, FALSE);*/
							}
						} else 
						{
							// added by Dmitry 20.10.2006
							if( SCROLL_MODE == _this->m_opts.m_displayMode )
							{
								if (NULL == _this->m_horScrollBar)
									ShowScrollBar(hwnd, SB_HORZ, TRUE);
								/*else
									ShowScrollBar(_this->m_horScrollBar, SB_CTL, TRUE);*/

								if (NULL == _this->m_vertScrollBar)
									ShowScrollBar(hwnd, SB_VERT, TRUE);
								/*else
									ShowScrollBar(_this->m_vertScrollBar, SB_CTL, TRUE);*/
							}
						}
#endif
					
						// Update these for the record
						// And consider that in full-screen mode the window
						// is actually bigger than the remote screen.
						GetClientRect(hwnd, &rect);
						_this->m_cliwidth = min( (int)(rect.right - rect.left), (int)(_this->m_si.framebufferWidth /** _this->m_opts.m_scale_num / _this->m_opts.m_scale_den*/));
						_this->m_cliheight = min( (int)(rect.bottom - rect.top), (int)(_this->m_si.framebufferHeight /** _this->m_opts.m_scale_num / _this->m_opts.m_scale_den*/));
						_this->m_hScrollMax = (int)_this->m_si.framebufferWidth + ((NULL!=_this->m_horScrollBar && (rect.bottom - rect.top) < _this->m_si.framebufferHeight)?GetSystemMetrics(SM_CYHSCROLL):0);/* * _this->m_opts.m_scale_num / _this->m_opts.m_scale_den*/
						_this->m_vScrollMax = (int)_this->m_si.framebufferHeight + ((NULL != _this->m_vertScrollBar && (rect.right - rect.left) < _this->m_si.framebufferWidth)?GetSystemMetrics(SM_CXVSCROLL):0);/**_this->m_opts.m_scale_num / _this->m_opts.m_scale_den*/
						
						int newhpos, newvpos;
						newhpos = max(0,min(_this->m_hScrollPos,_this->m_hScrollMax - max(_this->m_cliwidth, 0)));
						newvpos = max(0,min(_this->m_vScrollPos,_this->m_vScrollMax - max(_this->m_cliheight, 0)));
						if( SCROLL_MODE == _this->m_opts.m_displayMode   )
						{
							ScrollWindowEx(_this->m_hwnd,
										_this->m_hScrollPos - newhpos,
										_this->m_vScrollPos - newvpos,
										NULL, &rect, NULL, NULL,  SW_INVALIDATE);
							
							_this->m_hScrollPos = newhpos;
							_this->m_vScrollPos = newvpos;
							_this->UpdateScrollbars();
						}
						
						
					//Added by: Lars Werner (http://lars.werner.no)
					if(wParam==SIZE_MAXIMIZED&&_this->InFullScreenMode()==FALSE)
					{
						//_this->SetFullScreenMode(!_this->InFullScreenMode());
						//MessageBox(NULL,"Fullscreeen from maximizehora...","KAKE",MB_OK);
						//return 0;
					}
				
					//Modified by: Lars Werner (http://lars.werner.no)
					if(_this->InFullScreenMode()==TRUE)
						return 0;
					else
						break;
					}

				case WM_HSCROLL:
					{
						// added by Dmitry 20.10.2006
						if( _this->m_opts.m_displayMode!=SCROLL_MODE  )
							return TRUE;
						int dx = 0;
						int pos = HIWORD(wParam);
						switch (LOWORD(wParam)) {
						case SB_LINEUP:
							dx = -2; break;
						case SB_LINEDOWN:
							dx = 2; break;
						case SB_PAGEUP:
							dx = _this->m_cliwidth * -1/4; break;
						case SB_PAGEDOWN:
							dx = _this->m_cliwidth * 1/4; break;
						case SB_THUMBPOSITION:
							dx = pos - _this->m_hScrollPos;
						case SB_THUMBTRACK:
							dx = pos - _this->m_hScrollPos;
						}
						_this->ScrollScreen(dx,0);
						
						return 0;
					}
					
				case WM_VSCROLL:
					{
						// added by Dmitry 20.10.2006
						if( _this->m_opts.m_displayMode!=SCROLL_MODE  )
							return TRUE;
						int dy = 0;
						int pos = HIWORD(wParam);
						switch (LOWORD(wParam)) {
						case SB_LINEUP:
							dy = -2; break;
						case SB_LINEDOWN:
							dy = 2; break;
						case SB_PAGEUP:
							dy = _this->m_cliheight * -1/4; break;
						case SB_PAGEDOWN:
							dy = _this->m_cliheight * 1/4; break;
						case SB_THUMBPOSITION:
							dy = pos - _this->m_vScrollPos;
						case SB_THUMBTRACK:
							dy = pos - _this->m_vScrollPos;
						}
						_this->ScrollScreen(0,dy);
						return 0;
					}
					
					// RealVNC 335 method
				case WM_MOUSEWHEEL:
					if (!_this->m_opts.m_ViewOnly)
						_this->ProcessMouseWheel((SHORT)HIWORD(wParam));
					return 0;
				
				//Added by: Lars Werner (http://lars.werner.no) - These is the custom messages from the TitleBar
				case tbWM_CLOSE:
					//_this->m_ptrViewer->Stop(); //Deadlocks at least int replay session mode
					_this->m_ptrViewer->m_stream->GetMainStream()->CancelReceiveOperation();
					return 0;

				case tbWM_MINIMIZE:
					//_this->SetDormant(true);
					//ShowWindow(_this->m_hwnd, SW_MINIMIZE);
					if (NULL != _this->m_ptrViewer)
						_this->m_ptrViewer->OnMinimize();
					return 0;

				case tbWM_MAXIMIZE:
					if (NULL != _this->m_ptrViewer)
						_this->m_ptrViewer->OnRestore();
					return TRUE;
			} // end of iMsg switch
		} // End if Main Window
	}
	if( (_this->m_oldWndProc) && (hwnd == _this->m_hwndMain))
		return CallWindowProc(_this->m_oldWndProc, hwnd, iMsg, wParam, lParam );
	else 
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	
	// We know about an unused variable here.
#pragma warning(disable : 4101)
CATCH_LOG("ClientConnection::WndProc")
	return 0;
}


LRESULT CALLBACK ClientConnection::WndProchwnd(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
TRY_CATCH

	//	HWND parent;
	ClientConnection *_this = (ClientConnection *) GetWindowLong(hwnd, GWL_USERDATA);
	if (_this == NULL) return DefWindowProc(hwnd, iMsg, wParam, lParam);

	if (ClientConnection::m_msgSendRetryStuff == iMsg)
	{
		/// Sending retry stuff
		/// Firstly braking connection on host side
		char code = rfbRetrySession;
		for(int i=0; i< MAX_PATH; ++i) //MAX_PATH is safely more than any of VNC commands length
			_this->WriteExact( &code, 1 );

		/// Sending start command
		int len = strlen( START_COMMAND );
		char data[] =  START_COMMAND;
		_this->WriteExact( data , len );
		_this->m_inited = true;
	} else
	if (ClientConnection::m_msgSendInitialStuff == iMsg)
	{
		/// Sending initial stuff
		_this->m_nServerScale = _this->m_opts.m_nServerScale;
		if (_this->m_nServerScale > 1) 
			_this->SendServerScale(_this->m_nServerScale);

		// Sending view only \ visual pointer options \ catch alpha blend
		if (wParam != 0)
		{
			char buf[2];

			if (_this->m_ptrViewer->m_modeFromViewerSideSelected)
			{
				// Sending view SessionMode only if it was selected on viewer side before
				// session was started
				buf[0] = rfbSetViewOnly;
				buf[1] = _this->m_ptrViewer->m_viewOnly;
				_this->WriteExact(buf,2);
				buf[0] = rfbSetVisualPointer;
				buf[1] = _this->m_ptrViewer->m_visualPointer;
				_this->WriteExact(buf,2);
			}
			// Alphablend is send always
			buf[0] = rfbSetCaptureAlphaBlend;
			buf[1] = _this->m_ptrViewer->m_captureAlphaBlend;
			_this->WriteExact(buf,2);
			// Hide wallpaper on host
			buf[0] = rfbHideWallpaper;
			buf[1] = _this->m_ptrViewer->m_hideWallpaper;
			_this->WriteExact(buf,2);
		}
		/// Requesting full framebuffer update
		_this->SendFullFramebufferUpdateRequest();
		_this->m_inited = true;

	} else
	if (ClientConnection::m_msgSendStopSession == iMsg)
	{
		// Sending stop to RCHost
		SendMessage(hwnd, WM_CLOSE, 0, 0);
		char code = rfbStopSession;
		_this->WriteExact(&code,1);
	}
	else
	if (CRCViewer::m_msgSetAlphaBlend == iMsg)
	{
		char buf[2];
		buf[0] = rfbSetCaptureAlphaBlend;
		buf[1] = wParam;
		_this->WriteExact(buf,2);
	} else
	if (CRCViewer::m_msgSetViewOnly == iMsg)
	{
		char buf[2];
		buf[0] = rfbSetViewOnly;
		buf[1] = wParam;
		_this->WriteExact(buf,2);
	} else
	if (CRCViewer::m_msgSetVisualPointer == iMsg)
	{
		char buf[2];
		buf[0] = rfbSetVisualPointer;
		buf[1] = wParam;
		_this->WriteExact(buf,2);
	}
	else
		switch (iMsg) 
		{
			case WM_VSCROLL:
				return ClientConnection::WndProc(hwnd, iMsg, wParam, lParam);
			case WM_CREATE:
				SetTimer(_this->m_hwnd,3335, 1000, NULL);
				return 0;
			case WM_FULLSCREENUPDATED:
				_this->SendFullFramebufferUpdateRequest();
				return 0;
			case WM_REGIONUPDATED:
				//_this->DoBlit();
				_this->SendAppropriateFramebufferUpdateRequest();
				return 0;
			case WM_PAINT:
				_this->DoBlit();
				return 0;
			case WM_TIMER:
				if (wParam == _this->m_emulate3ButtonsTimer)
				{
					_this->SubProcessPointerEvent( 
						_this->m_emulateButtonPressedX,
						_this->m_emulateButtonPressedY,
						_this->m_emulateKeyFlags);
					KillTimer(_this->m_hwnd, _this->m_emulate3ButtonsTimer);
					_this->m_waitingOnEmulateTimer = false;
					return 0;
				} else if (wParam == _this->m_streamTimer && _this->m_nConfig != 0 && _this->m_nConfig != 1)
				{
					/// Flush stream timer
					try 
					{
						_this->m_asyncStream.FastFlushQeue();
					}
					catch(CStreamException &e)
					{
						MLog_Exception((CExceptionBase(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)))));
						_this->m_streamTimer.reset(0, NULL);
					}
					return 0;
				} else if (wParam == _this->m_aliveTimer)
				{
					/// Alive messages timer
					if (_this->m_recvRecently && _this->m_sentRecently)
					{
						_this->m_recvRecently = false;
						_this->m_sentRecently = false;
						return 0;
					}
					char code = rfbAliveMsg;
					Log.Add(_MESSAGE_,_T("Sending alive message"));
					_this->m_asyncStream.FastSend(&code, sizeof(code));
					return 0;
				}
				break;				
				
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
				if (GetFocus() != _this->m_hwndMain)
				{
					SetFocus(_this->m_hwndMain);
				}
				if (_this->m_SWselect) 
				{
					_this->m_SWpoint.x=LOWORD(lParam);
					_this->m_SWpoint.y=HIWORD(lParam);
					_this->SendSW(_this->m_SWpoint.x,_this->m_SWpoint.y);
					return 0;
				}
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MOUSEMOVE:
				{
					if (_this->m_SWselect) {return 0;}
					if (!_this->m_running) return 0;
					if (GetFocus() != _this->m_hwndMain) return 0;
					int x = LOWORD(lParam);
					int y = HIWORD(lParam);
					wParam = MAKEWPARAM(LOWORD(wParam), 0);
					if ( _this->m_opts.m_ViewOnly) return 0;
					try
					{
						_this->ProcessPointerEvent(x,y, wParam, iMsg);
					}
					catch(CStreamException& e)
					{
						MLog_Exception(e);
						SendMessage(hwnd,WM_CLOSE,0,0);
					}
					return 0;
				}
				
			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
				{
					if (!_this->m_running) return 0;
					if ( _this->m_opts.m_ViewOnly) return 0;
					_this->ProcessKeyEvent((int) wParam, (DWORD) lParam);
					return 0;
				}
				
			case WM_CHAR:
			case WM_SYSCHAR:
#ifdef UNDER_CE
				{
					int key = wParam;
					//vnclog.Print(4,_T("CHAR msg : %02x\n"), key);
					// Control keys which are in the Keymap table will already
					// have been handled.
					if (key == 0x0D  ||  // return
						key == 0x20 ||   // space
						key == 0x08)     // backspace
						return 0;
					
					if (key < 32) key += 64;  // map ctrl-keys onto alphabet
					if (key > 32 && key < 127) {
						_this->SendKeyEvent(wParam & 0xff, true);
						_this->SendKeyEvent(wParam & 0xff, false);
					}
					return 0;
				}
#endif
			case WM_DEADCHAR:
			case WM_SYSDEADCHAR:
				return 0;

			case WM_SETFOCUS:
				SetFocus(_this->m_hwnd);
				return 0;

			case WM_KILLFOCUS:
				return 0;
	
			case WM_CLOSE:
				{					
					// Close the worker thread as well
					_this->KillThread();
					DestroyWindow(hwnd);
					return 0;
				}
				
			case WM_DESTROY:
				{
					// Releasing scrollbars
					if (NULL != _this->m_vertScrollBar)
					{
						if (NULL != _this->m_vertScrollBarParent)
						{
							SetParent(_this->m_vertScrollBar, _this->m_vertScrollBarParent);
						}
					}
					// Releasing keyboard
					m_focused = false;
					if (FULLSCREEN_MODE == _this->m_opts.m_displayMode && m_fullScreen > 0)
						--m_fullScreen;
					// Close the worker thread if necessary
					_this->KillThread();
					#ifndef UNDER_CE
					// Remove us from the clipboard viewer chain
					BOOL res = ChangeClipboardChain( hwnd, _this->m_hwndNextViewer);
					#endif
					if (_this->m_waitingOnEmulateTimer)
					{
						KillTimer(_this->m_hwnd, _this->m_emulate3ButtonsTimer);
						KillTimer(_this->m_hwnd, 3335);
						_this->m_waitingOnEmulateTimer = false;
					}
					
					// We are currently in the main thread.
					// The worker thread should be about to finish if
					// it hasn't already. Wait for it.
					try 
					{
						_this->m_thread->join();
						_this->m_thread.reset();
						delete _this;
					}
					catch (...) 
					{
					// The thread probably hasn't been started yet,
					}
					SetWindowLong(hwnd, GWL_USERDATA, 0);
 					return 0;
				}
				
				
			case WM_QUERYNEWPALETTE:
				{
					TempDC hDC(hwnd);
					
					// Select and realize hPalette
					PaletteSelector p(hDC, _this->m_hPalette);
					InvalidateRect(hwnd, NULL, FALSE);
					UpdateWindow(hwnd);
					return TRUE;
				}
				
			case WM_PALETTECHANGED:
				// If this application did not change the palette, select
				// and realize this application's palette
				if ((HWND) wParam != hwnd)
				{
					// Need the window's DC for SelectPalette/RealizePalette
					TempDC hDC(hwnd);
					PaletteSelector p(hDC, _this->m_hPalette);
					// When updating the colors for an inactive window,
					// UpdateColors can be called because it is faster than
					// redrawing the client area (even though the results are
					// not as good)
#ifndef UNDER_CE
					UpdateColors(hDC);
#else
					InvalidateRect(hwnd, NULL, FALSE);
					UpdateWindow(hwnd);
#endif
					
				}
				break;
				
#ifndef UNDER_CE

			case WM_SIZING:
				{
					if( SCROLL_MODE!=_this->m_opts.m_displayMode )
						return 0;
					// Don't allow sizing larger than framebuffer
					RECT *lprc = (LPRECT) lParam;
					switch (wParam) {
					case WMSZ_RIGHT: 
					case WMSZ_TOPRIGHT:
					case WMSZ_BOTTOMRIGHT:
						lprc->right = min(lprc->right, lprc->left + _this->m_fullwinwidth+1);
						break;
					case WMSZ_LEFT:
					case WMSZ_TOPLEFT:
					case WMSZ_BOTTOMLEFT:
						lprc->left = max(lprc->left, lprc->right - _this->m_fullwinwidth);
						break;
					}
					
					switch (wParam) {
					case WMSZ_TOP:
					case WMSZ_TOPLEFT:
					case WMSZ_TOPRIGHT:
						lprc->top = max(lprc->top, lprc->bottom - _this->m_fullwinheight);
						break;
					case WMSZ_BOTTOM:
					case WMSZ_BOTTOMLEFT:
					case WMSZ_BOTTOMRIGHT:
						lprc->bottom = min(lprc->bottom, lprc->top + _this->m_fullwinheight);
						break;
					}
					RedrawWindow( hwnd , 0 , 0 , TRUE );
					return 0;
				}

			case WM_SETCURSOR:
				{
					// if we have the focus, let the cursor change as normal
					if (GetFocus() == hwnd) 
						break;
					
					HCURSOR hCursor = NULL;
					// Setting arrow cursor for unhandled remote screen area
					if (FULLSCREEN_MODE == _this->m_opts.m_displayMode ||
						SCALE_MODE == _this->m_opts.m_displayMode)
					{
							if (!_this->m_opts.m_scaleUpEnabled && 
								_this->m_opts.m_scale_den/_this->m_opts.m_dx < 1.0 &&
								_this->m_opts.m_scale_den/_this->m_opts.m_dy < 1.0)
							{
								RECT rc;
								GetClientRect(hwnd,&rc);
								int dx = ((rc.right-rc.left) - _this->m_si.framebufferWidth)/2;
								int dy = ((rc.bottom-rc.top) - _this->m_si.framebufferHeight)/2;
								POINT point;
								if (FALSE != GetCursorPos(&point))
								{
									if (FALSE != ScreenToClient(hwnd,&point))
									{
										if (point.x < dx || point.y < dy ||
											point.x > rc.right -dx || point.y > rc.bottom - dy)
										{
											hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
										}
									}
								}
							}
					}
					// Setting cursor for handler remote screen area
					if (NULL == hCursor)
					{
						if (_this->m_SWselect) 
							hCursor= LoadCursor(_this->m_pApp->m_instance, MAKEINTRESOURCE(IDC_CURSOR1));
						else
							switch (_this->m_opts.m_localCursor) 
							{
								case NOCURSOR:
									hCursor = LoadCursor(_this->m_pApp->m_instance, MAKEINTRESOURCE(IDC_NOCURSOR));
									break;
								case NORMALCURSOR:
									hCursor = LoadCursor(NULL, IDC_ARROW);
									break;
								case DOTCURSOR:
								default:
									hCursor = LoadCursor(_this->m_pApp->m_instance, MAKEINTRESOURCE(IDC_DOTCURSOR));
							}
					}
					SetCursor(hCursor);
					return 0;
				}
				
				
			case WM_DRAWCLIPBOARD:
				if (_this->m_inited)
					_this->ProcessLocalClipboardChange();
				return 0;
				
			case WM_CHANGECBCHAIN:
				{
					// The clipboard chain is changing
					HWND hWndRemove = (HWND) wParam;     // handle of window being removed 
					HWND hWndNext = (HWND) lParam;       // handle of next window in chain 
					// If next window is closing, update our pointer.
					if (hWndRemove == _this->m_hwndNextViewer)  
						_this->m_hwndNextViewer = hWndNext;  
					// Otherwise, pass the message to the next link.  
					else if (_this->m_hwndNextViewer != NULL) 
						::SendMessage(_this->m_hwndNextViewer, WM_CHANGECBCHAIN, 
						(WPARAM) hWndRemove,  (LPARAM) hWndNext );  
					return 0;
					
				}
#endif

			// Modif VNCon MultiView support
			// Messages used by VNCon - Copyright (C) 2001-2003 - Alastair Burr
			case WM_GETSCALING:
				{
					WPARAM wPar;
					wPar = MAKEWPARAM(_this->m_hScrollMax, _this->m_vScrollMax);
					SendMessage((HWND)wParam, WM_GETSCALING, wPar, lParam);
					return TRUE;
					
				}
				
			case WM_SETSCALING:
				{
					_this->m_opts.m_scaling = true;
					_this->m_opts.m_scale_num = wParam;
					_this->m_opts.m_scale_den = lParam;
					if (_this->m_opts.m_scale_num == 1 && _this->m_opts.m_scale_den == 1)
						_this->m_opts.m_scaling = false;
					_this->SizeWindow();
					InvalidateRect(hwnd, NULL, TRUE);
					return TRUE;
					
				}
				
			case WM_SETVIEWONLY:
				{
					_this->m_opts.m_ViewOnly = (wParam == 1);
					return TRUE;
				}
			// End Modif for VNCon MultiView support
			}//end switch (iMsg) 
			
			return DefWindowProc(hwnd, iMsg, wParam, lParam);

CATCH_LOG("ClientConnection::WndProchwnd")
	return 0;
}
void ClientConnection::ConvertAll(int width, int height, int xx, int yy,int bytes_per_pixel,BYTE* source,BYTE* dest,int framebufferWidth)
{
TRY_CATCH

	int bytesPerInputRow = width * bytes_per_pixel;
	int bytesPerOutputRow = framebufferWidth * bytes_per_pixel;
	BYTE *sourcepos,*destpos;
	destpos = (BYTE *)dest + (bytesPerOutputRow * yy)+(xx * bytes_per_pixel);
	sourcepos=(BYTE*)source;

    int y;
    width*=bytes_per_pixel;
    for (y=0; y<height; y++) {
        memcpy(destpos, sourcepos, width);
        sourcepos = (BYTE*)sourcepos + bytesPerInputRow;
        destpos = (BYTE*)destpos + bytesPerOutputRow;
    }

CATCH_THROW("ClientConnection::ConvertAll")
}

bool ClientConnection::Check_Rectangle_borders(int x,int y,int w,int h)
{
TRY_CATCH
	if (x<0) return false;
	if (y<0) return false;
	if (x+w>m_si.framebufferWidth) return false;
	if (y+h>m_si.framebufferHeight) return false;
	if (x+w<x) return false;
	if (y+h<y) return false;
	return true;
CATCH_THROW("ClientConnection::Check_Rectangle_borders")
}

bool ClientConnection::StartSession()
{
TRY_CATCH
	CARD8 start_session;
	ReadExact((char *)&start_session, sizeof( start_session ));
	if( rfbStartSession == start_session   )
	{
		char start = rfbStartSession;
		WriteExact( &start , sizeof( start ) );
		return true;
	}
	return false;
CATCH_THROW("ClientConnection::StartSession")
}

void ClientConnection::StopSession()
{
TRY_CATCH
	SendMessage(GetMainWindow(),m_msgSendStopSession, 0, 0);
CATCH_THROW()
}

LRESULT CALLBACK ClientConnection::KeyWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
TRY_CATCH

	ClientConnection *_this = (ClientConnection *) GetWindowLong(hwnd, GWL_USERDATA);
	if (_this == NULL) //return m_oldWndProc(hwnd, iMsg, wParam, lParam);
		DefWindowProc(hwnd, iMsg, wParam, lParam);
	switch (iMsg) 
	{
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			{
				_this->ProcessKeyEvent((int) wParam, (DWORD) lParam);
				return 0;
			};break;
		case WM_SYSCOMMAND:
			{
				switch (LOWORD(wParam))
				{
				case ID_VK_LWINDOWN:
					if (_this->m_opts.m_ViewOnly) return 0;
					_this->SendKeyEvent(XK_Super_L, true);
					return 0;
				case ID_VK_LWINUP:
					if (_this->m_opts.m_ViewOnly) return 0;
					_this->SendKeyEvent(XK_Super_L, false);
					return 0;
				case ID_VK_RWINDOWN:
					if (_this->m_opts.m_ViewOnly) return 0;
					_this->SendKeyEvent(XK_Super_R, true);
					return 0;
				case ID_VK_RWINUP:
					if (_this->m_opts.m_ViewOnly) return 0;
					_this->SendKeyEvent(XK_Super_R, false);
					return 0;
				case ID_VK_APPSDOWN:
					if (_this->m_opts.m_ViewOnly) return 0;
					_this->SendKeyEvent(XK_Menu, true);
					return 0;
				case ID_VK_APPSUP:
					if (_this->m_opts.m_ViewOnly) return 0;
					_this->SendKeyEvent(XK_Menu, false);
					return 0;
				}
		};break;
		case WM_CLOSE:
		{
			HWND parentWindow = _this->m_hwndMain; /*GetParent(mainWindow)*/;
			SetWindowLong( parentWindow , GWL_WNDPROC , (LONG)_this->m_oldWndProc );
		}
		break;

	}
	return _this->m_oldWndProc(hwnd, iMsg, wParam, lParam);//DefWindowProc(hwnd, iMsg, wParam, lParam);
CATCH_LOG("ClientConnection::KeyWndProc")
	return 0;
}

void ClientConnection::RestorePrevDisplayMode()
{
TRY_CATCH
	ChangeDisplayMode(m_prevDisplayMode);
CATCH_THROW()
}

void ClientConnection::ChangeDisplayMode( EDisplayMode mode_ )
{
TRY_CATCH

	m_prevDisplayMode = m_opts.m_displayMode;

	BOOL action = TRUE;
	if (m_opts.m_displayMode == FULLSCREEN_MODE)
		--m_fullScreen;
	EDisplayMode oldMode = m_opts.m_displayMode;
	m_opts.m_displayMode = mode_;
	switch( mode_ )
	{
		case SCALE_MODE:
			{
				m_opts.m_fAutoScaling = true;
				m_opts.m_scaling = true;
				m_fScalingDone = false;	
				switch(oldMode)
				{
					case SCROLL_MODE:
						{
							DWORD style = GetWindowLong( m_hwndMain , GWL_STYLE );
							style= style& ~WS_VSCROLL & ~WS_HSCROLL;
							SetWindowLong( m_hwndMain , GWL_STYLE , style );
							SetWindowPos( m_hwndMain, 0,0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED );
							break;
						}
					case FULLSCREEN_MODE:
						{
							TitleBar.Hide();
							ToggleFullScreen( false );
							SetParent(m_hwnd,m_hwndMain);
							SetWindowPos(m_hwnd, HWND_BOTTOM,  0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOACTIVATE);
							break;
						}
				}
				m_hScrollPos = 0;
				m_vScrollPos = 0;
				//m_opts.m_displayMode = mode_;
				PostMessage( m_hwndMain, WM_SIZE, 0, 0 );
			};break;
		case SCROLL_MODE:
			{
				switch(oldMode)
				{
					case FULLSCREEN_MODE:
						{
							TitleBar.Hide();
							ToggleFullScreen( false );
							SetParent(m_hwnd,m_hwndMain);
							SetWindowPos(m_hwnd, HWND_BOTTOM,  0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOACTIVATE);
							break;
						}
				}
				//m_opts.m_displayMode = mode_;
				m_opts.m_fAutoScaling = false;
				m_opts.m_scaling = false;
				m_fScalingDone = false;
				PostMessage( m_hwndMain, WM_SIZE, 0, 0 );
				//SetWindowLong(m_hwnd, GWL_STYøóiøLE , WS_HSCROLL|WS_VSCROLL);
			};break;
		case FULLSCREEN_MODE:
			{
				SetParent(m_hwnd,NULL);
				//m_opts.m_displayMode = mode_;
				int cx(GetSystemMetrics( SM_CXSCREEN ));
				int cy(GetSystemMetrics( SM_CYSCREEN ));
				if (cx == m_si.framebufferWidth && cy == m_si.framebufferHeight)
				{
					m_opts.m_fAutoScaling = false;
					m_opts.m_scaling = false;
					m_fScalingDone = false;	
				} else
				{
					m_opts.m_fAutoScaling = true;
					m_opts.m_scaling = true;
					m_fScalingDone = false;	
				}
				m_hScrollPos = 0;
				m_vScrollPos = 0;

				ToggleFullScreen( true );

				SetWindowPos(m_hwnd, HWND_TOPMOST,  0, 0, cx, cy, 0);
				PostMessage( m_hwndMain, WM_SIZE, 0, 0 );
				TitleBar.DisplayWindow(TRUE, TRUE);
				++m_fullScreen;
			}
			break;
	};
	RECT rect;
	GetClientRect(m_hwnd, &rect);
	if (NULL != m_horScrollBar)
		ShowScrollBar(m_horScrollBar, SB_CTL, m_opts.m_displayMode == SCROLL_MODE && (rect.right - rect.left) < m_si.framebufferWidth);
	if (NULL != m_vertScrollBar)
		ShowScrollBar(m_vertScrollBar, SB_CTL, m_opts.m_displayMode == SCROLL_MODE && (rect.bottom - rect.top) < m_si.framebufferHeight);
	SizeWindow();
	RedrawWindow( m_hwnd , 0 , 0 , RDW_ERASE );
	RedrawWindow( m_hwndMain , 0 , 0 , RDW_ERASE );
	if (NULL != m_ptrViewer)
		m_ptrViewer->NorifyDisplayModeChanged(mode_);

CATCH_THROW("ClientConnection::ChangeDisplayMode")
}

void ClientConnection::TurnOffFullScreen()
{
TRY_CATCH
	if (m_opts.m_displayMode == FULLSCREEN_MODE)
		ChangeDisplayMode(SCROLL_MODE);
CATCH_LOG("ClientConnection::TurnOffFullScreen")
}

HWND ClientConnection::GetMainWindow()
{
TRY_CATCH
	//return m_hwndMain;
	return m_hwnd;
CATCH_THROW("ClientConnection::GetMainWindow")
}

HWND ClientConnection::GetOriginalWindow()
{
TRY_CATCH
	return m_hwndMain;
CATCH_THROW()
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam)
{
	BOOL fAction = static_cast<BOOL>( lParam );
	if( (GetWindowLong( hwnd , GWL_STYLE ) == 0x94000C00 ) &&  (GetWindowLong( hwnd , GWL_EXSTYLE ) == 0x00080088 ) )
	{
		ShowWindow(hwnd, fAction?SW_HIDE:SW_SHOWNORMAL);
		return FALSE;
	}
	return TRUE;
}


#pragma warning(default :4101)
