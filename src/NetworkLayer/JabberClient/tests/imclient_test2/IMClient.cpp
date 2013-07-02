//===========================================================================
// SupportSpace ltd. @{SRCH}
//								IMClient
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// IMClient : 
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 10/29/2006 05:21:10 PM
// Comments: First Issue
//===========================================================================
#include "IMClient.h"


IMClient::IMClient(std::string			username,
				   std::string			resource,
				   std::string			password, 
				   std::string			server,
				   std::string			server_addr,
				   HWND					hWnd,
				   bool					bLog)				  
{
	m_hWnd = NULL;
	m_hWnd = hWnd;
	JID jid;

	jid.setUsername(username);
	jid.setResource(resource);
	jid.setServer(server);
	m_server = server;

	m_client = new Client(jid, password);
	m_client->registerConnectionListener( this );
	m_client->setServer(server_addr);
	//m_client->setAutoPresence(true); //TODO ??? what happens?
	//m_client->setInitialPriority(4);
	m_client->disco()->setVersion( "messageTest", GLOOX_VERSION, "Linux");
	m_client->disco()->setIdentity("client", "bot");

	//
	//  Our simple client version not requires Sessions
	//  The messages will be handled in the blocked handleMessage() function 
	m_client->registerMessageHandler( this ); 

	m_client->setForceNonSasl();

	//
	//  We will log this 
	if( bLog )
	    registerLogHandler();
	
	//
	//	Register this bject to receive updates on roster operations.
	//  m_client->rosterManager()->registerRosterListener( this ); TODO
	m_client->setTls(true);
}

IMClient::~IMClient()
{
	if( m_client != NULL )
	{
		delete m_client;
	}
}

bool IMClient::connect(bool block)
{
	bool  bRetVal = false;

	try {
		bRetVal = m_client->connect(block); //for non blocked set false
	} 
	catch (std::exception& e)
	{
		ATLTRACE ( _T("connect failed: = %s\n"), e.what() );
	}

	return bRetVal;
}

void IMClient::disconnect()
{
	try {
		m_client->disconnect();
	} 
	catch (std::exception& e)
	{
		ATLTRACE ( _T("disconnect failed: = %s\n"), e.what() );
	}
}

void IMClient::onConnect()
{
	ATLTRACE ( _T("Connected to server.") );
	if( m_hWnd )
		PostMessage( m_hWnd, WM_CONNECTED, 0, (LPARAM)0 );
}

void IMClient::onDisconnect(ConnectionError e)
{
	ATLTRACE ( _T("Connection closed; reason: [%d]"), e );
	if( m_hWnd )
		PostMessage( m_hWnd, WM_DISCONNECT, 0, (LPARAM)e );
}

bool IMClient::onTLSConnect( const CertInfo& info )
{
	ATLTRACE ( _T("TLS connection established.") );
	ATLTRACE ( _T("Server Certificate [%s]"), info.server );
	ATLTRACE ( _T("Certificate Issuer: [%s]"), info.issuer );

	return true;
}

void IMClient::handleMessage(Stanza *stanza, MessageSession *session)
{
	printf("New messsge. From:%s Subject:%s Body:%s\n",	stanza->from().full().c_str(),stanza->subject().c_str(),stanza->body().c_str() );

	if( stanza->error() == StanzaErrorUndefined )
	{
		ATLTRACE ( _T("Message From [%s]"), stanza->from().full().c_str() );
		ATLTRACE ( _T("Message Body [%s]"), stanza->body().c_str());
		ATLTRACE ( _T("Message Subject [%s]"), stanza->subject().c_str() );
	}
	else
	{
		ATLTRACE ( _T("Message delivery failed: [%d] [%s]"), stanza->error(), stanza->errorText() );
		return;
	}

	//	here is a list of messages that will be sent via XMPP protocol from SupportSpace Jubber Account
	//  to Supporter's CallManager instance
	// 
	//  NewCall			callid
	//  DeleteCall		callid
	//  InSessionCall	callid 
	//  UpdateSettings	token

	/*
	CRequest*	cpRequest = new CRequest();//will be deleted later!
	ParseRequest( cpRequest, stanza );
	if( (cpRequest->GetRequestSubject()).CompareNoCase(REQUESTSUBJECT_NEW)==0 )
		PostMessage( m_hWnd, WM_NEWREQUEST, 0, (LPARAM)cpRequest ); 
	else
		if( (cpRequest->GetRequestSubject()).CompareNoCase(REQUESTSUBJECT_DELETE)==0 )
			PostMessage( m_hWnd, WM_DELETEREQUEST, 0, (LPARAM)cpRequest ); 
		else
		{
			//TODO ... not supported request
			ATLTRACE ( _T("not supported request. %s"), cpRequest->GetRequestSubject() );

			if( cpRequest != NULL )
				delete cpRequest;
		}
	}
	*/
}

ConnectionError IMClient::idle(int timeout)
{
	ConnectionError	err = ConnNoError; 

	try {
		err = m_client->recv(timeout);
	} 
	catch (std::exception& e)
	{
		ATLTRACE (  _T("connect failed with exception: [%s]"), e.what() );
		return ConnNotConnected;
	}

	return err;
}
void IMClient::handleLog(LogLevel level, LogArea area, const std::string& message)
{
	ATLTRACE (  _T("LolLevel:%d LogArea:%d Message: %s"), level, area, message.c_str() );
}

void IMClient::update_status(Presence  status)
{
	JID		  jid;
	Stanza*	  pcStanza = Stanza::createPresenceStanza  (jid,"",status,""); 
	m_client->send( pcStanza ); // Also delete Stanza
}

ConnectionState IMClient::connection_state()const
{
	return m_client->state();
}

void IMClient::registerConnectionListener( ConnectionListener *cl )
{
	m_client->registerConnectionListener( cl );
}

void IMClient::removeConnectionListener( ConnectionListener *cl )
{
	m_client->removeConnectionListener( cl );
}

void IMClient::send_msg(std::string &to, std::string &body, std::string &subject)
{
	std::string addr = to + "@" + m_server + "//IMClient";
	JID	session_to = JID( addr );

	Stanza* msgStanza = Stanza::createMessageStanza(session_to, body, StanzaMessageChat, subject );

	m_client->send( msgStanza );
	printf( "message sent \n");
}

void IMClient::registerLogHandler( )
{
	 m_client->logInstance().registerLogHandler(LogLevelDebug, LogAreaAll, this);
}

void IMClient::removeLogHandler( )
{
	m_client->logInstance().removeLogHandler(this);
}

void IMClient::itemSubscribed( const std::string& jid )
{
	printf( "subscribed %s\n", jid.c_str() );
}

void IMClient::itemAdded( const std::string& jid )
{
	printf( "added %s\n", jid.c_str() );
}

void IMClient::itemUnsubscribed( const std::string& jid )
{
	printf( "unsubscribed %s\n", jid.c_str() );
}

void IMClient::itemRemoved( const std::string& jid )
{
	printf( "removed %s\n", jid.c_str() );
}

void IMClient::itemUpdated( const std::string& jid )
{
	printf( "updated %s\n", jid.c_str() );
}

void IMClient::roster( const Roster& roster )
{
	printf( "roster arriving\nitems:\n" );
	Roster::const_iterator it = roster.begin();
	for( ; it != roster.end(); ++it )
	{
		printf( "jid: %s, name: %s, subscription: %d\n",
			(*it).second->jid().c_str(), (*it).second->name().c_str(),
			(*it).second->subscription() );
		StringList g = (*it).second->groups();
		StringList::const_iterator it_g = g.begin();
		for( ; it_g != g.end(); ++it_g )
			printf( "\tgroup: %s\n", (*it_g).c_str() );
	}
}

void IMClient::presenceUpdated( const RosterItem& item, int /*status*/, const std::string& /*msg*/ )
{
	printf( "item changed: %s\n", item.jid().c_str() );
}

void IMClient::itemAvailable( const RosterItem& item, const std::string& /*msg*/ )
{
	printf( "item online: %s\n", item.jid().c_str() );
}

void IMClient::itemUnavailable( const RosterItem& item, const std::string& /*msg*/ )
{
	printf( "item offline: %s\n", item.jid().c_str() );
};

bool IMClient::subscriptionRequest( const std::string& jid, const std::string& /*msg*/ )
{
	printf( "subscription: %s\n", jid.c_str() );
	StringList groups;
	m_client->rosterManager()->subscribe( jid, "", groups, "" );
	return true;
}

bool IMClient::unsubscriptionRequest( const std::string& jid, const std::string& /*msg*/ )
{
	printf( "unsubscription: %s\n", jid.c_str() );
	return true;
}

void IMClient::nonrosterPresenceReceived( const JID& jid )
{
	printf( "received presence from entity not in the roster: %s\n", jid.full().c_str() );
} 

void	IMClient::subscribeNewContact(const std::string& jid, const std::string& name, StringList groups)
{
	printf( "SubscribeNewContact called\n" );
	//client->rosterManager()->add( jid, name, groups );
	m_client->rosterManager()->subscribe( jid, name, groups, "" );
	m_client->rosterManager()->synchronize();
};
