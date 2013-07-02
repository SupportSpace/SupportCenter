/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAbstractStream.cpp
///
///  Implements CAbstractStream class, abstract class for streams
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CAbstractStream.h>
#include <AidLib/CException/CException.h>
#include <AidLib/Logging/cLog.h>
#include <AidLib/CCritSection/CCritSection.h>

CAbstractStream::CAbstractStream()
	:	m_cs1Exists(false), m_cs1(NULL)
{
TRY_CATCH
	
	// Initialize critical section
	InitializeCriticalSection( &m_csSendSection );

	ClearQueue();

CATCH_THROW("CAbstractStream::CAbstractStream")
}

CAbstractStream::~CAbstractStream()
{
TRY_CATCH

	// Destroy critical section
	DeleteCriticalSection( &m_csSendSection );
	
CATCH_LOG("CAbstractStream::~CAbstractStream")
}

void CAbstractStream::Send2Queue( const char* pbuffer, const unsigned int& bufsize)
{
TRY_CATCH

	// Enter critical section
	CCritSection section( &m_csSendSection );

	char* pbuff = const_cast<char*>(pbuffer);
	char* pQueue = m_bQueue;
	unsigned int buflen = bufsize;

	while(m_nUsedSize + buflen >= m_nStreamBufferSize)
	{
		// Copy data from buffer into queue
		memcpy( m_pBuffer, pbuff, m_nStreamBufferSize - m_nUsedSize );

		// Put queue into stream
		SendInternal( pQueue, m_nStreamBufferSize );

		// Calculate new buffer
		pbuff += m_nStreamBufferSize - m_nUsedSize;
		buflen -= m_nStreamBufferSize - m_nUsedSize;

		// Clear queue
		ClearQueue();
	}

	// Copy data from buffer into queue
	memcpy( m_pBuffer, pbuff, buflen );

	// Calculate clear memory in the queue
	m_pBuffer += buflen;

	// Calculate used memory in the queue
	m_nUsedSize += buflen;

CATCH_THROW("CAbstractStream::Send2Queue")
}

void CAbstractStream::FlushQueue()
{
TRY_CATCH

	// Enter critical section
	CCritSection section( &m_csSendSection );

	if ( 0 == m_nUsedSize )
		return;


	// Get pointer to begin of queue
	char* pBuffer = m_bQueue;

	// Put queue's data into stream
	SendInternal( pBuffer, m_nUsedSize );

	// Clear queue
	ClearQueue();

CATCH_THROW("CAbstractStream::FlushQueue")
}

void CAbstractStream::ClearQueue()
{
TRY_CATCH

	// Enter critical section
	CCritSection section( &m_csSendSection );

	// Get pointer to begin of queue
	char* pBuffer = m_bQueue;

	// Clear queue
	FillMemory( pBuffer, m_nStreamBufferSize, 0 );

	// Change pointer to clear memory in queue
	m_pBuffer = m_bQueue;

	// Clear number of used bytes
	m_nUsedSize = 0;

CATCH_THROW("CAbstractStream::ClearQueue")
}

void CAbstractStream::Receive( char* pbuffer, const unsigned int& bufsize )
{
TRY_CATCH

	ReceiveInternal( pbuffer, bufsize );

CATCH_THROW("CAbstractStream::Receive")
}

void CAbstractStream::Send( const char* pbuffer, const unsigned int& bufsize )
{
TRY_CATCH

	// Enter critical section
	CCritSection section( &m_csSendSection );

	FlushQueue();
	SendInternal( pbuffer, bufsize );

CATCH_THROW("CAbstractStream::Send")
}

unsigned int CAbstractStream::GetInBuffer( char* buf, const unsigned int& bufLen )
{
TRY_CATCH

	unsigned int i;
	for( i = 0; HasInData() && i < bufLen; ++i )
	{
		Receive( buf + i, 1 );
	}
	return i;

CATCH_THROW("CAbstractStream::GetInBuffer")
}

void CAbstractStream::SetCS1(CRITICAL_SECTION *cs1)
{
TRY_CATCH
	if (!cs1) return;
	m_cs1 = cs1;
	m_cs1Exists	= true;
CATCH_THROW("CAbstractStream::SetCS1")
}


CRITICAL_SECTION* CAbstractStream::GetCS1() const
{
TRY_CATCH
	return m_cs1;
CATCH_THROW("CAbstractStream::GetCS1")
}

bool CAbstractStream::CS1Exists() const
{
TRY_CATCH
	return m_cs1Exists;
CATCH_THROW("CAbstractStream::CS1Exists")
}