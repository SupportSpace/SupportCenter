// CRelayedStreamDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NetworkLayerTest.h"
#include "CRelayedStreamDlg.h"
#include ".\crelayedstreamdlg.h"
#include <AidLib/Logging/cLog.h>
#include <NWL/Streaming/CNetworkLayer.h>
#include <AidLib/CSingleton/CSingleton.h>


// CRelayedStreamDlg dialog
CRelayedStreamDlg *m_this;
IMPLEMENT_DYNAMIC(CRelayedStreamDlg, CDialog)
CRelayedStreamDlg::CRelayedStreamDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRelayedStreamDlg::IDD, pParent)
	, m_relayPortEdit(0)
	, m_timeOut(0)
{
}

CRelayedStreamDlg::~CRelayedStreamDlg()
{
}

void CRelayedStreamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_NAT_SRV_ADDRESS, m_relayAddressEdit);
	DDX_Text(pDX, IDC_EDIT_NAT_SRV_PORT, m_relayPortEdit);
	DDX_Text(pDX, IDC_EDIT_NAT_TIMEOUT, m_timeOut);
	DDX_Control(pDX, IDC_EDIT_NAT_SRV_USER, m_serverUserIDEdit);
	DDX_Control(pDX, IDC_EDIT_NAT_SRV_PASS, m_servPassEdit);
	DDX_Control(pDX, IDC_EDIT_NAT_SRV_CONNECT, m_connIDEdit);
	DDX_Control(pDX, IDC_EDIT_NAT_SRV_PEER, m_sourcePeerIdEdit);
	DDX_Control(pDX, IDC_EDIT_NAT_SRV_REMOTE_PEER, m_destPeerIdEdit);
	DDX_Control(pDX, IDC_EDIT_NAT_USER_ID, m_userIDEdit);
	DDX_Control(pDX, IDC_EDIT_NAT_USER_KEY, m_PassEdit);
	DDX_Control(pDX, IDC_CHECK_NAT_MASTER, m_MasterRoleCheck);
	DDX_Control(pDX, IDC_BUTTON_NAT_DISCONNECT, m_disconnectBtn);
	DDX_Control(pDX, IDC_BUTTON_NAT_CONNECT, m_connectBtn);
	DDX_Control(pDX, IDC_EDIT_SEND_TEXT, m_SendText);
	DDX_Control(pDX, IDC_LIST_SEND, m_SendList);
	DDX_Control(pDX, IDC_LIST_RECEIVE, m_ReceiveList);
	DDX_Control(pDX, IDC_CHECK_ADD_BRAKLINE, m_AddBreakLine);
	DDX_Control(pDX, IDC_BUTTON_SEND, m_sendBtn);
}

BOOL CRelayedStreamDlg::OnInitDialog()
{
TRY_CATCH
	BOOL res = CDialog::OnInitDialog();
	/// Setting def values
	m_relayAddressEdit.SetWindowText(NWL_INSTANCE.GetRelayHost().c_str());
	m_relayPortEdit = NWL_INSTANCE.GetRelayTCPPort();
	m_serverUserIDEdit.SetWindowText("TestUser");
	m_servPassEdit.SetWindowText("TestUser");
	m_PassEdit.SetWindowText("TestUser");
	m_userIDEdit.SetWindowText("TestUser");
	m_connIDEdit.SetWindowText(GetGUID().c_str());
	UpdateData(FALSE);
	return res;
CATCH_LOG("CRelayedStreamDlg::OnInitDialog")
	return FALSE;
}

BEGIN_MESSAGE_MAP(CRelayedStreamDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_NAT_CONNECT, OnBnClickedButtonNatConnect)
	ON_BN_CLICKED(IDC_BUTTON_NAT_DISCONNECT, OnBnClickedButtonNatDisconnect)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_BUTTON_SEND, OnBnClickedButtonSend)
END_MESSAGE_MAP()


// CRelayedStreamDlg message handlers

void CRelayedStreamDlg::OnBnClickedButtonNatConnect()
{
TRY_CATCH
	TCHAR buf[MAX_PATH];
	UpdateData();
	bool masterRole = (m_MasterRoleCheck.GetCheck() != 0);
	m_stream.reset(new CRelayedNetworkStream<>);
	m_stream->SetIsMaster(masterRole);
	/// Network layer settings
	m_relayAddressEdit.GetWindowText(buf,MAX_PATH);
	NWL_INSTANCE.SetRelayHost(buf);
	NWL_INSTANCE.SetRelayTCPPort(m_relayPortEdit);
	/// Relay connection
	tstring servUserId, servPasswd;
	m_serverUserIDEdit.GetWindowText(buf,MAX_PATH);
	servUserId = buf;
	m_servPassEdit.GetWindowText(buf,MAX_PATH);
	servPasswd = buf;
	m_stream->SetRelayServer(NWL_INSTANCE.GetRelayHost(),		// Server Host
							NWL_INSTANCE.GetRelayTCPPort(),	// Server Port
							servUserId,										// Server User
							servPasswd										// Server Pass
							);
							
	tstring connId,srcPeerId,destPeerId;
	m_connIDEdit.GetWindowText(buf,MAX_PATH);
	connId = buf;
	m_sourcePeerIdEdit.GetWindowText(buf,MAX_PATH);
	srcPeerId = buf;
	m_destPeerIdEdit.GetWindowText(buf,MAX_PATH);
	destPeerId = buf;
	m_stream->SetConnectionId(connId, srcPeerId, destPeerId);
	/// TLS settings
	STLSCredentials tlsCredits;
	m_userIDEdit.GetWindowText(buf,MAX_PATH);
	tlsCredits.UserID = buf;
	m_PassEdit.GetWindowText(buf,MAX_PATH);
	tlsCredits.Key = buf;
	m_stream->SetCredentials(tlsCredits);
	m_stream->SetConnectTimeout(30000);
	/// Proper connecting 
	m_stream->Connect();
	m_disconnectBtn.EnableWindow(true);
	m_connectBtn.EnableWindow(false);
	cMsgBoxLog().Add(_MESSAGE_,_T("Connected"));
	m_this = this;
	timer = SetTimer( 1, 100, &CRelayedStreamDlg::OnTimer );
	m_sendBtn.EnableWindow(true);
CATCH_LOG("CRelayedStreamDlg::OnBnClickedButtonNatConnect")
}

void CRelayedStreamDlg::OnBnClickedButtonNatDisconnect()
{
TRY_CATCH
	KillTimer( 1 );
	if (!m_stream.get()) throw MCException("m_stream.get() == NULL");
	m_stream->Disconnect();
	m_disconnectBtn.EnableWindow(false);
	m_connectBtn.EnableWindow(true);
	m_sendBtn.EnableWindow(false);
CATCH_LOG("CRelayedStreamDlg::OnBnClickedButtonNatDisconnect")
}

void CRelayedStreamDlg::OnBnClickedButtonClear()
{
TRY_CATCH
	m_SendList.ResetContent();
	m_ReceiveList.ResetContent();
	m_SendText.SetWindowText( "" );
CATCH_LOG("CRelayedStreamDlg::OnBnClickedButtonClear")
}

void CRelayedStreamDlg::OnBnClickedButtonSend()
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

//	static_cast<CAbstractStream*>(*m_stream.get())->Send( buf, len );
	m_stream->Send( buf, len );

	FillMemory( buf, MAX_PATH, 0 );
	strcpy_s( buf, MAX_PATH, "->Sent" );
	m_SendList.AddString( buf );

	m_SendText.SetWindowText( "" );

CATCH_LOG("CRelayedStreamDlg::OnBnClickedButtonSend")
}

void CRelayedStreamDlg::ReadFromStream()
{
TRY_CATCH

	char buf[MAX_PATH];
//	bool rec = static_cast<CAbstractStream*>(*m_stream.get())->HasInData();// > 0;
	bool rec = m_stream->HasInData();// > 0;
	int count = 0;
	char* pbuf = buf;

	if ( rec )
	{
		strcpy_s( buf, MAX_PATH, "Receiving ->" );
		m_ReceiveList.AddString( buf );
		FillMemory( buf, MAX_PATH, 0 );
	}

//	while ( static_cast<CAbstractStream*>(*m_stream.get())->HasInData() )
	while ( m_stream->HasInData() )
	{
		try
		{
//			static_cast<CAbstractStream*>(*m_stream.get())->Receive( pbuf, 1 );
			m_stream->Receive( pbuf, 1 );
		}
		catch(CStreamException &e)
		{
			MLog_Exception(CExceptionBase(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__))));
			OnBnClickedButtonNatDisconnect(); ///TODO: send message
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

CATCH_LOG("CRelayedStreamDlg::ReadFromStream")
}

void CRelayedStreamDlg::OnTimer( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{
TRY_CATCH

	m_this->ReadFromStream();

CATCH_LOG("CRelayedStreamDlg::OnTimer")
}

