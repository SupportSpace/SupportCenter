/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCoBrokerProxy.cpp
///
///  CCoBrokerProxy ActiveX object implementation
///
///  @author Kirill Solovyov @date 04.10.2007
///
////////////////////////////////////////////////////////////////////////
// CCoBrokerProxy.cpp : Implementation of CCoBrokerProxy
#include "stdafx.h"
//#include <atlsafe.h>
#include "CCoBrokerProxy.h"
//#include <AidLib/CException/CException.h>
#include <AidLib/Com/ComException.h>
#include <AidLib/Strings/tstring.h>
//#include <AidLib/CScopedTracker/CScopedTracker.h>
#include <AidLib/Utils/Utils.h>
#include <RCEngine/AXstuff/AXstuff.h>
#include "..\..\Shared\BrokersTypes.h"
#include <Shlobj.h>
#include <HelperService/CSrvComm.h>
#include <AidLib/Logging/CLogFolder.h>
#include <AidLib/CCrypto/CCrypto.h>

#include "..\..\Shared\CAvailableServices.h"
#include "cleanoc.h"
#include <math.h>

#include <msi.h>
#pragma comment (lib, "Msi.lib")




// CCoBrokerProxy

#define DEF_FILE_LOG_NAME _T("SupportSpaceProxyBroker.log")

//#define BROKER_GUID _T("{2FF5923D-5B0C-4EAB-8CF7-7CC79F1A627E}")
//WNDPROC CCoBrokerProxy::oldWndWindowProc=NULL;

CCoBrokerProxy::CCoBrokerProxy():
	CInstanceTracker(_T("CCoBrokerProxy")),
	m_installerRId(0),
	m_installing(false),
	oldWndWindowProc(NULL),
	m_msgCreateBroker(RegisterWindowMessage(_T("CCoBrokerProxy::m_msgCreateBroker"))), //generate unique message code
	m_msgFireNotifyLogMessage(RegisterWindowMessage(_T("CCoBrokerProxy::m_msgFireNotifyLogMessage"))), //generate unique message code
	m_msgFireRequestSent(RegisterWindowMessage(_T("CCoBrokerProxy::m_msgFireRequestSent"))) //generate unique message code
{
TRY_CATCH
	m_ownLiveTime = true; //cLog member
	m_bWindowOnly=TRUE;
#ifdef _DEBUG
	//::MessageBox(NULL,_T("CCoBrokerProxy::CCoBrokerProxy()"),NULL,0);
#endif

CATCH_LOG()
}

CCoBrokerProxy::~CCoBrokerProxy()
{
TRY_CATCH
#ifdef _DEBUG
	//::MessageBox(NULL,_T("CCoBrokerProxy::~CCoBrokerProxy()"),NULL,0);
#endif
CATCH_LOG()
}

HRESULT CCoBrokerProxy::FinalConstruct()
{
TRY_CATCH
		m_wnd.Create(_T("STATIC"),HWND_MESSAGE);
		m_wnd.SetWindowLong(GWL_USERDATA,reinterpret_cast<LONG>(this));
		oldWndWindowProc=reinterpret_cast<WNDPROC>(m_wnd.SetWindowLong(GWL_WNDPROC,reinterpret_cast<LONG>(WndWindowProc)));

		CSingleton<CCrtExeptionHook>::instance();
		Log.RegisterLog(this);
		Log.RegisterLog(new cFileLog(Format(_T("%s\\%s"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str(),DEF_FILE_LOG_NAME).c_str()));
		//Log.RegisterLog(new cFileLog(Format(_T("%s\\%s"),GetSpecialFolderPath(CSIDL_PERSONAL).c_str(),DEF_FILE_LOG_NAME).c_str()));
		/// Reporting module initialized (before we've registered this as logger
		REPORT_MODULE("BrokerProxy");
		//m_instanceTracker.reset(new CInstanceTracker(_T("RCInstaller ActiveX")));
CATCH_LOG()
	return S_OK;

}

void CCoBrokerProxy::FinalRelease()
{
TRY_CATCH
#ifdef _DEBUG
	//::MessageBox(NULL,_T("CCoBrokerProxy::FinalRelease()"),NULL,0);
#endif
	if(m_brokerEvents.p)
	{
		HRESULT result;
		if((result=m_brokerEvents->EventUnadvise())!=S_OK)
			MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("C_ICoBrokerEvents binding (EventUnadvise) failed")),result));
		m_brokerEvents->m_owner=NULL;
		m_brokerEvents.Release();
	}
	m_broker.Release();
	//m_installer.Release();
	Log.Add(_MESSAGE_,_T("CCoBrokerProxy::FinalRelease()"));
	Log.UnRegisterLog(this);
	m_wnd.SetWindowLong(GWL_USERDATA,0);
	m_wnd.SetWindowLong(GWL_WNDPROC,reinterpret_cast<LONG>(oldWndWindowProc));
	m_wnd.DestroyWindow();
CATCH_LOG()
}

// multy thread event call
STDMETHODIMP CCoBrokerProxy::Advise(IUnknown* pUnkSink,DWORD* pdwCookie)
{
	//T* pT = static_cast<T*>(this);
	CCoBrokerProxy* pT = static_cast<CCoBrokerProxy*>(this);
	IUnknown* p;
	HRESULT hRes = S_OK;
	if (pdwCookie != NULL)
		*pdwCookie = 0;
	if (pUnkSink == NULL || pdwCookie == NULL)
		return E_POINTER;
	IID iid;
	IConnectionPointImpl<CCoBrokerProxy, &__uuidof(::_ICoBrokerProxyEvents), CComDynamicUnkArray>::GetConnectionInterface(&iid);
	hRes = pUnkSink->QueryInterface(iid, (void**)&p);
	if (SUCCEEDED(hRes))
	{
		//pT->Lock();
		//*pdwCookie = m_vec.Add(p);
		pT->Lock();
		*pdwCookie = IConnectionPointImpl<CCoBrokerProxy, &__uuidof(::_ICoBrokerProxyEvents), CComDynamicUnkArray>::m_vec.Add(reinterpret_cast<IUnknown*>(CComGITPtr<IUnknown>(p).Detach()));
		hRes = (*pdwCookie != NULL) ? S_OK : CONNECT_E_ADVISELIMIT;
		pT->Unlock();
		//if (hRes != S_OK)
			p->Release();
	}
	else if (hRes == E_NOINTERFACE)
		hRes = CONNECT_E_CANNOTCONNECT;
	if (FAILED(hRes))
		*pdwCookie = 0;
	return hRes;
}

STDMETHODIMP CCoBrokerProxy::Unadvise(DWORD dwCookie)
{
	//::MessageBox(NULL,_T("IConnectionPointImpl<CCoVBroker, &__uuidof(::_ICoVBrokerEvents), CComDynamicUnkArray>::Unadvise"),NULL,0);
	//T* pT = static_cast<T*>(this);
	CCoBrokerProxy* pT = static_cast<CCoBrokerProxy*>(this);
	pT->Lock();
	IUnknown* p = IConnectionPointImpl<CCoBrokerProxy, &__uuidof(::_ICoBrokerProxyEvents), CComDynamicUnkArray>::m_vec.GetUnknown(dwCookie);
	CComGITPtr<IUnknown>(reinterpret_cast<DWORD>(p));
	HRESULT hRes = IConnectionPointImpl<CCoBrokerProxy, &__uuidof(::_ICoBrokerProxyEvents), CComDynamicUnkArray>::m_vec.Remove(dwCookie) ? S_OK : CONNECT_E_NOCONNECTION;
	pT->Unlock();
	//if (hRes == S_OK && p != NULL)
	//	p->Release();
	return hRes;
}


HRESULT CCoBrokerProxy::__ComInvokeEventHandler(IDispatch* pDispatch, DISPID id, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult)
{
	//CComGITPtr<IUnknown> sink(reinterpret_cast<DWORD>(pDispatch));
	//CComPtr<IUnknown> unkn;
	//sink.CopyTo(&unkn);
	//sink.Detach();
	//CComPtr<IDispatch> disp;
	//unkn.QueryInterface(&disp);

	CComPtr<IGlobalInterfaceTable> git;
	HRESULT hr;
	hr = ::CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IGlobalInterfaceTable),(void**)&git);
	//CComPtr<IDispatch> disp;
	//git->GetInterfaceFromGlobal(reinterpret_cast<DWORD>(pDispatch),IID_IDispatch,(void**)&disp);
	CComPtr<IUnknown> unkn;
	git->GetInterfaceFromGlobal(reinterpret_cast<DWORD>(pDispatch),IID_IUnknown,(void**)&unkn);
	CComPtr<IDispatch> disp;
	unkn->QueryInterface(&disp);
	return ::__ComInvokeEventHandler(disp, id, wFlags, pDispParams, pVarResult);
}
void CCoBrokerProxy::AddList(const cLog::cEventDesc &EventDesc, const TCHAR *Item, ...) throw( )
{
	try
	{
		// Exit if verbosity level is _NO_TRACE_ of high than defined level
		if( (_NO_TRACE_ == EventDesc.getVerbosity()) || (EventDesc.getVerbosity() >= _TRACE_CALLS_) )
			return;
		SYSTEMTIME SysTime;
		GetLocalTime(&SysTime);
		TCHAR Buf[MAX_PATH];
		tstring TimeStr(_T("Proxy"));
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

		PostMessage(m_msgFireNotifyLogMessage, reinterpret_cast<WPARAM>(new CComBSTR(TimeStr.c_str())),EventDesc.getSeverity());
	}
	catch(...)
	{
	}
}

LRESULT CCoBrokerProxy::FireNotifyLogMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	try
	{
		// No prologue and epilogue, since it will cause recursive calls
		NotifyLogMessage(static_cast<BSTR>(*reinterpret_cast<CComBSTR*>(wParam)),lParam);
		delete reinterpret_cast<CComBSTR*>(wParam);
	}
	catch(...)
	{
	}
	return 0;
}

void CCoBrokerProxy::FireRequestSent(BSTR dstUserId,ULONG dstSvcId,BSTR srcUserId,ULONG srcSvcId,ULONG rId,ULONG rType,ULONG param,BSTR params)
{
TRY_CATCH
	USES_CONVERSION;
	PostMessage(m_msgFireRequestSent,reinterpret_cast<WPARAM>(new SRequest(_OLE2T(dstUserId),dstSvcId,_OLE2T(srcUserId),srcSvcId,rId,rType,param,_OLE2T(params))),NULL);
CATCH_THROW()
}

LRESULT CCoBrokerProxy::FireRequestSent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
		std::auto_ptr<SRequest> req(reinterpret_cast<SRequest*>(wParam));
		RequestSent(CComBSTR(req->m_dstUserId.c_str()),req->m_dstSvcId,CComBSTR(req->m_srcUserId.c_str()),req->m_srcSvcId,req->m_rId,req->m_rType,
		            req->m_param,CComBSTR(req->m_params.c_str()));
CATCH_LOG()
	return 0;
}


bool IsInstallOff(void)
{
TRY_CATCH
	HANDLE h;
	if(INVALID_HANDLE_VALUE!=(h=CreateFile(Format(_T("%s\\%s"),GetModulePath(GetCurrentModule()).c_str(),_T("install.off")).c_str(),0/*query*/,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL)))
	{
		CloseHandle(h);
		return true;
	}
	return false;
CATCH_THROW()
}

tstring GetCodeBaseUrl(const IID& _clsid)
{
TRY_CATCH
	USES_CONVERSION;
	char urlA[INTERNET_MAX_URL_LENGTH];
	LPOLESTR clsidOle;
	if(StringFromCLSID(_clsid,&clsidOle)!=S_OK)
		throw MCException("String from CLSID obtaining failed");
	char clsid[39];
	strcpy_s(clsid,OLE2A(clsidOle));
	CoTaskMemFree(clsidOle);
	// because occeche.dll functions is ANSI
	//if(clsidString.length()!=_countof(clsid)-1)
	//	throw MCException("Invalid parameter clsidString");
	//strcpy_s(clsid,CT2CA(clsidString.c_str()));
	GetCodeBaseUrlFromCLSIDstr(urlA,sizeof(urlA),clsid,sizeof(clsid));
	//OLECHAR wzDecodedUrl[INTERNET_MAX_URL_LENGTH];
	//DWORD cchDecodedUrl=INTERNET_MAX_URL_LENGTH;
	//if(CoInternetParseUrl(A2W(urlA),PARSE_CANONICALIZE,ICU_DECODE,wzDecodedUrl,cchDecodedUrl,&cchDecodedUrl,0)!=S_OK)
	//	throw std::exception("Url canonicalization failed");
	//return W2T(wzDecodedUrl);
	char *end;
	end=strrchr(urlA,'/');
	if(!end||end>urlA+INTERNET_MAX_URL_LENGTH-2)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Codebase url format is invalid: %s"),A2T(urlA));
	end[1]='\0';
	return A2T(urlA);
CATCH_THROW()
}

tstring GetToolServiceFileName(EBrokerServicesTypes type)
{
TRY_CATCH
	//tstring version;
	HRESULT result;
	tstring keyStr=Format(_T("CLSID\\{%s}\\InprocServer32"),AVAILABLE_SERVICES[type].m_GUID.c_str());
	CRegKey key;
	if(ERROR_SUCCESS!=(result=key.Open(HKEY_CLASSES_ROOT,keyStr.c_str(), KEY_READ)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Open key=[%s] failed"),keyStr.c_str()),result);
	ULONG fileLen=0;
	if(ERROR_SUCCESS!=(result=key.QueryStringValue(NULL,NULL,&fileLen)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Read key=[%s] default value data length failed"),keyStr.c_str()),result);
	std::auto_ptr<TCHAR> file(new TCHAR[fileLen]);
	if(ERROR_SUCCESS!=(result=key.QueryStringValue(NULL,file.get(),&fileLen)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Read key=[%s] default value failed"),keyStr.c_str()),result);
	TCHAR *pFile;
	pFile=(_T('\"')==*file.get())?file.get()+1:file.get();
	if(_T('\"')==file.get()[fileLen-2])
		file.get()[fileLen-2]=_T('\0');
	return pFile;
CATCH_LOG()
	return _T("");
}

tstring GetToolServiceVersion(EBrokerServicesTypes type)
{
TRY_CATCH
	//tstring version;
	HRESULT result;
	tstring keyStr=Format(_T("CLSID\\{%s}\\InprocServer32"),AVAILABLE_SERVICES[type].m_GUID.c_str());
	CRegKey key;
	if(ERROR_SUCCESS!=(result=key.Open(HKEY_CLASSES_ROOT,keyStr.c_str(), KEY_READ)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Open key=[%s] failed"),keyStr.c_str()),result);
	ULONG fileLen=0;
	if(ERROR_SUCCESS!=(result=key.QueryStringValue(NULL,NULL,&fileLen)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Read key=[%s] default value data length failed"),keyStr.c_str()),result);
	std::auto_ptr<TCHAR> file(new TCHAR[fileLen]);
	if(ERROR_SUCCESS!=(result=key.QueryStringValue(NULL,file.get(),&fileLen)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Read key=[%s] default value failed"),keyStr.c_str()),result);
	TCHAR *pFile;
	pFile=(_T('\"')==*file.get())?file.get()+1:file.get();
	if(_T('\"')==file.get()[fileLen-2])
		file.get()[fileLen-2]=_T('\0');
	ULONG versionLen=0;
	if(ERROR_SUCCESS!=(result=MsiGetFileVersion(pFile,NULL,&versionLen,NULL,0)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("file=[%s] version length obtaining failed"),pFile),result);
	versionLen+=1;
	std::auto_ptr<TCHAR> version(new TCHAR[versionLen]);
	if(ERROR_SUCCESS!=(result=MsiGetFileVersion(pFile,version.get(),&versionLen,0,NULL)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("file=[%s] version obtaining failed"),pFile),result);
	Log.Add(_MESSAGE_,_T("GetToolServiceVersion() file name=[%s] version=[%s]"),file.get(),version.get());
	return version.get();
CATCH_LOG()
	return _T("");
}


void CCoBrokerProxy::InstallBroker(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params)
{
TRY_CATCH
	USES_CONVERSION;
	//Log.Add(_MESSAGE_,_T("Available version = %s"),OLE2T(m_brokerParams.m_version));
	//if(GetToolServiceVersion(BST_BROKER)==OLE2T(m_brokerParams.m_version))
	//	return;//installation is not needed

	++m_installerRId;

	ULONG resVal=-1;
	//tstring brokerFeature=_T("Broker");
	tstring brokerFeature=AVAILABLE_SERVICES[BST_BROKER].m_feature;
	tstring svcDisplayNames=AVAILABLE_SERVICES[BST_BROKER].m_displayName;
	//TODO check: can _tstol() take NULL pointer
	long additionType=_tstol(_OLE2T(params));
	if(additionType!=0&&AVAILABLE_SERVICES.find(EBrokerServicesTypes(additionType))!=AVAILABLE_SERVICES.end())
	{
		brokerFeature+=_T(",")+AVAILABLE_SERVICES[EBrokerServicesTypes(additionType)].m_feature;
		svcDisplayNames+=_T(", ")+AVAILABLE_SERVICES[EBrokerServicesTypes(additionType)].m_displayName;
	}
	else
		Log.Add(_MESSAGE_,_T("Service was not found param=0x%x params=%s"),param,_OLE2T(params));

	//Log.Add(_MESSAGE_,_T("CCoBrokerProxy::InstallBroker()"));

	FireRequestSent(srcUserId,BSVCIDPDV_JS,srcUserId,BSVCIDPDV_BROKERPROXY,m_installerRId,BRT_PROGRESS,1/*begin*/,CComBSTR(L"Initializing installation..."));
	//Sleep(100);
	FireRequestSent(dstUserId,dstSvcId,srcUserId,BSVCIDPDV_BROKERPROXY,rId,BRT_INSTALLATION,1/*begin*/,
	            CComBSTR(Format(BRT_INSTALLATION_EXTEXT,svcDisplayNames.c_str()).c_str()));

	// only http use in installation, even if BrokerProxy installed from https
	tstring codebaseurl=GetCodeBaseUrl(GetObjectCLSID());
	tstring::size_type ihttps;
	const TCHAR HTTPS[]=_T("https");
	const TCHAR HTTP[]=_T("http");
	if(tstring::npos!=(ihttps=codebaseurl.find(HTTPS)))
	{
		codebaseurl.replace(ihttps,_countof(HTTPS)-1,HTTP,0,_countof(HTTP));
	}
	
	TCHAR appName[]=_T("msiexec.exe");
	tstring cmdLine=Format(_T("/i %s%s INSTALLUPG=%s REINSTALLMODE=vomus /qn /Liwearucmopv+ \"%s\\InitialInstall.log\""),
	                       codebaseurl.c_str(),
	//                       GetCodeBaseUrl(GetObjectCLSID()).c_str(),
	//                       _T("http://max/brokers/bin"),
	//                       _T("c:\\S2\\bin\\"),
	                       OLE2T(m_brokerParams.m_msiPath),
	                       brokerFeature.c_str(),
	                       LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str());

 Log.Add(_MESSAGE_,_T("CCoBrokerProxy::InstallBroker()cmdLine=%s"),cmdLine.c_str());	//+system version request
	OSVERSIONINFOEX osInf;
	osInf.dwOSVersionInfoSize=sizeof(osInf);
	if(!GetVersionEx((OSVERSIONINFO*)&osInf))
			throw MCException_Win("System version obtainning failed");
	//-system version request
	HANDLE hProcess=NULL;	
	if(IsUserAnAdmin() && osInf.dwMajorVersion < 6/*For vista we need to elevate even for admin*/)
	{
		STARTUPINFO si={sizeof(si)};
		GetStartupInfo(&si);
		PROCESS_INFORMATION pi;
		if(!CreateProcess(NULL,const_cast<LPTSTR>((tstring(appName)+_T(" ")+cmdLine).c_str()),NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,
												NULL,NULL,&si,&pi))
			throw MCException_Win("CreateProcess failed");
		CloseHandle(pi.hThread);
		hProcess=pi.hProcess;
	}
	else
	{
		std::auto_ptr<TCHAR> cmd(new TCHAR[cmdLine.length()+1]);
		_tcscpy_s(cmd.get(),cmdLine.length()+1,cmdLine.c_str());
		SHELLEXECUTEINFO shellInfo={sizeof(shellInfo),SEE_MASK_NOCLOSEPROCESS,0,_T("runas"),appName,cmd.get(),NULL,SW_SHOWNORMAL,0,NULL,NULL,
																0,0,0,0};
		if(!ShellExecuteEx(&shellInfo))
				throw MCException("Process starting failed with elevated privileges");
		hProcess=shellInfo.hProcess;
	}
	const DWORD timeout=900;
	DWORD time=GetTickCount();
	while(WaitForSingleObject(hProcess,timeout)==WAIT_TIMEOUT)
	{
		int percent=100*(1-exp(-(double)(GetTickCount()-time)/20000));
		if(percent>99)
			percent=99;
		FireRequestSent(srcUserId,BSVCIDPDV_JS,srcUserId,BSVCIDPDV_BROKERPROXY,m_installerRId,BRT_PROGRESS,percent,CComBSTR(L"Initializing installation..."));
	}
	GetExitCodeProcess(hProcess,&resVal);
	CloseHandle(hProcess);


	FireRequestSent(srcUserId,BSVCIDPDV_JS,srcUserId,BSVCIDPDV_BROKERPROXY,m_installerRId,BRT_PROGRESS,100/*end*/,CComBSTR(L"Initializing installation..."));
	tstring message;
	if(resVal) //error during installation process
		message=Format(BRT_INSTALLATION_EXERRORTEXT,svcDisplayNames.c_str(),resVal);
	else // no error
		message=Format(BRT_INSTALLATION_EXTEXT,svcDisplayNames.c_str());
	//Sleep(100);
	FireRequestSent(dstUserId,dstSvcId,srcUserId,BSVCIDPDV_BROKERPROXY,rId,BRT_INSTALLATION,100/*%*/,CComBSTR(message.c_str()));

	SetLastError(resVal);
	Log.WinError(_MESSAGE_,_T("Installation completed with code=%d "),resVal);

	Log.ClearList();
	Log.RegisterLog(this);
	Log.RegisterLog(new cFileLog(Format(_T("%s\\%s"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str(),DEF_FILE_LOG_NAME).c_str()));

	//Sleep(100);

	SetLastError(resVal);
	Log.WinError(_MESSAGE_,_T("Installation completed with code=%d "),resVal);


	PostMessage(m_msgCreateBroker);
CATCH_THROW()
}

void CCoBrokerProxy::Execute(void *Params)
{
TRY_CATCH
	CoInitializeEx(NULL,COINIT_MULTITHREADED);
	SRequest req=m_requests.front();
	InstallBroker(CComBSTR(req.m_dstUserId.c_str()),req.m_dstSvcId,CComBSTR(req.m_srcUserId.c_str()),req.m_srcSvcId,req.m_rId,req.m_rType,
	              req.m_param, CComBSTR(req.m_params.c_str()));
	CoUninitialize();
CATCH_LOG()
}


STDMETHODIMP CCoBrokerProxy::Init(BSTR msiPath, BSTR version, BSTR productCode)
{
TRY_CATCH_COM
	m_brokerParams.m_msiPath=msiPath;
	m_brokerParams.m_version=version;
	m_brokerParams.m_productCode=productCode;
	USES_CONVERSION;
	Log.Add(_MESSAGE_,_T("CCoBrokerProxy::Init(%s,%s,%s)"),_OLE2T(msiPath),_OLE2T(version),_OLE2T(productCode));
CATCH_LOG_COM
}

STDMETHODIMP CCoBrokerProxy::InitSession(BSTR relaySrv, BSTR sId, BSTR userId, BSTR passwd, BSTR remoteUserId)
{
TRY_CATCH_COM
	HRESULT result;
	if(m_broker.p)
	{
		CComVariant arg[5];
		arg[4]=relaySrv;
		arg[3]=sId;
		arg[2]=userId;
		arg[1]=passwd;
		arg[0]=remoteUserId;
		m_broker.InvokeN(L"InitSession",arg,_countof(arg));
	}
	else
		m_sessionsParams.push(CSessionParams(relaySrv, sId, userId, passwd, remoteUserId));
	USES_CONVERSION;
	Log.Add(_MESSAGE_,_T("CCoBrokerProxy::InitSession(%s,%s,%s,%s,%s)"),_OLE2T(relaySrv),_OLE2T(sId),_OLE2T(userId),_OLE2T(passwd),_OLE2T(remoteUserId));
CATCH_LOG_COM
}

STDMETHODIMP CCoBrokerProxy::HandleRequest(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params)
{
TRY_CATCH_COM
	HRESULT result;
	USES_CONVERSION;
	if(m_installing)
	{
		m_requests.push(SRequest(_OLE2T(dstUserId),dstSvcId,_OLE2T(srcUserId),srcSvcId,rId,rType,param,_OLE2T(params)));
	}
	else
	{
		if(!m_broker.p)
		{
			Log.Add(_MESSAGE_,_T("Available version = %s"),_OLE2T(m_brokerParams.m_version));
			if(GetToolServiceVersion(BST_BROKER)!=_OLE2T(m_brokerParams.m_version))//installation is needed
			{
				if(IsInstallOff())
					Log.Add(_WARNING_,_T("Broker installation TURN OFF. Remove file install.off"));
				else
				{
					m_requests.push(SRequest(_OLE2T(dstUserId),dstSvcId,_OLE2T(srcUserId),srcSvcId,rId,rType,param,_OLE2T(params)));
					m_installing=true;
					CThread::Start();
					return S_OK;
				}
			}
			//TODO
			m_requests.push(SRequest(_OLE2T(dstUserId),dstSvcId,_OLE2T(srcUserId),srcSvcId,rId,rType,param,_OLE2T(params)));
			CreateBroker();
			return S_OK;
		}
		//Log.Add(_MESSAGE_,_T("CCoBrokerProxy::HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),_OLE2T(dstUserId),dstSvcId,_OLE2T(srcUserId),srcSvcId,rId,rType,param,_OLE2T(params));
		CComVariant arg[8];
		arg[7]=dstUserId;
		arg[6]=dstSvcId;
		arg[5]=srcUserId;
		arg[4]=srcSvcId;
		arg[3]=rId;
		arg[2]=rType;
		arg[1]=param;
		arg[0]=params;
		m_broker.InvokeN(L"HandleRequest",arg,_countof(arg));
	}

	//Log.Add(_MESSAGE_,_T("CCoBrokerProxy::HandleRequest()"));
CATCH_LOG_COM
}

tstring CCoBrokerProxy::GetPageUrl()
{	
	//TODO: Make all errors to have common style. I.e. or can't or failed.
	USES_CONVERSION;
	CComPtr<IOleClientSite> clientSite;
	if(GetClientSite((IOleClientSite **)&clientSite)!=S_OK)
		throw MCException("IOleClientSite obtaining failed");
	CComPtr<IServiceProvider> srvProv;
	if(clientSite->QueryInterface(IID_IServiceProvider,(void**)&srvProv)!=S_OK)
		throw MCException("IServiceProvider obtaining failed");
	CComPtr<IWebBrowser2> webBrowser;
	if(srvProv->QueryService(IID_IWebBrowserApp,IID_IWebBrowser2,(void **)&webBrowser)!=S_OK)
		throw MCException("IWebBrowser2 obtaining failed");
	BSTR bstrURL;
	if(webBrowser->get_LocationURL(&bstrURL)!=S_OK)
		throw MCException("IWebBrowser2 url obtaining failed");
	OLECHAR wzDecodedUrl[INTERNET_MAX_URL_LENGTH];
	DWORD cchDecodedUrl=INTERNET_MAX_URL_LENGTH;
	if(CoInternetParseUrl(bstrURL,PARSE_CANONICALIZE,ICU_DECODE,wzDecodedUrl,cchDecodedUrl,&cchDecodedUrl,0)!=S_OK)
		throw MCException("Url canonicalization failed");
	return W2T(wzDecodedUrl);
}


void CCoBrokerProxy::CreateBroker()
{
TRY_CATCH
	HRESULT result;
	//Log.Add(_MESSAGE_,_T("CCoBrokerProxy::CreateBroker()"));
	CComPtr<IDispatch> broker;
	if(m_broker.p)
		throw MCException("Broker has been created already");

	//if(m_firstInstance.Create(NULL,FALSE,_T("BrokerProxy::m_firstInstance"))&&ERROR_ALREADY_EXISTS!=GetLastError())
	//{

	//	CRegKey reg;
	//	if(ERROR_SUCCESS!=reg.Open(HKEY_LOCAL_MACHINE,_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE"),KEY_READ))
	//		throw MCException_Win("IE path search failed");
	//	TCHAR buf[32768];
	//	DWORD bufLen=_countof(buf);
	//	if(ERROR_SUCCESS!=reg.QueryStringValue(NULL,buf,&bufLen))
	//		throw MCException_Win("IE path reading failed");
	//	
	//	tstring cmdLine=GetPageUrl();
	//	std::auto_ptr<TCHAR> cmd(new TCHAR[cmdLine.length()+1]);
	//	_tcscpy_s(cmd.get(),cmdLine.length()+1,cmdLine.c_str());
	//	SHELLEXECUTEINFO shellInfo={sizeof(shellInfo),SEE_MASK_NOCLOSEPROCESS,0,_T("open"),buf,cmd.get(),NULL,SW_SHOWNORMAL,0,NULL,NULL,
	//															0,0,0,0};
	//	if(!ShellExecuteEx(&shellInfo))
	//			throw MCException("Process starting failed with elevated privileges");

	//	return;
	//}
	//else if(ERROR_ALREADY_EXISTS==GetLastError())
	//{
	//	m_firstInstance.Close();
	//}
	//else
	//{
	//	Log.WinError(_ERROR_,_T("CCoBrokerProxy::CreateBroker() [BrokerProxy::m_firstInstance] mutex creation failed"));
	//}
	//TRY_CATCH
	//	EBrokerProxyStartTypes startParam=BPST_SAME;
	//	
	//	SRequest req=m_requests.front();
	//	tstring url;
	//	{
	//		tstring::size_type iBeginUrl=req.m_params.find(_T(";;"));
	//		if(iBeginUrl!=tstring::npos)
	//		{
	//			tstring::size_type iEndUrl=req.m_params.find(_T(";;"),iBeginUrl+2);
	//			url=req.m_params.substr(iBeginUrl+2,(tstring::npos!=iEndUrl)?iEndUrl-iBeginUrl-2:tstring::npos);
	//		}
	//	}
	//	Log.Add(_MESSAGE_,_T("CREATEBORKER url=[%s]"),url.c_str());

	//	//const TCHAR SESSION_ID_PARAMETER_NAME[]=_T("supportRequestId=");
	//	//tstring::size_type iWebSId=url.find(SESSION_ID_PARAMETER_NAME);
	//	//if(iWebSId!=tstring::npos&&iWebSId+_countof(SESSION_ID_PARAMETER_NAME)<url.size())
	//	//{
	//	//	std::auto_ptr<TCHAR> urlBuf(new TCHAR[url.length()+1]);
	//	//	//url.copy(urlBuf.get(),url.length()+1);
	//	//	_tcscpy_s(urlBuf.get(),url.length()+1,url.c_str());
	//	//	TCHAR *beginWebSId=urlBuf.get()+iWebSId+_countof(SESSION_ID_PARAMETER_NAME)-1;
	//	//	TCHAR *endWebSId=NULL;
	//	//	long webSId=_tcstol(beginWebSId,&endWebSId,10);
	//	//	url.erase(beginWebSId-urlBuf.get(),endWebSId-beginWebSId);
	//	//	SHA1Hash hash_buf;
	//	//	CRYPTO_INSTANCE.MakeHash((char*)&webSId,sizeof(webSId),hash_buf);
	//	//	tstring hashString;
	//	//	for(int i=0;i<sizeof(hash_buf);++i)
	//	//		hashString+=i2tstring((int)(unsigned char)hash_buf[i],16);
	//	//	url.insert(beginWebSId-urlBuf.get(),hashString);
	//	//}
	//	Log.Add(_WARNING_,_T("HASHING TURN OFF"));
	//	Log.Add(_MESSAGE_,_T("CREATEBORKER url=[%s]"),url.c_str());
	//	
	//	CAtlFile cnfg;
	//	if(S_OK==cnfg.Create(Format(_T("%s\\%s"),GetModulePath(GetCurrentModule()).c_str(),_T("BrokerProxy.cfg")).c_str(),GENERIC_READ,0,OPEN_EXISTING))
	//	{
	//		TRY_CATCH
	//			DWORD readLen=0;
	//			readLen=GetFileSize(cnfg.m_h,NULL);
	//			if(INVALID_FILE_SIZE==readLen&&readLen>32768)
	//				throw MCException_Win("BrokerProxy.cfg size obtaining failed or file too large");
	//			std::auto_ptr<char> buf(new char[readLen]);
	//			if(!ReadFile(cnfg.m_h,buf.get(),readLen,&readLen,NULL))
	//				throw MCException_Win("BrokerProxy.cfg file reading failed");
	//			startParam=EBrokerProxyStartTypes(atol(buf.get()));
	//			Log.Add(_MESSAGE_,_T("CREATEBORKER file startParam=[0x%x]"),startParam);
	//		CATCH_LOG()
	//	}
	//	else
	//	{
	//		tstring::size_type iStartParam=req.m_params.find(_T(";;"));
	//		if(tstring::npos!=iStartParam)
	//		{
	//			iStartParam=req.m_params.find(_T(";;"),iStartParam+2);
	//			if(iStartParam!=tstring::npos&&iStartParam+2<req.m_params.size())
	//			{
	//				startParam=EBrokerProxyStartTypes(_tstol(req.m_params.c_str()+iStartParam+2));
	//				Log.Add(_MESSAGE_,_T("CREATEBORKER js startParam=[0x%x]"),startParam);
	//			}
	//		}
	//	}
	//	Log.Add(_MESSAGE_,_T("CREATEBORKER url=[%s] startParam=[0x%x]"),url.c_str(),startParam);

	//	
	//	const TCHAR PARENTPROC_FILEMAPNAME[]=_T("SupportSpaceBrokerProxy0x%08x");
	//	if(BPST_SAME<startParam&&BPST_MAXELEMENT>startParam)
	//	{
	//		CAtlFileMapping<SParentProcParams> parent;
	//		if(S_OK==(result=parent.OpenMapping(Format(PARENTPROC_FILEMAPNAME,GetCurrentProcessId()).c_str(),0,0,FILE_MAP_READ)))
	//		{
	//			// handle leakage for lock parent process information file map object in system while current process exist
	//			HANDLE hParent;
	//			if(!DuplicateHandle(GetCurrentProcess(),parent.GetHandle(),GetCurrentProcess(),&hParent,NULL,FALSE,DUPLICATE_SAME_ACCESS))
	//				throw MCException_Win("DuplicateHandle() of parent process information file mapping failed");
	//			Log.Add(_MESSAGE_,_T("CREATEBORKER parent file mapping openning id=0x%x childId=0x%x wnd=0x%x"),
	//							static_cast<SParentProcParams*>(parent)->m_id,
	//							static_cast<SParentProcParams*>(parent)->m_childId,
	//							static_cast<SParentProcParams*>(parent)->m_wnd);
	//			SendMessage(static_cast<SParentProcParams*>(parent)->m_wnd,m_msgChildBrokerProxyCreated,0xf0f0f0f0,0x10101010);
	//		}
	//		else
	//		{
	//			Log.WinError(_MESSAGE_,_T("CREATEBORKER parent file mapping openning failed 0x%x "),result);
	//			tstring cmdLine;
	//			if(BPST_IE==startParam)
	//			{
	//				CRegKey reg;
	//				if(ERROR_SUCCESS!=(result=reg.Open(HKEY_LOCAL_MACHINE,_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE"),KEY_READ)))
	//					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IE path search failed")),result);
	//				DWORD bufLen=0;
	//				if(ERROR_SUCCESS!=(result=reg.QueryStringValue(NULL,NULL,&bufLen)))
	//					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IE path length obtaining failed")),result);
	//				std::auto_ptr<TCHAR> buf(new TCHAR[bufLen]);
	//				if(ERROR_SUCCESS!=(result=reg.QueryStringValue(NULL,buf.get(),&bufLen)))
	//					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IE path reading failed")),result);
	//				cmdLine=Format(_T("\"%s\" %s"),buf,url.c_str());
	//			}
	//			else if(BPST_NATIVE==startParam)
	//			{
	//				tstring path=GetToolServiceFileName(BST_BROKER);
	//				int lastPos = path.find_last_of(_T('\\'));
	//				if (tstring::npos !=lastPos)
	//					path = path.substr(0,lastPos);
	//				cmdLine=Format(_T("\"%s\\%s\" /workbench %s"),path.c_str(),_T("SupportSpace_tools.exe"),url.c_str());
	//			}
	//			
	//			Log.Add(_MESSAGE_,_T("CREATEBORKER cmdLine=[%s]"),cmdLine.c_str());
	//			STARTUPINFO si={sizeof(si)};
	//			GetStartupInfo(&si);
	//			PROCESS_INFORMATION pi;
	//			size_t cmdLineBufLen=cmdLine.length()+1;
	//			std::auto_ptr<TCHAR> cmdLineBuf(new TCHAR[cmdLineBufLen]);
	//			memcpy_s(cmdLineBuf.get(),cmdLineBufLen,cmdLine.c_str(),cmdLineBufLen);
	//			if(!CreateProcess(NULL,cmdLineBuf.get(),NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS|CREATE_SUSPENDED,NULL,NULL,&si,&pi))
	//				throw MCException_Win("CreateProcess failed");
	//			CHandle newProcMainThread(pi.hThread);
	//			// process remover (porcess create suspended, must be terminated by scope exit abnormal case)
	//			class _CProcessRemover
	//			{
	//				public:
	//					HANDLE m_h;
	//					//CHandle m_handle;
	//					_CProcessRemover(HANDLE h=INVALID_HANDLE_VALUE):m_h(h){}
	//					~_CProcessRemover()
	//					{
	//						if(INVALID_HANDLE_VALUE!=m_h)
	//						{
	//							TerminateProcess(m_h,0);
	//							CloseHandle(m_h);
	//						}
	//					}
	//			} pr(pi.hProcess);
	//			

	//			CAtlFileMapping<SParentProcParams> newProc;
	//			if(S_OK!=(result=newProc.MapSharedMem(sizeof(SParentProcParams),Format(PARENTPROC_FILEMAPNAME,pi.dwProcessId).c_str())))
	//				throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("MapSharedMem() failed")),result);
	//			static_cast<SParentProcParams*>(newProc)->m_id=GetCurrentProcessId();
	//			static_cast<SParentProcParams*>(newProc)->m_childId=pi.dwProcessId;
	//			static_cast<SParentProcParams*>(newProc)->m_wnd=m_wnd;
	//			// handle leakage for lock new process information file map object in system while current process exist
	//			HANDLE hNewProc;
	//			if(!DuplicateHandle(GetCurrentProcess(),newProc.GetHandle(),GetCurrentProcess(),&hNewProc,NULL,FALSE,DUPLICATE_SAME_ACCESS))
	//				throw MCException_Win("DuplicateHandle() of new process information file mapping failed");
	//			if(-1==ResumeThread(newProcMainThread))
	//				throw MCException_Win("Main thread of new process resuming failed");
	//			pr.m_h=INVALID_HANDLE_VALUE;
	//			return;
	//		}
	//	}
	//CATCH_LOG()

	//	
	////TODO 
	//Log.Add(_MESSAGE_,_T("CREATEBORKER BROKER CREATION"));
	////return;

	//+system version request
	OSVERSIONINFOEX osInf;
	osInf.dwOSVersionInfoSize=sizeof(osInf);
	if(!GetVersionEx((OSVERSIONINFO*)&osInf))
		throw MCException_Win("System Version request");
	//-system version request

	//+out of process instance creation
	if(osInf.dwMajorVersion>5)//Vista and higher
	{
		try
		{
			CSrvSTDQueueComm communicator(DEF_WND_CLS_NAME);
			#define MAGIC_BUFFER_SIZE 0xFFFF
			char buf[MAGIC_BUFFER_SIZE];
			SStartBroker msg;
			msg.buf = buf;
			msg.bufSize = MAGIC_BUFFER_SIZE;

			CScopedTracker<HGLOBAL> globalMem;
			globalMem.reset(GlobalAlloc(GMEM_MOVEABLE, msg.bufSize), GlobalFree);
			if (NULL == globalMem)
				throw MCException_Win("Failed to GlobalAlloc ");

			if (FALSE == communicator.SendMsg(SRVCOMM_START_BROKER, reinterpret_cast<char*>(&msg), sizeof(msg)))
				throw MCException("Failed to start broker through service. Service respond FALSE on request");

			CComPtr<IStream> stream;
			HRESULT hr;
			hr = CreateStreamOnHGlobal(globalMem, FALSE, &stream);
			if (S_OK != hr)
				throw MCException_Win(Format(_T("Failed to GlobalAlloc; result = %X"),hr));

			ULARGE_INTEGER size;
			size.QuadPart = msg.bufSize;
			hr = stream->SetSize(size);
			if (S_OK != hr)
				throw MCException_Win(Format(_T("Failed to stream->SetSize; result = %X"),hr));

			ULONG writtenCount;
			hr = stream->Write(msg.buf, msg.bufSize, &writtenCount);
			if (S_OK != hr)
				throw MCException_Win(Format(_T("Failed to stream->Write; result = %X"),hr));

			LARGE_INTEGER li;
			li.QuadPart = 0;
			stream->Seek(li, STREAM_SEEK_SET, NULL);
		
			/// Unmarshaling broker interface
			//const IID IID_IBroker = {0x98CFDD1C,0xF190,0x4225,{0x9C,0xE2,0x5A,0xF4,0xA4,0x9A,0xDB,0xD5}};
			hr = CoUnmarshalInterface(stream, IID_IDispatch, (void**)&broker);
			if (S_OK != hr)
				throw MCException_Win(Format(_T("Failed to CoUnmarshalInterface; result = %X"),hr));

			Log.Add(_MESSAGE_,_T("Broker successfully created through helper application"));
		}
		catch(CExceptionBase& ex)
		{
			MLog_Exception(ex)
			BIND_OPTS2 bo;
			memset(&bo,0,sizeof(bo));
			bo.cbStruct=sizeof(bo);
			//bo.hwnd=NULL;
			bo.dwClassContext=CLSCTX_LOCAL_SERVER;
			if((result=CoGetObject(L"Elevation:Administrator!new:{2FF5923D-5B0C-4EAB-8CF7-7CC79F1A627E}",&bo,IID_IDispatch,
				(void**)&broker))!=S_OK)
				throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Create out-of-process Broker are failed due to COM Elevation Moniker ")),result);
		}
	}
	else 
	{
		if((result=broker.CoCreateInstance(L"Broker.CoBroker",NULL,CLSCTX_LOCAL_SERVER))!=S_OK)
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Broker.CoBroker creation failed")),result);
	}

	if((result=CComObject<C_ICoBrokerEvents>::CreateInstance(&m_brokerEvents))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("C_ICoBrokerEvents object creation failed")),result);
	m_brokerEvents.p->AddRef();//object create with 0 refrences count
	m_brokerEvents->m_owner=this;
	CComPtr<IUnknown> unkn;
	if((result=broker.QueryInterface(&unkn))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IUnknown interface obtaining of broker failed")),result);
	if((result=m_brokerEvents->EventAdvise(unkn))!=S_OK)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("C_ICoBrokerEvents binding (EventAdvise) failed")),result);
	
	m_broker=broker;

	CComVariant initArg[3];
	initArg[2]=m_brokerParams.m_msiPath;
	initArg[1]=m_brokerParams.m_version;
	initArg[0]=m_brokerParams.m_productCode;
	broker.InvokeN(L"Init",initArg,_countof(initArg));

	while(!m_sessionsParams.empty())
	{
		CSessionParams& params=m_sessionsParams.front();
		CComVariant arg[5];
		arg[4]=params.m_relaySrv;
		arg[3]=params.m_sId;
		arg[2]=params.m_userId;
		arg[1]=params.m_passwd;
		arg[0]=params.m_remoteUserId;
		broker.InvokeN(L"InitSession",arg,_countof(arg));
		m_sessionsParams.pop();
	}

	//send parameters of BrokerProxy instance
	CComVariant arg[8];
	arg[7]=CComVariant(_T(""));   //dstUserId;
	arg[6]=CComVariant(0);        //dstSvcId;
	arg[5]=CComVariant(_T(""));   //srcUserId;
	arg[4]=CComVariant(0);        //srcSvcId;
	arg[3]=CComVariant(0);        //rRid
	arg[2]=BRT_BROKERPROXY_INFO;  //rtype
	arg[1]=GetCurrentProcessId(); //param
	arg[0]=CComVariant(Format(_T("%d;;%d"),GetCurrentThreadId(),(m_hWnd)?m_hWnd:m_wnd.m_hWnd).c_str());         //params
	result = broker.InvokeN(L"HandleRequest",arg,_countof(arg));
	if (S_OK != result)
		Log.WinError(_ERROR_,_T("Failed to set parameters of BrokerProxy 0x%x"),result);



	Log.Add(_MESSAGE_,_T("Setting up watchdog timer"));

	//CComVariant arg[8];
	arg[7]=CComVariant(_T(""));		//dstUserId;
	arg[6]=CComVariant(0);			//dstSvcId;
	arg[5]=CComVariant(_T(""));		//srcUserId;
	arg[4]=CComVariant(0);			//srcSvcId;
	arg[3]=CComVariant(0);			//rRid
	arg[2]=BRT_START_WATCHDOG;		//rtype
	arg[1]=GetCurrentProcessId();	//param
	arg[0]=CComVariant();			//params
	result = broker.InvokeN(L"HandleRequest",arg,_countof(arg));
	if (S_OK != result)
		Log.WinError(_ERROR_,_T("Failed to set watchdog timer %X"),result);

	//CCritSection cs(&m_csOfflineMessages);
	while(!m_requests.empty())
	{
		SRequest req=m_requests.front();

		CComVariant arg[8];
		arg[7]=CComBSTR(req.m_dstUserId.c_str());
		arg[6]=req.m_dstSvcId;
		arg[5]=CComBSTR(req.m_srcUserId.c_str());
		arg[4]=req.m_srcSvcId;
		arg[3]=req.m_rId;
		arg[2]=req.m_rType;
		arg[1]=req.m_param;
		arg[0]=CComBSTR(req.m_params.c_str());
		broker.InvokeN(L"HandleRequest",arg,_countof(arg));

		m_requests.pop();
	}
CATCH_THROW()
}

LRESULT CCoBrokerProxy::CreateBroker(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	m_installing=false;
	CreateBroker();
CATCH_LOG()
	return 0;
}

LRESULT CALLBACK CCoBrokerProxy::WndWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
//TRY_CATCH
	CCoBrokerProxy *broker=reinterpret_cast<CCoBrokerProxy*>(CWindow(hWnd).GetWindowLong(GWL_USERDATA));
	LRESULT lResult=0;
	if(broker)
		if(broker->ProcessWindowMessage(hWnd,uMsg,wParam,lParam,lResult,0))
			return 0;
		else
			return CallWindowProc(broker->oldWndWindowProc,hWnd,uMsg,wParam,lParam);
//CATCH_LOG()
	return 0;
}
BOOL CCoBrokerProxy::PostMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	if(m_hWnd)
		return ::PostMessage(m_hWnd,message,wParam,lParam);
	else if(m_wnd.m_hWnd)
		return ::PostMessage(m_wnd,message,wParam,lParam);
	else 
		return false;
}
