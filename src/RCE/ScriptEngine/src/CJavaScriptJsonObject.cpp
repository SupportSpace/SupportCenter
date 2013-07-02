/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CJavaScriptJsonObject.cpp
///
///  Implements CJavaScriptJsonObject class, responsible for loading and storing
///    properties of JSON object
///
///  @author Dmitry Netrebenko @date 18.12.2007
///
////////////////////////////////////////////////////////////////////////

#include "CJavaScriptJsonObject.h"
#include "resource.h"
#include <AidLib/CException/CException.h>
#include <AidLib/Utils/Utils.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>

CJavaScriptJsonObject::CJavaScriptJsonObject()
	:	m_name(JSON_OBJECT_NAME)
	,	m_toStringMethod(JSON_TOSTRING_METHOD)
{
TRY_CATCH
	m_code = LoadFromResources();
CATCH_THROW()
}

CJavaScriptJsonObject::~CJavaScriptJsonObject()
{
TRY_CATCH
CATCH_LOG()
}

CComBSTR CJavaScriptJsonObject::LoadFromResources()
{
TRY_CATCH
	HINSTANCE module = GetCurrentModule();
	/// Find resource
	HRSRC search = FindResource(module, MAKEINTRESOURCE(JSON_SCRIPT_FILE), TEXT("FILE"));
	if(NULL == search) 
		throw MCException_Win(_T("Find resource failed"));
	/// Get resource size
	DWORD dwSize = SizeofResource(module, search);
	if(0 == dwSize) 
		throw MCException_Win(_T("Invalid resource size"));
	/// Load resource
	boost::shared_ptr< boost::remove_pointer<HGLOBAL>::type > resource(LoadResource(module, search), FreeResource);
	if(!resource.get()) 
		throw MCException_Win(_T("Loading resource failed"));
	LPVOID lock = LockResource(resource.get());
	boost::scoped_array<char> buf(new char[dwSize + 1]);
	memset(buf.get(), 0, dwSize + 1);
	memcpy(buf.get(), lock, dwSize);
	UnlockResource(lock);
	tstring res(buf.get());
	USES_CONVERSION;
	CComBSTR bstr(T2OLE(res.c_str()));
	return bstr;
CATCH_THROW()
}

CComBSTR CJavaScriptJsonObject::GetCode() const
{
TRY_CATCH
	return m_code;
CATCH_THROW()
}

CComBSTR CJavaScriptJsonObject::GetVarName() const
{
TRY_CATCH
	return m_name;
CATCH_THROW()
}

CComBSTR CJavaScriptJsonObject::GetToStringMethodName() const
{
TRY_CATCH
	return m_toStringMethod;
CATCH_THROW()
}

