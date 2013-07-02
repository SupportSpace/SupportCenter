// NetworkLayerTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NetworkLayerTest.h"
#include "NetworkLayerTestDlg.h"
#include ".\networklayertestdlg.h"
#include <NWL/Streaming/CStreamException.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNetworkLayerTestDlg dialog

CNetworkLayerTestDlg* m_this;

CNetworkLayerTestDlg::CNetworkLayerTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNetworkLayerTestDlg::IDD, pParent)
	, m_Cipher(2)
	, m_Compression(2)
	, m_KeyExchange(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNetworkLayerTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_REMOTE_ADDR, m_RemoteAddress);
	DDX_Control(pDX, IDC_EDIT_REMOTE_PORT, m_RemotePort);
	DDX_Control(pDX, IDC_EDIT_LOCAL_PORT, m_LocalPort);
	DDX_Control(pDX, IDC_CHECK_USE_PROXY, m_UseProxy);
	DDX_Control(pDX, IDC_EDIT_PROXY_ADDR, m_ProxyAddress);
	DDX_Control(pDX, IDC_EDIT_PROXY_PORT, m_ProxyPort);
	DDX_Control(pDX, IDC_EDIT_TIMEOUT, m_Timeout);
	DDX_Control(pDX, IDC_EDIT_USER_ID, m_UserID);
	DDX_Control(pDX, IDC_EDIT_USER_KEY, m_UserKey);
	DDX_Radio(pDX, IDC_RADIO_CIPH_NONE, m_Cipher);
	DDX_Radio(pDX, IDC_RADIO_COMP_NONE, m_Compression);
	DDX_Radio(pDX, IDC_RADIO_KX_PSK, m_KeyExchange);
	DDX_Control(pDX, IDC_EDIT_SEND_TEXT, m_SendText);
	DDX_Control(pDX, IDC_BUTTON_SEND, m_SendButton);
	DDX_Control(pDX, IDC_BUTTON_CLEAR, m_ClearButton);
	DDX_Control(pDX, IDC_LIST_SEND, m_SendList);
	DDX_Control(pDX, IDC_LIST_RECEIVE, m_ReceiveList);
	DDX_Control(pDX, IDC_BUTTON_CONNECT, m_ConnectButton);
	DDX_Control(pDX, IDC_BUTTON_DISCONNECT, m_DisconnectButton);
	DDX_Control(pDX, IDC_CHECK_ADD_BRAKLINE, m_AddBreakLine);
}

BEGIN_MESSAGE_MAP(CNetworkLayerTestDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, OnBnClickedButtonDisconnect)
	ON_BN_CLICKED(IDC_BUTTON_SEND, OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_RADIO_CIPH_NONE, OnBnClickedRadioCiphNone)
	ON_BN_CLICKED(IDC_RADIO_CIPH_AES_128, OnBnClickedRadioCiphAes128)
	ON_BN_CLICKED(IDC_RADIO_CIPH_AES_256, OnBnClickedRadioCiphAes256)
	ON_BN_CLICKED(IDC_RADIO_3DES, OnBnClickedRadio3des)
	ON_BN_CLICKED(IDC_RADIO_CIPH_RC4_128, OnBnClickedRadioCiphRc4128)
	ON_BN_CLICKED(IDC_RADIO_COMP_NONE, OnBnClickedRadioCompNone)
	ON_BN_CLICKED(IDC_RADIO_COMP_ZLIB, OnBnClickedRadioCompZlib)
	ON_BN_CLICKED(IDC_RADIO_COMP_LZO, OnBnClickedRadioCompLzo)
	ON_BN_CLICKED(IDC_RADIO_KX_PSK, OnBnClickedRadioKxPsk)
	ON_BN_CLICKED(IDC_RADIO_KX_DHE_PSK, OnBnClickedRadioKxDhePsk)
END_MESSAGE_MAP()


// CNetworkLayerTestDlg message handlers

BOOL CNetworkLayerTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	m_RemoteAddress.SetWindowText( "192.168.0.5" );
	m_RemotePort.SetWindowText( "6000" );
	m_LocalPort.SetWindowText( "5000" );
	m_Timeout.SetWindowText( "30000" );
	m_ProxyAddress.SetWindowText( "192.168.0.1" );
	m_ProxyPort.SetWindowText( "3128" );
//	m_UserID.SetWindowText( "diman" );
//	m_UserKey.SetWindowText( "4ff85ddbacbb8e3134f5a485b1f84b62" );
	m_UserID.SetWindowText( "UserName" );
	m_UserKey.SetWindowText( "ThisIsTopSecretPassword" );

	m_SendList.SetHorizontalExtent( 1000 );
	m_ReceiveList.SetHorizontalExtent( 1000 );

	//Log.RegisterLog(new cFileLog(),true,cLog::_EXCEPTION);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNetworkLayerTestDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNetworkLayerTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CNetworkLayerTestDlg::ReadFromStream()
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

void  CNetworkLayerTestDlg::OnTimer( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{
TRY_CATCH

	m_this->ReadFromStream();

CATCH_LOG("CNetworkLayerTestDlg::OnTimer")
}

void CNetworkLayerTestDlg::DoConnect()
{
TRY_CATCH

	char buf[MAX_PATH];

	m_RemotePort.GetWindowText( buf, MAX_PATH );
	unsigned int remote_port = atoi( buf );

	m_LocalPort.GetWindowText( buf, MAX_PATH );
	unsigned int local_port = atoi( buf );

	m_Timeout.GetWindowText( buf, MAX_PATH );
	unsigned int timeout = atoi( buf );

	m_RemoteAddress.GetWindowText( buf, MAX_PATH );
	tstring remote_addr( buf );

//	Stream.SetConnectedEvent( boost::bind( &CNetworkLayerTestDlg::OnConnected, this, _1 ) );
//	Stream.SetDisconnectedEvent( boost::bind( &CNetworkLayerTestDlg::OnDisconnected, this, _1 ) );
//	Stream.SetConnectErrorEvent( boost::bind( &CNetworkLayerTestDlg::OnConnectError, this, _1, _2 ) );
	
	Stream.SetConnectTimeout( timeout );

//***************************************************************************

	m_UserID.GetWindowText( buf, MAX_PATH );
	tstring user_id( buf );

	m_UserKey.GetWindowText( buf, MAX_PATH );
	tstring user_key( buf );

	Stream.GetCredentials().UserID = user_id;
	Stream.GetCredentials().Key = user_key;

//***************************************************************************
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
	
	switch ( m_Compression )
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
	
	switch ( m_KeyExchange )
	{
	case 1:
		Stream.GetSuite().KeyExchange = KX_DHE_PSK;
		break;

	default:
		Stream.GetSuite().KeyExchange = KX_PSK;
	}

	Stream.GetSuite().PrimeBits = PB_1024;

//***************************************************************************
	m_ProxyPort.GetWindowText( buf, MAX_PATH );
	unsigned int proxy_port = atoi( buf );

	m_ProxyAddress.GetWindowText( buf, MAX_PATH );
	tstring proxy_addr( buf );

	bool use_proxy = ( BST_CHECKED == m_UseProxy.GetCheck() );

	Stream.SetConnectThroughProxy( use_proxy );
	Stream.GetProxySettings().ProxyURL = proxy_addr;
	Stream.GetProxySettings().ProxyPort = proxy_port;
//***************************************************************************

	m_SendList.AddString( "Connecting ..." );

	try
	{
		Stream.SetLocalAddr( local_port );
		Stream.SetRemoteAddr( remote_addr, remote_port );
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

CATCH_LOG("CNetworkLayerTestDlg::DoConnect")
}

void CNetworkLayerTestDlg::DoDisconnect()
{
TRY_CATCH

	m_SendList.AddString( "Disconnecting ..." );
	Stream.Disconnect();
	OnDisconnected(NULL);

CATCH_LOG("CNetworkLayerTestDlg::DoDisconnect")
}

void CNetworkLayerTestDlg::OnConnected(void*)
{
TRY_CATCH

	m_SendList.AddString( "** CONNECTED **" );

	m_this = this;
	timer = SetTimer( 1, 100, OnTimer );

	m_ConnectButton.EnableWindow( false );
	m_DisconnectButton.EnableWindow( true );
	m_SendButton.EnableWindow( true );

CATCH_LOG("CNetworkLayerTestDlg::OnConnected")
}

void CNetworkLayerTestDlg::OnDisconnected(void*)
{
TRY_CATCH

	KillTimer( 1 );
	m_SendList.AddString( "** DISCONNECTED **" );
	m_ConnectButton.EnableWindow( true );
	m_DisconnectButton.EnableWindow( false );
	m_SendButton.EnableWindow( false );

CATCH_LOG("CNetworkLayerTestDlg::OnDisconnected")
}

void CNetworkLayerTestDlg::OnConnectError(void*, EConnectErrorReason reason)
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

	default:
		m_SendList.AddString( "** CONNECT FAILED: Unknown Error **" );

	}

	m_ConnectButton.EnableWindow( true );
	m_DisconnectButton.EnableWindow( false );
	m_SendButton.EnableWindow( false );

CATCH_LOG("CNetworkLayerTestDlg::OnConnectError")
}

void CNetworkLayerTestDlg::OnBnClickedButtonClear()
{
TRY_CATCH

	m_SendList.ResetContent();
	m_ReceiveList.ResetContent();
	m_SendText.SetWindowText( "" );

CATCH_LOG("CNetworkLayerTestDlg::OnBnClickedButtonClear")
}

void CNetworkLayerTestDlg::OnBnClickedButtonConnect()
{
TRY_CATCH

	m_ConnectButton.EnableWindow( false );

	DoConnect();

CATCH_LOG("CNetworkLayerTestDlg::OnBnClickedButtonConnect")
}

void CNetworkLayerTestDlg::OnBnClickedButtonDisconnect()
{
TRY_CATCH

	DoDisconnect();

CATCH_LOG("CNetworkLayerTestDlg::OnBnClickedButtonDisconnect")
}

void CNetworkLayerTestDlg::OnBnClickedButtonSend()
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

CATCH_LOG("CNetworkLayerTestDlg::OnBnClickedButtonSend")
}

void CNetworkLayerTestDlg::OnBnClickedRadioCiphNone()
{
	m_Cipher = 0;
}

void CNetworkLayerTestDlg::OnBnClickedRadioCiphAes128()
{
	m_Cipher = 1;
}

void CNetworkLayerTestDlg::OnBnClickedRadioCiphAes256()
{
	m_Cipher = 2;
}

void CNetworkLayerTestDlg::OnBnClickedRadio3des()
{
	m_Cipher = 3;
}

void CNetworkLayerTestDlg::OnBnClickedRadioCiphRc4128()
{
	m_Cipher = 4;
}

void CNetworkLayerTestDlg::OnBnClickedRadioCompNone()
{
	m_Compression = 0;
}

void CNetworkLayerTestDlg::OnBnClickedRadioCompZlib()
{
	m_Compression = 1;
}

void CNetworkLayerTestDlg::OnBnClickedRadioCompLzo()
{
	m_Compression = 2;
}

void CNetworkLayerTestDlg::OnBnClickedRadioKxPsk()
{
	m_KeyExchange = 0;
}

void CNetworkLayerTestDlg::OnBnClickedRadioKxDhePsk()
{
	m_KeyExchange = 1;
}

