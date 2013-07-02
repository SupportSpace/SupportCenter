/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CProxyStrapper.cpp
///
///  Stapper for RCHost proxy application
///
///  @author "Archer Software" Sogin M. @date 02.10.2007
///
////////////////////////////////////////////////////////////////////////
#include "CProxyStrapper.h"
#include "CSessionEnumerator/CWTSSessionEnumerator.h"
#include "CSessionEnumerator/CNTDLLSessionEnumerator.h"
#include <Sddl.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/bind.hpp>
#include <algorithm>

CProxyStrapper::CProxyStrapper()
	:	m_sync(true), 
		m_stopped(true), 
		CThread(),
		CInstanceTracker(_T("CProxyStrapper"))
{
TRY_CATCH
	OSVERSIONINFOEX osInfo;
	osInfo.dwOSVersionInfoSize=sizeof(osInfo);
	if(0 == GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&osInfo)))
	{
		m_osVersion = 5; //WinXP by default
		Log.WinError(_ERROR_,_T("Failed to GetVersionEx"));
	} else
	{
		m_osVersion = osInfo.dwMajorVersion;
	}
	InsertExcludes();

	// Creating session enumerator
	m_sessionEnumerator.reset(new CWTSSessionEnumerator());
	//m_sessionEnumerator.reset(new CNTDLLSessionEnumerator());

CATCH_THROW()
}

void CProxyStrapper::InsertExcludes()
{
TRY_CATCH
	m_knownSessions.insert(0x10000);  // RDP-Tcp / EH-Tcp
	if (m_osVersion > 5) //Vista and higher
	{
		m_knownSessions.insert(0);			// Services
		m_knownSessions.insert(0x10001);	// RDP-Tcp on vista
	}
CATCH_THROW()
}

CProxyStrapper::~CProxyStrapper()
{
TRY_CATCH
	if ( !m_sync )
	{
		Stop();
		CThread::Stop(false,INFINITE);
	}
CATCH_LOG()
}

void CProxyStrapper::Stop()
{
TRY_CATCH
	if (m_sync)
		m_stopped = true;
	else
		CThread::Terminate();
CATCH_THROW()
}

void CProxyStrapper::Run(bool sync)
{
TRY_CATCH
	m_sync = sync;
	m_stopped = false;
	if (m_sync)
		Execute(NULL);
	else
		Start();
CATCH_THROW()
}

void CProxyStrapper::Execute(void *Params)
{
TRY_CATCH

	while( (m_sync && !m_stopped) || (!m_sync && !Terminated()) )
	{
		/// Retriving sessions set
		std::set<int> allSessions;
		TRY_CATCH
			/// We should fall back in case GetSessions failed
			/// Reason is following - during system start our service could start
			/// before TerminalService, so several times GetSessions could fail before normal work
			/// Furthermore we could solve this issue with service dependencies, but our serive should
			/// work even without terminal serive, so we cannot use such opportunity
			/// Even more, started terminal server is not guarantee, see http://forums.microsoft.com/MSDN/ShowPost.aspx?PostID=854861&SiteID=1
			m_sessionEnumerator->GetSessions(allSessions);
		CATCH_LOG()
		/// Searching for new sessions
		std::set<int> diff;
		std::set_difference(	allSessions.begin(),
								allSessions.end(), 
								m_knownSessions.begin(),
								m_knownSessions.end(),
								std::insert_iterator<std::set<int> >(diff, diff.begin()));
		std::set<int> pendingSessions; /// Session, injection to which failed
		for(std::set<int>::const_iterator sessionId = diff.begin();
			sessionId != diff.end();
			++sessionId)
		{
			try
			{
				StartProxy(*sessionId);
			}
			catch(CExceptionBase& exception)
			{
				Log.Add(exception);
				pendingSessions.insert(*sessionId);
			}
		}

		/// Now assome that know session is all sessions excluding pending sesion set
		m_knownSessions.clear();
		std::set_difference(	allSessions.begin(),
								allSessions.end(), 
								pendingSessions.begin(),
								pendingSessions.end(),
								std::insert_iterator<std::set<int> >(m_knownSessions, m_knownSessions.begin()));

		Sleep(SCAN_TIMEOUT);
	}
CATCH_LOG()
}

void CProxyStrapper::StartProxy(int sessionId)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("Starting proxy application for session %d"),sessionId);

	HANDLE newToken, currentToken;
	if (0 == OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &currentToken))
		throw MCException_Win("Failed to OpenProcessToken");


	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > spCurrentToken( currentToken, CloseHandle );

	if (0 == DuplicateTokenEx(spCurrentToken.get(), MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &newToken))
		throw MCException_Win("Failed to Duplicate process token");

	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > spNewToken( newToken, CloseHandle );

	DWORD tokenSessionId = sessionId;
	if (0 == SetTokenInformation(spNewToken.get(), TokenSessionId, &tokenSessionId, sizeof(tokenSessionId)))
		throw MCException_Win("Failed to SetTokenInformation");

	/// Setting highest integrity level
	TCHAR integritySid[20] = _T("S-1-16-16384");
	PSID pIntegritySid = NULL;
	if (ConvertStringSidToSid(integritySid, &pIntegritySid))
	{
		typedef struct _TOKEN_MANDATORY_LABEL 
		{
			SID_AND_ATTRIBUTES Label;
		} TOKEN_MANDATORY_LABEL,*PTOKEN_MANDATORY_LABEL;
		TOKEN_MANDATORY_LABEL TIL = {0};
		TIL.Label.Attributes = 0x00000020L /*SE_GROUP_INTEGRITY*/;
		TIL.Label.Sid = pIntegritySid;

		// Set the process integrity level
		if (!SetTokenInformation(	spNewToken.get(), 
									(TOKEN_INFORMATION_CLASS)25/*TokenIntegrityLevel*/, 
									&TIL,
									sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(pIntegritySid)))
		{
			Log.WinError(_WARNING_,_T("Failed to SetTokenInformation"));
		}
	} else
	{
		Log.WinError(_WARNING_,_T("Failed to ConvertStringSidToSid"));
	}

	boost::shared_ptr<PROCESS_INFORMATION> pi;
	pi.reset(new PROCESS_INFORMATION, boost::bind(&CProxyStrapper::ShutdownProcess,this,_1));
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = _T("winsta0\\default");

	TCHAR moduleName[MAX_PATH];
	GetModuleFileName(reinterpret_cast<HMODULE>(NULL), moduleName, MAX_PATH);

	if (0 == CreateProcessAsUser(	spNewToken.get(),
									NULL,
									(LPSTR)Format(_T("%s /appmode"),moduleName).c_str(),
									NULL,
									NULL,
									FALSE,
									NORMAL_PRIORITY_CLASS,
									NULL,
									NULL,
									&si,
									pi.get()
									))
		throw MCException_Win("Failed to CreateProcessAsUser");

	m_startedProcesses.push_back(pi);
	m_processSessions[pi->dwProcessId] = sessionId;

CATCH_THROW()
}

void CProxyStrapper::ShutdownProcess(PROCESS_INFORMATION *pi)
{
TRY_CATCH
	if (pi->hProcess != INVALID_HANDLE_VALUE)
	{
		/// Shuting down process
		/// Firstly - try to stop delecately
		try
		{
			if (0 == PostThreadMessage(pi->dwThreadId, WM_QUIT, 0, 0))
			{
				Log.WinError(_WARNING_,_T("Failed to send WM_QUIT to proxy object. Try to shutdown through proxy app. Pid(%d) "),pi->dwProcessId);
				ShutdownThroughProxy(pi->dwThreadId, m_processSessions[pi->dwProcessId]);
			}
			if (WAIT_OBJECT_0 != WaitForSingleObject(pi->hProcess, TERMINATE_PROXY_TIMEOUT))
			{
				throw MCException_Win("Waiting for process termination failed ");
			}
		}
		catch(CExceptionBase &e)
		{
			Log.Add(_ERROR_,_T("Soft process %d terminate failed, terminating it forcedly %s"),pi->dwProcessId, e.m_strWhat.c_str());
			if (0 == TerminateProcess(pi->hProcess,0))
				Log.WinError(_ERROR_,_T("Failed to terminate process. Pid(%d) "),pi->dwProcessId);
		}
		CloseHandle(pi->hThread);
		CloseHandle(pi->hProcess);
	} else
		Log.Add(_MESSAGE_,_T("Will not terminate process Pid(%d) with invalid handle"),pi->dwProcessId);
	delete pi;
CATCH_LOG()
}

void CProxyStrapper::ShutdownThroughProxy(const int threadId, const int sessionId)
{
TRY_CATCH

	HANDLE newToken, currentToken;
	if (0 == OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &currentToken))
		throw MCException_Win("Failed to OpenProcessToken");


	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > spCurrentToken( currentToken, CloseHandle );

	if (0 == DuplicateTokenEx(spCurrentToken.get(), MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &newToken))
		throw MCException_Win("Failed to Duplicate process token");

	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > spNewToken( newToken, CloseHandle );

	DWORD tokenSessionId = sessionId;
	if (0 == SetTokenInformation(spNewToken.get(), TokenSessionId, &tokenSessionId, sizeof(tokenSessionId)))
		throw MCException_Win("Failed to SetTokenInformation");

	/// Setting highest integrity level
	TCHAR integritySid[20] = _T("S-1-16-16384");
	PSID pIntegritySid = NULL;
	if (ConvertStringSidToSid(integritySid, &pIntegritySid))
	{
		typedef struct _TOKEN_MANDATORY_LABEL 
		{
			SID_AND_ATTRIBUTES Label;
		} TOKEN_MANDATORY_LABEL,*PTOKEN_MANDATORY_LABEL;
		TOKEN_MANDATORY_LABEL TIL = {0};
		TIL.Label.Attributes = 0x00000020L /*SE_GROUP_INTEGRITY*/;
		TIL.Label.Sid = pIntegritySid;

		// Set the process integrity level
		if (!SetTokenInformation(	spNewToken.get(), 
									(TOKEN_INFORMATION_CLASS)25/*TokenIntegrityLevel*/, 
									&TIL,
									sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(pIntegritySid)))
		{
			Log.WinError(_WARNING_,_T("Failed to SetTokenInformation"));
		}
	} else
	{
		Log.WinError(_WARNING_,_T("Failed to ConvertStringSidToSid"));
	}

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = _T("winsta0\\default");

	TCHAR moduleName[MAX_PATH];
	GetModuleFileName(reinterpret_cast<HMODULE>(NULL), moduleName, MAX_PATH);

	if (0 == CreateProcessAsUser(	spNewToken.get(),
									NULL,
									(LPSTR)Format(_T("%s /shutdown%d"),moduleName,threadId).c_str(),
									NULL,
									NULL,
									FALSE,
									NORMAL_PRIORITY_CLASS,
									NULL,
									NULL,
									&si,
									&pi
									))
		throw MCException_Win("Failed to CreateProcessAsUser");

	if (WAIT_OBJECT_0 != WaitForSingleObject(pi.hProcess, TERMINATE_PROXY_TIMEOUT))
	{
		TerminateProcess(pi.hProcess,0);
		throw MCException_Win("Waiting for process termination failed ");
	}

CATCH_THROW()
}
