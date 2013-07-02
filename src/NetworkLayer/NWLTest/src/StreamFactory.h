#pragma once
#include "afxwin.h"
#include <NWL/Streaming/CStreamFactory.h>


// CStreamFactoryDlg dialog
class CStreamFactoryDlg : public CDialog
{
	DECLARE_DYNAMIC(CStreamFactoryDlg)

	/// Pointer to current stream
	boost::shared_ptr<CAbstractNetworkStream> m_stream;
public:
	CStreamFactoryDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStreamFactoryDlg();

	/// On init dialog override
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_STREAM_FABRIC_TEST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	CEdit m_localPeerEdit;
	CEdit m_remotePeerEdit;
	CButton m_masterCheckBox;
protected:
	afx_msg void OnBnClickedConnectButton();
	afx_msg void OnBnClickedResetimButton();
	CEdit m_SrvUserId;
	CEdit m_SrvPassword;
	CEdit m_SrvSessionId;
	bool m_MasterRole;
	CButton m_sendBtn;
	CButton m_connectBtn;
	CButton m_disconnectBtn;
	CEdit m_customPort;
	CButton m_useCustomPort;
	afx_msg void OnBnClickedDisconnectButton();
	afx_msg void OnBnClickedButtonSend();

		
	UINT_PTR timer;
	void ReadFromStream();
	static void CALLBACK OnTimer( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime );

	CButton m_AddBreakLine;
	CListBox m_ReceiveList;
	CListBox m_SendList;
	CEdit m_SendText;
public:
	afx_msg void OnBnClickedButtonClear();
protected:
	int m_connectTimeout;
	CButton m_asyncConnectCheck;
};
