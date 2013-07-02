#include "../client.h"
#include "../messagehandler.h"
#include "../connectionlistener.h"
#include "../discohandler.h"
#include "../disco.h"
#include "../stanza.h"
#include "../gloox.h"
#include "../lastactivity.h"
#include "../flexoff.h"
#include "../flexoffhandler.h"
#include "../loghandler.h"
#include "../logsink.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

class FlexOffTest : public DiscoHandler, MessageHandler, ConnectionListener, FlexibleOfflineHandler,
                           LogHandler
{
  public:
    FlexOffTest() {}
    virtual ~FlexOffTest() {}

    void start()
    {

      JID jid( "hurkhurk@example.org/gloox" );
      j = new Client( jid, "hurkhurks" );
      j->registerConnectionListener( this );
      j->registerMessageHandler( this );
      j->disco()->registerDiscoHandler( this );
      j->disco()->setVersion( "messageTest", GLOOX_VERSION, "Linux" );
      j->disco()->setIdentity( "client", "bot" );
      StringList ca;
      ca.push_back( "/path/to/cacert.crt" );
      j->setCACerts( ca );

      f = new FlexibleOffline( j );
      f->registerFlexibleOfflineHandler( this );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

      j->connect();

      delete( j );
    }

    virtual void onConnect()
    {
      f->checkSupport();
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

    virtual void handleDiscoInfoResult( Stanza */*stanza*/, int /*context*/ )
    {
      printf( "handleDiscoInfoResult}\n" );
    }

    virtual void handleDiscoItemsResult( Stanza */*stanza*/, int /*context*/ )
    {
      printf( "handleDiscoItemsResult\n" );
    }

    virtual void handleDiscoError( Stanza */*stanza*/, int /*context*/ )
    {
      printf( "handleDiscoError\n" );
    }

    virtual void handleMessage( Stanza *stanza, MessageSession * /*session*/ )
    {
      printf( "type: %d, subject: %s, message: %s, thread id: %s\n", stanza->subtype(),
              stanza->subject().c_str(), stanza->body().c_str(), stanza->thread().c_str() );
      Tag *m = new Tag( "message" );
      m->addAttribute( "from", j->jid().full() );
      m->addAttribute( "to", stanza->from().full() );
      m->addAttribute( "type", "chat" );
      Tag *b = new Tag( "body", "You said:\n> " + stanza->body() + "\nI like that statement." );
      m->addChild( b );
      if( !stanza->subject().empty() )
      {
        Tag *s = new Tag( "subject", "Re:" +  stanza->subject() );
        m->addChild( s );
      }
      j->send( m );
    }

    virtual void handleMessage( const std::string& /*jid*/, Stanza * /*stanza*/ )
    {
    }

    virtual void handleFlexibleOfflineSupport( bool support )
    {
      if( support )
      {
        printf( "FlexOff: supported\n" );
        f->getMsgCount();
      }
      else
      {
        printf( "FlexOff: not supported\n" );
        j->disconnect();
      }
    }

    virtual void handleFlexibleOfflineMsgNum( int num )
    {
      printf( "FlexOff messgaes: %d\n", num );
      f->fetchHeaders();
    }

    virtual void handleFlexibleOfflineMessageHeaders( StringMap& headers )
    {
      printf( "FlexOff: %d headers received.\n", headers.size() );
      StringList l;
      l.push_back( "Fdd" );
      l.push_back( (*(headers.begin())).first );
      f->fetchMessages( l );
      f->removeMessages( l );
    }

    virtual void handleFlexibleOfflineResult( FlexibleOfflineResult result )
    {
      printf( "FlexOff: result: %d\n", result );
    }

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

  private:
    Client *j;
    FlexibleOffline *f;
};

int main( int /*argc*/, char** /*argv*/ )
{
  FlexOffTest *r = new FlexOffTest();
  r->start();
  delete( r );
  return 0;
}
