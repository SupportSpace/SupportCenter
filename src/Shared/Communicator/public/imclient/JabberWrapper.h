//===========================================================================
// SupportSpace ltd. @{SRCH}
//								IJabberInterface
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

#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"
#include <AidLib/CCritSection/CCritSection.h>
#include "JabberInterface.h"
#include "IMClient.h"

#define MAX_SIMULTANEOUSLY_REQUEST 100
#define MAX_NUM_RETRIES_BEFORE_FORCE_DISCONNECT 5
#define DEFAULT_THREAD_TIMEOUT 5000

template<class T>class CJabberWrapper: public IJabberInterface<T>
{
public:
	    friend class CExecutionQueueNode;
		CJabberWrapper (T&	communicator): 
				IJabberInterface(communicator),
		m_bJabberRX_ThreadExecution(false),m_bJabberTX_ThreadExecution(false),
		m_lpThreadId_RX(0),m_lpThreadId_TX(0),m_hJabberRX(INVALID_HANDLE_VALUE),m_hJabberTX(INVALID_HANDLE_VALUE),
		m_hExecutionQueueUpdated(INVALID_HANDLE_VALUE)
		{
			m_pcIMClient = NULL;
			m_hExecutionQueueUpdated = CreateSemaphore(NULL, 0, MAX_SIMULTANEOUSLY_REQUEST, NULL);
			if (m_hExecutionQueueUpdated == INVALID_HANDLE_VALUE)
				throw "Cannot create Event for Jabber worker queue";

			Log.Add(_MESSAGE_, _T("CJabberWrapper::CreateSemaphore passed"));
			InitializeCriticalSection(&m_hCritJabber);
		}

	~CJabberWrapper(void)
	{
		m_bWrokerThreadExecution = false;
		CloseHandle(m_hExecutionQueueUpdated);

	}
public:


	// Sends signal to Jabber thread Execution queue updated
	void SignalQueueUpdate(void)
	{
		Log.Add(_CALL_, _T("CJabberWrapper::SignalQueueUpdate() before ReleaseSemaphore "));
		ReleaseSemaphore(m_hExecutionQueueUpdated, 1, NULL);
		Log.Add(_CALL_, _T("CJabberWrapper::SignalQueueUpdate() after ReleaseSemaphore "));
	}
//   CreateIMClient	init imclient data. Start Jabber_TX WriterThread
//   @param   username		The username/local part of the JID.
//   @param   resource		The resource part of the JID.
//   @param   password		The password to use for authentication.
//   @param   server		The Jabber ID'S server part and the host name to connect to. If those are different
//   @param   server_addr	ipaddress or hostname of IM server
//   @param   hWnd			notification window 
//   @param   bLog			flag to enable gloox library logging
//   @param   idleTimeout	idle timeout for sending keep alive
//   @remarks The connection not opened by this call. To open connection use 
//	 @remarks CCommunicator::EnqueueExecutionQueue with CNodeSendConnectPtr parameter  	
	void CreateIMClient(const tstring&		username,
						const tstring&		resource,
						const tstring&		password, 
						const tstring&		server,
						const tstring&		server_addr,
						HWND				hWnd,
						bool				bLog,
						DWORD				idleTimeout)
	{
		Log.Add(_MESSAGE_, _T("CJabberWrapper:CreateIMClient begin"));
		CCritSection section(&m_hCritJabber);

		TRY_CATCH

			if(m_pcIMClient != NULL)
				DestroyIMClient();

			m_hWnd = hWnd;

			//
			//
			//	Create IMClient 
			m_pcIMClient = new IMClient(username, resource, password, server, server_addr, hWnd, bLog, idleTimeout);

			//
			//
			//	Start WriterThread look for jobs in the Queue: connect,send,disconnect and send keep alive if required
			m_bJabberTX_ThreadExecution =true;
			m_dwIdleTimeout	=idleTimeout;
			
			OutputDebugString( _T("CJabberWrapper::CreateIMClient CreateThread") );
			m_hJabberTX = CreateThread( NULL,
										0,
										CJabberWrapper::Jabber_TX,
										this,
										0,
										&m_lpThreadId_TX);

			if (m_hJabberTX == INVALID_HANDLE_VALUE)
			{
				Log.Add(_ERROR_, _T("CJabberWrapper:CreateIMClient begin"));
				throw "Cannot Create Jabber Thread";
			}

		CATCH_THROW( "CJabberWrapper::Jabber_TX failed")
	}
//  DestroyIMClient. Stop JabberTX WriterThread and delete imclient object.
//	@remarks ReaderThread JabberRX is stoped by call to method CCommunicator::EnqueueExecutionQueue 
//	@remarks with CNodeSendDisconnectPtr parameter  
	void DestroyIMClient()
	{
		TRY_CATCH
	
			Log.Add(_MESSAGE_, _T("CJabberWrapper:DestroyIMClient begin"));
			//
			//
			// first give other job in jobquoe to be performed todo here to limit nimber of retries
			DWORD	dwRetry = 0;
			while(m_Communicator.GetExecutionQueueLoad()!=0 && dwRetry < MAX_NUM_RETRIES_BEFORE_FORCE_DISCONNECT)
			{
				Log.Add(_MESSAGE_, _T("CJabberWrapper:DestroyIMClient. Detected Queue not empty. Give chance to empty. Retry:%d"),dwRetry);
				Sleep(1000);
				dwRetry++;
			}
			//
			//
			//	Terminate TX thread
			m_bJabberTX_ThreadExecution = false;
			SignalQueueUpdate();
			
			DWORD res = WaitForSingleObject(m_hJabberTX,DEFAULT_THREAD_TIMEOUT);
			if (res == WAIT_TIMEOUT)
			{
				Log.Add(_ERROR_, _T("DestroyIMClient:Cannot terminate JabberTX thread"));
				DWORD	exitCode = 0;
				
				if (!GetExitCodeThread(m_hJabberTX,&exitCode)) 
					Log.Add(_ERROR_, _T("GetExitCodeThread m_hJabberTX failed"));

				if(!TerminateThread(m_hJabberTX, exitCode))//todo
					Log.Add(_ERROR_, _T("TerminateThread m_hJabberTX failed"));

				CloseHandle(m_hJabberTX);
			}
			else
			{
				//
				//	
				//	Clean up all
				CloseHandle(m_hJabberTX);

				if(m_pcIMClient != NULL) 
				{
					delete m_pcIMClient;
					m_pcIMClient = NULL;
				}
				Log.Add(_MESSAGE_, _T("CJabberWrapper:DestroyIMClient end"));
			}

		CATCH_THROW( "CJabberWrapper::DestroyIMClient")
	}
	
private:
	CJabberWrapper(CJabberWrapper& cp) {};
	CJabberWrapper() {};
	// Jabber Worker TX ThreadID
	DWORD m_lpThreadId_TX;
	// Jabber Worker RX ThreadID
	DWORD m_lpThreadId_RX;
	
	// Worker Thread handle for TX thread
	HANDLE m_hJabberTX;
	// Worker Thread handle for RX thread
	HANDLE m_hJabberRX;

	//Jabber Critical Section
	CRITICAL_SECTION	m_hCritJabber;

	// Flags the thread to run/termionate
	bool m_bJabberRX_ThreadExecution;
	// Flags the thread to run/termionate
	bool m_bJabberTX_ThreadExecution;
	// Trigers on New data in the execution queue
	HANDLE m_hExecutionQueueUpdated;
	// Send KeepAlive to server to avoid connections to be cloesd after confiured idel timeout   	
	DWORD  m_dwIdleTimeout;

	//Jabber Client Instance
	IMClient* m_pcIMClient;

	// Invoke Jabber execution
	void CJabberWrapper<T>::SendCommandToJabberServer(CExecutionQueueNode* command)
	{
		Log.Add(_MESSAGE_, _T("SendCommandToJabberServer "));
		command->PerformJob( *this);
	}

//  connect() start ReaderThread (open connection and wait on socket for data or disconnect)
//	@remarks 
	void connect()
	{
		Log.Add(_MESSAGE_, _T("CJabberWrapper::connect() begin"));
		CCritSection section(&m_hCritJabber);

		TRY_CATCH

			if (m_hJabberRX != INVALID_HANDLE_VALUE || m_pcIMClient == NULL)
			{
				OutputDebugString( _T("connect() is not possible"));

				// Thread in progress
				DWORD dThreadStatus=0;
				if (!GetExitCodeThread(m_hJabberRX,&dThreadStatus)) 
					throw "Cannot retrieve Thread execution status";

				if (dThreadStatus == STILL_ACTIVE)
				{
					//Send Error Message to Initiator
					PostMessage( m_hWnd, WM_IM_CONNECTING, 0, (LPARAM)0 ); 
					return;			
				}
				if (!CloseHandle(m_hJabberRX))
					throw "Cannot close JabberRX ReaderThread handle";
				m_hJabberRX = INVALID_HANDLE_VALUE;
			}

			m_bJabberRX_ThreadExecution = true;
			m_hJabberRX = CreateThread(NULL,
										0,
										CJabberWrapper::Jabber_RX,
										this,
										0,
										&m_lpThreadId_RX);

			if (m_hJabberRX == INVALID_HANDLE_VALUE)
				throw "Cannot Create Jabber Thread";

			Log.Add(_MESSAGE_, _T("CJabberWrapper::connect() end"));

		CATCH_THROW( "CJabberWrapper::connect")
	}

//  Disconnect Stop ReaderThreead by call to disconnect that cause select release
//	@remarks is blocked 
	void disconnect()
	{
		TRY_CATCH

			Log.Add(_MESSAGE_, _T("CJabberWrapper::disconnect() begin"));
			CCritSection section(&m_hCritJabber);

			if (m_hJabberRX == INVALID_HANDLE_VALUE || m_pcIMClient == NULL)
			{
				Log.Add(_MESSAGE_, _T("PostMessage WM_IM_NOT_CONNECTED"));
				//Send Error Message to Initiator
				PostMessage( m_hWnd, WM_IM_NOT_CONNECTED, 0, (LPARAM)0 ); 
				return;			
			}

			Log.Add(_MESSAGE_, _T("IMClient->disconnect() called"));
			m_bJabberRX_ThreadExecution = false;
			m_pcIMClient->disconnect();
			Log.Add(_MESSAGE_, _T("IMClient->disconnect() finished...RX thread will be stoped as a result"));
			
			DWORD res = WaitForSingleObject(m_hJabberRX, DEFAULT_THREAD_TIMEOUT);
			if (res == WAIT_TIMEOUT)
			{
				Log.Add(_ERROR_, _T("Cannot terminate JabberRX Reader thread"));
				//throw "Cannot terminate Jabber thread";
			}
			
			CloseHandle(m_hJabberRX);
			m_hJabberRX = INVALID_HANDLE_VALUE;

			Log.Add(_MESSAGE_, _T("CJabberWrapper::disconnect() end"));

		CATCH_THROW( "CJabberWrapper::disconnect")
	}
	void update_status(Presence  status, const tstring& msg)
	{
		TRY_CATCH
			CCritSection section(&m_hCritJabber);
			if (m_hJabberRX == INVALID_HANDLE_VALUE  || m_pcIMClient == NULL )
			{
				//Send Error Message to Initiator
				PostMessage(m_hWnd, WM_IM_NOT_CONNECTED, 0, (LPARAM)0); 
				return;			
			}

			m_pcIMClient->update_status(status, msg);
			Log.Add(_MESSAGE_, _T("update_status sent"));
		CATCH_THROW( "CJabberWrapper::update_status")

	}
	void send_msg(tstring m_sTo, tstring m_sMessageBody, tstring m_sMessageSubject, tstring m_sToResource)
	{
		TRY_CATCH
			CCritSection section(&m_hCritJabber);
			if (m_hJabberRX == INVALID_HANDLE_VALUE  || m_pcIMClient == NULL )
			{
				//Send Error Message to Initiator
				PostMessage(m_hWnd, WM_IM_NOT_CONNECTED, 0, (LPARAM)0); 
				return;			
			}
			m_pcIMClient->send_msg(m_sTo, m_sMessageBody, m_sMessageSubject, m_sToResource);
		CATCH_THROW( "CJabberWrapper::send_msg")
	}
	void send_keepalive()
	{
		TRY_CATCH
			CCritSection section(&m_hCritJabber);
			if (m_hJabberRX == INVALID_HANDLE_VALUE  || m_pcIMClient == NULL )
			{
				//Send Error Message to Initiator
				Log.Add(_MESSAGE_, _T("CJabberWrapper::send_keepalive may not be sent - not connected"));
				PostMessage(m_hWnd, WM_IM_NOT_CONNECTED, 0, (LPARAM)0); 
				return;			
			}
			m_pcIMClient->ping();
		CATCH_THROW( "CJabberWrapper::ping")

	}

	// Jabber Client Execution Function		"ReaderThread"
	static DWORD  WINAPI Jabber_RX(LPVOID lpParameter)
	{
		TRY_CATCH

			OutputDebugString( _T("Jabber_RX::Starting Jabber RX thread"));
		    Log.Add(_MESSAGE_, _T("Jabber_RX::Starting Jabber RX thread"));
			CJabberWrapper* pJabberWrapper = reinterpret_cast<CJabberWrapper*>(lpParameter);
			ConnectionError	err = ConnNoError;

			if(pJabberWrapper->m_pcIMClient == NULL){
				PostMessage(pJabberWrapper->m_hWnd, WM_IM_NOT_CONNECTED, 0, (LPARAM)0); 
				Log.Add(_MESSAGE_, _T("Jabber_RX:: connection is not possible"));
				OutputDebugString( _T("Jabber_RX:: connection is not possible"));
				return 0;
			}

			PostMessage(pJabberWrapper->m_hWnd, WM_IM_CONNECTING, 0, (LPARAM)0 ); 
		
			if(pJabberWrapper->m_pcIMClient->connect(true) == true)//blocked called with true
			{
				Log.Add(_MESSAGE_, _T("Jabber_RX::pJabberWrapper->m_pcIMClient->connect == true"));		
			}
			else
			{
				Log.Add(_MESSAGE_, _T("Jabber_RX::pJabberWrapper->m_pcIMClient->connect == false"));
				//
				//	gloox may always call IsOnDisconnectCalled and then blocked connect() may return 
				//  in one case IsOnDisconnectCalled is not called
				//  this happenes when computer is restarted and messnger is launched automatically
				//  till issue will be closed in gloox the workaround is to send WM_IM_DISCONNECT here
				if(pJabberWrapper->m_pcIMClient->IsOnDisconnectCalled() == FALSE)
				{
					pJabberWrapper->m_pcIMClient->setDisconnectedState(TRUE);
					Log.Add(_MESSAGE_, _T("OnDisconnect not called by gloox. Use workaround till fixed"));
					PostMessage(pJabberWrapper->m_hWnd, WM_IM_DISCONNECT, (WPARAM)StreamErrorUndefined, (LPARAM)ConnConnectionRefused);
				}
			}
		
			Log.Add(_MESSAGE_, _T("Jabber_RX:: Jabber_RX thread ended"));

		CATCH_THROW ( "Jabber_RX: CJabberWrapper::Jabber_RX thread")
		return 0;
	}

	// Jabber Client Execution Function Transmit	"WriterThread"
	static DWORD  WINAPI Jabber_TX(LPVOID lpParameter)
	{
		try {
			Log.Add(_MESSAGE_, _T("Jabber_TX:Starting Jabber TX thread "));
			
			CJabberWrapper* pJabberWrapper = reinterpret_cast<CJabberWrapper*>(lpParameter);
			DWORD dwWaitResult = 0; 
			//
			//
			//	The main thread activity loop
			while(pJabberWrapper->m_bJabberTX_ThreadExecution)
			{
				dwWaitResult = WaitForSingleObject( pJabberWrapper->m_hExecutionQueueUpdated , 
													pJabberWrapper->m_dwIdleTimeout*1000);
		
				if(pJabberWrapper->m_bJabberTX_ThreadExecution == false)
				{
					Log.Add(_MESSAGE_, _T("Jabber_TX: pJabberWrapper->m_bJabberTX_ThreadExecution == false"));
					break;
				}
		
				switch (dwWaitResult) 
				{ 
				case WAIT_OBJECT_0: 
					Log.Add(_CALL_, _T("Jabber_TX:The semaphore object was signaled"));
					break; 
				case WAIT_TIMEOUT: 
					//todo check if connection opened - disconnect() was not called
					if(pJabberWrapper && pJabberWrapper->m_bJabberRX_ThreadExecution==TRUE && 
						pJabberWrapper->m_pcIMClient->IsConnected()==TRUE)
					{
						//todo add severity log
						Log.Add(_CALL_, _T("Jabber_TX: Semaphore was nonsignaled, so a time-out occurred. Send keep alive"));
						pJabberWrapper->send_keepalive();
					}
					break; //to avoid using of continue 
				}
				
				if(dwWaitResult==WAIT_TIMEOUT)
					continue;

				while(pJabberWrapper->m_Communicator.GetExecutionQueueLoad())
				{
					Log.Add(_CALL_, _T("Jabber_TX: Process next Job from Queue"));
					CExecutionQueueNode* command = pJabberWrapper->m_Communicator.DequeExecutionQueue();
					pJabberWrapper->SendCommandToJabberServer(command);
					if(command != NULL)
					{
						Log.Add(_CALL_, _T("Jabber_TX: delete command"));
						delete command;
						command = NULL;
					}
				}

				Log.Add(_CALL_, _T("Jabber_TX: No more jobs in the Queue"));
			}

		} catch(...)
		{
			Log.Add(_MESSAGE_, _T("Jabber_TX: Exception. Jabber Thread execution terminated "));
		}
		
		Log.Add(_MESSAGE_, _T("Jabber TX thread ended"));
		return 0;
	}
};