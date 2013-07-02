#pragma once
#include "afxwin.h"
#include <AidLib/Strings/tstring.h>


// CNetSettingsDlg dialog

class CNetSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CNetSettingsDlg)

public:
	CNetSettingsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNetSettingsDlg();

// Dialog Data
	enum { IDD = IDD_NET_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	CEdit m_relayHostEdit;
public:
	CEdit m_UserIDEdit;
	CEdit m_passwordEdit;
	int m_stunServerPort;
	int m_relayServerPort;
	int m_IMStubServerPort;
};

/// Relay server credentials
struct SRelayCredits
{
	tstring UserID;
	tstring Password;
};
