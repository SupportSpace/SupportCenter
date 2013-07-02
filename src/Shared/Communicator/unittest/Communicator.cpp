/////////////////////////////////////////////////////////////// VVVV //////////////////////////////////////////////////
#include "stdafx.h"
#include "Communicator.h"

WNDPROC CCommunicator::m_origWndProc = NULL;
CCommunicator* CCommunicator::self = NULL;

CCommunicator::CCommunicator(){
	m_IJabberInterface = new CJabberWrapper<CCommunicator>( *this );
	self = this;
	m_webClientSettings = NULL;
}

CCommunicator::~CCommunicator(void)
{
	if( m_IJabberInterface )  
		delete m_IJabberInterface;
}

// Function returns number of requests in the  Execution queue
const size_t CCommunicator::GetExecutionQueueLoad(void)
{
	return m_ExecutionQueue.size();
}

// Qeueu one Execution Node
CExecutionQueueNode* CCommunicator::DequeExecutionQueue(void)
{
	CExecutionQueueNode* pNode = m_ExecutionQueue.front();
	m_ExecutionQueue.pop();
	return pNode;
}

// Adds New execution command
void CCommunicator::EnqueueExecutionQueue(CExecutionQueueNode* pExecutionQueue)
{
	m_ExecutionQueue.push(pExecutionQueue);
	m_IJabberInterface->SignalQueueUpdate();
}

void CCommunicator::CreateIMClient(
				   std::string			username,
				   std::string			resource,
				   std::string			password, 
				   std::string			server,
				   std::string			server_addr,
				   HWND					hWnd,
				   bool					bLog,
				   DWORD				idleTimeout)
{
	m_IJabberInterface->CreateIMClient(username,resource, password, server, server_addr, hWnd, bLog );
}

void CCommunicator::DestroyIMClient()
{
	m_IJabberInterface->DestroyIMClient();
}

void CCommunicator::CreateWebClient(
						 const tstring&	sServerName,
						 const tstring&	sObjectName,
						 WORD  dwPort,
						 const tstring&	sUserName,
						 const tstring&	sPassword,
						 HWND  hWnd)
{
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
}
	
void CCommunicator::DestroyWebClient()
{
	AtlTrace(_T("CCommunicator::DestroyWebClient() CleanWebClientHash"));
	CleanWebClientHash();

	AtlTrace(_T("CCommunicator::DestroyWebClient() delete m_webClientNode"));
	if(m_webClientSettings) 
		delete m_webClientSettings;
	//must to remove our handle 
	AtlTrace(_T("CCommunicator::DestroyWebClient() SetWindowLong"));
	if(m_origWndProc!=NULL);
	{
		(WNDPROC)SetWindowLong(m_hWebClientWnd, GWL_WNDPROC, (LONG)m_origWndProc);
		m_origWndProc = NULL;
	}
}

void CCommunicator::CleanWebClientHash()
{
	AtlTrace(_T("CCommunicator::CleanWebClientHash()"));
	CWebClientWrapper* webClientWrapper = NULL; 
	WebClientHashIterator it = m_WebClientHash.begin();
 	
	for( ; it != m_WebClientHash.end();)
    {
		AtlTrace(_T("CCommunicator::CleanWebClientHash()...delete "));
		webClientWrapper = (*it).second;
		if( webClientWrapper )
			delete webClientWrapper;
	
		it = m_WebClientHash.erase(it);
    }
}

/// SendWebClientRequest asyncron
void CCommunicator::SendWebClientRequest(const tstring&	sRequest)
{
	//
	CWebClientWrapper* webClientWrapper = new CWebClientWrapper(m_webClientSettings, sRequest);
	//	
	m_WebClientHash.insert(WebClientHash::value_type(webClientWrapper->getThreadId(), webClientWrapper));
	//
	webClientWrapper->execute();
}

// TODO
void CCommunicator::OnWebClientRequestCompleted(DWORD threadId)
{
	AtlTrace(_T("OnWebClientRequestCompleted %d"), threadId );
	WebClientHashIterator it = m_WebClientHash.find(threadId);

	if(it != m_WebClientHash.end())
	{
		AtlTrace(_T("CCommunicator::OnWebClientRequestCompleted delete pWebClientWrapper"), threadId );
		CWebClientWrapper*	pWebClientWrapper = NULL; 
		pWebClientWrapper = (*it).second;

		if(pWebClientWrapper)
			delete pWebClientWrapper;

		m_WebClientHash.erase(it);
	}
}

LRESULT CALLBACK CCommunicator::HookWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg==WM_WC_THREAD_COMPLETED)
	{
		AtlTrace(_T("CCommunicator::HookWndProc  got WM_WC_THREAD_COMPLETED") );
		self->OnWebClientRequestCompleted((DWORD)lParam);
	}

	return CallWindowProc(m_origWndProc, hWnd, uMsg, wParam, lParam);
}