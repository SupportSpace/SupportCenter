#pragma once

#include <string>
#include <tchar.h>
#include <sstream>
#include <AidLib/AidLib.h>
#include <AidLib/Allocator/lib_allocator.h>
#include <vector>

typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, nwl_allocator<TCHAR> > AIDLIB_API tstring;
typedef std::basic_ostringstream<TCHAR, std::char_traits<TCHAR>, nwl_allocator<TCHAR> > AIDLIB_API tostringstream;
typedef TCHAR* PTCHAR;

/// converts int to tstring
/// @param i integer to convert
/// @param radix Base of i; which must be in the range 2–36.
/// @return string
AIDLIB_API tstring i2tstring(const int i,const int radix=10);

/// Formats string like sprintf do
/// @arg Format @see sprintf
/// @return formatted string
AIDLIB_API tstring Format(const tstring Format, ...);

/// Generates GUID
/// @return new GUID as string
AIDLIB_API tstring GetGUID();

/// Converts tsring to lower case
/// @param reference to original string
AIDLIB_API tstring LowerCase( const tstring& );

/// Converts tsring to upper case
/// @param reference to original string
AIDLIB_API tstring UpperCase( const tstring& );

/// Returns path to module
AIDLIB_API tstring GetModulePath(void* hInstance);

///	Returns path of special folder
AIDLIB_API tstring GetSpecialFolderPath(int csidl);

/// Removes trailing slashes from the end of the path
AIDLIB_API tstring RemoveTrailingSlashes(const tstring &path);

/// Tokenizes string
/// @param str - string to split on tokens
/// @param delimiters - string, containing tokens delimiters
AIDLIB_API std::vector<tstring> tokenize(const tstring& str, const tstring& delimiters);