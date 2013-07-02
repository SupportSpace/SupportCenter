/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CScriptEngineClient.cpp
///
///  Implements CScriptEngineClient class, responsible for ScriptEngineClient ActiveX
///
///  @author Dmitry Netrebenko @date 15.11.2007
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CScriptEngineClient.h"
#include <AidLib/Com/CVariantSerializer.h>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include "SScriptEngineMsg.h"
#include <AidLib/CException/CException.h>
#include <AidLib/Com/ComException.h>
#include "CFileManager.h"
#include <AidLib/Utils/Utils.h>
#include <atlsafe.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/bind.hpp>
#include "CBase64Encoder.h"
#include "CJavaScriptJsonObject.h"
#include <list>
#include <boost/bind.hpp>

CScriptEngineClient::CScriptEngineClient()
	:	CInstanceTracker(_T("CScriptEngineClient"))
	,	m_cleaner()
	,	m_scriptName(_T(""))
	,	m_requestIndex(0)
	,	m_dirs()
	,	m_syncTimeout(DEFAULT_SYNC_TIMEOUT)
	,	m_notificationThread()
	,	m_activityString(_T(""))
	, CRIdGen(100)
{
TRY_CATCH
	if(m_dirs.IsArchiveMode())
	{
		/// Add created temp directory to cleaner
		m_cleaner.AddDirectory(m_dirs.GetScriptDirectory());	
		/// Set password callback for decompression
		m_decompressor.SetPasswdCallback(boost::bind(&CScriptEngineClient::OnUnZipPasswd, this, _1, _2));
	}
	m_copiedVariables[CComBSTR(_T("g_app_server"))] = CComVariant();
	m_copiedVariables[CComBSTR(_T("gBaseUrl"))] = CComVariant();
	m_eventThreadsManager.Start();
CATCH_THROW()
}

CScriptEngineClient::~CScriptEngineClient()
{
TRY_CATCH
	m_brokerEvents.Revoke();
CATCH_LOG()
}

STDMETHODIMP CScriptEngineClient::Init(IUnknown *events)
{
TRY_CATCH_COM
	Log.Add(_MESSAGE_,_T("CScriptEngineClient::Ini(0x%08x) m_dwRef=0x%x"),events,m_dwRef);
	// Attaching to broker events (which is actually not events, but broker pointer, having the same sence)

	HRESULT result;
	CComPtr<IUnknown> eventsUnkn(events);
	CComPtr<_IBrokerClientEvents> brokerEvents;
	if(S_OK != (result=eventsUnkn.QueryInterface(&brokerEvents)))
	{
		SetLastError(result);
		throw MCException_Win(_T("QueryInterface(_IBrokerClientEvents) failed "));
	}
	
	if(S_OK!=(result=m_brokerEvents.Attach(brokerEvents)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Broker object GIT registeration failed")),result);


CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::HandleRequest(BSTR dstUserId,ULONG dstSvcId,BSTR srcUserId,ULONG srcSvcId,ULONG rId,ULONG rType,ULONG param,BSTR params)
{
TRY_CATCH_COM

	USES_CONVERSION;
	switch(rType)
	{
		case BRT_PING:
			{
				Log.Add(_MESSAGE_,_T("CScriptEngineClient::HandleRequest(BRT_PING)"));
				if(!m_brokerEvents.m_dwCookie)
					throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
				HRESULT result;
				CComPtr<_IBrokerClientEvents> brokerEvents;
				if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
				if(!brokerEvents.p)
					throw MCException("_IBrokerClientEvents has not marshaled");
				brokerEvents->RequestSent(srcUserId,srcSvcId,srcUserId,dstSvcId,rId,rType|BRT_RESPONSE,param,params);//response on ping
			}
			break;
		case BRT_PING|BRT_RESPONSE:
			{
				Log.Add(_MESSAGE_,_T("CScriptEngineClient::HandleRequest(BRT_PING|BRT_RESPONSE)"));
			}
			break;
		case BRT_SERVICE|BRT_RESPONSE:
			{
				if(BRR_BPFAILED==param)
				{
					/// OnHostInited
					m_eventThreadsManager.AddEvent(
						boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
						seceHostInited,
						0, 
						false, 
						Format(BRT_SERVICE_BPFAILED,OLE2T(params)));
				}
				else if(BRR_ERROR==param)
				{
					const tstring status=Format(_T("%s. Request handling failed"),BRT_SERVICE_DECLINED);
					/// Show progress: Customer declined
					m_eventThreadsManager.AddEvent(
						boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
						seceProgress,
						0, 
						false, 
						status);
					/// OnHostInited
					m_eventThreadsManager.AddEvent(
						boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
						seceHostInited,
						0, 
						false, 
						status);
				}
				else if(BRR_DECLINED==param)
				{
					/// OnHostInited
					m_eventThreadsManager.AddEvent(
						boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
						seceHostInited,
						0, 
						false, 
						BRT_SERVICE_DECLINED);
				}
				else if(BRR_APPROVED==param)
				{
					/// Show progress: Customer approved
					m_eventThreadsManager.AddEvent(
						boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
						seceProgress,
						0, 
						true, 
						BRT_SERVICE_APPROVED);
					if(!m_brokerEvents.m_dwCookie)
						throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
					HRESULT result;
					CComPtr<_IBrokerClientEvents> brokerEvents;
					if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
						throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
					if(!brokerEvents.p)
						throw MCException("_IBrokerClientEvents has not marshaled");

					if((result=brokerEvents->GetSubStream(CComBSTR(BUSERIDPDV_AUTOSET), BSVCIDPDV_AUTOSET, SESSID_SE, SESSP_SERVICE))!=S_OK)
						MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("Service substream obtaining failed")),result))
				}
				else if(BRR_BUSY==param)
				{
					/// Handle it like customer decline
					m_eventThreadsManager.AddEvent(
						boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
						seceHostInited,/*seceProgress,*/
						0, 
						false, 
						BRT_SERVICE_BUSY);
				}
				else
				{
					/// Show progress: SCriptEngine response is unknown (code=0x%x) %s"),param,OLE2T(params)));
					m_eventThreadsManager.AddEvent(
						boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
						seceProgress,
						0, 
						false, 
						Format(_T("ScriptEngine response is unknown (code=0x%x) %s"), param, OLE2T(params)));
					//TODO /// OnHostInited ????

					MLog_Exception(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Response type is unknown. HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),OLE2T(dstUserId),dstSvcId,OLE2T(srcUserId),srcSvcId,rId,rType,param,OLE2T(params)));
				}
			}
			break;
		case BRT_SET_SCRIPTNAME:
			Log.Add(_MESSAGE_,_T("CScriptEngineClient::::HandleRequest: BRT_SET_SCRIPTNAME %s received"),OLE2T(params));
			{
				TRY_CATCH
					StartScript(params);
				CATCH_LOG()
			}
			break;
		case BRT_INSTALLATION:
			{
				/// Show progress: installation
				m_eventThreadsManager.AddEvent(
					boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
					seceProgress,
					0, 
					true, 
					(param)?Format(BRT_INSTALLATION_EXFORMAT,OLE2T(params),param):_OLE2T(params));
			}
			break;
		case BRT_SRV_STATE_CHANGED:
			{
				if(ESM_SERVICE_STOPPED == param)
				{
					m_eventThreadsManager.AddEvent(
						boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
						seceUnexpectedError,
						SE_ERROR_STOP_SERVICE, 
						false, 
						Format(_T("Customer has stopped %s service."), m_scriptName.c_str()));
				}
			}
			break;
		default:
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Request type is unknown. HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),OLE2T(dstUserId),dstSvcId,OLE2T(srcUserId),srcSvcId,rId,rType,param,OLE2T(params));
	}
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::SetSubStream(BSTR dstUserId, ULONG dstSvcId, ULONG streamId, ULONG pointer_shared_ptr_stream)
{
TRY_CATCH_COM
	Log.Add(_MESSAGE_,_T("CScriptEngineClient::SetSubStream() pointer_shared_ptr_stream=0x%08x"),pointer_shared_ptr_stream);

	USES_CONVERSION;
	switch(streamId)
	{
		case SESSID_SE:
			{
				Log.Add(_MESSAGE_,_T("CScriptEngineClient::SetSubStream(SESSID_SE)"));
				boost::shared_ptr<CAbstractStream>* pstream = new boost::shared_ptr<CAbstractStream>(*reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream));
				m_stream = *reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream);
				InitReceiver(m_stream);
				m_eventThreadsManager.AddEvent(
					boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
					seceHostInited,
					0, 
					true, 
					_T(""));
				m_notificationThread.NotifyServiceStart();
				break;
			}
		default:
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Sub stream id is unknown. SetSbuStream(%s,0x%x,0x%x,0x%08x)"),OLE2T(dstUserId),dstSvcId,streamId,pointer_shared_ptr_stream);
	}
CATCH_LOG_COM
}

void CScriptEngineClient::StartScript(BSTR scriptName)
{
TRY_CATCH

	USES_CONVERSION;
	{
		CCritSection section(&m_envSection);
		m_execEnvironments.clear();
	}

	if(!m_scriptName.empty())
		throw MCException(_T("Script already started."));

	m_scriptName = tstring(OLE2T(scriptName));
	if(m_scriptName.empty())
		throw MCException(_T("Script name is empty."));

	/// Start notification thread
	m_notificationThread.Init(m_scriptName,m_brokerEvents);
	m_notificationThread.Start();

	m_dirs.SetScriptName(m_scriptName);

	if(m_dirs.IsArchiveMode())
	{
		tstring archName = m_dirs.GetSourceDirectory() + m_scriptName + tstring(SCRIPT_FILE_EXT);
		tstring scriptDir = m_dirs.GetScriptDirectory();
		if(FALSE == PathFileExists(archName.c_str()))
			throw MCException(Format(_T("Archive file '%s' with scripts does not exist."), archName.c_str())); 
		if(!m_decompressor.UnZipDirectory(T2A(const_cast<TCHAR*>(scriptDir.c_str())), T2A(const_cast<TCHAR*>(archName.c_str()))))
			throw MCException(_T("Cannot extract script files."));
	}

	/// Check up directories
	tstring localDir = m_dirs.GetLocalDirectory();
	tstring remoteDir = m_dirs.GetRemoteDirectory();
	if(FALSE == PathFileExists(localDir.c_str()))
		throw MCException(Format(_T("Directory '%s' with local scripts does not exist."), localDir.c_str())); 
	if(FALSE == PathFileExists(remoteDir.c_str()))
		throw MCException(Format(_T("Directory '%s' with remote scripts does not exist."), remoteDir.c_str())); 

	HRESULT result = S_OK;

	/// Get IOleClientSite interfaces
	CComPtr<IOleClientSite> clientSite;
	result = GetClientSite((IOleClientSite**)&clientSite);
	if((S_OK != result) || !clientSite)
	{
		SetLastError(result);
		throw MCException_Win(_T("IOleClientSite obtaining failed"));
	}
	/// Get IServiceProvider interface
	CComPtr<IServiceProvider> srvProvider;
	result = clientSite->QueryInterface(IID_IServiceProvider, (void**)&srvProvider);
	if((S_OK != result) || !srvProvider)
	{
		SetLastError(result);
		throw MCException_Win(_T("IServiceProvider obtaining failed"));
	}
	/// Get IWebBrowser2 interface
	CComPtr<IWebBrowser2> webBrowser;
	result = srvProvider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void **)&webBrowser);
	if((S_OK != result) || !webBrowser)
	{
		SetLastError(result);
		throw MCException_Win(_T("IWebBrowser2 obtaining failed"));
	}
	/// Attach to IWebBrowser events
	result = DispEventAdvise(webBrowser);
	if(S_OK != result)
	{
		SetLastError(result);
		throw MCException_Win(_T("Can not advise to WebBrowser events."));
	}
	/// Get IDispatch of current document
	CComPtr<IDispatch> dispDocument;
	result = webBrowser->get_Document(&dispDocument);
	if((S_OK != result) || !dispDocument)
	{
		SetLastError(result);
		throw MCException_Win(_T("Document IDispatch obtaining failed"));
	}
	/// Get IHTMLDocument interface
	CComPtr<IHTMLDocument> document;
	result = dispDocument->QueryInterface(IID_IHTMLDocument, (void**)&document);
	if((S_OK != result) || !document)
	{
		SetLastError(result);
		throw MCException_Win(_T("IHTMLDocument obtaining failed"));
	}
	/// Get IHTMLDocument2 interface
	CComPtr<IHTMLDocument2> document2;
	result = dispDocument->QueryInterface(IID_IHTMLDocument2, (void**)&document2);
	if((S_OK != result) || !document2)
	{
		SetLastError(result);
		throw MCException_Win(_T("IHTMLDocument2 obtaining failed"));
	}
	/// Get IHTMLDocument3 interface
	CComPtr<IHTMLDocument3> document3;
	result = dispDocument->QueryInterface(IID_IHTMLDocument3, (void**)&document3);
	if((S_OK != result) || !document3)
	{
		SetLastError(result);
		throw MCException_Win(_T("IHTMLDocument3 obtaining failed"));
	}
	/// Search for IDispatch of self <object> element
	CComPtr<IHTMLElement> elem;
	CComPtr<IDispatch> dispElem;
	/// Obtain "all" collection
	CComPtr<IHTMLElementCollection> allElems;
	result = document2->get_all(&allElems);
	if((S_OK != result) || !allElems)
	{
		SetLastError(result);
		throw MCException_Win(_T("IHTMLDocument2->get_all failed"));
	}
	/// Obtain IDispatch for "object" elements collection
	CComPtr<IDispatch> objElems;
	CComVariant tag(CComBSTR(L"object"));
	result = allElems->tags(tag, &objElems);
	if((S_OK != result) || !objElems)
	{
		SetLastError(result);
		throw MCException_Win(_T("IHTMLElementCollection->tags failed"));
	}
	/// Obtain "object" elements collection
	CComPtr<IHTMLElementCollection> objColl;
	result = objElems->QueryInterface(IID_IHTMLElementCollection, (void**)&objColl);
	if((S_OK != result) || !objColl)
	{
		SetLastError(result);
		throw MCException_Win(_T("IHTMLElementCollection obtaining failed"));
	}
	/// Get length of collection
	long count = 0;
	result = objColl->get_length(&count);
	if(S_OK != result)
	{
		SetLastError(result);
		throw MCException_Win(_T("IHTMLElementCollection->get_length failed"));
	}
	for(long i = 0; i < count; ++i)
	{
		CComPtr<IDispatch> dispObj;
		CComVariant index(i);
		/// Get IDispatch for element
		result = objColl->item(index, index, &dispObj);
		if((S_OK != result) || !dispObj)
		{
			SetLastError(result);
			throw MCException_Win(_T("IHTMLElementCollection->item failed"));
		}
		/// Get IHTMLObjectElement for element
		CComPtr<IHTMLObjectElement> object;
		result = dispObj->QueryInterface(IID_IHTMLObjectElement, (void**)&object);
		if((S_OK != result) || !object)
		{
			SetLastError(result);
			throw MCException_Win(_T("IHTMLObjectElement obtaining failed"));
		}
		/// Get object
		CComPtr<IDispatch> obj;
		result = object->get_object(&obj);
		if((S_OK != result) || !obj)
		{
			SetLastError(result);
			throw MCException_Win(_T("IHTMLObjectElement->get_object failed"));
		}
		if(obj == this)
		{
			/// Self <object> element is found
			result = dispObj->QueryInterface(IID_IHTMLElement, (void**)&elem);
			if(S_OK != result)
			{
				SetLastError(result);
				throw MCException_Win(_T("IHTMLElement obtaining failed"));
			}
			dispElem = dispObj;
			break;
		}
	}

	if(!elem || !dispElem)
		throw MCException(_T("Host element is not found."));

	/// Save IDispatch of self <object> element
	m_objectDisp.SetInterface(dispElem);

	/// Prepare ifare id
	CComBSTR id(L"");
	result = document3->get_uniqueID(&id);
	if(S_OK != result)
	{
		SetLastError(result);
		throw MCException_Win(_T("IHTMLDocument3->get_uniqueID failed"));
	}

	/// Get interface of global script module
	CComPtr<IDispatch> script;
	result = document->get_Script(&script);
	if((S_OK != result) || !script)
	{
		SetLastError(result);
		throw MCException_Win(_T("IHTMLDocument->get_Script failed"));
	}
	/// Get values of variables
	for(std::map<CComBSTR,CComVariant>::iterator index = m_copiedVariables.begin();
		index != m_copiedVariables.end();
		++index)
	{
		CComBSTR varName = index->first;
		tstring name = OLE2T(varName);
		if(m_script.IdentificatorExists(script, varName))
		{
			CComVariant value;
			m_script.GetProperty(script, varName, &value);
			CComVariant strValue;
			if(S_OK != VariantChangeType(&strValue, &value, 0, VT_BSTR))
			{
				Log.Add(_WARNING_, _T("Cannot convert the value of variable '%s' to string."), name.c_str());
				strValue = _T("");
			}
			index->second = strValue;
		}
		else
		{
			Log.Add(_WARNING_, _T("Variable '%s' not found in host window."), name.c_str());
			index->second = _T("");
		}
	}
	tstring defPage(DEFAULT_HTML_PAGE);
	tstring params(_T(""));
	for(std::map<CComBSTR,CComVariant>::iterator index = m_copiedVariables.begin();
		index != m_copiedVariables.end();
		++index)
	{
		if(index != m_copiedVariables.begin())
			params += _T("&");
		params += OLE2T(index->first);
		params += _T("=");
		CComVariant var = index->second;
		params += OLE2T(var.bstrVal);
	}

	if(params != _T(""))
		defPage += _T("?") + params;

	/// Prepare ifare html
	tstring idStr = OLE2T(id);
	tstring frameStr = Format(FRAME_TAG, idStr.c_str(), localDir.c_str(), defPage.c_str());
	CComBSTR frameCode(frameStr.c_str());
	/// Hiding ScriptEngine ActiveX
	CComPtr<IHTMLStyle> elementStyle;
	result = elem->get_style(&elementStyle);
	if(S_OK == result)
	{
		elementStyle->put_width(CComVariant(CComBSTR(L"0%")));
		elementStyle->put_height(CComVariant(CComBSTR(L"0%")));
	}
	else
	{
		SetLastError(result);
		Log.WinError(_ERROR_,_T("Failed to get element style. ScriptEngine ActiveX will be visible"));
	}
	/// Inject frame
	result = elem->insertAdjacentHTML(L"afterEnd",frameCode);
	if(S_OK != result)
	{
		SetLastError(result);
		throw MCException_Win(_T("Frame injection failed"));
	}
	m_frameURL = localDir + DEFAULT_HTML_PAGE;
CATCH_LOG()
}

STDMETHODIMP CScriptEngineClient::InitHost(void)
{
TRY_CATCH_COM
	InitHostInternal(true);
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::HostInited(LONG* inited)
{
TRY_CATCH_COM
	*inited = (NULL != m_stream.get());
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::Deploy(void)
{
TRY_CATCH_COM
	DeployInternal(true);
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::InvokeFile(BSTR language, BSTR fileName, BSTR procedureName, VARIANT param1, VARIANT param2, BSTR completionName)
{
TRY_CATCH_COM
	/// Add new execution environment
	SPExecEnvironment env = CreateEnvironment(
		CComBSTR(language),
		CComBSTR(fileName),
		CComBSTR(_T("")),
		CComBSTR(procedureName),
		CComBSTR(completionName),
		CComVariant(param1),
		CComVariant(param2));
	InvokeFileInternal(true, env);
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::InvokeCode(BSTR language, BSTR code, BSTR procedureName, VARIANT param1, VARIANT param2, BSTR completionName)
{
TRY_CATCH_COM
	/// Add new execution environment
	SPExecEnvironment env = CreateEnvironment(
		CComBSTR(language),
		CComBSTR(_T("")),
		CComBSTR(code),
		CComBSTR(procedureName),
		CComBSTR(completionName),
		CComVariant(param1),
		CComVariant(param2));
	InvokeCodeInternal(true, env);
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::GetRemoteFiles(VARIANT files)
{
TRY_CATCH_COM
	GetRemoteFilesInternal(true, files);
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::SetTimeOut(LONG timeOut)
{
TRY_CATCH_COM
	m_syncTimeout = timeOut;
	if(!m_stream.get())
		return S_OK;
	/// Create variant with timeout value
	CVariantSerializer::SPVariant var(new CComVariantEx);
	var->vt = VT_I4;
	var->lVal = timeOut;
	/// Serialize parameters
	unsigned int size = 0;
	CVariantSerializer::SPCharBuf varBuf = CVariantSerializer::VariantToBuf(var, &size);
	boost::scoped_array<char> buf(new char[size + SCRIPTENGINE_MSG_HEAD_SIZE]);
	SScriptEngineMsg* msg = reinterpret_cast<SScriptEngineMsg*>(buf.get());
	msg->m_size = size + SCRIPTENGINE_MSG_HEAD_SIZE;
	msg->m_type = semtTimeout;
	msg->m_id = GetRequestIndex();
	memcpy(msg->m_data, varBuf.get(), size);
	/// Send request
	CheckStream();
	m_stream->Send(buf.get(), msg->m_size);
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::WriteLogError(BSTR errorMsg)
{
TRY_CATCH_COM
	USES_CONVERSION;
	tstring text = OLE2T(errorMsg);
	Log.Add(_ERROR_, text.c_str());
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::WriteLogWarning(BSTR warnMsg)
{
TRY_CATCH_COM
	USES_CONVERSION;
	tstring text = OLE2T(warnMsg);
	Log.Add(_WARNING_, text.c_str());
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::WriteLogMessage(BSTR msgText)
{
TRY_CATCH_COM
	USES_CONVERSION;
	tstring text = OLE2T(msgText);
	Log.Add(_MESSAGE_, text.c_str());
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::InitHostSync(void)
{
TRY_CATCH_COM
	WaitForResponse(InitHostInternal(false));
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::DeploySync(void)
{
TRY_CATCH_COM
	WaitForResponse(DeployInternal(false));
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::InvokeFileSync(BSTR language, BSTR fileName, BSTR procedureName, VARIANT param1, VARIANT param2, VARIANT* result)
{
TRY_CATCH_COM
	/// Add new execution environment
	SPExecEnvironment env = CreateEnvironment(
		CComBSTR(language),
		CComBSTR(fileName),
		CComBSTR(_T("")),
		CComBSTR(procedureName),
		CComBSTR(_T("")),
		CComVariant(param1),
		CComVariant(param2));
	InvokeFileInternal(false, env);
	WaitForResponse(env->m_id);
	PrepareExecutionResult(env->m_id, result);
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::InvokeCodeSync(BSTR language, BSTR code, BSTR procedureName, VARIANT param1, VARIANT param2, VARIANT* result)
{
TRY_CATCH_COM
	/// Add new execution environment
	SPExecEnvironment env = CreateEnvironment(
		CComBSTR(language),
		CComBSTR(_T("")),
		CComBSTR(code),
		CComBSTR(procedureName),
		CComBSTR(_T("")),
		CComVariant(param1),
		CComVariant(param2));
	InvokeFileInternal(false, env);
	WaitForResponse(env->m_id);
	PrepareExecutionResult(env->m_id, result);
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::GetRemoteFilesSync(VARIANT files)
{
TRY_CATCH_COM
	WaitForResponse(GetRemoteFilesInternal(false, files));
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::NotifyActivity(BSTR activity)
{
TRY_CATCH_COM
	if(NULL == activity)
		m_activityString = _T("");
	else
	{
		USES_CONVERSION;
		m_activityString = OLE2T(activity);
	}
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineClient::BringOnTop()
{
TRY_CATCH_COM

	HRESULT result;
	if(!m_brokerEvents.m_dwCookie)
		throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
	CComPtr<_IBrokerClientEvents> brokerEvents;
	if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
	if(!brokerEvents.p)
		throw MCException("_IBrokerClientEvents has not marshaled");

	brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,RId(),BRT_SELECT_WIDGET,0,CComBSTR(_T("empty")));

CATCH_LOG_COM
}

unsigned int CScriptEngineClient::InitHostInternal(bool async)
{
TRY_CATCH
	/// Create request
	SPScriptEngineRequest request = CreateRequest(0, async);
	try
	{
		if(!m_brokerEvents.m_dwCookie)
			throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
		HRESULT result;
		CComPtr<_IBrokerClientEvents> brokerEvents;
		if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
		if(!brokerEvents.p)
			throw MCException("_IBrokerClientEvents has not marshaled");
		//tstring& reqParams=Format(_T("[Caption text];;[Message text];;[approve button text];;[decline button text];;%d"),param);
		//tstring& reqParams=Format(_T("Execute script request;;Expert requested script execution;;alow;;deny;;%d"),0/*Permission?*/);
		//m_brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_BROKER,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0,BRT_SERVICE,BST_SEHOST,CComBSTR(reqParams.c_str()));
		tstring& reqParams=Format(BRT_SERVICE_SEFORMAT,m_scriptName.c_str(),BRT_SERVICE_SETEXT,BRT_SERVICE_APPROVE,BRT_SERVICE_DECLINE);
		HRESULT res = brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_BROKER,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,RId(),BRT_SERVICE,BST_SEHOST,CComBSTR(reqParams.c_str()));
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Sending request to init host failed"));
		}
	}
	catch(CExceptionBase& ex)
	{
		TRY_CATCH
			RaiseCustomEvent(seceHostInited, 0, false, ex.m_strWhat);
		CATCH_LOG()
	}
	catch(...)
	{
		TRY_CATCH
			RaiseCustomEvent(seceHostInited, 0, false, _T("Unknown host initialization error."));
		CATCH_LOG()
	}
	return 0;
CATCH_THROW()
}

unsigned int CScriptEngineClient::DeployInternal(bool async)
{
TRY_CATCH
	m_notificationThread.NotifyActivity(GetActivity(seatDeploy));
	/// Create request
	SPScriptEngineRequest request = CreateRequest(GetRequestIndex(), async);
	try
	{
		std::list< std::pair<tstring, tstring> > scripts;
		tstring m_remoteDir = m_dirs.GetRemoteDirectory();
		tstring mask = m_remoteDir + tstring(SCRIPT_FILES_MASK);
		WIN32_FIND_DATA findData;
		HANDLE search = FindFirstFile(mask.c_str(), &findData);
		if(INVALID_HANDLE_VALUE == search)
			throw MCException_Win(_T("Find first script file failed"));
		boost::shared_ptr< boost::remove_pointer<HANDLE>::type > searchHandle(search, FindClose);
		if(0 == (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			tstring name(findData.cFileName);
			scripts.push_back(std::pair<tstring,tstring>(name, m_remoteDir + name));
		}
		while(TRUE == FindNextFile(searchHandle.get(), &findData))
		{
			if(0 == (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				tstring name(findData.cFileName);
				scripts.push_back(std::pair<tstring,tstring>(name, m_remoteDir + name));
			}
		}
		if(!scripts.size())
			throw MCException(_T("Script files are not found."));

		/// Create safe array with files
		SAFEARRAYBOUND rgsabound[2];
		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = static_cast<unsigned int>(scripts.size());
		rgsabound[1].lLbound = 0;
		rgsabound[1].cElements = 2;
		CComSafeArray<VARIANT> sarray;
		sarray.Create(rgsabound, 2);

		LONG idx = 0;
		LONG arrIndex[2];
		for(std::list< std::pair<tstring, tstring> >::iterator index = scripts.begin();
			index != scripts.end();
			++index)
		{
			tstring name = index->first;
			tstring fullName = index->second;
			CComBSTR bstrName(name.c_str());
			CComVariant varName(bstrName);

			/// Load file data
			unsigned int fileSize = 0;
			boost::shared_array<char> fileBuf = CFileManager::LoadFromFile(fullName, &fileSize);
			/// Encode data
			tstring encFile = CBase64Encoder::Encode(fileBuf, fileSize);
			CComBSTR content(encFile.c_str());
			CComVariant varContent(content);

			arrIndex[0] = idx;
			arrIndex[1] = 0;
			sarray.MultiDimSetAt(arrIndex, varName);
			arrIndex[1] = 1;
			sarray.MultiDimSetAt(arrIndex, varContent);
			idx++;
		}

		/// Serialize parameters
		CVariantSerializer::SPVariant var(new CComVariantEx(*sarray.GetSafeArrayPtr()));
		unsigned int size = 0;
		CVariantSerializer::SPCharBuf varBuf = CVariantSerializer::VariantToBuf(var, &size);
		/// Create request message
		boost::shared_array<char> buf(new char[size + SCRIPTENGINE_MSG_HEAD_SIZE]);
		SScriptEngineMsg* msg = reinterpret_cast<SScriptEngineMsg*>(buf.get());
		msg->m_size = size + SCRIPTENGINE_MSG_HEAD_SIZE;
		msg->m_type = semtDeploy;
		msg->m_id = request->m_id;
		memcpy(msg->m_data, varBuf.get(), size);
		/// Send request
		CheckStream();
		m_stream->Send(reinterpret_cast<char*>(msg), msg->m_size);
	}
	catch(CExceptionBase& ex)
	{
		TRY_CATCH
			m_notificationThread.NotifyActivity(_T(""));
			RaiseCustomEvent(seceDeployComplete, request->m_id, false, ex.m_strWhat);
		CATCH_LOG()
	}
	catch(...)
	{
		TRY_CATCH
			m_notificationThread.NotifyActivity(_T(""));
			RaiseCustomEvent(seceDeployComplete, request->m_id, false, _T("Unknown deploy error."));
		CATCH_LOG()
	}
	return request->m_id;
CATCH_THROW()
}

void CScriptEngineClient::InvokeFileInternal(bool async, SPExecEnvironment env)
{
TRY_CATCH
	m_notificationThread.NotifyActivity(GetActivity(seatInvokeFile));
	/// Create request
	SPScriptEngineRequest request = CreateRequest(env->m_id, async);
	try
	{
		/// Check parameters, objects will be converted to strings if it possible
		CComVariant tmpParam1(env->m_inputParam1);
		CComVariant tmpParam2(env->m_inputParam2);
		CComVariant tmpParam1Object(false);
		CComVariant tmpParam2Object(false);
		if(VT_DISPATCH == tmpParam1.vt)
		{
			tmpParam1 = m_script.ObjectToString(tmpParam1);
			tmpParam1Object = true;
		}
		if(VT_DISPATCH == tmpParam2.vt)
		{
			tmpParam2 = m_script.ObjectToString(tmpParam2);
			tmpParam2Object = true;
		}

		/// Create safe array with parameters
		SAFEARRAYBOUND rgsabound[1];
		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = 8;

		CComSafeArray<VARIANT> sarray;
		sarray.Create(rgsabound);
		sarray[0] = env->m_lang;
		sarray[1] = env->m_file;
		sarray[2] = env->m_proc;
		VariantCopy(&sarray[3], &tmpParam1);
		VariantCopy(&sarray[4], &tmpParam2);
		sarray[5] = tmpParam1Object;
		sarray[6] = tmpParam2Object;
		sarray[7] = CComVariant(request->m_async);

		/// Serialize array
		CVariantSerializer::SPVariant var(new CComVariantEx(*sarray.GetSafeArrayPtr()));
		unsigned int size = 0;
		CVariantSerializer::SPCharBuf varBuf = CVariantSerializer::VariantToBuf(var, &size);
		boost::scoped_array<char> buf(new char[size + SCRIPTENGINE_MSG_HEAD_SIZE]);
		SScriptEngineMsg* msg = reinterpret_cast<SScriptEngineMsg*>(buf.get());
		msg->m_size = size + SCRIPTENGINE_MSG_HEAD_SIZE;
		msg->m_type = semtExecFile;
		memcpy(msg->m_data, varBuf.get(), size);
		msg->m_id = env->m_id;

		/// Send request
		CheckStream();
		m_stream->Send(buf.get(), msg->m_size);
	}
	catch(CExceptionBase& ex)
	{
		TRY_CATCH
			m_notificationThread.NotifyActivity(_T(""));
			m_eventThreadsManager.AddEvent(
				boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
				seceCompletion,
				env->m_id, 
				false, 
				ex.m_strWhat);
		CATCH_LOG()
	}
	catch(...)
	{
		TRY_CATCH
			m_notificationThread.NotifyActivity(_T(""));
			m_eventThreadsManager.AddEvent(
				boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
				seceCompletion,
				env->m_id, 
				false, 
				_T("Unknown error at invoking procedure."));
		CATCH_LOG()
	}
CATCH_THROW()
}

void CScriptEngineClient::InvokeCodeInternal(bool async, SPExecEnvironment env)
{
TRY_CATCH
	m_notificationThread.NotifyActivity(GetActivity(seatInvokeCode));
	/// Create request
	SPScriptEngineRequest request = CreateRequest(env->m_id, async);
	try
	{
		/// Check parameters, objects will be converted to strings if it possible
		CComVariant tmpParam1(env->m_inputParam1);
		CComVariant tmpParam2(env->m_inputParam2);
		CComVariant tmpParam1Object(false);
		CComVariant tmpParam2Object(false);
		if(VT_DISPATCH == tmpParam1.vt)
		{
			tmpParam1 = m_script.ObjectToString(tmpParam1);
			tmpParam1Object = true;
		}
		if(VT_DISPATCH == tmpParam2.vt)
		{
			tmpParam2 = m_script.ObjectToString(tmpParam2);
			tmpParam2Object = true;
		}

		/// Create safe array with parameters
		SAFEARRAYBOUND rgsabound[1];
		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = 8;

		CComSafeArray<VARIANT> sarray;
		sarray.Create(rgsabound);
		sarray[0] = env->m_lang;
		sarray[1] = env->m_code;
		sarray[2] = env->m_proc;
		VariantCopy(&sarray[3], &tmpParam1);
		VariantCopy(&sarray[4], &tmpParam2);
		sarray[5] = tmpParam1Object;
		sarray[6] = tmpParam2Object;
		sarray[7] = CComVariant(request->m_async);

		/// Serialize array
		CVariantSerializer::SPVariant var(new CComVariantEx(*sarray.GetSafeArrayPtr()));
		unsigned int size = 0;
		CVariantSerializer::SPCharBuf varBuf = CVariantSerializer::VariantToBuf(var, &size);
		boost::scoped_array<char> buf(new char[size + SCRIPTENGINE_MSG_HEAD_SIZE]);
		SScriptEngineMsg* msg = reinterpret_cast<SScriptEngineMsg*>(buf.get());
		msg->m_size = size + SCRIPTENGINE_MSG_HEAD_SIZE;
		msg->m_type = semtExecCode;
		memcpy(msg->m_data, varBuf.get(), size);
		msg->m_id = env->m_id;

		/// Send request
		CheckStream();
		m_stream->Send(buf.get(), msg->m_size);
	}
	catch(CExceptionBase& ex)
	{
		TRY_CATCH
			m_notificationThread.NotifyActivity(_T(""));
			m_eventThreadsManager.AddEvent(
				boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
				seceCompletion,
				env->m_id, 
				false, 
				ex.m_strWhat);
		CATCH_LOG()
	}
	catch(...)
	{
		TRY_CATCH
			m_notificationThread.NotifyActivity(_T(""));
			m_eventThreadsManager.AddEvent(
				boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
				seceCompletion,
				env->m_id, 
				false, 
				_T("Unknown error at invoking procedure."));
		CATCH_LOG()
	}
CATCH_THROW()
}

unsigned int CScriptEngineClient::GetRemoteFilesInternal(bool async, VARIANT files)
{
TRY_CATCH
	m_notificationThread.NotifyActivity(GetActivity(seatGetRemoteFiles));
	/// Create request
	SPScriptEngineRequest request = CreateRequest(GetRequestIndex(), async);
	try
	{
		CVariantSerializer::SPVariant var(new CComVariantEx);
		/// Check parameter type
		switch(files.vt)
		{
		case VT_BSTR:
			{
				/// Parameter is string, create safe array with single element
				SAFEARRAY FAR* psa;
				SAFEARRAYBOUND rgsabound[1];
				rgsabound[0].lLbound = 0;
				rgsabound[0].cElements = 1;
				psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);

				long indexes[] = {0};
				CComVariant tmpVar(files);
				SafeArrayPutElement(psa, indexes, &tmpVar);

				var->vt = VT_ARRAY | VT_VARIANT;
				var->parray = psa;
			}
			break;
		case (VT_ARRAY | VT_VARIANT):
			{
				/// Parameter is safe array, copy arry
				VariantCopy(var.get(), &files);
			}
			break;
		case VT_DISPATCH:
			{
				/// Parameter is object, chech if it is a JS array and convert to safe array if possible
				CComPtr<IDispatch> disp(files.pdispVal);
				/// Get number of elements in array
				CComVariant result;
				CComBSTR prop(_T("length"));
				if(!CScriptObjectWrapper::IdentificatorExists(disp, prop))
					throw MCException(_T("Parameter is not array or string."));
				CScriptObjectWrapper::GetProperty(disp, prop, &result);
				if(VT_I4 != result.vt)
					throw MCException(_T("Invalid array size."));
				LONG count = result.lVal;

				/// Create safe array
				SAFEARRAY FAR* psa;
				SAFEARRAYBOUND rgsabound[1];
				rgsabound[0].lLbound = 0;
				rgsabound[0].cElements = count;
				psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
				var->vt = VT_ARRAY | VT_VARIANT;
				var->parray = psa;

				/// Copy array
				CComBSTR method(_T("slice"));
				CComVariant arg(0);
				CComVariant newArray;
				if(!CScriptObjectWrapper::IdentificatorExists(disp, method))
					throw MCException(_T("Parameter is not array or string."));
				CScriptObjectWrapper::ExecuteMethod(disp, method, &arg, 1, &newArray);
				if(VT_DISPATCH != newArray.vt)
					throw MCException("Can not copy array.");
				CComPtr<IDispatch> newArrayDisp(newArray.pdispVal);
				method = CComBSTR(_T("shift"));
				if(!CScriptObjectWrapper::IdentificatorExists(newArrayDisp, method))
					throw MCException(_T("Parameter is not array or string."));
				/// Copy elements to safe array
				for(LONG i = 0; i < count; ++i)
				{
					long indexes[] = {i};
					CComVariant tmpVar;
					CScriptObjectWrapper::ExecuteMethod(newArrayDisp, method, NULL, 0, &tmpVar);
					if(VT_BSTR != tmpVar.vt)
						throw MCException(_T("Array contains not string element."));
					SafeArrayPutElement(psa, indexes, &tmpVar);
				}
			}
			break;
		default:
			throw MCException(_T("Invalid argument type."));
		}
		/// Serialize parameters
		unsigned int size = 0;
		CVariantSerializer::SPCharBuf varBuf = CVariantSerializer::VariantToBuf(var, &size);
		boost::scoped_array<char> buf(new char[size + SCRIPTENGINE_MSG_HEAD_SIZE]);
		SScriptEngineMsg* msg = reinterpret_cast<SScriptEngineMsg*>(buf.get());
		msg->m_size = size + SCRIPTENGINE_MSG_HEAD_SIZE;
		msg->m_type = semtGetFiles;
		msg->m_id = request->m_id;
		memcpy(msg->m_data, varBuf.get(), size);
		/// Send request
		CheckStream();
		m_stream->Send(buf.get(), msg->m_size);
	}
	catch(CExceptionBase& ex)
	{
		TRY_CATCH
			m_notificationThread.NotifyActivity(_T(""));
			RaiseCustomEvent(seceFilesTransferred, request->m_id, false, ex.m_strWhat);
		CATCH_LOG()
	}
	catch(...)
	{
		TRY_CATCH
			m_notificationThread.NotifyActivity(_T(""));
			RaiseCustomEvent(seceFilesTransferred, request->m_id, false, _T("Unknown error at getting remote files."));
		CATCH_LOG()
	}
	return request->m_id;
CATCH_THROW()
}

void CScriptEngineClient::HandleProtocolMessage(unsigned int requestId, EScriptEngineMsgType msgType, boost::shared_array<char> msgData, const unsigned int size)
{
TRY_CATCH
	CVariantSerializer::SPVariant var;
	if(size > 0)
		var = CVariantSerializer::BufToVariant(msgData, size);
	switch(msgType)
	{
	case semtOnError: /// Error occurs at remote script execution
		HandleOnErrorMsg(requestId, var);
		break;
	case semtOnTimeout: /// Timeout expired at remote script execution
		HandleOnTimeoutMsg(requestId, var);
		break;
	case semtOnSuccess: /// Successful execution of remote script
		HandleOnSuccessMsg(requestId, var);
		break;
	case semtOnDeploy: /// OnDeploy message received
		HandleOnDeployMsg(requestId, var);
		break;
	case semtSetParams: /// SetReturnParameters message received
		HandleSetParamsMsg(requestId, var);
		break;
	case semtOnGetFiles: /// Remote files received
		HandleOnGetFilesMsg(requestId, var);
		break;
	default:
		throw MCException(_T("Invalid message type"));
	}
	m_notificationThread.NotifyActivity(_T(""));
CATCH_THROW()
}

void CScriptEngineClient::HandleOnErrorMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg)
{
TRY_CATCH
	USES_CONVERSION;
	/// Chech parameter
	if(VT_BSTR != msg->vt)
		throw MCException(_T("Invalid parameter type."));
	tstring errString = OLE2T(msg->bstrVal);
	/// Execute completion routine
	m_eventThreadsManager.AddEvent(
		boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
		seceCompletion,
		requestId, 
		false, 
		errString);
CATCH_THROW()
}

void CScriptEngineClient::HandleOnTimeoutMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg)
{
TRY_CATCH
	/// Execute completion routine
	m_eventThreadsManager.AddEvent(
		boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
		seceCompletion,
		requestId, 
		false, 
		_T("Script execution timeout expired."));
CATCH_THROW()
}

void CScriptEngineClient::HandleOnSuccessMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg)
{
TRY_CATCH
	/// Execute completion routine
	m_eventThreadsManager.AddEvent(
		boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
		seceCompletion,
		requestId, 
		true, 
		_T(""));
CATCH_THROW()
}

void CScriptEngineClient::HandleOnDeployMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg)
{
TRY_CATCH
	USES_CONVERSION;
	try
	{
		/// Check parameters
		if((VT_ARRAY | VT_VARIANT) != msg->vt)
			throw MCException(_T("Invalid parameter type."));
		SAFEARRAY* parray = msg->parray;
		if(!parray)
			throw MCException(_T("Invalid parameter structure."));
		CComVariant resultVar;
		CComVariant errorVar;
		long indexes[] = {0};
		HRESULT res = SafeArrayGetElement(parray, indexes, &resultVar);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract deploy result"));
		}
		if(VT_BOOL != resultVar.vt)
			throw MCException(_T("Invalid parameter type."));
		indexes[0] = 1;
		res = SafeArrayGetElement(parray, indexes, &errorVar);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract deploy error"));
		}
		if(VT_BSTR != errorVar.vt)
			throw MCException(_T("Invalid parameter type."));
		CComBSTR errorString(errorVar.bstrVal);
		VARIANT_BOOL success = resultVar.boolVal;
		tstring err = OLE2T(errorString);
		TRY_CATCH
			m_eventThreadsManager.AddEvent(
				boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
				seceDeployComplete,
				requestId, 
				VARIANT_TRUE == success, 
				err);
		CATCH_LOG()
	}
	catch(CExceptionBase& ex)
	{
		TRY_CATCH
			m_eventThreadsManager.AddEvent(
				boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
				seceDeployComplete,
				requestId, 
				false, 
				ex.m_strWhat);
		CATCH_LOG()
		throw;
	}
	catch(...)
	{
		TRY_CATCH
			m_eventThreadsManager.AddEvent(
				boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
				seceDeployComplete,
				requestId, 
				false, 
				_T("Unknown deploy error."));
		CATCH_LOG()
		throw;
	}
CATCH_THROW()
}

void CScriptEngineClient::HandleSetParamsMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg)
{
TRY_CATCH
	/// Check parameters
	if((VT_ARRAY | VT_VARIANT) != msg->vt)
		throw MCException(_T("Invalid parameter type."));
	SAFEARRAY* parray = msg->parray;
	if(!parray)
		throw MCException(_T("Invalid parameter structure."));
	CComVariant param1Var;
	CComVariant param2Var;
	CComVariant param1VarObject;
	CComVariant param2VarObject;
	long indexes[] = {0};
	HRESULT res = SafeArrayGetElement(parray, indexes, &param1Var);
	if(S_OK != res)
	{
		SetLastError(res);
		throw MCException_Win(_T("Failed to extract result parameter 1"));
	}
	indexes[0] = 1;
	res = SafeArrayGetElement(parray, indexes, &param2Var);
	if(S_OK != res)
	{
		SetLastError(res);
		throw MCException_Win(_T("Failed to extract result parameter 2"));
	}
	indexes[0] = 2;
	res = SafeArrayGetElement(parray, indexes, &param1VarObject);
	if(S_OK != res)
	{
		SetLastError(res);
		throw MCException_Win(_T("Failed to extract result parameter 1 type"));
	}
	indexes[0] = 3;
	res = SafeArrayGetElement(parray, indexes, &param2VarObject);
	if(S_OK != res)
	{
		SetLastError(res);
		throw MCException_Win(_T("Failed to extract result parameter 2 type"));
	}
	{
		/// Store parameters in corresponding execution environment
		CCritSection section(&m_envSection);
		VariantCopy(&m_execEnvironments[requestId]->m_params1, &param1Var);
		VariantCopy(&m_execEnvironments[requestId]->m_params2, &param2Var);
		m_execEnvironments[requestId]->m_params1Object = (param1VarObject.boolVal == VARIANT_TRUE);
		m_execEnvironments[requestId]->m_params2Object = (param2VarObject.boolVal == VARIANT_TRUE);
	}
CATCH_THROW()
}

void CScriptEngineClient::HandleOnGetFilesMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg)
{
TRY_CATCH
	USES_CONVERSION;
	/// Check parameters
	if((VT_ARRAY | VT_VARIANT) != msg->vt)
		throw MCException(_T("Invalid parameter type."));
	SAFEARRAY* parray = msg->parray;
	if(!parray)
		throw MCException(_T("Invalid parameter structure."));
	CComVariant resultVar;
	CComVariant errorVar;
	CComVariant filesVar;
	long indexes[] = {0};
	HRESULT res = SafeArrayGetElement(parray, indexes, &resultVar);
	if(S_OK != res)
	{
		SetLastError(res);
		throw MCException_Win(_T("Failed to extract deploy result"));
	}
	if(VT_BOOL != resultVar.vt)
		throw MCException(_T("Invalid parameter type."));
	indexes[0] = 1;
	res = SafeArrayGetElement(parray, indexes, &errorVar);
	if(S_OK != res)
	{
		SetLastError(res);
		throw MCException_Win(_T("Failed to extract deploy error"));
	}
	if(VT_BSTR != errorVar.vt)
		throw MCException(_T("Invalid parameter type."));
	CComBSTR errorString(errorVar.bstrVal);
	VARIANT_BOOL success = resultVar.boolVal;
	/// Check result
	if(FALSE == success)
	{
		/// Obtaining remote files failed
		tstring error = OLE2T(errorString);
		/// Raise "OnFilesTransferred" event
		m_eventThreadsManager.AddEvent(
			boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
			seceFilesTransferred,
			requestId, 
			false, 
			error);
		return;
	}
	try
	{
		/// Extract content of files and write to file system
		indexes[0] = 2;
		res = SafeArrayGetElement(parray, indexes, &filesVar);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract remote files"));
		}
		if((VT_ARRAY | VT_VARIANT) != filesVar.vt)
			throw MCException(_T("Invalid parameter type."));
		SAFEARRAY* files = filesVar.parray;
		for(ULONG i = 0; i < files->rgsabound[1].cElements; ++i)
		{
			CComVariant tmpVar;
			long newIndexes[] = {i, 0};
			SafeArrayGetElement(files, newIndexes, &tmpVar);
			if(VT_BSTR != tmpVar.vt)
				throw MCException(_T("Invalid parameter type."));
			tstring fileName = tstring(OLE2T(tmpVar.bstrVal));
			tstring::size_type pos = fileName.rfind(_T("\\"));
			if(tstring::npos != pos)
			{
				fileName.erase(0, pos + 1);
			}

			VariantClear(&tmpVar);
			long newIndexes2[] = {i, 1};
			SafeArrayGetElement(files, newIndexes2, &tmpVar);
			if(VT_BSTR != tmpVar.vt)
				throw MCException(_T("Invalid parameter type."));
			tstring content = OLE2T(tmpVar.bstrVal);

			int decLen = static_cast<unsigned int>(content.length());
			boost::shared_array<char> buf = CBase64Encoder::Decode(content, &decLen);

			SetCurrentDirectory(m_dirs.GetLocalDirectory().c_str());
			CFileManager::SaveToFile(fileName, buf, decLen);
		}
		TRY_CATCH
			m_eventThreadsManager.AddEvent(
			boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
			seceFilesTransferred,
			requestId, 
			true, 
			_T(""));
		CATCH_LOG()
	}
	catch(CExceptionBase& ex)
	{
		TRY_CATCH
			m_eventThreadsManager.AddEvent(
			boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
			seceFilesTransferred,
			requestId, 
			false, 
			ex.m_strWhat);
		CATCH_LOG()
			throw;
	}
	catch(...)
	{
		TRY_CATCH
			m_eventThreadsManager.AddEvent(
			boost::bind(&CScriptEngineClient::RaiseCustomEvent, this, _1, _2, _3, _4),
			seceFilesTransferred,
			requestId, 
			false, 
			_T("Unknown error at getting remote files."));
		CATCH_LOG()
			throw;
	}
CATCH_THROW()
}

void CScriptEngineClient::CheckStream()
{
TRY_CATCH
	if(!m_stream.get())
		throw MCException(_T("Transport stream is not created."));
CATCH_THROW()
}

CComPtr<IDispatch> CScriptEngineClient::GetSelfDispatch()
{
TRY_CATCH
	CComPtr<IDispatch> dispSelf;
	HRESULT result = QueryInterface(IID_IDispatch, (void**)&dispSelf);
	if((S_OK != result) || !dispSelf)
	{
		SetLastError(result);
		throw MCException_Win(_T("Obtaining of self IDispatch failed"));
	}
	return dispSelf;
CATCH_THROW()
}

unsigned int CScriptEngineClient::GetRequestIndex()
{
TRY_CATCH
	CCritSection section(&m_indexSection);
	m_requestIndex++;
	return m_requestIndex;
CATCH_THROW()
}

void CScriptEngineClient::ExecCompletionRoutine(unsigned int environmentId, bool success, const tstring& errorString)
{
TRY_CATCH
	/// Find execution environment by request id
	SPExecEnvironment env;
	{	
		CCritSection section(&m_envSection);
		std::map<unsigned int,SPExecEnvironment>::iterator index = m_execEnvironments.find(environmentId);
		if(index == m_execEnvironments.end())
			throw MCException(_T("Correcponding execution environment is not found."));
		env = index->second;
		m_execEnvironments.erase(environmentId);
	}
	/// Check name of completion routine
	if(env->m_completion == CComBSTR(L""))
	{
		Log.Add(_MESSAGE_, _T("Completion routine is not added for this execution environment."));
		return;
	}
	/// Create objects
	CComBSTR err(errorString.c_str());
	CComVariant tmpParam1 = env->m_params1;
	CComVariant tmpParam2 = env->m_params2;
	if((VT_BSTR == tmpParam1.vt) && env->m_params1Object)
		tmpParam1 = m_script.StringToObject(tmpParam1);
	if((VT_BSTR == tmpParam2.vt) && env->m_params2Object)
		tmpParam2 = m_script.StringToObject(tmpParam2);
	/// Prepare array of parameters
	boost::scoped_array<CComVariant> args(new CComVariant[4]);
	args[0] = tmpParam2;
	args[1] = tmpParam1;
	args[2] = err;
	args[3] = success;
	CComPtr<IDispatch> disp = m_script.GetGlobalModule();
	/// Execute completion routine
	CScriptObjectWrapper::ExecuteMethod(disp, env->m_completion, args.get(), 4, NULL);
CATCH_THROW()
}

void CScriptEngineClient::RaiseOnDeployComplete(bool success, const tstring& errorString)
{
TRY_CATCH
	SafeInvokeEvent(seceDeployComplete, CComVariant(success), CComVariant(errorString.c_str()), NULL);
CATCH_THROW()
}

void CScriptEngineClient::RaiseOnFilesTransferred(bool success, const tstring& errorString)
{
TRY_CATCH
	SafeInvokeEvent(seceFilesTransferred, CComVariant(success), CComVariant(errorString.c_str()), NULL);
CATCH_THROW()
}

void CScriptEngineClient::RaiseOnHostInited(bool success, const tstring& errorString)
{
TRY_CATCH
	SafeInvokeEvent(seceHostInited, CComVariant(success), CComVariant(errorString.c_str()), NULL);
CATCH_THROW()
}

void CScriptEngineClient::RaiseOnProgress(const tstring& progressMessage)
{
TRY_CATCH
	SafeInvokeEvent(seceProgress, CComVariant(progressMessage.c_str()), NULL);
CATCH_THROW()
}

void CScriptEngineClient::RaiseOnUnexpectedError(int errorCode, const tstring& errorString)
{
TRY_CATCH
	SafeInvokeEvent(seceUnexpectedError, CComVariant(errorCode), CComVariant(errorString.c_str()), NULL);
CATCH_THROW()
}

void CScriptEngineClient::RaiseCustomEvent(int eventId, unsigned int requestId, bool success, const tstring& stringParam)
{
TRY_CATCH
	bool async = true;
	if((seceProgress != eventId) && (seceUnexpectedError != eventId))
	{
		CCritSection section(&m_requestSection);
		std::map<unsigned int,SPScriptEngineRequest>::iterator index = m_requests.find(requestId);
		if(index == m_requests.end())
			throw MCException(_T("Unknown request id."));
		SPScriptEngineRequest request = index->second;
		async = request->m_async;
		if(async)
			m_requests.erase(index);
		else
		{
			request->m_result = success;
			request->m_error = stringParam;
			SetEvent(request->m_completeEvent.get());
			return;
		}
	}
	/// Dispatch event
	switch(eventId)
	{
	case seceCompletion:
		ExecCompletionRoutine(requestId, success, stringParam);
		break;
	case seceHostInited:
		RaiseOnHostInited(success, stringParam);
		break;
	case seceDeployComplete:
		RaiseOnDeployComplete(success, stringParam);
		break;
	case seceFilesTransferred:
		RaiseOnFilesTransferred(success, stringParam);
		break;
	case seceProgress:
		RaiseOnProgress(stringParam);
		break;
	case seceUnexpectedError:
		RaiseOnUnexpectedError(requestId, stringParam);
		break;
	default:
		throw MCException(_T("Invalid event type."));
	}
CATCH_THROW()
}

HRESULT __stdcall CScriptEngineClient::OnDocumentComplete(IDispatch *pDisp, VARIANT *URL)
{
TRY_CATCH_COM
	/// Some document is loaded into web browser
	USES_CONVERSION;

	HRESULT result;
	tstring tmpLocalDoc = LowerCase(m_frameURL);
	tstring tmpURLDoc = LowerCase(OLE2T(URL->bstrVal));
	TCHAR buf[MAX_PATH];
	memset(buf, 0, MAX_PATH);
	DWORD size = MAX_PATH;
	result = PathCreateFromUrl(tmpURLDoc.c_str(), buf, &size, NULL);
	if(S_OK == result)
		tmpURLDoc = LowerCase(buf);
	/// Check URL
	if(tmpLocalDoc != tmpURLDoc)
		return S_OK;
	/// Injected frame is loaded
	/// Store IDispatch for caller
	CComPtr<IDispatch> frameDisp(pDisp);
	/// Get IWebBrowser2 interface
	CComPtr<IWebBrowser2> browser;
	result = frameDisp->QueryInterface(IID_IWebBrowser2, (void**)&browser);
	if((S_OK != result) || !browser)
	{
		SetLastError(result);
		throw MCException_Win(_T("IWebBrowser2 obtaining failed"));
	}
	/// Detach from events
	DispEventUnadvise(browser);
	/// Get IDispatch for document
	CComPtr<IDispatch> dispDocument;
	result = browser->get_Document(&dispDocument);
	if((S_OK != result) || !dispDocument)
	{
		SetLastError(result);
		throw MCException_Win(_T("Document IDispatch obtaining failed"));
	}
	/// Get IHTMLDocument interface
	CComPtr<IHTMLDocument> document;
	result = dispDocument->QueryInterface(IID_IHTMLDocument, (void**)&document);
	if((S_OK != result) || !document)
	{
		SetLastError(result);
		throw MCException_Win(_T("IHTMLDocument obtaining failed"));
	}
	/// Get interface of global script module
	CComPtr<IDispatch> script;
	result = document->get_Script(&script);
	if((S_OK != result) || !script)
	{
		SetLastError(result);
		throw MCException_Win(_T("IHTMLDocument->get_Script failed"));
	}
	/// Save interface
	m_script.SetGlobalModule(script);
	/// Get IDispatch of <object> element
	CComPtr<IDispatch> dispElem = m_objectDisp.GetInterface();
	/// Check up existance of "ScriptEngineClient" variable
	CComBSTR varName(ENGINE_VARIABLE);
	if(m_script.IdentificatorExists(script, varName))
	{
		/// Set value of "ScriptEngineClient" variable
		m_script.SetScriptObject(varName, dispElem);
	}
	else
	{
		/// Add "ScriptEngineClient" variable to script, set value as <object> element
		m_script.AddObject(varName, dispElem);
	}
	/// Add JSON object if it does not exist
	if(!m_script.IdentificatorExists(script, JS_JSONOBJECT_INSTANCE.GetVarName()))
		m_script.AddCode(JS_JSONOBJECT_INSTANCE.GetCode());
CATCH_LOG_COM
}

int CScriptEngineClient::OnUnZipPasswd(char* buf, int size)
{
TRY_CATCH
	tstring passwd(UNZIP_PASSWD);
	memcpy(buf, passwd.c_str(), max(static_cast<unsigned int>(size - 1), passwd.length()));
	return 0;
CATCH_THROW()
}

SPScriptEngineRequest CScriptEngineClient::CreateRequest(unsigned int requestId, bool async)
{
TRY_CATCH
	/// Create request
	SPScriptEngineRequest request(new SScriptEngineRequest(requestId, async));
	{
		/// Add request to map
		CCritSection section(&m_requestSection);
		m_requests[request->m_id] = request;
	}
	return request;
CATCH_THROW()
}

SPExecEnvironment CScriptEngineClient::CreateEnvironment(CComBSTR lang, CComBSTR fileName, CComBSTR code, CComBSTR procedureName, CComBSTR completionName, CComVariant param1, CComVariant param2)
{
TRY_CATCH
	/// Add new execution environment
	SPExecEnvironment env(new SExecEnvironment);
	env->m_id = GetRequestIndex();
	env->m_completion = completionName;
	env->m_lang = lang;
	env->m_proc = procedureName;
	env->m_code = code;
	env->m_file = fileName;
	env->m_inputParam1 = param1;
	env->m_inputParam2 = param2;
	{
		CCritSection section(&m_envSection);
		m_execEnvironments[env->m_id] = env;
	}
	return env;
CATCH_THROW()
}

void CScriptEngineClient::WaitForResponse(unsigned int requestId)
{
TRY_CATCH
	try
	{
		SPScriptEngineRequest request;
		/// Find request by Id
		{
			CCritSection section(&m_requestSection);
			std::map<unsigned int,SPScriptEngineRequest>::iterator index = m_requests.find(requestId);
			if(index == m_requests.end())
				throw MCException(_T("Invalid request id."));
			request = index->second;
		}
		/// Wait for response
		DWORD result = WaitForSingleObjectWithMessageLoop(request->m_completeEvent.get(), m_syncTimeout);
		/// Remove request
		{
			CCritSection section(&m_requestSection);
			std::map<unsigned int,SPScriptEngineRequest>::iterator index = m_requests.find(requestId);
			if(index != m_requests.end())
				request = index->second;
		}
		switch(result)
		{
		case WAIT_FAILED:	/// WaitForSingleObject failed
			throw MCException_Win("Failed to WaitForSingleObject");
		case WAIT_OBJECT_0:	/// Event signaled
			{
				if(!request->m_result)
					throw MCException(request->m_error.c_str());
			}
			break;
		case WAIT_TIMEOUT:	/// Timeout happened
			throw MCException("Method execution timeout expired.");
		default:
			throw MCException_Win("Unknown result from WaitForSingleObject");
		}
	}
	catch(...)
	{
		m_notificationThread.NotifyActivity(_T(""));
		throw;
	}
CATCH_THROW()
}

void CScriptEngineClient::PrepareExecutionResult(unsigned int requestId, VARIANT* result)
{
TRY_CATCH
	try
	{
		if(!result)
			throw MCException(_T("Invalid pointer to output parameter."));
		/// Find and remove execution environment 
		CCritSection section(&m_envSection);
		std::map<unsigned int,SPExecEnvironment>::iterator index = m_execEnvironments.find(requestId);
		if(index == m_execEnvironments.end())
			throw MCException(_T("Cannot find execution environment."));
		SPExecEnvironment env = index->second;
		m_execEnvironments.erase(index);
		section.Unlock();
		/// Create return value
		CComVariant tmpParam1 = env->m_params1;
		if((VT_BSTR == tmpParam1.vt) && env->m_params1Object)
			tmpParam1 = m_script.StringToObject(tmpParam1);
		VariantCopy(result, &tmpParam1);
	}
	catch(...)
	{
		m_notificationThread.NotifyActivity(_T(""));
		throw;
	}
CATCH_THROW()
}

tstring CScriptEngineClient::GetActivity(ESEActivityType activityType)
{
TRY_CATCH
	static PTCHAR activities[] = {
		EMPTY_ACTIVITY,
		DEPLOY_ACTIVITY,
		INVOKE_FILE_ACTIVITY,
		INVOKE_CODE_ACTIVITY,
		GET_FILES_ACTIVITY,
	};
	if(_T("") == m_activityString)
		return activities[activityType];
	else
	{
		tstring str = m_activityString;
		m_activityString = _T("");
		return str;
	}
CATCH_THROW()
}

