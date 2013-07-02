/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCoBroker.cpp
///
///  CCoBroker ActiveX object implementation
///
///  @author Kirill Solovyov @date 11.10.2007
///
////////////////////////////////////////////////////////////////////////

// CCoBroker.cpp : Implementation of CCoBroker

#include "stdafx.h"
#include "CCoBroker.h"
//#include <AidLib/CException/CException.h>
#include <AidLib/Com/ComException.h>
#include <RCEngine/AXstuff/AXstuff.h>
#include <AidLib/Utils/Utils.h>
#include <boost/scoped_array.hpp>
#include <shlobj.h>
#include <RCEngine/sitelock/RCSiteLockImpl.h>
#include <AidLib/Logging/CLogFolder.h>
#include "CBSimplePersistentStorage.h"
#include "CBSimplePersistentStorageReg.h"
#include "CBSimplePersistentStorageFile.h"
#include "CWindowsMinimizer.h"

#include <msi.h>
#pragma comment (lib, "Msi.lib")

#define DEF_FILE_LOG_NAME _T("SupportSpaceTools.log")
#define DEF_REGISTRY_STORAGE _T("Software\SupportSpace\Broker")

tstring GetToolServiceFileName(EBrokerServicesTypes type);
tstring GetToolServiceVersion(EBrokerServicesTypes type);


//---------------------------------------------------------------------------------------------------------------
//// C_ICoBPInstallerEvents
//C_ICoBPInstallerEvents::C_ICoBPInstallerEvents():
//	m_owner(NULL)
//{
//TRY_CATCH
//CATCH_LOG()
//}
//
//C_ICoBPInstallerEvents::~C_ICoBPInstallerEvents()
//{
//TRY_CATCH
//	Log.Add(_MESSAGE_,_T("C_ICoBPInstallerEvents::~C_ICoBPInstallerEvents()"));
//CATCH_LOG()
//}
//
//HRESULT C_ICoBPInstallerEvents::EventAdvise(IUnknown *unkn)
//{
//TRY_CATCH
//	m_unkn.Attach(unkn);
//	return __hook(_ICoBPInstallerEvents,unkn);
//CATCH_THROW()
//}
//
//HRESULT C_ICoBPInstallerEvents::EventUnadvise(void)
//{
//TRY_CATCH
//	CComPtr<IUnknown> unkn;
//	m_unkn.CopyTo(&unkn);
//	m_unkn.Revoke();
//	return __unhook(_ICoBPInstallerEvents,unkn);
//CATCH_THROW()
//}
//
//STDMETHODIMP C_ICoBPInstallerEvents::NotifyFeatureInstalled(LONG result)
//{
//TRY_CATCH
//	if(!m_owner)
//		throw MCException("Owner don't set");
//	//Log.Add(_MESSAGE_,_T("CCoBroker::NotifyFeatureInstalled(0x%x)"),result);
//	m_owner->OnRequest(m_owner->m_params.m_userId,BSVCIDPDV_JS,m_owner->m_params.m_userId,BSVCIDPDV_BROKER,m_owner->m_installerRId,BRT_PROGRESS,100,_T("Installing..."));
//	//m_owner->OnRequest(m_owner->m_installerRequest.m_srcUserId,m_owner->m_installerRequest.m_srcSvcId,m_owner->m_params.m_userId,BSVCIDPDV_BROKER,
//	//                          m_owner->m_installerRequest.m_rId,BRT_INSTALLATION,100,
//	//                          Format(_T("[%s] service is installed"),
//	//                          AVAILABLE_SERVICES[EBrokerServicesTypes(m_owner->m_installerRequest.m_param)].m_feature.c_str()));
//	tstring message;
//	if(result) //error during installation process
//		message=Format(BRT_INSTALLATION_EXERRORTEXT,
//		               AVAILABLE_SERVICES[EBrokerServicesTypes(m_owner->m_installerRequest.m_param)].m_feature.c_str(),result);
//	else // no error
//		message=Format(BRT_INSTALLATION_EXTEXT,
//		               AVAILABLE_SERVICES[EBrokerServicesTypes(m_owner->m_installerRequest.m_param)].m_feature.c_str());
//	m_owner->OnRequest(m_owner->m_installerRequest.m_srcUserId,m_owner->m_installerRequest.m_srcSvcId,m_owner->m_params.m_userId,BSVCIDPDV_BROKER,
//	                   m_owner->m_installerRequest.m_rId,BRT_INSTALLATION,100,message);
//	return S_OK;
//CATCH_LOG()
//	return E_FAIL;
//}
//
//STDMETHODIMP C_ICoBPInstallerEvents::NotifyInstalling(LONG percentCompleted,BSTR status)
//{
//TRY_CATCH
//	if(!m_owner)
//		throw MCException("Owner don't set");
//	USES_CONVERSION;
//	//Log.Add(_MESSAGE_,_T("-CCoBroker::NotifyInstalling(%d,%s)"),percentCompleted,OLE2T(status));
//	m_owner->OnRequest(m_owner->m_params.m_userId,BSVCIDPDV_JS,m_owner->m_params.m_userId,BSVCIDPDV_BROKER,m_owner->m_installerRId,BRT_PROGRESS,percentCompleted,_T("Installing..."));
//	//m_owner->OnRequest(m_owner->m_installerRequest.m_srcUserId,m_owner->m_installerRequest.m_srcSvcId,m_owner->m_params.m_userId,BSVCIDPDV_BROKER,
//	//                           m_owner->m_installerRequest.m_rId,BRT_INSTALLATION,percentCompleted,
//	//                           Format(_T("[%s] service is installing"),
//	//                           AVAILABLE_SERVICES[EBrokerServicesTypes(m_owner->m_installerRequest.m_param)].m_feature.c_str()));
//	m_owner->OnRequest(m_owner->m_installerRequest.m_srcUserId,m_owner->m_installerRequest.m_srcSvcId,m_owner->m_params.m_userId,BSVCIDPDV_BROKER,
//	                   m_owner->m_installerRequest.m_rId,BRT_INSTALLATION,percentCompleted,
//	                   Format(BRT_INSTALLATION_EXTEXT,
//	                          AVAILABLE_SERVICES[EBrokerServicesTypes(m_owner->m_installerRequest.m_param)].m_feature.c_str()));
//
//	return S_OK;
//CATCH_LOG()
//	return E_FAIL;
//
//}
//---------------------------------------------------------------------------------------------------------------
//CBrokerLogReg
CBrokerLogReg::CBrokerLogReg()
{
TRY_CATCH
	//::MessageBox(NULL,_T("CBrokerLogReg"),NULL,0);
	Log.RegisterLog(new cFileLog(Format(_T("%s\\%s"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str(), DEF_FILE_LOG_NAME).c_str()));
	//Log.RegisterLog(new cFileLog(Format(_T("%s\\%s"),GetSpecialFolderPath(CSIDL_PERSONAL).c_str(),DEF_FILE_LOG_NAME).c_str()));
CATCH_THROW()
}
CBrokerLogReg::~CBrokerLogReg()
{
TRY_CATCH
CATCH_LOG()
}
//---------------------------------------------------------------------------------------------------------------
// CCoBroker
CBrokerLogReg CCoBroker::m_logReg;
CCoBroker::CCoBroker():
	m_msgFireNotifyLogMessage(RegisterWindowMessage(_T("CCoBroker::m_msgFireNotifyLogMessage"))),//generate unique message code
	m_msgChildBrokerCreated(RegisterWindowMessage(_T("CCoBroker::m_msgChildBrokerCreated"))) //generate unique message code
{
TRY_CATCH
#ifdef _DEBUG
	//::MessageBox(NULL,_T("CCoBroker::CCoBroker()"),NULL,0);
	Beep(1000,100);
#endif
CATCH_LOG()
}
CCoBroker::~CCoBroker()
{
#ifdef _DEBUG
	//::MessageBox(NULL,_T("CCoBroker::~CCoBroker()"),NULL,0);
	Beep(1000,100);
#endif
}

HRESULT CCoBroker::FinalConstruct()
{
TRY_CATCH
	//::MessageBox(NULL,_T("CCoBroker::FinalConstruct()"),NULL,0);
	Create(HWND_MESSAGE);
	/// Initializing singletons
	CSingleton<CCrtExeptionHook>::instance();
	/// Reporting module initialized (before we've registered this as logger
	REPORT_MODULE("Broker");
	m_ownLiveTime = true; //cLog member
	Log.RegisterLog(this);
	//Log.RegisterLog(new cFileLog(Format(_T("%s\\%s"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str(), DEF_FILE_LOG_NAME).c_str()));
	//Log.RegisterLog(new cFileLog(Format(_T("%s\\%s"),GetSpecialFolderPath(CSIDL_PERSONAL).c_str(),DEF_FILE_LOG_NAME).c_str()));
#ifdef _DEBUG
	Beep(500,100);
#endif
	return S_OK;
CATCH_LOG()
	return E_FAIL;
}

void CCoBroker::FinalRelease()
{
TRY_CATCH
	/// Close firewall
	SAllowConnections msg;
	msg.enabled = false;
	msg.pid = GetCurrentProcessId();
	if (NULL != m_communicator.get())
	{
		m_communicator->ReBind();
		m_communicator->SendMsg(SRVCOMM_ALLOW_CONNECTIONS, reinterpret_cast<char*>(&msg), sizeof(msg));
	}

	{
		CBlockerBlock b(&m_b);
		Log.Add(_MESSAGE_,_T("before m_servicesMgr.reset()"));
		m_servicesMgr.reset();
		Log.Add(_MESSAGE_,_T("after m_servicesMgr.reset()"));
		m_sessionsMgr.reset();
		Log.Add(_MESSAGE_,_T("after m_sessionsMgr.reset()"));
	}
	Log.UnRegisterLog(this);
	DestroyWindow();

#ifdef _DEBUG
	Beep(500,100);
#endif
CATCH_LOG()
}

void CCoBroker::AddList(const cLog::cEventDesc &EventDesc, const TCHAR *Item, ...) throw( )
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

		//HRESULT result;
		//result=NotifyLogMessage(CComBSTR(TimeStr.c_str()),EventDesc.getSeverity());
		::PostMessage(m_hWnd,m_msgFireNotifyLogMessage, reinterpret_cast<WPARAM>(new CComBSTR(TimeStr.c_str())),EventDesc.getSeverity());
	}
	catch(...)
	{
	}
}

LRESULT CCoBroker::FireNotifyLogMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
	// No prologue and epilogue, since it will cause recursive calls
	NotifyLogMessage(static_cast<BSTR>(*reinterpret_cast<CComBSTR*>(wParam)),lParam);
	delete reinterpret_cast<CComBSTR*>(wParam);
	return 0;
}

// multy thread event call
STDMETHODIMP CCoBroker::Advise(IUnknown* pUnkSink,DWORD* pdwCookie)
{
	//::MessageBox(NULL,_T("IConnectionPointImpl<CCoVBroker, &__uuidof(::_ICoVBrokerEvents), CComDynamicUnkArray>::Advise"),NULL,0);
	//T* pT = static_cast<T*>(this);
	CCoBroker* pT = static_cast<CCoBroker*>(this);
	IUnknown* p;
	HRESULT hRes = S_OK;
	if (pdwCookie != NULL)
		*pdwCookie = 0;
	if (pUnkSink == NULL || pdwCookie == NULL)
		return E_POINTER;
	IID iid;
	IConnectionPointImpl<CCoBroker, &__uuidof(::_ICoBrokerEvents), CComDynamicUnkArray>::GetConnectionInterface(&iid);
	hRes = pUnkSink->QueryInterface(iid, (void**)&p);
	if (SUCCEEDED(hRes))
	{
		//pT->Lock();
		//*pdwCookie = m_vec.Add(p);
		pT->Lock();
		*pdwCookie = IConnectionPointImpl<CCoBroker, &__uuidof(::_ICoBrokerEvents), CComDynamicUnkArray>::m_vec.Add(reinterpret_cast<IUnknown*>(CComGITPtr<IUnknown>(p).Detach()));
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

STDMETHODIMP CCoBroker::Unadvise(DWORD dwCookie)
{
	//::MessageBox(NULL,_T("IConnectionPointImpl<CCoVBroker, &__uuidof(::_ICoVBrokerEvents), CComDynamicUnkArray>::Unadvise"),NULL,0);
	//T* pT = static_cast<T*>(this);
	CCoBroker* pT = static_cast<CCoBroker*>(this);
	pT->Lock();
	IUnknown* p = IConnectionPointImpl<CCoBroker, &__uuidof(::_ICoBrokerEvents), CComDynamicUnkArray>::m_vec.GetUnknown(dwCookie);
	CComGITPtr<IUnknown>(reinterpret_cast<DWORD>(p));
	HRESULT hRes = IConnectionPointImpl<CCoBroker, &__uuidof(::_ICoBrokerEvents), CComDynamicUnkArray>::m_vec.Remove(dwCookie) ? S_OK : CONNECT_E_NOCONNECTION;
	pT->Unlock();
	//if (hRes == S_OK && p != NULL)
	//	p->Release();
	return hRes;
}


HRESULT CCoBroker::__ComInvokeEventHandler(IDispatch* pDispatch, DISPID id, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult)
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


STDMETHODIMP CCoBroker::Init(BSTR msiPath, BSTR version, BSTR productCode)
{
TRY_CATCH_COM

	/// Open firewall
	if (NULL == m_communicator.get())
	{
	TRY_CATCH
		m_communicator.reset(new CSrvSTDQueueComm(DEF_WND_CLS_NAME));
		SAllowConnections msg;
		msg.enabled = true;
		msg.pid = GetCurrentProcessId();
		m_communicator->SendMsg(SRVCOMM_ALLOW_CONNECTIONS, reinterpret_cast<char*>(&msg), sizeof(msg));
	CATCH_LOG()
	}

	USES_CONVERSION;
	m_brokerParams.m_msiPath=_OLE2T(msiPath);
	m_brokerParams.m_version=_OLE2T(version);
	m_brokerParams.m_productCode=_OLE2T(productCode);
	Log.Add(_MESSAGE_,_T("CCoBroker::Init(%s,%s,%s)"),_OLE2T(msiPath),_OLE2T(version),_OLE2T(productCode));
CATCH_LOG_COM
}

STDMETHODIMP CCoBroker::InitSession(BSTR relaySrv, BSTR sId, BSTR userId, BSTR passwd, BSTR remoteUserId)
{
TRY_CATCH_COM
	USES_CONVERSION;
	Log.Add(_MESSAGE_,_T("CCoBroker::InitSession(%s,%s,%s,%s,%s)"),_OLE2T(relaySrv),_OLE2T(sId),_OLE2T(userId),_OLE2T(passwd),_OLE2T(remoteUserId));
	CRequestsMgr::Init(_OLE2T(userId),_OLE2T(passwd));
	CRequestsMgr::InitSession(_OLE2T(relaySrv),_OLE2T(sId),_OLE2T(userId),_OLE2T(passwd),_OLE2T(remoteUserId),30000,false /*masterRole*/,true);
CATCH_LOG_COM
}

STDMETHODIMP CCoBroker::HandleRequest(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params)
{
TRY_CATCH_COM
	USES_CONVERSION;
	bool handled=false;
	//::MessageBox(NULL,_T("CCoBroker::HandleRequest()"),NULL,0);
	Log.Add(_MESSAGE_,_T("CCoBroker::HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),_OLE2T(dstUserId),dstSvcId,_OLE2T(srcUserId),srcSvcId,rId,rType,param,_OLE2T(params));
	if(BRT_BROKERPROXY_INFO==rType)
	{
		m_brokerProxyParams.m_pId=param;
		tstring _params=_OLE2T(params);
		m_brokerProxyParams.m_tId=_tstol(_params.c_str());
		tstring::size_type iWnd=_params.find(_T(";;"));
		if(iWnd==tstring::npos||iWnd+2>=_params.size())
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Params value is not recognized; params=[%s]"),_params.c_str());
		m_brokerProxyParams.m_wnd=reinterpret_cast<HWND>(_tstol(_params.c_str()+iWnd+2));
		handled=true;
	}
	else if((BRT_SERVICE|BRT_RESPONSE)==rType&&BSVCIDPDV_JS==srcSvcId)
		handled=HandleRequestService(_OLE2T(dstUserId),dstSvcId,_OLE2T(srcUserId),srcSvcId,rId,rType,param,_OLE2T(params));
	else if(BRT_PROPERTY_LOAD==rType)
	{
		//CBSimplePersistentStorageReg storage(HKEY_CURRENT_USER,DEF_REGISTRY_STORAGE);
		//CBSimplePersistentStorage storage;
		CBSimplePersistentStorageFile storage(LOGS_FOLDER_INSTANCE.GetLogsFolderName()+_T('\\'));
		tstring value=storage.Load(_OLE2T(params));
		OnRequest(_OLE2T(srcUserId),srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,rId,BRT_PROPERTY_LOAD|BRT_RESPONSE,0/*no metter*/,Format(_T("%s;;%s"),_OLE2T(params),value.c_str()));
		handled=true;
	}
	else if(BRT_PROPERTY_SAVE==rType)
	{
		//CBSimplePersistentStorageReg storage(HKEY_CURRENT_USER,DEF_REGISTRY_STORAGE);
		//CBSimplePersistentStorage storage;
		CBSimplePersistentStorageFile storage(LOGS_FOLDER_INSTANCE.GetLogsFolderName()+_T('\\'));
		tstring _params=_OLE2T(params);
		tstring name,value;
		tstring::size_type iValue=_params.find(_T(";;"));
		if(iValue==tstring::npos||iValue+2>=_params.size())
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Params value is not recognized; params=[%s]"),_params.c_str());
		name=_params.substr(0,iValue);
		value=_params.substr(iValue+2);
		storage.Save(name,value);
		handled=true;
	}
	else if(BRT_BROWSER_MINIMIZE==rType)
	{
		CWindowsMinimizer().MinimizeProcessWindows(m_brokerProxyParams.m_pId);
		handled=true;
	}
	if(!handled)
		OnJSRequest(_OLE2T(dstUserId),dstSvcId,_OLE2T(srcUserId),srcSvcId,rId,rType,param,_OLE2T(params));
CATCH_LOG_COM
}

bool CCoBroker::HandleRequestService(const tstring& dstUserId, ULONG dstSvcId, const tstring& srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, const tstring& params)
{
TRY_CATCH
	HRESULT result;
	EBrokerStartTypes startParam=BST_SAME;
	tstring url;
	{
		tstring::size_type iBeginUrl=params.find(_T(";;"));
		if(iBeginUrl!=tstring::npos)
		{
			tstring::size_type iEndUrl=params.find(_T(";;"),iBeginUrl+2);
			url=params.substr(iBeginUrl+2,(tstring::npos!=iEndUrl)?iEndUrl-iBeginUrl-2:tstring::npos);
		}
	}
	Log.Add(_MESSAGE_,_T("***HandleRequestService*** url=[%s]"),url.c_str());

	const TCHAR SESSION_ID_PARAMETER_NAME[]=_T("supportRequestId=");
	tstring::size_type iWebSId=url.find(SESSION_ID_PARAMETER_NAME);
	if(iWebSId!=tstring::npos&&iWebSId+_countof(SESSION_ID_PARAMETER_NAME)<url.size())
	{
		iWebSId+=_countof(SESSION_ID_PARAMETER_NAME)-1;
		tstring::size_type iWebSIdEnd=url.find_first_of(_T("&#"),iWebSId);
		if(tstring::npos==iWebSIdEnd)
			iWebSIdEnd=url.length();
		tstring sId=url.substr(iWebSId,iWebSIdEnd-iWebSId);
		Log.Add(_MESSAGE_,_T("***HandleRequestService*** sId=[%s]"),sId.c_str());
		SHA1Hash hash_buf;
		CRYPTO_INSTANCE.MakeHash((char*)sId.c_str(),sId.length()*sizeof(tstring::value_type),hash_buf);
		tstring hashString;
		for(int i=0;i<sizeof(hash_buf);++i)
			//hashString+=i2tstring((int)(unsigned char)hash_buf[i],16);
			hashString+=Format(_T("%02x"),(int)(unsigned char)hash_buf[i]);
		url.insert(iWebSIdEnd,tstring(_T("&sri="))+hashString);
	}
	//Log.Add(_WARNING_,_T("***HandleRequestService*** HASHING TURN OFF"));
	Log.Add(_MESSAGE_,_T("***HandleRequestService*** url=[%s]"),url.c_str());
	
	CAtlFile cnfg;
	if(S_OK==cnfg.Create(Format(_T("%s\\%s"),GetModulePath(GetCurrentModule()).c_str(),_T("Broker.cfg")).c_str(),GENERIC_READ,0,OPEN_EXISTING))
	{
		TRY_CATCH
			DWORD readLen=0;
			readLen=GetFileSize(cnfg.m_h,NULL);
			if(INVALID_FILE_SIZE==readLen&&readLen>32768)
				throw MCException_Win("Broker.cfg size obtaining failed or file too large");
			std::auto_ptr<char> buf(new char[readLen]);
			if(!ReadFile(cnfg.m_h,buf.get(),readLen,&readLen,NULL))
				throw MCException_Win("Broker.cfg file reading failed");
			startParam=EBrokerStartTypes(atol(buf.get()));
			Log.Add(_MESSAGE_,_T("***HandleRequestService*** file startParam=[0x%x]"),startParam);
		CATCH_LOG()
	}
	else
	{
		tstring::size_type iStartParam=params.find(_T(";;"));
		if(tstring::npos!=iStartParam)
		{
			iStartParam=params.find(_T(";;"),iStartParam+2);
			if(iStartParam!=tstring::npos&&iStartParam+2<params.size())
			{
				startParam=EBrokerStartTypes(_tstol(params.c_str()+iStartParam+2));
				Log.Add(_MESSAGE_,_T("***HandleRequestService*** js startParam=[0x%x]"),startParam);
			}
		}
	}
	Log.Add(_MESSAGE_,_T("***HandleRequestService*** url=[%s] startParam=[0x%x]"),url.c_str(),startParam);

	
	const TCHAR PARENTPROC_FILEMAPNAME[]=_T("SupportSpaceBroker0x%08x");
	if(BST_SAME<startParam&&BST_MAXELEMENT>startParam)
	{
		CAtlFileMapping<SParentProcParams> parent;
		if(S_OK==(result=parent.OpenMapping(Format(PARENTPROC_FILEMAPNAME,m_brokerProxyParams.m_pId).c_str(),0,0,FILE_MAP_READ)))
		{
			// handle leakage for lock parent process information file map object in system while current process exist
			HANDLE hParent;
			if(!DuplicateHandle(GetCurrentProcess(),parent.GetHandle(),GetCurrentProcess(),&hParent,NULL,FALSE,DUPLICATE_SAME_ACCESS))
				throw MCException_Win("DuplicateHandle() of parent process information file mapping failed");
			Log.Add(_MESSAGE_,_T("***HandleRequestService*** parent file mapping [%s] openning id=0x%x childId=0x%x wnd=0x%x"),
							Format(PARENTPROC_FILEMAPNAME,m_brokerProxyParams.m_pId).c_str(),
							static_cast<SParentProcParams*>(parent)->m_id,
							static_cast<SParentProcParams*>(parent)->m_childId,
							static_cast<SParentProcParams*>(parent)->m_wnd);
			::SendMessage(static_cast<SParentProcParams*>(parent)->m_wnd,m_msgChildBrokerCreated,0xf0f0f0f0,0x10101010);
		}
		else
		{
			Log.WinError(_MESSAGE_,_T("***HandleRequestService*** parent file mapping [%s] openning failed 0x%x "),Format(PARENTPROC_FILEMAPNAME,m_brokerProxyParams.m_pId).c_str(),result);
			tstring cmdLine;
			if(BST_IE==startParam)
			{
				CRegKey reg;
				if(ERROR_SUCCESS!=(result=reg.Open(HKEY_LOCAL_MACHINE,_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE"),KEY_READ)))
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IE path search failed")),result);
				DWORD bufLen=0;
				if(ERROR_SUCCESS!=(result=reg.QueryStringValue(NULL,NULL,&bufLen)))
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IE path length obtaining failed")),result);
				std::auto_ptr<TCHAR> buf(new TCHAR[bufLen]);
				if(ERROR_SUCCESS!=(result=reg.QueryStringValue(NULL,buf.get(),&bufLen)))
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IE path reading failed")),result);
				cmdLine=Format(_T("\"%s\" %s"),buf,url.c_str());
			}
			else if(BST_NATIVE==startParam)
			{
				tstring path=GetToolServiceFileName(BST_BROKER);
				int lastPos = path.find_last_of(_T('\\'));
				if (tstring::npos !=lastPos)
					path = path.substr(0,lastPos);
				cmdLine=Format(_T("\"%s\\%s\" /workbench %s"),path.c_str(),_T("SupportSpace_tools.exe"),url.c_str());
			}
			
			Log.Add(_MESSAGE_,_T("***HandleRequestService*** cmdLine=[%s]"),cmdLine.c_str());

			//open process as BrokerProxy process user

			HANDLE newToken, currentToken, hBPProcess;
			if(!(hBPProcess=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,m_brokerProxyParams.m_pId)))
				throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("BrokerProxy process id=0x%x open failed"),m_brokerProxyParams.m_pId),GetLastError());
			CHandle bPProcess(hBPProcess);
			if (0 == OpenProcessToken(bPProcess, TOKEN_ALL_ACCESS, &currentToken))
			throw MCException_Win("Failed to OpenProcessToken");
			
			boost::shared_ptr< boost::remove_pointer<HANDLE>::type > spCurrentToken( currentToken, CloseHandle );

			if (0 == DuplicateTokenEx(spCurrentToken.get(), MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &newToken))
				throw MCException_Win("Failed to Duplicate process token");

			boost::shared_ptr< boost::remove_pointer<HANDLE>::type > spNewToken( newToken, CloseHandle );

			//DWORD tokenSessionId = sessionId;
			//if (0 == SetTokenInformation(spNewToken.get(), TokenSessionId, &tokenSessionId, sizeof(tokenSessionId)))
			//	throw MCException_Win("Failed to SetTokenInformation");

			///// Setting highest integrity level
			//TCHAR integritySid[20] = _T("S-1-16-16384");
			//PSID pIntegritySid = NULL;
			//if (ConvertStringSidToSid(integritySid, &pIntegritySid))
			//{
			//	typedef struct _TOKEN_MANDATORY_LABEL 
			//	{
			//		SID_AND_ATTRIBUTES Label;
			//	} TOKEN_MANDATORY_LABEL,*PTOKEN_MANDATORY_LABEL;
			//	TOKEN_MANDATORY_LABEL TIL = {0};
			//	TIL.Label.Attributes = 0x00000020L /*SE_GROUP_INTEGRITY*/;
			//	TIL.Label.Sid = pIntegritySid;

			//	// Set the process integrity level
			//	if (!SetTokenInformation(	spNewToken.get(), 
			//								(TOKEN_INFORMATION_CLASS)25/*TokenIntegrityLevel*/, 
			//								&TIL,
			//								sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(pIntegritySid)))
			//	{
			//		Log.WinError(_WARNING_,_T("Failed to SetTokenInformation"));
			//	}
			//} else
			//{
			//	Log.WinError(_WARNING_,_T("Failed to ConvertStringSidToSid"));
			//}

			STARTUPINFO si={sizeof(si)};
			GetStartupInfo(&si);
			PROCESS_INFORMATION pi;
			size_t cmdLineBufLen=cmdLine.length()+1;
			std::auto_ptr<TCHAR> cmdLineBuf(new TCHAR[cmdLineBufLen]);
			memcpy_s(cmdLineBuf.get(),cmdLineBufLen,cmdLine.c_str(),cmdLineBufLen);
			if(!CreateProcessAsUser(spNewToken.get(),NULL,cmdLineBuf.get(),NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS|CREATE_SUSPENDED,NULL,NULL,&si,&pi))
			//if(!CreateProcess(NULL,cmdLineBuf.get(),NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS|CREATE_SUSPENDED,NULL,NULL,&si,&pi))
				throw MCException_Win("CreateProcess failed");
			CHandle newProcMainThread(pi.hThread);
			// process remover (porcess create suspended, must be terminated by scope exit abnormal case)
			class _CProcessRemover
			{
				public:
					HANDLE m_h;
					//CHandle m_handle;
					_CProcessRemover(HANDLE h=INVALID_HANDLE_VALUE):m_h(h){}
					~_CProcessRemover()
					{
						if(INVALID_HANDLE_VALUE!=m_h)
						{
							TerminateProcess(m_h,0);
							CloseHandle(m_h);
						}
					}
			} pr(pi.hProcess);
			Log.Add(_MESSAGE_,_T("***HandleRequestService*** CreateProcess() done"),cmdLine.c_str());

			CAtlFileMapping<SParentProcParams> newProc;
			if(S_OK!=(result=newProc.MapSharedMem(sizeof(SParentProcParams),Format(PARENTPROC_FILEMAPNAME,pi.dwProcessId).c_str())))
				throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("MapSharedMem() failed")),result);
			static_cast<SParentProcParams*>(newProc)->m_id=m_brokerProxyParams.m_pId;
			static_cast<SParentProcParams*>(newProc)->m_childId=pi.dwProcessId;
			static_cast<SParentProcParams*>(newProc)->m_wnd=m_hWnd;
			Log.Add(_MESSAGE_,_T("***HandleRequestService*** parent file mapping [%s] openning id=0x%x childId=0x%x wnd=0x%x"),
							Format(PARENTPROC_FILEMAPNAME,pi.dwProcessId).c_str(),
							(static_cast<SParentProcParams*>(newProc))->m_id,
							(static_cast<SParentProcParams*>(newProc))->m_childId,
							(static_cast<SParentProcParams*>(newProc))->m_wnd);
			// handle leakage for lock new process information file map object in system while current process exist
			HANDLE hNewProc;
			if(!DuplicateHandle(GetCurrentProcess(),newProc.GetHandle(),GetCurrentProcess(),&hNewProc,NULL,FALSE,DUPLICATE_SAME_ACCESS))
				throw MCException_Win("DuplicateHandle() of new process information file mapping failed");
			if(-1==ResumeThread(newProcMainThread))
				throw MCException_Win("Main thread of new process resuming failed");
			pr.m_h=INVALID_HANDLE_VALUE;
			Log.Add(_MESSAGE_,_T("***HandleRequestService*** return"));
			return true;
		}
	}
CATCH_LOG()
	return false;
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
	boost::scoped_array<TCHAR> file(new TCHAR[fileLen]);
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
	boost::scoped_array<TCHAR> version(new TCHAR[versionLen]);
	if(ERROR_SUCCESS!=(result=MsiGetFileVersion(pFile,version.get(),&versionLen,0,NULL)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("file=[%s] version obtaining failed"),pFile),result);
	Log.Add(_MESSAGE_,_T("GetToolServiceVersion() file name=[%s] version=[%s]"),file.get(),version.get());
	return version.get();
CATCH_LOG()
	return _T("");
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

void CCoBroker::BRT_SERVICE_Handler(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params)
{
TRY_CATCH
	CCritSection cs(&m_csBRT_SERVICE_Handler);
	CBlockerUse b(&m_b);
	if(_RUNNING==CThread::State)
	{
		m_sessionsMgr->SendRequest(srcUserId,srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,rId,BRT_SERVICE|BRT_RESPONSE,BRR_BUSY,_T("Service installation is in process, try later."));
		return;
	}
	const tstring version(GetToolServiceVersion(EBrokerServicesTypes(param)));
	if(version!=m_brokerParams.m_version)
	{
		if(m_servicesMgr->IsSrvWithTypeExist(EBrokerServicesTypes(param)))
		{
			m_sessionsMgr->SendRequest(srcUserId,srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,rId,BRT_INSTALLATION,100,_T("Service update is needed, but service in use."));
		}
		else
		{
			if(IsInstallOff())
				Log.Add(_WARNING_,_T("Services installation TURN OFF. Remove file install.off"));
			else
			{
				m_installerRequest.m_dstUserId=dstUserId;
				m_installerRequest.m_dstSvcId=dstSvcId;
				m_installerRequest.m_srcUserId=srcUserId;
				m_installerRequest.m_srcSvcId=srcSvcId;
				m_installerRequest.m_rId=rId;
				m_installerRequest.m_rType=rType;
				m_installerRequest.m_param=param;
				m_installerRequest.m_params=params;
				m_installerRId=RId();
				if(_T("")==version)
					m_installerAction=_T("ADDLOCAL");
				else
					m_installerAction=_T("REINSTALL");
				CThread::Start();
				return;
			}
		}
	}
	innerCreateToolService(dstUserId,dstSvcId,srcUserId,srcSvcId,rId,rType,param,params);
CATCH_THROW()
}
//TODO base class
void CCoBroker::innerCreateToolService(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params)
{
TRY_CATCH
	CBlockerUse b(&m_b);
	unsigned long newSvcId=CreateToolService(srcUserId,srcSvcId,m_params.m_userId,BSVCIDPDV_AUTOSET,EBrokerServicesTypes(param),NULL/*COM object*/);
	TRY_CATCH
		OnRequest(dstUserId,newSvcId,m_params.m_userId,BSVCIDPDV_BROKER,RId(),BRT_BROKERPROXY_INFO,m_brokerProxyParams.m_pId,Format(_T("%d;;%d"),m_brokerProxyParams.m_tId,m_brokerProxyParams.m_wnd));//sending BrokerProxy process id for exluding the process in SE child process destroyer (CChildWatcher)
	CATCH_LOG()
	m_sessionsMgr->SendRequest(srcUserId,BSVCIDPDV_BROKER,m_params.m_userId,dstSvcId,rId,rType|BRT_RESPONSE,BRR_APPROVED,i2tstring(srcSvcId)+_T(";;")+i2tstring(newSvcId));//response on service request to broker for remote user id set
	m_sessionsMgr->SendRequest(srcUserId,srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,rId,rType|BRT_RESPONSE,BRR_APPROVED,i2tstring(srcSvcId)+_T(";;")+i2tstring(newSvcId));//response on service request
CATCH_THROW()
}

void CCoBroker::Execute(void *Params)
{
TRY_CATCH
		ULONG oldUILevel;

		SetInternalUI(INSTALLUILEVEL_NONE,&oldUILevel);//set UI level 
		DWORD logFlags=INSTALLLOGMODE_FATALEXIT|INSTALLLOGMODE_ERROR|INSTALLLOGMODE_EXTRADEBUG|INSTALLLOGMODE_WARNING|INSTALLLOGMODE_USER|INSTALLLOGMODE_INFO|INSTALLLOGMODE_RESOLVESOURCE|INSTALLLOGMODE_OUTOFDISKSPACE|INSTALLLOGMODE_ACTIONSTART|INSTALLLOGMODE_ACTIONDATA|INSTALLLOGMODE_COMMONDATA|INSTALLLOGMODE_PROPERTYDUMP|INSTALLLOGMODE_VERBOSE;
		Log.Add(_MESSAGE_,_T("CoBPInstaller.EnableLog(0x%x)"),logFlags);
		EnableLog(/*0x3fff*/ logFlags,_T("Install"),INSTALLLOGATTRIBUTES_APPEND);

	OnRequest(m_installerRequest.m_srcUserId,m_installerRequest.m_srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,
	          m_installerRequest.m_rId,BRT_INSTALLATION,0,
	          Format(BRT_INSTALLATION_EXTEXT,
	                 AVAILABLE_SERVICES[EBrokerServicesTypes(m_installerRequest.m_param)].m_displayName.c_str()));

	DirectConfigureProductEx(Format(_T("%s=%s REINSTALLMODE=vomus"),
	                                         m_installerAction.c_str(),
	                                         AVAILABLE_SERVICES[EBrokerServicesTypes(m_installerRequest.m_param)].m_feature.c_str()));

	/// TODO: NotifyFeatureInstalled is not called on Vista
	//OnRequest(m_params.m_userId,BSVCIDPDV_JS,m_params.m_userId,BSVCIDPDV_BROKER,m_installerRId,BRT_PROGRESS,100,_T("Installing..."));

	innerCreateToolService(m_installerRequest.m_dstUserId,m_installerRequest.m_dstSvcId,m_installerRequest.m_srcUserId,m_installerRequest.m_srcSvcId,
	                  m_installerRequest.m_rId,m_installerRequest.m_rType,m_installerRequest.m_param,m_installerRequest.m_params);
	return;
CATCH_LOG()
	OnRequest(m_installerRequest.m_srcUserId,m_installerRequest.m_srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,m_installerRequest.m_rId,
	          m_installerRequest.m_rType|BRT_RESPONSE,BRR_ERROR,
	          Format(_T("%d;;%s;;%d;;%s"),0/*reserved*/,m_installerRequest.m_dstUserId.c_str(),m_installerRequest.m_dstSvcId,m_installerRequest.m_params.c_str()));

}

void CCoBroker::SendRequestJS(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CCoBroker::SendRequestJS(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),dstUserId.c_str(),dstSvcId,srcUserId.c_str(),srcSvcId,rId,rType,param,params.substr(0, MSG_BUF_SIZE - 100).c_str());
	RequestSent(CComBSTR(dstUserId.c_str()),dstSvcId,CComBSTR(srcUserId.c_str()),srcSvcId,rId,rType,param,CComBSTR(params.c_str()));
CATCH_THROW()
}

void CCoBroker::OnInstalled(LONG result)
{
TRY_CATCH
	OnRequest(m_params.m_userId,BSVCIDPDV_JS,m_params.m_userId,BSVCIDPDV_BROKER,m_installerRId,BRT_PROGRESS,100,_T("Installing..."));
	tstring message;
	if(result) //error during installation process
		message=Format(BRT_INSTALLATION_EXERRORTEXT,
		               AVAILABLE_SERVICES[EBrokerServicesTypes(m_installerRequest.m_param)].m_displayName.c_str(),result);
	else // no error
		message=Format(BRT_INSTALLATION_EXTEXT,
		               AVAILABLE_SERVICES[EBrokerServicesTypes(m_installerRequest.m_param)].m_displayName.c_str());
	OnRequest(m_installerRequest.m_srcUserId,m_installerRequest.m_srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,
	                   m_installerRequest.m_rId,BRT_INSTALLATION,100/*percent*/,message);
CATCH_THROW()
}

void CCoBroker::OnInstalling(LONG percentCompleted, const tstring& status)
{
TRY_CATCH
	OnRequest(m_params.m_userId,BSVCIDPDV_JS,m_params.m_userId,BSVCIDPDV_BROKER,m_installerRId,BRT_PROGRESS,percentCompleted,_T("Installing..."));
	OnRequest(m_installerRequest.m_srcUserId,m_installerRequest.m_srcSvcId,m_params.m_userId,BSVCIDPDV_BROKER,
	                   m_installerRequest.m_rId,BRT_INSTALLATION,percentCompleted,
	                   Format(BRT_INSTALLATION_EXTEXT,
	                   AVAILABLE_SERVICES[EBrokerServicesTypes(m_installerRequest.m_param)].m_displayName.c_str()));
CATCH_THROW()
}


LRESULT CCoBroker::OnChildBrokerCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CCoBroker::OnChildBrokerCreated(0x%x,0x%x,0x%x,0x%x)"),uMsg,wParam,lParam,bHandled);
	OnRequest(m_params.m_userId,BSVCIDPDV_JS,m_params.m_userId,BSVCIDPDV_BROKER,RId(),BRT_BROKER_STARTED,0/*no metter*/,_T(""));
CATCH_LOG()
	return 0;
}

LRESULT CALLBACK CCoBroker::safeWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
TRY_CATCH
	CWindowImplBaseT<CWindow,CControlWinTraits>* pThis = (CWindowImplBaseT<CWindow,CControlWinTraits>*)hWnd;
	CCoBroker* thisBroker = static_cast<CCoBroker*>(pThis);
	CComQIPtr<IUnknown, &IID_IUnknown> lock(static_cast<ICoBroker*>(thisBroker));
	return thisBroker->CWindowImpl<CCoBroker>::GetWindowProc()(hWnd,uMsg,wParam,lParam);
CATCH_LOG()
}