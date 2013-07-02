#include "../client.h"
#include "../connectionlistener.h"
#include "../stanza.h"
#include "../gloox.h"
#include "../disco.h"
#include "../loghandler.h"
#include "../logsink.h"
#include "../siprofileft.h"
#include "../siprofilefthandler.h"
#include "../socks5bytestreamdatahandler.h"
using namespace gloox;

#include <unistd.h>
#include <stdio.h>
#include <string>

#if defined( WIN32 ) || defined( _WIN32 )
# include <windows.h>
#endif

/**
 * Receives one file and displayes it. Does not save anything.
 */
class FTTest : public LogHandler, ConnectionListener, SIProfileFTHandler, SOCKS5BytestreamDataHandler
{
  public:
    FTTest() : m_quit( false ) {}

    virtual ~FTTest() {}

    void start()
    {

      JID jid( "hurkhurk@example.net/gloox" );
      j = new Client( jid, "hurkhurks" );
      j->registerConnectionListener( this );
      j->disco()->setVersion( "ftTest", GLOOX_VERSION, "Linux" );
      j->disco()->setIdentity( "client", "bot" );
      StringList ca;
      ca.push_back( "/path/to/cacert.crt" );
      j->setCACerts( ca );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

      f = new SIProfileFT( j, this );
      // you should obtain this using disco, really
      f->addStreamHost( JID( "proxy.jabber.org" ), "208.245.212.98", 7777 );

      if( j->connect( false ) )
      {
        ConnectionError ce = ConnNoError;
        while( ce == ConnNoError )
        {
          if( m_quit )
            j->disconnect();

          ce = j->recv( 100 );
          std::list<SOCKS5Bytestream*>::iterator it = m_s5bs.begin();
          for( ; it != m_s5bs.end(); ++it )
            (*it)->recv( 100 );
        }
        printf( "ce: %d\n", ce );
      }

      f->dispose( m_s5bs.front() );
      delete f;
      delete j;
    }

    virtual void onConnect()
    {
      printf( "connected!!!\n" );
    }

    virtual void onDisconnect( ConnectionError e )
    {
      printf( "message_test: disconnected: %d\n", e );
      if( e == ConnAuthenticationFailed )
        printf( "auth failed. reason: %d\n", j->authError() );
    }

    virtual bool onTLSConnect( const CertInfo& info )
    {
      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n"
              "from: %s\nto: %s\n",
              info.status, info.issuer.c_str(), info.server.c_str(),
              info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
              info.compression.c_str(), ctime( (const time_t*)&info.date_from ),
              ctime( (const time_t*)&info.date_to ) );
      return true;
    }

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

    virtual void handleFTRequest( const JID& from, const std::string& id, const std::string& name,
                                  long size, const std::string& hash,
                                  const std::string& date, const std::string& mimetype,
                                  const std::string& desc, int /*stypes*/, long /*offset*/, long /*length*/ )
    {
      printf( "received ft request from %s: %s (%ld bytes). hash: %s, date: %s, mime-type: %s\ndesc: %s\n",
              from.full().c_str(), name.c_str(), size, hash.c_str(), date.c_str(), mimetype.c_str(),
              desc.c_str() );
      f->acceptFT( from, id, SIProfileFT::FTTypeS5B );
    }

//     virtual void handleFTRequestResult( const JID& /*from*/, const std::string& /*sid*/ )
//     {
//     }

    virtual void handleFTRequestError( Stanza* /*stanza*/ )
    {
      printf( "ft request error\n" );
    }

    virtual void handleFTSOCKS5Bytestream( SOCKS5Bytestream* s5b )
    {
      printf( "received socks5 bytestream\n" );
      m_s5bs.push_back( s5b );
      s5b->registerSOCKS5BytestreamDataHandler( this );
      if( s5b->connect() )
      {
        printf( "ok! s5b connected to streamhost\n" );
      }
    }

    virtual void handleSOCKS5Data( SOCKS5Bytestream* /*s5b*/, const std::string& data )
    {
      printf( "received %d bytes of data:\n%s\n", data.length(), data.c_str() );
    }

    virtual void handleSOCKS5Error( SOCKS5Bytestream* /*s5b*/, Stanza* /*stanza*/ )
    {
      printf( "socks5 stream error\n" );
    }

    virtual void handleSOCKS5Open( SOCKS5Bytestream* /*s5b*/ )
    {
      printf( "stream opened\n" );
    }

    virtual void handleSOCKS5Close( SOCKS5Bytestream* /*s5b*/ )
    {
      printf( "stream closed\n" );
      m_quit = true;
    }

  private:
    Client *j;
    SIProfileFT* f;
    SOCKS5BytestreamManager* s5b;
    std::list<SOCKS5Bytestream*> m_s5bs;
    bool m_quit;
};

int main( int /*argc*/, char** /*argv*/ )
{
  FTTest *r = new FTTest();
  r->start();
  delete( r );
  return 0;
}
