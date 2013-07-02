//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFileSystem.cpp
///
///  Implements CFileSystem class
///  Performs file operations: browsing, deleting, renaming
///  
///  @author Alexander Novak @date 19.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CFileData.h"
#include "CFileSystem.h"
#include <shellapi.h>
#include <deque>
#include <AidLib/CException/CException.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/type_traits/remove_pointer.hpp>

// CFileSystem [BEGIN] ///////////////////////////////////////////////////////////////////////////////////

void CFileSystem::CreateDirectory(const tstring& path)
{
TRY_CATCH

	if ( !::CreateDirectory(path.c_str(),NULL) )
		throw MCException_ErrCode_Win(_T("Can't create directory"),GetLastError());

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

bool CFileSystem::HasContent(const tstring& checkDir)
{
TRY_CATCH

	bool result = false;
	CFileData fileData;
	tstring strSearch = checkDir;

	strSearch += ( *strSearch.rbegin()==_T('\\') ) ? _T("*.*") : _T("\\*.*");

	boost::shared_ptr<boost::remove_pointer<HANDLE>::type> hFind(	FindFirstFile(strSearch.c_str(),&fileData),
																	FindClose);
	
	if ( hFind.get()!=INVALID_HANDLE_VALUE )
	{
		do
			if ( !fileData.IsRootDirectory() )
			{
				result = true;
				break;
			}
		while ( FindNextFile(hFind.get(),&fileData) );
	}
	return result;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileSystem::RenameItem(const tstring& newPath, const tstring& oldPath)
{
TRY_CATCH

	if ( !::MoveFile(oldPath.c_str(),newPath.c_str()) )
		throw MCException_Win("Can't rename file or directory");
		
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileSystem::DeleteItem(const tstring& path)
{
TRY_CATCH

	SHFILEOPSTRUCT fos;

	//Alloc memory for a trailing zero + trailing zero for function requirement
	size_t sz_alloc = path.size() + 1 + 1;
	boost::scoped_array<TCHAR> deleteFrom(new TCHAR[sz_alloc]);
	_tcscpy_s(deleteFrom.get(), sz_alloc, path.c_str());
	deleteFrom[path.size()+1] = _T('\0');

	fos.hwnd					= 0;
	fos.wFunc					= FO_DELETE;
	fos.pFrom					= deleteFrom.get();
	fos.pTo						= NULL;
	fos.fFlags					= FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_SILENT;
	fos.fAnyOperationsAborted	= FALSE;
	fos.hNameMappings			= NULL;
	fos.lpszProgressTitle		= NULL;

	if ( int rezCode = SHFileOperation(&fos))
		throw MCException_ErrCode_Win("Can't delete file or directory",rezCode);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileSystem::ScanItems(const tstring& path, const void* userData, bool lookSubFolders)
{
TRY_CATCH

	CFileData fileData;
	std::deque<tstring> lstDirs;
	tstring strSearch = path;

	// Remove the termination slash
	if ( *strSearch.rbegin() == _T('\\') )
		strSearch.erase( strSearch.size()-1 );

	lstDirs.push_back(strSearch);

	while ( !lstDirs.empty() )
	{
		strSearch = lstDirs.front();
		strSearch +=_T("\\*.*");

		boost::shared_ptr<boost::remove_pointer<HANDLE>::type> hFind(	FindFirstFile(strSearch.c_str(),&fileData),
																		FindClose);

		if ( hFind.get()!=INVALID_HANDLE_VALUE )
		{
			unsigned int callbackResult = 1;
			do
			{
				if ( fileData.IsRootDirectory() )
					continue;

				callbackResult = OnFindItem(lstDirs.front(),fileData,userData);

				if ( fileData.IsDirectory() && lookSubFolders )
				{
					tstring strForSearch = lstDirs.front();
					strForSearch += _T("\\");
					strForSearch += fileData.GetFileName();

					lstDirs.push_back(strForSearch);
				}
			}
			while ( callbackResult && FindNextFile(hFind.get(),&fileData) );
		}
		lstDirs.pop_front();
	}// while ( !stDirs.empty() )

	OnFindComplete(path, userData);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

unsigned int CFileSystem::OnFindItem(const tstring& path, const CFileData& fileData, const void* userData)
{
TRY_CATCH

	return 1;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileSystem::OnFindComplete(const tstring& path, const void* userData)
{
TRY_CATCH

CATCH_THROW()
}
// CFileSystem [END] /////////////////////////////////////////////////////////////////////////////////////
