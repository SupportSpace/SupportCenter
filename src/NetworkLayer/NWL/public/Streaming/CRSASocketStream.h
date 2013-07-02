/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRSASocketStream.h
///
///  Declares CRSASocketStream class, responsible for socket stream with RSA protection
///
///  @author Dmitry Netrebenko @date 18.09.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CSecureSocketStream.h"
#include <AidLib/CCrypto/CRSAEncoder.h>
#include <boost/shared_ptr.hpp>
#include <NWL/NetworkLayer.h>

///  CRSASocketStream class, responsible for socket stream with RSA protection
///  Base class CSecureSocketStream - socket stream class with abstract security connection
class NWL_API CRSASocketStream 
	:	public CSecureSocketStream
{
private:
/// Prevents making copies of CRSASocketStream objects.
	CRSASocketStream( const CRSASocketStream& );
	CRSASocketStream& operator=( const CRSASocketStream& );

public:
///  Constructor
///  @param keySize - RSA key size
	CRSASocketStream(const unsigned short keySize = DEFAULT_RSA_KEY_SIZE);

///  Destructor
	virtual ~CRSASocketStream();

///  Initializes secure connection
///  @param masterRole - stream has master role
	virtual void InitSecureConnection( bool masterRole );

///  Returns true if secure connection is established
	virtual bool HasSecureConnection() const;

///  Checks data in the stream
///  @return returns amount of available data
	virtual bool HasInData();

///  Extracts data from input buffer
///  @param   Pointer to buffer
///  @param   Buffer size
///  @return  Number of bytes
	virtual unsigned int GetInBuffer( char*, const unsigned int& );

private:
///  Is stream connected using RSA
	bool					m_rsaConnection;
///  RSA encoder
	CRSAEncoder				m_encoder;
///  Buffer for receiving data
	boost::shared_ptr<char>	m_buffer;
///  Size of buffered data
	unsigned int			m_buffered;
///  Receive critical section
	CRITICAL_SECTION		m_recvSection;
///  Send critical section
	CRITICAL_SECTION		m_sendSection;

protected:
///  Gets data from the stream
///  @param   buffer for data
///  @param   number of bytes to get
	virtual unsigned int ReceiveInternal( char*, const unsigned int& );

///  Puts data to stream
///  @param   buffer with data
///  @param   number of bytes to put
	virtual unsigned int SendInternal( const char*, const unsigned int& );

///  Raises "Disconnected" event
	virtual void RaiseDisconnectedEvent();
};
