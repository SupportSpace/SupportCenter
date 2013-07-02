/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAbstractStream.h
///
///  Declares CAbstractStream class, abstract class for streams
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include "CStreamException.h"
#include <NWL/NetworkLayer.h>

#define STREAM_QUEUE_SIZE 8192

///  Abstract class for streams
///  @remarks
class NWL_API CAbstractStream
{
friend class CShadowedStream;
friend class CShadowedClient;
private:
/// Prevents making copies of CAbstractStream objects.
	CAbstractStream( const CAbstractStream& );
	CAbstractStream& operator=( const CAbstractStream& );

public:
///  Constructor
	CAbstractStream();

///  Destructor
	virtual ~CAbstractStream();

protected:
///  Abstract function to get data from the stream
///  @param   buffer for data
///  @param   number of bytes to get
///  @remarks
	virtual unsigned int ReceiveInternal( char*, const unsigned int& ) = NULL;

///  Abstract function to put data to stream
///  @param   buffer with data
///  @param   number of bytes to put
///  @remarks
	virtual unsigned int SendInternal( const char*, const unsigned int& ) = NULL;

public:
///  Get data from the stream
///  @param   buffer for data
///  @param   number of bytes to get
///  @remarks
	virtual void Receive( char*, const unsigned int& );

///  Put data to stream
///  @param   buffer with data
///  @param   number of bytes to put
///  @remarks
	virtual void Send( const char*, const unsigned int& );

///  Puts data to queue
///  @param   buffer with data
///  @param   number of bytes to put
///  @remarks
	virtual void Send2Queue( const char*, const unsigned int& );

///  Puts queue to stream
///  @remarks
	void FlushQueue();

///  Cancel reading from the stream
///  @remarks
	virtual void CancelReceiveOperation() {};

///  Checks data in the stream
///  @return returns amount of available data
	virtual bool HasInData() = NULL;


///  Extracts data from input buffer
///  @param   Pointer to buffer
///  @param   Buffer size
///  @return  Number of bytes
///  @remarks
	virtual unsigned int GetInBuffer( char*, const unsigned int& );

private:
///  Size of queue
	const static unsigned int	m_nStreamBufferSize = STREAM_QUEUE_SIZE;
	
///  Queue
	char						m_bQueue[m_nStreamBufferSize];

///  Pointer to free memory in queue
	char*						m_pBuffer;

///  Number of bytes in queue
	unsigned int				m_nUsedSize;

///  Clears queue
///  @remarks
	void ClearQueue();

protected:
///  Critical section for sending
	CRITICAL_SECTION			m_csSendSection;

	bool						m_cs1Exists;
	CRITICAL_SECTION			*m_cs1;

public:
	/// Setup critical section ¦1 for some internal issues
	/// @param cs1 pointer to critical section ¦1 structure
	virtual void SetCS1(CRITICAL_SECTION *cs1);
	
	/// Returns pointer to critical section ¦1
	/// @return pointer to critical section ¦1
	CRITICAL_SECTION* GetCS1() const;

	/// Returns true if CS1 exists
	/// @return true if CS1 exists
	bool CS1Exists() const;
};
