#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// Marscha@2004 - authSSP: from stdhdrs.h, required for logging
#include "VNCLog.h"

// No logging at all
#define LL_NONE		0
// Log server startup/shutdown
#define LL_STATE	0
// Log connect/disconnect
#define LL_CLIENTS	1
// Log connection errors (wrong pixfmt, etc)
#define LL_CONNERR	0
// Log socket errors
#define LL_SOCKERR	4
// Log internal errors
#define LL_INTERR	0

// Log internal warnings
#define LL_INTWARN	8
// Log internal info
#define LL_INTINFO	9
// Log socket errors
#define LL_SOCKINFO	10
// Log everything, including internal table setup, etc.
#define LL_ALL		10

// Macros for sticking in the current file name
#define VNCLOG(s)	(__FILE__ " : " s)
// Marscha@2004 - authSSP: end of change

#include "localization.h" // Act : add localization on messages

typedef BOOL (*CheckUserGroupPasswordFn)( char * userin,char *password,char *machine,char *group,int locdom);
CheckUserGroupPasswordFn CheckUserGroupPassword = 0;

int CheckUserGroupPasswordUni(char * userin,char *password,const char *machine);
int CheckUserGroupPasswordUni2(char * userin,char *password,const char *machine);

// Marscha@2004 - authSSP: if "New MS-Logon" is checked, call CheckUserPasswordSDUni
BOOL IsNewMSLogon();
//char *AddToModuleDir(char *filename, int length);
typedef int (*CheckUserPasswordSDUniFn)(const char * userin, const char *password, const char *machine);
CheckUserPasswordSDUniFn CheckUserPasswordSDUni = 0;

#define MAXSTRING 254

const TCHAR REGISTRY_KEY [] = "Software\\UltraVnc";

HKEY hkLocal=NULL;
HKEY hkDefault=NULL;
HKEY hkUser=NULL;

void
OpenRegistry()
{
	DWORD dw;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		REGISTRY_KEY,
		0,REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_READ,
		NULL, &hkLocal, &dw) != ERROR_SUCCESS)
		return;
	if (RegCreateKeyEx(hkLocal,
		"mslogon",
		0, REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_READ,
		NULL, &hkDefault, &dw) != ERROR_SUCCESS)
		return;
}

void
CloseRegistry()
{
	if (hkDefault != NULL) RegCloseKey(hkDefault);
	if (hkUser != NULL) RegCloseKey(hkUser);
	if (hkLocal != NULL) RegCloseKey(hkLocal);
}

LONG
LoadInt(HKEY key, LPCSTR valname, LONG defval)
{
	LONG pref;
	ULONG type = REG_DWORD;
	ULONG prefsize = sizeof(pref);

	if (RegQueryValueEx(key,
		valname,
		NULL,
		&type,
		(LPBYTE) &pref,
		&prefsize) != ERROR_SUCCESS)
		return defval;

	if (type != REG_DWORD)
		return defval;

	if (prefsize != sizeof(pref))
		return defval;

	return pref;
}

TCHAR *
LoadString(HKEY key, LPCSTR keyname)
{
	DWORD type = REG_SZ;
	DWORD buflen = 256*sizeof(TCHAR);
	TCHAR *buffer = 0;

	// Get the length of the string
	if (RegQueryValueEx(key,
		keyname,
		NULL,
		&type,
		NULL,
		&buflen) != ERROR_SUCCESS)
		return 0;

	if (type != REG_BINARY)
		return 0;
	buflen = 256*sizeof(TCHAR);
	buffer = new TCHAR[buflen];
	if (buffer == 0)
		return 0;

	// Get the string data
	if (RegQueryValueEx(key,
		keyname,
		NULL,
		&type,
		(BYTE*)buffer,
		&buflen) != ERROR_SUCCESS) {
		delete [] buffer;
		return 0;
	}

	// Verify the type
	if (type != REG_BINARY) {
		delete [] buffer;
		return 0;
	}

	return (TCHAR *)buffer;
}

void
SaveInt(HKEY key, LPCSTR valname, LONG val)
{
	RegSetValueEx(key, valname, 0, REG_DWORD, (LPBYTE) &val, sizeof(val));
}

void
SaveString(HKEY key,LPCSTR valname, TCHAR *buffer)
{
	RegSetValueEx(key, valname, 0, REG_BINARY, (LPBYTE) buffer, MAXSTRING);
}

void
savegroup1(TCHAR *value)
{
	OpenRegistry();
	if (hkDefault)SaveString(hkDefault, "group1", value);
	CloseRegistry();
}
TCHAR*
Readgroup1()
{
	TCHAR *value=NULL;
	OpenRegistry();
	if (hkDefault) value=LoadString (hkDefault, "group1");
	CloseRegistry();
	return value;
}

void
savegroup2(TCHAR *value)
{
	OpenRegistry();
	if (hkDefault)SaveString(hkDefault, "group2", value);
	CloseRegistry();
}
TCHAR*
Readgroup2()
{
	TCHAR *value=NULL;
	OpenRegistry();
	if (hkDefault) value=LoadString (hkDefault, "group2");
	CloseRegistry();
	return value;
}

void
savegroup3(TCHAR *value)
{
	OpenRegistry();
	if (hkDefault)SaveString(hkDefault, "group3", value);
	CloseRegistry();
}
TCHAR*
Readgroup3()
{
	TCHAR *value=NULL;
	OpenRegistry();
	if (hkDefault) value=LoadString (hkDefault, "group3");
	CloseRegistry();
	return value;
}

LONG
Readlocdom1(LONG returnvalue)
{
	OpenRegistry();
	if (hkDefault) returnvalue=LoadInt(hkDefault, "locdom1",returnvalue);
	CloseRegistry();
	return returnvalue;
}

void
savelocdom1(LONG value)
{
	OpenRegistry();
	if (hkDefault)SaveInt(hkDefault, "locdom1", value);
	CloseRegistry();

}

LONG
Readlocdom2(LONG returnvalue)
{
	OpenRegistry();
	if (hkDefault) returnvalue=LoadInt(hkDefault, "locdom2",returnvalue);
	CloseRegistry();
	return returnvalue;
}

void
savelocdom2(LONG value)
{
	OpenRegistry();
	if (hkDefault)SaveInt(hkDefault, "locdom2", value);
	CloseRegistry();

}

LONG
Readlocdom3(LONG returnvalue)
{
	OpenRegistry();
	if (hkDefault) returnvalue=LoadInt(hkDefault, "locdom3",returnvalue);
	CloseRegistry();
	return returnvalue;
}

void
savelocdom3(LONG value)
{
	OpenRegistry();
	if (hkDefault)SaveInt(hkDefault, "locdom3", value);
	CloseRegistry();

}

///////////////////////////////////////////////////////////
bool CheckAD()
{
	HMODULE hModule = LoadLibrary("Activeds.dll");
	if (hModule)
	{
		FreeLibrary(hModule);
		return true;
	}
	return false;
}

bool CheckNetapi95()
{
	HMODULE hModule = LoadLibrary("netapi32.dll");
	if (hModule)
	{
		FreeLibrary(hModule);
		return true;
	}
	return false;
}

bool CheckDsGetDcNameW()
{
	HMODULE hModule = LoadLibrary("netapi32.dll");
	if (hModule)
	{
		FARPROC test=NULL;
		test=GetProcAddress( hModule, "DsGetDcNameW" );
		FreeLibrary(hModule);
		if (test) return true;
	}
	return false;
}

bool CheckNetApiNT()
{
	HMODULE hModule = LoadLibrary("radmin32.dll");
	if (hModule)
	{
		FreeLibrary(hModule);
		return true;
	}
	return false;
}

int CheckUserGroupPasswordUni(char * userin,char *password,const char *machine)
{
	int result = 0;
	// Marscha@2004 - authSSP: if "New MS-Logon" is checked, call CUPSD in authSSP.dll,
	// else call "old" mslogon method.
	if (IsNewMSLogon()){
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH)) {
			char* p = strrchr(szCurrentDir, '\\');
			*p = '\0';
			strcat (szCurrentDir,"\\authSSP.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule) {
			CheckUserPasswordSDUni = (CheckUserPasswordSDUniFn) GetProcAddress(hModule, "CUPSD");
			//vnclog.Print(LL_INTINFO, VNCLOG("GetProcAddress"));
			HRESULT hr = CoInitialize(NULL);
			result = CheckUserPasswordSDUni(userin, password, machine);
			//vnclog.Print(LL_INTINFO, "CheckUserPasswordSDUni result=%i", result);
			CoUninitialize();
			FreeLibrary(hModule);
			//result = CheckUserPasswordSDUni(userin, password, machine);
		} else {
			LPCTSTR sz_ID_AUTHSSP_NOT_FO = // to be moved to localization.h
				"You selected ms-logon, but authSSP.dll\nwas not found.Check you installation";
			MessageBox(NULL, sz_ID_AUTHSSP_NOT_FO, sz_ID_WARNING, MB_OK);
		}
	} else 
		result = CheckUserGroupPasswordUni2(userin, password, machine);
	return result;
}

int CheckUserGroupPasswordUni2(char * userin,char *password,const char *machine)
{
	int result=0;
	BOOL NT4OS=false;
	BOOL W2KOS=false;
	char clientname[256];
	strcpy(clientname,machine);
	if (!CheckNetapi95() && !CheckNetApiNT())
	{
		return false;
	}
	OSVERSIONINFO VerInfo;
	VerInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	if (!GetVersionEx (&VerInfo))   // If this fails, something has gone wrong
		{
		  return FALSE;
		}
	if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && VerInfo.dwMajorVersion == 4)
		{
			NT4OS=true;
		}
	if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && VerInfo.dwMajorVersion >= 5)
		{
			W2KOS=true;
		}
	//////////////////////////////////////////////////
	// Load reg settings
	//////////////////////////////////////////////////
	char pszgroup1[256];
	char pszgroup2[256];
	char pszgroup3[256];
	char *group1=NULL;
	char *group2=NULL;
	char *group3=NULL;
	long locdom1=1;
	long locdom2=0;
	long locdom3=0;
	group1=Readgroup1();
	group2=Readgroup2();
	group3=Readgroup3();
	locdom1=Readlocdom1(locdom1);
	locdom2=Readlocdom2(locdom2);
	locdom3=Readlocdom3(locdom3);
	strcpy(pszgroup1,"VNCACCESS");
	strcpy(pszgroup2,"Administrators");
	strcpy(pszgroup3,"VNCVIEWONLY");
	if (group1){strcpy(pszgroup1,group1);}
	if (group2){strcpy(pszgroup2,group2);}
	if (group3){strcpy(pszgroup3,group3);}
	savegroup1(pszgroup1);
	savegroup2(pszgroup2);
	savegroup3(pszgroup3);
	savelocdom1(locdom1);
	savelocdom2(locdom2);
	savelocdom3(locdom3);
	if (group1){strcpy(pszgroup1,group1);delete group1;}
	if (group2){strcpy(pszgroup2,group2);delete group2;}
	if (group3){strcpy(pszgroup3,group3);delete group3;}

	//////////////////////////////////////////////////
	// logon user only works on NT>
	// NT4/w2k only as service (system account)
	// XP> works also as application
	// Group is not used...admin access rights is needed
	// MS keep changes there security model for each version....
	//////////////////////////////////////////////////
if (strcmp(pszgroup1,"")!=NULL)
{
	
	///////////////////////////////////////////////////
	// NT4 domain and workgroups
	//
	///////////////////////////////////////////////////
	{
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\workgrpdomnt4.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup1,locdom1);
				CoUninitialize();
				FreeLibrary(hModule);
			}
		else MessageBox(NULL, sz_ID_AUTH_NOT_FO, sz_ID_WARNING, MB_OK);

	}
	if (result==1) goto accessOK;
	////////////////////////////////////////////////////
	if ( NT4OS || W2KOS){
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\authadmin.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup1,locdom1);
				CoUninitialize();
				FreeLibrary(hModule);
			}
		else MessageBox(NULL, sz_ID_AUTH_NOT_FO, sz_ID_WARNING, MB_OK);

	}
	if (result==1) goto accessOK;
	/////////////////////////////////////////////////////////////////
	if (CheckAD() && W2KOS && (locdom1==2||locdom1==3))
	{
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\ldapauth.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup1,locdom1);
				CoUninitialize();
				FreeLibrary(hModule);
			}
	}
	if (result==1) goto accessOK;
	//////////////////////////////////////////////////////////////////////
	if (CheckAD() && NT4OS && CheckDsGetDcNameW() && (locdom1==2||locdom1==3))
	{
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\ldapauthnt4.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup1,locdom1);
				CoUninitialize();
				FreeLibrary(hModule);
			}
	}
	if (result==1) goto accessOK;
	//////////////////////////////////////////////////////////////////////
	if (CheckAD() && !NT4OS && !W2KOS && (locdom1==2||locdom1==3))
	{
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\ldapauth9x.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup1,locdom1);
				CoUninitialize();
				FreeLibrary(hModule);
			}
	}
	if (result==1) goto accessOK;
}
/////////////////////////////////////////////////
if (strcmp(pszgroup2,"")!=NULL)
{
	///////////////////////////////////////////////////
	// NT4 domain and workgroups
	//
	///////////////////////////////////////////////////
	{
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\workgrpdomnt4.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup2,locdom2);
				CoUninitialize();
				FreeLibrary(hModule);
			}
		else MessageBox(NULL, sz_ID_AUTH_NOT_FO, sz_ID_WARNING, MB_OK);

	}
	if (result==1) goto accessOK;
	//////////////////////////////////////////////////////
	if ( NT4OS || W2KOS){
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\authadmin.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup2,locdom2);
				CoUninitialize();
				FreeLibrary(hModule);
			}
		else MessageBox(NULL, sz_ID_AUTH_NOT_FO, sz_ID_WARNING, MB_OK);

	}
	if (result==1) goto accessOK;
	//////////////////////////////////////////////////////////////////
	if (CheckAD() && W2KOS && (locdom2==2||locdom2==3))
	{
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\ldapauth.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup2,locdom2);
				CoUninitialize();
				FreeLibrary(hModule);
			}
	}
	if (result==1) goto accessOK;
	///////////////////////////////////////////////////////////////////////
	if (CheckAD() && NT4OS && CheckDsGetDcNameW() && (locdom2==2||locdom2==3))
	{
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\ldapauthnt4.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup2,locdom2);
				CoUninitialize();
				FreeLibrary(hModule);
			}
	}
	if (result==1) goto accessOK;
	///////////////////////////////////////////////////////////////////////
	if (CheckAD() && !NT4OS && !W2KOS && (locdom2==2||locdom2==3))
	{
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\ldapauth9x.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup2,locdom2);
				CoUninitialize();
				FreeLibrary(hModule);
			}
	}
	if (result==1) goto accessOK;
}
////////////////////////////
if (strcmp(pszgroup3,"")!=NULL)
{
	///////////////////////////////////////////////////
	// NT4 domain and workgroups
	//
	///////////////////////////////////////////////////
	{
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\workgrpdomnt4.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup3,locdom3);
				CoUninitialize();
				FreeLibrary(hModule);
			}
		else MessageBox(NULL, sz_ID_AUTH_NOT_FO, sz_ID_WARNING, MB_OK);

	}
	if (result==1) goto accessOK;
	////////////////////////////////////////////////////////
	if ( NT4OS || W2KOS){
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\authadmin.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup3,locdom3);
				CoUninitialize();
				FreeLibrary(hModule);
			}
		else MessageBox(NULL, sz_ID_AUTH_NOT_FO, sz_ID_WARNING, MB_OK);

	}
	if (result==1) goto accessOK;
	////////////////////////////////////////////////////////////////
	if (CheckAD() && W2KOS && (locdom3==2||locdom3==3))
	{
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\ldapauth.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup3,locdom3);
				CoUninitialize();
				FreeLibrary(hModule);
			}
	}
	if (result==1) goto accessOK;
	///////////////////////////////////////////////////////////////////
	if (CheckAD() && NT4OS && CheckDsGetDcNameW() && (locdom3==2||locdom3==3))
	{
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\ldapauthnt4.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup3,locdom3);
				CoUninitialize();
				FreeLibrary(hModule);
			}
		}
		if (result==1) goto accessOK2;
		///////////////////////////////////////////////////////////////////
	if (CheckAD() && !NT4OS && !W2KOS && (locdom3==2||locdom3==3))
	{
		char szCurrentDir[MAX_PATH];
		if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
		{
			char* p = strrchr(szCurrentDir, '\\');
			if (p == NULL) return false;
			*p = '\0';
			strcat (szCurrentDir,"\\ldapauth9x.dll");
		}
		HMODULE hModule = LoadLibrary(szCurrentDir);
		if (hModule)
			{
				CheckUserGroupPassword = (CheckUserGroupPasswordFn) GetProcAddress( hModule, "CUGP" );
				HRESULT hr = CoInitialize(NULL);
				result=CheckUserGroupPassword(userin,password,clientname,pszgroup3,locdom3);
				CoUninitialize();
				FreeLibrary(hModule);
			}
		}
		if (result==1) goto accessOK2;
	}

	/////////////////////////////////////////////////
	// If we reach this place auth failed
	/////////////////////////////////////////////////
	{
				typedef BOOL (*LogeventFn)(char *machine,char *user);
				LogeventFn Logevent = 0;
				char szCurrentDir[MAX_PATH];
				if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
					{
						char* p = strrchr(szCurrentDir, '\\');
						*p = '\0';
						strcat (szCurrentDir,"\\logging.dll");
					}
				HMODULE hModule = LoadLibrary(szCurrentDir);
				if (hModule)
					{
						Logevent = (LogeventFn) GetProcAddress( hModule, "LOGFAILEDUSER" );
						Logevent((char *)clientname,userin);
						FreeLibrary(hModule);
					}
				return result;
	}

	accessOK://full access
	{
				typedef BOOL (*LogeventFn)(char *machine,char *user);
				LogeventFn Logevent = 0;
				char szCurrentDir[MAX_PATH];
				if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
					{
						char* p = strrchr(szCurrentDir, '\\');
						*p = '\0';
						strcat (szCurrentDir,"\\logging.dll");
					}
				HMODULE hModule = LoadLibrary(szCurrentDir);
				if (hModule)
					{
						Logevent = (LogeventFn) GetProcAddress( hModule, "LOGLOGONUSER" );
						Logevent((char *)clientname,userin);
						FreeLibrary(hModule);
					}
				return result;
	}

	accessOK2://readonly
	{
				typedef BOOL (*LogeventFn)(char *machine,char *user);
				LogeventFn Logevent = 0;
				char szCurrentDir[MAX_PATH];
				if (GetModuleFileName(NULL, szCurrentDir, MAX_PATH))
					{
						char* p = strrchr(szCurrentDir, '\\');
						*p = '\0';
						strcat (szCurrentDir,"\\logging.dll");
					}
				HMODULE hModule = LoadLibrary(szCurrentDir);
				if (hModule)
					{
						Logevent = (LogeventFn) GetProcAddress( hModule, "LOGLOGONUSER" );
						Logevent((char *)clientname,userin);
						FreeLibrary(hModule);
					}
				result=2;
	}

	return result;
}

// Marscha@2004 - authSSP: Is New MS-Logon activated?
BOOL IsNewMSLogon(){
	HKEY hkLocal=NULL;
	BOOL isNewMSLogon = FALSE;
	LONG data;
	ULONG type = REG_DWORD;
	ULONG datasize = sizeof(data);
		

	__try {
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			"Software\\ORL\\WinVNC3",
			NULL,
			KEY_QUERY_VALUE,
			&hkLocal) != ERROR_SUCCESS)
			__leave;
		
		if (RegQueryValueEx(hkLocal,
			"NewMSLogon",
			NULL,
			&type,
			(LPBYTE) &data,
			&datasize) != ERROR_SUCCESS)
			__leave;
		
		if (type != REG_DWORD ||
			datasize != sizeof(data))
			__leave;
		
		isNewMSLogon = data;
	} __finally {

		if (hkLocal != NULL) RegCloseKey(hkLocal);
	}
	return isNewMSLogon;
}

/*
char *AddToModuleDir(char *filename, int length){
	char *szCurrentDir = new char[length];
	if (GetModuleFileName(NULL, szCurrentDir, length))
	{
		char *p = strrchr(szCurrentDir, '\\');
		*p = '\0';
		strcat(szCurrentDir,"\\");
		strcat(szCurrentDir, filename);
	}
	strcpy(filename, szCurrentDir);
	delete szCurrentDir;
	return filename;
}
*/
