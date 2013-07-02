#include "../client.h"
#include "../connectionlistener.h"
#include "../adhoccommandprovider.h"
#include "../disco.h"
#include "../adhoc.h"
#include "../tag.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>


class AdhocTest : public ConnectionListener, AdhocCommandProvider
{
  public:
    AdhocTest() {};
    virtual ~AdhocTest() {};

    void start()
    {
      setlocale( LC_ALL, "" );

      JID jid( "hurkhurk@example.org/gloox" );
      j = new Client( jid, "hurkhurks" );
      j->disableRoster();
      j->registerConnectionListener( this );
      j->disco()->setVersion( "adhocTest", GLOOX_VERSION );
      j->disco()->setIdentity( "client", "bot" );

      a = new Adhoc( j, j->disco() );
      a->registerAdhocCommandProvider( this, "helloworld", "Hello World!" );
      a->registerAdhocCommandProvider( this, "config", "Configuration" );
      a->registerAdhocCommandProvider( this, "shutdown", "Shutdown" );

      j->connect();

      delete( j );
    }

    void handleAdhocCommand( const std::string& command, Tag */*tag*/ )
    {
      if( command == "helloworld" )
        printf( "Hello World!\n" );
      else if( command == "config" )
        printf( "configuration called\n" );
      else if( command == "shutdown" )
      {
        printf( "shutting down\n" );
      }
    }

    virtual void onConnect()
    {
    };

    virtual void onDisconnect( ConnectionError /*e*/ ) { printf( "disco_test: disconnected\n" ); };

    virtual bool onTLSConnect( const CertInfo& info )
    {
      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n",
              info.status, info.issuer.c_str(), info.server.c_str(),
              info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
              info.compression.c_str() );
      return true;
    };

  private:
    Client *j;
    Adhoc *a;
};

int main( int /*argc*/, char* /*argv[]*/ )
{
  AdhocTest *b = new AdhocTest();
  b->start();
  delete( b );
  return 0;
}
