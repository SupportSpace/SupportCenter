//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFileConnectivityQualityLog.h
///
///  Declares CFileConnectivityQualityLog class
///  Performs logging for file operations
///  
///  @author Anatoly Gutnick @date 21.08.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Logging/CLogFolder.h>
#include <AidLib/CException/CException.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TraceLog 1
#define FullLog  2

class CFileConnectivityQualityLog
	:	public cLog
{
public:
	tstring m_prefix;
	/// Here is performed proper output
	std::auto_ptr<cFileLog> m_fileLog;

	int m_logLevel;
public:
	/// Creates file transfer log
	/// @param fileName				File log name
	CFileConnectivityQualityLog() {}

	/// Initializes logging. After this call log file is created and logging starts
	/// @param filename
	void Init(const tstring& fname, const int logLevel);
	
	/// Sets prefix for logging, it goes before message body
	/// @param prefix				Prefix for message body
	void SetMessagePrefix(const tstring& prefix);

	/// Adds log message with current date/time + prefix + message
	/// @param message				Message body
	void AddMessage(_eSeverity logSeverity, const tstring& message);

	/// cLog interface implementation
	virtual void AddString(const TCHAR* LogString, const eSeverity Severity);
};
//--------------------------------------------------------------------------------------------------------

inline void CFileConnectivityQualityLog::SetMessagePrefix(const tstring& prefix)
{
	m_prefix = prefix;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
