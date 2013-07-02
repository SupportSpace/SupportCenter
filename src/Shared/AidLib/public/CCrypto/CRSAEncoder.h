/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRSAEncoder.h
///
///  Declares CRSAEncoder class - responsible for encoding/decoding data
///    using RSA algorithm
///
///  @author Dmitry Netrebenko @date 14.09.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/CCrypto/CCrypto.h>
#include <AidLib/AidLib.h>

/// Default RSA key size
#define DEFAULT_RSA_KEY_SIZE 512
/// Size of padding for RSA encoding
#define RSA_PADDING_SIZE 11

///  CRSAEncoder class - responsible for encoding/decoding data
///    using RSA algorithm
class AIDLIB_API CRSAEncoder
{
private:
/// Prevents making copies of CRSAEncoder objects
	CRSAEncoder(const CRSAEncoder&);
	CRSAEncoder& operator=(const CRSAEncoder&);

	///  CCryptKey class - responsible for managing of crypto keys
	class CCryptKey
	{
	private:
	/// Prevents making copies of CCryptKey objects
		CCryptKey(const CCryptKey&);
		CCryptKey& operator=(const CCryptKey&);
	/// Destroys previouly stored key
		void Free() { if(m_key) { CryptDestroyKey(m_key); m_key = 0; } };
	public:
	/// Constructor
		CCryptKey(HCRYPTKEY key = 0) : m_key(key) {};
	/// Destructor
		~CCryptKey() { Free(); };
	/// Key accessor
		HCRYPTKEY get() const { return m_key; };
	/// Assigns to new crypto key
		void reset(HCRYPTKEY key = 0) { if(m_key) Free(); m_key = key; };
	private:
	/// Crypto key
		HCRYPTKEY m_key;
	};

public:
/// Constructor
	CRSAEncoder();

/// Destructor
	~CRSAEncoder();

/// Initializes local RSA key pair
/// @param keySize - size of key in bits
	void Create(const unsigned short keySize = DEFAULT_RSA_KEY_SIZE);

/// Initializes remote RSA key from buffer
/// @param buf - buffer with public key
/// @param len - buffer size
	void SetRemoteKey(const unsigned char* buf, unsigned int len);

/// Exports RSA public key to buffer
/// @param buf - buffer to store results
/// @param len - [in] pointer to buffer size, [out] pointer to result size
	void GetLocalKey(unsigned char* buf, unsigned int* len);

/// Encodes buffer using local key
/// @param buf - [in] data for encryption, [out] encrypted data
/// @param bufLen - buffer size
/// @param dataLen - [in] data size, [out] encrypted data size
	void Encrypt(unsigned char* buf, unsigned int bufSize, unsigned int* dataSize);

/// Decodes buffer using remote key
/// @param buf - [in] encrypted data, [out] decrypted data
/// @param len - [in] encrypted data size, [out] decrypted data size
	void Decrypt(unsigned char* buf, unsigned int* len);

/// Returns current local key size
	unsigned short GetLocalKeySize() const;

private:
/// Key size in bits
	unsigned short	m_keySize;
/// Local crypt key
	CCryptKey		m_localKey;
/// Remote crypt key
	CCryptKey		m_remoteKey;
/// Instance of crypto context
	CCrypto			m_crypto;
};
