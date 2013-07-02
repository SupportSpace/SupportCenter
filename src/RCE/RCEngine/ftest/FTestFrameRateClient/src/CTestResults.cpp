/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTestResults.cpp
///
///  Implements CTestResults class, responsible for managementof test results
///
///  @author Dmitry Netrebenko @date 16.05.2007
///
////////////////////////////////////////////////////////////////////////

#include "CTestResults.h"
#include <windows.h>
#include <AidLib/CException/CException.h>
#include <AidLib/CSingleton/CSingleton.h>
#include "CSettings.h"
#include <boost/type_traits/remove_pointer.hpp>
#include <AidLib/Strings/tstring.h>

CTestResults::CTestResults()
	:	m_current(-1)
	,	m_entriesCount(0)
{
TRY_CATCH

	/// Prepare result entries
	m_entriesCount = SETTINGS_INSTANCE.GetResultEntriesCount();

	for(int i = 0; i < m_entriesCount; ++i)
	{
		SPResultEntry entry(new SResultEntry());
		m_entries.push_back(entry);
	}

	m_current = 0;

CATCH_THROW()
}

CTestResults::~CTestResults()
{
TRY_CATCH
CATCH_LOG()
}

void CTestResults::SaveResults()
{
TRY_CATCH

	/// Get file name to store results
	tstring fileName = SETTINGS_INSTANCE.GetResultsFileName();

	/// Open file
	HANDLE hFile = CreateFile(
		fileName.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if(INVALID_HANDLE_VALUE == hFile)
		throw MCException_Win(_T("Can not create file with results"));

	/// Create shared pointer to file's handle
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > spFile(hFile,CloseHandle);

	/// Write results
	DWORD write = 0;
	for(int i = 0; i < m_current; ++i)
	{
		SPResultEntry entry = m_entries[i];
		tstring resultStr = Format(_T("%d;%3.16f\n"), entry->m_time, entry->m_fps);
		
		DWORD len = static_cast<DWORD>(resultStr.length()) * sizeof(TCHAR);
		WriteFile(hFile, resultStr.c_str(), len, &write, NULL); 
		if(len != write)
			throw MCException_Win(_T("Error at saving test results"));
	}

CATCH_THROW()
}

void CTestResults::AddEntry(DWORD time, double fps)
{
TRY_CATCH

	if(m_current >= m_entriesCount)
		throw MCException(_T("Results buffer overflow"));

	/// Store results in internal vector
	m_entries[m_current]->m_time = time;
	m_entries[m_current]->m_fps = fps;

	m_current++;

CATCH_THROW()
}
