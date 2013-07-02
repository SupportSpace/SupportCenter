#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNTDLLSessionEnumerator.h
///
///  Class, enumerating sessions through Terminal Services
///
///  @author "Archer Software" Sogin M. @date 02.10.2007
///
////////////////////////////////////////////////////////////////////////
#include "CSessionEnumerator.h"
#include <windows.h>

typedef LONG NTSTATUS;
typedef LONG KPRIORITY;
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
#define SystemProcessesAndThreadsInformation 5
#define DEFAULT_INPUT_BUFFER_SIZE 0x8000

/// Class, enumerating sessions through Terminal Services
class CNTDLLSessionEnumerator : public CSessionEnumerator
{
private:
	/// Handle of NtDll.dll
	HINSTANCE m_hNtDll;
	/// Pointer to ZwQuerySystemInformation function
	NTSTATUS (WINAPI * m_pZwQuerySystemInformation)(UINT, PVOID, ULONG, PULONG);
public:
	/// ctor
	CNTDLLSessionEnumerator();

	/// Returns sessions set for localhost
	/// @param sessions [out] set of sessions for localhost
	virtual void GetSessions(std::set<int>& sessions);
};

