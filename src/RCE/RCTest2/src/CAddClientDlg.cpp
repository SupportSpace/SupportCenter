// CAddClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RCTest2.h"
#include "CAddClientDlg.h"
#include "caddclientdlg.h"
#include <RCEngine/Streaming/COutStreamGZipped.h>
#include <NWL/Streaming/CDirectNetworkStream.h>
#include <NWL/Streaming/CIMStub.h>
#include "CFactoryConnectDlg.h"

// CAddClientDlg dialog

typedef void (CAddClientDlg::*StreamConnectEvent)();
typedef void (CAddClientDlg::*StreamDisconnectEvent)();
typedef void (CAddClientDlg::*StreamErrorEvent)();


IMPLEMENT_DYNAMIC(CAddClientDlg, CDialog)
CAddClientDlg::CAddClientDlg(CWnd* pParent /*=NULL*/)
	:	CDialog(CAddClientDlg::IDD, pParent),
		m_shadowStream(reinterpret_cast<COutStreamGZipped*>(NULL)),
		m_stream(reinterpret_cast<CDirectNetworkStream*>(NULL))
{
TRY_CATCH

	m_stream.reset(new CDirectNetworkStream());
	m_stream->SetConnectedEvent(boost::bind( &CAddClientDlg::OnConnected, this, _1 ));
	m_stream->SetDisconnectedEvent(boost::bind( &CAddClientDlg::OnDisconnected, this, _1 ));
	m_stream->SetConnectErrorEvent(boost::bind( &CAddClientDlg::OnConnectError, this, _1, _2 ));

CATCH_THROW("CAddClientDlg::CAddClientDlg")
}

CAddClientDlg::~CAddClientDlg()
{
}

void CAddClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_IP, m_hostEditBox);
	DDX_Control(pDX, IDC_EDIT_DEST_PORT, m_remotePortEditBox);
	DDX_Control(pDX, IDC_EDIT_LOCAL_PORT, m_localPortEditBox);
	DDX_Control(pDX, IDC_BUTTON_CONNECT, m_connectButton);
	DDX_Control(pDX, IDC_BUTTON_SHADOW_STREAM, m_setShadowStreamButton);
	DDX_Control(pDX, IDC_EDIT_SOURCE_PEER, m_sourcePeerEdit);
	DDX_Control(pDX, IDC_EDIT_DEST_PEER, m_destPeerEdit);
	DDX_Control(pDX, IDC_EDIT_SESSION_ID,m_sessionIdEdit);
}


BEGIN_MESSAGE_MAP(CAddClientDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SHADOW_STREAM, OnBnClickedButtonShadowStream)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, OnBnClickedButtonConnect)
	ON_COMMAND(ID_OPTIONS_FULLSCREENMODE, OnOptionsFullscreenmode)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT2, OnBnClickedButtonConnect2)
	ON_BN_CLICKED(IDC_BUTTON_RESET_SERVER, OnBnClickedButtonResetServer)
END_MESSAGE_MAP()

BOOL CAddClientDlg::OnInitDialog()
{
	BOOL result(FALSE);
TRY_CATCH
	result = CDialog::OnInitDialog();
	m_localPortEditBox.SetWindowText("5900");
	m_remotePortEditBox.SetWindowText("5900");
CATCH_LOG("CAddClientDlg::OnInitDialog")
	return result;
}

// CAddClientDlg message handlers

void CAddClientDlg::OnBnClickedButtonShadowStream()
{
TRY_CATCH

	CFileDialog fileDialog(	FALSE /*save*/,
							"rce",
							"session");
	fileDialog.DoModal();
	tstring fileName = static_cast<LPCSTR>(fileDialog.GetPathName());
	COutStreamGZipped *fileStream = new COutStreamGZipped(fileName);
	m_shadowStream.reset(fileStream);

CATCH_LOG("CAddClientDlg::OnBnClickedButtonShadowStream")
}

boost::shared_ptr<CAbstractStream> CAddClientDlg::GetNewConnectedStream()
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
			boost::shared_ptr<CShadowedStream> shadowedStream(new CShadowedStream(stream, CShadowedStream::OUTPUT));
			shadowedStream->SetShadowStream(m_shadowStream);
			//m_shadowStream.release();
			return shadowedStream;
		}
		return stream;
	}
	m_stream.reset();
	return m_stream;
CATCH_THROW("CAddClientDlg::GetNewConnectedStream")
}

void CAddClientDlg::OnConnectError(void*, EConnectErrorReason reason)
{
TRY_CATCH
	PostMessage(WM_COMMAND,IDCANCEL,0);
CATCH_LOG("CAddClientDlg::OnConnectError")
}

void CAddClientDlg::OnConnected(void*)
{
TRY_CATCH
	PostMessage(WM_COMMAND,IDOK,0);
CATCH_LOG("CAddClientDlg::OnConnected")
}

void CAddClientDlg::OnDisconnected(void*)
{
TRY_CATCH
	PostMessage(WM_COMMAND,IDCANCEL,0);
CATCH_LOG("CAddClientDlg::OnDisconnected")
}

void CAddClientDlg::OnBnClickedButtonConnect()
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

CATCH_LOG("CAddClientDlg::OnBnClickedButtonConnect")
}


void CAddClientDlg::OnOptionsFullscreenmode()
{
	// TODO: Add your command handler code here
	char buf[MAX_PATH] = "13";
	int port = atoi(buf);
}

void CAddClientDlg::OnBnClickedButtonConnect2()
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
	m_astream = CFactoryConnectDlg().GetNewStream(sessionId,sourcePeer, destPeer, 30000 /*30 secs*/, true /*master role. Allways true for host*/);
	OnConnected(NULL);	

CATCH_LOG("CAddClientDlg::OnBnClickedButtonConnect2")
}

void CAddClientDlg::OnBnClickedButtonResetServer()
{
TRY_CATCH
	//char hostname[MAX_PATH];
	CIMStub im("RCTest2");
	im.ResetServer();
CATCH_THROW("CAddClientDlg::OnBnClickedButtonResetServer")
}
