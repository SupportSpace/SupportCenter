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
///  Added error code support
///
////////////////////////////////////////////////////////////////////////



#ifndef CEXCEPTION_H
#define CEXCEPTION_H
//-------------------------------------------------------------------------------

#include <AidLib/Strings/tstring.h>
#include <exception>
#include <windows.h>
#include <winnt.h>
#include <AidLib/AidLib.h>
#include <AidLib/Logging/cLog.h>
#include <AidLib/Logging/CCallTracerWrapper.h>
#include <AidLib/Logging/CInstanceTracker.h>

#define EX_DEVIDER _T("\r\n")
//_T("-> ")
#define MAX_EX_LEN 1024

#if defined _UNICODE || defined UNICODE
	#define STD_EXCEPTION_FROM_TCHAR(text) std::exception("Convertion to std::exception in unicode doesn't supported")
#else
	#define STD_EXCEPTION_FROM_TCHAR(text) std::exception(text)
#endif



//Base class for exceptions
class AIDLIB_API CExceptionBase : public std::exception
{
	friend class cLog;
private:
	///Information about source file
	///Line in source file, where exception where thrown
	unsigned int m_SrcLine;

	///Name of source file, where exception where thrown
	tstring m_SrcFile;

	///Data of compilation of  source file, where exception where thrown
	tstring m_SrcDate;
	
	///Windows error code, set by SetLastError api function
	DWORD m_winErrorCode;

protected:
	///Internal error code
	DWORD m_internalErrorCode;

public:
	///Full exception description 
	tstring m_strWhat;	// Exception reason
	///temp string for std::exception what()
	mutable std::basic_string<char> m_stdWhat;

	///Call stack
	tstring m_WhatHead;

	CExceptionBase(	const unsigned int _SrcLine,
					const PTCHAR _SrcFile,
					const PTCHAR _SrcDate) throw()
		:	m_strWhat(_T("Unknown exception")),
			STD_EXCEPTION_FROM_TCHAR(_T("Unknown exception")),
			m_SrcLine(_SrcLine),
			m_SrcFile(_SrcFile),
			m_SrcDate(_SrcDate),
			m_WhatHead(_T("")),
			m_winErrorCode(0),
			m_internalErrorCode(0){};

	CExceptionBase(	const unsigned int _SrcLine,
					const PTCHAR _SrcFile,
					const PTCHAR _SrcDate,
					const tstring &strWhat) throw( )
		:	m_strWhat(strWhat),
			STD_EXCEPTION_FROM_TCHAR(strWhat.c_str()),
			m_SrcLine(_SrcLine),
			m_SrcFile(_SrcFile),
			m_SrcDate(_SrcDate),
			m_WhatHead(_T("")),
			m_winErrorCode(0),
			m_internalErrorCode(0){};

	CExceptionBase(const CExceptionBase &ex) throw( )
		:	m_strWhat(ex.m_strWhat),
			STD_EXCEPTION_FROM_TCHAR(ex.m_strWhat.c_str()),
			m_SrcLine(ex.m_SrcLine),
			m_SrcFile(ex.m_SrcFile),
			m_SrcDate(ex.m_SrcDate),
			m_WhatHead(ex.m_WhatHead),
			m_winErrorCode(ex.m_winErrorCode),
			m_internalErrorCode(ex.m_internalErrorCode){};

	CExceptionBase(	const CExceptionBase &ex, const tstring &strWhatHead) throw( );

	CExceptionBase(	const unsigned int _SrcLine,
					const PTCHAR _SrcFile,
					const PTCHAR _SrcDate,
					const std::exception *ex,
					const tstring &strWhatHead) throw( );

	CExceptionBase(	const unsigned int _SrcLine,
					const PTCHAR _SrcFile,
					const PTCHAR _SrcDate,
					TCHAR *Format, ...) throw( );

	CExceptionBase(	const unsigned int _SrcLine,
					const PTCHAR _SrcFile,
					const PTCHAR _SrcDate,
					DWORD dwErrorCode,
					TCHAR *Format,
					...) throw( );

#ifdef __AFX_H__
	CExceptionBase(const unsigned int _SrcLine, const PTCHAR _SrcFile, const PTCHAR _SrcDate, CException *ex, const tstring &strWhatHead) throw( )
		: m_SrcLine(_SrcLine), m_SrcFile(_SrcFile), m_SrcDate(_SrcDate)
	{
		try
		{
			tstring str;
			TCHAR Buf[MAX_EX_LEN];
			ex->GetErrorMessage(Buf,MAX_EX_LEN,NULL);
			str += Buf;
			m_WhatHead = strWhatHead;
			m_strWhat = str;
		}
		catch(...)
		{
		}	
	}
#endif __AFX_H__
		
	//dwErrorCode - Windows error code, internalErrorCode - Error code for internal usage
	CExceptionBase(	const unsigned int _SrcLine,
					const PTCHAR _SrcFile,
					const PTCHAR _SrcDate,
					const tstring &strWhat,
					DWORD dwErrorCode,
					DWORD internalErrorCode = 0) throw( );
	
	inline DWORD GetInternalErrorCode()
	{
		return m_internalErrorCode;
	}

	inline DWORD GetWindowsErrorCode()
	{
		return m_winErrorCode;
	}
	
	virtual ~CExceptionBase( ) throw( );
	
	CExceptionBase& operator=(const CExceptionBase& ex) throw( );	

	virtual const char* what( ) const throw( );

	static tstring Format(const tstring Format, ...) throw( );

	operator std::exception () const throw( );
};

#define MCException_ErrCode(What, InternalErrCode) CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T(What)),0,InternalErrCode)
#define MCException_ErrCode_Win(What, InternalErrCode) CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T(What)),GetLastError(),InternalErrCode)

#define MCException_Win(What) CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),tstring(_T(What)),GetLastError())
#define MCException(What) CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T(What))


#define _M_THIS_FILE_  __LINE__,_T(__FILE__),_T(__DATE__)

#ifdef _DEBUG
	#define ENABLE_TRACE CCallTracer::instance().EnableTrace(true);
	#define DISABLE_TRACE CCallTracer::instance().EnableTrace(false);
#else
	#define ENABLE_TRACE 
	#define DISABLE_TRACE 
#endif


/// Creats CExceptionBase for current file/line with formatted output support
/// @param args arg list. First argument must be _M_THIS_FILE_
#define MCException_F(args) CExceptionBase args

//Here we can define trace or do not trace exceptions by our macro
#define TRACE_EXCEPTIONS 1

#if TRACE_EXCEPTIONS

//This macro starts exceptions handling block
#ifdef _DEBUG
	#define TRY_CATCH try{\
		CCallTracerWrapper CALLTRACER(_T(__FILE__), _T(__FUNCTION__), __LINE__);
#else
	#define TRY_CATCH try{
#endif

#define PREPARE_EXEPTION_MESSAGE(Str)\
	CExceptionBase::Format(_T("%s %s %s:%d"),Str,tstring(_T("                    ")).substr(0,max(0,20-tstring(Str).length())).c_str(),_T(__FILE__),__LINE__)

#ifdef __AFX_H__ //---------------------WITH MFC----------------------------------------------------------

#define CATCH_LEAVE_THROW(a,b) CATCH_THROW(a)

//This macro ends exceptons handling block
//and uses to trace functions call chains
#define CATCH_THROW(...)\
}\
catch(CExceptionBase *e)		{throw CExceptionBase(*e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch (std::exception *e)	{throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch(CExceptionBase &e)		{throw CExceptionBase(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch (std::exception &e)	{throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),&e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch(...)					{throw CExceptionBase(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__)),PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}


//This macro ends exceptions chains
//end log result as we want to log
#define CATCH_LOG(...)\
}\
catch(CExceptionBase *e)		{MLog_Exception((CExceptionBase(*e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)))));}\
catch (std::exception *e)	{MLog_Exception((CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)))));}\
catch(CExceptionBase &e)		{MLog_Exception((CExceptionBase(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)))));}\
catch (std::exception &e)	{MLog_Exception((CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),&e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)))));}\
catch(...)					{MLog_Exception((CExceptionBase(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__)),PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)))));}


#else  //__AFX_H__ ---------------------WITHOUT MFC--------------------------------------------------------

#define CATCH_THROW(...)\
}\
catch (std::exception *e)	{throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),e, PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch(CExceptionBase &e)	{throw CExceptionBase(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch (std::exception &e)	{throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),&e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch(...)					{throw CExceptionBase(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__)),PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}

//This macro ends exceptions chains
//end log result as we want to log
#define CATCH_LOG(...)\
}\
catch(CExceptionBase *e)	{MLog_Exception((CExceptionBase(*e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)))));}\
catch (std::exception *e)	{MLog_Exception((CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)))));}\
catch(CExceptionBase &e)	{MLog_Exception((CExceptionBase(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)))));}\
catch (std::exception &e)	{MLog_Exception((CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),&e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)))));}\
catch(...)					{MLog_Exception((CExceptionBase(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__)),PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)))));}
#endif //__AFX_H__




#endif //TRACE_EXCEPTIONS


//-------------------------------------------------------------------------------
#endif //CEXCEPTION_H
