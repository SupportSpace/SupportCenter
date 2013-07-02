// CJabberTestDlg.h : Declaration of the CJabberTestDlg

#pragma once


#include "resource.h"       // main symbols
#include <atlhost.h>
#include <AidLib/CException/CException.h>
#include <wtl/atlapp.h>
#include <wtl/atlddx.h>
#include <wtl/atlctrls.h>
#include "STestParams.h"
#include "SConnectParams.h"
#include <boost/shared_ptr.hpp>
#include "CAbstractTest.h"
// CJabberTestDlg

class CJabberTestDlg : 
	public CAxDialogImpl<CJabberTestDlg>,
	public cLog
{
protected:
	CListBox m_logCtrl;
protected:
BEGIN_MSG_MAP(CJabberTestDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	//COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	COMMAND_HANDLER(IDC_START, BN_CLICKED, OnBnClickedStart)
	COMMAND_HANDLER(IDC_CLEAR, BN_CLICKED, OnBnClickedClear)
	CHAIN_MSG_MAP(CAxDialogImpl<CJabberTestDlg>)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	/// A virtual method that prepare message string
	virtual void AddList(const cEventDesc &EventDesc, const TCHAR *Item, ...)throw( );
	virtual void AddString(const TCHAR* LogString, const eSeverity Severity)throw(){}

public:
	enum { IDD = IDD_JABBERTESTDLG };
	CJabberTestDlg();
	~CJabberTestDlg();
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	STestParams m_testParams;
	SConnectParams m_connectParams;
	void ToGUI(void);
	void FromGUI(void);
public:
	tstring& GetDlgItemText(int nID,tstring& text);
	boost::shared_ptr<CAbstractTest> m_test;
	bool m_testActive;
public:
	void OnTestComplete(bool result);
public:
	LRESULT OnBnClickedClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};


