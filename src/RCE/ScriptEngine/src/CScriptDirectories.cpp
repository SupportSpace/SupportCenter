/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CScriptDirectories.cpp
///
///  Implements CScriptDirectories class, responsible for managment of directories
///    with scripts
///
///  @author Dmitry Netrebenko @date 24.12.2007
///
////////////////////////////////////////////////////////////////////////

#include "CScriptDirectories.h"
#include <AidLib/CException/CException.h>
#include <AidLib/Utils/Utils.h>
#include <shlwapi.h>

CScriptDirectories::CScriptDirectories(bool createTempDir)
	:	m_archiveMode(true)
	,	m_sourceScriptDirectory(DEFAULT_SCRIPT_DIR)
	,	m_scriptDirectory(_T(""))
	,	m_localScriptDirectory(_T(""))
	,	m_remoteScriptDirectory(_T(""))
{
TRY_CATCH
	TCHAR fileName[MAX_PATH];
	memset(fileName, 0, sizeof(fileName));
	/// Get current application file name
	DWORD result = GetModuleFileName(
			GetCurrentModule(),
			fileName,
			MAX_PATH
		);
	if(0 == result)
		throw MCException_Win(_T("GetModuleFileName failed"));
	tstring currentDirName(fileName);
	/// Get root directory for scripts
	tstring::size_type pos = currentDirName.rfind(_T("\\"));
	if(tstring::npos != pos)
	{
		currentDirName = currentDirName.substr(0, pos);
		m_sourceScriptDirectory = currentDirName;
	}
	tstring indicatorFile = currentDirName + tstring(ARCHIVE_INDICATOR_FILE);
	m_archiveMode = (FALSE == PathFileExists(indicatorFile.c_str()));

	m_sourceScriptDirectory += SCRIPTS_SUBDIR;

	if(m_archiveMode)
	{
		if(createTempDir)
		{
			/// Create temp directory for scripts
			memset(fileName, 0, MAX_PATH);
			if(!GetTempPath(MAX_PATH, fileName))
				throw MCException(_T("GetTempPath failed."));
			m_scriptDirectory = tstring(fileName) + ::GetGUID() + tstring(_T("\\"));

			if(FALSE == CreateDirectory(m_scriptDirectory.c_str(), NULL))
				throw MCException_Win(_T("Create temp directory for scripts failed"));
			TCHAR longName[MAX_PATH*2];
			memset(longName, 0, MAX_PATH*2);
			if(!GetLongPathName(m_scriptDirectory.c_str(), longName, MAX_PATH*2))
				throw MCException_Win(_T("GetLongPathName failed"));
			m_scriptDirectory = tstring(longName);
		}
		else
			m_scriptDirectory = m_sourceScriptDirectory;
	}
	else
	{
		/// Try to get directory with DHTML from ini file
		TCHAR rootPath[MAX_PATH];
		memset(rootPath, 0, sizeof(rootPath));
		/// Receive directory from file
		DWORD ret = GetPrivateProfileString(SCRIPT_INI_SECTION, SCRIPT_INI_KEY, SCRIPT_INI_DEFAULT_VALUE, rootPath, MAX_PATH, indicatorFile.c_str());
		if(ret)
		{
			/// Directory received
			m_scriptDirectory = rootPath;
			if(FALSE == PathFileExists(m_scriptDirectory.c_str()))
			{
				/// Directory not exists. Setup script directory as default directory
				Log.Add(_ERROR_, _T("Invalid directory %s in config file. Default directory will be used."), m_scriptDirectory.c_str());
				m_scriptDirectory = m_sourceScriptDirectory;
			}
			else
			{
				pos = m_scriptDirectory.rfind(_T("\\"));
				if(tstring::npos != pos)
				{
					/// Add trailing path delimiter
					if(pos != m_scriptDirectory.size() - 1)
						m_scriptDirectory += _T("\\");
				}
			}
		}
		else
			m_scriptDirectory = m_sourceScriptDirectory;
	}
CATCH_THROW()
}

CScriptDirectories::~CScriptDirectories()
{
TRY_CATCH
CATCH_LOG()
}

void CScriptDirectories::SetScriptName(const tstring& scriptName)
{
TRY_CATCH
	m_localScriptDirectory = m_scriptDirectory + scriptName + tstring(LOCAL_SCRIPT_SUBDIR);
	m_remoteScriptDirectory = m_scriptDirectory + scriptName + tstring(REMOTE_SCRIPT_SUBDIR);
CATCH_THROW()
}

tstring CScriptDirectories::GetLocalDirectory() const
{
TRY_CATCH
	return m_localScriptDirectory;
CATCH_THROW()
}

tstring CScriptDirectories::GetRemoteDirectory() const
{
TRY_CATCH
	return m_remoteScriptDirectory;
CATCH_THROW()
}

tstring CScriptDirectories::GetSourceDirectory() const
{
TRY_CATCH
	return m_sourceScriptDirectory;
CATCH_THROW()
}

tstring CScriptDirectories::GetScriptDirectory() const
{
TRY_CATCH
	return m_scriptDirectory;
CATCH_THROW()
}

bool CScriptDirectories::IsArchiveMode() const
{
TRY_CATCH
	return m_archiveMode;
CATCH_THROW()
}
