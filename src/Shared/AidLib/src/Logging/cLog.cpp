//===========================================================================
// Archer Software.
//                                   cLog.cpp
//
//---------------------------------------------------------------------------
// Class for logging everything
//---------------------------------------------------------------------------
//
// Version : 01.00
// By      : Max Sogin
// Date    : 7/14/05 12:56:00 PM
//===========================================================================
#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif //_DEBUG

//#define _WIN32_WINNT 0x0400

#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>
#include <sstream>
#include <process.h>
#include <windows.h>
#include <set>
#include <algorithm>
#include <boost/bind.hpp>
#include <AidLib/CTime/CTime.h>
#include <AidLib/Logging/CLogVariant.h>

using namespace std;

#include <atlbase.h>
#include <atlconv.h>


///  Critical section wrapper
///  @remarks
class cCritSection
{
protected:
	LPCRITICAL_SECTION cs;
public:
	cCritSection(const LPCRITICAL_SECTION &_cs) 
		: cs(_cs)
	{
		EnterCriticalSection(cs);
	}

	virtual ~cCritSection()
	{
		LeaveCriticalSection(cs);
	}
};


typedef struct tagADSERRMSG
{
    HRESULT    hr;
    LPCTSTR    pszError;
}ADSERRMSG;


bool cLog::cEventDesc::ModuleNameReceived = false;
TCHAR cLog::cEventDesc::ModuleName[MAX_PATH];

cLog::sSeverity AIDLIB_API cLog::m_severities[] = 
{
	{ _MESSAGE,			_T("Message"),		_FIELD_TIME | _FIELD_PID | _FIELD_TID | _FIELD_MESSAGE },
	{ _WARNING,			_T("Warning"),		_FIELD_TIME | _FIELD_PID | _FIELD_TID | _FIELD_SOURCE_FILE | _FIELD_LINE_NUMBER | _FIELD_COMPILATION_DATE | _FIELD_SYSTEM_ERROR | _FIELD_MESSAGE },
	{ _ERROR,			_T("Error"),		_FIELD_TIME | _FIELD_PID | _FIELD_TID | _FIELD_SOURCE_FILE | _FIELD_LINE_NUMBER | _FIELD_COMPILATION_DATE | _FIELD_SYSTEM_ERROR | _FIELD_MESSAGE },
	{ _EXCEPTION,		_T("Exception"),	_FIELD_TIME | _FIELD_PID | _FIELD_TID | _FIELD_SOURCE_FILE | _FIELD_LINE_NUMBER | _FIELD_COMPILATION_DATE | _FIELD_SYSTEM_ERROR | _FIELD_MESSAGE | _FIELD_CALL_STACK },
	{ _UTEST_SUITE,		_T("UTest Suite"),	_FIELD_UTEST_SUITE | _FIELD_MESSAGE },
	{ _UTEST_CASE,		_T("UTest Case"),	_FIELD_UTEST_CASE | _FIELD_MESSAGE },
	{ _FTEST_SUITE,		_T("FTest Suite"),	_FIELD_FTEST_SUITE | _FIELD_MESSAGE },
	{ _FTEST_CASE,		_T("FTest Case"),	_FIELD_FTEST_CASE | _FIELD_MESSAGE }
};

cLog::sInformField AIDLIB_API cLog::m_informFields[] =
{
	{ _FIELD_TIME,					_T(""),						boost::bind( &cEventDesc::GetTimeString, _1, _2 )				},
	{ _FIELD_MESSAGE,				_T("Msg"),					NULL															},
	{ _FIELD_PID,					_T("pid"),					boost::bind( &cEventDesc::GetPidString, _1, _2 )				},
	{ _FIELD_TID,					_T("tid"),					boost::bind( &cEventDesc::GetTidString, _1, _2 )				},
	{ _FIELD_SOURCE_FILE,			_T("Source file"),			boost::bind( &cEventDesc::GetSourceFileString, _1, _2 )			},
	{ _FIELD_LINE_NUMBER,			_T("Line number"),			boost::bind( &cEventDesc::GetSourceLineString, _1, _2 )			},
	{ _FIELD_COMPILATION_DATE,		_T("Compilation date"),		boost::bind( &cEventDesc::GetCompilationDateString, _1, _2 )	},
	{ _FIELD_SYSTEM_ERROR,			_T("System error number"),	boost::bind( &cEventDesc::GetSystemErrorString, _1, _2 )		},
	{ _FIELD_UTEST_SUITE,			_T("UTest suite"),			boost::bind( &cEventDesc::GetTestSuiteString, _1, _2 )			},
	{ _FIELD_UTEST_CASE,			_T("UTest case"),			boost::bind( &cEventDesc::GetTestCaseString, _1, _2 )			},
	{ _FIELD_FTEST_SUITE,			_T("FTest suite"),			boost::bind( &cEventDesc::GetTestSuiteString, _1, _2 )			},
	{ _FIELD_FTEST_CASE,			_T("FTest case"),			boost::bind( &cEventDesc::GetTestCaseString, _1, _2 )			},
	{ _FIELD_CALL_STACK,			_T("Call stack"),			boost::bind( &cEventDesc::GetCallStackString, _1, _2 )			}
};



TCHAR* cLog::m_logFormats[MAX_LOG_FORMAT][MAX_LOG_MESSAGE_STRING] = 
{
	{
		_T("<%severity%>"),
		_T("%fieldname%%tab%%value%; "),
		_T("</%severity%>\n")
	},
	{
		_T("%severity%;"),
		_T("%fieldname% %value%;"),
		_T("\n")
	}
};

TCHAR* cLog::m_logMessageParameters[] = 
{
	_T("%severity%"),
	_T("%fieldname%"),
	_T("%value%"),
	_T("%tab%")
};

TCHAR* cLog::m_verbosityNames[] =
{
	_T("_NO_TRACE_"),
	_T("_TRACE_DEBUG_"),
	_T("_TRACE_STATES_"),
	_T("_TRACE_CALLS_")
};

TCHAR* cLog::m_formatNames[] =
{
	_T("_FMT_TEXT_"),
	_T("_FMT_CSV_")
};

#define _CHANGE_STATE_ McEventDesc(cLog::_MESSAGE,_TRACE_STATES_,NULL,NULL)

#ifndef SKIP_CRT_EXCEPTION_HOOK
/// Translates SEH exception to CExceptionBase
void CExceptionFromSEH( unsigned int u, EXCEPTION_POINTERS* pExp )
{
	tstring desc;
	switch(u)
	{
		case EXCEPTION_ACCESS_VIOLATION:
			desc = _T("EXCEPTION_ACCESS_VIOLATION");
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			desc = _T("EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
			break;
		case EXCEPTION_BREAKPOINT:
			desc = _T("EXCEPTION_BREAKPOINT");
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			desc = _T("EXCEPTION_DATATYPE_MISALIGNMENT");
			break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			desc = _T("EXCEPTION_FLT_DENORMAL_OPERAND");
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			desc = _T("EXCEPTION_FLT_DIVIDE_BY_ZERO");
			break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			desc = _T("EXCEPTION_FLT_INEXACT_RESULT");
			break;
		case EXCEPTION_FLT_INVALID_OPERATION:
			desc = _T("EXCEPTION_FLT_INVALID_OPERATION");
			break;
		case EXCEPTION_FLT_OVERFLOW:
			desc = _T("EXCEPTION_FLT_OVERFLOW");
			break;
		case EXCEPTION_FLT_STACK_CHECK:
			desc = _T("EXCEPTION_FLT_STACK_CHECK");
			break;
		case EXCEPTION_FLT_UNDERFLOW:
			desc = _T("EXCEPTION_FLT_UNDERFLOW");
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			desc = _T("EXCEPTION_ILLEGAL_INSTRUCTION");
			break;
		case EXCEPTION_IN_PAGE_ERROR:
			desc = _T("EXCEPTION_IN_PAGE_ERROR");
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			desc = _T("EXCEPTION_INT_DIVIDE_BY_ZERO");
			break;
		case EXCEPTION_INT_OVERFLOW:
			desc = _T("EXCEPTION_INT_OVERFLOW");
			break;
		case EXCEPTION_INVALID_DISPOSITION:
			desc = _T("EXCEPTION_INVALID_DISPOSITION");
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			desc = _T("EXCEPTION_NONCONTINUABLE_EXCEPTION");
			break;
		case EXCEPTION_PRIV_INSTRUCTION:
			desc = _T("EXCEPTION_PRIV_INSTRUCTION");
			break;
		case EXCEPTION_SINGLE_STEP:
			desc = _T("EXCEPTION_SINGLE_STEP");
			break;
		case EXCEPTION_STACK_OVERFLOW:
			desc = _T("EXCEPTION_STACK_OVERFLOW");
			break;
		default:
			desc = _T("UNKNOWN_SEH_EXCEPTION");
			break;
	}
	throw CExceptionBase(_M_THIS_FILE_,desc.c_str());
}

int CExceptionFromCPP( int reportType, char* userMessage, int* retVal )
{
	switch(reportType)
	{
	case _CRT_WARN:
		*retVal = 0;
		return 0;
	default:
		tstring desc(userMessage);
		tstring::size_type index = desc.find(_T("\n"));
		if(-1 != index)
			desc = desc.replace(index,1,_T(""));
		*retVal = 1;
		tstring stack = CCallTracer::instance().GetCallStack();
		CExceptionBase ex(_M_THIS_FILE_,desc.c_str());
		CExceptionBase ex2(ex, stack);
		(cMsgBoxLog()).Add(ex2);
		throw ex2;
	}
}
#endif

void cMsgBoxLog::AddString(const TCHAR* LogString, const eSeverity Severity)
{
try
{
	//Defining extra MessageBox flags for winnt > 4
	#if _WIN32_WINNT <= 0x0400
		#define MB_SERVICE_NOTIFICATION 0
	#endif

	switch(Severity)
	{
		case _EXCEPTION:
			MessageBox(0,LogString,_T("Exception"),MB_OK|MB_ICONERROR|MB_SERVICE_NOTIFICATION);
			break;
		case _ERROR:
			MessageBox(0,LogString,_T("Error"),MB_OK|MB_ICONERROR|MB_SERVICE_NOTIFICATION);
			break;
		case _WARNING:
			MessageBox(0,LogString,_T("Warning"),MB_OK|MB_SERVICE_NOTIFICATION);
			break;
		case _MESSAGE:
			MessageBox(0,LogString,_T("Information"),MB_OK|MB_ICONINFORMATION|MB_SERVICE_NOTIFICATION);
			break;
		default:
			MessageBox(0,LogString,_T("Unknown event type"),MB_OK|MB_ICONERROR|MB_SERVICE_NOTIFICATION);
			break;

	}
}
catch(...)
{
}
}

cFileLog::cFileLog() throw ( ) : cLog()
{
try
{
	GetModuleFileName(NULL,LogFileName,MAX_PATH);
	//Changing file extension
#ifdef _UNICODE
	swprintf_s(LogFileName,L"%s.Log",LogFileName);
#else
	sprintf_s(LogFileName,"%s.Log",LogFileName);
#endif //_UNICODE
}
catch(...)
{
}
}

cFileLog::cFileLog(const TCHAR* LogFileName) throw ( ) 
	: cLog()
{
try
{
	_tcscpy_s(this->LogFileName,LogFileName);
}
catch(...)
{
}
}


//===========================================================================
//
// @{FUN}                              ClearLog()
//
//---------------------------------------------------------------------------
// Effects 		: Use to clear up log file
// Return type	: bool 
// Errors		: on errors false returned (see winerror for details)
//===========================================================================
bool cFileLog::ClearLog() throw ( )
{
try
{
	HANDLE hFileLog = CreateFile(	LogFileName,
									GENERIC_WRITE,
									FILE_SHARE_READ,
									NULL,
									TRUNCATE_EXISTING,
									0,
									NULL );

	if (hFileLog == INVALID_HANDLE_VALUE) 
	{
		return false;
	}

	CloseHandle(hFileLog);
	return true;
}
catch(...)
{
	return false;
}
}

void cFileLog::AddString(const TCHAR* LogString, const eSeverity Severity)
{
try
{
	HANDLE hFileLog = CreateFile(	LogFileName,
									GENERIC_WRITE,
									FILE_SHARE_READ,
									NULL,
									OPEN_ALWAYS,
									0,
									NULL );

	if (hFileLog == INVALID_HANDLE_VALUE) 
	{
		return;
	}

	SetFilePointer(hFileLog,0,0,FILE_END);

	DWORD Written;

	WriteFile(hFileLog,LogString,(unsigned int)(_tcslen(LogString)*sizeof(TCHAR)),&Written,NULL);

	CloseHandle(hFileLog);
}
catch(...)
{
}
}

#ifdef _UNICODE
void cFileLog::AddString(const char* LogString, const eSeverity Severity)
{
try
{
	HANDLE hFileLog = CreateFile(	LogFileName,
									GENERIC_WRITE,
									FILE_SHARE_READ,
									NULL,
									OPEN_ALWAYS,
									0,
									NULL );

	if (hFileLog == INVALID_HANDLE_VALUE) 
	{
		return;
	}

	SetFilePointer(hFileLog,0,0,FILE_END);

	DWORD Written;

	WriteFile(hFileLog,LogString,strlen(LogString),&Written,NULL);

	CloseHandle(hFileLog);
}
catch(...)
{
}
}
#endif //_UNICODE


cEventLog::cEventLog() throw() : cLog()
{
try
{
	GetModuleFileName(NULL,EventsSourceName,MAX_PATH);
}
catch(...)
{
}
}

void cEventLog::AddString(const TCHAR* LogString, const eSeverity Severity)
{
try
{
	HANDLE hEventLog = RegisterEventSource( NULL, EventsSourceName ); 
	WORD wType;

	switch(Severity)
	{
	// TODO: uncomment
		//case _SUCCESS: 
		//		wType=EVENTLOG_SUCCESS; break;
		case _ERROR:
		case _EXCEPTION:
				wType=EVENTLOG_ERROR_TYPE; break;
		case _WARNING:
			wType=EVENTLOG_WARNING_TYPE; break;
		case _MESSAGE:
			wType=EVENTLOG_INFORMATION_TYPE; break;
		default:
			wType=EVENTLOG_ERROR_TYPE; break;
	}

	ReportEvent( hEventLog,				// handle
				 wType,					// event type 
				 0,						// event category (specific for application)
				 0,						// EventID, 
				 NULL,					// user security identifier (optional) 
				 1,						// number of strings to merge with message 
				 0,						// size of binary data, in bytes 
				 (const TCHAR **)&LogString, // array of strings to merge with message 
				 NULL );				// address of binary data 

	DeregisterEventSource( hEventLog );
}
catch(...)
{
}
}

void cBuffLog::AddString(const TCHAR *LogString, const eSeverity Severity) throw()
{
try
{
	tstring tmp(LogString);
	FullLen += (unsigned int)tmp.length() + 1;
	Messages.push_back(tmp);
}
catch(...)
{
}
}

//===========================================================================
//
// @{FUN}                          ~cBufLog()
//
//---------------------------------------------------------------------------
// Effects		: Destroys cBufLog object
// Errors		: No errors handled
//===========================================================================
cBuffLog::~cBuffLog() throw()
{
try
{
	for(;!Messages.empty();Messages.pop_back());
}
catch(...)
{
}
}


//===========================================================================
//
// @{FUN}                          GetLogBuffer()
//
//---------------------------------------------------------------------------
// Effects		: Returns all log in form of buffer
// Arguments	: unsigned int *Len [OUT] The length of result buffer in bytes
// Errors		: On error NULL returned
//===========================================================================	
char* cBuffLog::GetLogBuffer(unsigned int *Len) throw()
{
try
{
	if (!Len) return NULL;
	*Len=0;
	char *Result = new char[FullLen * sizeof(TCHAR)];
	if (!Result) return NULL;
	char *Current = Result;
	for(vector<tstring>::iterator i=Messages.begin();
		i!=Messages.end();
		i++)
	{
			memcpy(Current,(*i).c_str(),((*i).length()+1)*sizeof(TCHAR));
			Current+=((*i).length()+1)*sizeof(TCHAR);
	}
	memset(Result + (FullLen-1) * sizeof(TCHAR),0,sizeof(TCHAR));
	*Len=FullLen * sizeof(TCHAR);
	return Result;
}
catch(...)
{
	return NULL;
}
}



void cLog::Add(const CExceptionBase &e) throw ()
{
try
{
	AddList(cEventDesc(	_EXCEPTION,
						_TRACE_DEBUG_,
						e.m_SrcFile.c_str(),
						e.m_SrcLine,
						e.m_SrcDate.c_str(),
						0,
						NULL,
						NULL,
						e.m_WhatHead.c_str()),
				e.m_strWhat.c_str(),
				NULL);
}
catch(...)
{
}
}

//Adds formated record	
void cLog::Add(const cEventDesc &EventDesc, const TCHAR* Format, ...) throw()
{
	PTCHAR Mess(NULL);
try
{
	va_list args;
	va_start( args, Format );	// Init the user arguments list.

	unsigned int i=1;
	unsigned mult=1;
	int result;
	for(Mess=new TCHAR[MSG_BUF_SIZE];
		(result=_vsntprintf_s( Mess, MSG_BUF_SIZE*mult,_TRUNCATE, Format, args ))<0 && i<MSG_BUF_SIZE_DOUBLING;
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
	AddList(EventDesc,Mess,NULL);

	delete [] Mess;
	Mess = NULL;
}
catch(...)
{
	try
	{
		if (Mess) delete [] Mess; //Preventing memory leaks
		AddList(_ERROR_,_T("cLog::Add: Exception happened"),NULL);
	}
	catch(...)
	{
		//One more exception happened
		//going further
	}
}
}

//Create report for winerror
void cLog::WinError(cEventDesc &EventDesc, const TCHAR *Format, ...) throw( )
{
try
{
	EventDesc.SYSERRNO=GetLastError();
	
	LPVOID Buffer;
	DWORD BufferAllocated;
	va_list args;
	TCHAR Mess[MSG_BUF_SIZE];

	//Formatting winerror message
	if(!(BufferAllocated=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, EventDesc.SYSERRNO, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(PTCHAR) &Buffer, 16, NULL)))
	{	
		Buffer=_T("Unknown win error");
	}


	va_start( args, Format );	// Init the user arguments list.
#ifndef _UNICODE
	vsprintf_s(Mess, Format, args );
#else
	vswprintf_s(Mess, Format, args );
#endif //_UNICODE

	AddList(EventDesc,Mess,Buffer,NULL);

	if (BufferAllocated) LocalFree( Buffer );

}
catch(...)
{
}
}


//Create report for COM error
void cLog::ComError(cEventDesc &EventDesc, const TCHAR *Format, ...) throw( )
{
try
{
	EventDesc.SYSERRNO=GetLastError();
	va_list args;
	//TODO: handle error string correctly!
	PTCHAR ComErrorString=_T("SOME COM ERROR...");//GetComErrorString(hr);
	TCHAR Mess[MSG_BUF_SIZE];

	if (!ComErrorString) ComErrorString=_T("Unknown com error");

	va_start( args, Format );	// Init the user arguments list.
#ifndef _UNICODE
	vsprintf_s(Mess, Format, args );
#else
	vswprintf_s(Mess, Format, args );
#endif //_UNICODE

	AddList(EventDesc,ComErrorString,Mess,NULL);
	
	//delete [] ComErrorString;
}
catch(...)
{
}
}


//Adds string to a log composed from NULL terminated
//list of PTCHAR parametrs
void cLog::AddList(const cEventDesc &EventDesc, const TCHAR *Item, ...) throw( )
{
try
{
	// Exit if verbosity level is _NO_TRACE_ of high than defined level
	if( (_NO_TRACE_ == EventDesc.Verbosity) || (EventDesc.Verbosity > m_verbosity) )
		return;

	// Get severity structure
	sSeverity severity = m_severities[EventDesc.Severity];
	// Get severity name
	tstring SeverityStr = severity.m_severityName;
	// Get set of information fields for this severity
	unsigned int fields = severity.m_informFields;
	// Add severity marker
	tstring messageString = cEventDesc::Replace(
		m_logFormats[m_logFormat][_SEVERITY_MARKER],
		m_logMessageParameters[_PARAM_SEVERITY], 
		SeverityStr.c_str());

	// For each information field
	for(unsigned int i = 0; i<MAX_INFORM_FIELD_INDEX; i++)
	{
		sInformField field = m_informFields[i];
		// Process next field if field not in set
		if(!(field.m_type&fields))
			continue;
		// Get information string for field
		tstring fieldName = field.m_informString;
		tstring fieldValue = _T("");
		// Add params for MESSAGE
		if(_FIELD_MESSAGE == field.m_type)
		{
			va_list vl;
			for(va_start(vl, Item); Item; Item=va_arg(vl, PTCHAR))
				fieldValue += Item;
			va_end(vl);
		}
		else
		{
			// Get field value
			if(!field.m_getter)
				continue;
			fieldValue = field.m_getter(EventDesc, NULL);
		}

		if(fieldValue.empty())
			continue;

		tstring infoStr;

		// Format information string
		infoStr = cEventDesc::Replace(
			m_logFormats[m_logFormat][_INFORM_FIELD],
			m_logMessageParameters[_PARAM_FIELD_NAME], 
			fieldName.c_str());

		infoStr = cEventDesc::Replace(
			infoStr.c_str(), 
			m_logMessageParameters[_PARAM_VALUE], 
			fieldValue.c_str());

		infoStr = cEventDesc::AddTabsToInfoLine(
			infoStr.c_str(), 
			m_logMessageParameters[_PARAM_TABS]);

		messageString += infoStr;
	}

	// Add end marker
	messageString += cEventDesc::Replace(
		m_logFormats[m_logFormat][_END_MARKER],
		m_logMessageParameters[_PARAM_SEVERITY], 
		SeverityStr.c_str());

	//Preventing simultaneous execution of AddString functions
	EnterCriticalSection(&AddStringCS);

	try
	{
		//Calling virtual function of adding string to destinate log('s)
		AddString(messageString.c_str(), EventDesc.Severity);
	}
	catch(...)
	{
		try
		{
			AddString(_T("cLog: Exception during AddString happened"),_EXCEPTION);
		}
		catch(...)
		{
		}
	}

	LeaveCriticalSection(&AddStringCS);
}
catch(...)
{
}
}

void cLog::AddState(const TCHAR *Name, const TCHAR *Value, const TCHAR *NewValue)
{
	try
	{
		Add(_CHANGE_STATE_, _T("State of %s is changed from \"%s\" to \"%s\""), Name, Value, NewValue);
	}
	catch(...)
	{
	}
}

void cLog::AddState(const TCHAR *Name, const int Value, const int NewValue)
{
	try
	{
		Add(_CHANGE_STATE_, _T("State of %s is changed from %d to %d"), Name, Value, NewValue);
	}
	catch(...)
	{
	}
}

void cLog::AddState(const TCHAR *Name, const cDate Value, const cDate NewValue)
{
	try
	{
		tstring val;
		tstring newval;
		SYSTEMTIME valTime = (SYSTEMTIME)Value;
		SYSTEMTIME newvalTime = (SYSTEMTIME)NewValue;
		tstring DateFormat(_T("dd.MM.yyyy"));
		TCHAR Buf[MAX_PATH];

		if (!GetDateFormat(LOCALE_USER_DEFAULT,0,&valTime,DateFormat.c_str(),Buf,MAX_PATH))
			val = _T("Invalid date");
		else
			val = Buf;
		val += _T(" ");
		if (!GetTimeFormat(LOCALE_USER_DEFAULT,0,&valTime,NULL,Buf,MAX_PATH))
			val += _T("Invalid time");
		else
			val += Buf;


		if (!GetDateFormat(LOCALE_USER_DEFAULT,0,&newvalTime,DateFormat.c_str(),Buf,MAX_PATH))
			newval = _T("Invalid date");
		else
			newval = Buf;
		newval += _T(" ");
		if (!GetTimeFormat(LOCALE_USER_DEFAULT,0,&newvalTime,NULL,Buf,MAX_PATH))
			newval += _T("Invalid time");
		else
			newval += Buf;

		Add(_CHANGE_STATE_, _T("State of %s is changed from \"%s\" to \"%s\""), Name, val.c_str(), newval.c_str());
	}
	catch(...)
	{
	}
}

void cLog::AddState(const TCHAR *Name, const float Value, const float NewValue)
{
	try
	{
		Add(_CHANGE_STATE_, _T("State of %s is changed from %f to %f"), Name, Value, NewValue);
	}
	catch(...)
	{
	}
}

void cLog::AddState(const TCHAR *Name, const bool Value, const bool NewValue)
{
	try
	{
		Add(_CHANGE_STATE_, _T("State of %s is changed from %d to %d"), Name, Value, NewValue);
	}
	catch(...)
	{
	}
}



tstring cLog::cEventDesc::GetTimeString(const cEventDesc& eventDesc, VARIANT* varValue)
{
	SYSTEMTIME SysTime;
	GetLocalTime(&SysTime);
	if(varValue)
	{
		try
		{
			cDate data(SysTime);
			CLogVariant logVariant(data);
			VariantCopy(varValue, const_cast<VARIANT*>(logVariant.Variant()));
		}
		catch(...)
		{
		}
		return _T("");
	}
	else
	{
		DWORD DateFlags = DATE_LONGDATE;
		tstring DateFormat(_T("dd.MM.yyyy"));
		TCHAR Buf[MAX_PATH];
		tstring TimeStr;
		if (!GetDateFormat(LOCALE_USER_DEFAULT,0,&SysTime,DateFormat.c_str(),Buf,MAX_PATH))
			TimeStr = _T("Invalid date");
		else
			TimeStr = Buf;
		TimeStr+=_T(" ");
		if (!GetTimeFormat(LOCALE_USER_DEFAULT,0,&SysTime,NULL,Buf,MAX_PATH))
			TimeStr += _T("Invalid time");
		else
			TimeStr += Buf;
		return TimeStr;
	}
}

tstring cLog::cEventDesc::GetPidString(const cEventDesc& eventDesc, VARIANT* varValue)
{
	if(!ModuleNameReceived)
	{
		if(GetModuleFileName(NULL, ModuleName, MAX_PATH))
		{
			tstring tmp(ModuleName);
			tstring::size_type index = tmp.find_last_of('\\');
			if(-1 != index)
				tmp.erase(0, index + 1);
			memset(ModuleName, 0, MAX_PATH);
			memcpy(ModuleName, tmp.c_str(), tmp.length());
		}
		ModuleNameReceived = true;
	}

	tstring moduleName(ModuleName);
	moduleName = i2tstring(_getpid()) + _T(" (") + moduleName + _T(")");
	if(varValue)
	{
		try
		{
			CLogVariant logVariant(moduleName);
			VariantCopy(varValue, const_cast<VARIANT*>(logVariant.Variant()));
		}
		catch(...)
		{
		}
		return _T("");
	}
	else
	{
		return moduleName;
	}
}

tstring cLog::cEventDesc::GetTidString(const cEventDesc& eventDesc, VARIANT* varValue)
{
	LPVOID data = TlsGetValue(ThreadInitializer.Index());
	tstring tmp;
	if(!data)
		tmp = _T("Unnamed thread");
	else
		tmp = reinterpret_cast<TCHAR*>(data);
	tstring threadName = i2tstring(GetCurrentThreadId()) + _T(" (") + tmp + _T(")");

	if(varValue)
	{
		try
		{
			CLogVariant logVariant(threadName);
			VariantCopy(varValue, const_cast<VARIANT*>(logVariant.Variant()));
		}
		catch(...)
		{
		}
		return _T("");
	}
	else
	{
		return threadName;
	}
}

tstring cLog::cEventDesc::GetSourceFileString(const cEventDesc& eventDesc, VARIANT* varValue) 
{
	if(varValue)
	{
		try
		{
			tstring srcFile(eventDesc.File);
			if(srcFile.empty())
				return _T("");
			CLogVariant logVariant(srcFile);
			VariantCopy(varValue, const_cast<VARIANT*>(logVariant.Variant()));
		}
		catch(...)
		{
		}
		return _T("");
	}
	else
	{
		return eventDesc.File;
	}
}

tstring cLog::cEventDesc::GetSourceLineString(const cEventDesc& eventDesc, VARIANT* varValue) 
{
	if(varValue)
	{
		try
		{
			CLogVariant logVariant(static_cast<int>(eventDesc.Line));
			VariantCopy(varValue, const_cast<VARIANT*>(logVariant.Variant()));
		}
		catch(...)
		{
		}
		return _T("");
	}
	else
	{
		return i2tstring(eventDesc.Line);
	}
}

tstring cLog::cEventDesc::GetCompilationDateString(const cEventDesc& eventDesc, VARIANT* varValue) 
{
	if(varValue)
	{
		try
		{
			tstring srcDate(eventDesc.CompDate);
			if(srcDate.empty())
				return _T("");
			CLogVariant logVariant(srcDate);
			VariantCopy(varValue, const_cast<VARIANT*>(logVariant.Variant()));
		}
		catch(...)
		{
		}
		return _T("");
	}
	else
	{
		return eventDesc.CompDate;
	}
}

tstring cLog::cEventDesc::GetSystemErrorString(const cEventDesc& eventDesc, VARIANT* varValue)
{
	if(varValue)
	{
		try
		{
			unsigned int errInfo = eventDesc.SYSERRNO;
			if(!errInfo)
			{
				errInfo = GetLastError();
				if(!errInfo)
					return _T("");
			}
			CLogVariant logVariant(static_cast<int>(errInfo));
			VariantCopy(varValue, const_cast<VARIANT*>(logVariant.Variant()));
		}
		catch(...)
		{
		}
		return _T("");
	}
	else
	{
		if(eventDesc.SYSERRNO)
			return i2tstring(eventDesc.SYSERRNO);
		DWORD errNumber = GetLastError();
		if(errNumber)
			return i2tstring(errNumber);
		else
			return _T("");
	}
}

tstring cLog::cEventDesc::GetTestSuiteString(const cEventDesc& eventDesc, VARIANT* varValue) 
{
	if(varValue)
	{
		try
		{
			tstring testSuite(eventDesc.TestSuite);
			if(testSuite.empty())
				return _T("");
			CLogVariant logVariant(testSuite);
			VariantCopy(varValue, const_cast<VARIANT*>(logVariant.Variant()));
		}
		catch(...)
		{
		}
		return _T("");
	}
	else
	{
		return eventDesc.TestSuite;
	}
}

tstring cLog::cEventDesc::GetTestCaseString(const cEventDesc& eventDesc, VARIANT* varValue) 
{
	if(varValue)
	{
		try
		{
			tstring testCase(eventDesc.TestCase);
			if(testCase.empty())
				return _T("");
			CLogVariant logVariant(testCase);
			VariantCopy(varValue, const_cast<VARIANT*>(logVariant.Variant()));
		}
		catch(...)
		{
		}
		return _T("");
	}
	else
	{
		return eventDesc.TestCase;
	}
}

tstring cLog::cEventDesc::GetCallStackString(const cEventDesc& eventDesc, VARIANT* varValue) 
{
	if(varValue)
	{
		try
		{
			tstring callStack(eventDesc.CallStack);
			if(callStack.empty())
				return _T("");
			CLogVariant logVariant(callStack);
			VariantCopy(varValue, const_cast<VARIANT*>(logVariant.Variant()));
		}
		catch(...)
		{
		}
		return _T("");
	}
	else
	{
		return eventDesc.CallStack;
	}
}

tstring cLog::cEventDesc::Replace(const TCHAR *str, const TCHAR *pattern, const TCHAR *value)
{
	tstring tmpstring(str);
	tstring tmppattern(pattern);
	tstring tmpvalue(value);
	tstring::size_type index = tmpstring.find(tmppattern);
	while(-1 != index)
	{
		tmpstring = tmpstring.replace(index,tmppattern.length(),tmpvalue);
		index = tmpstring.find(tmppattern);
	}
	return tmpstring;
};

tstring cLog::cEventDesc::AddTabsToInfoLine(const TCHAR *str, const TCHAR *tabparam)
{
	tstring tmpstring(str);
	tstring tmpparam(tabparam);
	tstring::size_type index = tmpstring.find(tmpparam);
	if(-1 == index)
		return tmpstring;

	tmpstring = tmpstring.replace(index,tmpparam.length(),_T(" "));
/*
	if(index >= 2)
	{
		tmpstring = tmpstring.replace(index,tmpparam.length(),_T(" "));
	}
	else
	{
		tstring tmpstr(LOG_COLUMN_WIDTH-index, '.');
		tmpstring = tmpstring.replace(index,tmpparam.length(),tmpstr);
	}
*/
	return tmpstring;
}




//------------------------Universal log class-----------------------------
void cAnyLog::AddString(const TCHAR* LogString, const eSeverity Severity) throw()
{
try
{
//#ifdef _DEBUG
//	OutputDebugString(LogString);
//#endif //_DEBUG
}
catch(...)
{
}
}

void cAnyLog::AddList(const cEventDesc &EventDesc, const TCHAR *Item, ...) throw()
{
try
{
	cCritSection cs(&m_cs);
	tstring str;
	va_list vl;
	for(va_start(vl, Item); Item; Item=va_arg(vl, PTCHAR))
	{
		str += Item;
	}
	va_end(vl);
	if (EventDesc.getSeverity()>=0 && EventDesc.getSeverity()<MSeverityTypesCount)
		for(deque<cLog*>::iterator i=Logs[EventDesc.getSeverity()].begin();i!=Logs[EventDesc.getSeverity()].end();i++)
			(*i)->AddList(EventDesc, str.c_str(), NULL);
}
catch(...)
{
}
}

void cAnyLog::ClearList() throw()
{
try
{
	cCritSection cs(&m_cs);
	std::set<cLog*> logs_set;
	//Clearing log deques
	for(unsigned int i=_MESSAGE;i<=_FTEST_CASE;++i)
		for(;!Logs[i].empty();Logs[i].pop_back())
			logs_set.insert(*(Logs[i].end() - 1));
	for(std::set<cLog*>::iterator log = logs_set.begin();
		log != logs_set.end();
		++log)
	{
		if (!(*log)->m_ownLiveTime) delete *log;
	}
}
catch(...)
{
}
}

//===========================================================================
//
// @{FUN}                          RegisterLog()
//
//---------------------------------------------------------------------------
// Effects		: Adds new Log object for logging
//  Argument	: cLog *NewLog [IN] new log object
//  Argument	: const bool PushBack=true [IN] - defines push log back or forward
//	Argument	: const eSeverity Severity=-1 [IN] //for with severity. if -1 then to all of them
// Errors		: No errors handled
//===========================================================================
void cAnyLog::RegisterLog(cLog *NewLog, const bool PushBack, const int Severity) throw()
{
try
{
	cCritSection cs(&m_cs);
	cLog* newLog(NewLog);
	if (!NewLog) return;
	if (Severity==-1)
	{
		//To all severities
		for(unsigned int i=_MESSAGE;i<=_FTEST_CASE;i++)
			if (PushBack) 
				Logs[i].push_back(newLog);
			else
				Logs[i].push_front(newLog);
	} else
	{
		//To certain severity
		if (Severity>=_MESSAGE && Severity<=_FTEST_CASE)
			if (PushBack) 
				Logs[Severity].push_back(newLog);
			else
				Logs[Severity].push_front(newLog);
	}
}
catch(...)
{
}
}

//===========================================================================
//
// @{FUN}                          UnRegisterLog()
//
//---------------------------------------------------------------------------
// Effects		: Removes Log object from logging
//  Argument	: cLog *log [IN] log object to unregister
// Errors		: No errors handled
//===========================================================================
void cAnyLog::UnRegisterLog(const cLog *log)
{
try
{
	cCritSection cs(&m_cs);
	for(unsigned int i=_MESSAGE;i<=_FTEST_CASE;i++)
	{
		std::deque<cLog*>::iterator l = std::find(Logs[i].begin(),Logs[i].end(),log);
		if (l != Logs[i].end())
			Logs[i].erase(l);
	}
}
catch(...)
{
}
}

void cAnyLog::SetVerbosity(const eVerbosity verbosity) throw( )
{
	try
	{
		cCritSection cs(&m_cs);
		cLog::SetVerbosity(verbosity);
		for(unsigned int i=_MESSAGE;i<=_FTEST_CASE;i++)
		{
			for(std::deque<cLog*>::iterator log = Logs[i].begin(); log != Logs[i].end(); ++log)
				(*log)->SetVerbosity(verbosity);
		}
	}
	catch(...)
	{
	}
}

void cAnyLog::SetFormat(const eLogFormat logFormat) throw( )
{
	try
	{
		cCritSection cs(&m_cs);
		cLog::SetFormat(logFormat);
		for(unsigned int i=_MESSAGE;i<=_FTEST_CASE;i++)
		{
			for(std::deque<cLog*>::iterator log = Logs[i].begin(); log != Logs[i].end(); ++log)
				(*log)->SetFormat(logFormat);
		}
	}
	catch(...)
	{
	}
}

void cDbgOutLog::AddString(const TCHAR* LogString, const eSeverity Severity) throw()
{
try
{
	OutputDebugString(LogString);
}
catch(...)
{
}
}

//------------------------------------------------------------------------
