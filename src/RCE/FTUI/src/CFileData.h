//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFileData.h
///
///  Declares CFileData class
///  Wrapper for the WIN32_FIND_DATA structure
///  
///  @author Alexander Novak @date 19.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <tchar.h>
#include <windows.h>
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFileData
	:	public WIN32_FIND_DATA
{
public:
	bool m_drive;

	/// Default constructor for the CFileData
	CFileData()
		:	m_drive(false){};

	/// Constructs CFileData from the WIN32_FIND_DATA 
	/// @param w32fd				Pointer to the WIN32_FIND_DATA structure
	CFileData(const WIN32_FIND_DATA* w32fd)
		:	m_drive(false),
			WIN32_FIND_DATA(*w32fd){};

	/// Constructs CFileData from the WIN32_FIND_DATA 
	/// @param w32fd				Reference to the WIN32_FIND_DATA structure
	CFileData(const WIN32_FIND_DATA& w32fd)
		:	m_drive(false),
			WIN32_FIND_DATA(w32fd){};

	/// Constructs CFileData from the CFileData object 
	/// @param w32fd				Reference to the CFileData object
	CFileData(const CFileData& fd)
		:	m_drive(fd.m_drive),
			WIN32_FIND_DATA(fd){};

	/// Checks if it's root directory, named as "." or ".."
	/// @return				Returns true if it's root directory, otherwise - false
	bool IsRootDirectory() const;

	/// Checks if it's directory
	/// @return				Returns true if it's directory, otherwise - false
	bool IsDirectory() const;

	/// Checks if it's drive
	/// @return				Returns true if it's drive, otherwise - false
	bool IsDrive() const;

	/// Returns file attributes
	/// @return				File attributes
	DWORD GetAttributes() const;

	/// Returns file creation time in UTC
	/// @return				File creation time
	FILETIME GetCreationTime() const;

	/// Returns file last access time in UTC
	/// @return				File access time
	FILETIME GetLastAccessTime() const;

	/// Returns file last write time in UTC
	/// @return				File last write time
	FILETIME GetLastWriteTime() const;

	/// Returns file name
	/// @return				File name
	const TCHAR* GetFileName() const;

	/// Returns file size
	/// @return				File size in bytes
	ULARGE_INTEGER GetFileSize() const;
};
//--------------------------------------------------------------------------------------------------------

inline bool CFileData::IsDirectory() const
{
	return dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && !IsDrive();
}
//--------------------------------------------------------------------------------------------------------

inline bool CFileData::IsDrive() const
{
	return m_drive;
}
//--------------------------------------------------------------------------------------------------------

inline DWORD CFileData::GetAttributes() const
{
	return dwFileAttributes;
}
//--------------------------------------------------------------------------------------------------------

inline FILETIME CFileData::GetCreationTime() const
{
	return ftCreationTime;
}
//--------------------------------------------------------------------------------------------------------

inline FILETIME CFileData::GetLastAccessTime() const
{
	return ftLastAccessTime;
}
//--------------------------------------------------------------------------------------------------------

inline FILETIME CFileData::GetLastWriteTime() const
{
	return ftLastWriteTime;
}
//--------------------------------------------------------------------------------------------------------

inline const TCHAR* CFileData::GetFileName() const
{
	return cFileName;
}
//--------------------------------------------------------------------------------------------------------

inline ULARGE_INTEGER CFileData::GetFileSize() const
{
	ULARGE_INTEGER uli;
	uli.HighPart	= nFileSizeHigh;
	uli.LowPart		= nFileSizeLow;
	return uli;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
