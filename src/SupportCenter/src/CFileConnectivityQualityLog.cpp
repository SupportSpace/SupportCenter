//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFileConnectivityQualityLog.cpp
///
///  Implements CFileConnectivityQualityLog class
///  Performs logging for file operations
///  
///  @author Alexander Novak @date 11.01.2008
///	 @modified Max Sogin @date 13.01.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include "CFileConnectivityQualityLog.h"
#include <AidLib/CTime/cTime.h>

// CFileConnectivityQualityLog [BEGIN] //////////////////////////////////////////////////////////////////////////////

void CFileConnectivityQualityLog::AddMessage(_eSeverity logSeverity, const tstring& message)
{
TRY_CATCH

	if(m_logLevel==1 && logSeverity > _EXCEPTION)
		return;

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

void CFileConnectivityQualityLog::Init(const tstring& sid, const int logLevel)
{
TRY_CATCH

	m_logLevel = logLevel;
	
	//dd.MM.yyyy
	SYSTEMTIME now(cDate().GetNow());
	tstring date = Format(_T("%.2d.%.2d.%.4d"),now.wDay,now.wMonth,now.wYear);
	tstring FileTransferLogSubFolder = _T("Logs");
	tstring fileName;

	if (FALSE == CreateDirectory(Format(_T("%s\\%s"),RemoveTrailingSlashes(GetModulePath(GetCurrentModule())).c_str(),FileTransferLogSubFolder.c_str()).c_str(), NULL)
		&& ERROR_ALREADY_EXISTS != GetLastError())
	{
	
	}
	
	fileName= Format(_T("%s\\%s\\%s.log"),
					 RemoveTrailingSlashes(GetModulePath(GetCurrentModule())).c_str(),
					 FileTransferLogSubFolder.c_str(),
					 sid.c_str());

	m_fileLog.reset(new cFileLog(fileName.c_str()));

	// Calculating difference between UTC and local time
	FILETIME utcFt, localFt;
	GetSystemTimeAsFileTime(&utcFt);
	FileTimeToLocalFileTime(&utcFt,&localFt);
	
	LARGE_INTEGER liUtc, liLocal;
	liUtc.HighPart		= utcFt.dwHighDateTime;
	liUtc.LowPart		= utcFt.dwLowDateTime;
	liLocal.HighPart	= localFt.dwHighDateTime;
	liLocal.LowPart		= localFt.dwLowDateTime;
		
	liLocal.QuadPart -= liUtc.QuadPart;
	// Trunc to minutes
	liLocal.QuadPart /= 600000000L;
	TCHAR prefix[20];
	_stprintf_s(prefix,
				sizeof(prefix)/sizeof(prefix[0]),
				_T(" GMT (%+.2d:00) "),				// Don't print a minute difference, it looks ugly
				liLocal.QuadPart/60);

	SetMessagePrefix(prefix);

	AddMessage(cLog::_MESSAGE,Format(_T("SupportCenter started %s"),sid.c_str()));
	//Add(Format(_T("Expert name: %s\r\n"),expertName.c_str()).c_str());
	//Add(Format(_T("Customer name: %s\r\n"),customerName.c_str()).c_str());

CATCH_LOG()
}

void CFileConnectivityQualityLog::AddString(const TCHAR* LogString, const eSeverity Severity)
{
TRY_CATCH
	if (NULL == m_fileLog.get())
		return;
	m_fileLog->Add(LogString);
CATCH_THROW()
}
// CFileConnectivityQualityLog [END] ////////////////////////////////////////////////////////////////////////////////
