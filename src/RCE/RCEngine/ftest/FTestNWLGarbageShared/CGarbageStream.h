/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CGarbageStream.h
///
///  Declares CGarbageStream class, responsible stream which will send 
///    garbage to stream
///
///  @author Dmitry Netrebenko @date 06.06.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <winsock2.h>
#include <NWL/Streaming/CSocketStream.h>

///  CGarbageStream class, responsible stream which will send 
///    garbage to stream
///  Base class - CSocketStream from NWL
class CGarbageStream
	:	public CSocketStream
{
private:
///  Prevents making copies of CGarbageStream objects.
	CGarbageStream( const CGarbageStream& );
	CGarbageStream& operator=( const CGarbageStream& );

public:
///  Constructor
///  @param active - pointer to active flag
	CGarbageStream( bool* active );
///  Destructor
	~CGarbageStream();

private:
///  Time to send garbage
	DWORD		m_timeForGarbage;
///  Is session active
	bool*		m_sessionActive;

protected:

///  Gut data to stream
///  @param   buffer with data
///  @param   number of bytes to put
///  @remarks
	virtual unsigned int SendInternal( const char* buf, const unsigned int& len );

private:
///  Calculates time to send garbage
	void CalcNewGarbageTime();
};
