// CWebPage.h : header file for interface to JS
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

#if !defined(AFX_WEBPAGE_H__AEBD50B8_EE66_40AB_8B92_C4EECB9BCD22__INCLUDED_)
#define AFX_WEBPAGE_H__AEBD50B8_EE66_40AB_8B92_C4EECB9BCD22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlbase.h>
#include <Mshtml.h>

class CWebPage  
{
public:
	CWebPage();
	virtual ~CWebPage();

	bool			SetDocument(IDispatch* pDisp);
	LPDISPATCH		GetHtmlDocument() const;
	const CString	GetLastError() const;
	bool			GetJScript(CComPtr<IDispatch>& spDisp);
	bool			GetJScripts(CComPtr<IHTMLElementCollection>& spColl);
	CString			ScanJScript(CString& strAText, CStringArray& args);
	bool			CallJScript(const CString strFunc,const CStringArray& paramArray,CComVariant* pVarResult = NULL);

	void			CallJScriptByInvokeHelper(const CString strFunc, const CString sType = _T(""), const CString sData = _T(""), 
		const CString arg1 = _T(""), const CString arg2 = _T(""), const CString arg3 = _T(""));

protected:
	void			ShowError(LPCTSTR lpszText);

protected:
	CComPtr<IHTMLDocument2>	m_spDoc;
	CString			m_strError;
};

#endif // !defined(AFX_WEBPAGE_H__AEBD50B8_EE66_40AB_8B92_C4EECB9BCD22__INCLUDED_)
