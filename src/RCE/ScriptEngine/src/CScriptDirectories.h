/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CScriptDirectories.h
///
///  Declares CScriptDirectories class, responsible for managment of directories
///    with scripts
///
///  @author Dmitry Netrebenko @date 24.12.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <AidLib/Strings/tstring.h>

#define DEFAULT_SCRIPT_DIR			_T("C:\\")
#define SCRIPTS_SUBDIR				_T("\\Scripts\\")
#define LOCAL_SCRIPT_SUBDIR			_T("\\Local\\")
#define REMOTE_SCRIPT_SUBDIR		_T("\\Remote\\")
#define ARCHIVE_INDICATOR_FILE		_T("\\se_archive.off")
#define SCRIPT_INI_SECTION			_T("ScriptEngine")
#define SCRIPT_INI_KEY				_T("DhtmlRootPath")
#define SCRIPT_INI_DEFAULT_VALUE	_T("")

/// CScriptDirectories class, responsible for managment of directories with scripts
class CScriptDirectories
{
private:
/// Prevents making copies of CScriptDirectories objects.
	CScriptDirectories( const CScriptDirectories& );
	CScriptDirectories& operator=( const CScriptDirectories& );
public:
/// Constructor
/// @param createTempDir - indicates creation of temporary directory for scripts
	CScriptDirectories(bool createTempDir = true);
/// Destructor
	~CScriptDirectories();
/// Initializes all members depends on name of script
/// @param scriptName - name of script
	void SetScriptName(const tstring& scriptName);
/// Returns directory with local scripts
	tstring GetLocalDirectory() const;
/// Returns directory with remote scripts
	tstring GetRemoteDirectory() const;
/// Returns script source directory
	tstring GetSourceDirectory() const;
/// Returns script directory
	tstring GetScriptDirectory() const;
/// Returns true if scripts in archives
	bool IsArchiveMode() const;
private:
/// Indicates search scripts inside archives
	bool		m_archiveMode;
/// Directory where script placed 
	tstring		m_sourceScriptDirectory;
/// Directoty where unpacked script is placed
	tstring		m_scriptDirectory;
/// Directory with local scripts
	tstring		m_localScriptDirectory;
/// Directory with remote scripts
	tstring		m_remoteScriptDirectory;
};
