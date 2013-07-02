#include "../client.h"
#include "../messagesessionhandler.h"
#include "../messageeventhandler.h"
#include "../messageeventfilter.h"
#include "../chatstatehandler.h"
#include "../chatstatefilter.h"
#include "../connectionlistener.h"
#include "../disco.h"
#include "../stanza.h"
#include "../gloox.h"
#include "../lastactivity.h"
#include "../loghandler.h"
#include "../logsink.h"
#include "../inbandbytestream.h"
#include "../inbandbytestreammanager.h"
#include "../inbandbytestreamhandler.h"
#include "../inbandbytestreamdatahandler.h"
#include "../messagehandler.h"
using namespace gloox;

#include <unistd.h>
#include <stdio.h>
#include <string>

#if defined( WIN32 ) || defined( _WIN32 )
# include <windows.h>
#endif

class IBBTest : public MessageSessionHandler, ConnectionListener, LogHandler,
                    MessageEventHandler, MessageHandler, ChatStateHandler, InBandBytestreamHandler,
                    InBandBytestreamDataHandler
{
  public:
    IBBTest() : m_session( 0 ), m_messageEventFilter( 0 ), m_chatStateFilter( 0 ),
    m_ibbManager( 0 ), m_ibb( 0 ), c( 0 ), m_send( false ) {}

    virtual ~IBBTest() {}

    void start()
    {

      JID jid( "hurkhurk@example.org/gloox" );
      j = new Client( jid, "hurkhurks" );
      j->registerConnectionListener( this );
      j->registerMessageSessionHandler( this, 0 );
      j->disco()->setVersion( "messageTest", GLOOX_VERSION, "Linux" );
      j->disco()->setIdentity( "client", "bot" );
      j->disco()->addFeature( XMLNS_CHAT_STATES );
      StringList ca;
      ca.push_back( "/path/to/cacert.crt" );
      j->setCACerts( ca );

      m_ibbManager = new InBandBytestreamManager( j );
      m_ibbManager->registerInBandBytestreamHandler( this );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

      if( j->connect(false) )
      {
        ConnectionError ce = ConnNoError;
        while( ce == ConnNoError )
        {
          ce = j->recv();
          if( m_send )
          {
            m_ibb->sendBlock( "some data!\n" );
            printf( "sending\n" );
            ++c;
            if( c == 10 )
              m_send = false;
          }
        }
        printf( "ce: %d\n", ce );
      }

      // cleanup
     delete( j );
    }

    virtual void onConnect()
    {
      printf( "connected!!!\n" );
      JID jid( "you@example.org/res" );
      m_ibbManager->requestInBandBytestream( jid, this );
    }

    virtual void onDisconnect( ConnectionError e )
    {
      printf( "message_test: disconnected: %d\n", e );
      if( e == ConnAuthenticationFailed )
        printf( "auth failed. reason: %d\n", j->authError() );
    }

    virtual bool onTLSConnect( const CertInfo& info )
    {
      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n",
              info.status, info.issuer.c_str(), info.server.c_str(),
              info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
              info.compression.c_str() );
      return true;
    }

    virtual void handleMessage( Stanza *stanza, MessageSession * /*session*/ )
    {
      printf( "type: %d, subject: %s, message: %s, thread id: %s\n", stanza->subtype(),
              stanza->subject().c_str(), stanza->body().c_str(), stanza->thread().c_str() );

      std::string msg = "You said:\n> " + stanza->body() + "\nI like that statement.";
      std::string sub;
      if( !stanza->subject().empty() )
        sub = "Re: " +  stanza->subject();

      m_messageEventFilter->raiseMessageEvent( MessageEventDisplayed );
#if defined( WIN32 ) || defined( _WIN32 )
      Sleep( 1000 );
#else
      sleep( 1 );
#endif
      m_messageEventFilter->raiseMessageEvent( MessageEventComposing );
      m_chatStateFilter->setChatState( ChatStateComposing );
#if defined( WIN32 ) || defined( _WIN32 )
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
    }

    virtual bool handleIncomingInBandBytestream( const JID& from, InBandBytestream *ibb )
    {
      m_ibb = ibb;
      if( !m_session )
        m_session = new MessageSession( j, from );
      else
        printf( "already have a session\n" );
      m_ibb->attachTo( m_session );
      m_ibb->registerInBandBytestreamDataHandler( this );
      m_send = true;
      return true;
    }

    virtual void handleOutgoingInBandBytestream( const JID& to, InBandBytestream *ibb )
    {
      printf( "got requested ibb\n" );
      m_ibb = ibb;
      if( !m_session )
        m_session = new MessageSession( j, to );
      else
        printf( "already have a session\n" );
      m_ibb->attachTo( m_session );
      m_ibb->registerInBandBytestreamDataHandler( this );
      m_send = true;
    }

    virtual void handleInBandBytestreamError( const JID& /*remote*/, StanzaError /*se*/ )
    {
      printf( "unused\n" );
    }

    virtual void handleInBandData( const std::string& data, const std::string& sid )
    {
      printf( "incoming data from stream %s: %s\n", sid.c_str(), data.c_str() );
    }

    virtual void handleInBandError( const std::string& /*sid*/, const JID& /*remote*/, StanzaError /*se*/ )
    {
      printf( "unused\n" );
    }

    virtual void handleInBandClose( const std::string& /*sid*/, const JID& /*from*/ )
    {
      printf( "bytestream closed\n" );
    }

  private:
    Client *j;
    MessageSession *m_session;
    MessageEventFilter *m_messageEventFilter;
    ChatStateFilter *m_chatStateFilter;
    InBandBytestreamManager *m_ibbManager;
    InBandBytestream *m_ibb;
    int c;
    bool m_send;
};

int main( int /*argc*/, char** /*argv*/ )
{
  IBBTest *r = new IBBTest();
  r->start();
  delete( r );
  return 0;
}
