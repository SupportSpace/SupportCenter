#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  cLogTestSuite.h
///
///  Test suite for cLog
///
///  @author Sogin Max @date 09.02.2007
///
////////////////////////////////////////////////////////////////////////
#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>

namespace cLogTestSuite
{
	/// Defining log without formatting
	class CNotFormattedLog : public cLog
	{
	public:
		cEventDesc m_prevEventDesc;
		CNotFormattedLog() 
			: cLog(),
			m_prevEventDesc(_MESSAGE_)
		{};
	private:
		// TODO: refactor this after cLog will handle different log formats
		// Redeffining AddList to exclude output formatting
		virtual void AddList(const cEventDesc &EventDesc, const TCHAR *Item, ...) throw( )
		{
			va_list vl;
			tstring str;
			for(va_start(vl, Item); Item; Item=va_arg(vl, PTCHAR))
			{
				str += Item;
			}
			//Calling virtual function of adding string to destinate log('s)
			m_prevEventDesc = EventDesc;
			AddString(str.c_str(), EventDesc.getSeverity());
		}
	};

	/// Test log class descendant
	class CTestLog : public CNotFormattedLog
	{
	public:
		tstring m_prevMsg;
		CTestLog() : CNotFormattedLog() {};
	private:
		virtual void AddString(const TCHAR* LogString, const eSeverity Severity) throw()
		{
			m_prevMsg = LogString;
		}
	};

	/// Test for cLog exception safety
	class CInstigatorLog : public cLog
	{
		static void ThrowException()
		{
			throw std::exception("test exception");
		}
	public:
		virtual void AddString(const TCHAR* LogString, const eSeverity Severity) throw()
		{
			 //Causing exception
			ThrowException();
		}
	};

	/// Test case for cLog::Add method
	void TestAdd()
	{
		CTestLog log;
		// Testing severities + event description fields
		// Severity Message
		log.Add(_MESSAGE_,_T("Message text"));
		BOOST_CHECK(log.m_prevEventDesc.getLine() == __LINE__ - 1);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getFile(),__FILE__) == 0);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getCompDate(),__DATE__) == 0);
		BOOST_CHECK(log.m_prevEventDesc.getSeverity() == cLog::_MESSAGE);
		BOOST_CHECK(log.m_prevMsg == _T("Message text"));
		// Severity  Error
		log.Add(_ERROR_,_T("Error text"));
		BOOST_CHECK(log.m_prevEventDesc.getLine() == __LINE__ - 1);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getFile(),__FILE__) == 0);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getCompDate(),__DATE__) == 0);
		BOOST_CHECK(log.m_prevEventDesc.getSeverity() == cLog::_ERROR);
		BOOST_CHECK(log.m_prevMsg == _T("Error text"));
		// Severity Warning
		log.Add(_WARNING_,_T("Warning text"));
		BOOST_CHECK(log.m_prevEventDesc.getLine() == __LINE__ - 1);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getFile(),__FILE__) == 0);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getCompDate(),__DATE__) == 0);
		BOOST_CHECK(log.m_prevEventDesc.getSeverity() == cLog::_WARNING);
		BOOST_CHECK(log.m_prevMsg == _T("Warning text"));
		
		// Testing formatted output
		log.Add(_MESSAGE_,_T("This is %dst %s %s"),1,_T("test"),_T("message"));
		BOOST_CHECK(log.m_prevMsg == _T("This is 1st test message"));

		// Test first arg validation
		//BOOST_CHECK_NO_THROW(log.Add(_MESSAGE_,0));

		// Testing exception safety
		CInstigatorLog instigatorLog;
		BOOST_CHECK_NO_THROW(instigatorLog.Add(_MESSAGE_,_T("No exceptions foreva!")));
		BOOST_CHECK_THROW(instigatorLog.AddString(_T("No exceptions foreva!"),cLog::_MESSAGE),std::exception);

		// Note: wrong format strings handling not tested due to following reason:
		// It causes access violation SEH exceptions withing vsprintf, which are successully
		// catched by catch(...) in cLog::Add, but, while testing it thorough boost.test, library
		// AV exceptions are catched by boost SEH exceptions handler before catching by cLog::Add
		// causing test suite to fail, while indeed it works correctly.

		// Testing void Add(const eSeverity Severity, const CExceptionBase &e) throw( );
		CExceptionBase ex(MCException("Some exception"));
		log.Add(ex);
		BOOST_CHECK(log.m_prevEventDesc.getLine() == __LINE__ - 2);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getFile(),__FILE__) == 0);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getCompDate(),__DATE__) == 0);
		BOOST_CHECK(log.m_prevEventDesc.getSeverity() == cLog::_EXCEPTION);
		BOOST_CHECK(log.m_prevMsg == _T("Some exception"));

		// Testing void Add(const TCHAR *Str) throw( )
		log.Add(_T("Testing void Add(const TCHAR *Str) throw( )"));
		BOOST_CHECK(log.m_prevMsg == _T("Testing void Add(const TCHAR *Str) throw( )"));
	}

	void TestWinError()
	{
		// Ideal case for testing WinError method - is using FormatMessage mock
		// but since c++ doesn't provode clean way to do this, test case
		// will be restricted as follows:
		CTestLog log;
		// Testing severities + event description fields
		// Severity Message
		static const int TEST_ERROR = 123;
		SetLastError(TEST_ERROR);
		log.WinError(_MESSAGE_,_T("Message text"));
		BOOST_CHECK(log.m_prevEventDesc.getLine() == __LINE__ - 1);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getFile(),__FILE__) == 0);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getCompDate(),__DATE__) == 0);
		BOOST_CHECK(log.m_prevEventDesc.getSeverity() == cLog::_MESSAGE);
		BOOST_CHECK(log.m_prevEventDesc.getSYSERRNO() == TEST_ERROR);
		BOOST_CHECK(log.m_prevMsg != _T("Message text"));
		BOOST_CHECK(log.m_prevMsg.find(_T("Message text") == 0));
		// Severity  Error
		SetLastError(0);
		log.WinError(_ERROR_,_T("Error text"));
		BOOST_CHECK(log.m_prevEventDesc.getLine() == __LINE__ - 1);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getFile(),__FILE__) == 0);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getCompDate(),__DATE__) == 0);
		BOOST_CHECK(log.m_prevEventDesc.getSeverity() == cLog::_ERROR);
		BOOST_CHECK(log.m_prevEventDesc.getSYSERRNO() == 0);
		BOOST_CHECK(log.m_prevMsg != _T("Error text"));
		BOOST_CHECK(log.m_prevMsg.find(_T("Error text") == 0));
		// Severity Warning
		SetLastError(TEST_ERROR);
		log.WinError(_WARNING_,_T("Warning text"));
		BOOST_CHECK(log.m_prevEventDesc.getLine() == __LINE__ - 1);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getFile(),__FILE__) == 0);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getCompDate(),__DATE__) == 0);
		BOOST_CHECK(log.m_prevEventDesc.getSeverity() == cLog::_WARNING);
		BOOST_CHECK(log.m_prevEventDesc.getSYSERRNO() == TEST_ERROR);
		BOOST_CHECK(log.m_prevMsg != _T("Warning text"));
		BOOST_CHECK(log.m_prevMsg.find(_T("Warning text") == 0));
		
		// Testing formatted output
		SetLastError(TEST_ERROR);
		log.WinError(_MESSAGE_,_T("This is %dst %s %s"),1,_T("test"),_T("message"));
		BOOST_CHECK(log.m_prevMsg != _T("This is 1st test message"));
		BOOST_CHECK(log.m_prevEventDesc.getSYSERRNO() == TEST_ERROR);

		// Test first arg validation
		//BOOST_CHECK_NO_THROW(log.WinError(_MESSAGE_,0));

		// Testing exception safety
		CInstigatorLog instigatorLog;
		BOOST_CHECK_NO_THROW(instigatorLog.WinError(_MESSAGE_,_T("No exceptions foreva!")));

		// Note: wrong format strings handling not tested due to following reason:
		// It causes access violation SEH exceptions withing vsprintf, which are successully
		// catched by catch(...) in cLog::Add, but, while testing it thorough boost.test, library
		// AV exceptions are catched by boost SEH exceptions handler before catching by cLog::Add
		// causing test suite to fail, while indeed it works correctly.
	}

	void TestComError()
	{
		// Ideal case for testing ComError method - is using FormatMessage mock
		// but since c++ doesn't provode clean way to do this, test case
		// will be restricted as follows:
		CTestLog log;
		// Testing severities + event description fields
		// Severity Message
		static const int TEST_ERROR = 123;
		SetLastError(TEST_ERROR);
		log.ComError(_MESSAGE_,_T("Message text"));
		BOOST_CHECK(log.m_prevEventDesc.getLine() == __LINE__ - 1);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getFile(),__FILE__) == 0);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getCompDate(),__DATE__) == 0);
		BOOST_CHECK(log.m_prevEventDesc.getSeverity() == cLog::_MESSAGE);
		BOOST_CHECK(log.m_prevEventDesc.getSYSERRNO() == TEST_ERROR);
		BOOST_CHECK(log.m_prevMsg != _T("Message text"));
		BOOST_CHECK(log.m_prevMsg.find(_T("Message text") == 0));
		// Severity  Error
		SetLastError(0);
		log.ComError(_ERROR_,_T("Error text"));
		BOOST_CHECK(log.m_prevEventDesc.getLine() == __LINE__ - 1);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getFile(),__FILE__) == 0);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getCompDate(),__DATE__) == 0);
		BOOST_CHECK(log.m_prevEventDesc.getSeverity() == cLog::_ERROR);
		BOOST_CHECK(log.m_prevEventDesc.getSYSERRNO() == 0);
		BOOST_CHECK(log.m_prevMsg != _T("Error text"));
		BOOST_CHECK(log.m_prevMsg.find(_T("Error text") == 0));
		// Severity Warning
		SetLastError(TEST_ERROR);
		log.ComError(_WARNING_,_T("Warning text"));
		BOOST_CHECK(log.m_prevEventDesc.getLine() == __LINE__ - 1);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getFile(),__FILE__) == 0);
		BOOST_CHECK(_tcscmp(log.m_prevEventDesc.getCompDate(),__DATE__) == 0);
		BOOST_CHECK(log.m_prevEventDesc.getSeverity() == cLog::_WARNING);
		BOOST_CHECK(log.m_prevEventDesc.getSYSERRNO() == TEST_ERROR);
		BOOST_CHECK(log.m_prevMsg != _T("Warning text"));
		BOOST_CHECK(log.m_prevMsg.find(_T("Warning text") == 0));
		
		// Testing formatted output
		SetLastError(TEST_ERROR);
		log.ComError(_MESSAGE_,_T("This is %dst %s %s"),1,_T("test"),_T("message"));
		BOOST_CHECK(log.m_prevMsg != _T("This is 1st test message"));
		BOOST_CHECK(log.m_prevEventDesc.getSYSERRNO() == TEST_ERROR);

		// Test first arg validation
		//BOOST_CHECK_NO_THROW(log.ComError(_MESSAGE_,0));

		// Testing exception safety
		CInstigatorLog instigatorLog;
		BOOST_CHECK_NO_THROW(instigatorLog.ComError(_MESSAGE_,_T("No exceptions foreva!")));

		// Note: wrong format strings handling not tested due to following reason:
		// It causes access violation SEH exceptions withing vsprintf, which are successully
		// catched by catch(...) in cLog::Add, but, while testing it thorough boost.test, library
		// AV exceptions are catched by boost SEH exceptions handler before catching by cLog::Add
		// causing test suite to fail, while indeed it works correctly.
	}
}

test_suite* getcLogTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "cLogTestSuite" );
	suite->add( BOOST_TEST_CASE(cLogTestSuite::TestAdd) );
	suite->add( BOOST_TEST_CASE(cLogTestSuite::TestWinError) );
	suite->add( BOOST_TEST_CASE(cLogTestSuite::TestComError) );
	return suite;
}
