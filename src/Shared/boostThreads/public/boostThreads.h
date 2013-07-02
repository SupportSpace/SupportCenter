/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  boostThreads.h
///
///  since wi do not use standard boost build process
///	 use this header to include boost::thread library
///
///  @author Sogin Max @date 20.05.2007
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

#pragma comment(lib, "boostThreads.lib")

#include <boost/thread.hpp>
#include <boost/bind.hpp>