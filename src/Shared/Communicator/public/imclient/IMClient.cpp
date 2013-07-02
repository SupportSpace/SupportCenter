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
#include <windows.h>

#include "IMClient.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"
#include "ExecutionQueueNode.h"

IMClient::IMClient(const tstring&	username,
				   const tstring&	resource,
				   const tstring&	password, 
				   const tstring&	server,
				   const tstring&	server_addr,
				   HWND				hWnd,
				   bool				bLog,
				   DWORD			dwIdleTimeout)				  
{
TRY_CATCH

	m_hWnd = hWnd;
	JID jid;

	jid.setUsername( username.c_str() );
	jid.setResource( resource.c_str() );
	jid.setServer(server.c_str());
	m_server = server;

	m_client = new Client(jid, password.c_str());
	m_client->registerConnectionListener(this);
	m_client->setServer( server_addr.c_str());
	
	// Todo we do not want auto presence  - we will need to set presence after 
	// connect due status selected by user 
	// m_client->setAutoPresence(true); 
	// m_client->setInitialPriority(4); removed in gloox 0.9
	m_client->disco()->setVersion( _T("messageTest"), GLOOX_VERSION, _T("Linux") );
	m_client->disco()->setIdentity( _T("client"), _T("bot") );

	//
	//  Our simple client version not requires Sessions
	//  The messages will be handled in the blocked handleMessage() function 
	m_client->registerMessageHandler(this); 

	//
	//  We will log this 
	if( bLog )
	    registerLogHandler();
	
	//
	//	Register this bject to receive updates on roster operations
	//  todo for future versions 
	//  m_client->rosterManager()->registerRosterListener( this ); 
	m_client->setTls(true);

	//
	//
	m_bOnDisconnectCalled = FALSE;

	//
	//
	m_bDisconnected = FALSE;

CATCH_THROW(_T("IMClient::IMClient"))
}

IMClient::~IMClient()
{
TRY_CATCH

	if( m_client != NULL )
	{
		m_client->removeMessageHandler(this);
		m_client->removeConnectionListener(this);
		Log.Add(_MESSAGE_, _T("MClient::~IMClient started"));		
		delete m_client;
		m_client = NULL;
		Log.Add(_MESSAGE_, _T("MClient::~IMClient ended"));
	}

CATCH_LOG(_T("IMClient::~IMClient"))
}

bool IMClient::connect(bool block)
{
	bool  bRetVal = false;
TRY_CATCH

	m_bOnDisconnectCalled = FALSE;
	m_bDisconnected = FALSE;

	try {
		bRetVal = m_client->connect(block); //for non blocked set false
	} 
	catch (std::exception& e)
	{
		Log.Add(_MESSAGE_, _T("connect failed: = %s"), e.what() );
	}
	
	if( bRetVal == false && m_hWnd != NULL )
	{
		PostMessage( m_hWnd, WM_IM_CONNECT_FAILED, 0, (LPARAM)0 ); 
	}

CATCH_THROW(_T("IMClient::connect"))
	return bRetVal;
}

void IMClient::disconnect()
{
TRY_CATCH

	try {
		Log.Add(_MESSAGE_,_T("IMClient::disconnect..."));
		m_client->disconnect();
		Log.Add(_MESSAGE_,_T("IMClient::disconnect finished!"));
	} 
	catch (std::exception& e)
	{
		Log.Add(_MESSAGE_, _T("disconnect failed: = %s"), e.what() );
	}

CATCH_THROW(_T("IMClient::disconnect"))
}

void IMClient::onConnect()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("IMClient::onConnect()"));//todo remove it when tray catch will print 

	if( m_hWnd )
		PostMessage( m_hWnd, WM_IM_CONNECTED, 0, (LPARAM)0 );

CATCH_THROW(_T("IMClient::onConnect"))
}

void IMClient::onDisconnect(ConnectionError conErr)
{
TRY_CATCH

	long	detailErr = 0;
	m_bOnDisconnectCalled = TRUE;

	Log.Add(_MESSAGE_,_T("Connection closed; reason: [%d]"), conErr);

	switch(conErr)
	{
	case ConnAuthenticationFailed:
		if(m_client)
		{
			detailErr = (AuthenticationError)m_client->authError();
			Log.Add(_MESSAGE_,_T("ConnAuthenticationFailed; reason: [%d]"), detailErr);
		}
		break;
	case ConnStreamError:
	case ConnStreamClosed:
		if(m_client)
		{
			detailErr = (StreamError)m_client->streamError();
			Log.Add(_MESSAGE_,_T("ConnStreamError; reason: [%d]"), detailErr);
			Log.Add(_MESSAGE_,_T("streamErrorText; reason: [%s]"), m_client->streamErrorText().c_str());
		}
		break;
	default:
		break;
	}

	if(m_hWnd)
	{
		//	you can use ClientBase::streamError() to find out what exactly went wrong, 
		//	and ClientBase::streamErrorText() to retrieve any explaining text
        //	sent along with the error.
        //	If indicates an authentication error, you can use ClientBase::authError() to get a finer
        //	grained reason.
		PostMessage(m_hWnd, WM_IM_DISCONNECT, (WPARAM)detailErr, (LPARAM)conErr);
	}

CATCH_THROW(_T("IMClient::onConnect"))
}

bool IMClient::onTLSConnect(const CertInfo& info)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("Server Certificate [%s]"), info.server.c_str() );
	Log.Add(_MESSAGE_, _T("Certificate Issuer: [%s]"), info.issuer.c_str() );

	if( m_hWnd )
		PostMessage( m_hWnd, WM_IM_TLSCONNECTED, 0, (LPARAM)0 );

CATCH_THROW(_T("IMClient::onTLSConnect"))
	return true;
}

//	TODO: to clarify
//	here is a list of messages that will be sent via XMPP protocol from SupportSpace Jubber Account
//  to Supporter's CallManager instance
//  NewCall			callid
//  DeleteCall		callid
//  InSessionCall	callid 
//  UpdateSettings	token
void IMClient::handleMessage(Stanza* stanza, MessageSession *session)
{
TRY_CATCH

	if( stanza->error() == StanzaErrorUndefined )
	{
		Log.Add(_MESSAGE_,_T("New messsge. From:%s Subject:%s Body:%s"),
			stanza->from().full().c_str(),
			stanza->subject().c_str(),
			stanza->body().substr(0, MSG_BUF_SIZE - 100).c_str());
	
		PostMessage( m_hWnd, WM_IM_NEWCALL, 0, reinterpret_cast<LPARAM> ( new CNodeSendMessage(
				stanza->from().full().c_str(), stanza->subject().c_str(), stanza->body().c_str(), _T("") ))); 
	}
	else
	{
		Log.Add(_MESSAGE_,_T("Message delivery failed: [%d] [%s]"), stanza->error(), stanza->errorText().c_str() );
	}

CATCH_THROW(_T("IMClient::handleMessage"))
	return;
}

ConnectionError IMClient::idle(int timeout)
{
	ConnectionError	err = ConnNoError; 
TRY_CATCH

	try {
		err = m_client->recv(timeout);

		if( m_hWnd )
			PostMessage( m_hWnd, WM_IM_CONNECTFAILED, 0, (LPARAM)err );
	} 
	catch (std::exception& e)
	{
		Log.Add(_MESSAGE_, _T("connect failed with exception: [%s]"), e.what() );
		err = ConnNotConnected;
	}

CATCH_THROW(_T("IMClient::idle"))
	return err;
}
void IMClient::handleLog(LogLevel level, LogArea area, const std::string& message)
{
TRY_CATCH

   // write all logs except ClientBase::whitespacePing()  send( " " );
   if( message.compare(" ")!=0 )
		Log.Add(_MESSAGE_,_T("LolLevel:%d LogArea:%d Message: %s"), level, area, message.substr(0, MSG_BUF_SIZE-100).c_str());

CATCH_THROW(_T("IMClient::handleLog"))
}

void IMClient::update_status(Presence  status, const tstring& msg)
{
TRY_CATCH

	JID		  jid;
	Stanza*	  pcStanza = Stanza::createPresenceStanza(jid, msg.c_str(), status, _T("")); 
	m_client->send( pcStanza ); // this also delete Stanza due api ref

CATCH_THROW(_T("IMClient::update_status"))
}

ConnectionState IMClient::connection_state()const
{
TRY_CATCH

	return m_client->state();

CATCH_THROW(_T("IMClient::connection_state"))
}

void IMClient::registerConnectionListener( ConnectionListener *cl )
{
TRY_CATCH

	m_client->registerConnectionListener( cl );

CATCH_THROW(_T("IMClient::registerConnectionListener"))
}

void IMClient::removeConnectionListener( ConnectionListener *cl )
{
TRY_CATCH

	m_client->removeConnectionListener( cl );

CATCH_THROW(_T("IMClient::removeConnectionListener"))
}

void IMClient::send_msg(const tstring &to, const tstring &body, const tstring &subject,const tstring & sToResource)
{
TRY_CATCH

	tstring addr;
	
	if(sToResource.size()!=0)
		addr = to + _T("@") + m_server + _T("/") + sToResource;
	else
		addr = to + _T("@") + m_server;

	JID	session_to = JID( addr.c_str() );

	Stanza* msgStanza = Stanza::createMessageStanza( session_to, body.c_str(), StanzaMessageChat, subject.c_str() );

	m_client->send( msgStanza );
	Log.Add(_MESSAGE_, _T("Message sent To:[%s] Body:[%s] Subject:[%s]"), addr.c_str(), body.c_str(), subject.c_str()  );

CATCH_THROW(_T("IMClient::send_msg"))
}

void IMClient::registerLogHandler( )
{
TRY_CATCH

	 m_client->logInstance().registerLogHandler(LogLevelDebug, LogAreaAll, this);

CATCH_THROW(_T("IMClient::registerLogHandler"))
}

void IMClient::removeLogHandler( )
{
TRY_CATCH

	m_client->logInstance().removeLogHandler(this);

CATCH_THROW(_T("IMClient::removeLogHandler"))
}

void IMClient::itemSubscribed( const std::string& jid )
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("subscribed %s"), jid.c_str() );

CATCH_THROW(_T("IMClient::itemSubscribed"))
}

void IMClient::itemAdded( const std::string& jid )
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("added %s"), jid.c_str() );

CATCH_THROW(_T("IMClient::itemAdded"))
}

void IMClient::itemUnsubscribed( const std::string& jid )
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("unsubscribed %s"), jid.c_str() );

CATCH_THROW(_T("IMClient::itemUnsubscribed"))
}

void IMClient::itemRemoved( const std::string& jid )
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("removed %s"), jid.c_str() );

CATCH_THROW(_T("IMClient::itemRemoved"))
}

void IMClient::itemUpdated( const std::string& jid )
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("updated %s"), jid.c_str() );

CATCH_THROW(_T("IMClient::itemUpdated"))
}

void IMClient::roster( const Roster& roster )
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("roster arriving\nitems:") );
	Roster::const_iterator it = roster.begin();
	for( ; it != roster.end(); ++it )
	{
		Log.Add(_MESSAGE_, _T("jid: %s, name: %s, subscription: %d"),
			(*it).second->jid().c_str(), (*it).second->name().c_str(),
			(*it).second->subscription() );
		StringList g = (*it).second->groups();
		StringList::const_iterator it_g = g.begin();
		for( ; it_g != g.end(); ++it_g )
			Log.Add(_MESSAGE_,_T("\tgroup: %s"), (*it_g).c_str() );
	}

CATCH_THROW(_T("IMClient::roster"))
}

void IMClient::presenceUpdated( const RosterItem& item, int /*status*/, const std::string& /*msg*/ )
{
TRY_CATCH

	Log.Add(_MESSAGE_, _T("item changed: %s"), item.jid().c_str() );

CATCH_THROW(_T("IMClient::presenceUpdated"))
}

void IMClient::itemAvailable( const RosterItem& item, const std::string& /*msg*/ )
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("item online: %s"), item.jid().c_str() );

CATCH_THROW(_T("IMClient::itemAvailable"))
}

void IMClient::itemUnavailable( const RosterItem& item, const std::string& /*msg*/ )
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("item offline: %s"), item.jid().c_str() );

CATCH_THROW(_T("IMClient::itemUnavailable"))
};

bool IMClient::subscriptionRequest( const std::string& jid, const std::string& /*msg*/ )
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("subscription: %s"), jid.c_str() );
	StringList groups;
	m_client->rosterManager()->subscribe(jid, _T(""), groups, _T(""));

CATCH_THROW(_T("IMClient::subscriptionRequest"))
	return true;
}

bool IMClient::unsubscriptionRequest( const std::string& jid, const std::string& /*msg*/ )
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("unsubscription: %s"), jid.c_str() );

CATCH_THROW(_T("IMClient::unsubscriptionRequest"))
	return true;
}

void IMClient::nonrosterPresenceReceived( const JID& jid )
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("received presence from entity not in the roster: %s"), jid.full().c_str() );

CATCH_THROW(_T("IMClient::nonrosterPresenceReceived"))
} 

void  IMClient::subscribeNewContact(const std::string& jid, const std::string& name, StringList groups)
{
TRY_CATCH

	//client->rosterManager()->add( jid, name, groups );
	m_client->rosterManager()->subscribe( jid, name, groups, _T(""));
	m_client->rosterManager()->synchronize();

CATCH_THROW(_T("IMClient::subscribeNewContact"))
};

void  IMClient::ping()
{
TRY_CATCH

	m_client->whitespacePing();

CATCH_THROW(_T("IMClient::ping"))
}

BOOL    IMClient::IsConnected() const
{
TRY_CATCH

	if(m_bDisconnected==TRUE)
		return FALSE;

	if(m_client!=NULL && m_client->state()==StateConnected ) 
		return TRUE;

CATCH_THROW(_T("IMClient::IsConnected"))
	return FALSE;
}