/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFileManager.h
///
///  Implements CFileManager class, responsible for operations with files
///
///  @author Dmitry Netrebenko @date 19.11.2007
///
////////////////////////////////////////////////////////////////////////

#include "CFileManager.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CScopedTracker/CScopedTracker.h>

CFileManager::~CFileManager()
{
TRY_CATCH
CATCH_LOG()
}

boost::shared_array<char> CFileManager::LoadFromFile(const tstring& fileName, unsigned int* size)
{
TRY_CATCH
	if(NULL == size)
		throw MCException(_T("Invalid pointer."));

	HANDLE handle = CreateFile(
		fileName.c_str(), 
		GENERIC_READ, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		0, 
		NULL); 
	if(INVALID_HANDLE_VALUE == handle)
		throw MCException_Win(_T("Cannot open script file"));
	CScopedTracker<HANDLE> file(handle, CloseHandle);

	DWORD fileSize = GetFileSize(file.get(), NULL);
	if(INVALID_FILE_SIZE == fileSize)
		throw MCException_Win(_T("Cannot get file size"));

	boost::shared_array<char> buf(new char[fileSize + 1]);
	memset(buf.get(), 0, fileSize + 1);
	DWORD read = 0;
	BOOL res = ReadFile(
		file.get(),
		buf.get(),
		fileSize,
		&read,
		NULL);
	if((FALSE == res) || (read != fileSize))
		throw MCException_Win(_T("Cannot read data from file"));

	*size = fileSize;
	return buf;
CATCH_THROW()
}

void CFileManager::SaveToFile(const tstring& fileName, boost::shared_array<char> buf, const unsigned int size)
{
TRY_CATCH
	HANDLE handle = CreateFile(
		fileName.c_str(), 
		GENERIC_WRITE, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_ALWAYS, 
		0, 
		NULL); 
	if(INVALID_HANDLE_VALUE == handle)
		throw MCException_Win(_T("Cannot create script file"));
	CScopedTracker<HANDLE> file(handle, CloseHandle);

	DWORD write = 0;
	BOOL res = WriteFile(
		file.get(),
		buf.get(),
		size,
		&write,
		NULL);
	if((FALSE == res) || (write != size))
		throw MCException_Win(_T("Cannot write data into file"));
CATCH_THROW()
}

