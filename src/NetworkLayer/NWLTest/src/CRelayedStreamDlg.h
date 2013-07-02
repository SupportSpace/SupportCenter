#pragma once
#include "afxwin.h"
#include <NWL/Streaming/CRelayedNetworkStream.h>


// CRelayedStreamDlg dialog

class CRelayedStreamDlg : public CDialog
{
private:

	/// Network relayed stream
	std::auto_ptr<CRelayedNetworkStream<> > m_stream;
	UINT_PTR timer;
	void ReadFromStream();
	static void CALLBACK OnTimer( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime );
	DECLARE_DYNAMIC(CRelayedStreamDlg)
	
public:
	CRelayedStreamDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRelayedStreamDlg();


// Dialog Data
	enum { IDD = IDD_RELAYED_STREAM_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_relayAddressEdit;
	int m_relayPortEdit;
	CEdit m_serverUserIDEdit;
	CEdit m_servPassEdit;
	CEdit m_connIDEdit;
	CEdit m_sourcePeerIdEdit;
	CEdit m_destPeerIdEdit;
	CEdit m_userIDEdit;
	CEdit m_PassEdit;
	int m_timeOut;
	afx_msg void OnBnClickedButtonNatConnect();
	CButton m_MasterRoleCheck;
	CButton m_disconnectBtn;
	CButton m_connectBtn;
	afx_msg void OnBnClickedButtonNatDisconnect();
	CEdit m_SendText;
	CListBox m_SendList;
	CListBox m_ReceiveList;
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnBnClickedButtonSend();
	CButton m_AddBreakLine;
	CButton m_sendBtn;
};
