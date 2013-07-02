/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCleaner.cpp
///
///  Implements CCleaner class, responsible for removing temporary files
///
///  @author Dmitry Netrebenko @date 19.12.2007
///
////////////////////////////////////////////////////////////////////////

#include "CCleaner.h"
#include <AidLib/CException/CException.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/shared_ptr.hpp>
#include <AidLib/Utils/Utils.h>

CCleaner::CCleaner()
	:	m_currentDir(_T(""))
{
TRY_CATCH
	TCHAR fileName[MAX_PATH];
	memset(fileName, 0, sizeof(fileName));
	/// Get current application file name
	GetModuleFileName(
			GetCurrentModule(),
			fileName,
			MAX_PATH
		);
	tstring name(fileName);
	/// Get current module dir
	tstring::size_type pos = name.rfind(_T("\\"));
	if(tstring::npos != pos)
		m_currentDir = name.substr(0, pos);
CATCH_THROW()
}

CCleaner::~CCleaner()
{
TRY_CATCH
	Clear();
CATCH_LOG()
}

void CCleaner::AddDirectory(const tstring& dir)
{
TRY_CATCH
	m_dirs.push_back(dir);
CATCH_THROW()
}

void CCleaner::AddFile(const tstring& file)
{
TRY_CATCH
	m_files.push_back(file);
CATCH_THROW()
}

void CCleaner::Clear()
{
TRY_CATCH
	/// Set current directory as directory of current module
	SetCurrentDirectory(m_currentDir.c_str());
	std::list<tstring>::iterator index;
	/// Remove files
	for(index = m_files.begin(); index != m_files.end(); ++index)
	{
		tstring fileName = *index;
		DeleteFile(fileName.c_str());
	}
	m_files.clear();
	/// Remove directories
	for(index = m_dirs.begin(); index != m_dirs.end(); ++index)
	{
		tstring dir = *index;
		tstring::size_type pos = dir.rfind(_T("\\"));
		if(tstring::npos != pos)
		{
			if(pos != dir.size() - 1)
				dir += tstring(_T("\\"));
		}
		else
			dir += tstring(_T("\\"));
		RemoveDir(dir);
	}
	m_dirs.clear();
CATCH_THROW()
}

void CCleaner::RemoveDir(const tstring& dir)
{
TRY_CATCH
	tstring mask = dir + tstring(CLEANER_FILES_MASK);
	/// Find all files in temp directory and delete
	WIN32_FIND_DATA findData;
	HANDLE search = FindFirstFile(mask.c_str(), &findData);
	if(INVALID_HANDLE_VALUE == search)
		throw MCException_Win(_T("Find first script file failed. "));
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > searchHandle(search, FindClose);
	tstring name = tstring(findData.cFileName);
	if((name != tstring(_T("."))) && (name != tstring(_T(".."))))
	{
		if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			tstring dirName = dir + tstring(_T("\\")) + name + tstring(_T("\\"));
			RemoveDir(dirName);
		}
		else
		{
			tstring fileName = dir + tstring(_T("\\")) + name;
			DeleteFile(fileName.c_str());
		}
	}
	while(TRUE == FindNextFile(searchHandle.get(), &findData))
	{
		tstring name = tstring(findData.cFileName);
		if((name != tstring(_T("."))) && (name != tstring(_T(".."))))
		{
			if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				tstring dirName = dir + tstring(_T("\\")) + name + tstring(_T("\\"));
				RemoveDir(dirName);
			}
			else
			{
				tstring fileName = dir + tstring(_T("\\")) + name;
				DeleteFile(fileName.c_str());
			}
		}
	}
	searchHandle.reset();
	if(FALSE == RemoveDirectory(dir.c_str()))
		throw MCException_Win(_T("RemoveDirectory failed"));
CATCH_LOG()
}

