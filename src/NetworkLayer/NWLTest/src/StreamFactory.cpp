// StreamFactory.cpp : implementation file
//

#include "stdafx.h"
#include <Winsock2.h>
#include "NetworkLayerTest.h"
#include "StreamFactory.h"
#include ".\streamfactory.h"
#include <AidLib/Logging/CLog.h>
#include <NWL/Streaming/CIMStub.h>
#include "CFactoryConnectDlg.h"
#include <NWL/Streaming/CNetworkLayer.h>

// CStreamFactoryDlg dialog

IMPLEMENT_DYNAMIC(CStreamFactoryDlg, CDialog)
CStreamFactoryDlg::CStreamFactoryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStreamFactoryDlg::IDD, pParent)
	, m_MasterRole(false)
	, m_connectTimeout(0)
{
}

CStreamFactoryDlg::~CStreamFactoryDlg()
{
}

void CStreamFactoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOCAL_PEED_EDIT, m_localPeerEdit);
	DDX_Control(pDX, IDC_REMOTE_PEED_EDIT, m_remotePeerEdit);
	DDX_Control(pDX, IDC_MASTER_CHECK, m_masterCheckBox);
	DDX_Control(pDX, IDC_SERVER_USERID_EDIT, m_SrvUserId);
	DDX_Control(pDX, IDC_SERVER_PASSWORD_EDIT, m_SrvPassword);
	DDX_Control(pDX, IDC_BUTTON_SEND, m_sendBtn);
	DDX_Control(pDX, IDC_CONNECT_BUTTON, m_connectBtn);
	DDX_Control(pDX, IDC_DISCONNECT_BUTTON, m_disconnectBtn);
	DDX_Control(pDX, IDC_CHECK_ADD_BRAKLINE, m_AddBreakLine);
	DDX_Control(pDX, IDC_LIST_RECEIVE, m_ReceiveList);
	DDX_Control(pDX, IDC_LIST_SEND, m_SendList);
	DDX_Control(pDX, IDC_EDIT_SEND_TEXT, m_SendText);
	DDX_Text(pDX, IDC_CONNECT_TIMEOUT_EDIT, m_connectTimeout);
	DDX_Control(pDX, IDC_ASYNC_CONNECT_CHECK, m_asyncConnectCheck);
	DDX_Control(pDX, IDC_CUSTOM_PORT_EDIT, m_customPort);
	DDX_Control(pDX, IDC_CUSTOM_PORT_CHECK, m_useCustomPort);
}


BEGIN_MESSAGE_MAP(CStreamFactoryDlg, CDialog)
	ON_BN_CLICKED(IDC_CONNECT_BUTTON, OnBnClickedConnectButton)
	ON_BN_CLICKED(IDC_RESETIM_BUTTON, OnBnClickedResetimButton)
	ON_BN_CLICKED(IDC_DISCONNECT_BUTTON, OnBnClickedDisconnectButton)
	ON_BN_CLICKED(IDC_BUTTON_SEND, OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, OnBnClickedButtonClear)
END_MESSAGE_MAP()


// CStreamFactoryDlg message handlers
CStreamFactoryDlg *m_this;

void CStreamFactoryDlg::OnBnClickedConnectButton()
{
TRY_CATCH

	UpdateData();
	TCHAR buf[MAX_PATH];
	m_localPeerEdit.GetWindowText(buf,MAX_PATH);
	tstring sourcePeer = buf;
	m_remotePeerEdit.GetWindowText(buf,MAX_PATH);
	tstring destPeer = buf;
	m_SrvUserId.GetWindowText(buf,MAX_PATH);
	tstring srvUserId = buf;
	m_SrvPassword.GetWindowText(buf,MAX_PATH);
	tstring srvPassword = buf;
	m_SrvSessionId.GetWindowText(buf,MAX_PATH);
	tstring sessionId = buf;

	bool masterRole = (BST_CHECKED == m_masterCheckBox.GetCheck());
	bool async = (BST_CHECKED == m_asyncConnectCheck.GetCheck());

	bool useCustomPort = (BST_CHECKED == m_useCustomPort.GetCheck());
	m_customPort.GetWindowText(buf, MAX_PATH);
	int customPort = _ttoi(buf);
	
	NWL_INSTANCE.SetCustomDirectStreamPort(customPort);
	NWL_INSTANCE.SetDirectStreamPortMode(EPSM_CUSTOM);

	m_stream = CFactoryConnectDlg(async).GetNewStream(sessionId,sourcePeer, destPeer, m_connectTimeout, masterRole, srvUserId, srvPassword );
	Log.Add(_MESSAGE_,"%s connected",destPeer.c_str());

	m_this = this;
	timer = SetTimer( 1, 100, &CStreamFactoryDlg::OnTimer );
	m_disconnectBtn.EnableWindow(true);
	m_connectBtn.EnableWindow(false);
	m_sendBtn.EnableWindow(true);
	m_SendList.AddString( "** CONNECTED **" );


CATCH_LOG("CStreamFactoryDlg::OnBnClickedConnectButton")
}

void CStreamFactoryDlg::OnBnClickedResetimButton()
{
TRY_CATCH
	char hostname[MAX_PATH];
	gethostname(hostname,MAX_PATH);
	CIMStub im(hostname);
	im.ResetServer();
CATCH_LOG("CStreamFactoryDlg::OnBnClickedResetimButton")
}

/// On init dialog override
BOOL CStreamFactoryDlg::OnInitDialog()
{
	BOOL res;
TRY_CATCH
	res = CDialog::OnInitDialog();
	m_SrvUserId.SetWindowText("TestUser");
	m_SrvPassword.SetWindowText("TestUser");
	m_connectTimeout = 20000;
	m_customPort.SetWindowText("7000");
	UpdateData(FALSE);
CATCH_LOG("CStreamFactoryDlg::OnInitDialog")
	return res;
}

void CStreamFactoryDlg::OnBnClickedDisconnectButton()
{
TRY_CATCH
	KillTimer( 1 );
	if (m_stream.get()) m_stream.reset();
	m_disconnectBtn.EnableWindow(false);
	m_connectBtn.EnableWindow(true);
	m_sendBtn.EnableWindow(false);
	m_SendList.AddString( "** DISCONNECTED **" );
CATCH_LOG("CStreamFactoryDlg::OnBnClickedDisconnectButton")
}

void CStreamFactoryDlg::OnBnClickedButtonSend()
{
TRY_CATCH

	char buf[MAX_PATH];
	int len;

	strcpy_s( buf, MAX_PATH, "Sending ->" );
	m_SendList.AddString( buf );
	FillMemory( buf, MAX_PATH, 0 );

	m_SendText.GetWindowText( buf, MAX_PATH );
	m_SendList.AddString( buf );
	len = (unsigned int)strlen( buf );
	if ( BST_CHECKED == m_AddBreakLine.GetCheck() )
	{
		buf[len] = '\r';
		len++;
		buf[len] = '\n';
		len++;
	}

	m_stream->Send( buf, len );

	FillMemory( buf, MAX_PATH, 0 );
	strcpy_s( buf, MAX_PATH, "->Sent" );
	m_SendList.AddString( buf );

	m_SendText.SetWindowText( "" );

CATCH_LOG("CStreamFactoryDlg::OnBnClickedButtonSend")
}

void CStreamFactoryDlg::ReadFromStream()
{
TRY_CATCH

	char buf[MAX_PATH];
	bool rec = m_stream->HasInData();// > 0;
	int count = 0;
	char* pbuf = buf;

	if ( rec )
	{
		strcpy_s( buf, MAX_PATH, "Receiving ->" );
		m_ReceiveList.AddString( buf );
		FillMemory( buf, MAX_PATH, 0 );
	}

	while ( m_stream->HasInData() )
	{
		try
		{
			m_stream->Receive( pbuf, 1 );
		}
		catch(CStreamException &e)
		{
			MLog_Exception(CExceptionBase(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__))));
			OnBnClickedDisconnectButton(); ///TODO: send message
			return;
		}
		pbuf++;
		count++;
		if ( MAX_PATH - 1 == count )
		{
			m_ReceiveList.AddString( buf );
			FillMemory( buf, MAX_PATH, 0 );
			pbuf = buf;
			count = 0;
		}
	}

	if ( rec )
	{
		if ( count )
			m_ReceiveList.AddString( buf );
		FillMemory( buf, MAX_PATH, 0 );
		strcpy_s( buf, MAX_PATH, "->Received" );
		m_ReceiveList.AddString( buf );
	}

CATCH_LOG("CStreamFactoryDlg::ReadFromStream")
}

void CStreamFactoryDlg::OnTimer( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{
TRY_CATCH

	m_this->ReadFromStream();

CATCH_LOG("CStreamFactoryDlg::OnTimer")
}
void CStreamFactoryDlg::OnBnClickedButtonClear()
{
TRY_CATCH
	m_SendList.ResetContent();
	m_ReceiveList.ResetContent();
	m_SendText.SetWindowText( "" );
CATCH_LOG("CStreamFactoryDlg::OnBnClickedButtonClear")
}
