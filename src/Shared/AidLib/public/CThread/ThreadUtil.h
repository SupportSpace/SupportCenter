#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  ThreadUtil.h
///
///  Different thread utils
///
///  @author "Archer Software" Sogin M. @date 02.11.2007
///
////////////////////////////////////////////////////////////////////////
#include <set>
#include <windows.h>
#include <AidLib/AidLib.h>

/// Returns set of process threads ids
/// @param processId process to retrive threads
/// @remark on error exception is thrown
AIDLIB_API std::set<DWORD> GetProcessThreads(const DWORD processId);
