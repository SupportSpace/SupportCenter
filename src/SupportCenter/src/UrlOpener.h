// CUrlOpener.h : header file for interface to Internet Explorer opener
//
//===========================================================================
// SupportMessengerApp ltd. @{SRCH}
//								CWebPage
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

//===========================================================================
// @{CSEH}
//								CCoInitialize
//
//---------------------------------------------------------------------------
// Description    : Helper object handling initialization of the COM library.
//===========================================================================
class CCoInitialize
{
public:
	CCoInitialize()		{ CoInitialize(NULL); }
	~CCoInitialize()	{ CoUninitialize(); }
};

class CUrlOpener
{
public:
	CUrlOpener();

	~CUrlOpener(){};

	void    Open(LPCTSTR lpszURL, bool bNewWindow = true);
	BOOL	LauchIEBrowser(CString csUrlPage);

private:
	LPCTSTR GetBrowser(void);

private:
	// The default browser
	CString m_strBrowser;
	HWND	m_hwndExpertDesktop;
};

