/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CVersionInfo.cpp
///
///  Implements CVersionInfo class, responsible for obtaining version information
///
///  @author Dmitry Netrebenko @date 17.10.2007
///
////////////////////////////////////////////////////////////////////////

#include <AidLib/CVersionInfo/CVersionInfo.h>
#include <AidLib/CException/CException.h>
#include <windows.h>
#include <boost/shared_array.hpp>
#include <AidLib/Utils/Utils.h>

CVersionInfo::CVersionInfo()
{
TRY_CATCH
CATCH_THROW()
}

CVersionInfo::~CVersionInfo()
{
TRY_CATCH
CATCH_LOG()
}

tstring CVersionInfo::GetFileVersion(const TCHAR* fileName) const
{
TRY_CATCH

	DWORD				handle = 0;
	DWORD				versionInfoSize = 0;
	tstring				fmtVersionStr(_T("%d.%d.%d.%d"));
	tstring				versionStr(_T(""));
	LPVOID				fileVersion;
	UINT				versionSize = 0;
	VS_FIXEDFILEINFO*	versionPtr;

	// Get size of virsion info
	versionInfoSize = GetFileVersionInfoSize(
			fileName,
			&handle
		);
	
	if (versionInfoSize)
	{
		// Create shared array to auto destroy
		boost::shared_array<BYTE> saVersionInfo(new BYTE[versionInfoSize + 1]);
		memset(saVersionInfo.get(), 0, versionInfoSize + 1);
		
		// Get version info
		BOOL getInfoRes = GetFileVersionInfo(
				fileName,
				handle,
				versionInfoSize,
				saVersionInfo.get()
			);

		if (getInfoRes)
		{
			// Get version structure
			BOOL queryRes = VerQueryValue(
					saVersionInfo.get(),
					_T("\\"),
					&fileVersion,
					&versionSize
				);

			if (queryRes)
			{
				versionPtr = (VS_FIXEDFILEINFO*)fileVersion;

				// Format version string
				versionStr = Format(
						fmtVersionStr,
						(DWORD)(((versionPtr->dwProductVersionMS) & 0xFFFF0000) >> 16),
						(DWORD)((versionPtr->dwProductVersionMS) & 0x0000FFFF),
						(DWORD)(((versionPtr->dwProductVersionLS) & 0xFFFF0000) >> 16),
						(DWORD)((versionPtr->dwProductVersionLS) & 0x0000FFFF)
					);
			}
		}
	}

	return versionStr;

CATCH_THROW()
}

tstring CVersionInfo::GetCurrentFileVersion() const
{
TRY_CATCH

	TCHAR fileName[MAX_PATH];

	memset(fileName, 0, sizeof(fileName));

	// Get current application file name
	GetModuleFileName(
			GetCurrentModule(),
			fileName,
			MAX_PATH
		);

	return GetFileVersion(fileName);

CATCH_THROW()
}
