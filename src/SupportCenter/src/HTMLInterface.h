// HTMLInterface.h : header file
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CHTMLInterface
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CHTMLInterface :	provide interface to JavaScript functions via interface.js
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

#include "WebPage.h"

class CHTMLInterface
{
public:
	CHTMLInterface();
	~CHTMLInterface();
public:

	void	SetDocument(CWebPage	webPage);
	HRESULT INTERFACE_UpdateData(CString sType, CString sData, CString sArg1 = _T(""), CString sArg2  = _T(""), CString sArg3 = _T(""));

private:
	CWebPage	m_webPage;
};


