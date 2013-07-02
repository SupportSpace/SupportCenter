#pragma once
#include "afxwin.h"
#include <NWL/Streaming/CNATTraversingUDPNetworkStream.h>
#include <NWL/Streaming/CSocketSystem.h>
#include <NWL/TLS/CTLSSystem.h>


// CNATTraversalDialog dialog

class CNATTraversalDialog : public CDialog
{
	DECLARE_DYNAMIC(CNATTraversalDialog)

public:
	CNATTraversalDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNATTraversalDialog();

// Dialog Data
	enum { IDD = IDD_NAT_TRAVERSAL_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_SrvAddress;
	CEdit m_SrvPort;
	CEdit m_SrvUserId;
	CEdit m_SrvPassword;
	CEdit m_SrvSessionId;
	CEdit m_ConnectId;
	CEdit m_LocalPeerId;
	CEdit m_RemotePeerId;
	CEdit m_Timeout;
	CEdit m_UserId;
	CEdit m_Key;
	CEdit m_AuthDelay;
	CEdit m_AuthMax;
	CEdit m_BindDelay;
	CEdit m_BindMax;
	CEdit m_ProbeDelay;
	CEdit m_ProbeMax;
	CEdit m_PortRange;
	CEdit m_SendText;
	CListBox m_SendList;
	CListBox m_ReceiveList;
	int m_Cipher;
	int m_nCompression;
	int m_KeyExchange;
	CButton m_ConnectButton;
	CButton m_DisconnectButton;
	CButton m_SendButton;
	CButton m_AddBreakLine;
	CButton m_MasterMode;
	afx_msg void OnBnClickedRadioNatCiphNone();
	afx_msg void OnBnClickedRadioNatCiphAes128();
	afx_msg void OnBnClickedRadioNatCiphAes256();
	afx_msg void OnBnClickedRadioNat3des();
	afx_msg void OnBnClickedRadioNatCiphRc4128();
	afx_msg void OnBnClickedRadioNatCompNone();
	afx_msg void OnBnClickedRadioNatCompZlib();
	afx_msg void OnBnClickedRadioNatCompLzo();
	afx_msg void OnBnClickedRadioNatKxPsk();
	afx_msg void OnBnClickedRadioNatKxDhePsk();
	afx_msg void OnBnClickedButtonNatClear();
	afx_msg void OnBnClickedButtonNatConnect();
	afx_msg void OnBnClickedButtonNatDisconnect();
	afx_msg void OnBnClickedButtonNatSend();

private:
	void DoConnect();
	void DoDisconnect();

	void OnConnected(void*);
	void OnDisconnected(void*);
	void OnConnectError(void*, EConnectErrorReason);

	static void CALLBACK OnTimer(
	    HWND hwnd,	// handle of window for timer messages 
		UINT uMsg,	// WM_TIMER message
		UINT idEvent,	// timer identifier
		DWORD dwTime 	// current system time
   );

	void ReadFromStream();

private:
	CSocketSystem SockSys;
	CTLSSystem TLSSys;
	CNATTraversingUDPNetworkStream Stream;
	UINT_PTR timer;

};
