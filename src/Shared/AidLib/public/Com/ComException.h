/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  ComException.h
///
///  Declares prologue/epilogue macro for COM methods
///
///  @author Dmitry Netrebenko @date 16.11.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/CException/CException.h>

#ifdef _DEBUG
	#define TRY_CATCH_COM tstring SOME_COM_ERROR(_T(""));\
		try{\
		CCallTracerWrapper CALLTRACER(_T(__FILE__), _T(__FUNCTION__), __LINE__);
#else
	#define TRY_CATCH_COM tstring SOME_COM_ERROR;\
		try{
#endif

#define CATCH_LOG_COM \
return S_OK;\
}\
catch(CExceptionBase *e)	{CExceptionBase ex(*e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__))); SOME_COM_ERROR = ex.what(); MLog_Exception(ex);}\
catch (std::exception *e)	{CExceptionBase ex(__LINE__,_T(__FILE__),_T(__DATE__),e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__))); SOME_COM_ERROR = ex.what(); MLog_Exception(ex);}\
catch(CExceptionBase &e)	{CExceptionBase ex(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__))); SOME_COM_ERROR = ex.what(); MLog_Exception(ex);}\
catch (std::exception &e)	{CExceptionBase ex(__LINE__,_T(__FILE__),_T(__DATE__),&e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__))); SOME_COM_ERROR = ex.what(); MLog_Exception(ex);}\
catch(...)					{CExceptionBase ex(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__)),PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__))); SOME_COM_ERROR = ex.what(); MLog_Exception(ex);}\
return Error(SOME_COM_ERROR.c_str());

#define CATCH_THROW_COM \
}\
catch(_com_error *e)		{throw CExceptionBase(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T(e->ErrorMessage())), PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch(_com_error &e)		{throw CExceptionBase(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),_T(e.ErrorMessage())), PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch (std::exception *e)	{throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),e, PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch(CExceptionBase &e)	{throw CExceptionBase(e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch (std::exception &e)	{throw CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__),&e,PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}\
catch(...)					{throw CExceptionBase(CExceptionBase(__LINE__,_T(__FILE__),_T(__DATE__)),PREPARE_EXEPTION_MESSAGE(_T(__FUNCTION__)));}

/// The function retrieve ErrorInfo information and pack it to tstring
AIDLIB_API tstring TStringFromErrorInfo(void);

/// macros for pass to tstring NULL pointer
#define _OLE2T(x) (((x))?OLE2T((x)):_T(""))