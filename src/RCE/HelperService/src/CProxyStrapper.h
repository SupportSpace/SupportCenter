#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CProxyStrapper.h
///
///  Stapper for RCHost proxy application
///
///  @author "Archer Software" Sogin M. @date 02.10.2007
///
////////////////////////////////////////////////////////////////////////

#include <AidLib/CThread/CThread.h>
#include <AidLib/Logging/CInstanceTracker.h>
#include <boost/shared_ptr.hpp>
#include "CSessionEnumerator/CSessionEnumerator.h"
#include <list>

#define SCAN_TIMEOUT 1000
#define TERMINATE_PROXY_TIMEOUT 1500

/// Stapper for RCHost proxy application
/// It work as follows:
/// Each SCAN_TIMEOUT all sessions are enumerated with CSessions enumerator compatible class
/// For each new session proxy application instance is started with the same rights as service
/// For vista integrity level is elevated to highest
class CProxyStrapper : private CThread, public CInstanceTracker
{
private:
	/// sync/async flag
	bool m_sync;
	/// Stopped flag
	bool m_stopped;
	/// OS version
	int m_osVersion;

	/// Session enumerator instance
	boost::shared_ptr<CSessionEnumerator> m_sessionEnumerator;

	/// Set of known sessions
	std::set<int> m_knownSessions;

	/// Process sessions
	std::map<int, int> m_processSessions; 

	/// List of started processes for further shut down
	std::list<boost::shared_ptr<PROCESS_INFORMATION> > m_startedProcesses;

	/// Inserts to set session, which should be excluded
	void InsertExcludes();

	/// Sessions scanner method
	virtual void Execute(void *Params);

	/// Starts proxy application in session with corresponding Id
	/// @param sessionId - id of session to start application
	void StartProxy(int sessionId);

	/// Terminates process
	/// Closes handles of pi
	void  ShutdownProcess(PROCESS_INFORMATION *pi);

	/// Terminates process with proxy app (To solve vista Session Isolation issues)
	static void ShutdownThroughProxy(const int threadId, const int sessionId);
public:
	/// ctor
	CProxyStrapper();
	/// dtor
	virtual ~CProxyStrapper();

	/// Call to start proxy strapper
	/// @param sync specifyes if proxy should start in separate thread or not
	void Run(bool sync);

	/// Stops scanner
	virtual void Stop();
};
