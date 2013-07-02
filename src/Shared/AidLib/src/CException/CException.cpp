/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CExceptionBase.h
///
///  Base exception class
///
///  @author "Archer Software" Sogin M. @date 21.09.2006
///
///  @modified Alexander Novak @date 19.11.2007
///
///  Added Error code support
///
////////////////////////////////////////////////////////////////////////


#include <AidLib/CException/CException.h>

#ifdef _UNICODE
	#include <atlbase.h>
	#include <atlconv.h>
#endif //_UNICODE


//dwErrorCode - Windows error code, internalErrorCode - Error code for internal usage
CExceptionBase::CExceptionBase(	const unsigned int _SrcLine,
								const PTCHAR _SrcFile,
								const PTCHAR _SrcDate,
								const tstring &strWhat,
								DWORD dwErrorCode,
								DWORD internalErrorCode) throw( )
	:	STD_EXCEPTION_FROM_TCHAR(strWhat.c_str()), 
		m_SrcLine(_SrcLine), 
		m_SrcFile(_SrcFile), 
		m_SrcDate(_SrcDate), 
		m_WhatHead(_T("")),
		m_winErrorCode(dwErrorCode),
		m_internalErrorCode(internalErrorCode)
{
	try
	{
		LPVOID lpBuffer;
		if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR ) &lpBuffer, 16, NULL) != 0)
		{
			m_strWhat = strWhat + _T(". (0x") + i2tstring(dwErrorCode,16) + _T(") ") + (PTCHAR) lpBuffer;
			LocalFree( lpBuffer );
		}
		else
		{
			tstring str;
			str += strWhat + _T(". Unknown WinError code ") + i2tstring(dwErrorCode,16);
			m_strWhat = str;
		}
		tstring::size_type index = m_strWhat.find(_T("\r\n"));
		if(-1 != index)
			m_strWhat = m_strWhat.replace(index,2,_T(""));
	}
	catch(...)
	{
	}
}

#ifndef MSG_BUF_SIZE 
	#define MSG_BUF_SIZE 4096
#endif

CExceptionBase::CExceptionBase(	const unsigned int _SrcLine,
								const PTCHAR _SrcFile,
								const PTCHAR _SrcDate,
								TCHAR *Format,
								...) throw( )	
	:	STD_EXCEPTION_FROM_TCHAR(Format),
		m_SrcLine(_SrcLine), 
		m_SrcFile(_SrcFile), 
		m_SrcDate(_SrcDate), 
		m_WhatHead(_T("")),
		m_winErrorCode(0),
		m_internalErrorCode(0)
{
try
{
	TCHAR Mess[MSG_BUF_SIZE];
	va_list args;
	va_start( args, Format );	// Init the user arguments list.
#ifndef _UNICODE
	vsprintf_s(Mess, MSG_BUF_SIZE, Format, args );
#else
	vswprintf_s(Mess, MSG_BUF_SIZE, Format, args );
#endif //_UNICODE
	m_strWhat = Mess;
}
catch(...)
{
}
}

CExceptionBase::CExceptionBase(	const unsigned int _SrcLine,
								const PTCHAR _SrcFile,
								const PTCHAR _SrcDate,
								DWORD dwErrorCode,
								TCHAR *Format,
								...) throw( )
	:	STD_EXCEPTION_FROM_TCHAR(Format),
		m_SrcLine(_SrcLine), 
		m_SrcFile(_SrcFile), 
		m_SrcDate(_SrcDate), 
		m_WhatHead(_T("")),
		m_winErrorCode(0),
		m_internalErrorCode(0)
{
try
{
	TCHAR Mess[MSG_BUF_SIZE];
	va_list args;
	va_start( args, Format );	// Init the user arguments list.
#ifndef _UNICODE
	vsprintf_s(Mess, MSG_BUF_SIZE, Format, args );
#else
	vswprintf_(Mess, MSG_BUF_SIZE, Format, args );
#endif //_UNICODE
	m_strWhat += Mess;

	LPVOID lpBuffer;
	if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR ) &lpBuffer, 16, NULL) != 0)
	{
		m_strWhat =   Mess;
		m_strWhat += _T(". ");
		m_strWhat += (PTCHAR) lpBuffer;
		LocalFree( lpBuffer );
		tstring::size_type index = m_strWhat.find(_T("\r\n"));
		if(-1 != index)
			m_strWhat = m_strWhat.replace(index,2,_T(""));
	}
	else
	{
		tstring str;
		str += tstring(Mess) + _T(". Unknown WinError code ") + i2tstring(dwErrorCode);
		m_strWhat = str;
	}
}
catch(...)
{
}
}


CExceptionBase::CExceptionBase(	const CExceptionBase &ex,
								const tstring &strWhatHead) throw() 
	:	STD_EXCEPTION_FROM_TCHAR(ex.m_strWhat.c_str()),
		m_SrcLine(ex.m_SrcLine),
		m_SrcFile(ex.m_SrcFile),
		m_SrcDate(ex.m_SrcDate),
		m_winErrorCode(ex.m_winErrorCode),
		m_internalErrorCode(ex.m_internalErrorCode)
{
	try
	{
		tstring str = ex.m_WhatHead;
		if(!str.empty())
			str += EX_DEVIDER;
		m_strWhat = ex.m_strWhat;
		m_WhatHead = str + strWhatHead;
	}
	catch(...)
	{
	}
}


CExceptionBase::CExceptionBase(	const unsigned int _SrcLine,
								const PTCHAR _SrcFile,
								const PTCHAR _SrcDate,
								const std::exception *ex,
								const tstring &strWhatHead) throw( ) 
	:	STD_EXCEPTION_FROM_TCHAR(NULL!=ex?*ex:"Unknown exception"),
		m_SrcLine(_SrcLine), 
		m_SrcFile(_SrcFile),
		m_SrcDate(_SrcDate),
		m_winErrorCode(0),
		m_internalErrorCode(0)
{
try
{
	tstring str;
	
#ifdef _UNICODE
	USES_CONVERSION;
	str += A2T(ex->what());
#else
	str += ex->what();
#endif //_UNICODE

	m_strWhat = str;
	m_WhatHead = strWhatHead;
}
catch(...)
{
}
}

/*
#ifdef __AFX_H__
CExceptionBase::CExceptionBase(const unsigned int _SrcLine, const PTCHAR _SrcFile, const PTCHAR _SrcDate, CException *ex, const tstring &strWhatHead) throw( ) 
	: m_SrcLine(_SrcLine), m_SrcFile(_SrcFile), m_SrcDate(_SrcDate)
{
try
{
		tstring str;
		TCHAR Buf[MAX_EX_LEN];\
		ex->GetErrorMessage(Buf,MAX_EX_LEN,NULL);\
		str += Buf;
		m_WhatHead = strWhatHead;
		m_strWhat = str;
}
catch(...)
{
}
}
#endif
*/

CExceptionBase& CExceptionBase::operator= (const CExceptionBase &ex) throw( )
{
try
{
	if( &ex != this )
	{
		m_strWhat			= ex.m_strWhat;
		m_SrcLine			= ex.m_SrcLine;
		m_SrcFile			= ex.m_SrcFile;
		m_SrcDate			= ex.m_SrcDate;
		m_WhatHead			= ex.m_WhatHead;
		m_winErrorCode		= ex.m_winErrorCode;
		m_internalErrorCode	= ex.m_internalErrorCode;
	}
	return *this;
}
catch(...)
{
	return *this;
}
}

CExceptionBase::operator std::exception () const throw( )
{
try
{
	tstring what(_T("\n"));
	cBuffLog bufLog;
	bufLog.Add(*this);
	unsigned int len;
	PTCHAR buf = bufLog.GetLogBuffer(&len);
	what += buf;
	delete [] buf;
	return std::exception(what.c_str());
	//return std::exception(m_strWhat.c_str());
}
catch(...)
{
	return std::exception("std::exception CExceptionBase::operator() failed");
}
}


const char* CExceptionBase::what( ) const throw( )
{
try
{
#ifdef _UNICODE
	USES_CONVERSION;
	tstring what(_T("\n"));
	cBuffLog bufLog;
	bufLog.Add(*this);
	unsigned int len;
	PTCHAR buf = bufLog.GetLogBuffer(&len);
	what += buf;
	delete [] buf;
	return (m_stdWhat = T2A(what.c_str())).c_str();
	//return T2A(m_strWhat.c_str());
#else
	tstring what(_T("\n"));
	cBuffLog bufLog;
	bufLog.Add(*this);
	unsigned int len;
	PTCHAR buf = bufLog.GetLogBuffer(&len);
	what += buf;
	delete [] buf;
	return (m_stdWhat = what.c_str()).c_str();
	//return m_strWhat.c_str();
#endif //_UNICODE
}
catch(...)
{
	return "Erron in exception handling";
}
}

tstring CExceptionBase::Format(const tstring Format, ...) throw()
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
		Result = _T("CExceptionBase::Format Exception happened");
	}
	catch(...)
	{
		//One more exception happened
		//going further
	}
}
	return Result;
}

CExceptionBase::~CExceptionBase() throw( )
{
}