// CFactoryConnectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RCTest2.h"
#include "CFactoryConnectDlg.h"
#include <AidLib/Logging/clog.h>
#include <NWL/Streaming/CIMStub.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "CNetSettingsDlg.h"
#include <AidLib/CThread/CThreadLS.h>


// CFactoryConnectDlg dialog

IMPLEMENT_DYNAMIC(CFactoryConnectDlg, CDialog)
CFactoryConnectDlg::CFactoryConnectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFactoryConnectDlg::IDD, pParent), CThread()
{
TRY_CATCH
CATCH_THROW("CFactoryConnectDlg::CFactoryConnectDlg")
}

boost::shared_ptr<CAbstractStream> CFactoryConnectDlg::GetNewStream(const tstring &sessionId,const tstring &sourcePeer, const tstring& destPeer, const int timeOut, const bool masterRole)
{
TRY_CATCH
	m_sourcePeer = sourcePeer;
	m_destPeer = destPeer;
	m_timeOut = timeOut;
	m_sessionId = sessionId;
	m_masterRole = masterRole;
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
	Start();
CATCH_LOG("CFactoryConnectDlg::OnInitDialog")
	return 0;
}

void CFactoryConnectDlg::OnCancel()
{
TRY_CATCH
	AbortConnect();
CATCH_LOG("CFactoryConnectDlg::OnCancell")
}

CFactoryConnectDlg::~CFactoryConnectDlg()
{
TRY_CATCH
CATCH_LOG("CFactoryConnectDlg::~CFactoryConnectDlg")
}

void CFactoryConnectDlg::SendMsg(const tstring& peerId, const tstring& messageData)
{
TRY_CATCH
	CIMStub im(peerId);
	im.SendMsg(messageData);
CATCH_THROW("CFactoryConnectDlg::SendMsg")
}

void CFactoryConnectDlg::HandleMsg(const tstring& peerId, tstring& messageData)
{
TRY_CATCH
	CIMStub im(m_sourcePeer);
	im.HandleMsg(messageData);
CATCH_THROW("CFactoryConnectDlg::HandleMsg")
}


void CFactoryConnectDlg::Execute(void *Params)
{
	SET_THREAD_LS;

TRY_CATCH
	/// Starting connection
	try
	{
		SetServerUserId(CSingleton<SRelayCredits>::instance().UserID);
		SetServerPassword(CSingleton<SRelayCredits>::instance().Password);
		m_stream = Connect(m_sessionId,m_sourcePeer,m_destPeer,m_timeOut,m_masterRole);
		PostMessage(WM_COMMAND,IDOK,0);
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
		CDialog::OnCancel();
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
	CDialog::OnCancel();
}

void CFactoryConnectDlg::NotifyProgress(const int& percentCompleted, const tstring& status)
{
TRY_CATCH
	m_progressCtrl.SetPos(percentCompleted);
	m_captionStatic.SetWindowText(status.c_str());
CATCH_LOG("CFactoryConnectDlg::NotifyProgress")
}

void CFactoryConnectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progressCtrl);
	DDX_Control(pDX, IDC_CAPTION_STATIC, m_captionStatic);
}


BEGIN_MESSAGE_MAP(CFactoryConnectDlg, CDialog)
END_MESSAGE_MAP()


// CFactoryConnectDlg message handlers
