/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CCallTracer.cpp
///
///  Implements CCallTracer class, responsible for calls tracing
///
///  @author Dmitry Netrebenko @date 05.03.2007
///
////////////////////////////////////////////////////////////////////////

#include <AidLib/Logging/CCallTracer.h>
#include <AidLib/Logging/cLog.h>

#define LOG_ENTERING_FUNCTION(Name) Log.Add(_CALL_,_T("Entering function \"%s\""),Name)
#define LOG_LEAVING_FUNCTION(Name) Log.Add(_CALL_,_T("Leaving function \"%s\""),Name) 

CCallTracer::CCallTracer()
	:	m_traceEnabled(true)
{
}

CCallTracer::~CCallTracer()
{
}

void CCallTracer::TraceEnter(const TCHAR* srcFile, const TCHAR* srcFunc, const unsigned int srcLine)
{
	bool isnew = false;
	unsigned int line = srcLine;
	/// Create new STraceCall structure using params
	STraceCall call(const_cast<TCHAR*>(srcFile), const_cast<TCHAR*>(srcFunc), line, m_traceEnabled);
	if(!m_stack.empty())
	{
		/// Get previous call
		STraceCall& prev = m_stack.top();
		/// If files are equals, function are equals and lines are not equals then it is enclosed
		/// call of TRY_CATCH macro and we shouldn't add to stack, just level increment
		if(prev.srcFile == call.srcFile)
		{
			if(prev.srcFunc == call.srcFunc)
			{
				if(prev.srcLine == call.srcLine)
					isnew = true;
				else
				{
					isnew = false;
					prev.level++;
				}
			}
			else
				isnew = true;
		}
		else
			isnew = true;
	}
	else
		isnew = true;

	/// If it is new stack entry then add to stack and add log message if tracing enabled
	if(isnew)
	{
		m_stack.push(call);
		if(call.traceEnabled)
			LOG_ENTERING_FUNCTION(call.srcFunc);
	}
}

void CCallTracer::TraceLeave()
{
	/// Exit if stack is empty 
	if(m_stack.empty())
		return;
	/// Get reference to last call;
	STraceCall& last = m_stack.top();
	/// Decrement level
	last.level--;
	/// If level == 0 then remove call from stack and add log message if tracing enabled
	if(!last.level)
	{
		if(last.traceEnabled)
			LOG_LEAVING_FUNCTION(last.srcFunc);
		m_traceEnabled = last.traceEnabled;
		m_stack.pop();
	}
}

bool CCallTracer::EnableTrace(bool enable)
{
	/// Change status and return previous value
	bool oldValue = m_traceEnabled;
	m_traceEnabled = enable;
	return oldValue;
}

tstring CCallTracer::GetCallStack() const
{
	std::stack<STraceCall> stack = m_stack;
	tstring ret(_T(""));
	try
	{
		while(!stack.empty())
		{
			STraceCall& last = stack.top();

			ret += last.srcFunc;
			ret += _T("  -  ");
			ret += last.srcFile;
			ret += _T(" : ");
			ret += i2tstring(last.srcLine);
			if(1 != stack.size())
				ret += _T("\n");

			stack.pop();
		}
	}
	catch(...)
	{
	}

	return ret;
}
