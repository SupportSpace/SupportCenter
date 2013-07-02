//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CWebClientSettings
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CExecutionQueueNode :  class
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 10/02/2007 05:21:10 PM
// Comments: First Issue
//===========================================================================
#include <windows.h>
#include "WebClientSettings.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"

CWebClientSettings::CWebClientSettings(
    const tstring&	sServerName,
	const tstring&	sObjectName,
	WORD			dwPort,
	const tstring&	sUserName,
	const tstring&	sPassword,	
	HWND			hWnd):
		m_sServerName(sServerName),
		m_sObjectName(sObjectName), 
		m_dwPort(dwPort),
		m_sUserName(sUserName),
		m_sPassword(sPassword),
		m_hWnd(hWnd)
{
}

CWebClientSettings::~CWebClientSettings(void)
{
TRY_CATCH
CATCH_LOG(_T("CWebClientSettings::~CWebClientSettings"))
}