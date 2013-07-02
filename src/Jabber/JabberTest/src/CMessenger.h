/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CMessenger.h
///
///  Declares CMessenger class, base messenger
///
///  @author Dmitry Netrebenko @date 09.08.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "STestParams.h"
#include "SConnectParams.h"
#include "TestEvents.h"
#include <AidLib/Strings/tstring.h>

///  CMessenger class, base messenger
class CMessenger
{
private:
/// Prevents making copies of CMessenger objects.
	CMessenger(const CMessenger&);
	CMessenger& operator=(const CMessenger&);

public:
/// Constructor
	CMessenger();

/// Destructor
	virtual ~CMessenger();

/// Initializes internal data
/// @param testParams - structure with test parameters
/// @param connectParams - structure with connection parameters
	virtual void Init(const STestParams& testParams, const SConnectParams& connectParams);

/// Deinitializes session data
	virtual void DeInit() = NULL;

/// Sends message through jabber
/// @param msg - message
	virtual void Send(const tstring& msg) = NULL;

private:
/// On message received event
	OnMessageEvent		m_onMessage;

protected:
/// Tests parameters
	STestParams			m_testParams;

/// Connection parameters
	SConnectParams		m_connectParams;

protected:
/// Raises "OnMessage" event
/// @param successful - received message
	void RaiseOnMessage(const tstring& msg);

public:
/// Sets up "OnMessage" event handler
/// @param handler - event handler
	void SetOnMessageEvent(OnMessageEvent handler);

/// Returns test parameters structure
	STestParams& GetTestParams();

/// Returns connection parameters structure
	SConnectParams& GetConnectParams();
};
