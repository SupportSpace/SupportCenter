/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CGarbageThread.h
///
///  Declares CGarbageThread class, responsible thread which will send 
///    garbage to stream
///
///  @author Dmitry Netrebenko @date 06.06.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <AidLib/CThread/CThread.h>
#include <boost/shared_ptr.hpp>
#include <NWL/Streaming/CAbstractStream.h>

///  CGarbageThread class, responsible thread which will send 
///    garbage to stream
///  Base class - CThread from AidLib
class CGarbageThread
	:	public CThread
{
private:
///  Prevents making copies of CGarbageThread objects.
	CGarbageThread( const CGarbageThread& );
	CGarbageThread& operator=( const CGarbageThread& );

public:
///  Constructor
///  @param stream - connected stream
///  @param active - pointer to flag
	CGarbageThread( boost::shared_ptr<CAbstractStream> stream, bool* active );
///  Destructor
	~CGarbageThread();

///  Thread's entry point
///  @param Params - thread's parameters
	virtual void Execute( void* Params );

private:
///  Stream
	boost::shared_ptr<CAbstractStream>	m_stream;
///  Is session active
	bool*								m_sessionActive;

private:
///  Calculates time to send garbage
	DWORD CalcNextGarbageTime();
};

///  Shared pointer to CGarbageThread class
typedef boost::shared_ptr<CGarbageThread> SPGarbageThread;
