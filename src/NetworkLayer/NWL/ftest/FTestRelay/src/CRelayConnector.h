/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRelayConnector.h
///
///  Declares CRelayConnector class, responsible for testing of connection
///    with relay server
///
///  @author Dmitry Netrebenko @date 04.10.2007
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

#include <boost/shared_ptr.hpp>
#include <boost/threadpool.hpp>
#include <boost/thread.hpp>
#include <Aidlib/Strings/tstring.h>

/// Buffer size
#define BUFFER_SIZE		128

/// Step on which connection will be stopped
enum EStopConnect
{
	SC_NONE_STOP = 0,
	SC_AFTER_CONNECT,
	SC_BEFORE_AUTH_REQUEST,
	SC_AFTER_AUTH_REQUEST,
	SC_BEFORE_CHALLENGE,
	SC_AFTER_CHALLENGE,
	SC_BEFORE_CHECK_PORT,
	SC_AFTER_CHECK_PORT,
	SC_BEFORE_EXIT
};

#define MAX_ESTOPCONNECT (SC_BEFORE_EXIT + 1)

///  CRelayConnector class, responsible for testing of connection
///    with relay server
class CRelayConnector
{
private:
/// Prevents making copies of CRelayConnector objects
	CRelayConnector(const CRelayConnector&);
	CRelayConnector& operator=(const CRelayConnector&);
public:
/// Constructor
/// @param poolSize - count of threads in pool
	CRelayConnector(const size_t poolSize);
/// Destructor
	~CRelayConnector();
/// Starts test
/// @param count - count of peers to connect
	void Start(const int count);
private:
/// Thread pool
	boost::shared_ptr<boost::threadpool::pool> m_pool;
/// Thread for stub
	boost::shared_ptr<boost::thread> m_thread;
/// Termination flag
	bool m_terminated;
private:
/// Entry point for connection thread
/// @param userId - user name
/// @param password - user's password
/// @param stop - stopping step
/// @param exPort - port for stub
	void ThreadEntryPoint(const tstring& userId, const tstring& password, const EStopConnect stop, const unsigned int exPort);
/// Entry point for socket stub
	void StubEntryPoint();
};
