/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRCInstallerAXCtrl.cpp
///
///  IRCInstallerAXCtrl,  ActiveX installer
///
///  @author "Archer Software" Solovyov K. @date 20.12.2006
///
////////////////////////////////////////////////////////////////////////
// RCInstallerAXCtrl.cpp : Implementation of CRCInstallerAXCtrl
#include "stdafx.h"
#include "RCInstallerAXCtrl.h"
#include ".\rcinstalleraxctrl.h"
#include <RCEngine/AXstuff/AXstuff.h>
#include <boost\type_traits\remove_pointer.hpp>
#include <Msi.h>
#include "CLogBuilder.h"
#include <Sddl.h>
#pragma comment (lib, "Msi.lib")

tstring GetErrorInfoString()
{
	tstring errorStr;
TRY_CATCH
	
	CComPtr<IErrorInfo> errInfo;
	if (S_OK == GetErrorInfo(0, &errInfo))
	{
		CComBSTR errMess;
		if (S_OK == errInfo->GetDescription(&errMess))
		{
			USES_CONVERSION;
			errorStr = OLE2T(errMess);
		}
	}

CATCH_LOG()
	return errorStr;
}

/// Such definitions (see below), (duplicating msi.h) is needed since we want to be able to use WinInstaller 3.0 functions
/// through LoadLibrary, preventing from direct use of them through static linkage
#if (_WIN32_MSI <  300)
typedef enum tagMSIINSTALLCONTEXT
{
        MSIINSTALLCONTEXT_FIRSTVISIBLE   =  0,  // product visible to the current user
        MSIINSTALLCONTEXT_NONE           =  0,  // Invalid context for a product
        MSIINSTALLCONTEXT_USERMANAGED    =  1,  // user managed install context
        MSIINSTALLCONTEXT_USERUNMANAGED  =  2,  // user non-managed context
        MSIINSTALLCONTEXT_MACHINE        =  4,  // per-machine context
        MSIINSTALLCONTEXT_ALL            =  (MSIINSTALLCONTEXT_USERMANAGED | MSIINSTALLCONTEXT_USERUNMANAGED | MSIINSTALLCONTEXT_MACHINE),	// All contexts. OR of all valid values
        MSIINSTALLCONTEXT_ALLUSERMANAGED =  8,  // all user-managed contexts
} MSIINSTALLCONTEXT;

typedef enum tagMSICODE
{
	MSICODE_PRODUCT = 0x00000000L, // product code provided
	MSICODE_PATCH   = 0x40000000L  // patch code provided
}MSICODE;

#define INSTALLPROPERTY_LASTUSEDSOURCE        __TEXT("LastUsedSource")

#endif //_WIN32_MSI < 300


tstring GetCleanDomainName(const tstring& domainName)
{
TRY_CATCH
	//cleaning up domain name
	#define PROTO_PREFIX_COUNT 6
	static const tstring protocols[PROTO_PREFIX_COUNT] = {_T("http://"),_T("http:\\\\"),_T("https://"),_T("https:\\\\"),_T("ftp://"),_T("ftp:\\\\")};
	tstring clearDomainName(domainName);
	int pos;
	for(int i=0; i<PROTO_PREFIX_COUNT; ++i)
	{
		if ((pos = clearDomainName.find(protocols[i])) == 0)
		{
			clearDomainName.erase(0,protocols[i].length());
			break;
		}
	}
	if (!clearDomainName.empty() && (*(clearDomainName.end()-1) == _T('/') || *(clearDomainName.end()-1) == _T('\\')))
		clearDomainName.erase(clearDomainName.end()-1);

	return clearDomainName;
CATCH_THROW("GetCleanDomainName")
}

/// Returns true is string consists only spaces or is empty
bool IsDescriptionEmpty(const tstring descr)
{
TRY_CATCH
	static tstring spaces(_T(" \t\n\r"));
	for(tstring::const_iterator sym = descr.begin();
		sym != descr.end();
		++sym)
	{
		if (tstring::npos == spaces.find(*sym))
			return false;
	}
	return true;
CATCH_THROW()
}

/// Returns action name
tstring GetActionName(const tstring actionString)
{
TRY_CATCH
	tstring result(actionString);
	/// Removing time
	int pos;
	if (tstring::npos != (pos = result.find_last_of(':')))
		result = result.erase(0, pos + 1);
	/// Removing action description
	if (tstring::npos != (pos = result.find('.')))
		result = result.erase(pos, result.length()-1);
	return result;
CATCH_THROW()
}

tstring CRCInstallerAXCtrl::GetActionDescription(const tstring& actionString)
{
TRY_CATCH
	tstring result(actionString);
	/// Removing time
	int pos;
	if (tstring::npos != (pos = result.find_last_of(':')))
		result = result.erase(0, pos + 1);
	/// Removing action name
	if (tstring::npos != (pos = result.find('.')))
		result = result.erase(0, pos + 1);
	if (IsDescriptionEmpty(result))
		result = GetActionName(actionString);
	if (IsDescriptionEmpty(result))
		result = _T("Installing...");
	return result;
CATCH_LOG()
	return actionString;
}

void AddDomain2Trusted(const tstring& domainName)
{
TRY_CATCH
	tstring clearDomainName(GetCleanDomainName(domainName));
	static const tstring zoneKeyPrefix(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\ZoneMap\\Domains\\"));
	static const tstring zoneKeyPostfix(_T("http"));
	HKEY hkey;
	tstring keyName = zoneKeyPrefix + clearDomainName;
	DWORD disposition;
	DWORD res;
	if ((res = RegCreateKeyEx(HKEY_CURRENT_USER, keyName.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkey, &disposition)) != ERROR_SUCCESS)
	{
		SetLastError(res);
		throw MCException_Win("Failed to RegCreateKeyEx");
	}
	boost::shared_ptr<boost::remove_pointer<HKEY>::type> key(hkey,::RegCloseKey);
	DWORD value = 2; /*Trusted zone*/
	if ((res = RegSetValueEx(hkey, zoneKeyPostfix.c_str(), 0, REG_DWORD, reinterpret_cast<BYTE*>(&value), sizeof(value))) != ERROR_SUCCESS)
	{
		SetLastError(res);
		throw MCException_Win("Failed to RegSetValueEx");
	}

CATCH_THROW("AddDomain2Trusted")
}

typedef struct _TOKEN_MANDATORY_LABEL 
{
	SID_AND_ATTRIBUTES Label;
}	TOKEN_MANDATORY_LABEL, *PTOKEN_MANDATORY_LABEL;
typedef WINADVAPI BOOL (WINAPI *CONVERTSTRINGSIDTOSID) (LPCSTR, PSID);
#define SE_GROUP_INTEGRITY					(0x00000020L)
#define SE_GROUP_INTEGRITY_ENABLED			(0x00000040L)
#define SECURITY_MANDATORY_UNTRUSTED_RID	(0x00000000) //Untrusted.
#define SECURITY_MANDATORY_LOW_RID 			(0x00001000) //Low integrity.
#define SECURITY_MANDATORY_MEDIUM_RID 		(0x00002000) //Medium integrity.
#define SECURITY_MANDATORY_SYSTEM_RID 		(0x00004000) //System integrity.
#define SECURITY_MANDATORY_PROTECTED_PROCESS_RID	(0x00005000) //Protected process.
#define SECURITY_MANDATORY_HIGH_RID			(0x00003000L)

#define TOKEN_INTEGRITY_LEVEL (TOKEN_INFORMATION_CLASS)25

/*bool IsProcessIsolated()
{
TRY_CATCH
	bool result = true;
	HANDLE hToken;
	HANDLE hProcess;
	DWORD dwLengthNeeded;
	DWORD dwError = ERROR_SUCCESS;
	PTOKEN_MANDATORY_LABEL pTIL = NULL;
	DWORD dwIntegrityLevel;
	hProcess = GetCurrentProcess();
	if (OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_QUERY_SOURCE, &hToken)) 
	{
		// Get the Integrity level.
		if (!GetTokenInformation(hToken, TOKEN_INTEGRITY_LEVEL, NULL, 0, &dwLengthNeeded))
		{
			dwError = GetLastError();
			if (dwError == ERROR_INSUFFICIENT_BUFFER)
			{
				pTIL = (PTOKEN_MANDATORY_LABEL)LocalAlloc(0, 
					dwLengthNeeded);
				if (pTIL != NULL)
				{
					if (GetTokenInformation(hToken, TOKEN_INTEGRITY_LEVEL, pTIL, dwLengthNeeded, &dwLengthNeeded))
					{
						dwIntegrityLevel = *GetSidSubAuthority(pTIL->Label.Sid, (DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid)-1));

						if (dwIntegrityLevel < SECURITY_MANDATORY_MEDIUM_RID)
						{
							// Low Integrity
							result = true;
						}
						else if (dwIntegrityLevel >= SECURITY_MANDATORY_MEDIUM_RID && dwIntegrityLevel < SECURITY_MANDATORY_HIGH_RID)
						{
							// Medium Integrity
							result = false;
						}
						else if (dwIntegrityLevel >= SECURITY_MANDATORY_HIGH_RID)
						{
							// High Integrity
							result = false;
						}
					}
					LocalFree(pTIL);
				}
			}
		}
		CloseHandle(hToken);
		return result;
	}
CATCH_LOG()
	return true;
}*/

bool IsProcessIsolated()
{
	int result(true);
TRY_CATCH
	
	static const tstring testKey(GetGUID());
	static const tstring valueName(_T("testValue"));
	static const DWORD value = 12345;
	{
		/// Creating key
		HKEY hkey;
		DWORD disposition;
		DWORD res;
		if ((res = RegCreateKeyEx(HKEY_CURRENT_USER, testKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkey, &disposition)) != ERROR_SUCCESS)
		{
			SetLastError(res);
			throw MCException_Win("Failed to RegCreateKeyEx");
		}
		boost::shared_ptr<boost::remove_pointer<HKEY>::type> key(hkey,::RegCloseKey);
		if ((res = RegSetValueEx(hkey, valueName.c_str(), 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(value))) != ERROR_SUCCESS)
		{
			SetLastError(res);
			throw MCException_Win("Failed to RegSetValueEx");
		}
	}
	{
		/// Reading key
		HKEY hkey;
		DWORD res;
		if ((res = RegOpenKey(HKEY_CURRENT_USER, testKey.c_str(), &hkey)) != ERROR_SUCCESS)
		{
			SetLastError(res);
			throw MCException_Win("Failed to RegCreateKeyEx");
		}
		boost::shared_ptr<boost::remove_pointer<HKEY>::type> key(hkey,::RegCloseKey);
		DWORD type = REG_DWORD;
		DWORD size = sizeof(DWORD);
		DWORD value;
		if ((res = RegQueryValueEx(hkey, valueName.c_str(), 0, &type, reinterpret_cast<BYTE*>(&value), &size)) == ERROR_SUCCESS)
		{
			result = false;
		}
	}
	/// Erasing key
	int res;
	if ((res = RegDeleteKey(HKEY_CURRENT_USER, testKey.c_str())) != ERROR_SUCCESS)
	{
		SetLastError(res);
		Log.WinError(_ERROR_,_T("Failed to RegDeleteKey"));
	}

	return result;
CATCH_LOG("IsProcessIsolated")
	return true;
}

// CRCInstallerAXCtrl
//-------------------------------------------------------------------------------------

CRCInstallerAXCtrl::CRCInstallerAXCtrl()
	:	m_msgFireEventOtherThreadsLog(RegisterWindowMessage(_T("CRC_AXCtrl::FireEventOtherThreadsLog"))),//generate unique message code
		m_msgFireEventOtherThreadsFeatureInstalled(RegisterWindowMessage(_T("CRC_AXCtrl::FireEventOtherThreadsFeatureInstalled"))),//generate unique message code
		m_msgFireEventOtherThreadsInstalling(RegisterWindowMessage(_T("CRC_AXCtrl::FireEventOtherThreadsInstalling"))),
		m_GUIDProduct(PRODUCTGUIDSTR),
		m_GUIDCRCInstaller(INSTALLERGUIDSTR),
		m_thread(NULL)
{		
TRY_CATCH
	
	/// Initializing singletons
	CSingleton<CCrtExeptionHook>::instance();

	LOG_BUILDER_INSTANCE.AddRef();
	/// Reporting module initialized (before we've registered this as logger
	REPORT_MODULE(RCINSTALLER_NAME);
	m_instanceTracker.reset(new CInstanceTracker(_T("RCInstaller ActiveX")));

	m_ownLiveTime = true; //cLog member
	m_bWindowOnly=TRUE;//for fire event in other thread
	m_outProcInstance.instance=NULL;
	m_outProcInstance.connectionPoint=NULL;
	m_outProcInstance.cookieEvent=0;
	m_logMode=0;
	m_logAttributes=0;

	if(!m_msgFireEventOtherThreadsFeatureInstalled)
		throw MCException("Internal CRCHostAXCtrl::FireEventOtherThreadsFeatureInstalled messages do not registered");
	if(!m_msgFireEventOtherThreadsInstalling)
		throw MCException("Internal CRCHostAXCtrl::FireEventOtherThreadsInstalling messages do not registered");
	//+ external UI
	//TODO for getting FileInUse process use MsiSetExternalUIRecord function
	m_oldUI =	MsiSetExternalUI(	InstallUIHandler,
									INSTALLLOGMODE_PROGRESS|
									INSTALLLOGMODE_FATALEXIT|
									INSTALLLOGMODE_ERROR|
									INSTALLLOGMODE_WARNING|
									INSTALLLOGMODE_USER|
									//INSTALLLOGMODE_INFO|
									INSTALLLOGMODE_TERMINATE|
									INSTALLLOGMODE_FILESINUSE|
									INSTALLLOGMODE_ACTIONDATA|
									INSTALLLOGMODE_ACTIONSTART,
									reinterpret_cast<void*>(this));
	//- external UI
	//MessageBox(_T("CRCInstallerAXCtrl::CRCInstallerAXCtrl"));
CATCH_LOG("CRCInstallerAXCtrl::CRCInstallerAXCtrl")
}

void CRCInstallerAXCtrl::FinalRelease()
{
}

CRCInstallerAXCtrl::~CRCInstallerAXCtrl()
try
{
	//out of process the object instance
	ReleaseOutProcInstance();
	if(m_logMode)
		MsiEnableLog(0,NULL,0);
	MsiSetExternalUI(m_oldUI,0,0);

	/// Waiting for internal thread
	if (m_thread)
		if (WAIT_OBJECT_0 != WaitForSingleObject(m_thread, DEF_THREAD_WAIT_TIME))
			Log.WinError(_WARNING_,_T("Failed waiting for internal thread"));

	Log.Add(_MESSAGE_,_T("CRCInstallerAXCtrl::~CRCInstallerAXCtrl: Releasing logs..."));
#ifdef _DEBUG
	Log.UnRegisterLog(this);
#endif
	LOG_BUILDER_INSTANCE.Release();
}
catch(...)
{
}

LRESULT CRCInstallerAXCtrl::FireEventOtherThreadsLog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
	// No prologue and epilogue, since it will cause recursive calls
	NotifyLogMessage(static_cast<BSTR>(*reinterpret_cast<CComBSTR*>(wParam)),lParam);
	delete reinterpret_cast<CComBSTR*>(wParam);
	return 0;
}

void CRCInstallerAXCtrl::AddList(const cLog::cEventDesc &EventDesc, const TCHAR *Item, ...) throw( )
{
	try
	{
		// Exit if verbosity level is _NO_TRACE_ of high than defined level
		if( (_NO_TRACE_ == EventDesc.getVerbosity()) || (EventDesc.getVerbosity() >= _TRACE_CALLS_) )
			return;
		SYSTEMTIME SysTime;
		GetLocalTime(&SysTime);
		TCHAR Buf[MAX_PATH];
		tstring TimeStr;
		if(!GetTimeFormat(LOCALE_USER_DEFAULT,0,&SysTime,NULL,Buf,MAX_PATH))
			TimeStr += _T("Invalid time");
		else
			TimeStr += Buf;
		TimeStr += _T("> ");

		va_list vl;
		tstring str;
		for(va_start(vl, Item); Item; Item=va_arg(vl, PTCHAR))
		{
			str += Item;
		}
		va_end(vl);

		TimeStr += str;
		TimeStr+=_T(" ");
		if (EventDesc.getCallStack())
			TimeStr+=EventDesc.getCallStack();//call stack

		//TODO direct fire event, if it's failed, may be it are from other apartment(invoke event from main STA, from creating thread)
		if(NotifyLogMessage(CComBSTR(TimeStr.c_str()),EventDesc.getSeverity())!=S_OK)
			::PostMessage(m_hWnd,m_msgFireEventOtherThreadsLog, reinterpret_cast<WPARAM>(new CComBSTR(TimeStr.c_str())),
				EventDesc.getSeverity());
	}
	catch(...)
	{
	}
}
//-----------------------------------------------------------------------------------------


STDMETHODIMP CRCInstallerAXCtrl::GetServiceID(BSTR featureName,BSTR serviceName, BSTR* serviceGUID)
{
TRY_CATCH
	*serviceGUID=0; //To prevent cracshes in JS, setting out params to null
	USES_CONVERSION;
	//is owner feature installed through WI?
	DWORD useCount(0);
	WORD useDate(0);
	DWORD error;
	TCHAR productCode[GUID_LENGTH];
	if((error=MsiGetProductCode(m_GUIDCRCInstaller,productCode))!=ERROR_SUCCESS)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Product code getting is failed")),error);	
	if(MsiQueryFeatureState(productCode,OLE2T(featureName))!=INSTALLSTATE_LOCAL)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("The \"%s\" feature has been not installed."),OLE2T(featureName));
	//- Is the feature installed?
	//guid getting 
	CLSID clsid;
	if(CLSIDFromProgID(OLE2W(serviceName),&clsid)!=S_OK)
		throw MCException("Specific service not found");
	LPOLESTR olestr;
	if(StringFromCLSID(clsid,&olestr)!=S_OK)
		throw MCException("Converts a CLSID into a string failed");
	//TODO without CComBSTR, just olestr
	CComBSTR resstr(olestr);
	CoTaskMemFree(olestr);
	resstr.CopyTo(serviceGUID);
CATCH_LOG_COMERROR("CRCInstallerAXCtrl::GetServiceID")
}

STDMETHODIMP CRCInstallerAXCtrl::CancelInstalling(void)
{
TRY_CATCH
	m_cancelInstalling=IDCANCEL;
CATCH_LOG_COMERROR("CRCInstallerAXCtrl::InstallFeature")
}

HRESULT CRCInstallerAXCtrl::IsProcessUnderUIPI(BOOL* result)
{
TRY_CATCH
	*result = IsProcessIsolated();
CATCH_LOG_COMERROR("CRCInstallerAXCtrl::IsProcessUnderUIPI")
}

LRESULT CRCInstallerAXCtrl::FireEventOtherThreadsFeatureInstalled(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
TRY_CATCH
	NotifyInstalling(100 /*100%*/, CComBSTR(L"Complete"));
	NotifyFeatureInstalled(wParam);
	return 0;
CATCH_LOG_COMERROR("CRCInstallerAXCtrl::FireEventOtherThreadsFeatureInstalled")
}

LRESULT CRCInstallerAXCtrl::FireEventOtherThreadsInstalling(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
TRY_CATCH
	if (0 == lParam
		||
		0 == (reinterpret_cast<CComBSTR*>(lParam))->Length())
	{
		Log.Add(_WARNING_,_T("Empty installation state string received"));
		*reinterpret_cast<CComBSTR*>(lParam) = CComBSTR(L"Installing...");
	}
	NotifyInstalling(wParam,static_cast<BSTR>(*reinterpret_cast<CComBSTR*>(lParam)));
	delete reinterpret_cast<CComBSTR*>(lParam);
CATCH_LOG("CRCViewerAXCtrl::FireEventOtherThreadsConnecting")
	return 0;
}

void CRCInstallerAXCtrl::FireNotifyInstalling(int percentCompleted, const TCHAR* status)
{
TRY_CATCH
	//TODO direct fire event, if it's failed, may be it are from other apartment(invoke event from main STA, from creating thread)
	if (NULL == status
		||
		0 == _tcslen(status))
	{
		Log.Add(_WARNING_,_T("Empty installation state string received"));
		status = _T("Installing...");
	}
	if(NotifyInstalling(percentCompleted,CComBSTR(status))!=S_OK)
	{
		USES_CONVERSION;
		::PostMessage(m_hWnd,m_msgFireEventOtherThreadsInstalling,percentCompleted,reinterpret_cast<WPARAM>(new CComBSTR(T2W(status))));
	}
CATCH_LOG("CRCInstallerAXCtrl::FireNotifyInstalling")
}

STDMETHODIMP CRCInstallerAXCtrl::DirectConfigureProductEx(BSTR commandLine)
{
TRY_CATCH
	DWORD error;
	USES_CONVERSION;
	m_cancelInstalling = 0;
	m_progressTotal = ACTION_COUNT_GUESS;
	m_progressCurrent = 0;
	//+ last used source getting
	TCHAR productCode[GUID_LENGTH];
	TCHAR lastUsedSource[8192]; //8192 seems to be large enough buffer for MsiSourceListGetInfo f-n
	DWORD lastUsedSourceLength=_countof(lastUsedSource);
	if((error=MsiGetProductCode(m_GUIDCRCInstaller,productCode))!=ERROR_SUCCESS)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Product code getting is failed")),error);	
	//+ last used source getting
	// getting and invoke MsiSourceListGetInfo() function
	typedef UINT (WINAPI *MsiSourceListGetInfo_t)(LPCTSTR szProductCodeOrPatchCode, LPCTSTR szUserSid,DWORD dwContext,DWORD dwOptions,LPCTSTR szProperty,LPTSTR szValue,LPDWORD pcchValue);
	MsiSourceListGetInfo_t msiSourceListGetInfo=NULL;
	HMODULE hModule=LoadLibrary(_T("msi.dll"));
	boost::shared_ptr<boost::remove_pointer<HMODULE>::type> module(hModule,::FreeLibrary);
	if(msiSourceListGetInfo=reinterpret_cast<MsiSourceListGetInfo_t>(::GetProcAddress(hModule,
#ifdef UNICODE	
		"MsiSourceListGetInfoW"	
#else
		"MsiSourceListGetInfoA"
#endif // !UNICODE
		)))
	{
		if((error=msiSourceListGetInfo(productCode,NULL,MSIINSTALLCONTEXT_MACHINE,MSICODE_PRODUCT,INSTALLPROPERTY_LASTUSEDSOURCE,
				lastUsedSource,&lastUsedSourceLength))!=ERROR_SUCCESS)
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("LastUsedSource property getting is failed")),error);	
	}
	else
	{
		// if MsiSourceListGetInfo() is not found (WI version less v3.0)
		// get LastUsedSource from registry HKCR\Installer\\Products\\[product code complicated]\\SourceList 
		// val LastUsedSource=(REG_EXPAND_SZ)"u;1;http://max/iframe/bin/"
		// [product code complicated] is convert from prodactCode 
		// for example [product code complicated]=916C953B62536124AB94072259D3C0E8
		//							           productCode={B359C619-3526-4216-BA49-7022953D0C8E}
		// map array for convertion productCode string to [product code complicated] string
		int productCodeToRegMap[]={8,7,6,5,4,3,2,1,13,12,11,10,18,17,16,15,21,20,23,22,26,25,28,27,30,29,32,31,34,33,36,35};
		//	array for [product code complicated] string
		TCHAR productReg[_countof(productCodeToRegMap)+1];
		int i;
		for(i=0;i<_countof(productCodeToRegMap);++i)
			productReg[i]=productCode[productCodeToRegMap[i]];
		productReg[i]=_T('\0');
		// open reg key and get LastUsedSource register value
		tstring productKey;
		productKey+=tstring(_T("Installer\\Products\\"))+productReg+_T("\\SourceList");
		HKEY hkey;
		if((error=RegOpenKeyEx(HKEY_CLASSES_ROOT,productKey.c_str(),0,KEY_READ,&hkey))!=ERROR_SUCCESS)
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Opening key failed: ")+tstring(productKey),error);	
		boost::shared_ptr<boost::remove_pointer<HKEY>::type> key(hkey,::RegCloseKey);
		TCHAR lastUsedSourceUnexpanded[8192];
		TCHAR lastUsedSourceExpanded[8192];
		DWORD lastUsedSourceUnexpandedLength=sizeof(lastUsedSourceUnexpanded);
		DWORD valueType=REG_EXPAND_SZ;
		if((error=RegQueryValueEx(hkey,INSTALLPROPERTY_LASTUSEDSOURCE,0,&valueType,reinterpret_cast<BYTE*>(lastUsedSourceUnexpanded),
			&lastUsedSourceUnexpandedLength))!=ERROR_SUCCESS)
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("LastUsedSource value not found.")),error);	
		// extract Url from LastUsedSource register value
		if(!ExpandEnvironmentStrings(lastUsedSourceUnexpanded,lastUsedSourceExpanded,_countof(lastUsedSourceExpanded)-1))
			MCException_Win("ExpandEnvironmentStrings is failed");
		// string like "u;1;http://max/iframe/bin/" in register
		_tcscpy_s(lastUsedSource,_countof(lastUsedSource),_tcsrchr(lastUsedSourceExpanded,_T(';'))+1);
	}
	// package name getting for product SupportSpaceTools.msi
	TCHAR packageName[8192];
	DWORD packageNameLength=_countof(packageName);
	if((error=MsiGetProductInfo(productCode,INSTALLPROPERTY_PACKAGENAME,packageName,&packageNameLength))!=ERROR_SUCCESS)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Package name property getting is failed")),error);	
	
	tstring packagePath=tstring(lastUsedSource)+packageName;
	Log.Add(_MESSAGE_,_T("%s %s"),packagePath.c_str(),OLE2T(commandLine));
	//if((error=::MsiConfigureProductEx(m_GUIDProduct,INSTALLLEVEL_DEFAULT,INSTALLSTATE_DEFAULT,OLE2T(commandLine)))!=ERROR_SUCCESS)
	m_progressStatus.clear();
	TRY_CATCH
		if((error=MsiInstallProduct(packagePath.c_str(),OLE2T(commandLine)))!=ERROR_SUCCESS)
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring("Installation failed: ")+packagePath+tstring(OLE2T(commandLine)),error);
	CATCH_LOG_COMERROR("CRCInstallerAXCtrl::ConfigureProductEx");	
	
	return S_OK;

CATCH_LOG("CRCInstallerAXCtrl::DirectConfigureProductEx")
	NotifyFeatureInstalled(-1 /*error happened*/);
	return Error(L"CRCInstallerAXCtrl::DirectConfigureProductEx");//CComCoClass<>
}

STDMETHODIMP CRCInstallerAXCtrl::ConfigureProductEx(BSTR commandLine)
{
TRY_CATCH
	//TODO interthread synchronization!
	m_outProcInstance.commandLine=commandLine;
	//creation or getting pointer to out of process the object instance 
	CComPtr<IRCInstallerAXCtrl> instance=GetOutProcInstance();
	//UI Level
	INSTALLUILEVEL uiLevel;
	DWORD setuiLevel;
	uiLevel=MsiSetInternalUI(INSTALLUILEVEL_NOCHANGE,NULL);
	instance->SetInternalUI(uiLevel,&setuiLevel);
	//Logging
	if(m_logMode)
	{
		instance->EnableLog(m_logMode,m_logFile+_T("~"),m_logAttributes);
	}
	DWORD res;
	//outproc instance pointer marshal to other thread
	if((res=CoMarshalInterThreadInterfaceInStream(__uuidof(IRCInstallerAXCtrl),instance,&m_outProcInstance.stream))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring("Interthread marshaling failed"),res);

	/// Waiting for prev internal thread
	if (m_thread)
		if (WAIT_OBJECT_0 != WaitForSingleObject(m_thread, DEF_THREAD_WAIT_TIME))
			Log.WinError(_WARNING_,_T("Failed waiting for internal thread"));

	if((m_thread=CreateThread(NULL,0,OutProcConfigureProductThreadProc,this,0,/*&threadId*/NULL))==NULL)
		throw MCException_Win("Create thread for asynchronous call are failed");

	return S_OK;

CATCH_LOG("CRCInstallerAXCtrl::ConfigureProductEx")
	ReleaseOutProcInstance();
	NotifyFeatureInstalled(-1);
	return Error(L"CRCInstallerAXCtrl::ConfigureProductEx");//CComCoClass<>
}

DWORD WINAPI OutProcConfigureProductThreadProc(LPVOID lpParameter)
{	
	CRCInstallerAXCtrl *_this=NULL;
TRY_CATCH	
	CoInitializeEx(NULL,COINIT_MULTITHREADED);
	DWORD res;
	_this=reinterpret_cast<CRCInstallerAXCtrl*>(lpParameter);
	CComPtr<IRCInstallerAXCtrl> instance;
	if((res=CoGetInterfaceAndReleaseStream(_this->m_outProcInstance.stream,__uuidof(IRCInstallerAXCtrl),
		reinterpret_cast<void**>(&instance)))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring("Interthread unmarshaling are failed"),res);
	instance->DirectConfigureProductEx(_this->m_outProcInstance.commandLine);
CATCH_LOG("OutProcConfigureProductThreadProc")
TRY_CATCH
	if(!_this&&_this->NotifyFeatureInstalled(-1 /*error*/)!=S_OK)
		::PostMessage(_this->m_hWnd,_this->m_msgFireEventOtherThreadsFeatureInstalled,-1,0);
CATCH_LOG("OutProcConfigureProductThreadProc")
	CoUninitialize();
	return 0;
}

CComPtr<IRCInstallerAXCtrl> CRCInstallerAXCtrl::GetOutProcInstance(void)
{
TRY_CATCH
	//out of process instance has been created already
	if(NULL != m_outProcInstance.instance)
		return m_outProcInstance.instance;
	//out of process instance has not been created yet
	DWORD error;
	//+system version request
	OSVERSIONINFOEX osInf;
	osInf.dwOSVersionInfoSize=sizeof(osInf);
	if(!GetVersionEx((OSVERSIONINFO*)&osInf))
		throw MCException_Win("System Version request");
	//-system version request
	
	//+out of process instance creation
	if(osInf.dwMajorVersion>5)//Vista and higher
	{
		BIND_OPTS2 bo;
		memset(&bo,0,sizeof(bo));
		bo.cbStruct=sizeof(bo);
		//bo.hwnd=NULL;
		bo.dwClassContext=CLSCTX_LOCAL_SERVER;
		if((error=CoGetObject(L"Elevation:Administrator!new:{7B3BBD75-A77C-40D9-BD0E-943055093249}",&bo,__uuidof(IRCInstallerAXCtrl),
			(void**)&m_outProcInstance.instance))!=S_OK)
		{
			SetLastError(error);
			throw MCException_Win("Create out-of-process installer are failed due to COM Elevation Moniker ");
		}
	}
	else 
	{
		if((error=CoCreateInstance(GetObjectCLSID(),NULL,CLSCTX_LOCAL_SERVER/*CLSCTX_INPROC_SERVER*/,__uuidof(IRCInstallerAXCtrl),
			(void**)&m_outProcInstance.instance))!=S_OK)
		{
			SetLastError(error);
			throw MCException_Win("Create out-of-process installer are failed");
		}
	}
	//-out of process instance creation
	CComPtr<IConnectionPointContainer> cpc;
	if((error=m_outProcInstance.instance->QueryInterface(IID_IConnectionPointContainer,(void**)&cpc))!=S_OK)
	{	
		SetLastError(error);
		throw MCException_Win("Failed to query interface IID_IConnectionPointContainer from outproc RCInstaller ");
	}
	if((error=cpc->FindConnectionPoint(__uuidof(_IRCInstallerAXCtrlEvents),&m_outProcInstance.connectionPoint))!=S_OK)
	{	
		SetLastError(error);
		throw MCException_Win("Failed to find connection point _IRCInstallerAXCtrlEvents for outproc RCInstaller ");
	}
	if((error=m_outProcInstance.connectionPoint->Advise(static_cast<_IRCInstallerAXCtrlEvents*>(this),&m_outProcInstance.cookieEvent))!=S_OK)
	{	
		SetLastError(error);
		throw MCException_Win("Failed to advise _IRCInstallerAXCtrlEvents for outproc RCInstaller ");
	}
	//+event connection
	//m_outProcInstance.instance->StartWhatching(reinterpret_cast<ULONG>(m_hWnd));
	m_outProcInstance.instance->StartWhatching(GetCurrentProcessId());
	return m_outProcInstance.instance;
CATCH_THROW("CRCInstallerAXCtrl::GetOutProcInstance")
}

void CRCInstallerAXCtrl::ReleaseOutProcInstance(void)
{
TRY_CATCH
	//+out of process the object instance 
	if(m_outProcInstance.instance)
	{
		DWORD res;
		if(NULL != m_outProcInstance.connectionPoint)
		{
			/// Unadvising events
			if((res=m_outProcInstance.connectionPoint->Unadvise(m_outProcInstance.cookieEvent))!=S_OK)
			{
				tstring err = GetErrorInfoString();
				Log.WinError(_ERROR_,_T("Failed to unadvise from _IRCInstallerAXCtrlEvents for RCInstaller outproc instance. Error(%x) %s "), res, err.c_str());
			}
			m_outProcInstance.cookieEvent = 0;

			/// Releasing connection point
			m_outProcInstance.connectionPoint = NULL;
		}
		m_outProcInstance.instance = NULL;
	}
	//-out of process the object instance
CATCH_LOG("CRCInstallerAXCtrl::ReleaseOutProcInstance")
}



//TODO
//INT CALLBACK InstallUIHandlerRecord(LPVOID pvContext,UINT iMessageType,MSIHANDLE hRecord)
//{
//	CRCInstallerAXCtrl* pInstaller=reinterpret_cast<CRCInstallerAXCtrl*>(pvContext);
//	Log.Add(_MESSAGE_,_T("CRCInstallerAXCtrl::InstalluiHandler (0x%x)0x%x"),iMessageType,hRecord);
//	INSTALLMESSAGE mt = (INSTALLMESSAGE)(0xFF000000&(UINT)iMessageType);
//  UINT uiButtons_Icons = (0x00FFFFFF&(UINT)iMessageType);
//}

int FGetInteger(TCHAR*& rpch)
{
	TCHAR* pchPrev = rpch; 
  while (*rpch && *rpch != ' ')
      rpch++;
  *rpch = '\0';
  return _tstoi(pchPrev);
}

/// This method is taken directly from MSDN
INT CALLBACK CRCInstallerAXCtrl::InstallUIHandler(LPVOID pvContext,UINT iMessageType,LPCTSTR szMessage)
{
TRY_CATCH
	CRCInstallerAXCtrl* pInstaller=reinterpret_cast<CRCInstallerAXCtrl*>(pvContext);
	//Log.Add(_MESSAGE_,_T("CRCInstallerAXCtrl::InstalluiHandler (0x%x)%s"),iMessageType,szMessage);
	INSTALLMESSAGE mt = (INSTALLMESSAGE)(0xFF000000&(UINT)iMessageType);
	UINT uiButtons_Icons = (0x00FFFFFF&(UINT)iMessageType);
	switch (mt)
	{
	case INSTALLMESSAGE_PROGRESS:
		{
	//		int field[4]={-1,0,0,0};
	//		int parsedFields=0;
	//		//+parsing progress string
	//		char *pch =const_cast<LPSTR>(szMessage) ;
	//		if (0 == *pch)return pInstaller->m_cancelInstalling; // no msg
	//		while (*pch != 0)
	//		{
	//			char chField = *pch++;
	//			pch++; // for ':'
	//			pch++; // for sp
	//			switch (chField)
	//			{
	//			case '1': // field 1
	//				{	
	//					// progress message type
	//					if (0 == isdigit(*pch))goto endPars; // blank record
	//					field[0] = *pch++ - '0';
	//					parsedFields++;
	//					break;
	//				}
	//			case '2': // field 2
	//				{
	//					field[1] = FGetInteger(pch);
	//					parsedFields++;
	//					break;
	//				}
	//			case '3': // field 3
	//				{
	//					field[2] = FGetInteger(pch);
	//					parsedFields++;
	//					break;
	//				}
	//			case '4': // field 4
	//				{
	//					field[3] = FGetInteger(pch);
	//					parsedFields++;
	//					goto endPars; // done processing
	//				}
	//			default: // unknown field
	//				{
	//					field[0]=-1;
	//					goto endPars;
	//				}
	//			}
	//			pch++; // for space (' ') between fields
	//		}
	//endPars:	
	//		//+parsing progress string

	//		//handle only if all field are parsed
	//		if(parsedFields!=4)return pInstaller->m_cancelInstalling;

	//		// all fields off by 1 due to c array notation
	//		switch(field[0])
	//		{
	//		case 0: // reset progress bar
	//			{
	//				//field 1 = 0, field 2 = total number of ticks, field 3 = direction, field 4 = in progress
	//				pInstaller->m_progressTotal=field[1];
	//				pInstaller->m_progressCurrent=0;
	//				break;
	//			}
	//		case 1:
	//			{
	//				//field 1 = 1, field 2 will contain the number of ticks to increment the bar
	//				if(field[2]!=0)pInstaller->m_progressCurrent+=field[1];
	//				break;
	//			}
	//		case 2:
	//			{
	//				// only act if progress total has been initialized
	//				pInstaller->m_progressCurrent+=field[1];
	//				//::Sleep(500);
	//				break;
	//			}
	//		case 3: // fall through (we don't care to handle it -- total tick count adjustment)
	//			{
	//				// only act if progress total has been initialized
	//				pInstaller->m_progressTotal+=field[1];
	//				break;
	//			}
	//		default:
	//			{
	//				return pInstaller->m_cancelInstalling;
	//			}
	//		}
	//		if(pInstaller->m_progressTotal!=0)
	//		{
	//			//::Sleep(100);
	//			int progress=(double)pInstaller->m_progressCurrent*100/pInstaller->m_progressTotal;
	//			if(progress>=0&&progress<=100)
	//				pInstaller->FireNotifyInstalling(progress,pInstaller->m_progressStatus.c_str());
	//			else
	//			{	
	//				Log.Add(_MESSAGE_,_T("CRCInstallerAXCtrl::InstalluiHandler message(0x%x)%s"),iMessageType,szMessage);
	//				Log.Add(_MESSAGE_,_T("CRCInstallerAXCtrl::InstalluiHandler current=%d total=%d progress=%d"),pInstaller->m_progressCurrent,pInstaller->m_progressTotal,progress);
	//			}
	//		}
			return pInstaller->m_cancelInstalling;
		}
		// Sent after UI termination, no string data
	case INSTALLMESSAGE_TERMINATE:
		//TODO direct fire event, if it's failed, may be it are from other apartment(invoke event from main STA, from creating thread)
		BOOL handled;
		if (pInstaller->FireEventOtherThreadsFeatureInstalled(0,0,0,handled))
			::PostMessage(pInstaller->m_hWnd,pInstaller->m_msgFireEventOtherThreadsFeatureInstalled,0,0);
		Log.Add(_MESSAGE_,_T("CRCInstallerAXCtrl::InstalluiHandler INSTALLMESSAGE_TERMINATE %s"),szMessage);
		return pInstaller->m_cancelInstalling;
	case INSTALLMESSAGE_FILESINUSE:
		{
			Log.Add(_MESSAGE_,_T("CRCInstallerAXCtrl::InstalluiHandler (0x%x)%s"),iMessageType,szMessage);
			return ::MessageBox(pInstaller->m_hWnd,_T("\n Please, close all IE windows and push Retry buttom."),_T("File in use"),MB_RETRYCANCEL|MB_ICONQUESTION);
		}
	case INSTALLMESSAGE_ACTIONSTART:
		pInstaller->m_progressStatus = GetActionDescription(szMessage);
		pInstaller->FireNotifyInstalling(++pInstaller->m_progressCurrent,pInstaller->m_progressStatus.c_str());
		return pInstaller->m_cancelInstalling;
	case INSTALLMESSAGE_ACTIONDATA:
		return pInstaller->m_cancelInstalling;
	case INSTALLMESSAGE_FATALEXIT:
		{
			Log.Add(_ERROR_,_T("CRCInstallerAXCtrl::InstalluiHandler INSTALLMESSAGE_FATALEXIT %s"),szMessage);
			return IDOK;    
		}
	case INSTALLMESSAGE_ERROR:
		{
			Log.Add(_ERROR_,_T("CRCInstallerAXCtrl::InstalluiHandler INSTALLMESSAGE_ERROR %s"),szMessage);
			return IDOK;
		}        
	case INSTALLMESSAGE_WARNING:
		{
			Log.Add(_WARNING_,_T("CRCInstallerAXCtrl::InstalluiHandler INSTALLMESSAGE_WARNING %s"),szMessage);
			return IDOK;
		}
	case INSTALLMESSAGE_USER:
		{
			Log.Add(_MESSAGE_,_T("CRCInstallerAXCtrl::InstalluiHandler INSTALLMESSAGE_USER %s"),szMessage);
			return IDOK;
		}
	case INSTALLMESSAGE_INFO:
		{
			Log.Add(_MESSAGE_,_T("CRCInstallerAXCtrl::InstalluiHandler INSTALLMESSAGE_INFO %s"),szMessage);
			return IDOK;
		}
	default:
		//Log.Add(_MESSAGE_,_T("CRCInstallerAXCtrl::InstalluiHandler (0x%x 0x%x)%s"),iMessageType,INSTALLMESSAGE_PROGRESS,szMessage);  
		return pInstaller->m_cancelInstalling;
	}
CATCH_LOG("CRCInstallerAXCtrl::InstallUIHandler")
	return 0;
}
//typedef enum tagINSTALLUILEVEL
//{
//	INSTALLUILEVEL_NOCHANGE = 0,    // UI level is unchanged
//	INSTALLUILEVEL_DEFAULT  = 1,    // default UI is used
//	INSTALLUILEVEL_NONE     = 2,    // completely silent installation
//	INSTALLUILEVEL_BASIC    = 3,    // simple progress and error handling
//	INSTALLUILEVEL_REDUCED  = 4,    // authored UI, wizard dialogs suppressed
//	INSTALLUILEVEL_FULL     = 5,    // authored UI with wizards, progress, errors
//	INSTALLUILEVEL_ENDDIALOG    = 0x80, // display success/failure dialog at end of install
//	INSTALLUILEVEL_PROGRESSONLY = 0x40, // display only progress dialog
//	INSTALLUILEVEL_HIDECANCEL   = 0x20, // do not display the cancel button in basic UI
//	INSTALLUILEVEL_SOURCERESONLY = 0x100, // force display of source resolution even if quiet
//} INSTALLUILEVEL;

STDMETHODIMP CRCInstallerAXCtrl::SetInternalUI(ULONG dwUILevel, ULONG* dwOldUILevel)
{
TRY_CATCH	
	*dwOldUILevel=MsiSetInternalUI(INSTALLUILEVEL(dwUILevel), NULL);
	if(INSTALLUILEVEL_NOCHANGE!=dwUILevel&&INSTALLUILEVEL_NOCHANGE==*dwOldUILevel)
		throw MCException("Internal UI level setting are failed.");
CATCH_LOG_COMERROR("CRCInstallerAXCtrl::SetInternalUI")
}

HRESULT CRCInstallerAXCtrl::OnDraw(ATL_DRAWINFO& di)
{
TRY_CATCH		
	RECT& rc = *(RECT*)di.prcBounds;
	// Set Clip region to the rectangle specified by di.prcBounds
	HRGN hRgnOld = NULL;
	if (GetClipRgn(di.hdcDraw, hRgnOld) != 1)
		hRgnOld = NULL;
	bool bSelectOldRgn = false;

	HRGN hRgnNew = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);

	if (hRgnNew != NULL)
	{
		bSelectOldRgn = (SelectClipRgn(di.hdcDraw, hRgnNew) != ERROR);
	}
	// region leack in ATL realization
		if(hRgnNew!=NULL)
			DeleteObject(hRgnNew);

	Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
	SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
	LPCTSTR pszText = _T("ATL 7.0 : RCInstallerAXCtrl");
	TextOut(di.hdcDraw, 
		(rc.left + rc.right) / 2, 
		(rc.top + rc.bottom) / 2, 
		pszText, 
		lstrlen(pszText));

	if (bSelectOldRgn)
		SelectClipRgn(di.hdcDraw, hRgnOld);
	return S_OK;
CATCH_LOG("CRCInstallerAXCtrl::OnDraw")
	return E_FAIL;
}

//TODO: here and below, still needs to be reviewed and refactored

STDMETHODIMP CRCInstallerAXCtrl::EnableLog(ULONG logMode,BSTR logFile,ULONG logAttributes)
{
TRY_CATCH		
	m_logMode=logMode;
	m_logFile=logFile;
	m_logAttributes=logAttributes;
	//TODO check log by restricted user
	tstring logFName;
	TCHAR *pLogFName=NULL;
	if(logMode)//enable log
	{
		TCHAR modulePath[UNICODE_MAX_PATH]={0};
		if(!GetModuleFileName(_AtlBaseModule.m_hInst,modulePath,_countof(modulePath)))
			throw MCException_Win("GetModuleFileName() failed");
		logFName+=modulePath;
		try
		{
			logFName.resize(logFName.rfind(_T('\\')));
		}
		catch(std::length_error)
		{
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Module path is invalid: ")+logFName);
		}
		USES_CONVERSION;
		logFName+=tstring(_T("\\"))+OLE2T(logFile)+_T(".log");
		pLogFName=const_cast<TCHAR*>(logFName.c_str());
	}
	if(::MsiEnableLog(logMode,pLogFName,logAttributes)!=ERROR_SUCCESS)
		throw MCException("MSI log setting failed");
CATCH_LOG_COMERROR("CRCInstallerAXCtrl::EnableLog")
}

HRESULT CRCInstallerAXCtrl::FinalConstruct()
{
#ifdef _DEBUG
	try
	{
		Log.RegisterLog(this);
	}
	catch(...)
	{
	}
#endif
	return S_OK;
}

STDMETHODIMP CRCInstallerAXCtrl::StartWhatching(ULONG pid)
{
TRY_CATCH
	m_watchDog.AddClient(pid);
CATCH_LOG_COMERROR()
}
