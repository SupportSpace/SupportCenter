/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCleaner.h
///
///  Declares CCleaner class, responsible for removing temporary files
///
///  @author Dmitry Netrebenko @date 19.12.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <AidLib/Strings/tstring.h>
#include <list>

#define CLEANER_FILES_MASK			_T("*.*")

/// CCleaner class, responsible for removing temporary files
class CCleaner
{
private:
/// Prevents making copies of CCleaner objects.
	CCleaner( const CCleaner& );
	CCleaner& operator=( const CCleaner& );
public:
/// Constructor
	CCleaner();
/// Destructor
	~CCleaner();
/// Adds directory to list
/// @param dir - directory path
	void AddDirectory(const tstring& dir);
/// Adds file to list
/// @param file - file name with full path
	void AddFile(const tstring& file);
private:
/// List of temp directories
	std::list<tstring>	m_dirs;
/// List of temp files
	std::list<tstring>	m_files;
/// Directory of current module
	tstring				m_currentDir;
private:
/// Remove all temporary files and directories
	void Clear();
/// Remove directory
/// @param dir - directory path
	void RemoveDir(const tstring& dir);
};
