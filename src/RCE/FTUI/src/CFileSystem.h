//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFileSystem.h
///
///  Declares CFileSystem class
///  Performs file operations: browsing, deleting, renaming
///  
///  @author Alexander Novak @date 19.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFileSystem
{
public:
	CFileSystem(){};
	virtual ~CFileSystem(){};

	/// Creates directory by path
	/// @param path					Path to directory
	void CreateDirectory(const tstring& path);

	/// Checks if directory has any items inside
	/// @param checkDir				Path to checking
	/// @return				Returns true if directory has a file or directory inside, otherwise - false
	bool HasContent(const tstring& checkDir);

	/// Renames file or directory
	/// @param newPath				New item name
	/// @param oldPath				Existing item name
	void RenameItem(const tstring& newPath, const tstring& oldPath);

	/// Deletes file or directory
	/// @param path					Path to item
	void DeleteItem(const tstring& path);

	/// Browses for files and folders
	/// @param path					Path for browsing
	/// @param userData				User defined data
	/// @param lookSubFolders		Browse in sub directories
	/// @remarks			Calls OnFindItem method for all items
	void ScanItems(const tstring& path, const void* userData, bool lookSubFolders = false);

	/// Callback method for browsing
	/// @param path					Current path
	/// @param fileData				Information about item
	/// @param userData				User defined data
	/// @return				Return zero to stop the browsing process
	virtual unsigned int OnFindItem(const tstring& path, const CFileData& fileData, const void* userData);

	/// Callback method for browsing completion
	/// @param path					Requested path for scan operations
	/// @param userData				User defined data
	virtual void OnFindComplete(const tstring& path, const void* userData);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
