/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCoVBroker.cpp
///
///  CCoVBroker ActiveX object implementation
///
///  @author Kirill Solovyov @date 04.10.2007
///
////////////////////////////////////////////////////////////////////////
// CCoVBroker.cpp : Implementation of CCoVBroker
#include "stdafx.h"
#include "CCoVBroker.h"
#include <RCEngine/AXstuff/AXstuff.h>
#include <AidLib/Strings/tstring.h>
#include <AidLib/Utils/Utils.h>
#include <Shlobj.h>
#include <AidLib/Logging/CLogFolder.h>
#include <AidLib/Com/ComException.h>
#include "misc/CWidgetDragHelper.h"
//#define DEF_FILE_LOG_NAME _T("SupportCenter.log")

struct SSrvPublicParams
{
	TCHAR m_dispayName[20];
	ULONG m_srvType;
};

SSrvPublicParams publicServices[]=
	{
		{_T("Desktop Sharing"),BST_RCVIEWER},
		{_T("Diagnostics"),BST_SECLIENT},
		{_T("File Manager"), BST_FAVIEWER}
	};

#define DEF_FILE_LOG_NAME _T("SupportSpaceTools.log")

CCoVBroker::CCoVBroker():
	m_msgFireNotifyLogMessage(RegisterWindowMessage(_T("CCoVBroker::m_msgFireNotifyLogMessage")))//generate unique message code
{
TRY_CATCH
	m_bWindowOnly=TRUE;
	m_communicator.reset(new CSrvSTDQueueComm(DEF_WND_CLS_NAME));
CATCH_LOG()
}

HRESULT CCoVBroker::FinalConstruct()
{
TRY_CATCH
	/// Initializing singletons
	CSingleton<CCrtExeptionHook>::instance();
	/// Reporting module initialized (before we've registered this as logger
	REPORT_MODULE("VBroker");
	m_ownLiveTime = true; //cLog member
	//Log.RegisterLog(this);
	Log.RegisterLog(new cFileLog(Format(_T("%s\\%s"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str(), DEF_FILE_LOG_NAME).c_str()));
	//Log.RegisterLog(new cFileLog(Format(_T("%s\\%s"),GetSpecialFolderPath(CSIDL_PERSONAL).c_str(),DEF_FILE_LOG_NAME).c_str()));

	/// Open firewall
	SAllowConnections msg;
	msg.enabled = true;
	msg.pid = GetCurrentProcessId();
	if (NULL != m_communicator.get())
		m_communicator->SendMsg(SRVCOMM_ALLOW_CONNECTIONS, reinterpret_cast<char*>(&msg), sizeof(msg));

	return S_OK;
CATCH_LOG()
	return E_FAIL;
}

WNDPROC CCoVBroker::m_parentWndProc = NULL;

LRESULT CALLBACK CCoVBroker::BrowserWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
TRY_CATCH

	static int veryUsefulVariable = CSingleton<CWidgetDragHelper>::instance().Init(hwnd);

	LRESULT retValue = CallWindowProc(m_parentWndProc,hwnd,uMsg,wParam,lParam);

	switch ( uMsg )
	{
		case WM_LBUTTONDOWN:
			CSingleton<CWidgetDragHelper>::instance().OnMouseDown();
			//::SetCapture(hwnd);
			break;
		case WM_LBUTTONUP:
			CSingleton<CWidgetDragHelper>::instance().OnMouseUp();
			//::ReleaseCapture();
			break;
		case WM_MOUSEMOVE:
			CSingleton<CWidgetDragHelper>::instance().OnMouseMove();
			break;
	}
	return retValue;

CATCH_THROW()
}

LRESULT CCoVBroker::OnCreateVBrokerWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	// Subclass a parent window
	if ( HWND hParentWnd = ::GetParent(m_hWnd) )
	{
		m_parentWndProc = (WNDPROC)::SetWindowLongPtr(hParentWnd,GWL_WNDPROC,(LONG_PTR)BrowserWindowProc);

		Log.Add(_MESSAGE_,_T("CCoVBroker::FinalConstruct() Browser window was subclassed"));
	}
	else
		Log.Add(_ERROR_,_T("CCoVBroker::FinalConstruct() Browser window wasn't found"));

	return 0;

CATCH_THROW()
}

LRESULT CCoVBroker::OnDestroyVBrokerWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	// Unsubclass a parent window
	if ( HWND hParentWnd = ::GetParent(m_hWnd) )
	{
		::SetWindowLongPtr(hParentWnd,GWL_WNDPROC,(LONG_PTR)m_parentWndProc);

		Log.Add(_MESSAGE_,_T("CCoVBroker::FinalRelease() Browser window was unsubclassed"));
	}
	else
		Log.Add(_ERROR_,_T("CCoVBroker::FinalRelease() Browser window wasn't found"));

	return 0;

CATCH_THROW()
}

void CCoVBroker::FinalRelease()
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
CATCH_LOG()
TRY_CATCH
	{
		CBlockerBlock b(&m_b);
		m_servicesMgr.reset();
		Log.Add(_MESSAGE_,_T("after m_servicesMgr.reset()"));
		m_sessionsMgr.reset();
		Log.Add(_MESSAGE_,_T("after m_sessionsMgr.reset()"));
	}
	//Log.UnRegisterLog(this);
CATCH_LOG()
}

CCoVBroker::~CCoVBroker()
{
TRY_CATCH

CATCH_LOG()
}

void CCoVBroker::AddList(const cLog::cEventDesc &EventDesc, const TCHAR *Item, ...) throw( )
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

		//NotifyLogMessage(CComBSTR(TimeStr.c_str()),EventDesc.getSeverity());
		::PostMessage(m_hWnd,m_msgFireNotifyLogMessage, reinterpret_cast<WPARAM>(new CComBSTR(TimeStr.c_str())),EventDesc.getSeverity());
	}
	catch(...)
	{
	}
}

LRESULT CCoVBroker::FireNotifyLogMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
	try
	{
		// No prologue and epilogue, since it will cause recursive calls
		CComBSTR* str = reinterpret_cast<CComBSTR*>(wParam);
		if (NULL != static_cast<BSTR>(*str))
			NotifyLogMessage(static_cast<BSTR>(*str),lParam);
		else
			NotifyLogMessage(L"Empty log message",lParam);
		delete str;
	}
	catch(...)
	{
	}
	return 0;
}

// multy thread event call
STDMETHODIMP CCoVBroker::Advise(IUnknown* pUnkSink,DWORD* pdwCookie)
{
	//::MessageBox(NULL,_T("IConnectionPointImpl<CCoVBroker, &__uuidof(::_ICoVBrokerEvents), CComDynamicUnkArray>::Advise"),NULL,0);
	//T* pT = static_cast<T*>(this);
	CCoVBroker* pT = static_cast<CCoVBroker*>(this);
	IUnknown* p;
	HRESULT hRes = S_OK;
	if (pdwCookie != NULL)
		*pdwCookie = 0;
	if (pUnkSink == NULL || pdwCookie == NULL)
		return E_POINTER;
	IID iid;
	IConnectionPointImpl<CCoVBroker, &__uuidof(::_ICoVBrokerEvents), CComDynamicUnkArray>::GetConnectionInterface(&iid);
	hRes = pUnkSink->QueryInterface(iid, (void**)&p);
	if (SUCCEEDED(hRes))
	{
		//pT->Lock();
		//*pdwCookie = m_vec.Add(p);
		pT->Lock();
		*pdwCookie = IConnectionPointImpl<CCoVBroker, &__uuidof(::_ICoVBrokerEvents), CComDynamicUnkArray>::m_vec.Add(reinterpret_cast<IUnknown*>(CComGITPtr<IUnknown>(p).Detach()));
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

STDMETHODIMP CCoVBroker::Unadvise(DWORD dwCookie)
{
	//::MessageBox(NULL,_T("IConnectionPointImpl<CCoVBroker, &__uuidof(::_ICoVBrokerEvents), CComDynamicUnkArray>::Unadvise"),NULL,0);
	//T* pT = static_cast<T*>(this);
	CCoVBroker* pT = static_cast<CCoVBroker*>(this);
	pT->Lock();
	IUnknown* p = IConnectionPointImpl<CCoVBroker, &__uuidof(::_ICoVBrokerEvents), CComDynamicUnkArray>::m_vec.GetUnknown(dwCookie);
	CComGITPtr<IUnknown>(reinterpret_cast<DWORD>(p));
	HRESULT hRes = IConnectionPointImpl<CCoVBroker, &__uuidof(::_ICoVBrokerEvents), CComDynamicUnkArray>::m_vec.Remove(dwCookie) ? S_OK : CONNECT_E_NOCONNECTION;
	pT->Unlock();
	//if (hRes == S_OK && p != NULL)
	//	p->Release();
	return hRes;
}


HRESULT CCoVBroker::__ComInvokeEventHandler(IDispatch* pDispatch, DISPID id, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult)
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

STDMETHODIMP CCoVBroker::StartToolService(BSTR relaySrv, BSTR sId, BSTR userId, BSTR passwd, BSTR remoteUserId, ULONG svcType, IDispatch *host, ULONG* ret_svcId)
{
TRY_CATCH_COM
	USES_CONVERSION;
	Init(_OLE2T(userId),_OLE2T(passwd));
	Log.Add(_MESSAGE_,_T("CCoVBroker::StartToolService(%s,%s,%s,%s,%s,0x%x)"),_OLE2T(relaySrv),_OLE2T(sId),_OLE2T(userId),_OLE2T(passwd),_OLE2T(remoteUserId),svcType);
	InitSession(_OLE2T(relaySrv),_OLE2T(sId),_OLE2T(userId),_OLE2T(passwd),_OLE2T(remoteUserId),30000,true/*masterRole*/);
	if(_countof(publicServices)<=svcType)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Public srvType=0x%x does not exist"),svcType));
	*ret_svcId=CreateToolService(_OLE2T(remoteUserId),BSVCIDPDV_AUTOSET,_OLE2T(userId),BSVCIDPDV_AUTOSET,EBrokerServicesTypes(publicServices[svcType].m_srvType),host);
	if(BST_SECLIENT==publicServices[svcType].m_srvType)
		OnRequest(m_params.m_userId,*ret_svcId,m_params.m_userId,BSVCIDPDV_BROKER,0,BRT_SET_SCRIPTNAME,0/*no matter*/,publicServices[svcType].m_dispayName);
	Log.Add(_MESSAGE_,_T("CCoVBroker::StartToolService()=0x%x"),*ret_svcId);
CATCH_LOG_COM
}

STDMETHODIMP CCoVBroker::StopToolService(ULONG svcId)
{
TRY_CATCH_COM
	DestroyToolService(svcId);
CATCH_LOG_COM
}

STDMETHODIMP CCoVBroker::HandleRequest(BSTR dstUserId, ULONG dstSvcId, BSTR srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, BSTR params)
{
TRY_CATCH_COM
	USES_CONVERSION;
	OnJSRequest(_OLE2T(dstUserId),dstSvcId,_OLE2T(srcUserId),srcSvcId,rId,rType,param,_OLE2T(params));
CATCH_LOG_COM
}

STDMETHODIMP CCoVBroker::GetAvailableServices(SAFEARRAY **params)
{
TRY_CATCH_COM
	if(!params)
		throw MCException("Parameter is invalid");
	CComSafeArray<VARIANT> arg;
	arg.Create(_countof(publicServices));
	for(int i=0;i<_countof(publicServices);++i)
		arg[i]=CComVariant(CComBSTR(publicServices[i].m_dispayName));
	arg.CopyTo(params);
CATCH_LOG_COM
}

void CCoVBroker::SendRequestJS(const tstring& dstUserId, unsigned long dstSvcId, const tstring& srcUserId, unsigned long srcSvcId, unsigned long rId, unsigned long rType, unsigned long param,const tstring& params)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("CCoVBroker::SendRequestJS(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),dstUserId.c_str(),dstSvcId,srcUserId.c_str(),srcSvcId,rId,rType,param,params.c_str());
	RequestSent(CComBSTR(dstUserId.c_str()),dstSvcId,CComBSTR(srcUserId.c_str()),srcSvcId,rId,rType,param,CComBSTR(params.c_str()));
CATCH_THROW()
}
