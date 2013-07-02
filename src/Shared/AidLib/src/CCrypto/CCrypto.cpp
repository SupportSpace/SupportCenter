//===========================================================================
// Archer Software.
//                                 CCrypto.cpp
//
//---------------------------------------------------------------------------
// Class for work with low-level Crypto API
//---------------------------------------------------------------------------
//
// Version : 01.00
// By      : George L.
// Date    : 7/26/05 10:13:38 AM
//===========================================================================
//#ifdef _DEBUG
//	#undef THIS_FILE
//	static char THIS_FILE[] = __FILE__;
//#endif //_DEBUG

#include <AidLib/CCrypto/CCrypto.h>
#include <AidLib/CException/CException.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <boost/scoped_array.hpp>

//===========================================================================
//
// @{FUN}                          CCrypto()
//
//---------------------------------------------------------------------------
// Effects		: Data initialization
// Errors		: On errors exception thrown
//===========================================================================	
CCrypto::CCrypto()
{
TRY_CATCH
  hProv=0;
  HashVal=SHA1Size;
  Alg=CALG_SHA1;
  if(!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL,CRYPT_VERIFYCONTEXT))
	 throw MCException_Win("errorCryptAcquireContext");
  // Initialize critical section
  InitializeCriticalSection(
    &m_csHashSection);

CATCH_THROW("CCrypto::CCrypto")
};

//===========================================================================
//
// @{FUN}                          CCrypto()
//
//---------------------------------------------------------------------------
// Effects		: Destroing object's data
// Errors		: On errors exception thrown
//===========================================================================	
CCrypto::~CCrypto()
{
TRY_CATCH
  if(!hProv)//unregistered erorr somewhere above 
	throw MCException("hProv==NULL");
  if(!CryptReleaseContext(hProv,0))
	  throw MCException_Win("errorCryptRContext");
  // Close critical section
  DeleteCriticalSection(
    &m_csHashSection);
CATCH_THROW("CCrypto::~CCrypto")
};

//===========================================================================
//
// @{FUN}                          SetAlgVal()
//
//---------------------------------------------------------------------------
// Effects		: Set algorithm and size of hash-digest.
// Arguments	: hashval[IN] - new size of hash-digest
// Arguments	: alg[IN] - new algorithm identifiers for hashing
// Errors		: On errors exception thrown
//===========================================================================	
void CCrypto::SetAlgVal(ALG_ID alg, DWORD hashval)
{
TRY_CATCH
  Alg=alg;
  HashVal=hashval;
CATCH_THROW("CCrypto::SetAlgVal")
};

//===========================================================================
//
// @{FUN}                          MakeHash()
//
//---------------------------------------------------------------------------
// Effects		: Make hash-digest for file specified in the FilePath.
// Arguments	: hashval[IN] - size of hash-digest 
// Arguments	: HashBuf[OUT] - hash-digest(use types SHA1Hash(char[20]) or 
//              : MD5Hash(char[16]))
// Arguments	: FilePath[IN] - string containing path to file and file name
// Arguments	: unsigned int SleepEach[IN] - if SleepEach != 0 then 
//				: functions fall in sleep once for each n buffer readings
// Errors		: On errors exception thrown
//===========================================================================	
void CCrypto::MakeHash(tstring FilePath,char* HashBuf, int hashval, const unsigned int SleepEach)
{
TRY_CATCH
  CCritSection section( &m_csHashSection );
  switch(hashval){
		case SHA1Size: 
			SetAlgVal(CALG_SHA1,SHA1Size);
			break;
		case MD5Size: 
			SetAlgVal(CALG_MD5,MD5Size);
			break;
		default:
			throw MCException("Incorrect BufSize");
	};
  if(!hProv)throw MCException_Win("errorCryptAcquireContext");
  HCRYPTHASH hHash;
  if(!CryptCreateHash(hProv, Alg, 0, 0, &hHash))throw MCException_Win("errorCryptCreateHash");
  HANDLE fh=CreateFile(FilePath.c_str(),GENERIC_READ,FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
  if(fh==INVALID_HANDLE_VALUE)throw MCException_Win("errorOpenFile");
  DWORD DW=0;
  char *myBuf=new char[BUFSIZ];
  //BUFSIZ is the required user-allocated buffer 
  for(unsigned int Count=0;ReadFile(fh,myBuf,BUFSIZ,&DW,NULL)&&DW!=0;Count++){
    if(!CryptHashData(hHash, (BYTE*)myBuf,DW, 0)) throw MCException_Win("errorCryptDataHash");
	if (SleepEach && Count>=SleepEach)
	{
		Count=0;Sleep(1);
	}
  };
  if(GetLastError()!=NO_ERROR){
    delete [] myBuf;
    CloseHandle(fh);
    throw MCException_Win("errorReadFile");
  };
  delete [] myBuf;
  CloseHandle(fh);
  if(!CryptGetHashParam(hHash, HP_HASHVAL, (BYTE*)HashBuf, &HashVal, 0))throw MCException_Win("errorCryptValueHash");
  if(!CryptDestroyHash(hHash))throw MCException_Win("errorCryptDestroyHash");
CATCH_THROW("CCrypto::MakeHash_File")
};

//===========================================================================
//
// @{FUN}                          MakeHash()
//
//---------------------------------------------------------------------------
// Effects		: Make hash-digest for  the specified buffer 
// Arguments	: hashval[IN] - size of hash-digest 
// Arguments	: HashBuf[OUT] - hash-digest(use types SHA1Hash(char[20]) or 
//              : MD5Hash(char[16]))
// Arguments	: BufLen[IN] - length of buffer 
// Arguments	: Buf[IN] - buffer 
// Errors		: On errors exception thrown
//===========================================================================	
void CCrypto::MakeHash(char* Buf,int BufLen,char* HashBuf, int hashval)
{
TRY_CATCH
  CCritSection section( &m_csHashSection );
  switch(hashval){
		case SHA1Size: 
			SetAlgVal(CALG_SHA1,SHA1Size);
			break;
		case MD5Size: 
			SetAlgVal(CALG_MD5,MD5Size);
			break;
		default:
			throw MCException("Incorrect BufSize");
	};
  if(!hProv)throw MCException_Win("errorCryptAcquireContext");
  HCRYPTHASH hHash;
  if(!CryptCreateHash(hProv, Alg, 0, 0, &hHash))throw MCException_Win("errorCryptCreateHash");
  if(!CryptHashData(hHash, (BYTE*)Buf,BufLen, 0))throw MCException_Win("errorCryptDataHash");
  if(!CryptGetHashParam(hHash, HP_HASHVAL, (BYTE*)HashBuf, &HashVal, 0))throw MCException_Win("errorCryptValueHash");
  if(!CryptDestroyHash(hHash))throw MCException_Win("errorCryptDestroyHash");
CATCH_THROW("CCrypto::MakeHash_Buf")
};

//===========================================================================
//
// @{FUN}                          MakeHashStr()
//
//---------------------------------------------------------------------------
// Effects		: Make hash-digest for the specified string 
// Arguments	: hashval[IN] - size of hash-digest 
// Arguments	: HashBuf[OUT] - hash-digest(use types SHA1Hash(char[20]) or 
//              : MD5Hash(char[16]))
// Arguments	: str[IN] - unicode string 
// Errors		: On errors exception thrown
//===========================================================================	
void CCrypto::MakeHashStr(tstring str,char* HashBuf, int hashval)
{
TRY_CATCH
  CCritSection section( &m_csHashSection );
  switch(hashval){
		case SHA1Size: 
			SetAlgVal(CALG_SHA1,SHA1Size);
			break;
		case MD5Size: 
			SetAlgVal(CALG_MD5,MD5Size);
			break;
		default:
			throw MCException("Incorrect BufSize");
	};
  if(!hProv)throw MCException_Win("errorCryptAcquireContext");
  HCRYPTHASH hHash;
  if(!CryptCreateHash(hProv, Alg, 0, 0, &hHash))throw MCException_Win("errorCryptCreateHash");
  DWORD Size=(unsigned int)str.size();
  Size*=2;
  boost::scoped_array<BYTE> buf(new BYTE[Size]);
  memcpy(buf.get(),str.data(),Size);
  if(!CryptHashData(hHash, buf.get(), Size, 0)) throw MCException_Win("errorCryptDataHash");
  if(!CryptGetHashParam(hHash, HP_HASHVAL, (BYTE*)HashBuf, &HashVal, 0))throw MCException_Win("errorCryptValueHash");
  if(!CryptDestroyHash(hHash))throw MCException_Win("errorCryptDestroyHash");
CATCH_THROW("CCrypto::MakeHash_Str")
};

HCRYPTKEY CCrypto::CreateRSAKey(const unsigned short keySize)
{
TRY_CATCH
	CCritSection section(&m_csHashSection);
	DWORD flags = keySize;
	flags = flags << 16;
	HCRYPTKEY key;
	if(!CryptGenKey(hProv, CALG_RSA_KEYX, flags | CRYPT_EXPORTABLE, &key))
		throw MCException_Win("CryptGenKey failed");
	return key;
CATCH_THROW("CCrypto::CreateRSAKey")
}

HCRYPTKEY CCrypto::ImportRSAPublicKey(const unsigned char* buf, unsigned int len)
{
TRY_CATCH
	CCritSection section(&m_csHashSection);
	HCRYPTKEY key;
	if(!CryptImportKey(hProv, buf, len, NULL, 0, &key))
		throw MCException_Win("CryptImportKey failed");
	return key;
CATCH_THROW("CCrypto::ImportRSAPublicKey")
}

