/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSyncTest.h
///
///  Declares CSyncTest class, sync test
///
///  @author Dmitry Netrebenko @date 09.08.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include "CAbstractTest.h"
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <AidLib/Strings/tstring.h>
#include "STestParams.h"
#include "SConnectParams.h"
#include <set>

///  CSyncTest class, sync test
///  Base class - CAbstractTest
class CSyncTest :
	public CAbstractTest
{
private:
/// Prevents making copies of CSyncTest objects.
	CSyncTest(const CSyncTest&);
	CSyncTest& operator=(const CSyncTest&);

public:
/// Constructor
	CSyncTest();

/// Destructor
	virtual ~CSyncTest();

/// Initializes internal data
/// @param testParams - structure with test parameters
/// @param connectParams - structure with connection parameters
///	virtual void Init(const STestParams& testParams, const SConnectParams& connectParams);

/// Stops test
	virtual void Stop();

protected:
/// Runs test routines
	virtual void DoTest();

/// "OnMessage" event handler
/// @param msg - received message
	virtual void OnMessageReceived(const tstring& msg);

private:
/// Runs client routines
	void DoClientTest();

/// Runs server routines
	void DoServerTest();

private:
/// Event to wait message
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_event;

/// Set to store messages
	std::set<tstring> m_messages;

/// Critical section to access set of messages
	CRITICAL_SECTION m_section;
};
