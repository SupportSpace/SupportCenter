#pragma once
#include <AidLib/CThread/CThread.h>
#include "afxcmn.h"
#include "afxwin.h"
#include <NWL/Streaming/CStreamFactory.h>

// CFactoryConnectDlg dialog

class CFactoryConnectDlg : public CDialog, public CThread, public CStreamFactory
{
	DECLARE_DYNAMIC(CFactoryConnectDlg)
public:
	CFactoryConnectDlg(CWnd* pParent = NULL);   // standard constructor
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
	/// Sends a message to the specified destination peer (user ID).  
	/// @param peerId destination peer
	/// @param messageData Message data is an arbitrary string.
	virtual void SendMsg(const tstring& peerId, const tstring& messageData);

	/// Handles an incoming message from a specified source peer (user ID).
	/// @param peerId source peer
	/// @param messageData Message data is an arbitrary string.
	virtual void HandleMsg(const tstring& peerId, tstring& messageData);

	/// A callback method that is invoked by the factory to notify the progress 
	/// and status of connection attempt.
	/// @param percentCompleted Percent Completed is an integer value in the range of 0-100 
	/// that represents the connection progress (in terms of steps/actions done).
	/// @param status Status is a text message that describes the current step.
	virtual void NotifyProgress(const int& percentCompleted, const tstring& status);

	/// On init dialog override
	virtual BOOL OnInitDialog();

	/// Cancell dialog event handler
	virtual void OnCancel();

	/// result stream
	boost::shared_ptr<CAbstractStream> m_stream;

	/// timeout
	int m_timeOut;
	/// masterrole
	bool m_masterRole;
	/// source peerID
	tstring m_sourcePeer;
	/// dest peerID
	tstring m_destPeer;
	/// Session Id
	tstring m_sessionId;

	/// Connect error
	tstring m_errorMessage;
public:

	/// Returns new stream
	boost::shared_ptr<CAbstractStream> GetNewStream(const tstring &sessionId,const tstring &sourcePeer, const tstring& destPeer, const int timeOut, const bool masterRole);
};
