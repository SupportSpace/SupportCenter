// NetworkLayerTestDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <NWL/Streaming/CSocketSystem.h>
#include <NWL/TLS/CTLSSystem.h>
#include <NWL/Streaming/CDirectNetworkStream.h>


// CNetworkLayerTestDlg dialog
class CNetworkLayerTestDlg : public CDialog
{
// Construction
public:
	CNetworkLayerTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_NETWORKLAYERTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// Remote Address
	CEdit m_RemoteAddress;
	// RemotePort
	CEdit m_RemotePort;
	// Local Port
	CEdit m_LocalPort;
	// Use http proxy
	CButton m_UseProxy;
	// Proxy address
	CEdit m_ProxyAddress;
	// Proxy port
	CEdit m_ProxyPort;
	// Timeout
	CEdit m_Timeout;
	// User ID
	CEdit m_UserID;
	// User key
	CEdit m_UserKey;
	// Cipher
	int m_Cipher;
	// Compression
	int m_Compression;
	// Key exchange
	int m_KeyExchange;

private:
	CSocketSystem SockSys;
	CTLSSystem TLSSys;
	CDirectNetworkStream Stream;
	UINT_PTR timer;

private:
	void ReadFromStream();

	static void CALLBACK OnTimer(
	    HWND hwnd,	// handle of window for timer messages 
		UINT uMsg,	// WM_TIMER message
		UINT idEvent,	// timer identifier
		DWORD dwTime 	// current system time
   );

	void DoConnect();
	void DoDisconnect();

	void OnConnected(void*);
	void OnDisconnected(void*);
	void OnConnectError(void*, EConnectErrorReason);
public:
	// // Text to send
	CEdit m_SendText;
	CButton m_SendButton;
	CButton m_ClearButton;
	CListBox m_SendList;
	CListBox m_ReceiveList;
	afx_msg void OnBnClickedButtonClear();
	CButton m_ConnectButton;
	CButton m_DisconnectButton;
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonDisconnect();
	afx_msg void OnBnClickedButtonSend();
	CButton m_AddBreakLine;
	afx_msg void OnBnClickedRadioCiphNone();
	afx_msg void OnBnClickedRadioCiphAes128();
	afx_msg void OnBnClickedRadioCiphAes256();
	afx_msg void OnBnClickedRadio3des();
	afx_msg void OnBnClickedRadioCiphRc4128();
	afx_msg void OnBnClickedRadioCompNone();
	afx_msg void OnBnClickedRadioCompZlib();
	afx_msg void OnBnClickedRadioCompLzo();
	afx_msg void OnBnClickedRadioKxPsk();
	afx_msg void OnBnClickedRadioKxDhePsk();
};
