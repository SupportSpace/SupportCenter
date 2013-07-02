/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CScriptControlWrapper.h
///
///  Declares CScriptControlWrapper class, wrapper for ScriptControl object
///
///  @author Dmitry Netrebenko @date 19.11.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CScriptControlEventHandler.h"
#include <boost/function.hpp>
#include "SScriptObject.h"
#include <vector>
#include "CScriptObjectWrapper.h"

///  CScriptControlWrapper class, wrapper for ScriptControl object
class CScriptControlWrapper
	:	public CScriptControlEventHandler
{
private:
/// Prevents making copies of CScriptControlWrapper objects.
	CScriptControlWrapper( const CScriptControlWrapper& );
	CScriptControlWrapper& operator=( const CScriptControlWrapper& );

public:
/// Constructor
/// @param host - reference to host object
	CScriptControlWrapper();
/// Destructor
	~CScriptControlWrapper();

private:
/// Pointer to ScriptControl object
	IScriptControlPtr							m_scriptControl;
/// Callbacks
	boost::function<void(void)>					m_timeoutCallback;
	boost::function<void(const tstring&)>		m_errorCallback;
	boost::function<void(void)>					m_successCallback;
	boost::function<void(CComVariant)>			m_resultCallback;
	boost::function<void(int,CComVariant)>		m_paramDecodedCallback;
/// OnError is executed flag
	bool										m_errorReported;
/// Script object
	CScriptObjectWrapper						m_script;
protected:
/// ScriptControl events handlers
	virtual HRESULT __stdcall OnScriptControlError();
	virtual HRESULT __stdcall OnScriptControlTimeout();
public:
/// Executes script
/// @param async - async execution mode
/// @param lang - script language
/// @param code - code for execution
/// @param proc - procedure name
/// @param param1 - procedure parameters
/// @param param2 - procedure parameters
/// @param timeout - execution timeout
/// @param objects - vector of object to register in script
	void ExecCode(bool async, BSTR lang, BSTR code, BSTR proc, VARIANT param1, VARIANT param2, bool param1Object, bool param2Object, long timeout, std::vector<SPScriptObject> objects);
/// Sets callbacks for OnSuccess, OnTimeout and for OnError
	void SetCallbacks(boost::function<void(void)> successCallback, 
		boost::function<void(void)> timeoutCallback, 
		boost::function<void(const tstring&)> errorCallback, 
		boost::function<void(CComVariant)> resultCallback,
		boost::function<void(int,CComVariant)> paramDecodedCallback);
/// Returns reference to script object
	CScriptObjectWrapper& GetScriptObject()
	{
		return m_script;
	}
/// Terminates script execution
	void TerminateScript();
};
