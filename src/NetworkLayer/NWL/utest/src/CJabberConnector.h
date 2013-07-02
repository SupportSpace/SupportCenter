/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CJabberConnector.h
///
///  Implements CJabberConnector class, responsible for simulation connection
///    to Jabber server
///
///  @author Dmitry Netrebenko @date 26.06.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <gloox/gloox.h>
#include <gloox/client.h>
#include <gloox/jid.h>
#include <gloox/connectionlistener.h>
#include <AidLib/Strings/tstring.h>
#include <boost/shared_ptr.hpp>
#include <boostThreads/boostThreads.h>
#include <boost/bind.hpp>

///  CJabberConnector class, responsible for simulation connection
///    to Jabber server
class CJabberConnector : public gloox::ConnectionListener
{
private:
/// Prevents making copies of CJabberConnector objects.
	CJabberConnector( const CJabberConnector& );				
	CJabberConnector& operator=( const CJabberConnector& );	
public:
///  Constructor
///  @param server name
///  @param resource name
///  @param user id
///  @param password
	CJabberConnector(
		const tstring& server, 
		const tstring& resource, 
		const tstring& userId, 
		const tstring& passwd)
	{
		/// Create event object
		m_event = CreateEvent(
			NULL,
			TRUE,
			FALSE,
			NULL );
		/// Create connection string
		tstring user = userId + "@" + server + "/" + resource;
		/// Create JID
		gloox::JID jid(user.c_str());
		/// Create Jabber client
		m_client.reset(new gloox::Client(jid, passwd.c_str()));
		m_client->registerConnectionListener(this);

		/// Reset event and start thread with connect()
		ResetEvent(m_event);
		m_thread.reset(new boost::thread(boost::bind(&CJabberConnector::ThreadEntryPoint, this)));
		/// Wait for connection
		WaitForSingleObject(m_event, JABBER_CONNECT_TIMEOUT);
	}

///  Destructor
	~CJabberConnector()
	{
		/// Reset event
		ResetEvent(m_event);
		m_client->disconnect();
		m_thread->join();
		/// Wait for desconnection
		WaitForSingleObject(m_event, JABBER_CONNECT_TIMEOUT);
		CloseHandle(m_event);
	}

///  Thread's entry point
	void ThreadEntryPoint()
	{
		/// Connect to Jabber
		m_client->connect();
	}

///  OnConnect callback, sets event
	void onConnect() { SetEvent(m_event); };

///  OnDisconnect callback, sets event
	void onDisconnect(gloox::ConnectionError e) { SetEvent(m_event); };

///  OnTLSConnect callback
	bool onTLSConnect(const gloox::CertInfo& info) { return true; };

private:
///  Jabber client
	boost::shared_ptr<gloox::Client> m_client;
///  Thread for connection
	boost::shared_ptr<boost::thread> m_thread;
///  Connect event
	HANDLE m_event;
};
