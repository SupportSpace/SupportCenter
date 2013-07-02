// RCTest2Dlg.cpp : implementation file
//
#include "stdafx.h"
#include "RCTest2.h"
#include "RCTest2Dlg.h"
#include "CAddClientDlg.h"
#include "CAddViewerDialog.h"
#include "CViewerDlg.h"
#include "CNetSettingsDlg.h"
#include "rctest2dlg.h"
#include <RCEngine/Streaming/COutStreamGZipped.h>
#include <NWL/Streaming/CSocketSystem.h>
#include <AidLib/CSingleton/CSingleton.h>
#include <NWL/TLS/CTLSSystem.h>
#include <AidLib/CThread/CThreadLS.h>
#include <NetLog/CNetworkLog.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <atlbase.h>
#include <objbase.h>

#include "CInputController.h"
#include "CFakedPointer.h"

CComModule _Module;

#pragma comment(lib,"NetLog.lib")

CSocketSystem sockSystem;
CTLSSystem tlsSystem;

// CRCTest2Dlg dialog
CRCTest2Dlg::CRCTest2Dlg(CWnd* pParent /*=NULL*/)
	:	CDialog(CRCTest2Dlg::IDD, pParent), 
		CRCHost(),
		CThread(),
		m_shadowStreamRunning(false),
		m_fakedChat(NULL)
{
TRY_CATCH

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//Log.RegisterLog(new CListLog(&m_logListBox));
#ifdef _DEBUG
	try
	{
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		_Module.Init(NULL, GetModuleHandle(NULL));
		Log.RegisterLog(new cFileLog());
//		Log.RegisterLog(new CNetworkLog("RCTest2"));
	
	}
	catch(...)
	{}
#endif
	InitializeCriticalSection(&m_autoConnectCs);

	TRY_CATCH
	INPUT_CONTROLLER_INSTANCE.Init();
	CATCH_LOG()

CATCH_LOG("CRCTest2Dlg::CRCTest2Dlg")
}

CRCTest2Dlg::~CRCTest2Dlg()
{
TRY_CATCH
	DeleteCriticalSection(&m_autoConnectCs);
	
	for (unsigned int i=0; i < m_vecChatWindows.size(); i++)
		delete m_vecChatWindows[i];

CATCH_LOG("CRCTest2Dlg::~CRCTest2Dlg")
}

void CRCTest2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG_LIST, m_logListBox);
	DDX_Control(pDX, IDC_CLIENTS_LIST, m_clientsListBox);
	DDX_Control(pDX, IDC_VIEW_ONLY, m_viewOnlyCheckBox);
	DDX_Control(pDX, IDC_VISUAL_POINTER, m_visualPointerCheckBox);
	DDX_Control(pDX, IDC_PROTECT_WINDOW, m_protectWindowCheckBox);
	DDX_Control(pDX, IDC_KILL_CLIENT_BTN, m_killClientButton);
	DDX_Control(pDX, IDC_AUTO_ACCEPT_CHECK, m_autoAcceptClients);
	DDX_Control(pDX, IDC_SET_SHADOW_STREAM_BTN, m_setShadowStream);
	DDX_Control(pDX, IDC_SEND_GARBAGE_BTN, m_sendGarbageBtn);
}

BEGIN_MESSAGE_MAP(CRCTest2Dlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_ADD_CLIENT_BTN, OnBnClickedAddClientBtn)
	ON_BN_CLICKED(IDC_CLEAR_LOG_BTN, OnBnClickedClearLogBtn)
	ON_BN_CLICKED(IDC_SET_SHADOW_STREAM_BTN, OnBnClickedSetShadowStreamBtn)
	ON_BN_CLICKED(IDC_PROTECT_WINDOW, OnBnClickedProtectWindow)
	ON_LBN_SELCHANGE(IDC_CLIENTS_LIST, OnLbnSelchangeClientsList)
	ON_BN_CLICKED(IDC_KILL_CLIENT_BTN, OnBnClickedKillClientBtn)
	ON_BN_CLICKED(IDC_VIEW_ONLY, OnBnClickedViewOnly)
	ON_BN_CLICKED(IDC_VISUAL_POINTER, OnBnClickedVisualPointer)
	ON_BN_CLICKED(IDC_ADD_VIEWER_BTN, OnBnClickedAddViewerBtn)
	ON_BN_CLICKED(IDC_AUTO_ACCEPT_CHECK, OnBnClickedAutoAcceptCheck)
	ON_BN_CLICKED(IDC_NETSETTINGS_BTN, OnBnClickedNetsettingsBtn)
	ON_BN_CLICKED(IDC_SEND_GARBAGE_BTN, &CRCTest2Dlg::OnBnClickedSendGarbageBtn)
	ON_BN_CLICKED(IDC_CHAT_EMULATOR_BTN, &CRCTest2Dlg::OnBnClickedChatEmulatorBtn)
	ON_MESSAGE(WM_CHAT_IS_CLOSE,OnChatIsClosing)
END_MESSAGE_MAP()


// CRCTest2Dlg message handlers

BOOL CRCTest2Dlg::OnInitDialog()
{
TRY_CATCH

	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	OnLbnSelchangeClientsList();

	/// Default relay credentials
	CSingleton<SRelayCredits>::instance().UserID = "TestUser";
	CSingleton<SRelayCredits>::instance().Password = "TestUser";
	
CATCH_LOG("CRCTest2Dlg::OnInitDialog")
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRCTest2Dlg::OnPaint() 
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
HCURSOR CRCTest2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRCTest2Dlg::OnBnClickedAddClientBtn()
{
TRY_CATCH
	CAddClientDlg addClientDlg;
	boost::shared_ptr<CAbstractStream> stream(addClientDlg.GetNewConnectedStream());
	if (stream.get())
	{
		StartClient(stream,0); //TODO: deal with priority
	}
CATCH_LOG("CRCTest2Dlg::OnBnClickedAddClientBtn")
}

void CRCTest2Dlg::OnBnClickedClearLogBtn()
{
TRY_CATCH
	CListLog(&m_logListBox).Clear();
CATCH_LOG("CRCTest2Dlg::OnBnClickedClearLogBtn")
}

void CRCTest2Dlg::NotifySessionStarted(const int clientId)
{
TRY_CATCH
	Log.Add(_MESSAGE_,"Client id(%d) session started",clientId);
	//Adding client to clients list
	m_clientsListBox.SetItemData(m_clientsListBox.AddString(Format("client id(%d)",clientId).c_str()),clientId);
CATCH_LOG("CRCTest2Dlg::NotifySessionStarted")
}

void CRCTest2Dlg::NotifySessionStopped(const int clientId, ESessionStopReason ReasonCode)
{
TRY_CATCH
	tstring reason;
	switch(ReasonCode)
	{
		case LOCAL_STOP:
			reason = "Local stop";
			break;
		case REMOTE_STOP:
			reason = "Remote stop";
			break;
		case STREAM_ERROR:
			reason = "Stream error";
			break;
		default:
			reason = "unknown";
			break;
	}
	Log.Add(_MESSAGE_,"Client removed. Reason(%s)",reason.c_str());
	/// Removing client from clients list
	for(int i=0; i<m_clientsListBox.GetCount();++i)
	{
		if (m_clientsListBox.GetItemData(i) == clientId)
		{
			m_clientsListBox.DeleteString(i);
			break;
		}
	}
	OnLbnSelchangeClientsList();
CATCH_LOG("CRCTest2Dlg::NotifySessionStopped")
}
void CRCTest2Dlg::OnBnClickedSetShadowStreamBtn()
{
TRY_CATCH

	if (m_shadowStreamRunning)
	{
		SetShadowStream(boost::shared_ptr<CAbstractStream>(reinterpret_cast<CAbstractStream*>(NULL)));
		m_shadowStreamRunning = false;
		m_setShadowStream.SetWindowText("Set shadow stream");
	} else
	{
		CFileDialog fileDialog(	FALSE /*save*/,
								"rce",
								"session");
		fileDialog.DoModal();
		tstring fileName = static_cast<LPCSTR>(fileDialog.GetPathName());
		COutStreamGZipped *fileStream = new COutStreamGZipped(fileName);
		SetShadowStream(boost::shared_ptr<CAbstractStream>(fileStream));
		m_shadowStreamRunning = true;
		m_setShadowStream.SetWindowText("Reset shadow stream");
	}

CATCH_LOG("CRCTest2Dlg::OnBnClickedSetShadowStreamBtn")
}

void CRCTest2Dlg::OnBnClickedProtectWindow()
{
TRY_CATCH

	SetProtectedWindow(m_protectWindowCheckBox.GetCheck()?m_hWnd:NULL);
		
CATCH_LOG("CRCTest2Dlg::OnBnClickedProtectWindow")
}

void CRCTest2Dlg::OnLbnSelchangeClientsList()
{
TRY_CATCH

	/// Retriving client information
	int n,id;
	n = m_clientsListBox.GetCurSel();
	if (n == LB_ERR || m_clientsListBox.GetCount() == 0)
	{
		m_killClientButton.EnableWindow(false);
		m_viewOnlyCheckBox.EnableWindow(false);
		m_visualPointerCheckBox.EnableWindow(false);
		m_sendGarbageBtn.EnableWindow(false);
		return;
	}
	id = m_clientsListBox.GetItemData(n);
	m_killClientButton.EnableWindow(true);
	m_viewOnlyCheckBox.EnableWindow(true);
	m_visualPointerCheckBox.EnableWindow(true);
	m_sendGarbageBtn.EnableWindow(true);
	m_viewOnlyCheckBox.SetCheck( GetSessionMode(id, VIEW_ONLY) );
	m_visualPointerCheckBox.SetCheck( GetSessionMode(id, VISUAL_POINTER) );

CATCH_LOG("CRCTest2Dlg::OnLbnSelchangeClientsList")
}

void CRCTest2Dlg::OnBnClickedKillClientBtn()
{
TRY_CATCH

	int n,id;
	n = m_clientsListBox.GetCurSel();
	if (n == LB_ERR)
		return;
	id = m_clientsListBox.GetItemData(n);
	/// Stopping client
	StopClient(id);

CATCH_LOG("CRCTest2Dlg::OnBnClickedKillClientBtn")
}

void CRCTest2Dlg::OnBnClickedViewOnly()
{
TRY_CATCH

	int n,id;
	n = m_clientsListBox.GetCurSel();
	if (n == LB_ERR)
		return;
	id = m_clientsListBox.GetItemData(n);
	/// Settiong session mode
	SetSessionMode(id, VIEW_ONLY, m_viewOnlyCheckBox.GetCheck());

CATCH_LOG("CRCTest2Dlg::OnBnClickedViewOnly")
}

void CRCTest2Dlg::OnBnClickedVisualPointer()
{
TRY_CATCH

	int n,id;
	n = m_clientsListBox.GetCurSel();
	if (n == LB_ERR)
		return;
	id = m_clientsListBox.GetItemData(n);
	/// Settiong session mode
	SetSessionMode(id, VISUAL_POINTER, m_visualPointerCheckBox.GetCheck());

CATCH_LOG("CRCTest2Dlg::OnBnClickedVisualPointer")
}

void CRCTest2Dlg::OnBnClickedAddViewerBtn()
{
TRY_CATCH
	CAddViewerDialog addViewerDlg;
	boost::shared_ptr<CAbstractStream> stream(addViewerDlg.GetNewConnectedStream());
	if (stream.get())
	{
		CViewerDlg *viewer = new CViewerDlg(NULL, stream);
		if (addViewerDlg.DisplayModeSelected())
		{
			viewer->SetDisplayMode(addViewerDlg.GetDisplayMode());
		}
		addViewerDlg.SetOptions(viewer);
		viewer->Start();
	}
CATCH_LOG("CRCTest2Dlg::OnBnClickedAddViewerBtn")
}

void CRCTest2Dlg::OnBnClickedAutoAcceptCheck()
{
TRY_CATCH
	if (m_autoAcceptClients.GetCheck())
	{
		//Turning autoconnect on
		Start();
	} else
	{
		//Turning autoconnect off
		Terminate();
		if (m_autoConnectStream.get())
			m_autoConnectStream.reset(reinterpret_cast<CDirectNetworkStream*>(NULL));
	}
CATCH_LOG("CRCTest2Dlg::OnBnClickedAutoAcceptCheck")
}

void CRCTest2Dlg::Execute(void *Params)
{
	SET_THREAD_LS;

TRY_CATCH

	CCritSection cs(&m_autoConnectCs);
	while(!Terminated())
	{
		m_autoConnectStream.reset(new CDirectNetworkStream());
		STLSCredentials secret; //TODO: check secret
		secret.UserID = "testuser";
		secret.Key = "testuser";
		m_autoConnectStream->SetCredentials(secret);
		STLSSuite suite;
		suite.Compression = PRS_LZO;//PRS_NULL;
		suite.Cipher = CPH_AES_256;
		suite.KeyExchange = KX_PSK;
		m_autoConnectStream->SetSuite(suite);
		m_autoConnectStream->SetConnectTimeout(0 /*INFINITE*/);
		m_autoConnectStream->SetLocalAddr(5900);
		m_autoConnectStream->SetRemoteAddr("", 0);
		TRY_CATCH
			m_autoConnectStream->Connect(); //TODO: how to set another port?
			StartClient(m_autoConnectStream,0); //TODO: set priority
		CATCH_LOG("AutoConnect failed")
	}
	m_autoConnectStream.reset(reinterpret_cast<CDirectNetworkStream*>(NULL));
CATCH_LOG("CRCTest2Dlg::Execute")
}

void CRCTest2Dlg::OnBnClickedNetsettingsBtn()
{
TRY_CATCH
	CNetSettingsDlg dlg;
	dlg.DoModal();
CATCH_LOG("CRCTest2Dlg::OnBnClickedNetsettingsBtn")
}

void CRCTest2Dlg::OnBnClickedSendGarbageBtn()
{
TRY_CATCH

	int n,id;
	n = m_clientsListBox.GetCurSel();
	if (n == LB_ERR)
		return;
	id = m_clientsListBox.GetItemData(n);
	boost::shared_ptr<CAbstractStream> stream = GetClientStream(id);
	if (stream.get() != NULL)
	{
		srand(GetTickCount());
		char buf[MAX_PATH];
		for(int i=0; i<sizeof(buf); ++i)
		{
			buf[i] = rand() % 255;
		}
		stream->Send(buf, sizeof(buf));
	}
CATCH_LOG()
}

void CRCTest2Dlg::OnBnClickedChatEmulatorBtn()
{
TRY_CATCH

	if(!m_vecChatWindows.empty())
		return;

	m_fakedChat = new CFakedChat(this);
	m_fakedChat->Create(MAKEINTRESOURCE(IDD_FAKED_CHAT),this);
	m_vecChatWindows.push_back(m_fakedChat);
	
	try
	{
		HideLayeredWindow(m_fakedChat->m_hWnd);
	}
	catch(...)
	{
	}
	
	m_fakedChat->ShowWindow(SW_SHOW);

	if(!INPUT_CONTROLLER_INSTANCE.Started())
	{
		POINT point;
		GetCursorPos(&point);
		FAKED_POINTER_INSTANCE.MoveTo(point.x, point.y);
		FAKED_POINTER_INSTANCE.SetState(CFakedPointer::NORMAL);
		FAKED_POINTER_INSTANCE.Show();
		Log.Add(_MESSAGE_, _T("STARING CONTROLLER: %d, %d, %d"), m_fakedChat->m_hWnd, m_fakedChat->m_mouseMsg, m_fakedChat->m_keyboardMsg);
		INPUT_CONTROLLER_INSTANCE.Start(m_fakedChat->m_hWnd, WM_CHAT_MOUSE/*m_fakedChat->m_mouseMsg*/, WM_CHAT_KBRD/*m_fakedChat->m_keyboardMsg*/);
	}

CATCH_LOG()
}

LRESULT CRCTest2Dlg::OnChatIsClosing(WPARAM wParam, LPARAM lParam)
{
TRY_CATCH

	for (	std::vector<CFakedChat*>::iterator iChat=m_vecChatWindows.begin();
			iChat != m_vecChatWindows.end();
			iChat++)
	{
		if ( (*iChat)->m_hWnd == (HWND)wParam )
		{
			try
			{
				HideLayeredWindow(m_fakedChat->m_hWnd,true);
			}
			catch(...)
			{
			}
			delete (*iChat);
			m_vecChatWindows.erase(iChat);
			break;
		}
	}
	if(m_vecChatWindows.empty() && INPUT_CONTROLLER_INSTANCE.Started())
	{
		FAKED_POINTER_INSTANCE.Hide();
		INPUT_CONTROLLER_INSTANCE.Stop();
	}
CATCH_LOG()

	return 0;
}
