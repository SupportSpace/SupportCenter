// CFactoryConnectDlg.cpp : implementation file
//
#include "stdafx.h"
#include "CFactoryConnectDlg.h"
#include <AidLib/Logging/clog.h>
#include <NWL/Streaming/CIMStub.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "resource.h"
#include ".\cfactoryconnectdlg.h"
#include <AidLib/CThread/CThreadLS.h>


// CFactoryConnectDlg dialog

IMPLEMENT_DYNAMIC(CFactoryConnectDlg, CDialog)
CFactoryConnectDlg::CFactoryConnectDlg(bool async, CWnd* pParent /*=NULL*/)
	:	CDialog(CFactoryConnectDlg::IDD, pParent), 
		CThread(),
		m_async(async)
{
TRY_CATCH
	CThread::SetTerminateTimeout(INFINITE);
CATCH_THROW("CFactoryConnectDlg::CFactoryConnectDlg")
}

CFactoryConnectDlg::~CFactoryConnectDlg()
{
TRY_CATCH
	Stop(false);
CATCH_LOG("CFactoryConnectDlg::~CFactoryConnectDlg")
}

boost::shared_ptr<CAbstractNetworkStream> CFactoryConnectDlg::GetNewStream(const tstring &sessionId,const tstring &sourcePeer, const tstring& destPeer, const int timeOut, const bool masterRole, const tstring& srvUserID, const tstring& srvPass)
{
TRY_CATCH
	m_sessionId = sessionId;
	m_sourcePeer = sourcePeer;
	m_destPeer = destPeer;
	m_timeOut = timeOut;
	m_masterRole = masterRole;
	m_srvUserId = srvUserID;
	m_srvPass = srvPass;
	if (DoModal()==IDOK)
	{
		return m_stream;
	}
	throw MCStreamException(m_errorMessage);
CATCH_THROW("CFactoryConnectDlg::GetNewStream")
}


BOOL CFactoryConnectDlg::OnInitDialog()
{
TRY_CATCH
	BOOL res = CDialog::OnInitDialog();
	m_progressCtrl.SetRange(0,100);
	m_progressCtrl.SetPos(0);
	/// Starting connect
	if (m_async) 
	{
		SetServerUserId(m_srvUserId);//CSingleton<SRelayCredits>::instance().UserID);
		SetServerPassword(m_srvPass);//CSingleton<SRelayCredits>::instance().Password);
		m_stream = Connect(m_sessionId,m_sourcePeer,m_destPeer,m_timeOut,m_masterRole,true /*async*/);
	} else
		Start();
CATCH_LOG("CFactoryConnectDlg::OnInitDialog")
	return 0;
}

void CFactoryConnectDlg::ConnectCompletion(boost::shared_ptr<CAbstractNetworkStream> stream)
{
TRY_CATCH
	if (stream.get())
	{
		m_stream = stream;
		PostMessage(WM_COMMAND,IDOK,0);
	} else
	{
		m_errorMessage = m_error;
		PostMessage(WM_COMMAND,IDCANCEL,0);
	}
CATCH_LOG("CFactoryConnectDlg::ConnectCompletion")
}

void CFactoryConnectDlg::Execute(void *Params)
{
	HWND hWnd = m_hWnd;
	SET_THREAD_LS;
TRY_CATCH
	
	/// Starting connection
	try
	{
		SetServerUserId(m_srvUserId);//CSingleton<SRelayCredits>::instance().UserID);
		SetServerPassword(m_srvPass);//CSingleton<SRelayCredits>::instance().Password);
		m_stream = Connect(m_sessionId,m_sourcePeer,m_destPeer,m_timeOut,m_masterRole);
		::PostMessage(hWnd,WM_COMMAND,IDOK,0);
		return;
	}
	catch(CStreamException &e)
	{
		m_errorMessage = e.What();
		try
		{
			/// Retriving all my messages from server
			CIMStub im(m_sourcePeer);
			im.RemoveAllMyMessagesFromServer();
		}
		catch(...)
		{
		}
		//CDialog::OnCancel();
		::PostMessage(hWnd,WM_COMMAND,IDCANCEL,0);
		return;
	}
CATCH_LOG("CFactoryConnectDlg::Execute")
try
{
	/// Retriving all my messages from server
	CIMStub im(m_sourcePeer);
	im.RemoveAllMyMessagesFromServer();
}
catch(...)
{
}
	//CDialog::OnCancel();
	::PostMessage(hWnd,WM_COMMAND,IDCANCEL,0);
}

void CFactoryConnectDlg::NotifyProgress(const int& percentCompleted, const tstring& status)
{
TRY_CATCH
	m_progressCtrl.SetPos(percentCompleted);
	m_captionStatic.SetWindowText(status.c_str());
	Log.Add(_MESSAGE_,status.c_str());
CATCH_LOG("CFactoryConnectDlg::NotifyProgress")
}

void CFactoryConnectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progressCtrl);
	DDX_Control(pDX, IDC_CAPTION_STATIC, m_captionStatic);
}


BEGIN_MESSAGE_MAP(CFactoryConnectDlg, CDialog)
	ON_BN_CLICKED(ID_ABORT, OnBnClickedAbort)
END_MESSAGE_MAP()


// CFactoryConnectDlg message handlers

void CFactoryConnectDlg::OnBnClickedAbort()
{
TRY_CATCH
	AbortConnect();
CATCH_LOG("CFactoryConnectDlg::OnBnClickedAbort")
}
