/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CScriptEngineHost.h
///
///  Declares CScriptEngineHost class, responsible for ScriptEngineHost COM object
///
///  @author Dmitry Netrebenko @date 15.11.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "resource.h"       // main symbols

#include <AidLib/Com/CSafeEventsRaiser.h>
#include <AidLib/Strings/tstring.h>
#include <NWL/Streaming/CAbstractStream.h>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include "CProtocolMessageReceiver.h"
#include "CScriptControlWrapper.h"
#include "CCleaner.h"
#include <AidLib/Com/CComVariantEx.h>
#include <boost/type_traits/remove_pointer.hpp>
#include "CChildWatcher.h"

#include <AidLib/Logging/CInstanceTracker.h>
#include "..\..\Brokers\Shared\IBrokerClient.h"
#include "..\..\Brokers\Shared\CAvailableServices.h"
#include "..\..\Brokers\Shared\BrokersTypes.h"

#define DEFAULT_HOST_SCRIPT_DIR		_T("C:\\")
#define DEFAULT_HOST_EXEC_TIMEOUT	30000
#define SCRIPT_FILES_MASK			_T("*.*")

// IScriptEngineHost
[
	object,
	uuid("10F67F14-EFB7-4562-A688-5AA7FD99528C"),
	dual,	helpstring("IScriptEngineHost Interface"),
	pointer_default(unique)
]
__interface IScriptEngineHost : IDispatch
{
	[id(1), helpstring("method SetReturnParameters. Synchronously sends result to client side. ")]	HRESULT SetReturnParameters([in] VARIANT param1, [in] VARIANT param2);
	[id(2), helpstring("method WriteError. Adds error text to log.")]								HRESULT WriteLogError([in] BSTR errorText);
	[id(3), helpstring("method WriteWarning. Adds warning text to log.")]							HRESULT WriteLogWarning([in] BSTR warnText);
	[id(4), helpstring("method WriteMessage. Adds message text to log.")]							HRESULT WriteLogMessage([in] BSTR msgText);
	[propget, id(5), helpstring("property Arguments. Returns IDispatch for arguments.")]			HRESULT Arguments([out, retval] IDispatch** pVal);
	[id(6), helpstring("method Count. Returns count of arguments.")]								HRESULT Count([out,retval] LONG* outCount);
	[id(7), helpstring("method Item. Returns argument by index.")]									HRESULT Item([in] LONG index, [out,retval] VARIANT* outItem);
};


// _IScriptEngineHostEvents
[
	dispinterface,
	uuid("C3296672-7E82-44CF-AE90-747F63E8EE24"),
	helpstring("_IScriptEngineHostEvents Interface")
]
__interface _IScriptEngineHostEvents
{
};


///  CScriptEngineHost class, responsible for ScriptEngineHost COM object
[
	coclass,
	default(IScriptEngineHost, _IScriptEngineHostEvents),
//	threading(free),
	threading(neutral),
	event_source(com),
	vi_progid("ScriptEngine.ScriptEngineHost"),
	progid("ScriptEngine.ScriptEngineHost.1"),
	version(1.0),
	uuid("5DBCAD54-F260-4E79-8D1E-92405C0AB1C5"),
	helpstring("ScriptEngineHost Class")
]
class ATL_NO_VTABLE CScriptEngineHost :
	public IBrokerClient,
	public CInstanceTracker,
	public CProtocolMessageReceiver,
	public IScriptEngineHost
{
public:
	/// ActiveX friendly name, use by registration
	static const TCHAR* GetObjectFriendlyName() 
	{
		return _T("Support Platform Script Engine Host");
	}
	CScriptEngineHost();
	~CScriptEngineHost();

	__event __interface _IScriptEngineHostEvents;


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}
private:
/// Directory for scripts
	tstring													m_scriptsDir;
/// Script execution timeout
	LONG													m_execTimeout;
/// Id of current request
	unsigned int											m_requestId;
/// Transport stream
	boost::shared_ptr<CAbstractStream>						m_stream;
/// Wrapper for ScriptControl
	boost::shared_ptr<CScriptControlWrapper>				m_wrapper;
/// Cleaner to remove temp files and directories
	CCleaner												m_cleaner;
/// Execution parameters
	CComVariant												m_execParameters[2];
/// Is SetReturnParameters called
	bool													m_returnParamsSet;
/// Name of executed script
	tstring													m_scriptName;
/// Event for waiting approve/decline after user stops ScriptEngine
	boost::shared_ptr<boost::remove_pointer<HANDLE>::type>	m_userStopEvent;
/// User stops ScriptEngine flag
	bool													m_userCancelled;
/// Is notifications are enabled
	bool													m_notificationEnabled;
/// Watcher for child processes
	CChildWatcher											m_watcher;
private:
/// Sends OnError message
/// @param error - error string
	void SendOnErrorMsg(const tstring& error);
/// Sends OnTimeout message
	void SendOnTimeoutMsg();
/// Sends OnDeploy message
/// @param success - deploy successful
/// @param error - error description
	void SendOnDeployMsg(bool success, const tstring& error);
/// Sends OnSuccess message
	void SendOnSuccessMsg();
/// Sends OnFileTransferred message
	void SendOnFileTransferredMsg(bool success, const tstring& error, VARIANT* files);
/// Checks up transport stream
	void CheckStream();
/// Handler for BRT_SERVICE_DESTROYED message from infrastructure
/// Terminates script execution, terminates protocol thread, terminates child processes
	void OnServiceDestroyed();
/// Handler for BRT_STOP_SERVICE message from infrastructure
/// Terminates script execution, terminates child processes
	void OnUserStopService();
/// Sends parameters to client
	void SetReturnParametersInternal(CComVariant param1, CComVariant param2);
/// Sends result of execution to client
	void SendOnResultMsg(CComVariant result);
/// Handles "Deploy" message
/// @param requestId - id of request
/// @param data - variant with message data
	void HandleDeployMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg);
/// Handles "ExecFile" message
/// @param requestId - id of request
/// @param data - variant with message data
	void HandleExecFileMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg);
/// Handles "ExecCode" message
/// @param requestId - id of request
/// @param data - variant with message data
	void HandleExecCodeMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg);
/// Handles "Timeout" message
/// @param requestId - id of request
/// @param data - variant with message data
	void HandleTimeoutMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg);
/// Handles "GetFiles" message
/// @param requestId - id of request
/// @param data - variant with message data
	void HandleGetFilesMsg(unsigned int requestId, boost::shared_ptr<CComVariantEx> msg);
/// Callback for decoding parameter
/// @param index - index of parameter
/// @param param - parameter value
	void ParameterDecoded(int index, CComVariant param);
/// Destroys execution parameters
	void ClearParameters();
/// Executes script
/// @param async - async execution mode
/// @param lang - script language
/// @param code - code for execution
/// @param proc - procedure name
/// @param param1 - procedure parameters
/// @param param2 - procedure parameters
/// @param timeout - execution timeout
/// @param objects - vector of object to register in script
	void ExecCode(bool async, CComBSTR lang, CComBSTR code, CComBSTR proc, CComVariant param1, CComVariant param2, bool param1Object, bool param2Object);
protected:

/// borker intercommunication
	CComGITPtr<_IBrokerClientEvents> m_brokerEvents;

/// Handles received message
/// @param requestId - id of request
/// @param msgType - type of message
/// @param msgData - buffer with message's data
/// @param size - buffer size
	virtual void HandleProtocolMessage(unsigned int requestId, EScriptEngineMsgType msgType, boost::shared_array<char> msgData, const unsigned int size);

/// Raised when reciever's thread is stopped
	virtual void OnReceiverStopped();
public:
/// IScriptEngineHost interface realization
	STDMETHOD(SetReturnParameters)(VARIANT param1, VARIANT param2);
/// Writes error to log file
	STDMETHOD(WriteLogError)(BSTR errorText);
/// Writes warning to log file
	STDMETHOD(WriteLogWarning)(BSTR warnText);
/// Writes message to log file
	STDMETHOD(WriteLogMessage)(BSTR msgText);
/// Returns interface for arguments collection (stub)
	STDMETHOD(get_Arguments)(IDispatch** pVal);
/// Returns count of arguments
	STDMETHOD(Count)(LONG* outCount);
/// Returns argument by index
	STDMETHOD(Item)(LONG index, VARIANT* outItem);

/// The method initialize viewer by _IBrokerClientEvents interface
	STDMETHOD(Init)(IUnknown *events);
/// The method handle request sent by Broker
	STDMETHOD(HandleRequest)(BSTR dstUserId,ULONG dstSvcId,BSTR srcUserId,ULONG srcSvcId,ULONG rId,ULONG rType,ULONG param,BSTR params);
/// The method call when asked sub stream connected
	STDMETHOD(SetSubStream)(BSTR dstUserId, ULONG dstSvcId, ULONG streamId, ULONG pointer_shared_ptr_stream);
};

