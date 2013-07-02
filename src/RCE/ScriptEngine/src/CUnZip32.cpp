/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CUnZip32.cpp
///
///  Implements CUnZip32 class, responsible for decompression directories
///
///  @author Dmitry Netrebenko @date 19.12.2007
///
////////////////////////////////////////////////////////////////////////

#include <memory.h>
#include <stdio.h>
//#include <string.h>
#include "CUnZip32.h"
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <AidLib/CException/CException.h>

/// Define shared pointer to HANDLE
typedef boost::shared_ptr<boost::remove_pointer<HANDLE>::type> SPHandle;

CUnZip32* CUnZip32::m_this = NULL;

CUnZip32::CUnZip32()
{
TRY_CATCH
CATCH_THROW()
}

CUnZip32::~CUnZip32()
{
TRY_CATCH
CATCH_LOG()
}

bool CUnZip32::UnZipDirectory(char* szRootDir, char* szZipFileName)
{
TRY_CATCH
	int exfc, infc;
	char **exfv, **infv;
	int retcode;
	
	SPHandle hDCL = SPHandle( GlobalAlloc( GPTR, (DWORD)sizeof(DCL)) , GlobalFree);
	if (!hDCL.get())
		return false;

	LPDCL lpDCL = (LPDCL)GlobalLock(hDCL.get());
	if (!lpDCL)
		return false;
	
	SPHandle hUF = SPHandle( GlobalAlloc( GPTR, (DWORD)sizeof(USERFUNCTIONS)) , GlobalFree);
	if (!hUF.get())
		return false;

	m_lpUserFunctions = (LPUSERFUNCTIONS)GlobalLock(hUF.get());
	
	if (!m_lpUserFunctions)
		return false;
	
	m_lpUserFunctions->password = UnZipPassword;
	m_lpUserFunctions->print = UnZipPrint;
	m_lpUserFunctions->sound = NULL;
	m_lpUserFunctions->replace = UnZipReplace;
	m_lpUserFunctions->SendApplicationMessage = UnZipDllMessage;

	lpDCL->ncflag = 0; // Write to stdout if true 
	lpDCL->fQuiet = 2; // We want all messages. 1 = fewer messages,2 = no messages 
	lpDCL->ntflag = 0; // test zip file if true 
	lpDCL->nvflag = 0; // give a verbose listing if true 
	lpDCL->nzflag = 0; // display a zip file comment if true 
	lpDCL->ndflag = 1; // Recreate directories != 0, skip "../" if < 2
	lpDCL->naflag = 0; // Do not convert CR to CRLF 
	lpDCL->nfflag = 0; // Do not freshen existing files only 
	lpDCL->noflag = 1; // Over-write all files if true
	lpDCL->ExtractOnlyNewer = 0; // Do not extract only newer
	lpDCL->PromptToOverwrite = 0; // "Overwrite all" selected -> no query mode
	lpDCL->lpszZipFN = szZipFileName; // The archive name 
	lpDCL->lpszExtractDir = szRootDir; // The directory to extract to. This is set to NULL if you are extracting to the current directory.
	
	infc = exfc = 0;
	infv = exfv = NULL;

	m_this = this;
	// Unzip the directory content
	retcode = Wiz_SingleEntryUnzip(	infc,
									infv,
									exfc,
									exfv,
									lpDCL,
									m_lpUserFunctions);
	
 	return (retcode == 0);
CATCH_THROW()
}

void CUnZip32::SetPasswdCallback(UnZipPasswdCallback fn)
{
TRY_CATCH
	m_passwdCallback = fn;
CATCH_THROW()
}

