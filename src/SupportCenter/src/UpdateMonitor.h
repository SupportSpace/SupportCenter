// CUpdateMonitor.h : header file for class that monitor update progress
//
//===========================================================================
// SupportMessengerApp ltd. @{SRCH}
//								CUpdateMonitor
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// SupportMessengerApp :	
//
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 10/29/2006 05:21:10 PM
// Comments: First Issue
//===========================================================================
#pragma once

#define WM_UPDATE_COMPLETED      9001

#include "AidLib\Strings\tstring.h"
#include "stdafx.h"
#include "CVersionReader.h"
#include <msi.h>
#pragma comment(lib, "msi.lib")

// Length of GUID
#define MSI_GUID_LEN 38
#define TOOLS_PRODUCT_CODE			_T("{B359C619-3526-4216-BA49-7022953D0C8E}")

class CUpdateMonitor
{
public:
	CUpdateMonitor();
	~CUpdateMonitor(void);

	void Start(HWND  hWnd, CString		sMinRequiredVer);
	void Stop();
	BOOL IsUpdateRequired(CString		sMinRequiredVer, BOOL bPolling = FALSE);

	BOOL GetVersionFromRegistry(LPTSTR	 sVersionBuf);
	tstring GetProductVersion(const tstring&  productKey, SHORT keyType);

	BOOL IsMonitorRunning();
	void UpdateCompleted();

private:

	void OnUpdateCompleted();

	// CUpdateMonitor will hook OnTimer with CheckUpdate 
	static LRESULT CALLBACK HookWndProc(HWND hwnd,UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Original windows procedure
	static WNDPROC   m_origWndProc;
	// Handle to notification window
	HWND m_hNotifyWnd;
	// Communicator static 
	static CUpdateMonitor* self;

	CString		m_sMinRequiredVer;
	BOOL		m_bStarted;
};
