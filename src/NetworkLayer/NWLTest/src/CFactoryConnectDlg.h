#pragma once

#include "afxcmn.h"
#include "afxwin.h"
#include <AidLib/CThread/CThread.h>
#include <NWL/Streaming/CStreamFactoryRelayedImpl.h>
#include "resource.h"

// CFactoryConnectDlg dialog

class CFactoryConnectDlg : public CDialog, public CThread, public CStreamFactoryRelayedImpl
{
	DECLARE_DYNAMIC(CFactoryConnectDlg)
	bool m_async;
public:
	CFactoryConnectDlg(bool async, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFactoryConnectDlg();

// Dialog Data
	enum { IDD = IDD_PROGRESS_DLG };
    
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual void Execute(void *Params);

	DECLARE_MESSAGE_MAP()
	CProgressCtrl m_progressCtrl;
	CStatic m_captionStatic;

protected:
	/// A callback method that is invoked by the factory to notify the progress 
	/// and status of connection attempt.
	/// @param percentCompleted Percent Completed is an integer value in the range of 0-100 
	/// that represents the connection progress (in terms of steps/actions done).
	/// @param status Status is a text message that describes the current step.
	virtual void NotifyProgress(const int& percentCompleted, const tstring& status);

	/// Virtual metody, that notifyes connect is complete
	/// @param stream connected stream, null if not connected (NULL -> stream.get() == NULL)
	virtual void ConnectCompletion(boost::shared_ptr<CAbstractNetworkStream> stream);

	/// On init dialog override
	virtual BOOL OnInitDialog();

	/// result stream
	boost::shared_ptr<CAbstractNetworkStream> m_stream;

	/// timeout
	int m_timeOut;
	/// masterrole
	bool m_masterRole;
	/// source peerID
	tstring m_sourcePeer;
	/// dest peerID
	tstring m_destPeer;
	/// Connect error
	tstring m_sessionId;
	tstring m_errorMessage;
	tstring m_srvUserId;
	tstring m_srvPass;
public:

	/// Returns new stream
	boost::shared_ptr<CAbstractNetworkStream> GetNewStream(const tstring &sessionId,const tstring &sourcePeer, const tstring& destPeer, const int timeOut, const bool masterRole, const tstring& srvUserID, const tstring& srvPass);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedAbort();
};
