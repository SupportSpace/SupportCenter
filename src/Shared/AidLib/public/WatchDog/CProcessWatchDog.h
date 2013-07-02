/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CProcessWatchDog.h
///
///  Watch dog primitive, watching for processes instances
///
///  @author "Archer Software" Sogin M., Dmitry N. @date 30.10.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include <AidLib/CThread/CThread.h>
#include <AidLib/CThread/CThreadLS.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <AidLib/CCritSection/CCritSection.h>
#include <AidLib/CCritSection/CCritSectionObject.h>
#include <set>

/// breakdown timeout for waiting unblock
#define PROCESS_WATCHDOG_WAIT_TIMEOUT 10000
#define TERMINATION_TIMEOUT 7000

/// Watch dog primitive, watching for processes instances
/// As far as no wathing instances left - calls corresponding handler
class CProcessWatchDog : public CThread
{
protected:

	typedef boost::shared_ptr<boost::remove_pointer<HANDLE>::type> SPHANDLE;
	/// Type containing both pid and handle
	typedef std::pair<SPHANDLE,int> HandlePidPair;
	/// function, called, when all added clients are dead
	typedef boost::function<void()> AllClientsDeadHandler;

	/// Less impl for pair of pid/handle
	struct SPidCompare : public std::less<HandlePidPair>
	{
		bool operator()(const HandlePidPair& _Left, const HandlePidPair& _Right) const
		{	// apply operator< to operands
			return (_Left.second < _Right.second);
		}
	};

	/// Watching process instances
	std::set<HandlePidPair, SPidCompare> m_clients;
	/// Event for wait function unblocking
	SPHANDLE m_unblockEvent;
	CCritSectionSimpleObject m_criticalSection;

	/// function, called, when all added clients are dead
	AllClientsDeadHandler m_allClientsDeadHandler;
public:

	/// ctor
	/// @param allClientsDeadHandler function, called, when all added clients are dead
	CProcessWatchDog(AllClientsDeadHandler allClientsDeadHandler)
		: m_allClientsDeadHandler(allClientsDeadHandler)
	{
	TRY_CATCH
		m_unblockEvent.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
		if (NULL == m_unblockEvent.get())
			throw MCException_Win("Failed to CreateEvent ");
	CATCH_LOG()
	}

	virtual ~CProcessWatchDog()
	{
	TRY_CATCH
		Terminate();
		SetEvent(m_unblockEvent.get());
		if (WAIT_OBJECT_0 != WaitForSingleObject(hTerminatedEvent.get(), PROCESS_WATCHDOG_WAIT_TIMEOUT * 2))
		{
			Log.WinError(_WARNING_,_T("Wait for internal thread failed. Terminating thread forcedly "));
			TerminateThread(OpenThread(THREAD_TERMINATE,FALSE,GetTid()),0); //TODO: switch to forced stop
		}
	CATCH_LOG()
	}

	/// Add client process to watching
	/// @param pid watching process id
	void AddClient(const int pid)
	{
	TRY_CATCH
		CCritSection cs(&m_criticalSection);
		HANDLE process;
		SPHANDLE client;
		// Try to open process handle
		if (NULL == (process=OpenProcess(SYNCHRONIZE, FALSE, pid)))
			throw MCException_Win("Failed to OpenProcess ");
		client.reset(process, CloseHandle);
		if (m_clients.insert(HandlePidPair(client,pid)).second)
		{
			Log.Add(_MESSAGE_,_T("Added client pid(%d) handle(%d) to WatchDog"),pid,process);

			// Starting if wasn't started before
			if (Terminated())
				Start();

			if (!SetEvent(m_unblockEvent.get()))
				Log.WinError(_WARNING_,_T("Failed to SetEvent "));
		}

	CATCH_THROW()
	}

	/// Watching thread entry point
	virtual void Execute(void*)
	{
	TRY_CATCH
		SET_THREAD_LS;
		Log.Add(_MESSAGE_,_T("Process WatchDog thread started"));

		while(!Terminated())
		{
			CCritSection cs(&m_criticalSection);
			// Checking if we should self terminate
			if (m_clients.empty())
			{
				Log.Add(_MESSAGE_,_T("CProcessWatchDog: no more alive clients, calling watchdog handler"));
				m_allClientsDeadHandler();
				/// Stopping thread
				Terminate();
				continue;
			}

			// Preparing array of handles for wait
			boost::scoped_array<HANDLE> waitingHandles;
			DWORD count = static_cast<DWORD>(m_clients.size()) + 1;
			waitingHandles.reset(new HANDLE[count]);
			waitingHandles[0] = m_unblockEvent.get();
			int i=1;
			for(std::set<HandlePidPair, SPidCompare>::iterator client = m_clients.begin();
				client != m_clients.end();
				++client, ++i)
			{
				waitingHandles[i] = client->first.get();
			}
			cs.Unlock();

			/// Waiting for something happened
			DWORD result = WaitForMultipleObjects(count, waitingHandles.get(), FALSE, PROCESS_WATCHDOG_WAIT_TIMEOUT);
			switch(result)
			{
				case WAIT_FAILED:
					throw MCException_Win("Failed to WaitForMultipleObjects. Watchdog isn't functional. ");
				case WAIT_OBJECT_0:
				case WAIT_TIMEOUT:
					continue;
				default:
					if (result-WAIT_OBJECT_0 < count && result-WAIT_OBJECT_0 >= 0)
					{
						HANDLE deadProcess = waitingHandles[result-WAIT_OBJECT_0];
						cs.Lock();
						for(std::set<HandlePidPair, SPidCompare>::iterator client = m_clients.begin();
							client != m_clients.end();
							++client, ++i)
						{
							if (client->first.get() == deadProcess)
							{
								Log.Add(_MESSAGE_,_T("Watching process pid(%d) handle(%d) dead."),client->second, client->first.get());
								m_clients.erase(client);
								break;
							}
						}
					} else
					{
						Log.WinError(_WARNING_,_T("Unexpected result %d is received from WaitForMultipleObjects "),result);
					}
			}

		}
	CATCH_LOG()
	}
};

inline void SelfTerminatorThreadEntryPoint(void*)
{
#ifdef _DEBUG
	//::MessageBox(NULL,_T("SelfTerminatorThreadEntryPoint(void*)"),NULL,0);
#endif
	Sleep(TERMINATION_TIMEOUT);
#ifdef _DEBUG
	//::MessageBox(NULL,_T("SelfTerminatorThreadEntryPoint(void*) Sleep()"),NULL,0);
	Beep(1000,2000);
#endif
	TerminateProcess(GetCurrentProcess(),0);
	throw MCException_Win("Failed to terminate current process");
}

/// Watch dog primitive, watching for processes instances
/// As far as no wathing instances left - terminates its own process
class CSuicideProcessWatchDog : public CProcessWatchDog
{
	static void TerminateCurrentProcess()
	{
	TRY_CATCH
#ifdef _DEBUG
		//::MessageBox(NULL,_T("TerminateCurrentProcess()"),NULL,0);
#endif
		_beginthread(SelfTerminatorThreadEntryPoint,0,0);
#ifdef _DEBUG
		//::MessageBox(NULL,_T("TerminateCurrentProcess() after begin thread"),NULL,0);
#endif
	CATCH_LOG()
	}
public:
	/// ctor
	CSuicideProcessWatchDog()
		: CProcessWatchDog(&CSuicideProcessWatchDog::TerminateCurrentProcess)
	{
	TRY_CATCH
	CATCH_THROW()
	}
};
