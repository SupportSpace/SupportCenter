// CWebPage.h : header file for interface to JS
//
//===========================================================================
// SupportMessengerApp ltd. @{SRCH}
//								CWebPage
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// Workbench application for customer side	
//
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 19/05/2008 05:21:10 PM
// Comments: First Issue
//===========================================================================

#if !defined(AFX_WEBPAGE_H__AEBD50B8_EE66_40AB_8B92_C4EECB9BCD22__INCLUDED_)
#define AFX_WEBPAGE_H__AEBD50B8_EE66_40AB_8B92_C4EECB9BCD22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlbase.h>
#include <Mshtml.h>
#include <AidLib/Strings/tstring.h>

class CWebPage  
{
public:
	CWebPage();
	virtual ~CWebPage();

	/// call javascript function with parameter from the page   
	/// @param funcName - name of function defined in the page
	/// @param strParam1 - name of parameter for function call 
	bool			CallJScript(const tstring& strFuncName, const tstring& strParam1, CComVariant* pVarResult);
	/// set document of the page may be called before CallJScript
	/// @param pDisp - 
	bool			SetDocument(IDispatch* pDisp);
protected:
	LPDISPATCH		GetHtmlDocument() const;
	const tstring	GetLastError() const;
	bool			GetJScript(CComPtr<IDispatch>& spDisp);
	void			LogError(LPCTSTR lpszText);

protected:
	CComPtr<IHTMLDocument2>	m_spDoc;
	tstring			m_strError;
};

#endif // !defined(AFX_WEBPAGE_H__AEBD50B8_EE66_40AB_8B92_C4EECB9BCD22__INCLUDED_)
