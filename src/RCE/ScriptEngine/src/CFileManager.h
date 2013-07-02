/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFileManager.h
///
///  Declares CFileManager class, responsible for operations with files
///
///  @author Dmitry Netrebenko @date 19.11.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <AidLib/Strings/tstring.h>
#include <boost/shared_array.hpp>

///  CFileManager class, responsible for operations with files
class CFileManager
{
private:
/// Prevents construction of CFileManager objects
	CFileManager();

public:
/// Destructor
	~CFileManager();

/// Loads file
/// @param fileName - name of file to be loaded
/// @param size - [out] pointer to size
/// @return buffer with data
	static boost::shared_array<char> LoadFromFile(const tstring& fileName, unsigned int* size);
/// Saves buffer into file
/// @param fileName - name of file
/// @param buf - buffer with data
/// @param size - size of buffer
	static void SaveToFile(const tstring& fileName, boost::shared_array<char> buf, const unsigned int size);
};
