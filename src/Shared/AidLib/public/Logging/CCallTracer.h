/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCallTracer.h
///
///  Declares CCallTracer class, responsible for calls tracing
///
///  @author Dmitry Netrebenko @date 05.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/AidLib.h>
#include <TCHAR.H>
#include <stack>
#include <windows.h>
#include <map>
#include <AidLib/CSingleton/CSingleton.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <AidLib/CCritSection/CCritSectionObject.h>
#include <AidLib/Strings/tstring.h>

/// CCallTracer class, responsible for calls tracing
class AIDLIB_API CCallTracer
{
private:

/// STraceCall, structure to store call params
	struct STraceCall
	{
		/// Source file name
		TCHAR* srcFile;
		/// Function name
		TCHAR* srcFunc;
		/// Source line number
		unsigned int srcLine;
		/// Tracing status
		bool traceEnabled;
		/// Enclosure level
		unsigned int level;
		/// Constructor
		/// @param _srcFile - source file name
		/// @param _srcFunc - function name
		/// @param _srcLine - source line number
		/// @param _traceEnabled - tracing status
		STraceCall(TCHAR* _srcFile, TCHAR* _srcFunc, unsigned int _srcLine, bool _traceEnabled)
			:	srcFile(_srcFile), srcFunc(_srcFunc), srcLine(_srcLine), traceEnabled(_traceEnabled), level(1)
		{};
	};

/// Call stack
	std::stack<STraceCall>	m_stack;
/// Current tracing status
	bool					m_traceEnabled;

public:
/// Constructor
	CCallTracer();

/// Destructor
	~CCallTracer();

/// Trace "Enter function" event
/// @param srcFile - source file name
/// @param srcFunc - function name
/// @param srcLine - source line number
	void TraceEnter(const TCHAR* srcFile, const TCHAR* srcFunc, const unsigned int srcLine);

/// Trace "Leaving function" event
	void TraceLeave();

/// Changes tracing status
/// @param enable - new status
/// @return previous status
	bool EnableTrace(bool enable);

/// Accessor to CCallTracer thread singleton
	static CCallTracer& instance()
	{
		static std::map<unsigned int, CCallTracer> objMap;
		CCritSection section(&CSingleton<CCritSectionObject<CCallTracer> >::instance());
		return objMap[GetCurrentThreadId()];
	}

///  Returns call stack as tstring
	tstring GetCallStack() const;
};
