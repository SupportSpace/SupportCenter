/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CScriptEngineHost.cpp
///
///  Implements CScriptEngineHost class, responsible for ScriptEngineHost COM object
///
///  @author Dmitry Netrebenko @date 15.11.2007
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CScriptEngineHost.h"
#include <AidLib/Com/CVariantSerializer.h>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include "SScriptEngineMsg.h"
#include <AidLib/CException/CException.h>
#include <AidLib/Com/ComException.h>
#include "CFileManager.h"
#include <boost/bind.hpp>
#include <atlsafe.h>
#include <boost/type_traits/remove_pointer.hpp>
#include "CBase64Encoder.h"


CScriptEngineHost::CScriptEngineHost()
	:	CInstanceTracker(_T("CScriptEngineHost"))
	,	m_cleaner()
	,	m_scriptsDir(DEFAULT_HOST_SCRIPT_DIR)
	,	m_execTimeout(DEFAULT_HOST_EXEC_TIMEOUT)
	,	m_requestId(0)
	,	m_userCancelled(false)
	,	m_notificationEnabled(false)
	,	m_watcher()
{
TRY_CATCH
	TCHAR buf[MAX_PATH];
	memset(buf, 0, MAX_PATH);
	if(!GetTempPath(MAX_PATH, buf))
		throw MCException(_T("GetTempPath failed."));
	m_scriptsDir = tstring(buf) + tstring(_T("\\")) + ::GetGUID() + tstring(_T("\\"));
	if(FALSE == CreateDirectory(m_scriptsDir.c_str(), NULL))
		throw MCException_Win(_T("Create temp directory for scripts failed"));
	if(FALSE == SetCurrentDirectory(m_scriptsDir.c_str()))
		throw MCException_Win(_T("SetCurrentDirectory failed"));
	m_cleaner.AddDirectory(m_scriptsDir);
	/// Create event
	m_userStopEvent.reset(CreateEvent(NULL, FALSE, FALSE, NULL), CloseHandle);
CATCH_THROW()
}

CScriptEngineHost::~CScriptEngineHost()
{
TRY_CATCH
	OnServiceDestroyed();
	m_brokerEvents.Revoke();
CATCH_LOG()
}

STDMETHODIMP CScriptEngineHost::Init(IUnknown *events)
{
TRY_CATCH_COM
	Log.Add(_MESSAGE_,_T("CScriptEngineHost::Ini(0x%08x) m_dwRef=0x%x"),events,m_dwRef);
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

	/// Requesting stream
	if((result=brokerEvents->GetSubStream(CComBSTR(BUSERIDPDV_AUTOSET), BSVCIDPDV_AUTOSET, SESSID_SE, SESSP_SERVICE))!=S_OK)
	{
		SetLastError(result);
		throw MCException_Win("Service substream obtaining failed");
	}

CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineHost::HandleRequest(BSTR dstUserId,ULONG dstSvcId,BSTR srcUserId,ULONG srcSvcId,ULONG rId,ULONG rType,ULONG param,BSTR params)
{
TRY_CATCH_COM

	USES_CONVERSION;
	switch(rType)
	{
		case BRT_PING:
			{
				Log.Add(_MESSAGE_,_T("CScriptEngineHost::HandleRequest(BRT_PING)"));
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
				Log.Add(_MESSAGE_,_T("CScriptEngineHost::HandleRequest(BRT_PING|BRT_RESPONSE)"));
			}
			break;
		case BRT_SERVICE_DESTROYED:
			{
				m_watcher.EnableTermination(true);
				Log.Add(_MESSAGE_,_T("CScriptEngineHost::HandleRequest(BRT_SERVICE_DESTROYED)"));
				OnServiceDestroyed();
				SetEvent(m_userStopEvent.get());
			}
			break;
		case BRT_SET_SCRIPTNAME:
			{
				m_scriptName = OLE2T(params);
				if(!m_brokerEvents.m_dwCookie)
					throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
				HRESULT result;
				CComPtr<_IBrokerClientEvents> brokerEvents;
				if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
					throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
				if(!brokerEvents.p)
					throw MCException("_IBrokerClientEvents has not marshaled");
				brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_SRV_STATE_CHANGED,ESM_SERVICE_STARTED,CComBSTR(m_scriptName.c_str()));
				m_notificationEnabled = true;
				m_watcher.EnableTermination(false);
			}
			break;
		case BRT_STOP_SERVICE:
			{
				m_watcher.EnableTermination(true);
				m_notificationEnabled = false;
				OnUserStopService();
			}
			break;
		case BRT_SRV_STATE_CHANGED:
			{
				if(m_notificationEnabled)
				{
					if(!m_brokerEvents.m_dwCookie)
						throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
					HRESULT result;
					CComPtr<_IBrokerClientEvents> brokerEvents;
					if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
						throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
					if(!brokerEvents.p)
						throw MCException("_IBrokerClientEvents has not marshaled");
					brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_SRV_STATE_CHANGED,param,CComBSTR(params));
				}
			}
			break;
		case BRT_MESSAGEBOX|BRT_RESPONSE:
			{
				if(0 == param) /// User approved the request
				{
					m_notificationEnabled = true;
					m_userCancelled = false;
					SetEvent(m_userStopEvent.get());
					if(!m_brokerEvents.m_dwCookie)
						throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
					HRESULT result;
					CComPtr<_IBrokerClientEvents> brokerEvents;
					if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
						throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
					if(!brokerEvents.p)
						throw MCException("_IBrokerClientEvents has not marshaled");
					brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_SRV_STATE_CHANGED,ESM_SERVICE_STARTED,CComBSTR(m_scriptName.c_str()));
					m_watcher.EnableTermination(false);
				} else
					SetEvent(m_userStopEvent.get());
			}
			break;
		case BRT_BROKERPROXY_INFO:
			{
				m_watcher.AddExceptionProcess(param);
			}
			break;
		default:
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Request type is unknown. HandleRequest(%s,0x%x,%s,0x%x,0x%x,0x%x,0x%x,%s)"),OLE2T(dstUserId),dstSvcId,OLE2T(srcUserId),srcSvcId,rId,rType,param,OLE2T(params));
	}
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineHost::SetSubStream(BSTR dstUserId, ULONG dstSvcId, ULONG streamId, ULONG pointer_shared_ptr_stream)
{
TRY_CATCH_COM
	Log.Add(_MESSAGE_,_T("CScriptEngineHost::SetSubStream() pointer_shared_ptr_stream=0x%08x"),pointer_shared_ptr_stream);

	USES_CONVERSION;
	switch(streamId)
	{
		case SESSID_SE:
			{
				Log.Add(_MESSAGE_,_T("CScriptEngineHost::SetSubStream(SESSID_SE)"));
				boost::shared_ptr<CAbstractStream>* pstream = new boost::shared_ptr<CAbstractStream>(*reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream));
				m_stream = *reinterpret_cast<boost::shared_ptr<CAbstractStream>*>(pointer_shared_ptr_stream);
				InitReceiver(m_stream);
				break;
			}
		default:
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Sub stream id is unknown. SetSbuStream(%s,0x%x,0x%x,0x%08x)"),OLE2T(dstUserId),dstSvcId,streamId,pointer_shared_ptr_stream);
	}
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineHost::SetReturnParameters(VARIANT param1, VARIANT param2)
{
TRY_CATCH_COM
	CComVariant tmpParam1(param1);
	CComVariant tmpParam2(param2);
	SetReturnParametersInternal(tmpParam1, tmpParam2);
	m_returnParamsSet = true;
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineHost::WriteLogError(BSTR errorMsg)
{
TRY_CATCH_COM
	USES_CONVERSION;
	tstring text = OLE2T(errorMsg);
	Log.Add(_ERROR_, text.c_str());
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineHost::WriteLogWarning(BSTR warnMsg)
{
TRY_CATCH_COM
	USES_CONVERSION;
	tstring text = OLE2T(warnMsg);
	Log.Add(_WARNING_, text.c_str());
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineHost::WriteLogMessage(BSTR msgText)
{
TRY_CATCH_COM
	USES_CONVERSION;
	tstring text = OLE2T(msgText);
	Log.Add(_MESSAGE_, text.c_str());
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineHost::get_Arguments(IDispatch** pVal)
{
TRY_CATCH_COM
	if(!pVal)
		throw MCException(_T("Invalid pointer to output parameter."));
	IScriptEngineHost* host = static_cast<IScriptEngineHost*>(this);
	HRESULT res = host->QueryInterface(__uuidof(IDispatch), (void**)pVal);
	if(S_OK != res)
	{
		SetLastError(res);
		throw MCException_Win(_T("Failed to obtain IDispatch from IScriptEngineHost"));
	}
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineHost::Count(LONG* outCount)
{
TRY_CATCH_COM
	if(!outCount)
		throw MCException(_T("Invalid pointer to output parameter."));
	*outCount = 2;
CATCH_LOG_COM
}

STDMETHODIMP CScriptEngineHost::Item(LONG index, VARIANT* outItem)
{
TRY_CATCH_COM
	if((index < 0) || (index > 1))
		throw MCException(_T("Index out of range."));
	if(!outItem)
		throw MCException(_T("Invalid pointer to output parameter."));
	VariantCopy(outItem, &m_execParameters[index]);
CATCH_LOG_COM
}

void CScriptEngineHost::SendOnErrorMsg(const tstring& error)
{
TRY_CATCH
	/// Create variant witn error string
	CVariantSerializer::SPVariant var(new CComVariantEx);
	CComBSTR err(error.c_str());
	var->vt = VT_BSTR;
	var->bstrVal = err;
	/// Serialize parameters
	unsigned int size = 0;
	CVariantSerializer::SPCharBuf varBuf = CVariantSerializer::VariantToBuf(var, &size);
	boost::scoped_array<char> buf(new char[size + SCRIPTENGINE_MSG_HEAD_SIZE]);
	SScriptEngineMsg* msg = reinterpret_cast<SScriptEngineMsg*>(buf.get());
	msg->m_size = size + SCRIPTENGINE_MSG_HEAD_SIZE;
	msg->m_type = semtOnError;
	msg->m_id = m_requestId;
	memcpy(msg->m_data, varBuf.get(), size);
	/// Send message
	CheckStream();
	m_stream->Send(buf.get(), msg->m_size);
CATCH_THROW()
}

void CScriptEngineHost::SendOnTimeoutMsg()
{
TRY_CATCH
	/// Prepare message
	SScriptEngineMsg msg;
	msg.m_size = SCRIPTENGINE_MSG_HEAD_SIZE;
	msg.m_type = semtOnTimeout;
	msg.m_id = m_requestId;
	/// Send message
	CheckStream();
	m_stream->Send(reinterpret_cast<char*>(&msg), msg.m_size);
CATCH_THROW()
}

void CScriptEngineHost::SendOnSuccessMsg()
{
TRY_CATCH
	/// Prepare message
	SScriptEngineMsg msg;
	msg.m_size = SCRIPTENGINE_MSG_HEAD_SIZE;
	msg.m_type = semtOnSuccess;
	msg.m_id = m_requestId;
	/// Send message
	CheckStream();
	m_stream->Send(reinterpret_cast<char*>(&msg), msg.m_size);
CATCH_THROW()
}

void CScriptEngineHost::SendOnDeployMsg(bool success, const tstring& error)
{
TRY_CATCH
	/// Prepare safe array with parameters
	SAFEARRAYBOUND rgsabound[1];
	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = 2;

	CComBSTR err(error.c_str());
	CComSafeArray<VARIANT> sarray;
	sarray.Create(rgsabound);
	sarray[0] = success;
	sarray[1] = err;

	/// Serialize parameters
	CVariantSerializer::SPVariant var(new CComVariantEx(*sarray.GetSafeArrayPtr()));
	unsigned int size = 0;
	CVariantSerializer::SPCharBuf varBuf = CVariantSerializer::VariantToBuf(var, &size);
	boost::shared_array<char> buf(new char[size + SCRIPTENGINE_MSG_HEAD_SIZE]);
	SScriptEngineMsg* msg = reinterpret_cast<SScriptEngineMsg*>(buf.get());
	msg->m_size = size + SCRIPTENGINE_MSG_HEAD_SIZE;
	msg->m_type = semtOnDeploy;
	msg->m_id = m_requestId;
	memcpy(msg->m_data, varBuf.get(), size);
	/// Send message
	CheckStream();
	m_stream->Send(reinterpret_cast<char*>(msg), msg->m_size);
CATCH_THROW()
}

void CScriptEngineHost::SendOnFileTransferredMsg(bool success, const tstring& error, VARIANT* files)
{
TRY_CATCH
	/// Prepare safe array with parameters
	SAFEARRAYBOUND rgsabound[1];
	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = 3;

	CComBSTR err(error.c_str());
	CComSafeArray<VARIANT> sarray;
	sarray.Create(rgsabound);
	sarray[0] = success;
	sarray[1] = err;
	if(files)
		VariantCopy(&sarray[2], files);

	/// Serialize parameters
	CVariantSerializer::SPVariant var(new CComVariantEx(*sarray.GetSafeArrayPtr()));
	unsigned int size = 0;
	CVariantSerializer::SPCharBuf varBuf = CVariantSerializer::VariantToBuf(var, &size);
	boost::shared_array<char> buf(new char[size + SCRIPTENGINE_MSG_HEAD_SIZE]);
	SScriptEngineMsg* msg = reinterpret_cast<SScriptEngineMsg*>(buf.get());
	msg->m_size = size + SCRIPTENGINE_MSG_HEAD_SIZE;
	msg->m_type = semtOnGetFiles;
	msg->m_id = m_requestId;
	memcpy(msg->m_data, varBuf.get(), size);
	/// Send message
	CheckStream();
	m_stream->Send(reinterpret_cast<char*>(msg), msg->m_size);
CATCH_THROW()
}

void CScriptEngineHost::CheckStream()
{
TRY_CATCH
	if(!m_stream.get())
		throw MCException(_T("Transport stream is not created."));
CATCH_THROW()
}

void CScriptEngineHost::HandleProtocolMessage(unsigned int requestId, EScriptEngineMsgType msgType, boost::shared_array<char> msgData, const unsigned int size)
{
TRY_CATCH
	m_requestId = requestId;
	if(m_userCancelled)
	{
		HRESULT result;
		CComPtr<_IBrokerClientEvents> brokerEvents;
		if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
		if(!brokerEvents.p)
			throw MCException("_IBrokerClientEvents has not marshaled");
		tstring& reqParams=Format(BRT_SERVICE_SEFORMAT,m_scriptName.c_str(),BRT_SERVICE_SETEXT,BRT_SERVICE_APPROVE,BRT_SERVICE_DECLINE);
		brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*RID_PERMISSION*/,BRT_MESSAGEBOX,2/*buttons count*/,CComBSTR(reqParams.c_str()));
		if(m_userStopEvent.get())
		{
			WaitForSingleObject(m_userStopEvent.get(), INFINITE);
			if(m_userCancelled)
			{
				TRY_CATCH
					SendOnErrorMsg(BRT_SERVICE_DECLINED);
				CATCH_LOG()
				return;
			}
		}
	}

	CVariantSerializer::SPVariant var;
	if(size > 0)
		var = CVariantSerializer::BufToVariant(msgData, size);
	switch(msgType)
	{
	case semtDeploy: /// "Deploy" request received
		HandleDeployMsg(requestId, var);
		break;
	case semtExecFile: /// "ExecFile" request received
		HandleExecFileMsg(requestId, var);
		break;
	case semtExecCode: /// "ExecCode" request received
		HandleExecCodeMsg(requestId, var);
		break;
	case semtTimeout: /// "SetTimeout" request received
		HandleTimeoutMsg(requestId, var);
		break;
	case semtGetFiles: /// "GetRemoteFiles" request received
		HandleGetFilesMsg(requestId, var);
		break;
	default:
		throw MCException(_T("Invalid message type"));
	}
CATCH_THROW()
}

void CScriptEngineHost::HandleDeployMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg)
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
		for(ULONG i = 0; i < parray->rgsabound[1].cElements; ++i)
		{
			long indexes[] = {i, 0};
			CComVariant nameVar;
			HRESULT res = SafeArrayGetElement(parray, indexes, &nameVar);
			if(S_OK != res)
			{
				SetLastError(res);
				throw MCException_Win(_T("Failed to extract file name"));
			}
			if(VT_BSTR != nameVar.vt)
				throw MCException(_T("File name parameter is not string."));
			long indexes2[] = {i, 1};
			CComVariant contentVar;
			res = SafeArrayGetElement(parray, indexes2, &contentVar);
			if(S_OK != res)
			{
				SetLastError(res);
				throw MCException_Win(_T("Failed to extract file content"));
			}
			if(VT_BSTR != contentVar.vt)
				throw MCException(_T("File content parameter is not string."));

			/// Extract file name and content
			tstring name = OLE2T(nameVar.bstrVal);
			tstring fullName = m_scriptsDir + name;
			tstring content = OLE2T(contentVar.bstrVal);
			/// Decode file content
			int decLen = static_cast<unsigned int>(content.length());
			boost::shared_array<char> buf = CBase64Encoder::Decode(content, &decLen);
			/// Save file on file system
			CFileManager::SaveToFile(fullName, buf, decLen);
		}
		TRY_CATCH
			/// Successful deoloy of file
			SendOnDeployMsg(true, _T(""));
		CATCH_LOG()
	}
	catch(CExceptionBase& ex)
	{
		TRY_CATCH
			SendOnDeployMsg(false, ex.m_strWhat);
		CATCH_LOG()
		throw;
	}
	catch(...)
	{
		TRY_CATCH
			SendOnDeployMsg(false, _T("Unknown deploy error."));
		CATCH_LOG()
		throw;
	}
CATCH_THROW()
}

void CScriptEngineHost::HandleExecFileMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg)
{
TRY_CATCH
	try
	{
		/// Extract parameters and check up
		if((VT_ARRAY | VT_VARIANT) != msg->vt)
			throw MCException(_T("Invalid parameter type."));
		SAFEARRAY* parray = msg->parray;
		if(!parray)
			throw MCException(_T("Invalid parameter structure."));
		CComVariant langVar;
		CComVariant fileVar;
		CComVariant procVar;
		CComVariant param1Var;
		CComVariant param2Var;
		CComVariant param1VarObject;
		CComVariant param2VarObject;
		CComVariant async;
		long indexes[] = {0};
		HRESULT res = SafeArrayGetElement(parray, indexes, &langVar);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract language"));
		}
		indexes[0] = 1;
		res = SafeArrayGetElement(parray, indexes, &fileVar);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract file name"));
		}
		indexes[0] = 2;
		res = SafeArrayGetElement(parray, indexes, &procVar);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract procedure name"));
		}
		indexes[0] = 3;
		res = SafeArrayGetElement(parray, indexes, &param1Var);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract parameter 1"));
		}
		indexes[0] = 4;
		res = SafeArrayGetElement(parray, indexes, &param2Var);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract parameter 2"));
		}
		indexes[0] = 5;
		res = SafeArrayGetElement(parray, indexes, &param1VarObject);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract parameter 1 type"));
		}
		indexes[0] = 6;
		res = SafeArrayGetElement(parray, indexes, &param2VarObject);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract parameter 2 type"));
		}
		indexes[0] = 7;
		res = SafeArrayGetElement(parray, indexes, &async);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract execution mode"));
		}

		USES_CONVERSION;
		tstring name = OLE2T(fileVar.bstrVal);
		tstring fullName = m_scriptsDir + name;
		/// Load script from file
		unsigned int size = 0;
		boost::shared_array<char> buf = CFileManager::LoadFromFile(fullName, &size);
		CComBSTR code(A2T(buf.get()));
		/// Execute
		ExecCode(
			(VARIANT_TRUE == async.boolVal),
			langVar.bstrVal, 
			code, 
			procVar.bstrVal, 
			param1Var, 
			param2Var, 
			(VARIANT_TRUE == param1VarObject.boolVal),
			(VARIANT_TRUE == param2VarObject.boolVal));
	}
	catch(CExceptionBase& ex)
	{
		TRY_CATCH
			SendOnErrorMsg(ex.m_strWhat);
		CATCH_LOG()
		throw;
	}
	catch(...)
	{
		TRY_CATCH
			SendOnErrorMsg(_T("Unknown execution error."));
		CATCH_LOG()
		throw;
	}
CATCH_THROW()
}

void CScriptEngineHost::HandleExecCodeMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg)
{
TRY_CATCH
	try
	{
		/// Extract parameters and checks up
		if((VT_ARRAY | VT_VARIANT) != msg->vt)
			throw MCException(_T("Invalid parameter type."));
		SAFEARRAY* parray = msg->parray;
		if(!parray)
			throw MCException(_T("Invalid parameter structure."));
		CComVariant langVar;
		CComVariant codeVar;
		CComVariant procVar;
		CComVariant param1Var;
		CComVariant param2Var;
		CComVariant param1VarObject;
		CComVariant param2VarObject;
		CComVariant async;
		long indexes[] = {0};
		HRESULT res = SafeArrayGetElement(parray, indexes, &langVar);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract language"));
		}
		indexes[0] = 1;
		res = SafeArrayGetElement(parray, indexes, &codeVar);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract code"));
		}
		indexes[0] = 2;
		res = SafeArrayGetElement(parray, indexes, &procVar);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract procedure name"));
		}
		indexes[0] = 3;
		res = SafeArrayGetElement(parray, indexes, &param1Var);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract parameter 1"));
		}
		indexes[0] = 4;
		res = SafeArrayGetElement(parray, indexes, &param2Var);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract parameter 2"));
		}
		indexes[0] = 5;
		res = SafeArrayGetElement(parray, indexes, &param1VarObject);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract parameter 1 type"));
		}
		indexes[0] = 6;
		res = SafeArrayGetElement(parray, indexes, &param2VarObject);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract parameter 2 type"));
		}
		indexes[0] = 7;
		res = SafeArrayGetElement(parray, indexes, &async);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to extract execution mode"));
		}

		ExecCode(
			(VARIANT_TRUE == async.boolVal),
			langVar.bstrVal, 
			codeVar.bstrVal, 
			procVar.bstrVal, 
			param1Var, 
			param2Var, 
			(VARIANT_TRUE == param1VarObject.boolVal),
			(VARIANT_TRUE == param2VarObject.boolVal));

	}
	catch(CExceptionBase& ex)
	{
		TRY_CATCH
			SendOnErrorMsg(ex.m_strWhat);
		CATCH_LOG()
		throw;
	}
	catch(...)
	{
		TRY_CATCH
			SendOnErrorMsg(_T("Unknown execution error."));
		CATCH_LOG()
		throw;
	}
CATCH_THROW()
}

void CScriptEngineHost::HandleTimeoutMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg)
{
TRY_CATCH
	/// Extract timeout value
	if(VT_I4 != msg->vt)
		throw MCException(_T("Invalid parameter type."));
	m_execTimeout = msg->lVal;
CATCH_THROW()
}

void CScriptEngineHost::HandleGetFilesMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg)
{
TRY_CATCH
	USES_CONVERSION;
	try
	{
		/// Extract parameters and check up
		if((VT_ARRAY | VT_VARIANT) != msg->vt)
			throw MCException(_T("Invalid parameter type."));
		CComVariant files;
		SAFEARRAY* parray = msg->parray;

		/// Create safe array with files
		SAFEARRAY FAR* psa;
		SAFEARRAYBOUND rgsabound[2];
		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = parray->rgsabound[0].cElements;
		rgsabound[1].lLbound = 0;
		rgsabound[1].cElements = 2;
		psa = SafeArrayCreate(VT_VARIANT, 2, rgsabound);

		files.vt = VT_ARRAY | VT_VARIANT;
		files.parray = psa;

		for(ULONG i = 0; i < parray->rgsabound[0].cElements; ++i)
		{
			CComVariant fileName;
			long indexes[] = {i};
			SafeArrayGetElement(parray, indexes, &fileName);
			if(VT_BSTR != fileName.vt)
				throw MCException(_T("Invalid file name parameter."));
			tstring stringFileName = OLE2T(fileName.bstrVal);
			/// Load file from file system
			unsigned int fileSize = 0;
			boost::shared_array<char> fileBuf = CFileManager::LoadFromFile(stringFileName, &fileSize);
			/// Encode file data
			tstring encFile = CBase64Encoder::Encode(fileBuf, fileSize);
			CComBSTR content(encFile.c_str());

			CComVariant tmpNameVar(fileName);
			long newIndexes[] = {i,0};
			SafeArrayPutElement(psa, newIndexes, &tmpNameVar);

			CComVariant tmpContentVar(content);
			long newIndexes2[] = {i,1};
			SafeArrayPutElement(psa, newIndexes2, &tmpContentVar);
		}

		TRY_CATCH
			/// Send "OnFileTransferred" message
			SendOnFileTransferredMsg(true, _T(""), &files);
		CATCH_LOG()
	}
	catch(CExceptionBase& ex)
	{
		TRY_CATCH
			SendOnFileTransferredMsg(false, ex.m_strWhat, NULL);
		CATCH_LOG()
		throw;
	}
	catch(...)
	{
		TRY_CATCH
			SendOnFileTransferredMsg(false, _T("Unknown error at getting remote files."), NULL);
		CATCH_LOG()
		throw;
	}
CATCH_THROW()
}

void CScriptEngineHost::OnServiceDestroyed()
{
TRY_CATCH
	TRY_CATCH
		m_wrapper.reset();
	CATCH_LOG()
	TRY_CATCH
		StopReceiver();
	CATCH_LOG()
CATCH_THROW()
}

void CScriptEngineHost::SetReturnParametersInternal(CComVariant param1, CComVariant param2)
{
TRY_CATCH
	USES_CONVERSION;
	/// Prepare parameters, convert objects to strings if possible
	CComVariant tmpParam1(param1);
	CComVariant tmpParam2(param2);
	CComVariant tmpParam1Object(false);
	CComVariant tmpParam2Object(false);
	if(VT_DISPATCH == tmpParam1.vt)
	{
		if(!m_wrapper)
			throw MCException(_T("ScriptControl is not created."));
		tmpParam1 = m_wrapper->GetScriptObject().ObjectToString(tmpParam1);
		tmpParam1Object = true;
	}
	if(VT_DISPATCH == tmpParam2.vt)
	{
		if(!m_wrapper)
			throw MCException(_T("ScriptControl is not created."));
		tmpParam2 = m_wrapper->GetScriptObject().ObjectToString(tmpParam2);
		tmpParam2Object = true;
	}
	/// Create array with parameters
	SAFEARRAYBOUND rgsabound[1];
	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = 4;

	CComSafeArray<VARIANT> sarray;
	sarray.Create(rgsabound);
	sarray[0] = tmpParam1;
	sarray[1] = tmpParam2;
	sarray[2] = tmpParam1Object;
	sarray[3] = tmpParam2Object;

	/// Serialize parameters
	CVariantSerializer::SPVariant var(new CComVariantEx(*sarray.GetSafeArrayPtr()));
	unsigned int size = 0;
	CVariantSerializer::SPCharBuf varBuf = CVariantSerializer::VariantToBuf(var, &size);
	boost::shared_array<char> buf(new char[size + SCRIPTENGINE_MSG_HEAD_SIZE]);
	SScriptEngineMsg* msg = reinterpret_cast<SScriptEngineMsg*>(buf.get());
	msg->m_size = size + SCRIPTENGINE_MSG_HEAD_SIZE;
	msg->m_type = semtSetParams;
	msg->m_id = m_requestId;
	memcpy(msg->m_data, varBuf.get(), size);
	/// Send message
	CheckStream();
	m_stream->Send(reinterpret_cast<char*>(msg), msg->m_size);
CATCH_THROW()
}

void CScriptEngineHost::SendOnResultMsg(CComVariant result)
{
TRY_CATCH
	if(!m_returnParamsSet)
		SetReturnParametersInternal(result, CComVariant());
CATCH_THROW()
}

void CScriptEngineHost::ParameterDecoded(int index, CComVariant param)
{
TRY_CATCH
	if((index < 0) || (index > 1))
		throw MCException(_T("Index out of range."));
	m_execParameters[index] = param;
CATCH_THROW()
}

void CScriptEngineHost::ClearParameters()
{
TRY_CATCH
	m_execParameters[0] = CComVariant();
	m_execParameters[1] = CComVariant();
CATCH_THROW()
}

void CScriptEngineHost::ExecCode(bool async, 
								 CComBSTR lang, 
								 CComBSTR code, 
								 CComBSTR proc, 
								 CComVariant param1, 
								 CComVariant param2, 
								 bool param1Object, 
								 bool param2Object)
{
TRY_CATCH
	try
	{
		/// Create engine and set properties
		m_wrapper.reset(new CScriptControlWrapper);
		m_wrapper->SetCallbacks(
			boost::bind(&CScriptEngineHost::SendOnSuccessMsg, this),
			boost::bind(&CScriptEngineHost::SendOnTimeoutMsg, this),
			boost::bind(&CScriptEngineHost::SendOnErrorMsg, this, _1),
			boost::bind(&CScriptEngineHost::SendOnResultMsg, this, _1),
			boost::bind(&CScriptEngineHost::ParameterDecoded, this, _1, _2));

		/// Create list of object to inject into script
		CComBSTR engineName(_T("ScriptEngineHost"));
		SPScriptObject selfObject(new SScriptObject(engineName, true));
		IScriptEngineHost* host = static_cast<IScriptEngineHost*>(this);
		HRESULT res = host->QueryInterface(__uuidof(IDispatch), (void**)&selfObject->m_object);
		if(S_OK != res)
		{
			SetLastError(res);
			throw MCException_Win(_T("Failed to obtain IDispatch from IScriptEngineHost"));
		}
		CComBSTR argName(_T("WScript"));
		SPScriptObject argObject(new SScriptObject(argName, true));
		argObject->m_object = selfObject->m_object;
		std::vector<SPScriptObject> objects;
		objects.push_back(selfObject);
		objects.push_back(argObject);
		m_execParameters[0] = param1;
		m_execParameters[1] = param2;
		m_returnParamsSet = false;
		/// Execute code
		m_wrapper->ExecCode(
			async,
			lang, 
			code, 
			proc, 
			param1, 
			param2, 
			param1Object,
			param2Object,
			m_execTimeout, 
			objects);
		ClearParameters();
		m_wrapper.reset();
	}
	catch(CExceptionBase& ex)
	{
		TRY_CATCH
			ClearParameters();
			m_wrapper.reset();
			SendOnErrorMsg(ex.m_strWhat);
		CATCH_LOG()
	}
	catch(...)
	{
		TRY_CATCH
			ClearParameters();
			m_wrapper.reset();
			SendOnErrorMsg(_T("Unknown execution error."));
		CATCH_LOG()
	}
CATCH_THROW()
}

void CScriptEngineHost::OnReceiverStopped()
{
TRY_CATCH
	if(!m_brokerEvents.m_dwCookie)
		throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
	HRESULT result;
	CComPtr<_IBrokerClientEvents> brokerEvents;
	if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
	if(!brokerEvents.p)
		throw MCException("_IBrokerClientEvents has not marshaled");
	brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_SRV_STATE_CHANGED,ESM_SERVICE_STOPPED,CComBSTR(m_scriptName.c_str()));
CATCH_THROW()
}

void CScriptEngineHost::OnUserStopService()
{
TRY_CATCH
	if(!m_userCancelled)
	{
		m_userCancelled = true;
		if(!m_brokerEvents.m_dwCookie)
			throw MCException("_IBrokerClientEvents has not set. Call IBrokerClient::Init() method at first");
		HRESULT result;
		CComPtr<_IBrokerClientEvents> brokerEvents;
		if(S_OK!=(result=m_brokerEvents.CopyTo(&brokerEvents)))
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T("IBrokerClientEvents obtaining failed in Broker")),result);
		if(!brokerEvents.p)
			throw MCException("_IBrokerClientEvents has not marshaled");
		brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_SRV_STATE_CHANGED,ESM_SERVICE_STOPPED,CComBSTR(m_scriptName.c_str()));
		brokerEvents->RequestSent(CComBSTR(BUSERIDPDV_LOCAL),BSVCIDPDV_JS,CComBSTR(BUSERIDPDV_AUTOSET),BSVCIDPDV_AUTOSET,0/*rid*/,BRT_SRV_STATE_CHANGED,ESM_SERVICE_STOPPED,CComBSTR(m_scriptName.c_str()));
	}
CATCH_THROW()
}

