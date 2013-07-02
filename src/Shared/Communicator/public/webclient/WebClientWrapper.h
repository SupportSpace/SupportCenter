//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CWebClientWrapper
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CExecutionQueueNode :  class
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Alex Gantman
// Date    : 10/02/2007 05:21:10 PM
// Comments: First Issue
// Modified by: Anatoly Gutnick	
//===========================================================================
#pragma once

#include "WebClientSettings.h"
#include "WebServiceClient.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"


#define WM_WC_THREAD_COMPLETED 2000
#define WM_WC_RESPONSE		   2001
//#define DEFAULT_THREAD_TIMEOUT 5000

class CWebClientWrapper
{
public:
	CWebClientWrapper(CWebClientSettings* sWebClientNode,const tstring&	sRequest):
	  m_lpThreadId(0), m_pWebClientSettings(sWebClientNode),m_sRequest(sRequest),
	  m_hWebClientWorker(INVALID_HANDLE_VALUE)
	{
TRY_CATCH

		m_hWebClientWorker = CreateThread(NULL,
										 0,
										 WebClientThreadProc,
										 this,
										 CREATE_SUSPENDED ,
										 &m_lpThreadId);

		if (m_hWebClientWorker == INVALID_HANDLE_VALUE)
			throw "Cannot Create Jabber Thread";

CATCH_THROW(_T("CWebClientWrapper::CWebClientWrapper()"))

	}

	~CWebClientWrapper(void)
	{
TRY_CATCH

		if (m_hWebClientWorker != NULL)
		{
			DWORD res = WaitForSingleObject(m_hWebClientWorker,DEFAULT_THREAD_TIMEOUT );
			if (res == WAIT_TIMEOUT)
				throw "Cannot terminate Web Client thread";
		}

		CloseHandle( m_hWebClientWorker );

CATCH_LOG(_T("CWebClientWrapper::~CWebClientWrapper()"))

		//if( m_pWebClientSettings != NULL )
		//	delete m_pWebClientSettings;
	}

	DWORD getThreadId()
	{
		return m_lpThreadId;
	}
	void execute()
	{
		ResumeThread(m_hWebClientWorker);
	}
	void suspend()
	{
		SuspendThread(m_hWebClientWorker);
	}

	tstring const getRequest()
	{
		return m_sRequest;
	}

	


private:
	//Handle of current worker thread
	HANDLE				m_hWebClientWorker;
	//Worker thread Id
	DWORD				m_lpThreadId;
	//Web Server request Data
	CWebClientSettings*	m_pWebClientSettings;
	//
	tstring				m_sRequest;
	
	// Web Client Execution Function
	static DWORD  WINAPI WebClientThreadProc(LPVOID lpParameter)
	{
		CWebClientWrapper*  pWebClientWrapper = NULL;
		DWORD				threadId = 0;
		HWND				hWnd = 0;

TRY_CATCH
		//Doing all the web staff
		pWebClientWrapper= reinterpret_cast<CWebClientWrapper*>(lpParameter);
		threadId = pWebClientWrapper->getThreadId();
		hWnd = pWebClientWrapper->m_pWebClientSettings->getHwnd();

		CWebServiceClient client(
			pWebClientWrapper->m_pWebClientSettings->getServerName(),
			pWebClientWrapper->m_pWebClientSettings->getPort(),
			pWebClientWrapper->m_pWebClientSettings->getObjectName(),
			pWebClientWrapper->m_pWebClientSettings->getUserName(),
			pWebClientWrapper->m_pWebClientSettings->getPassword() );

		do
		{
			tstring		sSoapMessageXML = pWebClientWrapper->getRequest().c_str();

			if ( !client.ConnectToHttpsServer() )
			{
				Log.Add(_MESSAGE_, _T("BuildSoapRequest failed Text:%s"), client.GetLastErrorString() );
				Log.Add(_MESSAGE_, _T("BuildSoapRequest failed Code:%d"), client.GetLastErrorCode() );
				break;
			}

			if ( !client.SendHttpsRequest( sSoapMessageXML, sSoapMessageXML.length() ))
			{
				Log.Add(_MESSAGE_, _T("SendHttpsRequest failed Text: [%s]"), client.GetLastErrorString() );
				Log.Add(_MESSAGE_, _T("SendHttpsRequest failed Code: [%d]"), client.GetLastErrorCode() );
				break;
			}

			tstring response = client.GetRequestResult();
			Log.Add( _MESSAGE_, _T("response text: %s"), response.c_str() );

			//PostMessage(hWnd, WM_WC_RESPONSE, 0, (LPARAM)pResponseObject); 
		}while(false);

CATCH_LOG(_T("CWebClientWrapper::~WebClientThreadProc()"))

		Log.Add(_MESSAGE_, _T("PostMessage WM_WC_THREAD_COMPLETED"));
		PostMessage(hWnd,WM_WC_THREAD_COMPLETED,0,(LPARAM)threadId); 
		return 0;
	}
};
