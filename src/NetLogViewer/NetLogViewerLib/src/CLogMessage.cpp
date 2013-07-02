/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLogMessage.cpp
///
///  NetLogMessage COM object
///
///  @author Sogin Max @date 27.03.2007
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CLogMessage.h"
#include <boost/scoped_array.hpp>
#include <atlenc.h>

#pragma warning( disable: 4996 )//<func> was declared deprecated

CLogMessage::CLogMessage()
	:	m_receivedDate(cDate().GetNow())
{
TRY_CATCH
CATCH_LOG()
}


STDMETHODIMP CLogMessage::get_Message(BSTR* pVal)
{
TRY_CATCH
	tstring name = CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_MESSAGE);
	*pVal = CComBSTR(m_logMessage.GetProperty(name).bstrVal);
	//USES_CONVERSION;
	//Log.Add(_MESSAGE_,"%s(%s)",name.c_str(),W2T(m_logMessage.GetProperty(name).bstrVal));
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::put_Message(BSTR newVal)
{
TRY_CATCH
	CComVariant vt(newVal);
	m_logMessage.SetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_MESSAGE), vt);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::get_PID(BSTR* pVal)
{
TRY_CATCH
	*pVal = CComBSTR(m_logMessage.GetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_PID)).bstrVal);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::put_PID(BSTR newVal)
{
TRY_CATCH
	CComVariant vt(newVal);
	m_logMessage.SetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_PID), vt);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::get_TID(BSTR* pVal)
{
TRY_CATCH
	*pVal = CComBSTR(m_logMessage.GetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_TID)).bstrVal);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::put_TID(BSTR newVal)
{
TRY_CATCH
	CComVariant vt(newVal);
	m_logMessage.SetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_TID), vt);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::get_AddedDate(DATE* pVal)
{
TRY_CATCH
	VARIANT vt = m_logMessage.GetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_TIME));
	if (vt.vt == VT_EMPTY)
	{
		//SYSTEMTIME sysTime = static_cast<SYSTEMTIME>(cDate().GetNow());
		//SystemTimeToVariantTime(&sysTime,pVal);
		*pVal = NULL;
	} else
	{
		*pVal = vt.date;
	}
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::put_AddedDate(DATE newVal)
{
TRY_CATCH
	CComVariant vt(newVal);
	m_logMessage.SetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_TIME), vt);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::get_Severity(SHORT* pVal)
{
TRY_CATCH
	*pVal = m_severity;
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::put_Severity(SHORT newVal)
{
TRY_CATCH
	m_severity = newVal;
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::get_CallStack(BSTR* pVal)
{
TRY_CATCH
	*pVal = CComBSTR(m_logMessage.GetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_CALL_STACK)).bstrVal);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::put_CallStack(BSTR newVal)
{
TRY_CATCH
	CComVariant vt(newVal);
	m_logMessage.SetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_CALL_STACK), vt);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::get_FileName(BSTR* pVal)
{
TRY_CATCH
	*pVal = CComBSTR(m_logMessage.GetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_SOURCE_FILE)).bstrVal);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::put_FileName(BSTR newVal)
{
TRY_CATCH
	CComVariant vt(newVal);
	m_logMessage.SetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_SOURCE_FILE), vt);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::get_CompileDate(BSTR* pVal)
{
TRY_CATCH
	*pVal = CComBSTR(m_logMessage.GetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_COMPILATION_DATE)).bstrVal);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::put_CompileDate(BSTR newVal)
{
TRY_CATCH
	CComVariant vt(newVal);
	m_logMessage.SetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_COMPILATION_DATE), vt);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::get_FileLine(SHORT* pVal)
{
TRY_CATCH
	*pVal = m_logMessage.GetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_LINE_NUMBER)).intVal;
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::put_FileLine(SHORT newVal)
{
TRY_CATCH
	CComVariant vt(newVal);
	m_logMessage.SetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_LINE_NUMBER), vt);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::get_ModuleName(BSTR* pVal)
{
TRY_CATCH
	*pVal = CComBSTR(m_logMessage.GetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_PID)).bstrVal);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::put_ModuleName(BSTR newVal)
{
TRY_CATCH
	CComVariant vt(newVal);
	m_logMessage.SetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_PID), vt);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::get_SysError(BSTR* pVal)
{
TRY_CATCH
	CComVariant vt(m_logMessage.GetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_SYSTEM_ERROR)));
	if ( vt.vt == VT_EMPTY || vt.intVal == 0)
	{
		*pVal = NULL;
		return S_OK;
	}
	/// Getting syserror
	PTCHAR Buffer;
	DWORD BufferAllocated;
	//Formatting winerror message
	tstring intVal;
	if(!(BufferAllocated=FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER |
										FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
										NULL, vt.intVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
										(PTCHAR) &Buffer, 16, NULL)))
	{	
		intVal = i2tstring(vt.intVal);
		Buffer = const_cast<PTCHAR>(intVal.c_str());
	}
	*pVal = CComBSTR(Buffer).Copy();
	// Deallocating buffer if it was allocated before
	if (BufferAllocated) 
		LocalFree( Buffer );
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::put_SysError(BSTR newVal)
{
TRY_CATCH
	CComVariant vt(newVal);
	m_logMessage.SetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_SYSTEM_ERROR), vt);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::get_TestCase(BSTR* pVal)
{
TRY_CATCH
	*pVal = CComBSTR(m_logMessage.GetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_UTEST_CASE)).bstrVal);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::put_TestCase(BSTR newVal)
{
TRY_CATCH
	CComVariant vt(newVal);
	m_logMessage.SetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_UTEST_CASE), vt);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::get_TestSuite(BSTR* pVal)
{
TRY_CATCH
	*pVal = CComBSTR(m_logMessage.GetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_UTEST_SUITE)).bstrVal);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::put_TestSuite(BSTR newVal)
{
TRY_CATCH
	CComVariant vt(newVal);
	m_logMessage.SetProperty(CSingleton<CNetLogFieldsMap>::instance().GetFieldIndex(cLog::_FIELD_UTEST_SUITE), vt);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::Decode(BYTE* buffer, LONG size)
{
TRY_CATCH
	m_logMessage.DecodeFromBuffer(reinterpret_cast<char*>(buffer), size);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::get_ReceivedDate(DATE* pVal)
{
TRY_CATCH
	SYSTEMTIME sysTime = static_cast<SYSTEMTIME>(m_receivedDate);
	SystemTimeToVariantTime(&sysTime,pVal);
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::put_ReceivedDate(DATE newVal)
{
TRY_CATCH
	m_receivedDate = newVal;
	return S_OK;
CATCH_LOG_COMERROR()
}

tstring Buf2Hex(const unsigned char *Buf, const unsigned int Size)
{
TRY_CATCH
	if (!Buf || !Size) return _T("");

	PTCHAR Result = new TCHAR[Size*2 + 1];
	if (!Result) throw(MCException("Insufficient memory"));
	Result[Size*2] = 0;

	for(unsigned int i=0;i<Size;i++)
	{
#ifdef _UNICODE
		wsprintf(Result + i*2,_T("%.2X"),Buf[i]);
#else
		std::sprintf(Result + i*2,_T("%.2X"),Buf[i]);
#endif //_UNICODE
	}
	
	tstring Res(Result);
	delete [] Result;
	return Res;

CATCH_THROW("cAssistLib::Buf2Hex");
}


STDMETHODIMP CLogMessage::Serialize2String(BSTR* str)
{
TRY_CATCH
	boost::scoped_array<char> buf;
	int bufSize = m_logMessage.GetEncodedSize();
	buf.reset(new char[bufSize]);

	boost::scoped_array<char> hexBuf;
	int hexBufSize = bufSize*2 + 1;
	hexBuf.reset(new char[hexBufSize]);

	m_logMessage.EncodeToBuffer(buf.get(), bufSize);
	if (Base64Encode(	reinterpret_cast<BYTE*>(buf.get()),
						bufSize,
						reinterpret_cast<LPSTR>(hexBuf.get()),
						&hexBufSize ) == FALSE)
	{
		throw MCException("Base64Encode failed");
	}
	hexBuf.get()[hexBufSize - 1] = 0;
	USES_CONVERSION;
	*str = CComBSTR(A2W(hexBuf.get())).Copy();
	return S_OK;
CATCH_LOG_COMERROR()
}

STDMETHODIMP CLogMessage::DeserializeFromString(BSTR str)
{
TRY_CATCH
	CComBSTR bstr(str);
	int size = Base64DecodeGetRequiredLength( bstr.Length() );
	boost::scoped_array<BYTE> raw( new BYTE[size] );
	USES_CONVERSION;
	if( Base64Decode( W2A(bstr.m_str), bstr.Length(), raw.get(), &size ) == FALSE )
	{
		throw MCException("Base64Decode failed");
	}
	m_logMessage.DecodeFromBuffer(reinterpret_cast<char*>(raw.get()), size);
	return S_OK;
CATCH_LOG_COMERROR()
}
