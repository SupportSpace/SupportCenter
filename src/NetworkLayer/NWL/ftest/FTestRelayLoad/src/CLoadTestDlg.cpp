// CLoadTestDlg.cpp : Implementation of CLoadTestDlg

#include "stdafx.h"
#include "CLoadTestDlg.h"
#include "CTestCollection.h"
#include <AidLib/CException/CException.h>
#include "CTestFactory.h"
#include <boost/bind.hpp>


// CLoadTestDlg

LRESULT CLoadTestDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	CAxDialogImpl<CLoadTestDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	m_testsCombo.Attach(GetDlgItem(IDC_TESTTYPECOMBO));

	int defItem = 0;
	ETestType defType = TESTS_INSTANCE.DefaultTest();
	SettingsMap& tests = TESTS_INSTANCE.GetTestSettings();
	SettingsMap::iterator index;
	for(index = tests.begin(); index != tests.end(); ++index)
	{
		int item = m_testsCombo.AddString(index->second->GetName().c_str());
		ETestType type = index->first;
		if(type == defType)
			defItem = item;
		m_testsCombo.SetItemData(item, (DWORD_PTR)type);
	}
	m_testsCombo.SetCurSel(defItem);
	BOOL handled;
	OnTestTypeSelect(0,0,NULL,handled);
	EnableControls(tsStopped);
	bHandled = TRUE;
CATCH_LOG()
	return 1;  // Let the system set the focus
}

LRESULT CLoadTestDlg::OnTestTypeSelect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
TRY_CATCH
	LoadSettings();
CATCH_LOG()
	return 0;
}

boost::shared_ptr<CTestSettings> CLoadTestDlg::LoadSettings()
{
TRY_CATCH
	int item = m_testsCombo.GetCurSel();
	boost::shared_ptr<CTestSettings> settings = TESTS_INSTANCE.GetSettingsById(static_cast<ETestType>(item));
	CEdit(GetDlgItem(IDC_IPADDRESS)).SetWindowText(settings->GetSettings()->GetRelayHost().c_str());
	CEdit(GetDlgItem(IDC_RELAYPORTEDIT)).SetWindowText(i2tstring(settings->GetSettings()->GetRelayPort()).c_str());
	CEdit(GetDlgItem(IDC_USEREDIT)).SetWindowText(settings->GetSettings()->GetUser().c_str());
	CEdit(GetDlgItem(IDC_PASSWDEDIT)).SetWindowText(settings->GetSettings()->GetPassword().c_str());
	CEdit(GetDlgItem(IDC_POOLEDIT)).SetWindowText(i2tstring(settings->GetSettings()->GetPoolSize()).c_str());
	CEdit(GetDlgItem(IDC_CLIENTSEDIT)).SetWindowText(i2tstring(settings->GetSettings()->GetPeersCount()).c_str());
	CEdit(GetDlgItem(IDC_EXTPORTEDIT)).SetWindowText(i2tstring(settings->GetSettings()->GetExtPort()).c_str());
	CEdit(GetDlgItem(IDC_INTPORTEDIT)).SetWindowText(i2tstring(settings->GetSettings()->GetIntPort()).c_str());
	CEdit(GetDlgItem(IDC_BLOCKSIZEEDIT)).SetWindowText(i2tstring(settings->GetSettings()->GetBlockSize()).c_str());
	CEdit(GetDlgItem(IDC_BLOCKSCOUNTEDIT)).SetWindowText(i2tstring(settings->GetSettings()->GetBlocksCount()).c_str());
	return settings;
CATCH_THROW()
}

boost::shared_ptr<CTestSettings> CLoadTestDlg::SaveSettings()
{
TRY_CATCH
	int item = m_testsCombo.GetCurSel();
	boost::shared_ptr<CTestSettings> settings = TESTS_INSTANCE.GetSettingsById(static_cast<ETestType>(item));

	TCHAR buf[TEXT_BUF_SIZE];
	memset(buf,0,TEXT_BUF_SIZE);
	CEdit(GetDlgItem(IDC_IPADDRESS)).GetWindowText(buf, TEXT_BUF_SIZE);
	settings->GetSettings()->SetRelayHost(buf);

	memset(buf,0,TEXT_BUF_SIZE);
	CEdit(GetDlgItem(IDC_RELAYPORTEDIT)).GetWindowText(buf, TEXT_BUF_SIZE);
	settings->GetSettings()->SetRelayPort(_ttoi(buf));

	memset(buf,0,TEXT_BUF_SIZE);
	CEdit(GetDlgItem(IDC_USEREDIT)).GetWindowText(buf, TEXT_BUF_SIZE);
	settings->GetSettings()->SetUser(buf);

	memset(buf,0,TEXT_BUF_SIZE);
	CEdit(GetDlgItem(IDC_PASSWDEDIT)).GetWindowText(buf, TEXT_BUF_SIZE);
	settings->GetSettings()->SetPassword(buf);

	memset(buf,0,TEXT_BUF_SIZE);
	CEdit(GetDlgItem(IDC_POOLEDIT)).GetWindowText(buf, TEXT_BUF_SIZE);
	settings->GetSettings()->SetPoolSize(_ttoi(buf));

	memset(buf,0,TEXT_BUF_SIZE);
	CEdit(GetDlgItem(IDC_CLIENTSEDIT)).GetWindowText(buf, TEXT_BUF_SIZE);
	settings->GetSettings()->SetPeersCount(_ttoi(buf));

	memset(buf,0,TEXT_BUF_SIZE);
	CEdit(GetDlgItem(IDC_EXTPORTEDIT)).GetWindowText(buf, TEXT_BUF_SIZE);
	settings->GetSettings()->SetExtPort(_ttoi(buf));

	memset(buf,0,TEXT_BUF_SIZE);
	CEdit(GetDlgItem(IDC_INTPORTEDIT)).GetWindowText(buf, TEXT_BUF_SIZE);
	settings->GetSettings()->SetIntPort(_ttoi(buf));

	memset(buf,0,TEXT_BUF_SIZE);
	CEdit(GetDlgItem(IDC_BLOCKSIZEEDIT)).GetWindowText(buf, TEXT_BUF_SIZE);
	settings->GetSettings()->SetBlockSize(_ttoi(buf));

	memset(buf,0,TEXT_BUF_SIZE);
	CEdit(GetDlgItem(IDC_BLOCKSCOUNTEDIT)).GetWindowText(buf, TEXT_BUF_SIZE);
	settings->GetSettings()->SetBlocksCount(_ttoi(buf));

	return settings;
CATCH_THROW()
}


LRESULT CLoadTestDlg::OnStartClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
TRY_CATCH
	/// Create test
	m_test = TESTFACTORY_INSTANCE.CreateTest(SaveSettings());
	/// Setup callbacks
	m_test->SetEvents(boost::bind(&CLoadTestDlg::OnTestStart, this), 
		boost::bind(&CLoadTestDlg::OnTestComplete, this, _1),
		boost::bind(&CLoadTestDlg::OnInfo, this, _1));
	/// Start test
	m_test->Start();
CATCH_LOG()
	return 0;
}

void CLoadTestDlg::OnTestStart()
{
TRY_CATCH
	/// Change controls state
	EnableControls(tsRunning);
CATCH_THROW()
}

void CLoadTestDlg::OnTestComplete(std::list<tstring> results)
{
TRY_CATCH
	/// Change controls state
	EnableControls(tsStopped);
	/// Display results
	CListBox(GetDlgItem(IDC_RESULTLIST)).ResetContent();
	std::list<tstring>::iterator index;
	for(index = results.begin(); index != results.end(); ++index)
	{
		tstring value = *index;
		CListBox(GetDlgItem(IDC_RESULTLIST)).AddString(value.c_str());
	}
CATCH_THROW()
}

void CLoadTestDlg::EnableControls(ETestState state)
{
TRY_CATCH
	CEdit(GetDlgItem(IDC_IPADDRESS)).EnableWindow(tsStopped == state);
	CEdit(GetDlgItem(IDC_RELAYPORTEDIT)).EnableWindow(tsStopped == state);
	CEdit(GetDlgItem(IDC_USEREDIT)).EnableWindow(tsStopped == state);
	CEdit(GetDlgItem(IDC_PASSWDEDIT)).EnableWindow(tsStopped == state);
	CEdit(GetDlgItem(IDC_POOLEDIT)).EnableWindow(tsStopped == state);
	CEdit(GetDlgItem(IDC_CLIENTSEDIT)).EnableWindow(tsStopped == state);
	CEdit(GetDlgItem(IDC_EXTPORTEDIT)).EnableWindow(tsStopped == state);
	CEdit(GetDlgItem(IDC_INTPORTEDIT)).EnableWindow(tsStopped == state);
	CEdit(GetDlgItem(IDC_BLOCKSIZEEDIT)).EnableWindow(tsStopped == state);
	CEdit(GetDlgItem(IDC_BLOCKSCOUNTEDIT)).EnableWindow(tsStopped == state);

	CComboBox(GetDlgItem(IDC_TESTTYPECOMBO)).EnableWindow(tsStopped == state);
	CButton(GetDlgItem(IDC_TESTSTARTBUTTON)).EnableWindow(tsStopped == state);
	CButton(GetDlgItem(IDC_TESTSTOPBUTTON)).EnableWindow(tsRunning == state);
CATCH_THROW()
}


LRESULT CLoadTestDlg::OnStopClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
TRY_CATCH
	if(m_test.get())
		m_test->Stop();
CATCH_LOG()
	return 0;
}

LRESULT CLoadTestDlg::OnClearClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
TRY_CATCH
	CListBox(GetDlgItem(IDC_RESULTLIST)).ResetContent();
	CListBox(GetDlgItem(IDC_OUTLIST)).ResetContent();
CATCH_LOG()
	return 0;
}

void CLoadTestDlg::OnInfo(const tstring& info)
{
TRY_CATCH
	CListBox(GetDlgItem(IDC_OUTLIST)).InsertString(0, info.c_str());
CATCH_THROW()
}

