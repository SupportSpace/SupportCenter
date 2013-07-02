#include "../client.h"
#include "../messagesessionhandler.h"
#include "../messageeventhandler.h"
#include "../messageeventfilter.h"
#include "../chatstatehandler.h"
#include "../chatstatefilter.h"
#include "../connectionlistener.h"
#include "../discohandler.h"
#include "../disco.h"
#include "../stanza.h"
#include "../gloox.h"
#include "../lastactivity.h"
#include "../loghandler.h"
#include "../logsink.h"
using namespace gloox;

#include <unistd.h>
#include <stdio.h>
#include <locale.h>
#include <string>

class MessageTest : public DiscoHandler, MessageSessionHandler, ConnectionListener, LogHandler,
                    MessageEventHandler, MessageHandler, ChatStateHandler
{
  public:
    MessageTest() : m_session( 0 ), m_messageEventFilter( 0 ), m_chatStateFilter( 0 ) {};

    virtual ~MessageTest() {};

    void start()
    {
      setlocale( LC_ALL, "" );

      JID jid( "hurkhurk@example.org/gloox" );
      j = new Client( jid, "hurkhurks" );
      j->setAutoPresence( true );
      j->setInitialPriority( 4 );
      j->registerConnectionListener( this );
      j->setAutoMessageSession( true, this );
      j->disco()->registerDiscoHandler( this );
      j->disco()->setVersion( "messageTest", GLOOX_VERSION, "Linux" );
      j->disco()->setIdentity( "client", "bot" );
      j->disco()->addFeature( XMLNS_CHAT_STATES );
      StringList ca;
      ca.push_back( "/path/to/cacert.crt" );
      j->setCACerts( ca );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

      if( j->connect(false) )
      {
        ConnectionError ce = ConnNoError;
        while( ce == ConnNoError )
        {
          ce = j->recv();
        }
        printf( "ce: %d\n", ce );
      }

      // cleanup
      if( m_session )
      {
        m_session->removeMessageHandler();
        delete m_chatStateFilter;
        delete m_messageEventFilter;
        delete m_session;
      }
      delete( j );
    }

    virtual void onConnect()
    {
      printf( "connected!!!\n" );
    };

    virtual void onDisconnect( ConnectionError e )
    {
      printf( "message_test: disconnected: %d\n", e );
      if( e == ConnAuthenticationFailed )
        printf( "auth failed. reason: %d\n", j->authError() );
    };

    virtual bool onTLSConnect( const CertInfo& info )
    {
      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n",
              info.status, info.issuer.c_str(), info.server.c_str(),
              info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
              info.compression.c_str() );
      return true;
    };

    virtual void handleDiscoInfoResult( Stanza * /*stanza*/, int /*context*/ )
    {
      printf( "handleDiscoInfoResult}\n" );
    }

    virtual void handleDiscoItemsResult( Stanza * /*stanza*/, int /*context*/ )
    {
      printf( "handleDiscoItemsResult\n" );
    }

    virtual void handleDiscoError( Stanza * /*stanza*/, int /*context*/ )
    {
      printf( "handleDiscoError\n" );
    }

    virtual void handleMessage( Stanza *stanza )
    {
      printf( "type: %d, subject: %s, message: %s, thread id: %s\n", stanza->subtype(),
              stanza->subject().c_str(), stanza->body().c_str(), stanza->thread().c_str() );

      std::string msg = "You said:\n> " + stanza->body() + "\nI like that statement.";
      std::string sub;
      if( !stanza->subject().empty() )
        sub = "Re: " +  stanza->subject();

      m_messageEventFilter->raiseMessageEvent( MessageEventDisplayed );
#ifdef WIN32
      Sleep( 1000 );
#else
      sleep( 1 );
#endif
      m_messageEventFilter->raiseMessageEvent( MessageEventComposing );
      m_chatStateFilter->setChatState( ChatStateComposing );
#ifdef WIN32
      Sleep( 2000 );
#else
      sleep( 2 );
#endif
      m_session->send( msg, sub );

      if( stanza->body() == "quit" )
        j->disconnect();
    }

    virtual void handleMessageEvent( const JID& from, MessageEventType event )
    {
      printf( "received event: %d from: %s\n", event, from.full().c_str() );
    }

    virtual void handleChatState( const JID& from, ChatStateType state )
    {
      printf( "received state: %d from: %s\n", state, from.full().c_str() );
    }

    virtual void handleMessageSession( MessageSession *session )
    {
      // this will leak if you talk to this bot from more than one full JID.
      m_session = session;
      printf( "got new session\n");
      m_session->registerMessageHandler( this );
      m_messageEventFilter = new MessageEventFilter( m_session );
      m_messageEventFilter->registerMessageEventHandler( this );
      m_chatStateFilter = new ChatStateFilter( m_session );
      m_chatStateFilter->registerChatStateHandler( this );
    }

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    };

  private:
    Client *j;
    MessageSession *m_session;
    MessageEventFilter *m_messageEventFilter;
    ChatStateFilter *m_chatStateFilter;
};

int main( int /*argc*/, char* /*argv[]*/ )
{
  MessageTest *r = new MessageTest();
  r->start();
  delete( r );
  return 0;
  return 0;
}
