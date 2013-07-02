/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CEventThreadsManager.h
///
///  Declares CEventThreadsManager class, responsible for management of 
///    event threads
///
///  @author Dmitry Netrebenko @date 15.01.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/CThread/CThread.h>
#include <windows.h>
#include <map>
#include <boost/function.hpp>
#include <AidLib/Strings/tstring.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include "CEventThread.h"
#include <AidLib/CCritSection/CCritSectionObject.h>

/// CEventThreadsManager class, responsible for management of 
///   event threads
/// Base class - CThread from AidLib
class CEventThreadsManager 
	:	public CThread
{
private:
/// Prevents making copies of CEventThreadsManager objects
	CEventThreadsManager(const CEventThreadsManager&);
	CEventThreadsManager& operator=(const CEventThreadsManager&);
public:
/// Constructor
	CEventThreadsManager();
/// Destructor
	~CEventThreadsManager();
/// Thread entry point
	virtual void Execute(void *Params);
/// Creates thread for calling of event
/// @param eventFunc - function which calls event
/// @param eventId - id of event
/// @param requestId - id of request
/// @param success - result
/// @param errorString - string with error description
	void AddEvent(EventFunc eventFunc, int eventId, unsigned int requestId, bool success, const tstring& errorString);
private:
/// Map of threads
	std::map< HANDLE, boost::shared_ptr<CEventThread> >		m_eventThreads;
/// Event which will signal threads changes
	boost::shared_ptr<boost::remove_pointer<HANDLE>::type>	m_changeEvent;
/// Critical section to protect map with threads
	CCritSectionSimpleObject								m_section;
};
