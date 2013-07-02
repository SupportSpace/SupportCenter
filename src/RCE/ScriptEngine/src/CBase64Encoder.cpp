/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CBase64Encoder.cpp
///
///  Implements CBase64Encoder class, responsible for encoding/decoding binary
///    data to string
///
///  @author Dmitry Netrebenko @date 17.12.2007
///
////////////////////////////////////////////////////////////////////////

#include "CBase64Encoder.h"
#include <AidLib/CException/CException.h>

CBase64Encoder::~CBase64Encoder()
{
TRY_CATCH
CATCH_LOG()
}

tstring CBase64Encoder::Encode(boost::shared_array<char> data, const int dataSize)
{
TRY_CATCH
	int encSize = dataSize * 2 + 1;
	boost::shared_array<char> buf(new char[encSize]);
	memset(buf.get(), 0, encSize);
	BOOL encResult = Base64Encode(
		reinterpret_cast<BYTE*>(data.get()), 
		dataSize, 
		buf.get(), 
		&encSize, 
		ATL_BASE64_FLAG_NOPAD | ATL_BASE64_FLAG_NOCRLF);
	if(FALSE == encResult)
		throw MCException(_T("Base64Encode failed."));
	tstring result(buf.get());
	return result;
CATCH_THROW()
}

boost::shared_array<char> CBase64Encoder::Decode(const tstring& str, int* decodedSize)
{
TRY_CATCH
	if(!decodedSize)
		throw MCException(_T("Invalid pointer (decodedSize)"));
	int strSize = static_cast<int>(str.length());
	boost::shared_array<char> buf(new char[strSize]);
	int decSize = strSize;
	BOOL decResult = Base64Decode(
		str.c_str(),
		strSize,
		reinterpret_cast<BYTE*>(buf.get()),
		&decSize);
	if(FALSE == decResult)
		throw MCException(_T("Base64Decode failed."));
	*decodedSize = decSize;
	return buf;
CATCH_THROW()
}
