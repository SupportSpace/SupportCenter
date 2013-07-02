/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CGlooxMessenger.cpp
///
///  implements CGlooxMessenger class, messanger which uses gloox library 
///    to access jabber
///
///  @author Dmitry Netrebenko @date 09.08.2007
///
////////////////////////////////////////////////////////////////////////

#include "CGlooxMessenger.h"
#include <AidLib/CException/CException.h>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <gloox/connectiontcpclient.h>

CGlooxMessenger::CGlooxMessenger()
{
TRY_CATCH
	m_event.reset( CreateEvent(NULL, TRUE, FALSE, NULL), CloseHandle );
CATCH_THROW()
}

CGlooxMessenger::~CGlooxMessenger()
{
TRY_CATCH
	if(m_client.get())
		m_client->disconnect();
	if(m_thread.get())
		m_thread->join();
CATCH_LOG()
}

void CGlooxMessenger::Init(const STestParams& testParams, const SConnectParams& connectParams)
{
TRY_CATCH
	CMessenger::Init(testParams, connectParams);

	/// Create connection string
	tstring user = m_connectParams.m_userName + "@" + m_connectParams.m_serverName + "/" + m_connectParams.m_resource;
	/// Create JID
	gloox::JID jid(user.c_str());
	/// Create Jabber client
	m_client.reset(new gloox::Client(jid, m_connectParams.m_userPasswd.c_str()));
	m_client->registerConnectionListener(this);
	m_client->registerMessageHandler(this);

	gloox::ConnectionTCPClient* connector = new gloox::ConnectionTCPClient(m_client.get(), m_client->logInstance(), m_connectParams.m_serverAddr.c_str(), m_connectParams.m_serverPort );
	m_client->setConnectionImpl(connector);

	/// Reset event and start thread with connect()
	ResetEvent(m_event.get());
	m_error = gloox::ConnNoError;
	m_thread.reset(new boost::thread(boost::bind(&CGlooxMessenger::ConnectThreadEntryPoint, this)));
	/// Wait for connection
	if(WAIT_TIMEOUT == WaitForSingleObject(m_event.get(), m_connectParams.m_connectTimeout))
		throw MCException(_T("Connect timeout expiered"));
	else
	{
		if(gloox::ConnNoError != m_error)
			throw MCException(Format(_T("Jabber connection error: %d"), m_error));
	}
	Log.Add(_MESSAGE_, _T("Connected"));
CATCH_THROW()
}

void CGlooxMessenger::DeInit()
{
TRY_CATCH
	if(m_client.get())
		m_client->disconnect();
CATCH_THROW()
}

void CGlooxMessenger::Send(const tstring& msg)
{
TRY_CATCH
	/// Create connection string
	tstring user = m_connectParams.m_remoteUserName + "@" + m_connectParams.m_serverName + "/" + m_connectParams.m_resource;
	/// Create JID
	gloox::JID jid(user.c_str());
	/// Create message
//	boost::scoped_ptr<gloox::Stanza> stanza(gloox::Stanza::createMessageStanza(jid, msg.c_str()));
	gloox::Stanza* stanza = gloox::Stanza::createMessageStanza(jid, msg.c_str());
	/// Send message
	m_client->send(stanza);
	Log.Add(_MESSAGE_, _T("---> %s"), msg.c_str());
CATCH_THROW()
}

void CGlooxMessenger::ConnectThreadEntryPoint()
{
TRY_CATCH
	if(m_client.get())
		m_client->connect();
CATCH_LOG()
}

void CGlooxMessenger::onConnect()
{
TRY_CATCH
	SetEvent(m_event.get()); 
CATCH_THROW()
}

void CGlooxMessenger::onDisconnect(gloox::ConnectionError e)
{ 
TRY_CATCH
	m_error = e;
	SetEvent(m_event.get()); 
CATCH_THROW()
}

bool CGlooxMessenger::onTLSConnect(const gloox::CertInfo& info)
{ 
TRY_CATCH
	return true; 
CATCH_THROW()
}

void CGlooxMessenger::handleMessage(gloox::Stanza *stanza, gloox::MessageSession *session)
{
TRY_CATCH
	tstring msg(stanza->body().c_str());
	if(!msg.length())
		return;
	Log.Add(_MESSAGE_, _T("<--- %s"), msg.c_str());
	RaiseOnMessage(msg);
CATCH_THROW()
}

