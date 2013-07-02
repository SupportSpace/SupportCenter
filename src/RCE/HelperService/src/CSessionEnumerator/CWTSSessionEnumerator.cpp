/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CWTSSessionEnumerator.cpp
///
///  Class, enumerating sessions through Terminal Services
///
///  @author "Archer Software" Sogin M. @date 02.10.2007
///
////////////////////////////////////////////////////////////////////////
#include "CWTSSessionEnumerator.h"
#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/shared_ptr.hpp>
#include <WtsApi32.h>

#pragma comment(lib,"WtsApi32.lib")

void CWTSSessionEnumerator::GetSessions(std::set<int>& sessions)
{
TRY_CATCH

	if (!sessions.empty())
		sessions.clear();

	PWTS_SESSION_INFO psessionInfo;
	DWORD count;
	if (0 == WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &psessionInfo, &count))
		throw MCException_Win("Failed to WTSEnumerateProcesses");

	boost::shared_ptr<WTS_SESSION_INFO> spSessionInfo(psessionInfo, WTSFreeMemory);
	for(DWORD i=0; i<count; ++i,++psessionInfo)
	{
		if (psessionInfo->State == WTSActive)
			sessions.insert(psessionInfo->SessionId);
	}
	
CATCH_THROW()
}
