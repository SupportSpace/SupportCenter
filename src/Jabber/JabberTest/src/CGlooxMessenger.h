/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CGlooxMessenger.h
///
///  Declares CGlooxMessenger class, messanger which uses gloox library 
///    to access jabber
///
///  @author Dmitry Netrebenko @date 09.08.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include "CMessenger.h"
#include <gloox/gloox.h>
#include <gloox/client.h>
#include <gloox/jid.h>
#include <gloox/connectionlistener.h>
#include <gloox/messagehandler.h>
#include <gloox/stanza.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <AidLib/Strings/tstring.h>
#include "STestParams.h"
#include "SConnectParams.h"
#include <boost/thread.hpp>

///  CGlooxMessenger class, messenger which uses gloox library 
///    to access jabber
///  Base class - CMassenger
///  Base class - Connection Listener
///  Base class - Message handler
class CGlooxMessenger :
	public CMessenger,
	public gloox::ConnectionListener,
	public gloox::MessageHandler
{
public:
/// Constructor
	CGlooxMessenger();

/// Destructor
	virtual ~CGlooxMessenger();

/// Initializes internal data
/// @param testParams - structure with test parameters
/// @param connectParams - structure with connection parameters
	virtual void Init(const STestParams& testParams, const SConnectParams& connectParams);

/// Deinitializes session data
	virtual void DeInit();

/// Sends message through jabber
/// @param msg - message
	virtual void Send(const tstring& msg);

private:
/// Jabber client
	boost::shared_ptr<gloox::Client> m_client;

/// Thread for connection
	boost::shared_ptr<boost::thread> m_thread;

/// Event to wait connection
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_event;

/// Connection error
	gloox::ConnectionError m_error;

private:
/// Entry point for connection thread
	void ConnectThreadEntryPoint();

/// OnConnect callback, sets event
	void onConnect();

/// OnDisconnect callback, sets event
	void onDisconnect(gloox::ConnectionError e);

/// OnTLSConnect callback
	bool onTLSConnect(const gloox::CertInfo& info);

/// Message handler
	void handleMessage(gloox::Stanza *stanza, gloox::MessageSession *session = 0);
};
