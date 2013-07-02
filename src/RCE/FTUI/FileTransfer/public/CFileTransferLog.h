//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFileTransferLog.h
///
///  Declares CFileTransferLog class
///  Performs logging for file operations
///  
///  @author Alexander Novak @date 11.01.2008
///	 @modified Max Sogin @date 13.01.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Logging/CLogFolder.h>
#include <AidLib/CException/CException.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFileTransferLog
	:	public cLog
{
public:
	tstring m_prefix;
	/// Here is performed proper output
	std::auto_ptr<cFileLog> m_fileLog;
public:
	/// Creates file transfer log
	/// @param fileName				File log name
	CFileTransferLog() {}

	/// Initializes logging. After this call log file is created and logging starts
	/// @param sid session id
	/// @param customerName - customer user name
	/// @param expertName - expert user name
	void Init(const tstring& sid, const tstring& customerName, const tstring& expertName);
	
	/// Sets prefix for logging, it goes before message body
	/// @param prefix				Prefix for message body
	void SetMessagePrefix(const tstring& prefix);

	/// Adds log message with current date/time + prefix + message
	/// @param message				Message body
	void AddMessage(const tstring& message);

	/// cLog interface implementation
	virtual void AddString(const TCHAR* LogString, const eSeverity Severity);
};
//--------------------------------------------------------------------------------------------------------

inline void CFileTransferLog::SetMessagePrefix(const tstring& prefix)
{
	m_prefix = prefix;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
