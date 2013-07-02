//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFileTransferLog.cpp
///
///  Implements CFileTransferLog class
///  Performs logging for file operations
///  
///  @author Alexander Novak @date 11.01.2008
///	 @modified Max Sogin @date 13.01.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CFileTransferLog.h"
#include <AidLib/CTime/cTime.h>

// CFileTransferLog [BEGIN] //////////////////////////////////////////////////////////////////////////////

void CFileTransferLog::AddMessage(const tstring& message)
{
TRY_CATCH

	SYSTEMTIME sysTime;
	GetSystemTime(&sysTime);

	TCHAR frmtStr[25];
	_stprintf_s(frmtStr,
				sizeof(frmtStr)/sizeof(frmtStr[0]),
				_T("%d/%.2d/%d %d:%.2d:%.2d"),
				sysTime.wDay,
				sysTime.wMonth,
				sysTime.wYear,
				sysTime.wHour,
				sysTime.wMinute,
				sysTime.wSecond);

	tstring logMessage(frmtStr);

	logMessage += m_prefix;
	logMessage += message;
	logMessage += _T("\r\n");

	Add(logMessage.c_str());

CATCH_THROW()
}

void CFileTransferLog::Init(const tstring& sid, const tstring& customerName, const tstring& expertName)
{
TRY_CATCH	
	
	//dd.MM.yyyy
	SYSTEMTIME now(cDate().GetNow());
	tstring date = Format(_T("%.2d.%.2d.%.4d"),now.wDay,now.wMonth,now.wYear);
	tstring FileTransferLogSubFolder = _T("FileManagerSessions");
	tstring fileName;
	if (FALSE == CreateDirectory(Format(_T("%s\\%s"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str(),FileTransferLogSubFolder.c_str()).c_str(), NULL)
		&& ERROR_ALREADY_EXISTS != GetLastError())
		fileName= Format(_T("%s\\SessionID_%s_%s.log"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str(),sid.c_str(),date.c_str());
	else
		fileName= Format(_T("%s\\%s\\SessionID_%s_%s.log"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str(),FileTransferLogSubFolder.c_str(),sid.c_str(),date.c_str());
	m_fileLog.reset(new cFileLog(fileName.c_str()));
	AddMessage(Format(_T("SessionID %s"),sid.c_str()));
	Add(Format(_T("Expert name: %s\r\n"),expertName.c_str()).c_str());
	Add(Format(_T("Customer name: %s\r\n"),customerName.c_str()).c_str());

CATCH_LOG()
}

void CFileTransferLog::AddString(const TCHAR* LogString, const eSeverity Severity)
{
TRY_CATCH
	if (NULL == m_fileLog.get())
		return;
	m_fileLog->Add(LogString);
CATCH_THROW()
}
// CFileTransferLog [END] ////////////////////////////////////////////////////////////////////////////////
