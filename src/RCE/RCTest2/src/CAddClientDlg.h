#pragma once
#include "afxwin.h"
#include <RCEngine/RCEngine.h>
#include <boost/shared_ptr.hpp>

class COutStreamGZipped;
class CDirectNetworkStream;
// CAddClientDlg dialog

class CAddClientDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddClientDlg)

	/// Current stream object
	boost::shared_ptr<CDirectNetworkStream> m_stream;
	boost::shared_ptr<CAbstractStream> m_astream;

public:
	CAddClientDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddClientDlg();

	/// Returns new connected stream or NULL if failed
	/// @return new connected stream or NULL if failed
	boost::shared_ptr<CAbstractStream> GetNewConnectedStream();

	BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_ADD_CLIENT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	CEdit m_hostEditBox;
	CEdit m_remotePortEditBox;
	CEdit m_localPortEditBox;
	CButton m_connectButton;
	CButton m_setShadowStreamButton;

	boost::shared_ptr<COutStreamGZipped> m_shadowStream;
public:
	afx_msg void OnBnClickedButtonShadowStream();
	void OnConnectError(void*, EConnectErrorReason reason);
	void OnConnected(void*);
	void OnDisconnected(void*);
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnOptionsFullscreenmode();
	CEdit m_sourcePeerEdit;
	CEdit m_destPeerEdit;
	CEdit m_sessionIdEdit;
	afx_msg void OnBnClickedButtonConnect2();
	afx_msg void OnBnClickedButtonResetServer();
};
