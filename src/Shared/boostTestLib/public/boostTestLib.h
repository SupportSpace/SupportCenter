/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  boostTestLib.h
///
///  main header for unit test framework
///
///  @author Sogin Max @date 08.02.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef BOOST_HAS_WINTHREADS 
	#define BOOST_HAS_WINTHREADS
#endif
#ifndef BOOST_THREAD_USE_LIB 
	#define BOOST_THREAD_USE_LIB
#endif
#ifndef BOOST_THREAD_NO_LIB
	#define BOOST_THREAD_NO_LIB
#endif

#include <boostTestLib/CLogFormatter.h>
#include <boost/test/unit_test.hpp>
#include <boost/test/framework.hpp>
#include <boost/test/results_collector.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/results_reporter.hpp>
#include <boost/test/detail/unit_test_parameters.hpp>

// Boost
#include <boost/cstdlib.hpp>

// STL
#include <stdexcept>
#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/test/detail/suppress_warnings.hpp>
#include <boost/test/unit_test_log.hpp>

// boost test static lib
#pragma comment(lib, "boostTestLib.lib")
#pragma comment(lib, "boostThreads.lib")

using namespace boost;
using namespace boost::unit_test;
using boost::unit_test::test_suite;

const char* xml_head = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>\n\
						<!-- ?xml-stylesheet type=\"text/xsl\" href=\"boost_test.xsl\"? -->\n\
						<UnitTests>\n";
const char* xml_tail = "\n</UnitTests>\n";

int BOOST_TEST_CALL_DECL runTests( int argc, char* argv[] )
{
	using namespace boost::unit_test;
	try 
	{

        framework::init( argc, argv );

		/// initializing outut
		Log.RegisterLog(new cConsLog());
		cFileLog *fileLog = new cFileLog();
		fileLog->ClearLog();
		Log.RegisterLog(fileLog);
		unit_test_log.set_formatter(new CLogFormatter());
		unit_test_log.set_threshold_level( log_test_suites );

        framework::run();

        results_reporter::make_report();

        return runtime_config::no_result_code() 
                    ? boost::exit_success 
                    : results_collector.results( framework::master_test_suite().p_id ).result_code();
    }
    catch( std::logic_error const& ex ) {
        std::cerr << "Boost.Test internal framework error: " << ex.what() << std::endl;
        
        return boost::exit_exception_failure;
    }
    catch( ... ) {
        std::cerr << "Boost.Test internal framework error: unknown reason" << std::endl;
        
        return boost::exit_exception_failure;
    }
}

#include <boost/threadpool.hpp>
#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>

extern HANDLE hunitTestComplete;
extern int hangupTimeOut;

void PreventHangup()
{
TRY_CATCH
	if (!hunitTestComplete)
	{
		hunitTestComplete = CreateEvent(NULL,TRUE,FALSE,NULL);
		if (!hunitTestComplete)
			throw MCException("Failed to create CreateEvent");
	}
	if (WAIT_TIMEOUT == WaitForSingleObject(hunitTestComplete, hangupTimeOut))
	{
		Log.Add(_ERROR_,_T("Test excecution time limit (%dms) exceeded. Terminating test"),hangupTimeOut);
		TerminateProcess(GetCurrentProcess(),1);
	}
CATCH_LOG()
}

// ************************************************************************** //
// **************                 unit test main               ************** //
// ************************************************************************** //
int BOOST_TEST_CALL_DECL _tmain( int argc, char* argv[] )
{
	bool wrapXML(false);

	/// Creating log singleton instance
	Log;
	
#ifdef LIMIT_EXECUTION_TIME
	hangupTimeOut = LIMIT_EXECUTION_TIME;
	boost::threadpool::pool hangupThreadPool(1);
	hangupThreadPool.schedule(boost::bind(&PreventHangup));
#endif

	for(int i=0;i<argc;++i)
	if (LowerCase(argv[i]) == "--output_format=xml")
	{
		wrapXML = true;
		break;
	}

	// Wrapping xml
	if (wrapXML)
	{
		fprintf(stderr,xml_head);
	}
	
	// running tests
	int result = runTests(argc, argv);

	// Wrapping xml
	if (wrapXML)
	{
		fprintf(stderr,xml_tail);
	}

	if (hunitTestComplete)
	{
		SetEvent(hunitTestComplete);
	}

	return result;
}
