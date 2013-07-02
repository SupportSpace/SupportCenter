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
#pragma once

#include "CommunicatorDef.h"
#include "imclient/ExecutionQueueNode.h"
#include "imclient/JabberWrapper.h"
#include "webclient/WebClientWrapper.h"

#include <queue>

using std::queue;

typedef std::map<DWORD, CWebClientWrapper*> WebClientHash;
typedef std::map<DWORD, CWebClientWrapper*>::iterator WebClientHashIterator;

class COMMUNICATOR_API CCommunicator
{
public:
	CCommunicator();

	~CCommunicator(void);
public:
///  CreateIMClient	init data required to communication with IM Server 
///  @param   username		The username/local part of the JID.
///  @param   resource		The resource part of the JID.
///  @param   password		The password to use for authentication.
///  @param   server		The Jabber ID'S server part and the host name to connect to. If those are different
///  @param   server_addr	ipaddress or hostname of IM server
///  @param   hWnd			notification window 
///  @param   bLog			flag to enable gloox library logging
///  @param   idleTimeout	idle timeout for sending keep alive
///  @remarks The connection not opened by this call. To open connection use 
//	 @remarks CCommunicator::EnqueueExecutionQueue with CNodeSendConnectPtr parameter  	
//   @remarks Communicator will send messaages using hWnd. Seee messages prefixed with WM_IM_
	void CreateIMClient( const tstring& username,
						 const tstring& resource, 
						 const tstring& password, 
						 const tstring& server,
						 const tstring& server_addr,
						 HWND  hWnd, 
						 bool  bLog, 
						 DWORD idleTimeout);

///  DestroyIMClient
	void DestroyIMClient();
	
// Function returns number of requests in the  Execution queue
	const size_t GetExecutionQueueLoad(void);
// Qeueu one Execution Node
	CExecutionQueueNode* DequeExecutionQueue(void);
///  EnqueueExecutionQueue			Adds New execution command 
///  @param   nExecutionQueue		node will be added to Queue
///  @remarks   see classed derived from CExecutionQueueNode
	void EnqueueExecutionQueue(CExecutionQueueNode* nExecutionQueue);


///  CreateWebClient init data required to communication with WebService.  
///  @param   sServerName	The host name of an WebService, alternately,the string can contain the IP number 
///  of the site, in ASCII dotted-decimal format (for example, 11.0.1.45)
///  @param   sObjectName	
///  @param   dwPort		Port on WebService
///  @param   sUserName		Contains the name of the user to log on. 
///  @param   sPassword		Contains the password to use to log on.
///  @param   hWnd			hadle of notification window
///  @remarks The connection not opened by this call. Use SendWebClientRequest to connect and send.
///	 @remarks DestroyWebClient may be called for correspondent call to CreateWebClient
	void CreateWebClient(const tstring&	sServerName,
						 const tstring&	sObjectName,
						 WORD dwPort,
						 const tstring&	sUserName,
						 const tstring&	sPassword,
						 HWND  hWnd);
///  DestroyWebClient 
///	 @remarks 
	void DestroyWebClient();

///  SendWebClientRequest is asynchronous HTTPS reuqest.
///  @param    sRequest	- request string may be ready for sending string for example in xml soap format 
///	 @remarks  The calling process may handle the messages send by the communicator (WebClientWrapper)
//	 @remarks  for example WM_WEBCLIENT_RESPONSE 
	void SendWebClientRequest(const tstring& sRequest);

private:
	// Properly close 
	void   OnWebClientRequestCompleted(DWORD threadId);
	// CCommunicator will hook WebClientThreadCompleted 
	static LRESULT CALLBACK HookWndProc(HWND hwnd,UINT uMsg, WPARAM wParam, LPARAM lParam);
	//	delete all from WebClientHash
	void CleanWebClientHash();

private:
	// Jabber thread execution queue
	queue<CExecutionQueueNode*> m_ExecutionQueue;
	// Map of threads for monitoring 
	WebClientHash m_WebClientHash;
	// Jabber execution thread
	IJabberInterface<CCommunicator> *m_IJabberInterface;
	// Original windows procedure
	static WNDPROC   m_origWndProc;
	// Handle to notification window
	HWND m_hWebClientWnd;
	// Communicator static 
	static CCommunicator* self;
	//  
	CWebClientSettings*   m_webClientSettings;	
};