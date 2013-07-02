//===========================================================================
// Archer Software.
//                                   cLog.h
//
//---------------------------------------------------------------------------
// Class for logging everything
//---------------------------------------------------------------------------
////
// Version : 01.00
// By      : Max Sogin
// Date    : 7/14/05 12:56:00 PM
//===========================================================================


#ifndef CLOG_H
#define CLOG_H

//-------------------------------------------------------------------------------
#include <AidLib/Strings/tstring.h>
#include <tchar.h>
#include <windows.h>
#include <vector>
#include <deque>
#include <AidLib/AidLib.h>
#include <boost/function.hpp>
#include <process.h>
#include <map>
#include <AidLib/CThread/CThreadLSInitializer.h>
#include <oleauto.h>

#ifndef SKIP_CRT_EXCEPTION_HOOK 
	#include <crtdbg.h>
#endif


#define TEOLN _T("\r\n")

#pragma warning( disable : 4290 )


#define MSG_BUF_SIZE 4096
/// max count of dubling of message buffer when the buffer too small
#define MSG_BUF_SIZE_DOUBLING 6

class CExceptionBase;
class cDate;

/// Translates SEH exception to CExceptionBase
void CExceptionFromSEH( unsigned int u, EXCEPTION_POINTERS* pExp );
/// Translates C++ exception to CExceptionBase
int CExceptionFromCPP( int reportType, char* userMessage, int* retVal );


/// Verbosity levels
typedef enum _eVerbosity
{
	_NO_TRACE_ = 0,
	_TRACE_DEBUG_,
	_TRACE_STATES_,
	_TRACE_CALLS_
} AIDLIB_API eVerbosity;

/// Log formats
typedef enum _eLogFormat
{
	_FMT_TEXT_ = 0,
	_FMT_CSV_
} AIDLIB_API eLogFormat;

#define MAX_LOG_FORMAT 2

/// Defines verbosity level for cLog constructor
#define DAFAULT_VERBOSITY _TRACE_DEBUG_
#ifndef LOG_VERBOSITY
	#define LOG_VERBOSITY DAFAULT_VERBOSITY
#endif
/// Size of array for information fields
#define MAX_INFORM_FIELD_INDEX 13
/// Column width for log
#define LOG_COLUMN_WIDTH 25

/// Defines log format
#define DEFAULT_LOG_FORMAT _FMT_TEXT_
#ifndef LOG_FORMAT
	#define LOG_FORMAT DEFAULT_LOG_FORMAT
#endif

/// Environment variables names
#define LOG_VERBOSITY_ENV _T("LOG_VERBOSITY")
#define LOG_FORMAT_ENV _T("LOG_FORMAT")

//===========================================================================
// @{CSEH}
//								cLog
//
//---------------------------------------------------------------------------
// Description		: Base class for all our log classes
//===========================================================================
class AIDLIB_API cLog
{
private:

#ifndef _DYNAMIC_AID_
	cLog(const cLog&);
	cLog& operator=(const cLog&);
#endif


public:

	//Critical section for AddString calls
	//to prevent simultaneous execution of AddString functions
	CRITICAL_SECTION AddStringCS;

	
	//Types of log messages
	typedef enum _eSeverity
	{
		_MESSAGE		= 0,
		_WARNING		= 1,
		_ERROR			= 2,
		_EXCEPTION		= 3,
		_UTEST_SUITE	= 4,
		_UTEST_CASE		= 5,
		_FTEST_SUITE	= 6,
		_FTEST_CASE		= 7
	} eSeverity;

//protected:

	// Structure to store severity properties
	typedef struct _sSeverity
	{
		// Severity
		eSeverity		m_severity;
		// Severity string
		TCHAR*			m_severityName;
		// Set of the fields for current severity
		unsigned int	m_informFields;
	} sSeverity;


public:

	//This class describes single event;
	class cEventDesc
	{
	private:
	friend class cLog;
	friend class CNetworkLog;

		static TCHAR ModuleName[MAX_PATH]; // Name of current module
		static bool ModuleNameReceived;
		eSeverity Severity;			//Event (error) severity
		eVerbosity Verbosity;		//Event verbosity
		const TCHAR *File;			//File name where event created
		unsigned int Line;			//Line of file where event created
		const TCHAR *CompDate;		//Creation date of file where event created
		unsigned int SYSERRNO;		//System error number
		const TCHAR *TestSuite;		//Test suite name
		const TCHAR *TestCase;		//Test case name
		const TCHAR *CallStack;		//Call stack for exceptions
	public:
		cEventDesc(	const eSeverity _Severity,
					const eVerbosity _Verbosity,
					const TCHAR *_File,
					unsigned int _Line,
					const TCHAR *_CompDate,
					unsigned int _SYSERRNO,
					const TCHAR *_TestSuite,
					const TCHAR *_TestCase,
					const TCHAR *_CallStack) throw () : Severity(_Severity), Verbosity(_Verbosity),
		File(_File), Line(_Line), CompDate(_CompDate), SYSERRNO(_SYSERRNO), TestSuite(_TestSuite), 
		TestCase(_TestCase), CallStack(_CallStack)
		{};
		eSeverity getSeverity() const throw() {return Severity;};
		eVerbosity getVerbosity() const throw() {return Verbosity;};
		const TCHAR* getFile() const throw() {return File;};
		unsigned int getLine() const throw() {return Line;};
		const TCHAR *getCompDate() const throw() {return CompDate;};
		unsigned int getSYSERRNO() const throw() {return SYSERRNO;};
		const TCHAR *getTestSuite() const throw() {return TestSuite;};
		const TCHAR *getTestCase() const throw() {return TestCase;};
		const TCHAR *getCallStack() const throw() {return CallStack;};
		cEventDesc& operator =(const cEventDesc& eventDesc)
		{
			if (this == &eventDesc)
				return *this;
			Severity = eventDesc.Severity;
			Verbosity = eventDesc.Verbosity;
			File = eventDesc.File;
			Line = eventDesc.Line;
			CompDate= eventDesc.CompDate;
			SYSERRNO = eventDesc.SYSERRNO;
			TestSuite = eventDesc.TestSuite;
			TestCase = eventDesc.TestCase;
			CallStack = eventDesc.CallStack;
			return *this;
		};
		/// Returns current date/time as tstring
		static tstring GetTimeString(const cEventDesc& eventDesc, VARIANT* varValue = NULL);
		/// Returns current process id and process file name
		static tstring GetPidString(const cEventDesc& eventDesc, VARIANT* varValue = NULL);
		/// Returns current thread id and thread's class name (from TLS)
		static tstring GetTidString(const cEventDesc& eventDesc, VARIANT* varValue = NULL);
		/// Returns source file name
		static tstring GetSourceFileString(const cEventDesc& eventDesc, VARIANT* varValue = NULL);
		/// Returns line number as tstring
		static tstring GetSourceLineString(const cEventDesc& eventDesc, VARIANT* varValue = NULL);
		/// Returns compilation date as tstring
		static tstring GetCompilationDateString(const cEventDesc& eventDesc, VARIANT* varValue = NULL);
		/// Get system error number as tstring
		static tstring GetSystemErrorString(const cEventDesc& eventDesc, VARIANT* varValue = NULL);
		/// Returns test suite name
		static tstring GetTestSuiteString(const cEventDesc& eventDesc, VARIANT* varValue = NULL);
		/// Returns test case name
		static tstring GetTestCaseString(const cEventDesc& eventDesc, VARIANT* varValue = NULL);
		/// Returns call stack
		static tstring GetCallStackString(const cEventDesc& eventDesc, VARIANT* varValue = NULL);
		/// Replaces part of the string by other string
		static tstring Replace(const TCHAR *str, const TCHAR *pattern, const TCHAR *value);
		/// Adds "." to each log line
		static tstring AddTabsToInfoLine(const TCHAR *str, const TCHAR *tabparam);
	};

//protected:

	// Information field type
	typedef enum _eInformFieldType
	{
		_FIELD_NONE				= 0,
		_FIELD_TIME				= 1,
		_FIELD_PID				= 2,
		_FIELD_TID				= 4,
		_FIELD_SOURCE_FILE		= 8,
		_FIELD_LINE_NUMBER		= 16,
		_FIELD_COMPILATION_DATE	= 32,
		_FIELD_SYSTEM_ERROR		= 64,
		_FIELD_UTEST_SUITE		= 128,
		_FIELD_UTEST_CASE		= 256,
		_FIELD_FTEST_SUITE		= 512,
		_FIELD_FTEST_CASE		= 1024,
		_FIELD_MESSAGE			= 2048,
		_FIELD_CALL_STACK		= 4096,
		_FIELD_ALL				= _FIELD_TIME | _FIELD_PID | _FIELD_TID | _FIELD_SOURCE_FILE | _FIELD_LINE_NUMBER | _FIELD_COMPILATION_DATE | 
									_FIELD_SYSTEM_ERROR | _FIELD_UTEST_SUITE | _FIELD_UTEST_CASE | _FIELD_FTEST_SUITE | _FIELD_FTEST_CASE | 
									_FIELD_MESSAGE | _FIELD_CALL_STACK
	} eInformFieldType;

	// Function to obtain field value
	typedef boost::function<tstring (const cEventDesc&, VARIANT* varValue)> fParamGetFunc;

	// Structure to store information field properties
	typedef struct _sInformField
	{
		// Field type
		eInformFieldType		m_type;
		// Information string
		TCHAR*					m_informString;
		// Function to obtain field value
		fParamGetFunc			m_getter;
	} sInformField;

protected:

	// Log string lines
	typedef enum _eLogMessageString
	{
		// Severity marker
		_SEVERITY_MARKER = 0,
		// Information field
		_INFORM_FIELD,
		// End message marker
		_END_MARKER
	} eLogMessageString;

#define MAX_LOG_MESSAGE_STRING 3

	// Log message parameters
	typedef enum _eLogMessageParameter
	{
		// Severity name
		_PARAM_SEVERITY = 0,
		// Information field name
		_PARAM_FIELD_NAME,
		// Field value
		_PARAM_VALUE,
		// Line tabs
		_PARAM_TABS
	} eLogMessageParameter;

public:
	// Array of the severity settings
	static sSeverity m_severities[];
	// Array of the information fields settings
	static sInformField m_informFields[];

protected:
	// Array of parameters
	static TCHAR* m_logMessageParameters[];
	// Array of verbosity names
	static TCHAR* m_verbosityNames[];
	// Array of formats names
	static TCHAR* m_formatNames[];
	// Array of log message strings
	static TCHAR* m_logFormats[MAX_LOG_FORMAT][MAX_LOG_MESSAGE_STRING];

friend class cAnyLog;
friend class CCallTracerWrapper;
protected:

	/// Set to true to prevent destruction from any log
	bool m_ownLiveTime;
	/// Verbosity level
	eVerbosity m_verbosity;
	/// Format
	eLogFormat m_logFormat;

	//This fn must be redeclared in descedent classes
	virtual void AddString(const TCHAR* LogString, const eSeverity Severity) throw() = NULL;

public:
	cLog() throw() 
		: m_ownLiveTime(false)
	{
	try
	{
		InitializeCriticalSection(&AddStringCS);
		TCHAR Buf[MAX_PATH];

		// Get verbosity from environment variable
		if(GetEnvironmentVariable(LOG_VERBOSITY_ENV, Buf, MAX_PATH))
		{
			unsigned int i;
			tstring value(Buf);
			for(i = _NO_TRACE_;i <= _TRACE_CALLS_;i++)
			{
				tstring verbosity(m_verbosityNames[i]);
				if( !value.compare(verbosity) || !value.compare(i2tstring(i)) )	
					break;
			}
			if(i <= _TRACE_CALLS_)
				m_verbosity = static_cast<eVerbosity>(i);
			else
				m_verbosity = LOG_VERBOSITY;
		}
		else
		  m_verbosity = LOG_VERBOSITY;

		// Get format from environment variable
		if(GetEnvironmentVariable(LOG_FORMAT_ENV, Buf, MAX_PATH))
		{
			unsigned int i;
			tstring value(Buf);
			for(i = _FMT_TEXT_;i <= _FMT_CSV_;i++)
			{
				tstring fmt(m_formatNames[i]);
				if( !value.compare(fmt) || !value.compare(i2tstring(i)) )	
					break;
			}
			if(i <= _FMT_CSV_)
				m_logFormat = static_cast<eLogFormat>(i);
			else
				m_logFormat = LOG_FORMAT;
		}
		else
		  m_logFormat = LOG_FORMAT;

	}
	catch(...)
	{
	}
	};


	virtual ~cLog() throw()
	{
	try
	{
		DeleteCriticalSection(&AddStringCS);
	}
	catch(...)
	{
	}
	};

	//Adds string to a log composed from NULL terminated
	//list of PTCHAR parametrs
	virtual void AddList(const cEventDesc &EventDesc, const TCHAR *Item, ...) throw( );

	//Adds formated record	
	void Add(const cEventDesc &EventDesc, const TCHAR* Format, ...) throw();

	//Create report for exception
	void Add(const CExceptionBase &e) throw( );

	//Adds raw string to output
	void Add(const TCHAR *Str) throw( )
	{
	try
	{
		AddString(Str,_MESSAGE);
	}
	catch(...)
	{
	}
	}

	// Adds record for changing state message
	void AddState(const TCHAR *Name, const TCHAR *Value, const TCHAR *NewValue);
	void AddState(const TCHAR *Name, const int Value, const int NewValue);
	void AddState(const TCHAR *Name, const cDate Value, const cDate NewValue);
	void AddState(const TCHAR *Name, const float Value, const float NewValue);
	void AddState(const TCHAR *Name, const bool Value, const bool NewValue);

	//Create report for winerror
	void WinError(cEventDesc &EventDesc, const TCHAR *Format, ...) throw( );

	//Create report for ADSI error
	void ADSError(cEventDesc &EventDesc, HRESULT hr, const TCHAR *Format, ...) throw( );

	//Create report for COM error
	void ComError(cEventDesc &EventDesc, const TCHAR *Format, ...) throw( );

	// Get verbosity level
	eVerbosity GetVerbosity() const throw( )
	{
		return m_verbosity;
	}
	// Set new verbosity level
	virtual void SetVerbosity(const eVerbosity verbosity) throw( )
	{
		m_verbosity = verbosity;
	}
	// Get log format
	eLogFormat GetFormat() const throw( )
	{
		return m_logFormat;
	}
	// Set log format
	virtual void SetFormat(const eLogFormat logFormat) throw( )
	{
		m_logFormat = logFormat;
	}
};

//class to put errors on console
//===========================================================================
// @{CSEH}
//								cConsLog
//
//---------------------------------------------------------------------------
// Description		: Class for reporting events on application consol if it is available
//===========================================================================
#ifdef _CONSOLE
	#include <atlbase.h>
	#include <atlconv.h>

//	#include <AssistLib.h>
#endif //_CONSOLE

class AIDLIB_API cConsLog : public cLog
{
private:
	virtual void AddString(const TCHAR* LogString, const eSeverity Severity) throw()
	{
	try
	{
		//#ifdef _CONSOLE
		//Here i will add a console availablity check
			#ifdef _UNICODE
				USES_CONVERSION;
				//cout<<W2A(LogString);	
				printf_s(W2A(LogString));
			#else
				printf_s(LogString);
				//cout<<LogString;
			#endif //_UNICODE
		//#endif //_CONSOLE
	}
	catch(...)
	{
	}
	}
public:
};

//===========================================================================
// @{CSEH}
//								cMsgBoxLog
//
//---------------------------------------------------------------------------
// Description		: Class for reporting errors as windows message boxes
//===========================================================================
class AIDLIB_API cMsgBoxLog : public cLog
{
private:
	virtual void AddString(const TCHAR* LogString, const eSeverity Severity) throw();

public:
};


//===========================================================================
// @{CSEH}
//								cDbgOutLog
//
//---------------------------------------------------------------------------
// Description		: Class for reporting errors as windows message boxes
//===========================================================================
class AIDLIB_API cDbgOutLog : public cLog
{
private:
	virtual void AddString(const TCHAR* LogString, const eSeverity Severity) throw();
public:
};


//===========================================================================
// @{CSEH}
//								cEventLog
//
//---------------------------------------------------------------------------
// Description		: Class for reporting events into winnt event log.
//===========================================================================
class AIDLIB_API cEventLog : public cLog
{
private:
	TCHAR EventsSourceName[MAX_PATH];
	virtual void AddString(const TCHAR *LogString, const eSeverity Severity) throw();
public:
	cEventLog() throw ( );
};


//===========================================================================
// @{CSEH}
//								cFileLog
//
//---------------------------------------------------------------------------
// Description		: Class for logging into file.
//===========================================================================
class AIDLIB_API cFileLog : public cLog
{
protected:
	//Name of file to log in	(the same as module name + .log)
	TCHAR LogFileName[MAX_PATH];
	virtual void AddString(const TCHAR *LogString, const eSeverity Severity) throw();
#ifdef _UNICODE
	//The same as the previous but for char buffers
	void AddString(const char *LogString, const eSeverity Severity) throw();
#endif //_UNICODE
public:
	cFileLog(const TCHAR* LogFileName) throw ( );

	//===========================================================================
	//
	// @{FUN}                              ClearLog()
	//
	//---------------------------------------------------------------------------
	// Effects 		: Use to clear up log file
	// Return type	: bool 
	// Errors		: on errors false returned (see winerror for details)
	//===========================================================================
	virtual bool ClearLog() throw ( );

	//As a name for the log application name with .log extension taken
	//in this case
	cFileLog() throw ( );
};

//------------------------Universal log class-----------------------------



//===========================================================================
// @{CSEH}
//								cBuffLog
//
//---------------------------------------------------------------------------
// Description		: Class for logging to abstract buffer
//===========================================================================
class AIDLIB_API cBuffLog : public cLog
{
private:
	//Array of received messages
	std::vector<tstring> Messages;
	virtual void AddString(const TCHAR *LogString, const eSeverity Severity) throw();

	//Summ of all messages lengthes including null characters
	unsigned int FullLen;
public:

	cBuffLog() throw () : cLog(), FullLen(1)
	{
	}

	//===========================================================================
	//
	// @{FUN}                          ~cBuffLog()
	//
	//---------------------------------------------------------------------------
	// Effects		: Destroys cBufLog object
	// Errors		: No errors handled
	//===========================================================================
	virtual ~cBuffLog() throw();

	//===========================================================================
	//
	// @{FUN}                          GetLogBuffer()
	//
	//---------------------------------------------------------------------------
	// Effects		: Returns all log in form of buffer
	// Arguments	: unsigned int *Len [OUT] The length of result buffer in bytes
	// Errors		: On error NULL returned
	//===========================================================================	
	char* GetLogBuffer(unsigned int *Len) throw();
};


//===========================================================================
// @{CSEH}
//								cAnyLog
//
//---------------------------------------------------------------------------
// Description		: Main logging class. It can report events basing on its severity matrix.
//===========================================================================
class AIDLIB_API cAnyLog : public cLog
{
private:
																						
	/// Critical section for internal issues
	CRITICAL_SECTION m_cs;

	#define MSeverityTypesCount _FTEST_CASE-_MESSAGE+1
	
	//Log chains for every severity type
	std::deque<cLog*> Logs[MSeverityTypesCount];

	virtual void AddString(const TCHAR *LogString, const eSeverity Severity) throw();
	virtual void AddList(const cEventDesc &EventDesc, const TCHAR *Item, ...) throw( );

public:
	//===========================================================================
	//
	// @{FUN}                          RegisterLog()
	//
	//---------------------------------------------------------------------------
	// Effects		: Adds new Log object for logging
	//  Argument	: cLog *NewLog [IN] new log object
	//  Argument	: const bool PushBack=true [IN]- defines push log back or forward
	//	Argument	: const eSeverity Severity=-1 [IN]//for with severity. if -1 then to all of them
	// Errors		: No errors handled
	//===========================================================================
	void RegisterLog(cLog *NewLog, const bool PushBack=true, const int Severity=-1) throw();

	//===========================================================================
	//
	// @{FUN}                          UnRegisterLog()
	//
	//---------------------------------------------------------------------------
	// Effects		: Removes Log object from logging
	//  Argument	: cLog *log [IN] log object to unregister
	// Errors		: No errors handled
	//===========================================================================
	void UnRegisterLog(const cLog *log);

    void ClearList() throw();

	//Default constructor
	cAnyLog(const bool &Empty = false) throw ( )
	{
		try
		{
			InitializeCriticalSection(&m_cs);
			if (Empty) return;

			#ifdef _DEBUG
				RegisterLog(new cDbgOutLog());
			#endif
		}
		catch(...)
		{
		}
	}

	//===========================================================================
	//
	// @{FUN}                          ~cAnyLog()
	//
	//---------------------------------------------------------------------------
	// Effects		: Destroys cAnyLog object
	// Errors		: No errors handled
	//===========================================================================
	virtual ~cAnyLog() throw ( )
	{
		try
		{
			ClearList();
			DeleteCriticalSection(&m_cs);
		}
		catch(...)
		{
		}
	}

	virtual void SetVerbosity(const eVerbosity verbosity) throw( );
	virtual void SetFormat(const eLogFormat logFormat) throw( );

};

///  Access through CSingleton
class AIDLIB_API CCrtExeptionHook
{
public:
	CCrtExeptionHook()
	{
		#ifndef SKIP_CRT_EXCEPTION_HOOK
			m_sehTranslator = _set_se_translator(CExceptionFromSEH);
			m_cppHook = _CrtSetReportHook(CExceptionFromCPP);
		#endif
	}
	~CCrtExeptionHook()
	{
		#ifndef SKIP_CRT_EXCEPTION_HOOK
			_CrtSetReportHook(m_cppHook);
			_set_se_translator(m_sehTranslator);
		#endif
	}
private:
	_se_translator_function		m_sehTranslator;
	_CRT_REPORT_HOOK					m_cppHook;

};


#ifdef _CONSOLE
	#ifdef AssistLib_H
	//	cConsLog __ConsLog__;
	#endif //AssistLib_H
#endif //_CONSOLE
//------------------------------------------------------------------------


#define McEventDesc(Severity,Verbosity,Suite,Case) cLog::cEventDesc(	Severity,\
																	Verbosity,\
																	_T(__FILE__),\
																	__LINE__,\
																	_T(__DATE__),\
																	0,\
																	Suite,\
																	Case,\
																	NULL)


#define _MESSAGE_ McEventDesc(cLog::_MESSAGE,_TRACE_DEBUG_,NULL,NULL)
#define _WARNING_ McEventDesc(cLog::_WARNING,_TRACE_DEBUG_,NULL,NULL)
#define _ERROR_ McEventDesc(cLog::_ERROR,_TRACE_DEBUG_,NULL,NULL)
#define _UTEST_SUITE_(Name) McEventDesc(cLog::_UTEST_SUITE,_TRACE_DEBUG_,Name,NULL)
#define _UTEST_CASE_(Name) McEventDesc(cLog::_UTEST_CASE,_TRACE_DEBUG_,NULL,Name)
#define _FTEST_SUITE_(Name) McEventDesc(cLog::_FTEST_SUITE,_TRACE_DEBUG_,Name,NULL)
#define _FTEST_CASE_(Name) McEventDesc(cLog::_FTEST_CASE,_TRACE_DEBUG_,NULL,Name)
#define _CALL_ McEventDesc(cLog::_MESSAGE,_TRACE_CALLS_,NULL,NULL)



//Loggs exception
#define MLog_Exception(ex) Log.Add(ex);

template <typename T>
class cLogSingleton
{
	cLogSingleton();
	cLogSingleton(const cLogSingleton&);
	cLogSingleton& operator=(const cLogSingleton&);
public:
	static T& instance()
	{
		static T obj;
		return obj;
	}
};

//#define CONSTRUCTOR( name )	 #name

#include <AidLib/CSingleton/CSingleton.h>

//Standart log
#define Log CProcessSingleton<cAnyLog>::instance()
//#define Log cLogSingleton<cAnyLog>::instance()
#define vnclog cLogSingleton<VNCLog>::instance()
//Define macro for logging state changing
#define TRACE_STATE(Name,Value,NewValue) Log.AddState(Name,Value,NewValue)

//-------------------------------------------------------------------------------
#endif //CLOG_H
