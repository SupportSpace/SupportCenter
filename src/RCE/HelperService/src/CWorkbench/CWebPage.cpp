/////////////////////////////////////////////////////////////////////////
///
///  Supportspace Ltd
///
///  CWebPage.cpp
///
///  Workbench application for customer side
///
///  @author "Supportspace Ltd" Anatoly Gutnick @date 19.05.2008
///
////////////////////////////////////////////////////////////////////////
#include "CWebPage.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define CHECK_POINTER(p)\
	ATLASSERT(p != NULL);\
	if(p == NULL)\
	{\
		LogError(_T("NULL pointer"));\
		return false;\
	}

const tstring GetSystemErrorMessage(DWORD dwError)
{
	tstring strError;
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

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
CWebPage::CWebPage()
{
TRY_CATCH
	m_spDoc = NULL;
CATCH_THROW(_T("CWebPage ::CWebPage"))
}
//////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////
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

	m_spDoc = NULL;

	CComPtr<IDispatch> spDisp = pDisp;

	HRESULT hr = spDisp->QueryInterface(IID_IHTMLDocument2,(void**)&m_spDoc);
	if(FAILED(hr))
	{
		LogError(_T("Failed to get HTML document COM object"));
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

void CWebPage::LogError(LPCTSTR lpszText)
{
TRY_CATCH

	m_strError = _T("JSCall Error:\n") + tstring(lpszText);
	Log.Add(_ERROR_,_T("CWebPage::ShowError %s"), m_strError );

CATCH_THROW(_T("CWebPage ::ShowError"))
}

const tstring CWebPage::GetLastError() const
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

bool CWebPage::CallJScript(const tstring& strFuncName, const tstring& strParam1, CComVariant* pVarResult)
{
TRY_CATCH

	CComPtr<IDispatch> spScript;
	if(!GetJScript(spScript))
	{
		LogError(_T("Cannot GetScript"));
		return false;
	}
	CComBSTR bstrMember(strFuncName.c_str());
	DISPID dispid = NULL;
	HRESULT hr = spScript->GetIDsOfNames(IID_NULL,&bstrMember,1,
											LOCALE_SYSTEM_DEFAULT,&dispid);
	if(FAILED(hr))
	{
		LogError(GetSystemErrorMessage(hr).c_str());
		return false;
	}

	DISPPARAMS dispparams;
	memset(&dispparams, 0, sizeof dispparams);
//
	if(strParam1.length()!=0)
	{
		CComBSTR bstrParam1(strParam1.c_str());
		VARIANT vntOpCtrlArg[1];
		ZeroMemory(vntOpCtrlArg, sizeof(VARIANT)*1);
		dispparams.rgvarg = vntOpCtrlArg,
		dispparams.cArgs = 1;
		dispparams.rgvarg[0].vt = VT_BSTR;
		dispparams.rgvarg[0].bstrVal =SysAllocString(bstrParam1);
	}
// 	
	EXCEPINFO excepInfo;
	memset(&excepInfo, 0, sizeof excepInfo);
   	CComVariant vaResult;
	UINT nArgErr = (UINT)-1;  // initialize to invalid arg
         
	hr = spScript->Invoke(dispid,IID_NULL,0,
							DISPATCH_METHOD,&dispparams,&vaResult,&excepInfo,&nArgErr);

	if(dispparams.cArgs == 1)
		SysFreeString(dispparams.rgvarg[0].bstrVal);	
	//delete [] stDispParams.rgvarg;
	if(FAILED(hr))
	{
		LogError(GetSystemErrorMessage(hr).c_str());
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