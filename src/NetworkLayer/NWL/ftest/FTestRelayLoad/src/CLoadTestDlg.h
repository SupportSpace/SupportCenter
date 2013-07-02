// CLoadTestDlg.h : Declaration of the CLoadTestDlg

#pragma once

#include "resource.h"       // main symbols

#include <atlhost.h>
#include <wtl/atlapp.h>
#include <wtl/atlddx.h>
#include <wtl/atlctrls.h>
#include <atlcrack.h>
#include <boost/shared_ptr.hpp>
#include "CAbstractTest.h"
#include "CTestSettings.h"
#include <list>

#define TEXT_BUF_SIZE 1024

// CLoadTestDlg

class CLoadTestDlg 
	:	public CAxDialogImpl<CLoadTestDlg>
{
private:
	CComboBox m_testsCombo;
	boost::shared_ptr<CAbstractTest> m_test;

	enum ETestState
	{
		tsStopped,
		tsRunning
	};
public:
	CLoadTestDlg()
	{
	}

	~CLoadTestDlg()
	{
	}

	enum { IDD = IDD_LOADTESTDLG };

BEGIN_MSG_MAP(CLoadTestDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedClose)
	COMMAND_HANDLER(IDCLOSE, BN_CLICKED, OnBnClickedClose)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedClose)
	COMMAND_HANDLER(IDC_TESTTYPECOMBO, CBN_SELCHANGE, OnTestTypeSelect)
	COMMAND_HANDLER(IDC_TESTSTARTBUTTON, BN_CLICKED, OnStartClick)
	COMMAND_HANDLER(IDC_TESTSTOPBUTTON, BN_CLICKED, OnStopClick)
	COMMAND_HANDLER(IDC_CLEARBUTTON, BN_CLICKED, OnClearClick)
	CHAIN_MSG_MAP(CAxDialogImpl<CLoadTestDlg>)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnBnClickedClose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if(IDOK != wID)
			EndDialog(wID);
		return 0;
	}

	LRESULT OnTestTypeSelect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStartClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStopClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClearClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
/// Loads/Saves settings
	boost::shared_ptr<CTestSettings> LoadSettings();
	boost::shared_ptr<CTestSettings> SaveSettings();
/// Handlers for test events
	void OnTestStart();
	void OnTestComplete(std::list<tstring> results);
	void OnInfo(const tstring& info);
/// Enables/disables controls depends on state
	void EnableControls(ETestState state);
};


