#include "stdafx.h"
#include "WebPage.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"
#include "UnicodeConvert.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define CHECK_POINTER(p)\
	ATLASSERT(p != NULL);\
	if(p == NULL)\
	{\
		ShowError(_T("NULL pointer"));\
		return false;\
	}

const CString GetSystemErrorMessage(DWORD dwError)
{
	CString strError;
	LPTSTR lpBuffer;

	if(!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,  dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
			(LPTSTR) &lpBuffer, 0, NULL))

	{
		strError = "FormatMessage Netive Error" ;
	}
	else
	{
		strError = lpBuffer;
		LocalFree(lpBuffer);
	}
	return strError;
}

CString GetNextToken(CString& strSrc, const CString strDelim,bool bTrim =false, bool bFindOneOf=true)
{
TRY_CATCH

	CString strToken;
	int idx = bFindOneOf? strSrc.FindOneOf(strDelim) : strSrc.Find(strDelim);
	if(idx != -1)
	{
		strToken  = strSrc.Left(idx);
		strSrc = strSrc.Right(strSrc.GetLength() - (idx + 1) );
	}
	else
	{
		strToken = strSrc;
		strSrc.Empty();
	}
	if(bTrim)
	{
		strToken.TrimLeft();
		strToken.TrimRight();
	}
	return strToken;
CATCH_THROW(_T("GetNextToken"))
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CWebPage::CWebPage()
{
TRY_CATCH
	m_spDoc = NULL;
CATCH_THROW(_T("CWebPage ::CWebPage"))
}

CWebPage::~CWebPage()
{
TRY_CATCH
	if(m_spDoc)
		m_spDoc.Release();
CATCH_LOG(_T("CWebPage ::~CWebPage"))
}

bool CWebPage::SetDocument(IDispatch* pDisp)
{
	bool	RetVal = false;
TRY_CATCH

	CHECK_POINTER(pDisp);

	if (m_spDoc != NULL)
	{
		Log.Add (_WARNING_, _T("m_spDoc is not NULL!"));
	}

	m_spDoc = NULL;

	CComPtr<IDispatch> spDisp = pDisp;

	HRESULT hr = spDisp->QueryInterface(IID_IHTMLDocument2,(void**)&m_spDoc);

	if(FAILED(hr))
	{
		ShowError(_T("Failed to get HTML document COM object"));
		RetVal =false;
	}
	else
	{
		RetVal =true;
	}

CATCH_THROW(_T("CWebPage ::SetDocument"))
	return RetVal;
}

bool CWebPage::GetJScript(CComPtr<IDispatch>& spDisp)
{
	bool	RetVal = false;
TRY_CATCH

	CHECK_POINTER(m_spDoc);
	HRESULT hr = m_spDoc->get_Script(&spDisp);
	ATLASSERT(SUCCEEDED(hr));
	RetVal = SUCCEEDED(hr);

CATCH_THROW(_T("CWebPage ::GetJScript"))
	return RetVal;
}

bool CWebPage::GetJScripts(CComPtr<IHTMLElementCollection>& spColl)
{
	bool	RetVal = false;
TRY_CATCH

	CHECK_POINTER(m_spDoc);
	HRESULT hr = m_spDoc->get_scripts(&spColl);
	ATLASSERT(SUCCEEDED(hr));
	RetVal = SUCCEEDED(hr);

CATCH_THROW(_T("CWebPage ::GetJScripts"))
	return RetVal;
}

// returned java script function name, input string is truncating
CString CWebPage::ScanJScript(CString& strAText, CStringArray& args)
{
TRY_CATCH

	args.RemoveAll();
	CString strDelim(" \n\r\t"),strSrc(strAText);
	bool bFound = false;
	while(!strSrc.IsEmpty())
	{
		CString strStart = GetNextToken(strSrc,strDelim);
		if(strStart == "function")
		{
			bFound = true;
			break;
		}
		if(strStart == "/*")
		{
			// Skip comments
			while(!strSrc.IsEmpty())
			{
				CString strStop = GetNextToken(strSrc,strDelim);
				if(strStop == "*/")
				{
					break;
				}
			}
		}
	}

	if(!bFound)
		return _T("");
	
	CString strFunc = GetNextToken(strSrc,_T("("),true);
	CString strArgs = GetNextToken(strSrc,_T(")"),true);

	// Parse arguments
	CString strArg;
	while(!(strArg = GetNextToken(strArgs,_T(","))).IsEmpty())
		args.Add(strArg);

	strAText= strSrc;
	return strFunc;

CATCH_THROW(_T("CWebPage ::ScanJScript"))
}

void CWebPage::ShowError(LPCTSTR lpszText)
{
TRY_CATCH

	m_strError = _T("JSCall Error:\n") + CString(lpszText);
	Log.Add(_ERROR_,_T("CWebPage::ShowError %s"), m_strError );

CATCH_THROW(_T("CWebPage ::ShowError"))
}

const CString CWebPage::GetLastError() const
{
TRY_CATCH
CATCH_THROW(_T("CWebPage ::GetLastError"))
	return m_strError;
}

LPDISPATCH CWebPage::GetHtmlDocument() const
{
TRY_CATCH
CATCH_THROW(_T("CWebPage ::GetHtmlDocument"))
	return m_spDoc;
}

bool CWebPage::CallJScript(const CString strFunc, const CStringArray& paramArray,CComVariant* pVarResult)
{
TRY_CATCH

	CComPtr<IDispatch> spScript;
	if(!GetJScript(spScript))
	{
		ShowError(_T("Cannot GetScript"));
		return false;
	}
	CComBSTR bstrMember(strFunc);
	DISPID dispid = NULL;
	HRESULT hr = spScript->GetIDsOfNames(IID_NULL,&bstrMember,1,
											LOCALE_SYSTEM_DEFAULT,&dispid);
	if(FAILED(hr))
	{
		ShowError(GetSystemErrorMessage(hr));
		return false;
	}

	const int arraySize = paramArray.GetSize();

	DISPPARAMS dispparams;
	memset(&dispparams, 0, sizeof dispparams);
	dispparams.cArgs = arraySize;
	dispparams.rgvarg = new VARIANT[dispparams.cArgs];
	
	for( int i = 0; i < arraySize; i++)
	{
		CString	str = paramArray.GetAt(arraySize - 1 - i);
		CComBSTR bstr = FromUtf8ToUtf16(str.GetBuffer()).c_str();//todo for unicode
		//CComBSTR bstr = paramArray.GetAt(arraySize - 1 - i); // back reading
		bstr.CopyTo(&dispparams.rgvarg[i].bstrVal);
		dispparams.rgvarg[i].vt = VT_BSTR;
	}
	dispparams.cNamedArgs = 0;

	EXCEPINFO excepInfo;
	memset(&excepInfo, 0, sizeof excepInfo);
   	CComVariant vaResult;
	UINT nArgErr = (UINT)-1;  // initialize to invalid arg
         
	hr = spScript->Invoke(dispid,IID_NULL,0,
							DISPATCH_METHOD,&dispparams,&vaResult,&excepInfo,&nArgErr);

	/////////////////// bug fix memory leak code start ///////////////
	for( int j = 0; j < arraySize; j++)
	::SysFreeString(dispparams.rgvarg[j].bstrVal);
	//bstr_t _bstrVal(dispparams.rgvarg[j].bstrVal,FALSE); 
	/////////////////// bug fix memory leak code end ///////////////
	
	delete [] dispparams.rgvarg;
	if(FAILED(hr))
	{
		ShowError(GetSystemErrorMessage(hr));
		return false;
	}
	
	if(pVarResult)
	{
		*pVarResult = vaResult;
	}

	if(spScript)
		spScript.Release();

	return true;

CATCH_THROW(_T("CWebPage ::CallJScript"))
}
//	
//	todo - leave alternative call to JavaScript
//	the bug was found in JS - not delete calls from list of calls
//	NEED #import "C:\WINDOWS\system32\mshtml.tlb" // location of mshtml.tlb for IDispatchPtr
/*
void	CWebPage::CallJScriptByInvokeHelper(
		const CString strFunc, 
		const CString sType,
		const CString sData, 
		const CString sArg1, 
		const CString sArg2, 
		const CString sArg3)
{
TRY_CATCH

	if(m_spDoc)
	{
		IDispatchPtr spDisp;
		m_spDoc->get_Script(&spDisp);
		if (spDisp)
		{
			// Evaluate is the name of the script function.
			CComBSTR bstrMember(strFunc);
			DISPID dispid;

			HRESULT hr = spDisp->GetIDsOfNames(IID_NULL, &bstrMember, 1, LOCALE_SYSTEM_DEFAULT, &dispid);

			if (SUCCEEDED(hr))
			{
				COleVariant vtResult;
				static BYTE parms[] = VTS_WBSTR VTS_WBSTR VTS_WBSTR VTS_WBSTR VTS_WBSTR;
				//static BYTE parms[] = VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR;
		
				COleDispatchDriver dispDriver(spDisp, FALSE);

				CComBSTR  bstrType = FromUtf8ToUtf16(((CString)sType).GetBuffer()).c_str();//todo for unicode;
				CComBSTR  bstrData = FromUtf8ToUtf16(((CString)sData).GetBuffer()).c_str();//todo for unicode;
				CComBSTR  bstrArg1 = FromUtf8ToUtf16(((CString)sArg1).GetBuffer()).c_str();//todo for unicode;
				CComBSTR  bstrArg2 = FromUtf8ToUtf16(((CString)sArg2).GetBuffer()).c_str();//todo for unicode;
				CComBSTR  bstrArg3 = FromUtf8ToUtf16(((CString)sArg3).GetBuffer()).c_str();//todo for unicode;

				dispDriver.InvokeHelper(dispid, DISPATCH_METHOD, VT_EMPTY,
						NULL, parms, 
						bstrType, 
						bstrData,
						bstrArg1,
						bstrArg2,
						bstrArg3);

				dispDriver.ReleaseDispatch();
			}
		}
	}

CATCH_THROW(_T("CWebPage ::CallJScript"))
}
*/