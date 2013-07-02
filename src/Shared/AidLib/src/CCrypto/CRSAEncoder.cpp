/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRSAEncoder.cpp
///
///  Implements CRSAEncoder class - responsible for encoding/decoding data
///    using RSA algorithm
///
///  @author Dmitry Netrebenko @date 14.09.2007
///
////////////////////////////////////////////////////////////////////////

#include <AidLib/CCrypto/CRSAEncoder.h>
#include <AidLib/CException/CException.h>
#include <boost/scoped_array.hpp>

CRSAEncoder::CRSAEncoder()
	:	m_keySize(0)
{
TRY_CATCH
CATCH_THROW()
}

CRSAEncoder::~CRSAEncoder()
{
TRY_CATCH
CATCH_LOG()
}

void CRSAEncoder::Create(const unsigned short keySize)
{
TRY_CATCH
	m_keySize = keySize;
	/// Generate local keys pair
	m_localKey.reset(m_crypto.CreateRSAKey(m_keySize));
CATCH_THROW()
}

void CRSAEncoder::SetRemoteKey(const unsigned char* buf, unsigned int len)
{
TRY_CATCH
	if(!buf)
		throw MCException("Invalid buffer");
	if(!len)
		throw MCException("Invalid buffer size");
	/// Import remote key from buffer
	m_remoteKey.reset(m_crypto.ImportRSAPublicKey(buf, len));
CATCH_THROW()
}

void CRSAEncoder::GetLocalKey(unsigned char* buf, unsigned int* len)
{
TRY_CATCH
	if(!m_localKey.get())
		throw MCException("Local key is not created");
	if(!len)
		throw MCException("Invalid buffer size pointer");
	DWORD size;
	/// Get buffer size for local public key
	if(!CryptExportKey(m_localKey.get(), 0, PUBLICKEYBLOB, 0, NULL, &size))
		throw MCException_Win("CryptExportKey(size) failed");
	if(!buf)
	{
		*len = size;
		return;
	}
	if(size > *len)
		throw MCException("Too small buffer size");
	/// Export local public key
	if(!CryptExportKey(m_localKey.get(), 0, PUBLICKEYBLOB, 0, buf, &size))
		throw MCException_Win("CryptExportKey failed");
	/// Size of data
	*len = size;
CATCH_THROW()
}

void CRSAEncoder::Encrypt(unsigned char* buf, unsigned int bufSize, unsigned int* dataSize)
{
TRY_CATCH
	if(!m_remoteKey.get())
		throw MCException("Remote key is not created");
	if(!dataSize)
		throw MCException("Invalid buffer size");
	DWORD keySize = 0;
	/// Get key size
	if(!CryptEncrypt(m_remoteKey.get(), 0, TRUE, 0, NULL, &keySize, *dataSize))
		throw MCException_Win("CryptEncrypt(size) failed");
	/// Block size
	DWORD blockSize = keySize - RSA_PADDING_SIZE;
	if(0 == blockSize)
		throw MCException("Invalid block size.");
	/// Count of blocks
	DWORD count = *dataSize / blockSize;
	DWORD lastBlockSize = *dataSize % blockSize;
	if(lastBlockSize)
		count++;
	else
		lastBlockSize = blockSize;
	/// Size of buffer to store encrypted data
	DWORD needSize = keySize * count;
	*dataSize = needSize;
	if(!buf)
		return;
	if(needSize > bufSize)
		throw MCException("Invalid buffer size");
	boost::scoped_array<unsigned char> result(new unsigned char[needSize]);
	/// Encode blocks
	for(unsigned int i = 0; i < count; ++i)
	{
		unsigned char* dataPointer = buf + i * blockSize;
		unsigned char* resultPointer = result.get() + i * keySize;
		boost::scoped_array<unsigned char> block(new unsigned char[keySize]);
		BOOL lastStep = (i == count - 1);
		DWORD restSize = lastStep ? lastBlockSize : blockSize;
		memcpy(block.get(), dataPointer, restSize);
		if(!CryptEncrypt(m_remoteKey.get(), 0, lastStep, 0, block.get(), &restSize, keySize))
			throw MCException_Win("CryptEncrypt failed");
		memcpy(resultPointer, block.get(), keySize);
	}
	/// Copy result
	memcpy(buf, result.get(), needSize);
CATCH_THROW()
}

void CRSAEncoder::Decrypt(unsigned char* buf, unsigned int* len)
{
TRY_CATCH
	if(!m_localKey.get())
		throw MCException("Local key is not created");
	if(!buf)
		throw MCException("Invalid buffer");
	if(!len)
		throw MCException("Invalid buffer size pointer");
	DWORD keySize = m_keySize / 8;
	if(*len % keySize)
		throw MCException("Invalid buffer size");
	DWORD count = *len / keySize;
	boost::scoped_array<unsigned char> result(new unsigned char[*len]);
	unsigned char* resultPointer = result.get();
	DWORD totalDataSize = 0;
	/// Decode blocks
	for(unsigned int i = 0; i < count; ++i)
	{
		unsigned char* dataPointer = buf + i * keySize;
		boost::scoped_array<unsigned char> block(new unsigned char[keySize]);
		BOOL lastStep = (i == count - 1);
		memcpy(block.get(), dataPointer, keySize);
		DWORD dataSize = keySize;
		if(!CryptDecrypt(m_localKey.get(), 0, lastStep, 0, block.get(),&dataSize))
			throw MCException_Win("CryptDecrypt failed");
		memcpy(resultPointer, block.get(), dataSize);
		resultPointer += dataSize;
		totalDataSize += dataSize;
	}
	memset(buf, 0, * len);
	memcpy(buf, result.get(), totalDataSize);
	/// Decoded data size
	*len = totalDataSize;
CATCH_THROW()
}

unsigned short CRSAEncoder::GetLocalKeySize() const
{
TRY_CATCH
	return m_keySize;
CATCH_THROW()
}
