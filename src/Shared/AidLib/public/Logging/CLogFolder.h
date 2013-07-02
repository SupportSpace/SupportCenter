/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLogFolder.h
///
///  Class for slimplified logs gathering into one folder
///
///  @author "Archer Software" Sogin M. @date 08.08.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once

#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>
#include <AidLib/CScopedTracker/CScopedTracker.h>
#include <AidLib/Utils/Utils.h>

#define ORIGINAL_WIN32_WINNT _WIN32_WINNT
#if(_WIN32_WINNT < 0x0500)
	#define _WIN32_WINNT 0x0500
#endif
#include "Sddl.h"
#define _WIN32_WINNT ORIGINAL_WIN32_WINNT

/// File for creating / retriving the name of logging folder
class CLogsFolder
{
private:
	friend class CSingleton<CLogsFolder>;
#define LOGSFOLDERNAMECONTAINERNAME _T("Global\\F8987FBC-296B-4132-8192-7A5C43C7C7C2")

	/// Memory mapping to store logs path, when it's known
	/// Mapping needed to share it between modules within process
	CScopedTracker<HANDLE> m_mapping;
	CScopedTracker<void*> m_mappingBuf;

	/// Closed ctor
	CLogsFolder() {};

	/// Set new log folder name (saves it to memmory mapped file)
	void SetLogsFolderName(const tstring& logsFolderName)
	{
	TRY_CATCH
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = FALSE;
			
		tstring SDString = GetOSVersion()>5?_T("S:(ML;;NW;;;LW)D:(A;;GAFA;;;WD)"):_T("D:(A;;GAFA;;;WD)");
		if (FALSE == ConvertStringSecurityDescriptorToSecurityDescriptor(	SDString.c_str(), 
																			SDDL_REVISION_1, 
																			&sa.lpSecurityDescriptor, 
																			NULL) )
			throw MCException_Win(_T("Failed to ConvertStringSecurityDescriptorToSecurityDescriptor "));
		CScopedTracker<PSECURITY_DESCRIPTOR> psd;
		psd.reset(sa.lpSecurityDescriptor, LocalFree);

		DWORD mappingSize = (logsFolderName.length()+1)*sizeof(TCHAR);	
		m_mapping.reset(CreateFileMapping(	INVALID_HANDLE_VALUE,	// use paging file
											&sa,					// default security 
											PAGE_READWRITE,			// read/write access
											0,						// max. object size 
											mappingSize,			// buffer size  
											LOGSFOLDERNAMECONTAINERNAME	// name of mapping object
											), CloseHandle);
		if (NULL == m_mapping || INVALID_HANDLE_VALUE == m_mapping)
			throw MCException_Win("Failed to CreateFileMapping ");

		m_mappingBuf.reset( MapViewOfFile(m_mapping, FILE_MAP_ALL_ACCESS, 0, 0, mappingSize), UnmapViewOfFile );
	
		CopyMemory(m_mappingBuf, logsFolderName.c_str(), mappingSize);

	CATCH_THROW()
	}
public:
	/// Creates log folder (with All access to All rights)
	void CreateLogsFolder()
	{
	TRY_CATCH
		tstring modulePath = RemoveTrailingSlashes(GetModulePath(GetCurrentModule()));
		tstring directoryName = Format(_T("%s\\Logs"),modulePath.c_str());
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = FALSE;

		tstring SDString = GetOSVersion()>5?_T("S:(ML;OICI;NW;;;LW)D:(A;OICI;GAFARPWPCCDCLCSWRCWDWOGA;;;WD)"):_T("D:(A;OICI;GAFARPWPCCDCLCSWRCWDWOGA;;;WD)");
		if (FALSE == ConvertStringSecurityDescriptorToSecurityDescriptor(	SDString.c_str(), 
																			SDDL_REVISION_1, 
																			&sa.lpSecurityDescriptor, 
																			NULL) )
			throw MCException_Win(_T("Failed to ConvertStringSecurityDescriptorToSecurityDescriptor "));
		CScopedTracker<PSECURITY_DESCRIPTOR> psd;
		psd.reset(sa.lpSecurityDescriptor, LocalFree);
		if (FALSE == CreateDirectory(directoryName.c_str(), &sa)
			&&
			ERROR_ALREADY_EXISTS != GetLastError())
			throw MCException_Win("Failed to create logs directory ");
		SetLogsFolderName(directoryName);
	CATCH_LOG()
	}

	/// Returns folder name for logs
	/// If memory mapped file exists - returns value from it
	/// current module path - elsevay
	/// empty string for in case of some hard error is returned
	tstring GetLogsFolderName()
	{
	TRY_CATCH
		CScopedTracker<HANDLE> mapping;
		mapping.reset(OpenFileMapping(FILE_MAP_READ, FALSE, LOGSFOLDERNAMECONTAINERNAME), CloseHandle);
		if (NULL == mapping)
			throw MCException_Win("Failed to OpenFileMapping ");

		CScopedTracker<void*> buf;
		buf.reset(MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0), UnmapViewOfFile);
		if (NULL == buf)
			throw MCException_Win("Failed to MapViewOfFile ");

		tstring folder = reinterpret_cast<TCHAR*>(buf.get());
		return folder;

	CATCH_LOG()
	TRY_CATCH
		/// Failed to read folder name from memory mapped file
		// return Format(_T("%s\\Logs"),RemoveTrailingSlashes(GetModulePath(GetCurrentModule())).c_str());
		return RemoveTrailingSlashes(GetModulePath(GetCurrentModule()));
	CATCH_LOG()
		return tstring();
	}
};

#define LOGS_FOLDER_INSTANCE CSingleton<CLogsFolder>::instance()