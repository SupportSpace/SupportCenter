#include "StdAfx.h"
#include "ProtectedData.h"
#include "AidLib/CException/CException.h"
#include <AidLib/Logging/cLog.h> 

CProtectedData::CProtectedData(void)
{
TRY_CATCH
CATCH_LOG(_T("CProtectedData::CProtectedData()"))
}

CProtectedData::~CProtectedData(void)
{
TRY_CATCH
CATCH_LOG(_T("CProtectedData::~CProtectedData()"))
}

void	CProtectedData::CryptData(IN TCHAR*  str_in, OUT DATA_BLOB* DataOut)
{
TRY_CATCH

	// Declare and initialize variables.
	DATA_BLOB DataIn;

	BYTE *pbDataInput = (BYTE *)str_in;
	DWORD cbDataInput = (DWORD)(_tcslen(str_in) + 1)*sizeof(TCHAR);

	DataIn.pbData = pbDataInput;    
	DataIn.cbData = cbDataInput;

	//-------------------------------------------------------------------
	//  Begin protect phase.
	// _tprintf(_T("The data to be encrypted is: %s\n"), str_in);

	if(CryptProtectData(
		&DataIn,
		NULL,								// A description string. 
		NULL,                               // Optional entropy	not used.
		NULL,                               // Reserved.
		NULL,							    // Pass a PromptStruct.
		CRYPTPROTECT_UI_FORBIDDEN || CRYPTPROTECT_AUDIT,
		DataOut))
	{
		 Log.Add(_MESSAGE_,_T("The encryption phase worked."));
	}
	else
	{
	 	 Log.Add(_ERROR_,_T("CProtectedData::CryptData Error number %x."),GetLastError );
	}

CATCH_LOG(_T("CProtectedData::~CryptData()"))
}

void CProtectedData::DeCryptData(IN DATA_BLOB* DataIn, OUT TCHAR*	str_out)
{
TRY_CATCH

	// Declare and initialize variables.
	DATA_BLOB DataVerify;

	//-------------------------------------------------------------------
	//  Begin unprotect phase
	//_tprintf(_T("The data to be decrypted is: %s\n"), (TCHAR*)DataIn->pbData );

	//-------------------------------------------------------------------
	if (CryptUnprotectData(
		DataIn,
		NULL,
		NULL,                 // Optional entropy
		NULL,                 // Reserved
		NULL,				  // Optional PromptStruct
		CRYPTPROTECT_UI_FORBIDDEN || CRYPTPROTECT_AUDIT,
		&DataVerify))
	{
		//_tprintf(_T("The decrypted data is: %s\n"), DataVerify.pbData);
		//Log.Add(_MESSAGE_,_T("The decrypted data is: %s"), DataVerify.pbData);
		_tcscpy_s(str_out, DataVerify.cbData ,(TCHAR*)DataVerify.pbData);
	}
	else
	{
		Log.Add(_ERROR_,_T("CProtectedData::DeCryptData Error number %x."),GetLastError );
	}
	//-------------------------------------------------------------------
	//  Clean up.
	LocalFree(DataVerify.pbData);

CATCH_LOG(_T("CProtectedData::~DeCryptData()"))
}