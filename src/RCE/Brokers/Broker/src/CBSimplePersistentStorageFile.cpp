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

#include "CBSimplePersistentStorageFile.h"
#include <atlfile.h>

CBSimplePersistentStorageFile::CBSimplePersistentStorageFile(const tstring& path):
	m_path(path)
{
}

CBSimplePersistentStorageFile::~CBSimplePersistentStorageFile(void)
{
}

tstring CBSimplePersistentStorageFile::Load(const tstring& name)
{
TRY_CATCH
	HRESULT result;
	CAtlFile file;
	tstring fullName=m_path+name;
	if(S_OK!=(result=file.Create(fullName.c_str(),GENERIC_READ,FILE_SHARE_DELETE,OPEN_EXISTING)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("File with name=[%s] openning failed"),fullName.c_str()),result);
	ULONGLONG valueLen;
	if(S_OK!=file.GetSize(valueLen))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("File with name=[%s] size getting failed"),fullName.c_str()),result);
	std::auto_ptr<char> value(new char[valueLen]);
	if(S_OK!=file.Read(value.get(),valueLen))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("File with name=[%s] reading failed"),fullName.c_str()),result);
	if(::DeleteFile(fullName.c_str())==0)
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("File with name=[%s] deleting failed"),fullName.c_str()),result);

	return reinterpret_cast<TCHAR*>(value.get());
CATCH_THROW()
}

void CBSimplePersistentStorageFile::Save(const tstring& name, const tstring& value)
{
TRY_CATCH
	HRESULT result;
	CAtlFile file;
	tstring fullName=m_path+name;
	if(S_OK!=(result=file.Create(fullName.c_str(),GENERIC_WRITE,0,CREATE_ALWAYS)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("File with name=[%s] openning failed"),fullName.c_str()),result);
	DWORD valueLen=(value.length()+1)*sizeof(tstring::value_type);
	if(S_OK!=(result=file.Write(value.c_str(),valueLen)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("File with name=[%s] writing failed"),fullName.c_str()),result);
	if(S_OK!=(result=file.SetSize(valueLen)))
		throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),Format(_T("File with name=[%s] resize failed"),fullName.c_str()),result);
CATCH_THROW()
}