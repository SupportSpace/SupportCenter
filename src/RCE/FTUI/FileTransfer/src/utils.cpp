//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  utils.cpp
///
///  Implements classes and methods for using in the FileTransfer library
///  
///  @modified Alexander Novak @date 23.01.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "utils.h"
#include "zip\zip_arch.h"
#include "filetransferdata.h"
#include <AidLib/CException/CException.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <algorithm>
#include "CFileTransferLog.h"

#pragma warning( disable: 4996 )//<func> was declared deprecated

namespace transfer_utils
{
	BOOL IsFolder(const tstring& fn)
	{
		///return (GetFileAttributes( fn.c_str() ) & FILE_ATTRIBUTE_DIRECTORY)?TRUE:FALSE; 
		/// fixed bug RCE-18
		//WIN32_FILE_ATTRIBUTE_DATA info={0};
		//BOOL ret = GetFileAttributesEx( fn.c_str() , GetFileExInfoStandard , &info );
		//return ( ret )?( info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ):FALSE;
		SetErrorMode(SEM_FAILCRITICALERRORS); // No popup please !
		BOOL res = PathIsDirectory( fn.c_str() );
		SetErrorMode(0); 
		return res;
	}

	BOOL ZipFolder( const tstring& fn , tstring& nfn )
	{
		std::vector<char> szZipPath;
		szZipPath.resize( 512 );

		//GetTempPath( static_cast<DWORD>(szZipPath.capacity()) , &szZipPath[0] );	//FIX to real folder name instead of folder.arc
		//lstrcat(&szZipPath[0], "folder.arc");										//FIX to real folder name instead of folder.arc

		size_t pos = fn.find_last_of("\\");

		tstring path;//( fn.begin() , fn.begin() + pos );

		//if( path[pos-1]!=':' )
		//	path.assign( fn.begin() , fn.begin() + pos + 1 );
		//else
		// Commented since path doesn't initialized in line 33
		path.assign( fn.begin() , fn.begin() + pos+1 );

		tstring folder( fn.begin() + ++pos , fn.end() );

		GetTempPath( static_cast<DWORD>(szZipPath.capacity()) , &szZipPath[0] );	//FIX to real folder name instead of folder.arc
		lstrcat(&szZipPath[0], folder.c_str());										//FIX to real folder name instead of folder.arc
		lstrcat(&szZipPath[0], _T(".arc"));											//FIX to real folder name instead of folder.arc

		CZipUnZip32 zip_arch;
		//TCHAR chPath[512]={'\0'}; 
		//TCHAR chFolder[512]={'\0'}; 
		//strcpy( chPath , path.c_str() );
		//strcpy( chFolder , folder.c_str() );

		DeleteFile(&szZipPath[0]);			// Delete file if it's exist in temporary directory

		if( zip_arch.ZipDirectory( const_cast<char*>(path.c_str()) , const_cast<char*>(folder.c_str()) , &szZipPath[0] , true )) 
		{
			nfn =  &szZipPath[0];
			return TRUE;
		};
		return FALSE;
	}

	BOOL UnzipAndDeleteFile(const tstring& root, const tstring& fn)
	{
		BOOL ret = FALSE;
		CZipUnZip32 zip_arch;
		//tstring path( fn );
		//size_t pos = path.find_last_of("\\");
		//if( path[pos-1]!=':' )
		//	path[ pos ] = '\0';
		//else
		//	path[pos+1] = '\0';

//		ret = zip_arch.UnZipDirectory( const_cast<char*>( path.c_str() ) , const_cast<char*>(fn.c_str()) );
//		ret&= ::DeleteFile( fn.c_str() );

		ret = zip_arch.UnZipDirectory( const_cast<char*>( root.c_str() ) , const_cast<char*>(fn.c_str()) );
		ret&= ::DeleteFile( fn.c_str() );

		return ret;
	}

	unsigned short get_buf_len_and_prepare( char* buf )
	{
		TRY_CATCH
		char* it = buf;
		for( ;it;++it )
		{
			if( '\n' == *it )
			{
				if( '\0' == *(it+1) )
				{
					*it ='\0';
					return (unsigned short)( it - buf );
				}
				else
				{
					*it = '\0';
				}
			}
		}
		return -1;
		CATCH_THROW("get_buf_len_and_prepare")
	}


	EDriveTypes get_drive_types( char* drive )
	{
		TRY_CATCH
		//	UINT nType = GetDriveType(drive);
		//switch (nType)
		//{
		//case DRIVE_FIXED:
		//	return local;
		//	break;
		//case DRIVE_REMOVABLE:
		//	return removable;
		//	break;
		//case DRIVE_CDROM:
		//	return cdrom;
		//	break;
		//case DRIVE_REMOTE:
		//	return remote;
		//	break;
		//}
		//return unspecified;
		
		static CDriveType getDriveType;
		
		return getDriveType(drive);
		
		CATCH_THROW("get_drive_types")
	}

	spPair get_drives()
	{
		TCHAR szDrivesList[256]; // Format when filled : "C:\<NULL>D:\<NULL>....Z:\<NULL><NULL>
		DWORD dwLen;
		DWORD nIndex = 0;
		int nType = 0;
		TCHAR szDrive[4];
		TCHAR szBuf[256];
		TCHAR szSendingBuf[512]={'\0'};
		dwLen = GetLogicalDriveStrings(256, szDrivesList);

		// We add Drives types to this drive list...
		while (nIndex < dwLen - 3)
		{
			strcpy(szDrive, szDrivesList + nIndex);
			EDriveTypes type = get_drive_types( szDrive );
			ULARGE_INTEGER free;
			ULARGE_INTEGER total;
			//if( removable == type || cdrom == type )
			//{
			//	free.QuadPart = 0;
			//	total.QuadPart = 0;
			//}
			//else
			//{
			//	GetDiskFreeSpaceEx( szDrive , 0 , &total , &free  );
			//	free.QuadPart= free.QuadPart/(1024*1024);
			//	total.QuadPart=total.QuadPart/(1024*1024);
			//}

			free.QuadPart = total.QuadPart = 0;
			if ( type != DT_FLOPPY )
			{
				UINT prevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
				GetDiskFreeSpaceEx(szDrive,NULL,&total,&free);
				SetErrorMode(prevErrorMode);
			}
			
			// We replace the "\" char following the drive letter and ":"
			// with a char corresponding to the type of drive
			//We obtain something like "C:l:total_size:free_size<NULL>D:c:total_size:free_size<NULL>....Z:n\<NULL><NULL>"
			// Isn't it ugly ?

			sprintf(szBuf,"%s:%i:%I64u:%I64u\n" , szDrive , type, total.QuadPart, free.QuadPart);
			nIndex += 4;
			strncat( szSendingBuf , szBuf , strlen( szBuf )+1 );
		}
		int length = get_buf_len_and_prepare( szSendingBuf )+1;		
		CHAR* ret = new CHAR[ length+1 ];
		
		for( int i = 0; i < length; ++i )
			ret[i] = szSendingBuf[i];
		
		ret[length] = '\0';
		return std::make_pair( length, spBuffer(ret));
	}

	/**
	* This is a helpful function for TokenizeDrives function
	*/
	void TokenizeInfo(TDriveInfo::value_type& di, char* info)
	{
		TRY_CATCH
			char d;
		int ret = sscanf( info , "%c:\\:%d:%d:%d" , &d , &di.type , &di.space , &di.used_space );
		di.drive = d;
		di.drive+=":";
		CATCH_THROW("CFileAccessClient::TokenizeInfo")
	}

	/**
	* You recived from server information looks like "C:l:total_size:free_size<NULL>D:
	* c:total_size:free_size<NULL>....Z:n\<NULL><NULL>" and this function parses this
	* information and saves in sotore.
	*/
	void TokenizeDrives(TDriveInfo& dl, unsigned int len, char* drives)
	{
		TRY_CATCH
			char p[64];
		DrivesInfo di;
		for( unsigned i=0,k=0;i<len;++i )
		{
			if( drives[i]!='\0' )
				p[k++] = drives[i];
			else
			{
				p[k] = '\0';
				TokenizeInfo( di ,p  );
				dl.push_back( di );
				k=0;
			}
		}
		CATCH_THROW("CFileAccessClient::TokenizeDrives")
	}

	void TemplateMethod( const TCHAR* folder ,  CHookOperation& operation )
	{
		char szDir[MAX_PATH];
		strcpy( szDir , folder );
		strcat(szDir, "*.*");

		WIN32_FIND_DATA fd;
		HANDLE ff;
		BOOL fRet = TRUE;



		SetErrorMode(SEM_FAILCRITICALERRORS); // No popup please !
		ff = FindFirstFile(szDir, &fd);
		SetErrorMode( 0 );
		// fix RCE-63 && RCE-43
		if (ff == INVALID_HANDLE_VALUE)
		{
			return; 
		}

		int nItem=0;

		while ( fRet )
		{
			// sf@2003 - Convert file time to local time
			// We've made the choice off displaying all the files 
			// off client AND server sides converted in clients local
			// time only. So we don't convert server's files times.
			/* 
			FILETIME LocalFileTime;
			FileTimeToLocalFileTime(&fd.ftLastWriteTime, &LocalFileTime);
			fd.ftLastWriteTime.dwLowDateTime = LocalFileTime.dwLowDateTime;
			fd.ftLastWriteTime.dwHighDateTime = LocalFileTime.dwHighDateTime;
			*/
			if (strcmp(fd.cFileName, "."))
			{
				operation.DoOperation( fd );
			}
			fRet = FindNextFile(ff, &fd);
		}
		FindClose(ff);
	}


	class ToUpper
	{
	public:
		ToUpper(const std::locale& l)
			: m_l  (l)
			, m_ct (std::use_facet<std::ctype<char> >(m_l))
		{
		}

		char operator()(char c) const
		{
			return m_ct.toupper(c);
		}

	private:
		std::locale m_l;
		const std::ctype<char>& m_ct;
	};

	class ToLower
	{
	public:
		ToLower(const std::locale& l): m_l  (l)
			, m_ct (std::use_facet<std::ctype<char> >(m_l))
		{
		}


		char operator()(char c) const
		{
			return m_ct.tolower(c);
		}

	private:
		std::locale m_l;
		const std::ctype<char>& m_ct;
	};

	void to_upper(const std::string& in, std::string& out, const std::locale& lc)
	{
		if (out.length() < in.length())
			out.resize(in.length());
		std::transform(in.begin(), in.end(), out.begin(), ToUpper(lc));
	}

	void to_lower(const std::string& in, std::string& out, const std::locale& lc)
	{
		if (out.length() < in.length())
			out.resize(in.length());
		std::transform(in.begin(), in.end(), out.begin(), ToLower(lc));
	}

	int compare_no_case(const tstring& l, const tstring& r, const std::locale& lc)
	{
		ToUpper transformer (lc);

		std::string l_nc;
		l_nc.resize(l.length());
		std::transform(l.begin(), l.end(), l_nc.begin(), transformer);

		std::string r_nc;
		r_nc.resize(r.length());
		std::transform(r.begin(), r.end(), r_nc.begin(), transformer);

		return l_nc.compare(r_nc);
	}

	tstring get_folder_name( const tstring& path_)
	{
		/// try to extract folder's name from path
		/// path consists something like this  disk_name:\root_folder\folder1\folder2\ ...folderN\
		/// amd I must extract folderN
		tstring temp_path( path_.begin() , path_.end() - 1 );
		tstring::size_type pos = temp_path.find_last_of("\\");
		if( tstring::npos != pos )
			return tstring( temp_path.begin() + pos+1 , temp_path.end() );
		return tstring();
	}

	tstring get_file_name( const tstring& path_)
	{
		/// try to extract file's name from path
		/// path consists something like this  disk_name:\root_folder\folder1\folder2\ ...folderN\filemane.ext
		/// amd I must extract filemane.ext

		//tstring temp_path( path_.begin() , path_.end() - 1 );
		tstring::size_type pos = path_.find_last_of("\\");
		if( tstring::npos != pos )
			return tstring( path_.begin() + pos+1 , path_.end() );
		return tstring();
	}
};

// CDriveType [BEGIN] ////////////////////////////////////////////////////////////////////////////////////

EDriveTypes CDriveType::operator()(const TCHAR* driveName)
{
TRY_CATCH

	DWORD mappedType = m_mapDriveTypes[driveName];
	if ( mappedType )
		return EDriveTypes( mappedType & 0x7FFFFFFF );
	
	EDriveTypes driveType = DT_UNKNOW;

	switch ( GetDriveType(driveName) )
	{
		case DRIVE_FIXED:
			driveType = DT_FIXED;
			break;

		case DRIVE_REMOTE:
			driveType = DT_REMOTE;
			break;

		case DRIVE_CDROM:
			driveType = DT_CDROM;
			break;

		case DRIVE_RAMDISK:
			driveType = DT_RAMDISK;
			break;

		case DRIVE_REMOVABLE:
		{
			// Try to determinate floppy drive through DeviceIoControl
			tstring deviceName(_T("\\\\.\\"));
			deviceName += driveName;

			// Remove the termination slash
			if ( deviceName.size() && *deviceName.rbegin() == _T('\\') )
				deviceName.erase( deviceName.size()-1 );

			boost::shared_ptr<boost::remove_pointer<HANDLE>::type> hDevice( CreateFile(	deviceName.c_str(),
																						0,
																						FILE_SHARE_READ | FILE_SHARE_WRITE,
																						NULL,
																						OPEN_EXISTING,
																						0,
																						NULL),CloseHandle);
			if ( hDevice.get() != INVALID_HANDLE_VALUE )
			{
				DISK_GEOMETRY diskGeometry;
				DWORD returnedBytes;
				BOOL result = DeviceIoControl(	hDevice.get(),
												IOCTL_DISK_GET_DRIVE_GEOMETRY,
												NULL,
												0,
												&diskGeometry,
												sizeof(diskGeometry),
												&returnedBytes,
												NULL);
				if ( result && returnedBytes == sizeof(diskGeometry) )
					switch ( diskGeometry.MediaType )
					{
						case Unknown:
						case FixedMedia:
						case RemovableMedia:
							driveType = DT_REMOVABLE;
							break;

						default:
							driveType = DT_FLOPPY;
					}
			}
			if ( driveType == DT_UNKNOW )		// Try to determinate floppy drive through driveName
			{
				if ( _tcscmp(deviceName.c_str(),_T("\\\\.\\A:"))==0 || _tcscmp(deviceName.c_str(),_T("\\\\.\\B:"))==0 )
					driveType = DT_FLOPPY;
				else 
					driveType = DT_REMOVABLE;
			}
		}
	}
	m_mapDriveTypes[driveName] = (DWORD)driveType | 0x80000000;
	return driveType;

CATCH_THROW()
}
// CDriveType [END] //////////////////////////////////////////////////////////////////////////////////////

// CDriveIconIndex [BEGIN] ///////////////////////////////////////////////////////////////////////////////

CDriveIconIndex::CDriveIconIndex()
{
TRY_CATCH

	GetSystemDirectoryW(m_shelResDll,MAX_PATH);
	wcscat_s(m_shelResDll,MAX_PATH,L"\\shell32.dll");

	m_indexTable[0].m_driveType = DT_FLOPPY;
	m_indexTable[0].m_iconIndex = 0x80000000 | 6;

	m_indexTable[1].m_driveType = DT_FIXED;
	m_indexTable[1].m_iconIndex = 0x80000000 | 8;

	m_indexTable[2].m_driveType = DT_REMOVABLE;
	m_indexTable[2].m_iconIndex = 0x80000000 | 7;

	m_indexTable[3].m_driveType = DT_CDROM;
	m_indexTable[3].m_iconIndex = 0x80000000 | 11;

	m_indexTable[4].m_driveType = DT_REMOTE;
	m_indexTable[4].m_iconIndex = 0x80000000 | 9;

	m_indexTable[5].m_driveType = DT_RAMDISK;
	m_indexTable[5].m_iconIndex = 0x80000000 | 12;

	m_indexTable[6].m_driveType = DT_UNKNOW;
	m_indexTable[6].m_iconIndex = 0x80000000 | 53;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

int CDriveIconIndex::operator()(EDriveTypes driveType)
{
TRY_CATCH

	for ( int i = 0; i < sizeof(m_indexTable)/sizeof(m_indexTable[0]); i++ )
		if ( m_indexTable[i].m_driveType == driveType )
		{
			if ( m_indexTable[i].m_iconIndex & 0x80000000 )		// Not cached yet
			{
				m_indexTable[i].m_iconIndex = Shell_GetCachedImageIndex(m_shelResDll,
																		m_indexTable[i].m_iconIndex & 0x7FFFFFFF,
																		0);
				
				if ( m_indexTable[i].m_iconIndex == -1 )
					m_indexTable[i].m_iconIndex = 0;			// Use a zero-indexed icon as default
			}
			return m_indexTable[i].m_iconIndex;
		}
	return 0;

CATCH_THROW()
}
// CDriveIconIndex [END] /////////////////////////////////////////////////////////////////////////////////

// CDirContentLogging [BEGIN] ////////////////////////////////////////////////////////////////////////////

bool CDirContentLogging::IsRootDirectory(const TCHAR* fileName)
{
TRY_CATCH

	const TCHAR* rootMask = _T("..");

	int i = 0;
	while ( rootMask[i] && fileName[i]==rootMask[i] )
		i++;

	return ( i != 0 && fileName[i] == _T('\0') );

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CDirContentLogging::BrowseContent(const tstring& path, void (CDirContentLogging::*cbItem)(const tstring&))
{
TRY_CATCH

	WIN32_FIND_DATA fileData;
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
			do
			{
				if ( ( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && IsRootDirectory(fileData.cFileName) )
					continue;

				tstring fullPath = lstDirs.front();
				fullPath += _T("\\");
				fullPath += fileData.cFileName;

				(this->*cbItem)(fullPath);

				if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
					tstring strForSearch = lstDirs.front();
					strForSearch += _T("\\");
					strForSearch += fileData.cFileName;

					lstDirs.push_back(strForSearch);
				}
			}
			while ( FindNextFile(hFind.get(),&fileData) );
		}
		lstDirs.pop_front();
	}// while ( !stDirs.empty() )

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CDirContentLogging::PrintToLog(const tstring& fileName)
{
TRY_CATCH

	m_ftLog->AddMessage(Format(_T("%s%s"),m_prefix.c_str(),fileName.c_str()).c_str());

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CDirContentLogging::SaveToStorage(const tstring& fileName)
{
TRY_CATCH
	
	m_storage.push_back(fileName);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CDirContentLogging::CDirContentLogging(CFileTransferLog* fileTransferLog)
	:	m_ftLog(fileTransferLog)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CDirContentLogging::ContentToLog(const tstring& path, const tstring& prefix)
{
TRY_CATCH

	m_prefix = prefix;
	BrowseContent(path,&CDirContentLogging::PrintToLog);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CDirContentLogging::StoreContent(const tstring& path)
{
TRY_CATCH

	m_storage.clear();
	BrowseContent(path,&CDirContentLogging::SaveToStorage);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CDirContentLogging::StoredContentToLog(const tstring& prefix)
{
TRY_CATCH

	for ( unsigned int i = 0; i < m_storage.size(); i++ )
		m_ftLog->AddMessage(Format(_T("%s%s"),prefix.c_str(),m_storage[i].c_str()).c_str());

CATCH_THROW()
}
// CDirContentLogging [END] //////////////////////////////////////////////////////////////////////////////
