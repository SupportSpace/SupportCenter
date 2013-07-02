// CJabberTestDlg.cpp : Implementation of CJabberTestDlg

#include "stdafx.h"
#include "CJabberTestDlg.h"
#include <boost/shared_array.hpp>
#include "CSyncTest.h"
#include <boost/bind.hpp>
// CJabberTestDlg

CJabberTestDlg::CJabberTestDlg():
	m_testActive(false)
{
TRY_CATCH
CATCH_THROW()
}

CJabberTestDlg::~CJabberTestDlg()
{
TRY_CATCH
	
CATCH_LOG()
}

void CJabberTestDlg::AddList(const cLog::cEventDesc &EventDesc, const TCHAR *Item, ...) throw( )
{
	try
	{
		// Exit if verbosity level is _NO_TRACE_ of high than defined level
		if( (_NO_TRACE_ == EventDesc.getVerbosity()) || (EventDesc.getVerbosity() >= _TRACE_CALLS_) )
			return;
		SYSTEMTIME SysTime;
		GetLocalTime(&SysTime);
		TCHAR Buf[MAX_PATH];
		tstring TimeStr;
		if(!GetTimeFormat(LOCALE_USER_DEFAULT,0,&SysTime,NULL,Buf,MAX_PATH))
			TimeStr += _T("Invalid time");
		else
			TimeStr += Buf;
		TimeStr += "> ";

		va_list vl;
		tstring str;
		for(va_start(vl, Item); Item; Item=va_arg(vl, PTCHAR))
		{
			str += Item;
		}
		va_end(vl);

		switch(EventDesc.getSeverity())
			{	
				case 0: TimeStr+=_T("_MES ");break;
				case 1: TimeStr+=_T("_WAR ");break;
				case 2: TimeStr+=_T("_ERR ");break;
				case 3: TimeStr+=_T("_EXC ");break;
				case 4: TimeStr+=_T("_UTS ");break;
				case 5: TimeStr+=_T("_UTC ");break;
				case 6: TimeStr+=_T("_FTS ");break;
				case 7: TimeStr+=_T("_FTC ");break;
				default:TimeStr+=_T("unkn ");
			}
		
		TimeStr += str;
		TimeStr+=_T(" ");
		if (EventDesc.getCallStack())
			TimeStr+=EventDesc.getCallStack();//call stack

		//TODO dead lock if main (GUI)thread blocked.
		m_logCtrl.InsertString(0,TimeStr.c_str());
	}
	catch(...)
	{
	}
}

LRESULT CJabberTestDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	CAxDialogImpl<CJabberTestDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	m_logCtrl.Attach(GetDlgItem(IDC_LOGCONROL));
	m_ownLiveTime = true; //cLog member
	Log.RegisterLog(this);
	ToGUI();
	bHandled = TRUE;
	return 1;  // Let the system set the focus
CATCH_THROW()
}

LRESULT CJabberTestDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	Log.UnRegisterLog(this);
	return 0;
}

LRESULT CJabberTestDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
TRY_CATCH
	//EndDialog(wID);
	return 0;
CATCH_THROW()
}

LRESULT CJabberTestDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
TRY_CATCH
	EndDialog(wID);
	return 0;
CATCH_THROW()
}



LRESULT CJabberTestDlg::OnBnClickedStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
TRY_CATCH
	if(m_testActive)
	{
		m_test->Stop();
	}
	else
	{
		FromGUI();
		ToGUI();
		m_test.reset(new CSyncTest());
		m_test->SetOnCompleteEvent(boost::bind(&CJabberTestDlg::OnTestComplete, this, _1));
		m_test->Init(m_testParams, m_connectParams);
		SetDlgItemText(IDC_START,_T("Stop"));
		m_testActive=true;
		m_test->Start();
	}
CATCH_LOG()
	return 0;
}


void CJabberTestDlg::ToGUI(void)
{
TRY_CATCH
	CButton(GetDlgItem(IDC_TESTSERVER)).SetCheck((m_testParams.m_server)?BST_CHECKED:BST_UNCHECKED);
	CComboBox(GetDlgItem(IDC_TESTCLIENTTYPE)).SetCurSel(m_testParams.m_clientType);
	CEdit(GetDlgItem(IDC_TESTMSGCOUNT)).SetWindowText(i2tstring(m_testParams.m_msgCount).c_str());
	CEdit(GetDlgItem(IDC_TESTSENDDELAY)).SetWindowText(i2tstring(m_testParams.m_sendDelay).c_str());
	CEdit(GetDlgItem(IDC_TESTWAITTIMEOUT)).SetWindowText(i2tstring(m_testParams.m_waitTimeout).c_str());
	CEdit(GetDlgItem(IDC_TESTBULKSIZE)).SetWindowText(i2tstring(m_testParams.m_bulkSize).c_str());

	CEdit(GetDlgItem(IDC_USERNAME)).SetWindowText(m_connectParams.m_userName.c_str());
	CEdit(GetDlgItem(IDC_USERPASSWD)).SetWindowText(m_connectParams.m_userPasswd.c_str());
	CEdit(GetDlgItem(IDC_REMOTEUSERNAME)).SetWindowText(m_connectParams.m_remoteUserName.c_str());
	CEdit(GetDlgItem(IDC_SERVERADDR)).SetWindowText(m_connectParams.m_serverAddr.c_str());
	CEdit(GetDlgItem(IDC_SERVERPORT)).SetWindowText(i2tstring(m_connectParams.m_serverPort).c_str());
	CEdit(GetDlgItem(IDC_SERVERNAME)).SetWindowText(m_connectParams.m_serverName.c_str());
	CEdit(GetDlgItem(IDC_RESOURCE)).SetWindowText(m_connectParams.m_resource.c_str());
	CEdit(GetDlgItem(IDC_CONNECTTIMEOUT)).SetWindowText(i2tstring(m_connectParams.m_connectTimeout).c_str());

CATCH_LOG()
}

void CJabberTestDlg::FromGUI(void)
{
	//TODO check values
TRY_CATCH
	m_testParams.m_server=BST_CHECKED==CButton(GetDlgItem(IDC_TESTSERVER)).GetCheck();
	m_testParams.m_clientType=static_cast<EClientType>(CComboBox(GetDlgItem(IDC_TESTCLIENTTYPE)).GetCurSel());
	m_testParams.m_msgCount=GetDlgItemInt(IDC_TESTMSGCOUNT);
	m_testParams.m_sendDelay=GetDlgItemInt(IDC_TESTSENDDELAY);
	m_testParams.m_waitTimeout=GetDlgItemInt(IDC_TESTWAITTIMEOUT);
	m_testParams.m_bulkSize=GetDlgItemInt(IDC_TESTBULKSIZE);

	m_connectParams.m_userName=GetDlgItemText(IDC_USERNAME,tstring());
	m_connectParams.m_userPasswd=GetDlgItemText(IDC_USERPASSWD,tstring());
	m_connectParams.m_remoteUserName=GetDlgItemText(IDC_REMOTEUSERNAME,tstring());
	m_connectParams.m_serverAddr=GetDlgItemText(IDC_SERVERADDR,tstring());
	m_connectParams.m_serverPort=GetDlgItemInt(IDC_SERVERPORT);
	m_connectParams.m_serverName=GetDlgItemText(IDC_SERVERNAME,tstring());
	m_connectParams.m_resource=GetDlgItemText(IDC_RESOURCE,tstring());
	m_connectParams.m_connectTimeout=GetDlgItemInt(IDC_CONNECTTIMEOUT);
CATCH_LOG()
}

tstring& CJabberTestDlg::GetDlgItemText(int nID,tstring& text)
{
TRY_CATCH
	//TODO check nID
	int strLen=GetDlgItem(nID).GetWindowTextLength();
	strLen+=1;
	boost::shared_array<TCHAR> str(new TCHAR[strLen]);
	CAxDialogImpl<CJabberTestDlg>::GetDlgItemText(nID,str.get(),strLen);
	text=str.get();
	return text;
CATCH_THROW()
}

void CJabberTestDlg::OnTestComplete(bool result)
{
TRY_CATCH
	SetDlgItemText(IDC_START,_T("Start"));
	m_testActive=false;
	if(result)
		Log.Add(_MESSAGE_,_T("-------------------- Test Passed ----------------------"));
	else
		Log.Add(_MESSAGE_,_T("-------------------- Test Failed ----------------------"));
CATCH_LOG()
}

LRESULT CJabberTestDlg::OnBnClickedClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
TRY_CATCH
	m_logCtrl.ResetContent();
	return 0;
CATCH_THROW()
}
