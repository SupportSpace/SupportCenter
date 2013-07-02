//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CSubStream.h
///
///  Declares a substream class
///  Provide operations with a stream through CSubStreamDispatcher
///  
///  @author Alexander Novak @date 27.09.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Multiplexer/CSubStreamDispatcher.h>
#include <NWL/Streaming/CAbstractStream.h>
#include <boost/shared_ptr.hpp>
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class NWL_API CSubStream
	:	public CAbstractStream
{
	friend class CStreamMultiplexerBase;
	unsigned int m_serviceID;

	boost::shared_ptr<CSubStreamDispatcher> m_dispatcher;
	
	CSubStream(const CSubStream&);
	CSubStream& operator=(const CSubStream&);
protected:

	/// Put data to the substream
	/// @param data			Buffer with data to send
	/// @param size			Number of bytes to send
	/// @return		Number of sent bytes
	virtual unsigned int SendInternal(const char* data, const unsigned int& size);

	/// Get data from the substream
	/// @param data			Buffer for data to receive
	/// @param size			Size of the buffer
	/// @return		Number of received bytes
	virtual unsigned int ReceiveInternal(char* data, const unsigned int& size);

	CSubStream(){}
public:

	/// Unregister a SubStream from the SubStreamDispatcher
	virtual ~CSubStream();

	/// Return substream's service identifier
	/// @return		Service identifier
	unsigned int GetServiceID();

	/// Cancel receive operation
	virtual void CancelReceiveOperation();

	/// Checks if data are available in the stream
	/// @return		[true] - if data are available, [false] in otherwise
	virtual bool HasInData();

	/// Get available data from the stream
	/// @param data			Buffer for data to get
	/// @param size			Size of the buffer
	/// @return		Number of received bytes
	virtual unsigned int GetInBuffer(char* data, const unsigned int& size);
};
//--------------------------------------------------------------------------------------------------------

inline unsigned int CSubStream::GetServiceID()
{
	return m_serviceID;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
