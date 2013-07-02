// RCTest2Dlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <RCEngine/RCEngine.h>
#include <AidLib/CThread/CThread.h>
#include <NWL/Streaming/CDirectNetworkStream.h>
#include <AidLib/CThread/CThreadLS.h>
#include "CFakedChat.h"

#pragma warning( push )
#pragma warning( disable: 4996 )//<func> was declared deprecated

// CRCTest2Dlg dialog
class CRCTest2Dlg : public CDialog, public CRCHost, public CThread /*thread for autoaccepting clients*/
{
private:

	/// Stream for autoconnects
	boost::shared_ptr<CDirectNetworkStream> m_autoConnectStream;
	CRITICAL_SECTION m_autoConnectCs;
	bool m_shadowStreamRunning;

	/// Logging into ListBox
	class CListLog : public cLog, public CThread
	{
	private:

		/// listbox for logging reference
		CListBox* m_listBox;
	public:
		/// Initializes logger
		/// @param listBox listbox for logging
		CListLog(CListBox* listBox)
			: m_listBox(listBox), CThread()
		{
			Start();
		}

		~CListLog()
		{
			PostThreadMessage(GetTid(),WM_QUIT,0,0);
		}

		/// Clears log
		void Clear()
		{
			m_listBox->ResetContent();
		}
	protected:

		/// CLog AddString redefinition
		virtual void AddString(const TCHAR *LogString, const eSeverity Severity) throw()
		{
			TCHAR seps[]   = _T("\n");
			std::auto_ptr<char> str = std::auto_ptr<char>(new char[strlen(LogString)+1]);
			strcpy(str.get(),LogString);
			for (	PTCHAR token = _tcstok( str.get() , seps );
					token;
					token = _tcstok(NULL, seps) )
			{
				char* str = strdup(token);
				PostThreadMessage(GetCurrentThreadId(),WM_USER,0,reinterpret_cast<LPARAM>(str));
				//m_listBox->AddString(token);			
			}
		}

		virtual void Execute(void*)
		{
			SET_THREAD_LS;
		try
		{
			for(MSG msg;
				GetMessage(&msg,NULL,0,0) &&
				!Terminated();)
			{
				switch(msg.message)
				{
					case WM_USER:
						m_listBox->AddString(reinterpret_cast<LPCSTR>(msg.lParam));
						delete reinterpret_cast<char*>(msg.lParam);
						break;
					default:
						break;
				}
			}
		}
		catch(...)
		{
		}
		}
	};

private:
	std::vector<CFakedChat*> m_vecChatWindows;
public:
	CRCTest2Dlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CRCTest2Dlg();

	/// DOXYS_OFF
// Dialog Data
	enum { IDD = IDD_RCTEST2_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnChatIsClosing(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	virtual void OnCancel()
	{
		Log.ClearList();
		CDialog::OnCancel();
	}

	virtual void OnOK()
	{
		Log.ClearList();
		CDialog::OnOK();
	}

	virtual void Execute(void *Params);

public:
	CFakedChat* m_fakedChat;
	CListBox m_logListBox;
	CListBox m_clientsListBox;
	CButton m_viewOnlyCheckBox;
	CButton m_visualPointerCheckBox;
	CButton m_protectWindowCheckBox;
	afx_msg void OnBnClickedAddClientBtn();
	afx_msg void OnBnClickedClearLogBtn();
	/// DOXYS_ON

	/// CRCHost interface notifications
	/// A virtual method that notifies session has started.
	/// @param clientId corresponding cliend identifier
	virtual void NotifySessionStarted(const int clientId);

	/// A virtual method that notifies session has stopped and why 
	/// @param ReasonCode stop reason (Stop() called; Remote Stop() called; Stream signaled end-of-file; etc.)
	/// @param clientId corresponding cliend identifier
	virtual void NotifySessionStopped(const int clientId, ESessionStopReason ReasonCode);
	afx_msg void OnBnClickedSetShadowStreamBtn();
	afx_msg void OnBnClickedProtectWindow();
	afx_msg void OnLbnSelchangeClientsList();
	CButton m_killClientButton;
	afx_msg void OnBnClickedKillClientBtn();
	afx_msg void OnBnClickedViewOnly();
	afx_msg void OnBnClickedVisualPointer();
	afx_msg void OnBnClickedAddViewerBtn();
	CButton m_autoAcceptClients;
	afx_msg void OnBnClickedAutoAcceptCheck();
	CButton m_setShadowStream;
	afx_msg void OnBnClickedNetsettingsBtn();
	afx_msg void OnBnClickedSendGarbageBtn();
	CButton m_sendGarbageBtn;
	afx_msg void OnBnClickedChatEmulatorBtn();
};

#pragma warning( pop )
