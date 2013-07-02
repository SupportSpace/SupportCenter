// SupportMessengerApp.h : header file ot the main instance class
//
//===========================================================================
// SupportMessengerApp ltd. @{SRCH}
//								SupportMessengerApp
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

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "Settings.h"			//todo
#include "SupportMessengerDlg.h"
#include "TransparentClassAtom.h"
#include "AppSingelton.h"
#include "afxpriv.h"
#include "custsite.h"
#include "CFileConnectivityQualityLog.h"

//
#define CMD_LINE_PARAM_AUTOSTART		_T("/autostart")
#define CMD_LINE_PARAM_UNINSTALL		_T("/uninstall")
#define CMD_LINE_PARAM_ENABLE_NET_LOG	_T("/netlog")
#define CMD_LINE_PARAM_DISABLE_FILE_LOG	_T("/filelog")
#define CMD_LINE_PARAM_TRACE_LOG		_T("/tracelog")
#define CMD_LINE_PARAM_WORKBENCH_MODE	_T("/workbench")

#define IE_MAJOR_VERSION_REQUIRED	 7

#define WM_UNINSTALL_COMMAND		 WM_USER + 702

// CSupportMessengerApp:
// See SupportMessenger.cpp for the implementation of this class
//

class CSupportMessengerApp : public CWinApp, public CAppSingelton 
{
public:
	CSupportMessengerApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int  ExitInstance(); // return app exit code

	BOOL	LauchWorkbenchProcess(CString sFullUrl, eCallType callType, CString sCustomerDisplayName, long lUid);

private:
	void	SuppressServerBusyDialog();
	long    GetFileSize(const TCHAR *fileName);
	LRESULT GetIEVersion();
	BOOL	BackupLogfile(const TCHAR* logDirectory, const TCHAR* fileName);
	BOOL	GetDiplayTime(FILETIME ftCreate, LPTSTR lpszString);
	LRESULT CheckTeamViewerInstalled();

public:
	CSupportMessengerDlg *		m_pCSupportMessengerDlg;
	CTransparentClassAtom*		m_pcTransparentClassAtom;
	BOOL						m_bAppStartedAutomatiaclly;
	CString						m_sApplicationPath;
	CSettingsIni				m_cSettings; // contains settings for support manager TODO -temporary
	CWorkbenchDlg*				m_pWorkbenchDlg;
	DWORD						m_dwIEMajorVersion;
	CFileConnectivityQualityLog			m_conQualityLog;
	
// Implementation
	DECLARE_MESSAGE_MAP()

};

extern CSupportMessengerApp theApp;