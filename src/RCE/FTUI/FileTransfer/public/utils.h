//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  utils.h
///
///  Declares classes and methods for using in the FileTransfer library
///  
///  @modified Alexander Novak @date 23.01.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once 

#include <windows.h>
#include <vector>
#include <AidLib/Strings/tstring.h>
#include <boost/shared_array.hpp>
#include "filetransferdata.h"
#include <locale>
#include <map>

namespace transfer_utils
{

	typedef boost::shared_array<CHAR> spBuffer;
	typedef std::pair<size_t,spBuffer> spPair;
	/// chacks if file folder or not
	/// @param  fn file name
	/// @return TRUE if file is a fofder otherwise FALSE
	BOOL IsFolder(const tstring& fn);
	/// Make zip archove of the folder
	/// @param  fn folder name	
	/// @param  nfn new name of the archive file
	/// @return TRUE if OK otherwise FALSE
	BOOL ZipFolder( const tstring& fn , tstring& nfn );
	/// Extracts file from archive and then deletes  it
	/// @param  fn name of the archive file
	/// @return TRUE if OK otherwise FALSE	
	/// @remark archive file always locates in folder where target folder ought to be copied
	BOOL UnzipAndDeleteFile(const tstring& root, const tstring& fn);
	/// Gets name of the devices in host machine and fills string which looks like  "C:l:total_size:free_size<NULL>D:c:total_size:free_size<NULL>....Z:n\<NULL><NULL>"
	spPair get_drives();
	/// Fills the vector of drives 
	/// @param dl [out] vector which connsists information about drives
	/// @param len   length of the string
	/// @param drives  string which consists drives info @see get_drives
	void TokenizeDrives(TDriveInfo& dl, unsigned int len, char* drives);
	/// helpful function @see TokenizeDrives
	void TokenizeInfo(TDriveInfo::value_type& di, char* info);
	/// Helpful class for Template Pattern
	class CHookOperation
	{
	public:
		virtual void DoOperation( const WIN32_FIND_DATA& data ) = 0;
	};
	/// Template method	
	void TemplateMethod( const TCHAR* folder ,  CHookOperation& operation );
	/// no case compare function
	int compare_no_case(const tstring& l, const tstring& r, const std::locale& lc);
	/// Retrieve folder name
    tstring get_folder_name( const tstring&);
	/// Retrieve file name	
	tstring get_file_name( const tstring&);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Determinates drive type by it name
class CDriveType
{
	std::map<tstring,DWORD> m_mapDriveTypes;
public:
	/// Returns drive type
	/// @param	driveName			Name of the drive
	/// @return				Drive type
	EDriveTypes operator()(const TCHAR* driveName);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Determinates icon index by drive type and add it into the system image list
class CDriveIconIndex
{
	struct SIndexTable
	{
		EDriveTypes m_driveType;
		int m_iconIndex;
	};
	SIndexTable m_indexTable[7];
	wchar_t m_shelResDll[MAX_PATH];
public:
	CDriveIconIndex();
	
	/// Returns index for image in the system image list
	/// @param	driveType			Type of the drive
	/// @return				Index for image in the system image list
	int operator()(EDriveTypes driveType);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFileTransferLog;

class CDirContentLogging
{
	tstring m_prefix;
	std::vector<tstring> m_storage;
	CFileTransferLog* m_ftLog;
	bool IsRootDirectory(const TCHAR* fileName);
	void BrowseContent(const tstring& path, void (CDirContentLogging::*cbItem)(const tstring&));
	void PrintToLog(const tstring& fileName);
	void SaveToStorage(const tstring& fileName);
public:
	CDirContentLogging(CFileTransferLog* fileTransferLog);
	void ContentToLog(const tstring& path, const tstring& prefix);
	void StoreContent(const tstring& path);
	void StoredContentToLog(const tstring& prefix);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////