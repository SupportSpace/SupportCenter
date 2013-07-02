#include "stdafx.h"
#include "HTMLInterface.h"

#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"

CHTMLInterface::CHTMLInterface()
{
	TRY_CATCH
	CATCH_THROW(_T("CHTMLInterface::CHTMLInterface"))
}

CHTMLInterface::~CHTMLInterface()
{
	TRY_CATCH
	CATCH_LOG(_T("CHTMLInterface::~CHTMLInterface"))
}


HRESULT CHTMLInterface::INTERFACE_UpdateData(CString sType, CString sData, CString sArg1, CString sArg2, CString sArg3)
{
TRY_CATCH

	//CComVariant pVarResult;
	CStringArray paramArray;
	bool bRes = false;
	
	paramArray.Add( sType );
	paramArray.Add( sData );
	
	paramArray.Add( sArg1 );
	paramArray.Add( sArg2 );
	paramArray.Add( sArg3 );

	bRes = m_webPage.CallJScript(_T("INTERFACE_UpdateData"), paramArray, NULL);
	if(bRes==false)
	{
		Log.Add(_WARNING_,_T("INTERFACE_UpdateData failed. Type: %s"), sType );	
		return S_FALSE;
	}
	//m_webPage.CallJScript( _T("INTERFACE_UpdateData"), paramArray, &pVarResult);
	//m_webPage.CallJScriptByInvokeHelper( _T("INTERFACE_UpdateData"),sType, sData, sArg1, sArg2, sArg3);

CATCH_THROW(_T("CHTMLInterface::INTERFACE_UpdateData"))
	return S_OK;
}

void	CHTMLInterface::SetDocument(CWebPage	webPage)
{ 
TRY_CATCH
	m_webPage = webPage; 
CATCH_THROW(_T("CHTMLInterface::SetDocument"))
};