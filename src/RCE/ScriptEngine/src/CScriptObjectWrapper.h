/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CScriptObjectWrapper.h
///
///  Declares CScriptObjectWrapper class, wrapper for script object
///
///  @author Dmitry Netrebenko @date 18.12.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <AidLib/Com/CInterfaceMarshaler.h>

#define STRING_TOOBJECT_METHOD		_T("eval")

/// CScriptObjectWrapper class, wrapper for script object
class CScriptObjectWrapper
{
private:
/// Prevents making copies of CScriptObjectWrapper objects.
	CScriptObjectWrapper( const CScriptObjectWrapper& );
	CScriptObjectWrapper& operator=( const CScriptObjectWrapper& );
public:
/// Constructor
	CScriptObjectWrapper();
/// Destructor
	~CScriptObjectWrapper();
/// Sets interface of global script module
	void SetGlobalModule(CComPtr<IDispatch> globalModule);
/// Returns interface of global script module
	CComPtr<IDispatch> GetGlobalModule() const;
private:
/// IDispatch of global script module
	CInterfaceMarshaler<IDispatch>				m_globalModule;
public:
/// Executes method of script object
/// @param object - IDispatch interface of script object
/// @param method - method name
/// @param args - pointer to array of VARIANT parameters
/// @param argc - count of parameters
/// @param result - result of execution
	static void ExecuteMethod(CComPtr<IDispatch> object, CComBSTR method,  VARIANT* args, const unsigned int argc, VARIANT* result);
/// Checks up existance of identificator for object
/// @param object - IDispatch interface of script object
/// @param ident - identificator name
	static bool IdentificatorExists(CComPtr<IDispatch> object, CComBSTR ident);
/// Gets value of property 
/// @param object - IDispatch interface of script object
/// @param propertyName - name of property
/// @param result - result of execution
	static void GetProperty(CComPtr<IDispatch> object, CComBSTR propertyName, VARIANT* result);
/// Sets value of property 
/// @param object - IDispatch interface of script object
/// @param propertyName - name of property
/// @param value - variant with value
/// @param byRef - set value by ref
	static void SetProperty(CComPtr<IDispatch> object, CComBSTR propertyName, CComVariant value, bool byRef = false);
/// Returns IDispatch of script object by name
/// @param object - object name
	CComPtr<IDispatch> GetScriptObject(CComBSTR object);
/// Sets new value of script object
/// @param variableName - variable name of object
/// @param object - new value of variable
	void SetScriptObject(CComBSTR variableName, CComPtr<IDispatch> object);
/// Adds code to script global module
/// @param code - new script code
	void AddCode(CComBSTR code);
/// Adds variable to script global module
/// @param variableName - name of variable
	void AddVariable(CComBSTR variableName);
/// Creates new variable and sets it's value
/// @param variableName - variable name of object
/// @param object - value of variable
	void AddObject(CComBSTR variableName, CComPtr<IDispatch> object);
/// Converts object to string using JSON object
/// @param object - IDispatch of converted object
/// @return string
	CComBSTR ObjectToString(CComPtr<IDispatch> object);
/// Converts object to string using JSON object
/// @param string - converted string
/// @return IDispatch of created object
	CComPtr<IDispatch> StringToObject(CComBSTR string);
/// Converts object to string using JSON object
/// @param object - variant with IDispatch of converted object
/// @return variant with string
	CComVariant ObjectToString(CComVariant object);
/// Converts object to string using JSON object
/// @param string - variant with converted string
/// @return variant with IDispatch of created object
	CComVariant StringToObject(CComVariant string);

};
