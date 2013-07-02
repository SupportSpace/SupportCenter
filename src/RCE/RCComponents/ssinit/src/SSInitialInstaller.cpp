/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SSInitialInstaller.cpp
///
///  CSSInitialInstaller, initial installation object 
///
///  @author "Archer Software" Kirill Solovyov. @date 22.03.2007
///
////////////////////////////////////////////////////////////////////////
// SSInitialInstaller.cpp : Implementation of CSSInitialInstaller
#include "stdafx.h"
#include "cleanoc.h"



tstring a2tstring(const char* str)
{
try
{
	if (str == NULL)
		return _T("NULL");
	USES_CONVERSION;
	return A2T(str);
}
catch(...)
{
	return _T("a2tstring: failed to A2T");
}
}

std::string tstring2string(const tstring &str)
try
{
	USES_CONVERSION;
	return T2A(str.c_str());
}
catch(...)
{
	return "tstring2string: failed to T2A";
}

// 1. find /L (not /l) and change log name file to "exe module path" + "log file name"
tstring& CorrectParams(tstring& params)
{
	tstring::size_type index;
	//1. full log file name
	if(tstring::npos!=(index=params.find(_T("/L"))))
	{
		tstring::size_type paramsLen=params.length();
		if(tstring::npos!=(index=params.find(_T(' '),index)))
		{
			for(;index<paramsLen&&params[index]==_T(' ');++index);
			if(index<paramsLen)
			{
				TCHAR modulePath[32768]={0};
				DWORD result;
				if((result=GetModuleFileName(NULL,modulePath,_countof(modulePath)))!=NULL&&result!=_countof(modulePath))
				{
					TCHAR *p=_tcsrchr(modulePath,_T('\\'));
					if(p!=NULL&&p<modulePath+_countof(modulePath))
					{
						*(p+1)=_T('\0');

						// name of log file enclosed by qoutation marks
						if(params[index]==_T('\"'))
							params.insert(index+1,tstring(modulePath));
						else
						{
							tstring::size_type indexEOFName;
							if(tstring::npos!=(indexEOFName=params.find(_T(' '),index)))
								params.insert(indexEOFName,_T("\""));
							else
								params.append(_T("\""));
							params.insert(index,_T('\"')+tstring(modulePath));
						}
					}
				}
			}
		}
	}
	return params;
}

CSSInitialInstaller::CSSInitialInstaller()
{
}

CSSInitialInstaller::~CSSInitialInstaller()
{
}

// CSSInitialInstaller
HRESULT CSSInitialInstaller::OnDraw(ATL_DRAWINFO& di)
{
	/// We shouldn't draw anything
	return S_OK;
}

HRESULT CSSInitialInstaller::FinalConstruct()
{
	return S_OK;
}

void CSSInitialInstaller::FinalRelease()
{
}


STDMETHODIMP CSSInitialInstaller::Install(ULONG base, BSTR url, BSTR params, ULONG flags,ULONG* msiResult)
try
{
	*msiResult=0;
	if(flags&IF_FREE_LIB)
		CoFreeUnusedLibraries();
	if(!(flags&IF_NOINSTALL))
	{
		tstring baseUrl;
		switch(base)
		{
		case 0: //codebase
			baseUrl=GetCodeBaseUrl();
			break;
		case 1:	//page
			baseUrl=GetPageUrl();
			break;
		default:
			throw std::exception("\"base\" parameter is invalid");
		}
		//+ url only, not file
		try
		{
			baseUrl.resize(baseUrl.rfind(_T('/')));
		}
		catch(std::length_error &e)
		{
			std::string message("Base url did not found or is invalid: ");
			throw std::exception((message+tstring2string(baseUrl)).c_str());
		}
		baseUrl+=_T("/");
		TCHAR appName[]=_T("msiexec.exe");
		USES_CONVERSION;
		tstring cmdLine=_T("/i ")+baseUrl+OLE2T(url)+_T(" ")+CorrectParams(tstring(OLE2T(params)));
		if(flags&IF_SHOW_CMD)
			::MessageBox(NULL,cmdLine.c_str(),_T("Command line"),0); //This messagebox is just mean, to simplyfy JS debugging. 
															 //It will never be shown for usual calls
		//+system version request
		OSVERSIONINFOEX osInf;
		osInf.dwOSVersionInfoSize=sizeof(osInf);
		if(!GetVersionEx((OSVERSIONINFO*)&osInf))
			throw std::exception("System version obtainning failed");
		//-system version request
		HANDLE hProcess=NULL;	
		//+out of process instance creation
		if(IsUserAnAdmin() && osInf.dwMajorVersion < 6/*For vista we need to elevate even for admin*/)
		{
			STARTUPINFO si={sizeof(si)};
			GetStartupInfo(&si);
			PROCESS_INFORMATION pi;
			if(!CreateProcess(NULL,const_cast<LPTSTR>((tstring(appName)+_T(" ")+cmdLine).c_str()),NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,
												NULL,NULL,&si,&pi))
				throw std::exception("CreateProcess failed");
			CloseHandle(pi.hThread);
			hProcess=pi.hProcess;
		}
		else
		{
			SHELLEXECUTEINFO shellInfo={sizeof(shellInfo),SEE_MASK_NOCLOSEPROCESS,0,_T("runas"),appName,cmdLine.c_str(),NULL,SW_SHOWNORMAL,0,NULL,NULL,
																	0,0,0,0};
			if(!ShellExecuteEx(&shellInfo))
				throw std::exception("Process starting failed with elevated privileges");
			hProcess=shellInfo.hProcess;
		}
		//synchronouse call
		if(flags&IF_SYNC)
			WaitForSingleObject(hProcess,INFINITE);
		GetExitCodeProcess(hProcess,msiResult);
		CloseHandle(hProcess);
	}
	//re-launch IE
	if(flags&IF_RELAUNCH)
	{
		tstring pageUrl(GetPageUrl());
		STARTUPINFO si={sizeof(si)};
		GetStartupInfo(&si);
		PROCESS_INFORMATION pi;
		TCHAR buffer[8192];
		if(!GetModuleFileName(NULL,buffer,_countof(buffer)))
			throw std::exception("Browser module detection failed");
		if(!CreateProcess(NULL,const_cast<LPTSTR>((tstring(_T("\""))+buffer+_T("\" \"")+pageUrl+_T("\"")).c_str()),NULL,NULL,FALSE,
											NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi))
			throw std::exception("Browser starting failed");
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return S_OK;
}
catch(std::exception &e)
{
	tstring error(_T("CSSInitialInstaller::Install failed: "));
	error+=a2tstring(e.what());
	// TODO Add function with win error
	Add(GetLastError(),error.c_str());
	return Error(error.c_str());
}

tstring CSSInitialInstaller::GetPageUrl()
{	
	//TODO: Make all errors to have common style. I.e. or can't or failed.
	USES_CONVERSION;
	CComPtr<IOleClientSite> clientSite;
	if(GetClientSite((IOleClientSite **)&clientSite)!=S_OK)
		throw std::exception("IOleClientSite obtaining failed");
	CComPtr<IServiceProvider> srvProv;
	if(clientSite->QueryInterface(IID_IServiceProvider,(void**)&srvProv)!=S_OK)
		throw std::exception("IServiceProvider obtaining failed");
	CComPtr<IWebBrowser2> webBrowser;
	if(srvProv->QueryService(IID_IWebBrowserApp,IID_IWebBrowser2,(void **)&webBrowser)!=S_OK)
		throw std::exception("IWebBrowser2 obtaining failed");
	BSTR bstrURL;
	if(webBrowser->get_LocationURL(&bstrURL)!=S_OK)
		throw std::exception("IWebBrowser2 url obtaining failed");
	OLECHAR wzDecodedUrl[INTERNET_MAX_URL_LENGTH];
	DWORD cchDecodedUrl=INTERNET_MAX_URL_LENGTH;
	if(CoInternetParseUrl(bstrURL,PARSE_CANONICALIZE,ICU_DECODE,wzDecodedUrl,cchDecodedUrl,&cchDecodedUrl,0)!=S_OK)
		throw std::exception("Url canonicalization failed");
	return W2T(wzDecodedUrl);
}

tstring CSSInitialInstaller::GetCodeBaseUrl()
{
	USES_CONVERSION;
	char urlA[INTERNET_MAX_URL_LENGTH];
	LPOLESTR clsidOle;
	if(StringFromCLSID(GetObjectCLSID(),&clsidOle)!=S_OK)
		throw std::exception("String from CLSID obtaining failed");
	char clsid[39];
	strcpy_s(clsid,OLE2A(clsidOle));
	CoTaskMemFree(clsidOle);
	// because occeche.dll functions is ANSI
	GetCodeBaseUrlFromCLSIDstr(urlA,sizeof(urlA),clsid,sizeof(clsid));	
	return A2T(urlA);
}


STDMETHODIMP CSSInitialInstaller::GetComponentVersion(BSTR componentGUID, BSTR productKey, SHORT keyType, BSTR* version)
{
	tstring error(_T("CSSInitialInstaller::GetComponentVersion failed: "));
	try
	{
		CVersionReader versionReader(productKey, static_cast<CVersionReader::ESearchKeyType>(keyType));
		CComBSTR(versionReader.GetComponentVersion(componentGUID).c_str()).CopyTo(version);
		return S_OK;
	}
	catch(tstring& errorString)
	{
		error += errorString;
	}
	catch(std::exception exception)
	{
		error += a2tstring(exception.what());
	}
	catch(...)
	{
		error = _T("Unknown");
	}
	Add(GetLastError(),error.c_str());
	CComBSTR(L"").CopyTo(version);
	return Error(error.c_str());
}

void CSSInitialInstaller::AddList(const tstring errorMessage)
try
{
	OutputDebugString(errorMessage.c_str());
}
catch(...)
{
}
