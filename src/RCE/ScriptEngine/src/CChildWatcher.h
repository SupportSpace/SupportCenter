/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CChildWatcher.h
///
///  Declares CChildWatcher class, responsible for terminating child processes
///
///  @author Dmitry Netrebenko @date 16.04.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/CThread/CThread.h>
#include <set>

#define TERMINATE_WAIT_TIMEOUT	100
#define TERMINATE_WAIT_STEP		10
#define TERMINATE_TRIES			30

/// CChildWatcher class, responsible for terminating child processes
/// Base class - CThread
class CChildWatcher
	:	public CThread
{
private:
/// Prevents making copies of CChildWatcher objects.
	CChildWatcher( const CChildWatcher& );
	CChildWatcher& operator=( const CChildWatcher& );
public:
/// Constructor
	CChildWatcher();
/// Destructor
	~CChildWatcher();
/// Thread entry point
	virtual void Execute(void *Params);
/// Sets termination flag
	void EnableTermination(const bool enabled);
/// Adds process id to "exceptions" set
	void AddExceptionProcess(const DWORD processId);
private:
/// Termination flag
	bool m_terminationEnabled;
/// Start flag
	bool m_started;
/// Set of process ids which should continue to work at stopping
	std::set<DWORD>	m_exceptionProcesses;
};
