//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CCommunicator
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CCommunicator :  class
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Alex Gantman
// Date    : 10/02/2007 05:21:10 PM
// Comments: First Issue
// Modified by: Anatoly Gutnick	
//===========================================================================
#include "stdafx.h"
#include "../public/Communicator.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"

WNDPROC CCommunicator::m_origWndProc = NULL;
CCommunicator* CCommunicator::self = NULL;

CCommunicator::CCommunicator(){
TRY_CATCH

	m_IJabberInterface = NULL;
	m_webClientSettings = NULL;
	self = this;

CATCH_THROW(_T("CCommunicator::CCommunicator"))
}

CCommunicator::~CCommunicator(void)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CCommunicator::~CCommunicator"));
	DestroyIMClient();
	DestroyWebClient();
	
CATCH_LOG(_T("CCommunicator::~CCommunicator"))
}

// Function returns number of requests in the  Execution queue
const size_t CCommunicator::GetExecutionQueueLoad(void)
{
TRY_CATCH

	return m_ExecutionQueue.size();

CATCH_THROW(_T("CCommunicator::GetExecutionQueueLoad"))
}

// Qeueu one Execution Node
CExecutionQueueNode* CCommunicator::DequeExecutionQueue(void)
{
	CExecutionQueueNode* pNode = NULL;
TRY_CATCH

	pNode = m_ExecutionQueue.front();
	m_ExecutionQueue.pop();

CATCH_THROW(_T("CCommunicator::DequeExecutionQueue"))
	return pNode;
}

// Adds New execution command
void CCommunicator::EnqueueExecutionQueue(CExecutionQueueNode* pExecutionQueue)
{
TRY_CATCH

	m_ExecutionQueue.push(pExecutionQueue);
	if(m_IJabberInterface!=NULL)
		m_IJabberInterface->SignalQueueUpdate();

CATCH_THROW(_T("CCommunicator::EnqueueExecutionQueue"))
}
void CCommunicator::CreateIMClient(
				   const tstring&			username,
				   const tstring&			resource,
				   const tstring&			password, 
				   const tstring&			server,
				   const tstring&			server_addr,
				   HWND						hWnd,
				   bool						bLog,
				   DWORD					idleTimeout)
{
TRY_CATCH

	m_IJabberInterface = new CJabberWrapper<CCommunicator>(*this);
	if(m_IJabberInterface)
		m_IJabberInterface->CreateIMClient(username, resource, password, server, server_addr, hWnd, bLog, idleTimeout);

CATCH_THROW(_T("CCommunicator::CreateIMClient"))
}

void CCommunicator::DestroyIMClient()
{
TRY_CATCH

	if(m_IJabberInterface) 
	{
		m_IJabberInterface->DestroyIMClient();
		delete m_IJabberInterface;
		m_IJabberInterface = NULL;
	}

CATCH_THROW(_T("CCommunicator::DestroyIMClient"))
}

void CCommunicator::CreateWebClient(
						 const tstring&	sServerName,
						 const tstring&	sObjectName,
						 WORD  dwPort,
						 const tstring&	sUserName,
						 const tstring&	sPassword,
						 HWND  hWnd)
{
TRY_CATCH
	//todo to define what to return if already connected
	if(m_webClientSettings == NULL)
	{
		m_webClientSettings	= new CWebClientSettings( sServerName,
													  sObjectName,
													  dwPort,
													  sUserName,
													  sPassword,
													  hWnd);
		m_hWebClientWnd = hWnd;
		//CCommunicator is responsoble to free resource correct
		//to handle WM_WC_THREAD_COMPLETED message it uses notification window of calling thread
		m_origWndProc = (WNDPROC)SetWindowLong(m_hWebClientWnd, GWL_WNDPROC, (LONG)CCommunicator::HookWndProc);
	}

CATCH_THROW(_T("CCommunicator::CreateWebClient"))
}
	
void CCommunicator::DestroyWebClient()
{
TRY_CATCH

	if(m_webClientSettings) 
	{
		//Log.Add(_T("CCommunicator::DestroyWebClient() delete m_webClientNode"));
		delete m_webClientSettings;
		m_webClientSettings = NULL;
		CleanWebClientHash();
	}

	Log.Add(_T("CCommunicator::DestroyWebClient() SetWindowLong"));
	if(m_origWndProc!=NULL)
	{
		SetWindowLong(m_hWebClientWnd, GWL_WNDPROC, (LONG)m_origWndProc);
		m_origWndProc = NULL;
	}

CATCH_THROW(_T("CCommunicator::DestroyWebClient"))
}

void CCommunicator::CleanWebClientHash()
{
TRY_CATCH

	CWebClientWrapper* webClientWrapper = NULL; 
	WebClientHashIterator it = m_WebClientHash.begin();
 	
	for( ; it != m_WebClientHash.end();)
    {
		Log.Add(_T("CCommunicator::CleanWebClientHash()...delete "));
		webClientWrapper = (*it).second;
		if(webClientWrapper)
			delete webClientWrapper;
	
		it = m_WebClientHash.erase(it);
    }

CATCH_THROW(_T("CCommunicator::CleanWebClientHash"))
}

/// SendWebClientRequest asyncron
void CCommunicator::SendWebClientRequest(const tstring&	sRequest)
{
TRY_CATCH
	//
	CWebClientWrapper* webClientWrapper = new CWebClientWrapper(m_webClientSettings, sRequest);
	//	
	m_WebClientHash.insert(WebClientHash::value_type(webClientWrapper->getThreadId(), webClientWrapper));
	//
	webClientWrapper->execute();

CATCH_THROW(_T("CCommunicator::SendWebClientRequest"))
}

// TODO
void CCommunicator::OnWebClientRequestCompleted(DWORD threadId)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("OnWebClientRequestCompleted %d"), threadId );
	WebClientHashIterator it = m_WebClientHash.find(threadId);

	if(it != m_WebClientHash.end())
	{
		Log.Add(_MESSAGE_,_T("CCommunicator::OnWebClientRequestCompleted delete pWebClientWrapper"), threadId );
		CWebClientWrapper*	pWebClientWrapper = NULL; 
		pWebClientWrapper = (*it).second;

		if(pWebClientWrapper)
			delete pWebClientWrapper;

		m_WebClientHash.erase(it);
	}

CATCH_THROW(_T("CCommunicator::OnWebClientRequestCompleted"))
}

LRESULT CALLBACK CCommunicator::HookWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg==WM_WC_THREAD_COMPLETED)
	{
		Log.Add(_T("CCommunicator::HookWndProc  got WM_WC_THREAD_COMPLETED") );
		self->OnWebClientRequestCompleted((DWORD)lParam);
	}

	return CallWindowProc(m_origWndProc, hWnd, uMsg, wParam, lParam);
}