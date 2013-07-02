#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  cFileLogTestSuite.h
///
///  Test suite for cFileLog
///
///  @author Sogin Max @date 09.02.2007
///
////////////////////////////////////////////////////////////////////////
#include <AidLib/Logging/cLog.h>
#include <AidLib/Strings/tstring.h>
#include <boost/scoped_array.hpp>

namespace cFileLogTestSuite
{
	/// Descedant class for test purposes
	class cTestLog : public cFileLog
	{
	public:
		/// Equivalent log string - to test file output
		tstring m_logString;
		cTestLog(const TCHAR* LogFileName) : cFileLog(LogFileName)
		{
		}

		virtual void AddString(const TCHAR* LogString, const eSeverity Severity) throw()
		{
			m_logString += LogString;
			cFileLog::AddString(LogString, Severity);
		}
	};

	/// Fixture for logging
	class cFileLogFixture
	{
	public:
		tstring m_logFileName;
		cTestLog m_log;
		cFileLogFixture()
			:	m_logFileName(GetGUID()),
				m_log(m_logFileName.c_str())
		{
		}

		~cFileLogFixture()
		{
			//Erasing log file
			DeleteFile(m_logFileName.c_str());
		}
	};

	/// cFileLog::AddString test case
	void TestAddString()
	{
		cFileLogFixture f;
		f.m_log.Add(_MESSAGE_,_T("Message test"));
		f.m_log.Add(_WARNING_,_T("Warning test"));
		f.m_log.Add(_ERROR_,_T("Error test"));
		HANDLE hFile = CreateFile(f.m_logFileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			BOOST_FAIL("Failed to CreateFile");
			return;
		}
		DWORD size = GetFileSize(hFile,0);
		boost::scoped_array<char> buf;
		buf.reset(new char[size]);
		DWORD read;
		ReadFile(hFile,buf.get(),size,&read,0);
		/// Checking read was successfull
		BOOST_CHECK( read == size && size > 0);
		/// Checking strings size equivalence
		BOOST_CHECK( size == f.m_log.m_logString.length());
		/// Checking strings equivalence
		BOOST_CHECK( 0 == memcmp(buf.get(),f.m_log.m_logString.c_str(),min(f.m_log.m_logString.length(),size)));
		CloseHandle(hFile);
	};

	/// cFileLog::ClearLog test case
	void TestClearLog()
	{
		cFileLogFixture f;
		f.m_log.Add(_MESSAGE_,_T("Message test"));
		f.m_log.Add(_WARNING_,_T("Warning test"));
		f.m_log.Add(_ERROR_,_T("Error test"));
		f.m_log.ClearLog();
		HANDLE hFile = CreateFile(f.m_logFileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			BOOST_FAIL("Failed to CreateFile");
			return;
		}
		DWORD size = GetFileSize(hFile,0);
		boost::scoped_array<char> buf;
		buf.reset(new char[size]);
		DWORD read;
		ReadFile(hFile,buf.get(),size,&read,0);
		/// Checking read was successfull
		BOOST_CHECK( read == size && size == 0);
		CloseHandle(hFile);
	};
}

test_suite* getcFileLogTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "cFileLogTestSuite" );
	suite->add( BOOST_TEST_CASE(cFileLogTestSuite::TestAddString) );
	suite->add( BOOST_TEST_CASE(cFileLogTestSuite::TestClearLog) );
	return suite;
}
