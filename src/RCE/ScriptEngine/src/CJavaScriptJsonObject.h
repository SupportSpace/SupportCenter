/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CJavaScriptJsonObject.h
///
///  Declares CJavaScriptJsonObject class, responsible for loading and storing
///    properties of JSON object
///
///  @author Dmitry Netrebenko @date 18.12.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <AidLib/Loki/Singleton.h>

#define JSON_OBJECT_NAME			_T("JSON")
#define JSON_TOSTRING_METHOD		_T("stringify")

/// CJavaScriptJsonObject class, responsible for loading and storing
///   properties of JSON object
/// Access through singleton
class CJavaScriptJsonObject
{
private:
/// Prevents making copies of CJavaScriptJsonObject object
	CJavaScriptJsonObject(const CJavaScriptJsonObject&);
	CJavaScriptJsonObject& operator=(const CJavaScriptJsonObject&);
public:
/// Constructor
	CJavaScriptJsonObject();
/// Destructor
	~CJavaScriptJsonObject();
/// Returns string with code of object
	CComBSTR GetCode() const;
/// Returns string with JSON variable name
	CComBSTR GetVarName() const;
/// Returns string with name of method to convert objects to strings
	CComBSTR GetToStringMethodName() const;
private:
/// Code of JSON object
	CComBSTR	m_code;
/// Variable name of JSON object
	CComBSTR	m_name;
/// Name of method to convert objects to strings
	CComBSTR	m_toStringMethod;
private:
/// Loads code of JavaScript JSON object from resources
/// @return string with code
	CComBSTR LoadFromResources();
};

/// Should be used to CJavaScriptJsonObject as single instance
#define JS_JSONOBJECT_INSTANCE Loki::SingletonHolder<CJavaScriptJsonObject, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
