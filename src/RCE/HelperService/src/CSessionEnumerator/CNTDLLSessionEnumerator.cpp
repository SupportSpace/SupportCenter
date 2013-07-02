/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNTDLLSessionEnumerator.cpp
///
///  Class, enumerating sessions through Terminal Services
///
///  @author "Archer Software" Sogin M. @date 02.10.2007
///
////////////////////////////////////////////////////////////////////////
#include "CNTDLLSessionEnumerator.h"
#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>
#include <boost/scoped_array.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/shared_ptr.hpp>

///DOXYS_OFF
typedef enum _KWAIT_REASON
{
	Executive,
	FreePage,
	PageIn,
	PoolAllocation,
	DelayExecution,
	Suspended,
	UserRequest,
	WrExecutive,
	WrFreePage,
	WrPageIn,
	WrPoolAllocation,
	WrDelayExecution,
	WrSuspended,
	WrUserRequest,
	WrEventPair,
	WrQueue,
	WrLpcReceive,
	WrLpcReply,
	WrVirtualMemory,
	WrPageOut,
	WrRendezvous,
	Spare2,
	Spare3,
	Spare4,
	Spare5,
	Spare6,
	WrKernel,
	MaximumWaitReason
} KWAIT_REASON, *PKWAIT_REASON;

//
// ClientId
//
typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID *PCLIENT_ID;

typedef struct _UNICODE_STRING {
    USHORT        Length;
    USHORT        MaximumLength;
    PWSTR         Buffer;
} UNICODE_STRING;

typedef struct _VM_COUNTERS {
    SIZE_T        PeakVirtualSize;
    SIZE_T        VirtualSize;
    ULONG         PageFaultCount;
    SIZE_T        PeakWorkingSetSize;
    SIZE_T        WorkingSetSize;
    SIZE_T        QuotaPeakPagedPoolUsage;
    SIZE_T        QuotaPagedPoolUsage;
    SIZE_T        QuotaPeakNonPagedPoolUsage;
    SIZE_T        QuotaNonPagedPoolUsage;
    SIZE_T        PagefileUsage;
    SIZE_T        PeakPagefileUsage;
} VM_COUNTERS;

typedef struct _SYSTEM_THREADS {
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG         WaitTime;
    PVOID         StartAddress;
    CLIENT_ID     ClientId;
    KPRIORITY     Priority;
    KPRIORITY     BasePriority;
    ULONG         ContextSwitchCount;
    LONG          State;
    LONG          WaitReason;
} SYSTEM_THREADS, * PSYSTEM_THREADS;

typedef struct _SYSTEM_PROCESSES {
    ULONG             NextEntryDelta;
    ULONG             ThreadCount;
    ULONG             Reserved1[6];
    LARGE_INTEGER     CreateTime;
    LARGE_INTEGER     UserTime;
    LARGE_INTEGER     KernelTime;
    UNICODE_STRING    ProcessName;
    KPRIORITY         BasePriority;
    ULONG             ProcessId;
    ULONG             InheritedFromProcessId;
    ULONG             HandleCount;
    ULONG             Reserved2[2];
    VM_COUNTERS       VmCounters;
//#if _WIN32_WINNT >= 0x500
    IO_COUNTERS       IoCounters;
//#endif
    SYSTEM_THREADS    Threads[1];
} SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;
///DOXYS_ON

CNTDLLSessionEnumerator::CNTDLLSessionEnumerator()
{
TRY_CATCH

	//Getting handle to ntdll.dll
	m_hNtDll = GetModuleHandle(_T("ntdll.dll"));
	if (!m_hNtDll) 
		throw MCException_Win("Failed to GetModuleHandle");
	
	//Getting pointer to ZwQuerySystemInformation function
	*(FARPROC *)&m_pZwQuerySystemInformation = GetProcAddress(m_hNtDll, _T("ZwQuerySystemInformation"));
	if (NULL == m_pZwQuerySystemInformation)
		throw MCException_Win("Failed to GetProcAddress for ZwQuerySystemInformation");

CATCH_THROW()
}

void CNTDLLSessionEnumerator::GetSessions(std::set<int>& sessions)
{
TRY_CATCH

	NTSTATUS status;
	ULONG bufferSize = DEFAULT_INPUT_BUFFER_SIZE;
	boost::scoped_array<char> buffer;

	//Using DEFAULT_INPUT_BUFFER_SIZE (32K) as the defailt value for input buffer 
	//and if it's not enough enlarging it
	do
	{
		buffer.reset(new char[bufferSize]);
		if (NULL == buffer.get())
			throw MCException_Win("Failed to allocate mem for internal buffer");

		status = m_pZwQuerySystemInformation(	SystemProcessesAndThreadsInformation,
												buffer.get(), bufferSize, NULL);

		if (STATUS_INFO_LENGTH_MISMATCH == status)
			bufferSize *= 2;
		else if (!NT_SUCCESS(status))
			throw MCException_Win("Failed to ZwQuerySystemInformation");
	}
	while (STATUS_INFO_LENGTH_MISMATCH == status);

	/// Clearing sessions set
	if (!sessions.empty())
		sessions.clear();

	PSYSTEM_PROCESSES processes = reinterpret_cast<PSYSTEM_PROCESSES>(buffer.get());
	//New set (since we had to remove finished processes)
	for(;;)
	{
		boost::shared_ptr<boost::remove_pointer<HANDLE>::type> process;
		process.reset(OpenProcess(PROCESS_QUERY_INFORMATION,0,processes->ProcessId),CloseHandle);
		if (NULL != process.get())
		{
			HANDLE hToken;
			if (0 != OpenProcessToken(process.get(), TOKEN_READ, &hToken))
			{
				boost::shared_ptr<boost::remove_pointer<HANDLE>::type> token;
				token.reset(hToken, CloseHandle);
				DWORD sessionId;
				DWORD ret;
				if (0 != GetTokenInformation(token.get(), TokenSessionId, &sessionId, sizeof(sessionId), &ret))
				{
					/// Finally we get process session id;
					sessions.insert(sessionId);
				}
			}
		}
		if (processes->NextEntryDelta == 0)
			break;
		// find the address of the next process structure
		processes = reinterpret_cast<PSYSTEM_PROCESSES>(reinterpret_cast<LPBYTE>(processes) + processes->NextEntryDelta);
	}

CATCH_THROW()
}
