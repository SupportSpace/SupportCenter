// CNATTraversalDialog.cpp : implementation file
//

#include "stdafx.h"
#include "NetworkLayerTest.h"
#include "CNATTraversalDialog.h"
#include <NWL/Streaming/CStreamException.h>
#include <AidLib/Logging/cLog.h>
#include ".\cnattraversaldialog.h"
#include <NWL/Streaming/CNetworkLayer.h>


CNATTraversalDialog* m_this_nat;


// CNATTraversalDialog dialog

IMPLEMENT_DYNAMIC(CNATTraversalDialog, CDialog)
CNATTraversalDialog::CNATTraversalDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CNATTraversalDialog::IDD, pParent)
	, m_Cipher(2)
	, m_nCompression(1)
	, m_KeyExchange(0)
{
}

CNATTraversalDialog::~CNATTraversalDialog()
{
}

BOOL CNATTraversalDialog::OnInitDialog()
{
	CDialog::OnInitDialog();


//	m_SrvAddress.SetWindowText( "192.168.0.133" );
	m_SrvAddress.SetWindowText( "192.168.0.66" );
	m_SrvPort.SetWindowText( "5902" );
//	m_SrvUserId.SetWindowText( "diman" );
//	m_SrvPassword.SetWindowText( "secret" );
	m_SrvSessionId.SetWindowText("1234567");
	m_SrvUserId.SetWindowText( "TestUser" );
	m_SrvPassword.SetWindowText( "A7B3F3CA-0096-4d02-8936-31B2392F973F" );
	m_ConnectId.SetWindowText( "conn_1" );
	m_LocalPeerId.SetWindowText( "peer_1" );
	m_RemotePeerId.SetWindowText( "peer_2" );
	m_Timeout.SetWindowText( "30000" );
	m_UserId.SetWindowText( "PeerUserId" );
	m_Key.SetWindowText( "ThisIsSecretKey" );
	m_AuthDelay.SetWindowText( "5000" );
	m_AuthMax.SetWindowText( "3" );
	m_BindDelay.SetWindowText( "5000" );
	m_BindMax.SetWindowText( "3" );
	m_ProbeDelay.SetWindowText( "5000" );
	m_ProbeMax.SetWindowText( "3" );
	m_PortRange.SetWindowText( "3" );
	m_SendText.SetWindowText( "" );
	m_MasterMode.SetCheck( BST_CHECKED );


	m_SendList.SetHorizontalExtent( 1000 );
	m_ReceiveList.SetHorizontalExtent( 1000 );

	//Log.RegisterLog(new cFileLog(),true,cLog::_EXCEPTION);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CNATTraversalDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_NAT_SRV_ADDRESS, m_SrvAddress);
	DDX_Control(pDX, IDC_EDIT_NAT_SRV_PORT, m_SrvPort);
	DDX_Control(pDX, IDC_EDIT_NAT_SRV_USER, m_SrvUserId);
	DDX_Control(pDX, IDC_EDIT_NAT_SRV_SESSION_ID, m_SrvSessionId);
	DDX_Control(pDX, IDC_EDIT_NAT_SRV_PASS, m_SrvPassword);
	DDX_Control(pDX, IDC_EDIT_NAT_SRV_CONNECT, m_ConnectId);
	DDX_Control(pDX, IDC_EDIT_NAT_SRV_PEER, m_LocalPeerId);
	DDX_Control(pDX, IDC_EDIT_NAT_TIMEOUT, m_Timeout);
	DDX_Control(pDX, IDC_EDIT_NAT_USER_ID, m_UserId);
	DDX_Control(pDX, IDC_EDIT_NAT_USER_KEY, m_Key);
	DDX_Control(pDX, IDC_EDIT_NAT_AUTH_DELAY, m_AuthDelay);
	DDX_Control(pDX, IDC_EDIT_NAT_AUTH_MAX, m_AuthMax);
	DDX_Control(pDX, IDC_EDIT_NAT_BIND_DELAY, m_BindDelay);
	DDX_Control(pDX, IDC_EDIT_NAT_BIND_MAX, m_BindMax);
	DDX_Control(pDX, IDC_EDIT_NAT_PROBE_DELAY, m_ProbeDelay);
	DDX_Control(pDX, IDC_EDIT_NAT_PROBE_MAX, m_ProbeMax);
	DDX_Control(pDX, IDC_EDIT_NAT_PORT_RANGE, m_PortRange);
	DDX_Control(pDX, IDC_EDIT_NAT_SEND_TEXT, m_SendText);
	DDX_Control(pDX, IDC_LIST_NAT_SEND, m_SendList);
	DDX_Control(pDX, IDC_LIST_NAT_RECEIVE, m_ReceiveList);
	DDX_Radio(pDX, IDC_RADIO_NAT_CIPH_NONE, m_Cipher);
	DDX_Radio(pDX, IDC_RADIO_NAT_COMP_NONE, m_nCompression);
	DDX_Radio(pDX, IDC_RADIO_NAT_KX_PSK, m_KeyExchange);
	DDX_Control(pDX, IDC_BUTTON_NAT_CONNECT, m_ConnectButton);
	DDX_Control(pDX, IDC_BUTTON_NAT_DISCONNECT, m_DisconnectButton);
	DDX_Control(pDX, IDC_BUTTON_NAT_SEND, m_SendButton);
	DDX_Control(pDX, IDC_CHECK_NAT_ADD_BRAKLINE, m_AddBreakLine);
	DDX_Control(pDX, IDC_CHECK_NAT_MASTER, m_MasterMode);
	DDX_Control(pDX, IDC_EDIT_NAT_SRV_REMOTE_PEER, m_RemotePeerId);
}


BEGIN_MESSAGE_MAP(CNATTraversalDialog, CDialog)
	ON_BN_CLICKED(IDC_RADIO_NAT_CIPH_NONE, OnBnClickedRadioNatCiphNone)
	ON_BN_CLICKED(IDC_RADIO_NAT_CIPH_AES_128, OnBnClickedRadioNatCiphAes128)
	ON_BN_CLICKED(IDC_RADIO_NAT_CIPH_AES_256, OnBnClickedRadioNatCiphAes256)
	ON_BN_CLICKED(IDC_RADIO_NAT_3DES, OnBnClickedRadioNat3des)
	ON_BN_CLICKED(IDC_RADIO_NAT_CIPH_RC4_128, OnBnClickedRadioNatCiphRc4128)
	ON_BN_CLICKED(IDC_RADIO_NAT_COMP_NONE, OnBnClickedRadioNatCompNone)
	ON_BN_CLICKED(IDC_RADIO_NAT_COMP_ZLIB, OnBnClickedRadioNatCompZlib)
	ON_BN_CLICKED(IDC_RADIO_NAT_COMP_LZO, OnBnClickedRadioNatCompLzo)
	ON_BN_CLICKED(IDC_RADIO_NAT_KX_PSK, OnBnClickedRadioNatKxPsk)
	ON_BN_CLICKED(IDC_RADIO_NAT_KX_DHE_PSK, OnBnClickedRadioNatKxDhePsk)
	ON_BN_CLICKED(IDC_BUTTON_NAT_CLEAR, OnBnClickedButtonNatClear)
	ON_BN_CLICKED(IDC_BUTTON_NAT_CONNECT, OnBnClickedButtonNatConnect)
	ON_BN_CLICKED(IDC_BUTTON_NAT_DISCONNECT, OnBnClickedButtonNatDisconnect)
	ON_BN_CLICKED(IDC_BUTTON_NAT_SEND, OnBnClickedButtonNatSend)
END_MESSAGE_MAP()


// CNATTraversalDialog message handlers

void CNATTraversalDialog::OnBnClickedRadioNatCiphNone()
{
	m_Cipher = 0;
}

void CNATTraversalDialog::OnBnClickedRadioNatCiphAes128()
{
	m_Cipher = 1;
}

void CNATTraversalDialog::OnBnClickedRadioNatCiphAes256()
{
	m_Cipher = 2;
}

void CNATTraversalDialog::OnBnClickedRadioNat3des()
{
	m_Cipher = 3;
}

void CNATTraversalDialog::OnBnClickedRadioNatCiphRc4128()
{
	m_Cipher = 4;
}

void CNATTraversalDialog::OnBnClickedRadioNatCompNone()
{
	m_nCompression = 0;
}

void CNATTraversalDialog::OnBnClickedRadioNatCompZlib()
{
	m_nCompression = 1;
}

void CNATTraversalDialog::OnBnClickedRadioNatCompLzo()
{
	m_nCompression = 2;
}

void CNATTraversalDialog::OnBnClickedRadioNatKxPsk()
{
	m_KeyExchange = 0;
}

void CNATTraversalDialog::OnBnClickedRadioNatKxDhePsk()
{
	m_KeyExchange = 1;
}

void CNATTraversalDialog::DoConnect()
{
TRY_CATCH

	char buf[MAX_PATH];


	Stream.SetIsMaster( BST_CHECKED == m_MasterMode.GetCheck() );
//--->


	m_SrvAddress.GetWindowText( buf, MAX_PATH );
	tstring srv_address( buf );

	m_SrvPort.GetWindowText( buf, MAX_PATH );
	unsigned int srv_port = atoi( buf );

	m_SrvUserId.GetWindowText( buf, MAX_PATH );
	tstring srv_userid( buf );

	m_SrvPassword.GetWindowText( buf, MAX_PATH );
	tstring srv_passwd( buf );


	Stream.SetRelayServer( srv_address, srv_port, srv_userid, srv_passwd );

	NWL_INSTANCE.SetRelayHost(srv_address);
	NWL_INSTANCE.SetRelayUDPPort(srv_port);
	NWL_INSTANCE.SetRelayPasswd(srv_passwd);
//--->


	m_ConnectId.GetWindowText( buf, MAX_PATH );
	tstring connect_id( buf );

	m_LocalPeerId.GetWindowText( buf, MAX_PATH );
	tstring local_peer_id( buf );

	m_RemotePeerId.GetWindowText( buf, MAX_PATH );
	tstring remote_peer_id( buf );


	Stream.SetConnectionId( connect_id, local_peer_id, remote_peer_id );
//--->


	m_Timeout.GetWindowText( buf, MAX_PATH );
	unsigned int timeout = atoi( buf );


	Stream.SetConnectTimeout( timeout );
//--->


	m_UserId.GetWindowText( buf, MAX_PATH );
	tstring user_id( buf );

	m_Key.GetWindowText( buf, MAX_PATH );
	tstring user_key( buf );


	Stream.GetCredentials().UserID = user_id;
	Stream.GetCredentials().Key = user_key;
//--->


	m_AuthDelay.GetWindowText( buf, MAX_PATH );
	unsigned int auth_delay = atoi( buf );

	m_AuthMax.GetWindowText( buf, MAX_PATH );
	unsigned int auth_max = atoi( buf );

	m_BindDelay.GetWindowText( buf, MAX_PATH );
	unsigned int bind_delay = atoi( buf );

	m_BindMax.GetWindowText( buf, MAX_PATH );
	unsigned int bind_max = atoi( buf );

	m_ProbeDelay.GetWindowText( buf, MAX_PATH );
	unsigned int probe_delay = atoi( buf );

	m_ProbeMax.GetWindowText( buf, MAX_PATH );
	unsigned int probe_max = atoi( buf );

	m_PortRange.GetWindowText( buf, MAX_PATH );
	unsigned int port_range = atoi( buf );


	Stream.SetAuthRetry( auth_delay, auth_max );
	Stream.SetBindRetry( bind_delay, bind_max );
	Stream.SetProbeRetry( probe_delay, probe_max );
	Stream.SetProbePortRange( port_range );

	NWL_INSTANCE.SetAuthRetry(auth_delay, auth_max);
//--->


	switch ( m_Cipher )
	{
	case 1:
		Stream.GetSuite().Cipher = CPH_AES_128;
		break;

	case 2:
		Stream.GetSuite().Cipher = CPH_AES_256;
		break;

	case 3:
		Stream.GetSuite().Cipher = CPH_3DES;
		break;

	case 4:
		Stream.GetSuite().Cipher = CPH_RC4_128;
		break;

	default:
		Stream.GetSuite().Cipher = CPH_NULL;
	}
//--->


	switch ( m_nCompression )
	{
	case 1:
		Stream.GetSuite().Compression = PRS_ZLIB;
		break;

	case 2:
		Stream.GetSuite().Compression = PRS_LZO;
		break;

	default:
		Stream.GetSuite().Compression = PRS_NULL;
	}
//--->


	switch ( m_KeyExchange )
	{
	case 1:
		Stream.GetSuite().KeyExchange = KX_DHE_PSK;
		break;

	default:
		Stream.GetSuite().KeyExchange = KX_PSK;
	}
//--->


	Stream.GetSuite().PrimeBits = PB_1024;
//--->





	m_SendList.AddString( "Connecting ..." );

	try
	{
		Stream.Connect();
		OnConnected(NULL);
	}
	catch(CStreamException ex)
	{
		m_SendList.AddString( ex.what.c_str() );
		m_ConnectButton.EnableWindow( true );
		m_DisconnectButton.EnableWindow( false );
		m_SendButton.EnableWindow( false );
	}

CATCH_LOG("CNATTraversalDialog::DoConnect")
}

void CNATTraversalDialog::DoDisconnect()
{
TRY_CATCH

	m_SendList.AddString( "Disconnecting ..." );
	Stream.Disconnect();
	OnDisconnected(NULL);

CATCH_LOG("CNATTraversalDialog::DoDisconnect")
}

void CNATTraversalDialog::OnConnected(void*)
{
TRY_CATCH

	m_SendList.AddString( "** CONNECTED **" );

	m_this_nat = this;
	timer = SetTimer( 2, 100, OnTimer );

	m_ConnectButton.EnableWindow( false );
	m_DisconnectButton.EnableWindow( true );
	m_SendButton.EnableWindow( true );

CATCH_LOG("CNATTraversalDialog::::OnConnected")
}

void CNATTraversalDialog::OnDisconnected(void*)
{
TRY_CATCH

	KillTimer( 2 );
	m_SendList.AddString( "** DISCONNECTED **" );
	m_ConnectButton.EnableWindow( true );
	m_DisconnectButton.EnableWindow( false );
	m_SendButton.EnableWindow( false );

CATCH_LOG("CNATTraversalDialog::OnDisconnected")
}

void CNATTraversalDialog::OnConnectError(void*, EConnectErrorReason reason)
{
TRY_CATCH

	switch( reason )
	{
	case cerWinError:
		m_SendList.AddString( "** CONNECT FAILED: WinError **" );
		break;

	case cerAuthFailed:
		m_SendList.AddString( "** AUTHENTICATION FAILED **" );
		break;

	case cerTimeout:
		m_SendList.AddString( "** CONNECT FAILED: Timeout expired **" );
		break;

	case cerTriesPassed:
		m_SendList.AddString( "** CONNECT FAILED: All connection tries are passed **" );
		break;

	default:
		m_SendList.AddString( "** CONNECT FAILED: Unknown Error **" );

	}

	m_ConnectButton.EnableWindow( true );
	m_DisconnectButton.EnableWindow( false );
	m_SendButton.EnableWindow( false );

CATCH_LOG("CNATTraversalDialog::OnConnectError")
}

void  CNATTraversalDialog::OnTimer( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{
TRY_CATCH

	m_this_nat->ReadFromStream();

CATCH_LOG("CNATTraversalDialog::OnTimer")
}



void CNATTraversalDialog::OnBnClickedButtonNatClear()
{
TRY_CATCH

	m_SendList.ResetContent();
	m_ReceiveList.ResetContent();
	m_SendText.SetWindowText( "" );

CATCH_LOG("CNATTraversalDialog::OnBnClickedButtonClear")
}

void CNATTraversalDialog::OnBnClickedButtonNatConnect()
{
TRY_CATCH

	m_ConnectButton.EnableWindow( false );

	DoConnect();

CATCH_LOG("CNATTraversalDialog::OnBnClickedButtonConnect")
}

void CNATTraversalDialog::OnBnClickedButtonNatDisconnect()
{
TRY_CATCH

	DoDisconnect();

CATCH_LOG("CNATTraversalDialog::OnBnClickedButtonDisconnect")
}

void CNATTraversalDialog::OnBnClickedButtonNatSend()
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

	Stream.Send( buf, len );

	FillMemory( buf, MAX_PATH, 0 );
	strcpy_s( buf, MAX_PATH, "->Sent" );
	m_SendList.AddString( buf );

	m_SendText.SetWindowText( "" );

CATCH_LOG("CNATTraversalDialog::OnBnClickedButtonSend")
}

void CNATTraversalDialog::ReadFromStream()
{
TRY_CATCH

	char buf[MAX_PATH];
	bool rec = Stream.HasInData();// > 0;
	int count = 0;
	char* pbuf = buf;

	if ( rec )
	{
		strcpy_s( buf, MAX_PATH, "Receiving ->" );
		m_ReceiveList.AddString( buf );
		FillMemory( buf, MAX_PATH, 0 );
	}

	while ( Stream.HasInData() )
	{
		try
		{
			Stream.Receive( pbuf, 1 );
		}
		catch(CStreamException &e)
		{
			MLog_Exception(CExceptionBase(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__))));
			OnDisconnected(NULL);
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

CATCH_LOG("CNetworkLayerTestDlg::ReadFromStream")
}

