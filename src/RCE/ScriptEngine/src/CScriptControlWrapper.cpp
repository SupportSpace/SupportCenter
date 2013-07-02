/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CScriptControlWrapper.cpp
///
///  Implements CScriptControlWrapper class, wrapper for ScriptControl object
///
///  @author Dmitry Netrebenko @date 19.11.2007
///
////////////////////////////////////////////////////////////////////////

#include "CScriptControlWrapper.h"
#include <AidLib/CException/CException.h>
#include <AidLib/Strings/tstring.h>
#include <boost/shared_ptr.hpp>
#include <atlsafe.h>
#include <AidLib/Com/ComException.h>
#include <AidLib/CScopedTracker/CScopedTracker.h>
#include <boost/bind.hpp>
#include "CJavaScriptJsonObject.h"

CScriptControlWrapper::CScriptControlWrapper()
	:	m_errorReported(false)
{
TRY_CATCH
	/// Create MS ScriptControl
	HRESULT result = S_OK;
	result = m_scriptControl.CreateInstance(__uuidof(ScriptControl));
	if(S_OK != result)
	{
		SetLastError(result);
		throw MCException_Win(_T("Cannot create ScriptControl object"));
	}
	/// Attach to events of ScriptControl
	result = DispEventAdvise(m_scriptControl);
	if(S_OK != result)
	{
		SetLastError(result);
		throw MCException_Win(_T("Cannot advise to ScriptControl events"));
	}
CATCH_THROW()
}

CScriptControlWrapper::~CScriptControlWrapper()
{
TRY_CATCH
	TerminateScript();
CATCH_LOG()
}

HRESULT __stdcall CScriptControlWrapper::OnScriptControlError()
{
TRY_CATCH
	/// Get interface of error and extract description
	CComPtr<IScriptError> error; 
	HRESULT res = m_scriptControl->get_Error(&error);
	if(!error || (S_OK != res))
	{
		SetLastError(res);
		throw MCException_Win(_T("Failed to get ScriptControl error interface"));
	}
	long number;
	res = error->get_Number(&number);
	if(S_OK != res)
	{
		SetLastError(res);
		throw MCException_Win(_T("Failed to get error number"));
	}
	CComBSTR source;
	res = error->get_Source(&source);
	if(S_OK != res)
	{
		SetLastError(res);
		throw MCException_Win(_T("Failed to get error source"));
	}
	CComBSTR desc;
	res = error->get_Description(&desc);
	if(S_OK != res)
	{
		SetLastError(res);
		throw MCException_Win(_T("Failed to get error description"));
	}
	CComBSTR text;
	res = error->get_Text(&text);
	if(S_OK != res)
	{
		SetLastError(res);
		throw MCException_Win(_T("Failed to get error text"));
	}
	long line;
	res = error->get_Line(&line);
	if(S_OK != res)
	{
		SetLastError(res);
		throw MCException_Win(_T("Failed to get error line"));
	}
	long column;
	res = error->get_Column(&column);
	if(S_OK != res)
	{
		SetLastError(res);
		throw MCException_Win(_T("Failed to get error column"));
	}

	/// Prepare error message
	USES_CONVERSION;
	tstring errorStr(_T(""));
	if((NULL == text.m_str) || (0 == text.Length()))
		errorStr = Format(_T("%s (%X): %s at (line: %d, column: %d)"), OLE2T(source), number, OLE2T(desc), line, column);
	else
		errorStr = Format(_T("%s (%X): %s at (line: %d, column: %d, text: \"%s\")"), OLE2T(source), number, OLE2T(desc), line, column, OLE2T(text));
	if(m_errorCallback && !m_errorReported)
		m_errorCallback(errorStr);
	m_errorReported = true;
CATCH_LOG()
	return S_OK;
}

HRESULT __stdcall CScriptControlWrapper::OnScriptControlTimeout()
{
TRY_CATCH
	if(m_timeoutCallback && !m_errorReported)
		m_timeoutCallback();
	m_errorReported = true;
CATCH_LOG()
	return S_OK;
}

void CScriptControlWrapper::ExecCode(bool async, BSTR lang, BSTR code, BSTR proc, VARIANT param1, VARIANT param2, bool param1Object, bool param2Object, long timeout, std::vector<SPScriptObject> objects)
{
TRY_CATCH
	CComVariant res;
	m_errorReported = false;
	if(!m_scriptControl)
		throw MCException(_T("ScriptControl is not created."));

	USES_CONVERSION;
	/// Reset engine
	m_scriptControl->raw_Reset();
	tstring tmpLang(OLE2T(lang));
	bool isJavaScript = ((tstring(_T("javascript")) == LowerCase(tmpLang)) || (tstring(_T("jscript")) == LowerCase(tmpLang)));
	/// Set ScriptControl's properties
	HRESULT result = m_scriptControl->put_Language(lang);
	if(S_OK != result)
	{
		SetLastError(result);
		throw MCException_Win(_T("Failed to set language"));
	}
	result = m_scriptControl->put_Timeout(timeout);
	if(S_OK != result)
	{
		SetLastError(result);
		throw MCException_Win(_T("Failed to set timeout"));
	}
	result = m_scriptControl->put_AllowUI(FALSE);
	if(S_OK != result)
	{
		SetLastError(result);
		throw MCException_Win(_T("Failed to set UI"));
	}
	/// Add objects to ScriptControl
	for(std::vector<SPScriptObject>::iterator index = objects.begin(); index != objects.end(); ++index)
	{
		SPScriptObject object = *index;
		result = m_scriptControl->raw_AddObject(object->m_name, object->m_object, object->m_addMemmber);
		if(S_OK != result)
		{
			SetLastError(result);
			throw MCException_Win(_T("Failed to add object to ScriptControl"));
		}
	}
	/// Get interface of global script module
	CComPtr<IDispatch> script;
	result = m_scriptControl->get_CodeObject(&script);
	if((S_OK != result) || !script)
	{
		SetLastError(result);
		throw MCException_Win(_T("Obtaining code object failed"));
	}
	m_script.SetGlobalModule(script);

	/// Convert parameters to objects
	CComVariant tmpParam1(param1);
	CComVariant tmpParam2(param2);
	if(isJavaScript)
	{
		/// Add JSON object
		result = m_scriptControl->raw_AddCode(JS_JSONOBJECT_INSTANCE.GetCode());
		if(S_OK != result)
		{
			SetLastError(result);
			throw MCException_Win(_T("Failed to add code to ScriptControl"));
		}
		if((VT_BSTR == tmpParam1.vt) && param1Object)
		{
			tmpParam1 = m_script.StringToObject(tmpParam1);
			if(m_paramDecodedCallback)
				m_paramDecodedCallback(0, tmpParam1);
		}
		if((VT_BSTR == tmpParam2.vt) && param2Object)
		{
			tmpParam2 = m_script.StringToObject(tmpParam2);
			if(m_paramDecodedCallback)
				m_paramDecodedCallback(1, tmpParam2);
		}
	}

	/// Add code
	result = m_scriptControl->raw_AddCode(code);
	if(S_OK != result)
	{
		SetLastError(result);
		if(m_errorReported)
		{
			Log.WinError(_ERROR_, _T("Failed to add code to ScriptControl. "));
			return;
		}
		else
		{
			m_errorReported = true;
			throw MCException_Win(_T("Failed to add code to ScriptControl"));
		}
	}
	/// Exit if procedure name is not specified
	if(CComBSTR(proc) == CComBSTR(_T("")))
	{
		if(m_successCallback)
			m_successCallback();
		return;
	}

	/// Create array with parameters
	CComSafeArray<VARIANT> safeArray;
	SAFEARRAYBOUND rgsabound[1];
	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = 2;
	safeArray.Create(rgsabound);
	safeArray[0] = tmpParam1;
	safeArray[1] = tmpParam2;
	SAFEARRAY* parray = *safeArray.GetSafeArrayPtr();

	/// Run script
	result = m_scriptControl->raw_Run(proc, &parray, &res);
	if(S_OK != result)
	{
		SetLastError(result);
		if(m_errorReported)
			Log.WinError(_ERROR_, _T("Failed to execute script. "));
		else
		{
			m_errorReported = true;
			throw MCException_Win(_T("Failed to execute script"));
		}
	}
	/// Call "OnSuccess" callback
	if(S_OK == result)
	{
		if(m_resultCallback)
			m_resultCallback(res);
		if(m_successCallback)
			m_successCallback();
	}
CATCH_THROW()
}

void CScriptControlWrapper::SetCallbacks(boost::function<void(void)> successCallback, 
										 boost::function<void(void)> timeoutCallback, 
										 boost::function<void(const tstring&)> errorCallback, 
										 boost::function<void(CComVariant)> resultCallback,
										 boost::function<void(int,CComVariant)> paramDecodedCallback)
{
TRY_CATCH
	m_successCallback = successCallback;
	m_timeoutCallback = timeoutCallback;
	m_errorCallback = errorCallback;
	m_resultCallback = resultCallback;
	m_paramDecodedCallback = paramDecodedCallback;
CATCH_THROW()
}

void CScriptControlWrapper::TerminateScript()
{
TRY_CATCH
	if(m_scriptControl)
	{
		/// Detach from events
		DispEventUnadvise(m_scriptControl);
		m_scriptControl = NULL;
	}
CATCH_THROW()
}

