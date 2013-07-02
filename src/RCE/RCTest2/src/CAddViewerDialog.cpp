// CAddViewerDialog.cpp : implementation file
//

#include "stdafx.h"
#include "RCTest2.h"
#include "CAddViewerDialog.h"
#include "caddviewerdialog.h"
#include <RCEngine/Streaming/COutStreamGZipped.h>
#include <RCEngine/Streaming/CInStreamGZipped.h>
#include <NWL/Streaming/CDirectNetworkStream.h>
#include "CViewerDlg.h"
#include "CFactoryConnectDlg.h"

typedef void (CAddViewerDialog::*StreamConnectEvent)();
typedef void (CAddViewerDialog::*StreamDisconnectEvent)();
typedef void (CAddViewerDialog::*StreamErrorEvent)();

const tstring CAddViewerDialog::m_fullScreenStr	= "full screen";
const tstring CAddViewerDialog::m_scaleStr		= "scale mode";
const tstring CAddViewerDialog::m_scrollStr		= "scroll mode";

#define rfbEncodingRaw						0
#define rfbEncodingCopyRect					1
#define rfbEncodingRRE						2
#define rfbEncodingCoRRE					4
#define rfbEncodingHextile					5
#define rfbEncodingZlib						6
#define rfbEncodingTight					7
#define rfbEncodingZlibHex					8
#define rfbEncodingUltra					9
#define rfbEncodingZRLE						16
#define rfbEncodingCache					0xFFFF0000
#define rfbEncodingCacheEnable				0xFFFF0001
#define rfbEncodingXOR_Zlib					0xFFFF0002
#define rfbEncodingXORMonoColor_Zlib		0xFFFF0003
#define rfbEncodingXORMultiColor_Zlib		0xFFFF0004
#define rfbEncodingSolidColor				0xFFFF0005
#define rfbEncodingXOREnable				0xFFFF0006
#define rfbEncodingCacheZip					0xFFFF0007
#define rfbEncodingSolMonoZip				0xFFFF0008
#define rfbEncodingUltraZip					0xFFFF0009


// CAddViewerDialog dialog

IMPLEMENT_DYNAMIC(CAddViewerDialog, CDialog)
CAddViewerDialog::CAddViewerDialog(CWnd* pParent /*=NULL*/)
	:	CDialog(CAddViewerDialog::IDD, pParent),
		m_shadowStream(reinterpret_cast<COutStreamGZipped*>(NULL)),
		m_stream(reinterpret_cast<CDirectNetworkStream*>(NULL)),
		m_selEncoding(FALSE),
		m_selColors(FALSE),
		m_useCustomOptions(false),
		m_viewOnly(false),
		m_visualPointer(false)
		, m_captureLayered(FALSE)
{
TRY_CATCH

	m_dispModeStr[0] = 0;
	m_stream.reset(new CDirectNetworkStream());
	m_stream->SetConnectedEvent(boost::bind( &CAddViewerDialog::OnConnected, this, _1 ));
	m_stream->SetDisconnectedEvent(boost::bind( &CAddViewerDialog::OnDisconnected, this, _1 ));
	m_stream->SetConnectErrorEvent(boost::bind( &CAddViewerDialog::OnConnectError, this, _1, _2 ));

	#define BASE IDC_ZRLE_RADIO
	m_encodings[IDC_ZRLE_RADIO		- BASE] = rfbEncodingZRLE;
	m_encodings[IDC_ZLIBXOR_RADIO	- BASE] = rfbEncodingXOR_Zlib;
	m_encodings[IDC_ZLIBHEX_RADIO	- BASE] = rfbEncodingZlibHex;
	m_encodings[IDC_HEXTILE_RADIO2	- BASE] = rfbEncodingHextile;
	m_encodings[IDC_RRE_RADIO		- BASE] = rfbEncodingRRE;
	m_encodings[IDC_CoRRE_RADIO		- BASE] = rfbEncodingCoRRE;
	m_encodings[IDC_RAW_RADIO		- BASE] = rfbEncodingRaw;
	m_encodings[IDC_Ultra_RADIO		- BASE] = rfbEncodingUltra;
	m_encodings[IDC_TIGHT_RADIO		- BASE] = rfbEncodingTight;

CATCH_THROW("CAddViewerDialog::CAddViewerDialog")
}

CAddViewerDialog::~CAddViewerDialog()
{
}

void CAddViewerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_IP, m_hostEditBox);
	DDX_Control(pDX, IDC_EDIT_DEST_PORT, m_remotePortEditBox);
	DDX_Control(pDX, IDC_EDIT_LOCAL_PORT, m_localPortEditBox);
	DDX_Control(pDX, IDC_BUTTON_CONNECT, m_connectButton);
	DDX_Control(pDX, IDC_BUTTON_SHADOW_STREAM, m_setShadowStreamButton);
	DDX_Control(pDX, IDC_DISPLAY_MODE_COMBO, m_displayModeCombo);
	DDX_Control(pDX, IDC_ENCODING_STATIC, m_encodingGroupBox);
	DDX_Control(pDX, IDC_AUTODETECT_CHECK, m_autoDetectSettingsCheck);
	DDX_Radio(pDX, IDC_ZRLE_RADIO, m_selEncoding);
	DDX_Radio(pDX, IDC_FULL_RADIO, m_selColors);
	DDX_Control(pDX, IDC_COPYRECT_CHECK, m_useCopyRectCheck);
	DDX_Control(pDX, IDC_COLORS_STATIC, m_colorsGroupBox);
	DDX_Control(pDX, IDC_CACHEENC_CHECK, m_useCacheCheck);
	DDX_Control(pDX, IDC_ZIPTIGHT_CHECK, m_zipTightCompressionCheck);
	DDX_Control(pDX, IDC_JPEGQ_CHECK, m_jpegQualityCheck);
	DDX_Control(pDX, IDC_ZIPCOMPRESSION_EDIT, m_zipCompressionEdit);
	DDX_Control(pDX, IDC_JPEGQ_EDIT, m_jpegQualityEdit);
	DDX_Control(pDX, IDC_VIEW_ONLY_CHECK, m_viewOnlyCheckBox);
	DDX_Control(pDX, IDC_VISUAL_POINTER_CHECK, m_visualPointerCheckBox);
	DDX_Control(pDX, IDC_EDIT_SOURCE_PEER, m_sourcePeerEdit);
	DDX_Control(pDX, IDC_EDIT_DEST_PEER, m_destPeerEdit);
	DDX_Check(pDX, IDC_LAYEREDCAPTURE_CHECK, m_captureLayered);
	DDX_Control(pDX, IDC_EDIT_SESSION_ID,m_sessionIdEdit);
}


BEGIN_MESSAGE_MAP(CAddViewerDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_SHADOW_STREAM, OnBnClickedButtonShadowStream)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_FROM_SHADOW_STREAM, OnBnClickedButtonCreateFromShadowStream)
	ON_CBN_SELCHANGE(IDC_DISPLAY_MODE_COMBO, OnCbnSelchangeDisplayModeCombo)
	ON_BN_CLICKED(IDC_AUTODETECT_CHECK, OnBnClickedAutodetectCheck)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT2, OnBnClickedButtonConnect2)
END_MESSAGE_MAP()


// CAddViewerDialog message handlers

void CAddViewerDialog::OnBnClickedButtonConnect()
{
TRY_CATCH

	char buf[MAX_PATH];
	m_remotePortEditBox.GetWindowText(buf,MAX_PATH);
	int port = atoi(buf);
	m_localPortEditBox.GetWindowText(buf,MAX_PATH);
	int localport = atoi(buf);
	m_hostEditBox.GetWindowText(buf,MAX_PATH);
	tstring addr;
	addr.assign(buf);

	STLSCredentials secret; //TODO: check secret
	secret.UserID = "testuser";
	secret.Key = "testuser";
	m_stream->SetCredentials(secret);
	STLSSuite suite;
	suite.Compression = PRS_LZO;//PRS_NULL;
	suite.Cipher = CPH_AES_256;
	suite.KeyExchange = KX_PSK;
	m_stream->SetSuite(suite);
	m_stream->SetConnectTimeout(60 * 1000); //TODO what timeout should be here?
	//m_stream->SetConnectThroughProxy(false);
	m_stream->SetLocalAddr(localport);
	m_stream->SetRemoteAddr(addr, port);
	m_stream->Connect(true /*async*/);

	m_connectButton.EnableWindow(false);
	m_hostEditBox.EnableWindow(false);
	m_remotePortEditBox.EnableWindow(false);
	m_localPortEditBox.EnableWindow(false);
	
CATCH_LOG("CAddViewerDialog::OnBnClickedButtonConnect")}

void CAddViewerDialog::OnBnClickedButtonShadowStream()
{
TRY_CATCH

	CFileDialog fileDialog(	FALSE /*save*/,
							"rce",
							"session");
	fileDialog.DoModal();
	tstring fileName = static_cast<LPCSTR>(fileDialog.GetPathName());
	COutStreamGZipped *fileStream = new COutStreamGZipped(fileName);
	m_shadowStream.reset(fileStream);

CATCH_LOG("CAddViewerDialog::OnBnClickedButtonShadowStream")
}

boost::shared_ptr<CAbstractStream> CAddViewerDialog::GetNewConnectedStream()
{
TRY_CATCH
	if (DoModal() == IDOK)
	{
		/// Since dialog, accepting this event is closed - disconnecting evenets
		m_stream->SetConnectedEvent(NULL);
		m_stream->SetDisconnectedEvent(NULL);
		m_stream->SetConnectErrorEvent(NULL);
		boost::shared_ptr<CAbstractStream> stream;
		if (m_astream.get())
			stream = m_astream;
		else 
			stream = m_stream;
		/// This is an example how to shadow streams without CRCHost internal shadowing
		if (m_shadowStream.get())
		{
			boost::shared_ptr<CShadowedStream> shadowedStream(new CShadowedStream(stream, CShadowedStream::INPUT));
			shadowedStream->SetShadowStream(m_shadowStream);
			//m_shadowStream.release();
			return shadowedStream;
		}
		return stream;
	}
	/// Failed to connect
	m_stream.reset();
	return m_stream;
CATCH_THROW("CAddViewerDialog::GetNewConnectedStream")
}

BOOL CAddViewerDialog::OnInitDialog()
{
	BOOL result(FALSE);
TRY_CATCH
	result = CDialog::OnInitDialog();
	m_localPortEditBox.SetWindowText("5900");
	m_remotePortEditBox.SetWindowText("5900");
	m_displayModeCombo.AddString(m_fullScreenStr.c_str());
	m_displayModeCombo.AddString(m_scaleStr.c_str());
	m_displayModeCombo.AddString(m_scrollStr.c_str());

	m_autoDetectSettingsCheck.SetCheck(TRUE);
	m_zipCompressionEdit.SetWindowText("6");
	m_jpegQualityEdit.SetWindowText("6");
	OnBnClickedAutodetectCheck();

CATCH_LOG("CAddViewerDialog::OnInitDialog")
	return result;
}

void CAddViewerDialog::OnConnectError(void*, EConnectErrorReason reason)
{
TRY_CATCH
	PostMessage(WM_COMMAND,IDCANCEL,0);
CATCH_LOG("CAddViewerDialog::OnConnectError")
}

void CAddViewerDialog::OnConnected(void*)
{
TRY_CATCH
	PostMessage(WM_COMMAND,IDOK,0);
CATCH_LOG("CAddViewerDialog::OnConnected")
}

void CAddViewerDialog::OnDisconnected(void*)
{
TRY_CATCH
	PostMessage(WM_COMMAND,IDCANCEL,0);
CATCH_LOG("CAddViewerDialog::OnDisconnected")
}

void CAddViewerDialog::OnBnClickedButtonCreateFromShadowStream()
{
TRY_CATCH

	CFileDialog fileDialog(	TRUE /*open*/,
							"rce",
							"session");
	if (fileDialog.DoModal() == IDOK)
	{
		tstring fileName = static_cast<LPCSTR>(fileDialog.GetPathName());
		CInStreamGZipped *fileStream = new CInStreamGZipped(fileName);
		CViewerDlg *viewer = new CViewerDlg(NULL, boost::shared_ptr<CAbstractStream>(fileStream));
		/// Setting display mode
		if (DisplayModeSelected())
			viewer->SetDisplayMode(GetDisplayMode());
		ReadOptions();
		SetOptions(viewer);
		viewer->Start();
		EndDialog(IDCANCEL);
	}

CATCH_LOG("CAddViewerDialog::OnBnClickedButtonCreateFromShadowStream")
}

EDisplayMode CAddViewerDialog::GetDisplayMode()
{
TRY_CATCH
	
	tstring displayMode(m_dispModeStr);
	if (m_fullScreenStr == displayMode) return FULLSCREEN_MODE;
	if (m_scaleStr == displayMode) return  SCALE_MODE;
	if (m_scrollStr == displayMode) return SCROLL_MODE;
	throw MCException("Unknown display mode");

CATCH_THROW("")
}

bool CAddViewerDialog::DisplayModeSelected()
{
TRY_CATCH
	return strlen(m_dispModeStr);
CATCH_LOG("CAddViewerDialog::DisplayModeSelected")
	return false;
}
void CAddViewerDialog::OnCbnSelchangeDisplayModeCombo()
{
TRY_CATCH
	m_displayModeCombo.GetWindowText(m_dispModeStr,MAX_PATH);
CATCH_LOG("CAddViewerDialog::OnCbnSelchangeDisplayModeCombo")
}

void CAddViewerDialog::OnBnClickedAutodetectCheck()
{
TRY_CATCH
	if (m_autoDetectSettingsCheck.GetCheck())
	{
		for(int i = IDC_ZRLE_RADIO; i<= IDC_JPEGQ_EDIT; ++i)
		{
			GetDlgItem(i)->EnableWindow(false);
			m_useCustomOptions = false;
		}
	} else
	{
		for(int i = IDC_ZRLE_RADIO; i<= IDC_JPEGQ_EDIT; ++i)
		{
			GetDlgItem(i)->EnableWindow(true);
			m_useCustomOptions = true;
		}
	}
CATCH_LOG("CAddViewerDialog::OnBnClickedAutodetectCheck")
}

void CAddViewerDialog::SetOptions(CRCViewer* viewer)
{
TRY_CATCH
	/// Setting options
	if (m_useCustomOptions)
	{
		viewer->SetCustomOptions(m_customOptions);
	}
	viewer->SetSessionMode(VIEW_ONLY, m_viewOnly);
	viewer->SetSessionMode(VISUAL_POINTER, m_visualPointer);
	viewer->SetCaptureAlphaBlend(m_captureLayered);

CATCH_THROW("CAddViewerDialog::SetOptions")
}

void CAddViewerDialog::OnOK()
{
TRY_CATCH
	ReadOptions();
	CDialog::OnOK();
CATCH_LOG("CAddViewerDialog::OnOK")
}

void CAddViewerDialog::ReadOptions()
{
TRY_CATCH
	if (m_useCustomOptions) //TODO: remove this to Cloes event handler
	{
		UpdateData();
		m_customOptions.m_colorsCount = m_selColors;
		m_customOptions.m_PreferredEncoding = m_encodings[m_selEncoding]; ///TODO: check this
		m_customOptions.m_useCompressLevel = m_zipTightCompressionCheck.GetCheck();
		char buf[MAX_PATH];
		m_zipCompressionEdit.GetWindowText(buf,MAX_PATH);
		m_customOptions.m_compressLevel = atoi(buf);
		m_customOptions.m_enableJpegCompression = m_jpegQualityCheck.GetCheck();
		m_jpegQualityEdit.GetWindowText(buf,MAX_PATH);
		m_customOptions.m_jpegQualityLevel = atoi(buf);
	}
	m_viewOnly = m_viewOnlyCheckBox.GetCheck();
	m_visualPointer = m_visualPointerCheckBox.GetCheck();
CATCH_THROW("CAddViewerDialog::ReadOptions")
}

void CAddViewerDialog::OnBnClickedButtonConnect2()
{
TRY_CATCH

	char buf[MAX_PATH];
	tstring destPeer,sourcePeer,sessionId;
	m_destPeerEdit.GetWindowText(buf,MAX_PATH);
	destPeer = buf;
	m_sourcePeerEdit.GetWindowText(buf,MAX_PATH);
	sourcePeer = buf;
	m_sessionIdEdit.GetWindowText(buf,MAX_PATH);
	sessionId = buf;
	m_astream = CFactoryConnectDlg().GetNewStream(sessionId,sourcePeer, destPeer, 30000 /*30 secs*/, false /*master role. Allways false for viewer*/);
	OnConnected(NULL);	

CATCH_LOG("CAddViewerDialog::OnBnClickedButtonConnect2")
}