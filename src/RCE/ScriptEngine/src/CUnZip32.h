/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CUnZip32.h
///
///  Declares CUnZip32 class, responsible for decompression directories
///
///  @author Dmitry Netrebenko @date 19.12.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <boost/function.hpp>

#define UNZIP_DLL_NAME "unzip32.dll"

/// Define password callback for decompression directory
typedef boost::function<int(char*,int)> UnZipPasswdCallback;

// Dll Callback functions
typedef int	(WINAPI DLLPRNT)	(LPSTR, unsigned long);
typedef int (WINAPI DLLPASSWORD)(LPSTR, int, LPCSTR, LPCSTR);
typedef int (WINAPI DLLSERVICE)	(LPCSTR, unsigned long);
typedef void(WINAPI DLLSND)		(void);
typedef int (WINAPI DLLREPLACE)	(LPSTR);
typedef void(WINAPI DLLMESSAGE)	(unsigned long, unsigned long, unsigned,
								 unsigned, unsigned, unsigned, unsigned, unsigned,
								 char, LPSTR, LPSTR, unsigned long, char);

// Unzip callback functions struct
typedef struct
{
	DLLPRNT*		print;
	DLLSND*			sound;
	DLLREPLACE*		replace;
	DLLPASSWORD*	password;
	DLLMESSAGE*		SendApplicationMessage;
	DLLSERVICE*		ServCallBk;
	unsigned long	TotalSizeComp;
	unsigned long	TotalSize;
	unsigned long	CompFactor;       /* "long" applied for proper alignment, only */
	unsigned long	NumMembers;
	WORD			cchComment;
} USERFUNCTIONS, far * LPUSERFUNCTIONS;

// Unzip options struct
typedef struct
{
	int		ExtractOnlyNewer;
	int		SpaceToUnderscore;
	int		PromptToOverwrite;
	int		fQuiet;
	int		ncflag;
	int		ntflag;
	int		nvflag;
	int		nfflag;
	int		nzflag;
	int		ndflag;
	int		noflag;
	int		naflag;
	int		nZIflag;
	int		C_flag;
	int		fPrivilege;
	LPSTR	lpszZipFN;
	LPSTR	lpszExtractDir;
} DCL, far * LPDCL;

// Dll exported functions
typedef int (WINAPI * _DLL_UNZIP)(int, char **, int, char **, LPDCL, LPUSERFUNCTIONS);
typedef int (WINAPI * _USER_FUNCTIONS)(LPUSERFUNCTIONS);

extern "C"
{
int WINAPI Wiz_SingleEntryUnzip(int ifnc, char **ifnv, int xfnc, char **xfnv,
								LPDCL lpDCL, LPUSERFUNCTIONS lpUserFunc);
}

/// CUnZip32 class, responsible for decompression directories
class CUnZip32 
{
private:
/// Prevents making copies of CUnZip32 objects
	CUnZip32(const CUnZip32&);
	CUnZip32& operator=(const CUnZip32&);
public:
/// Constructor
	CUnZip32();
/// Destructor
	virtual ~CUnZip32();
/// Uncompress directory
/// @param szRootDir - uncompress destination directory
/// @param szZipFileName - name of archive
	bool UnZipDirectory(char* szRootDir, char* szZipFileName);
/// Sets password callback
/// @param fn - callback function
	void SetPasswdCallback(UnZipPasswdCallback fn);
private:
	// Unzip part
	_DLL_UNZIP				m_PWiz_SingleEntryUnzip;	// Unzip function pointer
	_USER_FUNCTIONS			m_PWiz_Init;				// Init function pointer
	LPUSERFUNCTIONS			m_lpUserFunctions;
	int						m_hUnzipFile;
	static CUnZip32*		m_this;
	UnZipPasswdCallback		m_passwdCallback;
private:
/// UnZip callbacks
	static int WINAPI UnZipReplace(char *filename)
	{
		return 1;
	}
	static void WINAPI UnZipDllMessage(	unsigned long ucsize, unsigned long csiz,
								unsigned cfactor,
								unsigned mo, unsigned dy, unsigned yr, unsigned hh, unsigned mm,
								char c, LPSTR filename, LPSTR methbuf, unsigned long crc, char fCrypt){};
	static int WINAPI UnZipPassword(char *p, int n, const char *m, const char *name)
	{
		if(m_this)
		{
			if(m_this->m_passwdCallback)
				return m_this->m_passwdCallback(p,n);
		}
		return 1;
	}
	static int WINAPI UnZipPrint(LPSTR buf, unsigned long size)
	{
		return (unsigned int) size;
	}
};

