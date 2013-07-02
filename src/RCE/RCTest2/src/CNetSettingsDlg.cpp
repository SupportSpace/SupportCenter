// CNetSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RCTest2.h"
#include "CNetSettingsDlg.h"
#include <AidLib/Logging/cLog.h>
#include <NWL/Streaming/CNetworkLayer.h>
#include <NWL/Streaming/CStreamException.h>
#include <AidLib/CSingleton/CSingleton.h>

// CNetSettingsDlg dialog

IMPLEMENT_DYNAMIC(CNetSettingsDlg, CDialog)
CNetSettingsDlg::CNetSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNetSettingsDlg::IDD, pParent)
	, m_stunServerPort(0)
	, m_relayServerPort(0)
	, m_IMStubServerPort(0)
{
}

CNetSettingsDlg::~CNetSettingsDlg()
{
}

void CNetSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RELAY_EDIT, m_relayHostEdit);
	DDX_Control(pDX, IDC_USERID_EDIT, m_UserIDEdit);
	DDX_Control(pDX, IDC_PASSWORD_EDIT, m_passwordEdit);
	DDX_Text(pDX, IDC_RELAYPORT_EDIT, m_relayServerPort);
	DDX_Text(pDX, IDC_STUNPORT_EDIT, m_stunServerPort);
	DDX_Text(pDX, IDC_IMSTUBPORT_EDIT, m_IMStubServerPort);
}


BEGIN_MESSAGE_MAP(CNetSettingsDlg, CDialog)
END_MESSAGE_MAP()


// CNetSettingsDlg message handlers
BOOL CNetSettingsDlg::OnInitDialog()
{
TRY_CATCH
	BOOL res = CDialog::OnInitDialog();
	m_relayHostEdit.SetWindowText(NWL_INSTANCE.GetRelayHost().c_str());
	m_UserIDEdit.SetWindowText(CSingleton<SRelayCredits>::instance().UserID.c_str());
	m_passwordEdit.SetWindowText(CSingleton<SRelayCredits>::instance().Password.c_str());
	m_relayServerPort = NWL_INSTANCE.GetRelayTCPPort();
	m_stunServerPort = NWL_INSTANCE.GetRelayUDPPort();
	m_IMStubServerPort = NWL_INSTANCE.GetIMStubPort();
	UpdateData(FALSE);
	return res;
CATCH_LOG("CNetSettingsDlg::OnInitDialog")
	return FALSE;
}

void CNetSettingsDlg::OnOK()
{
TRY_CATCH
	//Settint relay host
	TCHAR buf[MAX_PATH];
	m_relayHostEdit.GetWindowText(buf,MAX_PATH);
	NWL_INSTANCE.SetRelayHost(buf);
	m_UserIDEdit.GetWindowText(buf,MAX_PATH);
    CSingleton<SRelayCredits>::instance().UserID =  buf;
	m_passwordEdit.GetWindowText(buf,MAX_PATH);
	CSingleton<SRelayCredits>::instance().Password = buf;
	UpdateData();
	NWL_INSTANCE.SetRelayTCPPort(m_relayServerPort);
	NWL_INSTANCE.SetRelayUDPPort(m_stunServerPort);
	NWL_INSTANCE.SetIMStubPort(m_IMStubServerPort);
	CDialog::OnOK();
CATCH_LOG("CNetSettingsDlg::OnOK")
}