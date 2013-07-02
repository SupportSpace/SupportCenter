/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CScriptObjectWrapper.cpp
///
///  Implements CScriptObjectWrapper class, wrapper for script object
///
///  @author Dmitry Netrebenko @date 18.12.2007
///
////////////////////////////////////////////////////////////////////////

#include "CScriptObjectWrapper.h"
#include <AidLib/CException/CException.h>
#include <AidLib/Strings/tstring.h>
#include "CJavaScriptJsonObject.h"

CScriptObjectWrapper::CScriptObjectWrapper()
{
TRY_CATCH
CATCH_THROW()
}

CScriptObjectWrapper::~CScriptObjectWrapper()
{
TRY_CATCH
CATCH_LOG()
}

void CScriptObjectWrapper::SetGlobalModule(CComPtr<IDispatch> globalModule)
{
TRY_CATCH
	m_globalModule.SetInterface(globalModule);
CATCH_THROW()
}

CComPtr<IDispatch> CScriptObjectWrapper::GetGlobalModule() const
{
TRY_CATCH
	return m_globalModule.GetInterface();
CATCH_THROW()
}

void CScriptObjectWrapper::ExecuteMethod(CComPtr<IDispatch> object, CComBSTR method, VARIANT* args, const unsigned int argc, VARIANT* result)
{
TRY_CATCH
	USES_CONVERSION;
	if(!object)
		throw MCException(_T("Invalid pointer to script object."));
	DISPID idMethod = 0;
	/// Find method
	HRESULT hr = object->GetIDsOfNames(IID_NULL, &method, 1, LOCALE_SYSTEM_DEFAULT, &idMethod);
	if(S_OK != hr)
	{
		tstring methodName = OLE2T(method);
		tstring err = Format(_T("Method '%s' is not found"), methodName.c_str());
		SetLastError(hr);
		throw MCException_Win(err.c_str());
	}
	/// Execute method
	DISPPARAMS params = {args, NULL, argc, 0};
	hr = object->Invoke(idMethod, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &params, result, NULL, NULL);
	if(S_OK != hr)
	{
		SetLastError(hr);
		throw MCException_Win(_T("Error at method execution"));
	}
CATCH_THROW()
}

bool CScriptObjectWrapper::IdentificatorExists(CComPtr<IDispatch> object, CComBSTR ident)
{
TRY_CATCH
	if(!object)
		throw MCException(_T("Invalid pointer to script object."));
	DISPID id = 0;
	/// Find identificator
	HRESULT hr = object->GetIDsOfNames(IID_NULL, &ident, 1, LOCALE_SYSTEM_DEFAULT, &id);
	return (S_OK == hr);
CATCH_THROW()
}

void CScriptObjectWrapper::GetProperty(CComPtr<IDispatch> object, CComBSTR propertyName, VARIANT* result)
{
TRY_CATCH
	USES_CONVERSION;
	if(!object)
		throw MCException(_T("Invalid pointer to script object."));
	tstring propName = OLE2T(propertyName);
	DISPID idProp = 0;
	/// Find property
	HRESULT hr = object->GetIDsOfNames(IID_NULL, &propertyName, 1, LOCALE_SYSTEM_DEFAULT, &idProp);
	if(S_OK != hr)
	{
		tstring err = Format(_T("Property '%s' not found"), propName.c_str());
		SetLastError(hr);
		throw MCException_Win(err.c_str());
	}
	/// Get property value
	DISPPARAMS params = {NULL, NULL, 0, 0};
	hr = object->Invoke(idProp, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &params, result, NULL, NULL);
	if(S_OK != hr)
	{
		tstring err = Format(_T("Error at getting value of '%s' property"), propName.c_str());
		SetLastError(hr);
		throw MCException_Win(err.c_str());
	}
CATCH_THROW()
}

void CScriptObjectWrapper::SetProperty(CComPtr<IDispatch> object, CComBSTR propertyName, CComVariant value, bool byRef)
{
TRY_CATCH
	USES_CONVERSION;
	if(!object)
		throw MCException(_T("Invalid pointer to script object."));
	tstring propName = OLE2T(propertyName);
	DISPID idProp = 0;
	/// Find property
	HRESULT hr = object->GetIDsOfNames(IID_NULL, &propertyName, 1, LOCALE_SYSTEM_DEFAULT, &idProp);
	if(S_OK != hr)
	{
		tstring err = Format(_T("Property '%s' not found"), propName.c_str());
		SetLastError(hr);
		throw MCException_Win(err.c_str());
	}
	/// Set property value
	DISPID type = DISPID_PROPERTYPUT;
	DISPPARAMS params = {&value, &type, 1, 1};
	WORD kind;
	if(byRef)
		kind = DISPATCH_PROPERTYPUTREF;
	else
		kind = DISPATCH_PROPERTYPUT;
	hr = object->Invoke(idProp, IID_NULL, LOCALE_SYSTEM_DEFAULT, kind, &params, NULL, NULL, NULL);
	if(S_OK != hr)
	{
		tstring err = Format(_T("Error at setting value of '%s' property"), propName.c_str());
		SetLastError(hr);
		throw MCException_Win(err.c_str());
	}
CATCH_THROW()
}

CComPtr<IDispatch> CScriptObjectWrapper::GetScriptObject(CComBSTR object)
{
TRY_CATCH
	USES_CONVERSION;
	CComPtr<IDispatch> globalModule = m_globalModule.GetInterface();
	tstring objName = OLE2T(object);
	DISPID idObject = 0;
	/// Find object
	HRESULT hr = globalModule->GetIDsOfNames(IID_NULL, &object, 1, LOCALE_SYSTEM_DEFAULT, &idObject);
	if(S_OK != hr)
	{
		tstring err = Format(_T("Object '%s' is not found"), objName.c_str());
		SetLastError(hr);
		throw MCException_Win(err.c_str());
	}
	/// Get IDispatch of object
	DISPPARAMS params = {NULL, NULL, 0, 0};
	CComVariant result;
	hr = globalModule->Invoke(idObject, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &params, &result, NULL, NULL);
	if((S_OK != hr) || (VT_DISPATCH != result.vt) || !result.pdispVal)
	{
		tstring err = Format(_T("Error at obtaining interface of '%s' object"), objName.c_str());
		SetLastError(hr);
		throw MCException_Win(err.c_str());
	}
	return CComPtr<IDispatch>(result.pdispVal);
CATCH_THROW()
}

void CScriptObjectWrapper::SetScriptObject(CComBSTR variableName, CComPtr<IDispatch> object)
{
TRY_CATCH
	USES_CONVERSION;
	CComPtr<IDispatch> globalModule = m_globalModule.GetInterface();
	DISPID idVariable = 0;
	/// Find variable
	HRESULT hr = globalModule->GetIDsOfNames(IID_NULL, &variableName, 1, LOCALE_SYSTEM_DEFAULT, &idVariable);
	if(S_OK != hr)
	{
		tstring varName = OLE2T(variableName);
		tstring err = Format(_T("Variable '%s' is not found"), varName.c_str());
		SetLastError(hr);
		throw MCException_Win(err.c_str());
	}
	/// Set object value
	DISPID type = DISPID_PROPERTYPUT;
	CComVariant arg(object);
	DISPPARAMS params = {&arg, &type, 1, 1};
	hr = globalModule->Invoke(idVariable, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYPUTREF, &params, NULL, NULL, NULL);
	if(S_OK != hr)
	{
		SetLastError(hr);
		throw MCException_Win(_T("Error at setting object value"));
	}
CATCH_THROW()
}

void CScriptObjectWrapper::AddCode(CComBSTR code)
{
TRY_CATCH
	/// Execute "eval(code)"
	CComPtr<IDispatch> globalModule = m_globalModule.GetInterface();
	CComBSTR methodName(STRING_TOOBJECT_METHOD);
	CComVariant arg(code);
	ExecuteMethod(globalModule, methodName, &arg, 1, NULL);
CATCH_THROW()
}

void CScriptObjectWrapper::AddVariable(CComBSTR variableName)
{
TRY_CATCH
	CComBSTR code(L"var ");
	code += variableName;
	code += CComBSTR(L" = null;");
	AddCode(code);
CATCH_THROW()
}

void CScriptObjectWrapper::AddObject(CComBSTR variableName, CComPtr<IDispatch> object)
{
TRY_CATCH
	AddVariable(variableName);
	SetScriptObject(variableName, object);
CATCH_THROW()
}

CComBSTR CScriptObjectWrapper::ObjectToString(CComPtr<IDispatch> object)
{
TRY_CATCH
	CComVariant obj(object);
	CComVariant result = ObjectToString(obj);
	CComBSTR str(result.bstrVal);
	return str;
CATCH_THROW()
}

CComPtr<IDispatch> CScriptObjectWrapper::StringToObject(CComBSTR string)
{
TRY_CATCH
	CComVariant str(string);
	CComVariant result = StringToObject(str);
	CComPtr<IDispatch> obj(result.pdispVal);
	return obj;
CATCH_THROW()
}

CComVariant CScriptObjectWrapper::ObjectToString(CComVariant object)
{
TRY_CATCH
	if(VT_DISPATCH != object.vt)
		throw MCException(_T("Invalid object."));
	CComVariant result;
	/// Get name of JSON variable
	CComBSTR jsonName = JS_JSONOBJECT_INSTANCE.GetVarName();
	/// Get name of method to convert object
	CComBSTR methodName = JS_JSONOBJECT_INSTANCE.GetToStringMethodName();
	/// Get IDispatch of JSON object
	CComPtr<IDispatch> jsonObject = GetScriptObject(jsonName);
	/// Prepare parameters with object
	CComVariant arg(object);
	/// Convert object to string
	ExecuteMethod(jsonObject, methodName, &arg, 1, &result);
	if(VT_BSTR != result.vt)
		throw MCException(_T("Object is not converted to string."));
	return result;
CATCH_THROW()
}

CComVariant CScriptObjectWrapper::StringToObject(CComVariant string)
{
TRY_CATCH
	if(VT_BSTR != string.vt)
		throw MCException(_T("Invalid string."));
	CComPtr<IDispatch> globalModule = m_globalModule.GetInterface();
	CComVariant result;
	CComBSTR argStr(L"(");
	argStr += CComBSTR(string.bstrVal);
	argStr += CComBSTR(L")");
	CComBSTR methodName(STRING_TOOBJECT_METHOD);
	/// Prepare parameters with string
	CComVariant arg(argStr);
	/// Execute "eval(code)"
	ExecuteMethod(globalModule, methodName, &arg, 1, &result);
	if(VT_DISPATCH != result.vt)
		throw MCException(_T("String is not converted to object."));
	return result;
CATCH_THROW()
}
