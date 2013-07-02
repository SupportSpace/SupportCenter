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
#pragma once

#include <Aidlib\Strings\tstring.h>

class CWebClientSettings
{
public:
	CWebClientSettings( const tstring&	sServerName,
						const tstring&	sObjectName,
						WORD			dwPort,
						const tstring&	sUserName,
						const tstring&	sPassword,
						HWND			hWnd);

	CWebClientSettings(const CWebClientSettings& wenClientNode)
	{
		
	};
	CWebClientSettings& operator =(const CWebClientSettings& wenClientNode);
public:
	~CWebClientSettings(void);

	tstring getServerName() const {return m_sServerName;};
	tstring getObjectName() const { return m_sObjectName;};
	WORD    getPort() const { return m_dwPort;};
	tstring getUserName() const { return m_sUserName;};
	tstring getPassword() const { return m_sPassword;};
	HWND	getHwnd() const { return m_hWnd;};

private:
	tstring			m_sServerName;
	tstring			m_sObjectName;
	WORD			m_dwPort;
	tstring			m_sUserName;
	tstring			m_sPassword;	
	HWND			m_hWnd;
};
