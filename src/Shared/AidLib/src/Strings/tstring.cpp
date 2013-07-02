#include <AidLib/Strings/tstring.h>
#include <stdarg.h>
#include <Rpc.h>
#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>
#include <boost/shared_array.hpp>
#include <shlobj.h>
AIDLIB_API tstring i2tstring(const int i, const int radix)
{
	TCHAR buf[30];
	_itot_s(i,buf,radix);
	return buf;
}

#ifndef MSG_BUF_SIZE
#define MSG_BUF_SIZE 4096
#endif
AIDLIB_API tstring Format(const tstring Format, ...)
{
PTCHAR Mess(NULL);
tstring Result;
try
{
	va_list args;
	va_start( args, Format );	// Init the user arguments list.

	unsigned int i=1;
	unsigned mult=1;
	int result;
	for(Mess=new TCHAR[MSG_BUF_SIZE];
		(result=_vsntprintf_s( Mess, MSG_BUF_SIZE*mult,_TRUNCATE, Format.c_str(), args ))<0 && i<MSG_BUF_SIZE_DOUBLING;
		i++, mult <<= 1)
	{
		delete [] Mess;
		Mess = NULL;
		Mess=new TCHAR[MSG_BUF_SIZE * (mult << 1)];
	}

	if (result<0)
	{
		TCHAR truncated[]=_T("MESSAGE TRUNCATED");
		_tcscpy_s(Mess+MSG_BUF_SIZE*mult-_countof(truncated)-1,_countof(truncated),truncated);
	}
	Result = Mess;

	delete [] Mess;
	Mess = NULL;
}
catch(...)
{
	try
	{
		if (Mess) delete [] Mess; //Preventing memory leaks
		Result = _T("Format Exception happened");
	}
	catch(...)
	{
		//One more exception happened
		//going further
	}
}
	return Result;
}

AIDLIB_API tstring GetGUID()
{
TRY_CATCH

	//Generating UUID
	UUID Uuid;
	RPC_STATUS Status=UuidCreate(&Uuid);
	if (Status!=RPC_S_OK && Status!=RPC_S_UUID_LOCAL_ONLY)
	{
		if (Status==RPC_S_UUID_NO_ADDRESS) throw MCException("Failed to UuidCreate: Cannot get Ethernet or token-ring hardware address for this computer.");
		throw MCException_Win("Failed to UuidCreate: ");
	}

	//Converting UUID to string
	#ifdef _UNICODE
		unsigned short* UuidStr;
	#else
		unsigned char *UuidStr;
	#endif //_UNICODE
	Status = UuidToString(&Uuid, &UuidStr);
	if (Status!=RPC_S_OK)
	{
		if (Status==RPC_S_OUT_OF_MEMORY) throw MCException("Failed to UuidToString: The system is out of memory.");
		throw MCException_Win("Failed to UuidToString: ");
	}
	tstring Result(reinterpret_cast<PTCHAR>(UuidStr));

	//Freeing string, allocated by UuidToString
	RpcStringFree(&UuidStr);

	return Result;

CATCH_THROW("GetGUID")
}

AIDLIB_API tstring LowerCase( const tstring& str )
{
TRY_CATCH

	if ( !str.length() ) 
		return tstring();

	boost::shared_array<TCHAR> buf( new TCHAR[str.length() + 1] );
	_tcscpy_s( buf.get(), str.length() + 1 , str.c_str() );
	tstring Result = CharLower( buf.get() );
	return Result;

CATCH_THROW("LowerCase")
}

AIDLIB_API tstring UpperCase( const tstring& str )
{
TRY_CATCH

	if ( !str.length() ) 
		return tstring();

	boost::shared_array<TCHAR> buf( new TCHAR[str.length() + 1] );
	_tcscpy_s( buf.get(), str.length() + 1, str.c_str() );
	tstring Result = CharUpper( buf.get() );
	return Result;

CATCH_THROW("UpperCase")
}

AIDLIB_API tstring GetModulePath(void* hInstance)
{
TRY_CATCH
	TCHAR modName[MAX_PATH];
	if (!GetModuleFileName(reinterpret_cast<HMODULE>(hInstance), modName, MAX_PATH))
	{
		throw MCException_Win("Failed to GetModuleName");
	}
	//Path of a module
	tstring path=modName;
	int lastPos = path.find_last_of(_T('\\'));
	if (tstring::npos !=lastPos)
		path = path.substr(0,lastPos);
	return path;
CATCH_THROW()
}

AIDLIB_API tstring GetSpecialFolderPath(int csidl)
{
TRY_CATCH
	TCHAR pathBuf[32768];
	if(!SHGetSpecialFolderPath(NULL,pathBuf,csidl,TRUE))
		throw MCException_Win("Failed to SHGetSpecialFolderPath");
	tstring path=pathBuf;
	return path;
CATCH_THROW()
}

AIDLIB_API tstring RemoveTrailingSlashes(const tstring &_path)
{
TRY_CATCH
	
	tstring path(_path);
	while(!path.empty() && (*(path.end()-1) == _T('\\') || *(path.end()-1) == _T('/')))
		path.erase(path.end()-1,path.end());
	return path;

CATCH_THROW()
}

AIDLIB_API std::vector<tstring> tokenize(const tstring& str, const tstring& delimiters)
{
TRY_CATCH

	std::vector<tstring> tokens;

	// skip delimiters at beginning.
	tstring::size_type lastPos = str.find_first_not_of(delimiters, 0);

	// find first "non-delimiter".
	tstring::size_type pos = str.find_first_of(delimiters, lastPos);

	while (tstring::npos != pos || tstring::npos != lastPos)
	{
		// found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));

		// skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);

		// find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}

	return tokens;

CATCH_THROW()
}