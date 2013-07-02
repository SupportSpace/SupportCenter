#include <AidLib/Utils/Utils.h>
#include <Tlhelp32.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/shared_ptr.hpp>
#include <AidLib/CException/CException.h>

AIDLIB_API DWORD TerminateChildProcesses(DWORD parentProcessId, std::set<DWORD>& exceptions)
{
	DWORD count = 0;
TRY_CATCH
	/// Create snapshot
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(INVALID_HANDLE_VALUE == snap)
		throw MCException_Win(_T("CreateToolhelp32Snapshot failed"));
	boost::shared_ptr<boost::remove_pointer<HANDLE>::type> spSnap(snap,CloseHandle);
	PROCESSENTRY32 process;
	memset(&process, 0, sizeof(PROCESSENTRY32));
	process.dwSize = sizeof(PROCESSENTRY32);
	/// Get first process
	if(FALSE == Process32First(spSnap.get(), &process))
	{
		if(ERROR_NO_MORE_FILES == GetLastError())
			return count;
		throw MCException_Win(_T("Process32First failed"));
	}
	/// Check up parent process and terminate
	if((process.th32ParentProcessID == parentProcessId) && (exceptions.end() == exceptions.find(process.th32ProcessID)))
		TerminateProcess(process.th32ProcessID, exceptions);
	/// Get next process
	while(TRUE == Process32Next(spSnap.get(), &process))
	{
		/// Check up parent process and terminate
		if((process.th32ParentProcessID == parentProcessId) && (exceptions.end() == exceptions.find(process.th32ProcessID)))
			TerminateProcess(process.th32ProcessID, exceptions);
	}
CATCH_LOG()
	return count;
}

AIDLIB_API void TerminateProcess(DWORD processId, std::set<DWORD>& exceptions)
{
TRY_CATCH
	/// Terminate child processes
	TerminateChildProcesses(processId, exceptions);
	/// Open process for termination
	boost::shared_ptr<boost::remove_pointer<HANDLE>::type> spProcess(
		OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE, FALSE, processId),
		CloseHandle);
	if(!spProcess.get())
		throw MCException_Win(_T("OpenProcess failed"));
	/// Get exit code
	DWORD exitCode = 0;
	GetExitCodeProcess(spProcess.get(), &exitCode);
	/// Terminate process
	if(FALSE == ::TerminateProcess(spProcess.get(), exitCode))
		throw MCException_Win(_T("TerminateProcess failed"));
CATCH_LOG()
}
