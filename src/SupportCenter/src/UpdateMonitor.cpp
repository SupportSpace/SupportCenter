#include "StdAfx.h"
#include "UpdateMonitor.h"
#include "AidLib/CException/CException.h"
#include <AidLib/Logging/cLog.h> 
#include "LocalDataDef.h"

WNDPROC CUpdateMonitor::m_origWndProc = NULL;
CUpdateMonitor* CUpdateMonitor::self = NULL;

#define UPDATE_POLLING_TIMER_ID  51000
#define UPDATE_POLLING_INTERVAL  5000

CUpdateMonitor::CUpdateMonitor()
{
	self = this;
	m_hNotifyWnd = NULL;
	m_bStarted = FALSE;
}

CUpdateMonitor::~CUpdateMonitor(void)
{
}

BOOL CUpdateMonitor::IsMonitorRunning()
{
TRY_CATCH
CATCH_LOG(_T("CUpdateMonitor::IsMonotirRunning"))
	return m_bStarted;
}

void CUpdateMonitor::Start(HWND  hWnd, CString sMinRequiredVersion)
{
TRY_CATCH
	if(m_bStarted==TRUE)
		Stop();	//todo critical section here
	
	m_bStarted = TRUE;
	
	m_sMinRequiredVer = sMinRequiredVersion;
	m_hNotifyWnd = hWnd;
	m_origWndProc = (WNDPROC)SetWindowLong(m_hNotifyWnd, GWL_WNDPROC, (LONG)CUpdateMonitor::HookWndProc);
	::SetTimer(m_hNotifyWnd, UPDATE_POLLING_TIMER_ID, UPDATE_POLLING_INTERVAL, NULL);
	

CATCH_LOG(_T("CUpdateMonitor::Start"))
}

void CUpdateMonitor::Stop()
{
TRY_CATCH

	::KillTimer(m_hNotifyWnd, UPDATE_POLLING_TIMER_ID);
	if(m_origWndProc!=NULL)
	{
		SetWindowLong(m_hNotifyWnd, GWL_WNDPROC, (LONG)m_origWndProc);
		m_origWndProc = NULL;
	}

	m_bStarted = FALSE;

CATCH_LOG(_T("CUpdateMonitor::Stop"))
}

void CUpdateMonitor::UpdateCompleted()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CUpdateMonitor::UpdateCompleted"));
	OnUpdateCompleted();
	
CATCH_LOG(_T("CUpdateMonitor::UpdateCompleted"))
}

void CUpdateMonitor::OnUpdateCompleted()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CUpdateMonitor::OnUpdateCompleted"));
	
	if(m_hNotifyWnd)	
		PostMessage(m_hNotifyWnd, WM_UPDATE_COMPLETED, 0, (LPARAM)0); 
	
	Stop();

CATCH_LOG(_T("CUpdateMonitor::OnUpdateCompleted"))
}

BOOL CUpdateMonitor::IsUpdateRequired(CString sMinRequiredVersion, BOOL bPolling)
{
	/*
TRY_CATCH
	
	// todo verbosity 
	Log.Add(_CALL_,_T("CUpdateMonitor::IsUpdateRequired"));
	TCHAR	 sInstalledVerr[MAX_BIGBUF_LEN] = { 0 };
	
	//
	//	Retrieve product version installed
	//

	tstring	  instProductVersion = GetProductVersion(TOOLS_PRODUCT_CODE, CVersionReader::SK_UPGRAGE_CODE);
	if(instProductVersion.compare(_T("")) == 0)
	{
		//todo show error?
		Log.Add(_MESSAGE_,_T("CUpdateMonitor::IsUpdateRequired GetProductVersion failed...Try to update "));
		return TRUE;
	}
	//
	//	Compare with sMinRequiredVersion - simples way to compare for now
	//
	if(sMinRequiredVersion.Compare(instProductVersion.c_str())==0)
	{
		Log.Add(_MESSAGE_,_T("CUpdateMonitor::IsUpdateRequired Update not required!"));
		return FALSE;
	}
	else
	{
		//todo verbosity _TRACE_CALLS_
		if(bPolling==FALSE)
			Log.Add(_MESSAGE_,_T("CUpdateMonitor::IsUpdateRequired Update required. Installed version: %s"), instProductVersion.c_str());

		return TRUE;
	}

CATCH_LOG(_T("CUpdateMonitor::IsUpdateRequired"))
*/
	return FALSE; //No update. TODO Vadim - uncomment this section	
}

LRESULT CALLBACK CUpdateMonitor::HookWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
TRY_CATCH

	if(uMsg==WM_TIMER && (UINT_PTR)wParam == UPDATE_POLLING_TIMER_ID)
	{
		// todo verbosity
		Log.Add(_CALL_,_T("CUpdateMonitor::HookWndProc UPDATE_POLLING_TIMER_ID"));

		if(self->IsUpdateRequired(self->m_sMinRequiredVer, TRUE)==FALSE)
			self->OnUpdateCompleted();

		return 0;
	}

CATCH_LOG(_T("CUpdateMonitor::HookWndProc"))

	return CallWindowProc(m_origWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL CUpdateMonitor::GetVersionFromRegistry(LPTSTR	 sVersionBuf)
{
	CRegKey  m_regKey;
	HRESULT	 lResult = S_OK;
	ULONG	 nBytes = 0;
	
	TCHAR	szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };
	_stprintf_s(szCurrentUserPath, MAX_REGBUF_LEN, _T("%s"), szBaseRegistryPath);//todo

	if( m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS )
		return FALSE;

	nBytes = sizeof(sVersionBuf);
	lResult = m_regKey.QueryStringValue(sVersion, sVersionBuf, &nBytes);
	
	m_regKey.Close();
	return TRUE;
}

tstring CUpdateMonitor::GetProductVersion(const tstring& productUpgradeCode, SHORT keyType)
{
	tstring version;
	tstring error(_T("CUpdateMonitor::GetProductVersion failed: "));
	try
	{
		CVersionReader versionReader(productUpgradeCode, static_cast<CVersionReader::ESearchKeyType>(keyType));
		version = versionReader.GetProductVersion();
		return version;
	}
	catch(tstring& errorString)
	{
		error += errorString;
	}
	catch(std::exception exception)
	{
		error += exception.what();//todo for unicode
	}
	catch(...)
	{
		error = _T("Unknown");
	}

	Log.Add(_ERROR_, _T("%s"), error);
	return _T("");
}

