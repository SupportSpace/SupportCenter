/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CEventThread.h
///
///  Declares CEventThread class, responsible for event calling thread
///
///  @author Dmitry Netrebenko @date 15.01.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/CThread/CThread.h>
#include <windows.h>
#include <boost/function.hpp>
#include <AidLib/Strings/tstring.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>

/// Defines type of event function
typedef boost::function<void (int, unsigned int, bool, tstring)> EventFunc;

/// CEventThread class, responsible for event calling thread
/// Base class - CThread from AidLib
class CEventThread 
	:	public CThread
{
private:
/// Prevents making copies of CEventThread objects
	CEventThread(const CEventThread&);
	CEventThread& operator=(const CEventThread&);
public:
/// Constructor
/// @param eventFunc - function which calls event
/// @param eventId - id of event
/// @param requestId - id of request
/// @param success - result
/// @param errorString - string with error description
	CEventThread(EventFunc eventFunc, int eventId, unsigned int requestId, bool success, const tstring& errorString);
/// Destructor
	~CEventThread();
/// Thread entry point
	virtual void Execute(void *Params);
/// Returns handle of complete event
	HANDLE GetCompleteEvent();
private:
/// Event function
	EventFunc												m_eventFunc;
/// Complete event
	boost::shared_ptr<boost::remove_pointer<HANDLE>::type>	m_completeEvent;
/// Id of event
	int														m_eventId;
/// Id of request
	unsigned int											m_requestId;
/// Result
	bool													m_success;
/// Error string
	tstring													m_errorString;
};
