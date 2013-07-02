#pragma once

#include <RCEngine/RCEngine.h>

// CViewerDlg dialog

class CViewerDlg : public CDialog, public CRCViewer
{
	DECLARE_DYNAMIC(CViewerDlg)

public:
	CViewerDlg(CWnd* pParent, boost::shared_ptr<CAbstractStream> stream);   // standard constructor
	virtual ~CViewerDlg();

// Dialog Data
	enum { IDD = IDD_VIEWER_DIALOG };

protected:

	/// Remote desktop width
	int m_cx;
	/// Remoted desktop height
	int m_cy;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	/// A virtual method that notifies session has started.
	virtual void NotifySessionStarted();

	/// A virtual method that notifies session has stopped and why 
	/// @param ReasonCode stop reason (Stop() called; Remote Stop() called; Stream signaled end-of-file; etc.)
	virtual void NotifySessionStopped(ESessionStopReason ReasonCode);


	/// A virtual method that called after session is started, 
	/// or notifies remote desktop size is changed. Size is real remote desktop size.
	/// @param width new remote desktop width
	/// @param height new remote desktop height
	virtual void SetRemoteDesktopSize(const int width, const int height);

	virtual void OnCancel();
	virtual void OnOK();

	/// Unblock file stream if needed
	void UnblockFileStream();

	DECLARE_MESSAGE_MAP()

	virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
	afx_msg void OnCommandsStart();
	afx_msg void OnCommandsStop();
	afx_msg void OnOptionsViewonly();
	afx_msg void OnOptionsVisualpointer();
	afx_msg void OnOptionsScalemode();
	afx_msg void OnOptionsFullscreenmode();
	afx_msg void OnOptionsScrollmode();
	afx_msg void OnReplayStart();
	afx_msg void OnReplayStop();
	afx_msg void OnReplayPause();
	afx_msg void OnFastforwardX1();
	afx_msg void OnFastforwardX2();
	afx_msg void OnFastforwardX4();
	afx_msg void OnFastforwardX8();
	afx_msg void OnFastforwardX20();
	afx_msg void OnFastforwardFastest();
	afx_msg void OnZoomStretching();
	afx_msg void OnOptionsSetshadowstream();
public:
	afx_msg void OnCommandsSendctrl();
	afx_msg void OnCommandsSendgarbageintostream();
};
