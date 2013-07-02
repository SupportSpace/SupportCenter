/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCallTracerWrapper.h
///
///  Declares CCallTracerWrapper class - wrapper for CCallTracer class
///
///  @author Dmitry Netrebenko @date 05.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Logging/cLog.h>
#include "CCallTracer.h"

/// CCallTracerWrapper class - wrapper for CCallTracer class
class CCallTracerWrapper
{
private:
/// Prevents making copies of CCallTracerWrapper class
	CCallTracerWrapper(const CCallTracerWrapper&);
	CCallTracerWrapper& operator=(const CCallTracerWrapper&);

public:
/// Constructor
/// @param srcFile - source file name
/// @param srcFunc - function name
/// @param srcLine - source line number
	CCallTracerWrapper(const TCHAR* srcFile, const TCHAR* srcFunc, const unsigned int srcLine)
	{
		try
		{
			if(_TRACE_CALLS_ == Log.m_verbosity)
				CCallTracer::instance().TraceEnter(srcFile, srcFunc, srcLine);
		}
		catch(...)
		{
		}
	};

/// Destructor
	~CCallTracerWrapper()
	{
		try
		{
			if(_TRACE_CALLS_ == Log.m_verbosity)
				CCallTracer::instance().TraceLeave();
		}
		catch(...)
		{
		}
	};
};

