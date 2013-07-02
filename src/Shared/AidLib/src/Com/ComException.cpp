/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  ComException.cpp
///
///  Implementation for COM error handling
///
///  @author Kirill Solovyov @date 11.02.2008
///
////////////////////////////////////////////////////////////////////////
#include <AidLib/Com/ComException.h>
#include <atlcomcli.h>

AIDLIB_API tstring TStringFromErrorInfo(void)
{
TRY_CATCH
	CComPtr<IErrorInfo> spErrInfo;
	tstring message;
	if(S_OK==GetErrorInfo(0, &spErrInfo))
	{
		USES_CONVERSION;
		CComBSTR buf;
		spErrInfo->GetSource(&buf);
		message+=Format(_T("Source: %s\n"),OLE2T(buf));
		spErrInfo->GetDescription(&buf);
		message+=Format(_T("Description: %s\n"),OLE2T(buf));
		spErrInfo->GetHelpFile(&buf);
		message+=Format(_T("HelpFile: %s\n"),OLE2T(buf));
		DWORD dwBuf;
		spErrInfo->GetHelpContext(&dwBuf);
		message+=Format(_T("HelpContext: 0x%x\n"),dwBuf);
	}
	return message;
CATCH_THROW()
}
