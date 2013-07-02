#include "../component.h"
#include "../connectionlistener.h"
#include "../loghandler.h"
#include "../discohandler.h"
#include "../disco.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

class ComponentTest : public DiscoHandler, ConnectionListener, LogHandler
{
  public:
    ComponentTest() {};
    virtual ~ComponentTest() {};

    void start()
    {
      setlocale( LC_ALL, "" );

      j = new Component( XMLNS_COMPONENT_ACCEPT, "example.org",
                         "component.example.org", "secret", 5000 );
      j->disco()->setVersion( "componentTest", GLOOX_VERSION );

      j->registerConnectionListener( this );
      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

      j->connect();

      delete( j );
    }

    virtual void onConnect()
    {
      printf( "connected -- disconnecting...\n" );
//       j->disconnect( STATE_DISCONNECTED );
    };

    virtual void onDisconnect( ConnectionError /*e*/ ) { printf( "component: disconnected\n" ); };

    virtual bool onTLSConnect( const CertInfo& info )
    {
      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n",
              info.status, info.issuer.c_str(), info.server.c_str(),
              info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
              info.compression.c_str() );
      return true;
    };

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

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    };

  private:
    Component *j;
};

int main( int /*argc*/, char* /*argv[]*/ )
{
  ComponentTest *r = new ComponentTest();
  r->start();
  delete( r );
  return 0;
}
