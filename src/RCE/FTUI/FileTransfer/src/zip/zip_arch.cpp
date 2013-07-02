/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
// 
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// If the source code for the program is not available from the place from
// which you received this file, check 
// http://ultravnc.sourceforge.net/
//
////////////////////////////////////////////////////////////////////////////
/*
  Copyright (c) 1990-2002 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2000-Apr-09 or later
  (the contents of which are also included in unzip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/

#include <memory.h>
#include <stdio.h>
#include <string.h>
#include "zip_arch.h"
#include "filetransferdata.h"
//#include <boost/shared_ptr.hpp>
//#include <boost/type_traits/remove_pointer.hpp>



///typedef boost::shared_ptr<boost::remove_pointer<HANDLE>::type> SPHandle;
typedef boost::shared_ptr<CHAR> SPArray;

////////////////////////////////////////////////////////////////////////
// ZIP32 DLL PART
/////////////////////////////////////////////////////////////////////////

//
//
//
CZipUnZip32::CZipUnZip32()
{
	//m_hZipDll = NULL;
	//m_hUnzipDll = NULL;

	m_ZpOpt.fSuffix = FALSE;        /* include suffixes (not yet implemented) */
	m_ZpOpt.fEncrypt = FALSE;       /* true if encryption wanted */
	m_ZpOpt.fSystem = TRUE;        /* true to include system/hidden files */
	m_ZpOpt.fVolume = FALSE;        /* true if storing volume label */
	m_ZpOpt.fExtra = FALSE;         /* true if including extra attributes */
	m_ZpOpt.fNoDirEntries = FALSE;  /* true if ignoring directory entries */
	m_ZpOpt.fVerbose = FALSE;       /* true if full messages wanted */
	m_ZpOpt.fQuiet = TRUE;         /* true if minimum messages wanted */
	m_ZpOpt.fCRLF_LF = FALSE;       /* true if translate CR/LF to LF */
	m_ZpOpt.fLF_CRLF = FALSE;       /* true if translate LF to CR/LF */
	m_ZpOpt.fJunkDir = FALSE;       /* true if junking directory names */
	m_ZpOpt.fGrow = FALSE;          /* true if allow appending to zip file */
	m_ZpOpt.fForce = FALSE;         /* true if making entries using DOS names */
	m_ZpOpt.fMove = FALSE;          /* true if deleting files added or updated */
	m_ZpOpt.fUpdate = FALSE;        /* true if updating zip file--overwrite only if newer */
	m_ZpOpt.fFreshen = FALSE;       /* true if freshening zip file--overwrite only */
	m_ZpOpt.fJunkSFX = FALSE;       /* true if junking sfx prefix*/
	m_ZpOpt.fLatestTime = FALSE;    /* true if setting zip file time to time of latest file in archive */
	m_ZpOpt.fComment = FALSE;       /* true if putting comment in zip file */
	m_ZpOpt.fOffsets = FALSE;       /* true if updating archive offsets for sfx files */
	m_ZpOpt.fDeleteEntries = FALSE; /* true if deleting files from archive */
	m_ZpOpt.fRecurse = 1;           /* subdir recursing mode: 1 = "-r", 2 = "-R" */
	m_ZpOpt.fRepair = 0;            /* archive repair mode: 1 = "-F", 2 = "-FF" */
	m_ZpOpt.Date = NULL;
	m_ZpOpt.fExcludeDate = FALSE;      /* Exclude files newer than specified date */
	m_ZpOpt.fIncludeDate = FALSE;      /* Include only files newer than specified date */ 
	m_ZpOpt.fLevel = '1';/* Not using, set to NULL pointer */
	m_ZpOpt.fPrivilege = FALSE;        /* Use privileges (WIN32 only) */
	m_ZpOpt.fEncryption = FALSE;       
}

CZipUnZip32::~CZipUnZip32()
{
}


// 
// Zip the given directory content
// 
bool CZipUnZip32::ZipDirectory(char* szRootDir, char* szDirectoryToZip, char* szZipFileName, bool fCompress)
{
	LPSTR szFileList;
	char **index, *sz;
	int retcode, i;
	size_t cc;



	// Init the User Function struct
	SPHandle hZUF = SPHandle ( GlobalAlloc( GPTR, (DWORD)sizeof(ZIPUSERFUNCTIONS)) , GlobalFree ) ;
	if (!hZUF.get())
		return false;

	m_lpZipUserFunctions = (LPZIPUSERFUNCTIONS)GlobalLock(hZUF.get());
	if (!m_lpZipUserFunctions)
		return false;

	m_lpZipUserFunctions->print = DummyPrint;
	m_lpZipUserFunctions->password = DummyPassword;
	m_lpZipUserFunctions->comment = DummyComment;


	if (!ZpInit(m_lpZipUserFunctions))
		return false;

							
	// Set the infos on the file (directory) to zip
	m_ZpZCL.argc = 1;      
	m_ZpZCL.lpszZipFN = szZipFileName;
	SPHandle hFileList = SPHandle( GlobalAlloc( GPTR, 0x10000L) , GlobalFree );
	if ( hFileList.get() )
	{
	  szFileList = (char far *)GlobalLock(hFileList.get());
	}

	index = (char **)szFileList;
	cc = (sizeof(char *) * m_ZpZCL.argc);
	sz = szFileList + cc;
	for (i = 0; i < m_ZpZCL.argc; i++)
	{
		cc = strlen(szDirectoryToZip);
		strcpy(sz, szDirectoryToZip);
		index[i] = sz;
		sz += (cc + 1);
	}

	m_ZpZCL.FNV = (char **)szFileList;

	if (fCompress)
		m_ZpOpt.fLevel = '1';
	else
		m_ZpOpt.fLevel = '0';
	m_ZpOpt.fTemp = FALSE;
	m_ZpOpt.szTempDir = "";
	m_ZpOpt.szRootDir = szRootDir;
	ZpSetOptions(&m_ZpOpt);

	// Zip the Directory
	retcode = ZpArchive(m_ZpZCL);

	return (retcode == 0);
}


int WINAPI DummyPassword(LPSTR p, int n, LPCSTR m, LPCSTR name)
{
	return 1;
}

int WINAPI DummyPrint(char far *buf, unsigned long size)
{
	return (unsigned int) size;
}


int WINAPI DummyComment(char far *szBuf)
{
	szBuf[0] = '\0';
	return 1;
} 


/////////////////////////////////////////////////////////////////////////
// END OF ZIP32 DLL PART
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// UNZIP32 DLL PART
/////////////////////////////////////////////////////////////////////////

bool CZipUnZip32::UnZipDirectory(char* szRootDir, char* szZipFileName)
{
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
	
	m_lpUserFunctions->password = password;
	m_lpUserFunctions->print = DisplayBuf;
	m_lpUserFunctions->sound = NULL;
	m_lpUserFunctions->replace = GetReplaceDlgRetVal;
	m_lpUserFunctions->SendApplicationMessage = ReceiveDllMessage;

	lpDCL->ncflag = 0; // Write to stdout if true 
	lpDCL->fQuiet = 2; // We want all messages. 1 = fewer messages,2 = no messages 
	lpDCL->ntflag = 0; // test zip file if true 
	lpDCL->nvflag = 0; // give a verbose listing if true 
	lpDCL->nzflag = 0; // display a zip file comment if true 
	lpDCL->ndflag = 1; // Recreate directories != 0, skip "../" if < 2
	lpDCL->naflag = 0; // Do not convert CR to CRLF 
	lpDCL->nfflag = 0; // Do not freshen existing files only 
	lpDCL->noflag = 1; // Over-write all files if true

    lpDCL->nZIflag = FALSE;
/*
    ExtractOnlyNewer  = true for "update" without interaction
                        (extract only newer/new files, without queries)
    SpaceToUnderscore = true if convert space to underscore
    PromptToOverwrite = true if prompt to overwrite is wanted
    fQuiet            = quiet flag:
                         0 = all messages, 1 = few messages, 2 = no messages
    ncflag            = write to stdout if true
    ntflag            = test zip file
    nvflag            = verbose listing
    nfflag            = "freshen" (replace existing files by newer versions)
    nzflag            = display zip file comment
    ndflag            = controls (sub)directory recreation during extraction
                        0 = junk paths from filenames
                        1 = "safe" usage of paths in filenames (skip "../")
                        2 = allow also unsafe path components (dir traversal)
    noflag            = true if you are to always overwrite existing files
    naflag            = do end-of-line translation
    nZIflag           = get ZipInfo if TRUE
    C_flag            = be case insensitive if TRUE
    fPrivilege        = 1 => restore ACLs in user mode,
                        2 => try to use privileges for restoring ACLs
    lpszZipFN         = zip file name
    lpszExtractDir    = directory to extract to. This should be NULL if you
                        are extracting to the current directory.
 */

	lpDCL->ExtractOnlyNewer = 0; // Do not extract only newer
	lpDCL->PromptToOverwrite = 0; // "Overwrite all" selected -> no query mode
	lpDCL->lpszZipFN = szZipFileName; // The archive name 
	lpDCL->lpszExtractDir = szRootDir; // The directory to extract to. This is set to NULL if you are extracting to the current directory.
	
	infc = exfc = 0;
	infv = exfv = NULL;

	// Unzip the directory content
	retcode = Wiz_SingleEntryUnzip/*(*m_PWiz_SingleEntryUnzip)*/(	infc,
											infv,
											exfc,
											exfv,
											lpDCL,
											m_lpUserFunctions
										);
	
	//FreeUpUnzipMemory();
 	return (retcode == 0);
}

int WINAPI GetReplaceDlgRetVal(char *filename)
{
	return 1;
}


void WINAPI ReceiveDllMessage(	unsigned long ucsize, unsigned long csiz,
								unsigned cfactor,
								unsigned mo, unsigned dy, unsigned yr, unsigned hh, unsigned mm,
								char c, LPSTR filename, LPSTR methbuf, unsigned long crc, char fCrypt)
{
}

int WINAPI password(char *p, int n, const char *m, const char *name)
{
	return 1;
}

//#include <AidLib/logging/clog.h>

int WINAPI DisplayBuf(LPSTR buf, unsigned long size)
{
//	Log.Add(_MESSAGE_,_T("ARCH MSG: %s"),buf);
	return (unsigned int) size;
}
