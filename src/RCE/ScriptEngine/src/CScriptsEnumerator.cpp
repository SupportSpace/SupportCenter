/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CScriptsEnumerator.cpp
///
///  Implements CScriptsEnumerator class, responsible for ScriptsEnumerator ActiveX
///
///  @author Dmitry Netrebenko @date 11.12.2007
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CScriptsEnumerator.h"
#include <AidLib/CException/CException.h>
#include <AidLib/Utils/Utils.h>
#include <AidLib/Com/ComException.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <list>

CScriptsEnumerator::CScriptsEnumerator()
	:	m_dirs(false)
{
TRY_CATCH
CATCH_THROW()
}

CScriptsEnumerator::~CScriptsEnumerator()
{
TRY_CATCH
CATCH_LOG()
}

STDMETHODIMP CScriptsEnumerator::GetScriptList(BSTR** names, ULONG* count)
{
TRY_CATCH_COM
	USES_CONVERSION;
	/// Find all scripts
	std::list<tstring> files;
	tstring mask = m_dirs.GetScriptDirectory();
	if(m_dirs.IsArchiveMode())
		mask += tstring(SCRIPT_FILES_MASK);
	else
		mask += tstring(SCRIPT_ALL_FILES_MASK);
	WIN32_FIND_DATA findData;
	HANDLE search = FindFirstFile(mask.c_str(), &findData);
	if(INVALID_HANDLE_VALUE == search)
		throw MCException_Win(_T("Find first script failed"));
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > searchHandle(search, FindClose);
	if(m_dirs.IsArchiveMode())
	{
		if(0 == (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			tstring name = GetArchiveName(&findData);
			if(name != _T(""))
				files.push_back(name);
		}
		while(TRUE == FindNextFile(searchHandle.get(), &findData))
		{
			if(0 == (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				tstring name = GetArchiveName(&findData);
				if(name != _T(""))
					files.push_back(name);
			}
		}
	}
	else
	{
		if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			tstring name(findData.cFileName);
			if((_T(".") != name) && (_T("..") != name))
				files.push_back(name);
		}
		while(TRUE == FindNextFile(searchHandle.get(), &findData))
		{
			if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				tstring name(findData.cFileName);
				if((_T(".") != name) && (_T("..") != name))
					files.push_back(name);
			}
		}
	}
	if(!files.size())
	{
		*names = NULL;
		*count = 0;
		return S_OK;
	}
	/// Create array of names
	BSTR* scripts = new BSTR[files.size()];
	*names = scripts;
	*count = static_cast<ULONG>(files.size());
	for(ULONG i = 0; i < files.size(); ++i)
	{
		tstring name(files.front());
		files.pop_front();
		scripts[i] = ::SysAllocString(T2OLE(name.c_str()));
	}
CATCH_LOG_COM
}

tstring CScriptsEnumerator::GetArchiveName(const WIN32_FIND_DATA* fileData)
{
TRY_CATCH
	if(!fileData)
		return _T("");
	tstring fileName(fileData->cFileName);
	tstring::size_type pos = fileName.rfind(_T("."));
	if(tstring::npos == pos)
		return _T("");
	tstring name = fileName.substr(0, pos);
	fileName.erase(0, pos + 1);
	if(LowerCase(fileName) != tstring(SCRIPT_FILES_EXT))
		return _T("");
	return name;
CATCH_THROW()
}

