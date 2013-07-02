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

#include <NWL/Streaming/CRSASocketStream.h>
#include <NWL/Streaming/CStreamException.h>
#include <boost/scoped_array.hpp>
#include <AidLib/CCritSection/CCritSection.h>

CRSASocketStream::CRSASocketStream(const unsigned short keySize)
	:	CSecureSocketStream()
	,	m_encoder()
	,	m_rsaConnection(false)
	,	m_buffered(0)
{
TRY_CATCH
	/// Initialize critical sections
	InitializeCriticalSection(&m_recvSection);
	InitializeCriticalSection(&m_sendSection);
	/// Create key pair
	m_encoder.Create(keySize);
CATCH_THROW()
}

CRSASocketStream::~CRSASocketStream()
{
TRY_CATCH
	/// Delete critical sections
	DeleteCriticalSection(&m_recvSection);
	DeleteCriticalSection(&m_sendSection);
CATCH_LOG()
}

unsigned int CRSASocketStream::ReceiveInternal(char* buf, const unsigned int& len)
{
TRY_CATCH

	unsigned int ret = 0;
	if (!len)
		return 0;
	if (m_rsaConnection)
	{
		/// Enter critical section
		CCritSection section(&m_recvSection);
		/// While buffer has data with length less than requested
		while(len > m_buffered)
		{
			/// Receive block size
			unsigned int dataSize;
			ret = CSocketStream::ReceiveInternal(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
			if (!ret)
			{
				RaiseDisconnectedEvent();
				throw MCSocketStreamException( _T("ReadError") );
			}
			if(!dataSize)
				throw MCSocketStreamException( _T("Invalid message size") );
			/// Receive data block
			boost::scoped_array<char> buffer(new char[dataSize]);
			ret = CSocketStream::ReceiveInternal(buffer.get(), dataSize);
			if (!ret)
			{
				RaiseDisconnectedEvent();
				throw MCSocketStreamException( _T("ReadError") );
			}
			/// Decryption
			m_encoder.Decrypt(reinterpret_cast<unsigned char*>(buffer.get()), &dataSize);
			/// Reallocate internal buffer and add received data
			boost::shared_ptr<char> newBuffer(new char[m_buffered + dataSize]);
			memcpy(newBuffer.get(), m_buffer.get(), m_buffered);
			memcpy(newBuffer.get() + m_buffered, buffer.get(), dataSize);
			m_buffer = newBuffer;
			m_buffered += dataSize;
		}
		/// Copy data to external buffer
		memcpy(buf, m_buffer.get(), len);
		m_buffered -= len;
		/// Reallocate internal buffer and remove requested data
		if(!m_buffered)
		{
			m_buffer.reset();
		}
		else
		{
			boost::shared_ptr<char> newBuffer(new char[m_buffered]);
			memcpy(newBuffer.get(), m_buffer.get() + len, m_buffered);
			m_buffer = newBuffer;
		}
		return len;
	}
	else
		ret = CSocketStream::ReceiveInternal(buf, len);

	if (!ret)
	{
		RaiseDisconnectedEvent();
		throw MCSocketStreamException( _T("ReadError") );
	}
	return ret;

CATCH_THROW()
}

unsigned int CRSASocketStream::SendInternal(const char* buf, const unsigned int& len)
{
TRY_CATCH

	unsigned int ret = 0;
	if(!len)
		return 0;
	if (m_rsaConnection)
	{
		/// Get buffer size for encrypted data
		unsigned int bufferSize = len;
		m_encoder.Encrypt(NULL, 0, &bufferSize);
		unsigned int dataSize = len;
		boost::scoped_array<char> buffer(new char[bufferSize]);
		memset(buffer.get(), 0, bufferSize);
		memcpy(buffer.get(), buf, len);
		m_encoder.Encrypt(reinterpret_cast<unsigned char*>(buffer.get()), bufferSize, &dataSize);
		{
			CCritSection section(&m_sendSection);
			ret = CSocketStream::SendInternal(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
			if(ret) ret = CSocketStream::SendInternal(buffer.get(), dataSize);
		}
	}
	else
		ret = CSocketStream::SendInternal(buf, len);

	if (!ret)
	{
		RaiseDisconnectedEvent();
		throw MCSocketStreamException( _T("Failed to send") );
	}
	return ret;

CATCH_THROW()
}

unsigned int CRSASocketStream::GetInBuffer(char* buf, const unsigned int& len)
{
TRY_CATCH
	if (m_ReceiveCancelled)
	{
		m_ReceiveCancelled = false;
		throw MCStreamException("Cancelled");
	}
	unsigned int ret;
	for(ret = 0; HasInData() && ret < len; ++ret)
	{
		ReceiveInternal(buf + ret, 1);
	}
	return ret;
CATCH_THROW()
}

void CRSASocketStream::RaiseDisconnectedEvent()
{
TRY_CATCH
	// Raise event from base class
	CSocketStream::RaiseDisconnectedEvent();
	m_rsaConnection = false;
CATCH_THROW("CRSASocketStream::RaiseDisconnectedEvent")
}

bool CRSASocketStream::HasInData()
{
TRY_CATCH
	return (m_buffered > 0) || CSocketStream::HasInData();
CATCH_THROW()
}

void CRSASocketStream::InitSecureConnection(bool masterRole)
{
TRY_CATCH
	unsigned int keySize;
	m_encoder.GetLocalKey(NULL, &keySize);
	boost::scoped_array<char> buffer(new char[keySize]);
	m_encoder.GetLocalKey(reinterpret_cast<unsigned char*>(buffer.get()), &keySize);
	unsigned int ret = CSocketStream::SendInternal(reinterpret_cast<char*>(&keySize), sizeof(keySize));
	if(!ret)
		throw MCSocketStreamException( _T("Failed to send public key size") );
	ret = CSocketStream::SendInternal(buffer.get(), keySize);
	if(!ret)
		throw MCSocketStreamException( _T("Failed to send public key") );
	ret = CSocketStream::ReceiveInternal(reinterpret_cast<char*>(&keySize), sizeof(keySize));
	if(!ret)
		throw MCSocketStreamException( _T("Failed to receive public key size") );
	if(!keySize)
		throw MCSocketStreamException( _T("Invalid public key size") );
	buffer.reset(new char[keySize]);
	ret = CSocketStream::ReceiveInternal(buffer.get(), keySize);
	if(!ret)
		throw MCSocketStreamException( _T("Failed to receive public key") );
	m_encoder.SetRemoteKey(reinterpret_cast<unsigned char*>(buffer.get()), keySize);
	m_rsaConnection = true;
CATCH_THROW()
}

bool CRSASocketStream::HasSecureConnection() const
{
TRY_CATCH
	return m_rsaConnection;
CATCH_THROW()
}

