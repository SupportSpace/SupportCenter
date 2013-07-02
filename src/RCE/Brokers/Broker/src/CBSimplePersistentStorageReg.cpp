/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CBSimplePersistentStorageReg.cpp
///
///  CBSimplePersistentStorageReg simple persistent via registry class implementation
///
///  @author Kirill Solovyov @date 10.05.2008
///
////////////////////////////////////////////////////////////////////////

#include "CBSimplePersistentStorageReg.h"

CBSimplePersistentStorageReg::CBSimplePersistentStorageReg(HKEY keyParent, const tstring& keyName)
{
TRY_CATCH
	HRESULT result;
	if(ERROR_SUCCESS!=(result=m_key.Create(keyParent,keyName.c_str())))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Create key=[0x%x]/[%s] failed"),keyParent,keyName.c_str()),result);
	//tstring version;
CATCH_LOG()
}

CBSimplePersistentStorageReg::~CBSimplePersistentStorageReg(void)
{
TRY_CATCH
CATCH_LOG()
}

tstring CBSimplePersistentStorageReg::Load(const tstring& name)
{
TRY_CATCH
	HRESULT result;
	ULONG valueLen=0;
	if(ERROR_SUCCESS!=(result=m_key.QueryStringValue(name.c_str(),NULL,&valueLen)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Read property with name=[%s] failed (length)"),name.c_str()),result);
	std::auto_ptr<TCHAR> value(new TCHAR[valueLen]);
	if(ERROR_SUCCESS!=(result=m_key.QueryStringValue(name.c_str(),value.get(),&valueLen)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Read property with name=[%s] failed"),name.c_str()),result);
	return value.get();
CATCH_THROW()
}

void CBSimplePersistentStorageReg::Save(const tstring& name, const tstring& value)
{
TRY_CATCH
	HRESULT result;
	if(ERROR_SUCCESS!=(result=m_key.SetStringValue(name.c_str(),value.c_str())))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("Save property with name=[%s] and value=[%s] failed (length)"),name.c_str(),value.c_str()),result);
CATCH_THROW()
}