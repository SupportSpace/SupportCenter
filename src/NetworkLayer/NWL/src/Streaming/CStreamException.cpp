/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStreamException.cpp
///
///  Implements CStreamException class, exceptions for streams.
///
///  @author Dmitry Netrebenko @date 20.09.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CStreamException.h>

tstring GetSystemErrorMessage( DWORD dwErrorCode )
{
	tstring strResult( _T("") );
	LPVOID lpBuffer;

	if ( FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, 
			dwErrorCode, 
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpBuffer, 
			16, 
			NULL) )
	{
		strResult = (LPCTSTR)lpBuffer;
		LocalFree( lpBuffer );
	}
	else
	{
		tostringstream os;
		os << "Unknown error code " << dwErrorCode;
		strResult = os.str();
	}

	tstring::size_type index = strResult.find(_T("\r\n"));
	if(-1 != index)
		strResult = strResult.replace(index,2,_T(""));
	return strResult;
}

CStreamException::CStreamException(const CStreamException &ex) throw()
: CExceptionBase(ex)
{
	m_strWhat = ex.What();
}

CStreamException::CStreamException(const CStreamException &ex, const tstring &strWhatHead) throw()
: CExceptionBase(ex, strWhatHead)
{
	m_strWhat = ex.What();
}

CStreamException::CStreamException( 
			const unsigned int _SrcLine, 
			const PTCHAR _SrcFile, 
			const PTCHAR _SrcDate, 
			const tstring& strWhat )
: CExceptionBase( _SrcLine, _SrcFile, _SrcDate, strWhat )
{
	m_strWhat = strWhat;
}

CStreamException::CStreamException( 
			const unsigned int _SrcLine, 
			const PTCHAR _SrcFile, 
			const PTCHAR _SrcDate, 
			const tstring& strWhat, 
			DWORD dwErrorCode, 
			DWORD internalErrorCode)
: CExceptionBase( _SrcLine, _SrcFile, _SrcDate, strWhat + ". " + GetSystemErrorMessage( dwErrorCode ) , dwErrorCode, internalErrorCode)
{
	m_strWhat = strWhat + ". " + GetSystemErrorMessage( dwErrorCode );
}

CStreamException::~CStreamException()
{
}

CStreamException::operator tstring() const
{
	return m_strWhat;
}

const tstring CStreamException::What() const
{
	return m_strWhat;
}

