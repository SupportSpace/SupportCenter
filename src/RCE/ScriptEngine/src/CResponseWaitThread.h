/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CResponseWaitThread.h
///
///  Declares CResponseWaitThread class, responsible for waiting of response
///
///  @author Dmitry Netrebenko @date 06.12.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/CThread/CThread.h>
#include <AidLib/CCritSection/CCritSectionObject.h>
#include <map>
#include <AidLib/Strings/tstring.h>
#include <boost/function.hpp>

/// CResponseWaitThread class, responsible for waiting of response
/// Base class - CThread from AidLib
class CResponseWaitThread
	:	public CThread
{
private:
/// Prevents making copies of CResponseWaitThread objects
	CResponseWaitThread(const CResponseWaitThread&);
	CResponseWaitThread& operator=(const CResponseWaitThread&);
public:
/// Constructor
/// @param timeout - waiting timeout
/// @param timeoutCallback - callback for deploy timeout
	CResponseWaitThread(DWORD timeout, boost::function<void(void)> timeoutCallback);
/// Destructor
	~CResponseWaitThread();
/// Thread's entry point
	virtual void Execute(void *Params);
/// Handles request
/// @param requestId - id of request
/// @return count requests in map
	unsigned int HandleRequest(unsigned int requestId);
/// Adds request to map
/// @param requestId - id of request
/// @param data
	void AddRequest(unsigned int requestId, const tstring& data);
private:
/// Critical section for map of requests
	CCritSectionSimpleObject									m_section;
/// Map of requests
	std::map<unsigned int,tstring>								m_requests;
/// Message event
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type >	m_event;
/// Callback for OnTimeout event
	boost::function<void(void)>									m_timeoutCallback;
/// Wait timeout
	DWORD														m_timeout;
};
