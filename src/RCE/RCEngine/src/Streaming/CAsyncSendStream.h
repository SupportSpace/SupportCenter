/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAsyncSendStream.h
///
///  Stream with asynchronous send method 
///
///  @author "Archer Software" Sogin M. @date 21.09.2007
///
////////////////////////////////////////////////////////////////////////
#include <NWL/Streaming/CAbstractStream.h>
#include <boost/shared_ptr.hpp>
#include <AidLib/Logging/CInstanceTracker.h>

#ifndef BOOST_THREAD_NO_LIB
	#define BOOST_THREAD_NO_LIB
#endif
#include <boost/thread.hpp>

/// Maximum thread start timeout. 30 secs seems to be quite enough
#define MAX_START_TIMEOUT 30000

/// Stream with asynchronous send method
/// Proper is wrapper on any sync stream
class CAsyncSendStream : 
	public CAbstractStream,
	public CInstanceTracker
{
private:
	/// Any sync (or even async) stream
	boost::shared_ptr<CAbstractStream> m_stream;

	/// Internal thread
	std::auto_ptr<boost::thread> m_thread;

	/// Thread id
	unsigned int m_threadId;
	
	/// Internal thread entry point
	void ThreadEntryPoint();

	/// Handle of event for thread start waiting
	HANDLE m_startedEvent;

	/// Internal messages codes
	const unsigned int m_msgSend;
	const unsigned int m_msgSend2Queue;
	const unsigned int m_msgFlush;
public:
	/// Constructor
	/// @param stream pointer to stream for wrap over
	CAsyncSendStream(boost::shared_ptr<CAbstractStream> stream);

	/// Destructor
	virtual ~CAsyncSendStream();

	/// The same as SendInternal, but inline and public
	/// Can be user if no Send2Queue are done for the same object
	/// It's added only to solve performance issuses
	/// Use it only if you excactly know what you're doing
	inline void FastSend( const char* buf, const unsigned int& len )
	{
		char* newBuf = new char[len];
		memcpy(newBuf,buf,len);
		if (FALSE == PostThreadMessage(m_threadId, m_msgSend, len, reinterpret_cast<LPARAM>(newBuf)))
		{
			delete [] newBuf;
			throw MCStreamException_Win(_T("CAsyncSendStream::FastSend Failed to send message"),GetLastError());
		}
	}

	inline void FastSend2Queue( const char* buf, const unsigned int& len )
	{
		char* newBuf = new char[len];
		memcpy(newBuf,buf,len);
		if (FALSE == PostThreadMessage(m_threadId, m_msgSend2Queue, len, reinterpret_cast<LPARAM>(newBuf)))
		{
			delete [] newBuf;
			throw MCStreamException_Win(_T("CAsyncSendStream::FastSend2Queue Failed to send message"),GetLastError());
		}
	}

	inline void FastFlushQeue()
	{
		if (FALSE == PostThreadMessage(m_threadId, m_msgFlush, 0, 0))
			throw MCStreamException_Win(_T("CAsyncSendStream::FastFlushQeue Failed to send message"),GetLastError());
	}

protected:
	/// Abstract function to get data from the stream
	/// @param   buffer for data
	/// @param   number of bytes to get
	/// @remarks
	virtual unsigned int ReceiveInternal( char*, const unsigned int& );

	/// Abstract function to put data to stream
	/// @param   buffer with data
	/// @param   number of bytes to put
	/// @remarks
	virtual unsigned int SendInternal( const char*, const unsigned int& );

	/// Checks data in the stream
	/// @return returns amount of available data
	virtual bool HasInData();
};