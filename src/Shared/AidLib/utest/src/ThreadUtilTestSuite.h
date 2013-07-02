#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  ThreadUtilTestSuite.h
///
///  Test suite for ThreadUtils
///
///  @author Sogin Max @date 02.11.2007
///
////////////////////////////////////////////////////////////////////////
#include <AidLib/CThread/ThreadUtil.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/threadpool.hpp>

namespace ThreadUtilTestSuite
{

	void TestThreadEntry()
	{
		std::set<DWORD> threads = GetProcessThreads(GetCurrentProcessId());
		BOOST_CHECK(threads.size() > 1);
		BOOST_CHECK(threads.find(GetCurrentThreadId()) != threads.end());
		BOOST_CHECK(threads.find(-1) == threads.end());
	}

	void TestGetProcessThreads()
	{
		for(int i=0; i<100; ++i)
		{
			boost::threadpool::pool pool(100);
			pool.schedule(TestThreadEntry);
		}
	}
}

test_suite* getThreadUtilTestSuiteTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "ThreadUtilTestSuiteTestSuite" );
	suite->add( BOOST_TEST_CASE(ThreadUtilTestSuite::TestGetProcessThreads) );
	return suite;
}
