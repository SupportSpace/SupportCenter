/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CShadowedClient.h
///
///  Stream, emulating client
///
///  @author "Archer Software" Sogin M. @date 04.10.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include <NWL/Streaming/CAbstractStream.h>
#include <boost/shared_ptr.hpp>

/// Stream, emulating client
class CShadowedClient : public CAbstractStream
{
private:
	/// shadow stream, used only for Send operations
	boost::shared_ptr<CAbstractStream> m_stream;
	/// Buffer with initial messages that should be got
	std::auto_ptr<char> m_initialMessage;
	/// Buffer with update requests messages that should be got
	std::auto_ptr<char> m_updateRequest;
	/// Current position in buffer
	int m_pos;
	/// Initial message buffer size
	int m_initialMessageSize;
	/// true if init is complete
	bool m_inited;
	/// true if read cancelled
	bool m_cancelled;
	/// Pointer to width in update request
	unsigned short *m_width;
	/// Pointer to height in update request
	unsigned short *m_height;
	/// Critical section for read operation
	CRITICAL_SECTION m_csRead;
	/// Critical sectio for write operation
	CRITICAL_SECTION m_csWrite;
protected:

	///  Abstract function to get data from the stream
	///  @param   buffer for data
	///  @param   number of bytes to get
	virtual unsigned int ReceiveInternal( char*, const unsigned int& );

	///  Abstract function to put data to stream
	///  @param   buffer with data
	///  @param   number of bytes to put
	virtual unsigned int SendInternal( const char*, const unsigned int& );

public:
	/// initialises object instance
	/// @param stream shadow stream
	CShadowedClient(boost::shared_ptr<CAbstractStream> stream);
	/// dtor
	virtual ~CShadowedClient(void);

	///  Cancel reading from the stream
	virtual void CancelReceiveOperation();

	///  Checks data in the stream
	///  @return true if stream has data to read
	virtual bool HasInData();
};
