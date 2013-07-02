/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CBSimplePersistentStorage.cpp
///
///  CBSimplePersistentStorage simple persistent implementation
///
///  @author Kirill Solovyov @date 10.05.2008
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CBSimplePersistentStorage.h"
#include "CBPropertiesStorage.h"

CBSimplePersistentStorage::CBSimplePersistentStorage(void)
{
TRY_CATCH
CATCH_LOG()
}

CBSimplePersistentStorage::~CBSimplePersistentStorage(void)
{
TRY_CATCH
CATCH_LOG()
}
tstring CBSimplePersistentStorage::Load(const tstring& name)
{
TRY_CATCH
	CBPropertiesStorage storage;
	HRESULT result;
	_tcscpy_s(storage.m_qNAME,name.c_str());
	if(S_OK!=(result=storage.OpenAll()))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Properties table (props.dbf) openning failed")),result);
	if(DB_S_ENDOFROWSET==(result=storage.MoveFirst()))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T("Properties with name=[%s] not found"),name.c_str());
	else if(S_OK!=result)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Properties with name=[%s] search failed"),name.c_str()),result);
	return storage.m_VALUE;
CATCH_THROW()
}

void CBSimplePersistentStorage::Save(const tstring& name, const tstring& value)
{
TRY_CATCH
	CBPropertiesStorage storage;
	HRESULT result;
	_tcscpy_s(storage.m_qNAME,name.c_str());
	result=storage.OpenAll();
	if(DB_E_NOTABLE==result)
	{
		CCommand<> creation;
		if(S_OK!=(result=creation.Open(storage.m_session,L"CREATE TABLE props.dbf (\"NAME\" VARCHAR(254),\"VALUE\" MEMO)")))
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Properties table (props.dbf) creation failed")),result);
		creation.Close();
		storage.CloseAll();
		if(S_OK!=(result=storage.OpenAll()))
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Properties table (props.dbf) openning failed after creation")),result);
	}
	else if(S_OK!=result)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Properties table (props.dbf) openning failed")),result);
	result=storage.MoveFirst();
	_tcscpy_s(storage.m_NAME,name.c_str());
	storage.m_dwNAMELength=name.length();
	storage.m_dwNAMEStatus=DBSTATUS_S_OK;
	_tcscpy_s(storage.m_VALUE,value.c_str());
	storage.m_dwVALUELength=value.length();
	storage.m_dwVALUEStatus=DBSTATUS_S_OK;
	if(DB_S_ENDOFROWSET==result)
	{
		if(S_OK!=(result=storage.Insert()))
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Property with name=[%s] value=[%s] Insert() failed"),name.c_str(),value.c_str()),result);
	}
	else if(S_OK==result)
	{
		if(S_OK!=(result=storage.SetData()))
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Property with name=[%s] value=[%s] SetData() failed"),name.c_str(),value.c_str()),result);
		if(S_OK!=(result=storage.Update()))
			throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Property with name=[%s] value=[%s] Update() failed"),name.c_str(),value.c_str()),result);
	}
	else
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Property with name=[%s] search failed"),name.c_str()),result);
CATCH_THROW()
}
