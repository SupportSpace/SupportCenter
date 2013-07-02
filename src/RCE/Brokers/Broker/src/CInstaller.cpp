/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CInstaller.cpp
///
///  CInstaller object implementation
///
///  @author Kirill Solovyov @date 28.01.2008
///
////////////////////////////////////////////////////////////////////////

#include "CInstaller.h"
#include <AidLib/CException/CException.h>
#include <boost/shared_ptr.hpp>
#include <boost\type_traits\remove_pointer.hpp>
#include <AidLib/Logging/CLogFolder.h>
#pragma comment (lib, "Msi.lib")

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


CInstaller::CInstaller(void):
	m_GUIDCRCInstaller(_T("{2FF5923D-5B0C-4EAB-8CF7-7CC79F1A627E}"))
{
TRY_CATCH
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
CATCH_LOG()
}

CInstaller::~CInstaller(void)
{
TRY_CATCH
	if(m_logMode)
		MsiEnableLog(0,NULL,0);
	MsiSetExternalUI(m_oldUI,0,0);
CATCH_LOG()
}

UINT CInstaller::DirectConfigureProductEx(const tstring& commandLine)
{
TRY_CATCH
	m_result=-1;//initialization internal result code
	DWORD error;
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
	Log.Add(_MESSAGE_,_T("%s %s"),packagePath.c_str(),commandLine.c_str());
	//if((error=::MsiConfigureProductEx(m_GUIDProduct,INSTALLLEVEL_DEFAULT,INSTALLSTATE_DEFAULT,OLE2T(commandLine)))!=ERROR_SUCCESS)
	m_progressStatus.clear();
	if((error=MsiInstallProduct(packagePath.c_str(),commandLine.c_str()))!=ERROR_SUCCESS)
		MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Installation failed: "))+packagePath+commandLine,error));
	if(-1==m_result)
	{
		m_result=error;
		OnInstalled(m_result);
	}
	return m_result;
CATCH_LOG()
	OnInstalled(-1 /*error happened*/);
	return -1;
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

tstring CInstaller::GetActionDescription(const tstring& actionString)
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

int FGetInteger(TCHAR*& rpch)
{
	TCHAR* pchPrev = rpch; 
  while (*rpch && *rpch != ' ')
      rpch++;
  *rpch = '\0';
  return _tstoi(pchPrev);
}

/// This method is taken directly from MSDN
INT CALLBACK CInstaller::InstallUIHandler(LPVOID pvContext,UINT iMessageType,LPCTSTR szMessage)
{
TRY_CATCH
	CInstaller* pInstaller=reinterpret_cast<CInstaller*>(pvContext);
	//Log.Add(_MESSAGE_,_T("CCoBPInstaller::InstalluiHandler (0x%x)%s"),iMessageType,szMessage);
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
	//				Log.Add(_MESSAGE_,_T("CCoBPInstaller::InstalluiHandler message(0x%x)%s"),iMessageType,szMessage);
	//				Log.Add(_MESSAGE_,_T("CCoBPInstaller::InstalluiHandler current=%d total=%d progress=%d"),pInstaller->m_progressCurrent,pInstaller->m_progressTotal,progress);
	//			}
	//		}
			return pInstaller->m_cancelInstalling;
		}
		// Sent after UI termination, no string data
	case INSTALLMESSAGE_TERMINATE:
		//TODO direct fire event, if it's failed, may be it are from other apartment(invoke event from main STA, from creating thread)
		BOOL handled;
		//TODO
		//if (pInstaller->FireEventOtherThreadsFeatureInstalled(0,0,0,handled))
		//	::PostMessage(pInstaller->m_hWnd,pInstaller->m_msgFireEventOtherThreadsFeatureInstalled,0,0);
		Log.Add(_MESSAGE_,_T("CCoBPInstaller::InstalluiHandler INSTALLMESSAGE_TERMINATE %s"),szMessage);
		return pInstaller->m_cancelInstalling;
	case INSTALLMESSAGE_FILESINUSE:
		{
			Log.Add(_MESSAGE_,_T("CCoBPInstaller::InstalluiHandler (0x%x)%s"),iMessageType,szMessage);
			//TODO
			//return ::MessageBox(pInstaller->m_hWnd,_T("\n Please, close all IE windows and push Retry buttom."),_T("File in use"),MB_RETRYCANCEL|MB_ICONQUESTION);
			//return ::MessageBox(NULL,_T("\n Please, close all IE windows and push Retry buttom."),_T("File in use"),MB_RETRYCANCEL|MB_ICONQUESTION);
			return ::MessageBox(NULL,_T("\n Please, close all IE windows and push Retry buttom."),_T("File in use"),MB_ABORTRETRYIGNORE|MB_ICONQUESTION);
		}
	case INSTALLMESSAGE_ACTIONSTART:
		pInstaller->m_progressStatus = GetActionDescription(szMessage);
		pInstaller->OnInstalling(++pInstaller->m_progressCurrent,pInstaller->m_progressStatus.c_str());
		return pInstaller->m_cancelInstalling;
	case INSTALLMESSAGE_ACTIONDATA:
		return pInstaller->m_cancelInstalling;
	case INSTALLMESSAGE_FATALEXIT:
		{
			Log.Add(_ERROR_,_T("CCoBPInstaller::InstalluiHandler INSTALLMESSAGE_FATALEXIT %s"),szMessage);
			return IDOK;    
		}
	case INSTALLMESSAGE_ERROR:
		{
			Log.Add(_ERROR_,_T("CCoBPInstaller::InstalluiHandler INSTALLMESSAGE_ERROR %s"),szMessage);
			return IDOK;
		}        
	case INSTALLMESSAGE_WARNING:
		{
			Log.Add(_WARNING_,_T("CCoBPInstaller::InstalluiHandler INSTALLMESSAGE_WARNING %s"),szMessage);
			return IDOK;
		}
	case INSTALLMESSAGE_USER:
		{
			Log.Add(_MESSAGE_,_T("CCoBPInstaller::InstalluiHandler INSTALLMESSAGE_USER %s"),szMessage);
			return IDOK;
		}
	case INSTALLMESSAGE_INFO:
		{
			Log.Add(_MESSAGE_,_T("CCoBPInstaller::InstalluiHandler INSTALLMESSAGE_INFO %s"),szMessage);
			return IDOK;
		}
	default:
		//Log.Add(_MESSAGE_,_T("CCoBPInstaller::InstalluiHandler (0x%x 0x%x)%s"),iMessageType,INSTALLMESSAGE_PROGRESS,szMessage);  
		return pInstaller->m_cancelInstalling;
	}
CATCH_LOG()
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

void CInstaller::SetInternalUI(ULONG dwUILevel, ULONG* dwOldUILevel)
{
TRY_CATCH	
	*dwOldUILevel=MsiSetInternalUI(INSTALLUILEVEL(dwUILevel), NULL);
	if(INSTALLUILEVEL_NOCHANGE!=dwUILevel&&INSTALLUILEVEL_NOCHANGE==*dwOldUILevel)
		throw MCException("Internal UI level setting are failed.");
CATCH_THROW()
}

void CInstaller::EnableLog(ULONG logMode,const tstring& logFile,ULONG logAttributes)
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
		logFName=Format(_T("%s\\%s.log"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str(),logFile.c_str());
		pLogFName=const_cast<TCHAR*>(logFName.c_str());
	}
	if(::MsiEnableLog(logMode,pLogFName,logAttributes)!=ERROR_SUCCESS)
		throw MCException("MSI log setting failed");
CATCH_THROW()
}
