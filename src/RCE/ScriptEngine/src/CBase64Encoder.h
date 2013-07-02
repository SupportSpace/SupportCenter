/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CBase64Encoder.h
///
///  Declares CBase64Encoder class, responsible for encoding/decoding binary
///    data to string
///
///  @author Dmitry Netrebenko @date 17.12.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <atlenc.h>
#include <boost/shared_array.hpp>
#include <AidLib/Strings/tstring.h>

/// CBase64Encoder class, responsible for encoding/decoding binary
///   data to string
class CBase64Encoder
{
private:
/// Prevents construction of CBase64Encoder objects
	CBase64Encoder();
public:
/// Destructor
	~CBase64Encoder();
/// Encodes data to string
/// @param data - buffer with data
/// @param dataSize - size of data
/// @return string with encoded data
	static tstring Encode(boost::shared_array<char> data, const int dataSize);
/// Decodes data from string into buffer
/// @param str - string with encoded data
/// @param encodedSize - pointer where size of decoded data will be placed
/// @return buffer with decoded data
	static boost::shared_array<char> Decode(const tstring& str, int* decodedSize);
};
