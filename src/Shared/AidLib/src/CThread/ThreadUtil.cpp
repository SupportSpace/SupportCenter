/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  ThreadUtil.cpp
///
///  Different thread utils
///
///  @author "Archer Software" Sogin M. @date 02.11.2007
///
////////////////////////////////////////////////////////////////////////

#include <AidLib/CThread/ThreadUtil.h>
#include <AidLib/CException/CException.h>
#include <AidLib/Logging/cLog.h>
#include <AidLib/CScopedTracker/CScopedTracker.h>
#include <Tlhelp32.h>

AIDLIB_API std::set<DWORD> GetProcessThreads(const DWORD processId)
{
TRY_CATCH
	std::set<DWORD> threads;
	CScopedTracker<HANDLE> threadSnap;

	// Retriving threads snapshot
	threadSnap.reset(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, processId),CloseHandle);
	Log.Add(_MESSAGE_,_T("Retriving threads for process(%d), with handle(%d)"), processId, threadSnap.get());
	if (INVALID_HANDLE_VALUE == threadSnap)
		throw MCException_Win("Failed to CreateToolhelp32Snapshot");

	// Enumerating threads
	THREADENTRY32 threadEntry = {0};
	threadEntry.dwSize = sizeof(THREADENTRY32);
	if(Thread32First(threadSnap, &threadEntry))
	{
		do 
		{
			if (processId == threadEntry.th32OwnerProcessID)
				threads.insert(threadEntry.th32ThreadID);
			threadEntry.dwSize = sizeof(THREADENTRY32);
			threadEntry.th32ThreadID = 0;
		}
		while(Thread32Next(threadSnap, &threadEntry));
	} else
		throw MCException_Win("Failed to Thread32First");
	return threads;

CATCH_THROW()
}

