/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CVersionInfo.h
///
///  Declares CVersionInfo class, responsible for obtaining version information
///
///  @author Dmitry Netrebenko @date 17.10.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>
#include <AidLib/Loki/Singleton.h>
#include <AidLib/AidLib.h>

/// CVersionInfo class, responsible for obtaining version information
class AIDLIB_API CVersionInfo
{
private:
/// Prevents making copies of CVersionInfo objects
	CVersionInfo(const CVersionInfo&);
	CVersionInfo& operator=(const CVersionInfo&);
public:
/// Constructor
	CVersionInfo();
/// Destructor
	~CVersionInfo();
/// Return file version of file
/// @param fileName - name of file
/// @returm string with version of file
	tstring GetFileVersion(const TCHAR* fileName) const;
/// Returns version of the current file
	tstring GetCurrentFileVersion() const;
};

/// Should be used to CStatisticClient as single instance
#define VERSION_INFO_INSTANCE Loki::SingletonHolder<CVersionInfo, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
