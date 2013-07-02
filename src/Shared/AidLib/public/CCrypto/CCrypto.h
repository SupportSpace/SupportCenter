//===========================================================================
// Archer Software.
//                                   CCrypto.h
//
//---------------------------------------------------------------------------
// Class for work with low-level Crypto API
//---------------------------------------------------------------------------
//
// Version : 01.00
// By      : George L.
// Date    : 7/26/05 10:12:02 AM
//===========================================================================

#ifndef	CCRYPTO_H
#define	CCRYPTO_H

#include <windows.h> 
#include <Winbase.h>
#include <wincrypt.h>
#include <AidLib/Strings/tstring.h>
#include <AidLib/Loki/Singleton.h>

#define SHA1Size 20
#define MD5Size 16
typedef char SHA1Hash[SHA1Size];
typedef char MD5Hash[MD5Size];

//===========================================================================
// @{CSEH}
//								CCrypto
//
//---------------------------------------------------------------------------
// Description		: Class for work with low-level Crypto API
//===========================================================================
class AIDLIB_API CCrypto
{
protected:
  //handle to cryptographic service providers (CSPs). 
  HCRYPTPROV hProv;

  //algorithm identifiers
  ALG_ID Alg;
  
  //size of HashBufer
  DWORD HashVal;

  CRITICAL_SECTION m_csHashSection;

  void SetAlgVal(ALG_ID alg, DWORD hashval);
public:
  CCrypto();
  virtual ~CCrypto();

  //make hash-digest for file specified in the FilePath
  //if SleepEach != 0 then functions fall in sleep once for each n buffer readings
  void MakeHash(tstring FilePath,char* HashBuf, int hashval=SHA1Size, const unsigned int SleepEach=0);

  //make hash-digest for  the specified buffer 
  void MakeHash(char* Buf,int BufLen,char* HashBuf, int hashval=SHA1Size);

  //make hash-digest for  the specified string 
  void MakeHashStr(tstring str,char* HashBuf, int hashval=MD5Size);

  // Creates RSA key pair with keySize size
  HCRYPTKEY CreateRSAKey(const unsigned short keySize);

  // Creates RSA public key from buffer
  HCRYPTKEY ImportRSAPublicKey(const unsigned char* buf, unsigned int len);

};

/// Should be used to access CCrypto as single instance
#define CRYPTO_INSTANCE Loki::SingletonHolder<CCrypto, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()

#endif // CCRYPTO_H

