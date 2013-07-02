#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CThreadTestSuite.h
///
///  Test suite for CThread
///
///  @author Sogin Max @date 09.02.2007
///
////////////////////////////////////////////////////////////////////////
#include <AidLib/CThread/CThread.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>

namespace CThreadTestSuite
{
	const unsigned int MSG_REQUEST	= WM_USER+1;
	const unsigned int MSG_REPLY	= WM_USER+2;
	const unsigned int REPLY_TIMEOUT= 3000;

	/// Class for testing thread
	class CTestThread : public CThread
	{
	public:
		/// needed to test constructor's Param
		void* m_param;
		CTestThread(void *_Param = NULL)
			: CThread(_Param, true /*create message queue*/)
		{
		}
		
		/// Thread entry point
		virtual void Execute(void *Param)
		{
			m_param = Param;
			MSG msg;
			while(!Terminated())
			{
				if (PeekMessage(&msg,0,0,0,PM_REMOVE))
				{
					SetEvent(reinterpret_cast<HANDLE>(msg.wParam));
				}
				else
				{
					///SwitchToThread();
					Sleep(1); //Since SwitchToThread requires some global defines
				}
			}
		}
	};

	/// Checks if thread responces on requests
	bool IsThreadRunning(CTestThread& t)
	{
		HANDLE hEvent = CreateEvent(NULL,TRUE /*manual reset*/, FALSE /*initial state*/, NULL);
		boost::shared_ptr<boost::remove_pointer<HANDLE>::type> phEvent(hEvent,::CloseHandle);
		/// Posting request to thread
		PostThreadMessage(t.GetTid(),MSG_REQUEST,reinterpret_cast<LPARAM>(hEvent),0);
		/// Waiting for reply
		if (WaitForSingleObject(hEvent,REPLY_TIMEOUT) != WAIT_OBJECT_0)
			return false;
		else
			return true;
	}

	/// CThread::CThread test case
	void TestCtor()
	{
		CTestThread t1;
		BOOST_CHECK( !IsThreadRunning(t1) );

		CTestThread t2(&t1);
		BOOST_CHECK( !IsThreadRunning(t2) );
		t2.Start();
		BOOST_CHECK( IsThreadRunning(t2) );
		/// Checking param correctness
		BOOST_CHECK( t2.m_param == reinterpret_cast<void*>(&t1) );
	}

	/// CThread::Start test case
	void TestStart()
	{
		CTestThread t;
		t.Start();
		BOOST_CHECK( IsThreadRunning(t) );
	}

	/// CThread::Stop test case
	void TestStop()
	{
		CTestThread t;
		BOOST_CHECK( !IsThreadRunning(t) );
		t.Start();
		BOOST_CHECK( IsThreadRunning(t) );
		t.Stop(false);
		BOOST_CHECK( !IsThreadRunning(t) );
		BOOST_CHECK( t.Terminated() );
	}

	/// CThread::Suspend test case
	void TestSuspend()
	{
		CTestThread t;
		t.Start();
		t.Suspend();
		BOOST_CHECK( !IsThreadRunning(t) );
		BOOST_CHECK( !t.Terminated() );
		t.Start();
		BOOST_CHECK( IsThreadRunning(t) );
	}
}

test_suite* getCThreadTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CThreadTestSuite" );
	suite->add( BOOST_TEST_CASE(CThreadTestSuite::TestCtor) );
	suite->add( BOOST_TEST_CASE(CThreadTestSuite::TestStart) );
	suite->add( BOOST_TEST_CASE(CThreadTestSuite::TestStop) );
	suite->add( BOOST_TEST_CASE(CThreadTestSuite::TestSuspend) );
	return suite;
}
