/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStreamException.h
///
///  Declares CStreamException class, exceptions for streams.
///
///  @author Dmitry Netrebenko @date 20.09.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>
#include <AidLib/CException/CException.h>
#include <NWL/NetworkLayer.h>

///  Exception class for streams
///  Base class is CExceptionBase
class NWL_API CStreamException :
	public CExceptionBase
{
private:
	
public:
///  Constructor
///
///  @param   Line number
///  @param   Source file name
///  @param   Date
///  @param   Reason
	CStreamException( const unsigned int, const PTCHAR, const PTCHAR, const tstring& );

///  Constructor for system errors
///
///  @param   Line number
///  @param   Source file name
///  @param   Date
///  @param   Reason
///  @param   System error code
///  @param   internalErrorCode internal error code
	CStreamException( const unsigned int, const PTCHAR, const PTCHAR, const tstring&, DWORD, DWORD internalErrorCode = 0);

	CStreamException(const CStreamException &ex);
	CStreamException(const CStreamException &ex, const tstring &strWhatHead);

///  Destructor
	~CStreamException();

public:
///  Conversion operator
	operator tstring() const;

///  Returns exception's message
///  string message
	virtual const tstring What() const;

	/// Accessor to exception message.
	__declspec( property( get=What ) ) const tstring what;
	
};

#define MCStreamException_ErrCode(What, InternalErrCode) CStreamException(__LINE__,_T(__FILE__),_T(__DATE__),_T(What),DWORD(0),DWORD(InternalErrCode))
#define MCStreamException_ErrCode_Win(What, InternalErrCode) CStreamException(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T(What)),DWORD(Code),DWORD(InternalErrCode))

#define MCStreamException_Win(What,Code) CStreamException(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T(What)),DWORD(Code))
#define MCStreamException(What) CStreamException(__LINE__,_T(__FILE__),_T(__DATE__),_T(What))
#define MCSocketStreamException(What) MCStreamException_Win(What,WSAGetLastError())

#pragma warning( push )
#pragma warning( disable : 4005 )
#define CATCH_THROW(...)\
}\
catch(CStreamException &e)		{throw CStreamException(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch (std::exception *e)	{throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),e, PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch(CExceptionBase &e)		{throw CExceptionBase(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch (std::exception &e)	{throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),&e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch(...)					{throw CExceptionBase(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__)),PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}
#pragma warning( pop )
