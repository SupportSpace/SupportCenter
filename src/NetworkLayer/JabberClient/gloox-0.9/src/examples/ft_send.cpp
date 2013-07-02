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
#include "../socks5bytestreamserver.h"
using namespace gloox;

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <ios>

#if defined( WIN32 ) || defined( _WIN32 )
# include <windows.h>
#endif

/**
 * Usage:
 *   ft_send jid@server/full /path/to/file
 *
 * Sends the given file to the given full JID.
 */
class FTTest : public LogHandler, ConnectionListener, SIProfileFTHandler, SOCKS5BytestreamDataHandler
{
  public:
    FTTest( const JID& to, const std::string& file ) : m_s5b( 0 ), m_to( to ), m_file( file ), m_quit( false ) {}

    virtual ~FTTest() {}

    void start()
    {

      struct stat f_stat;
      if( stat( m_file.c_str(), &f_stat ) )
        return;

      m_size = f_stat.st_size;
      std::ifstream ifile( m_file.c_str(), std::ios_base::in | std::ios_base::binary );
      if( !ifile )
        return;

      JID jid( "hurkhurk@example.net/glooxsendfile" );
      j = new Client( jid, "hurkhurks" );
      j->registerConnectionListener( this );
      j->disco()->setVersion( "ftsend", GLOOX_VERSION, "Linux" );
      j->disco()->setIdentity( "client", "bot" );
      StringList ca;
      ca.push_back( "/path/to/cacert.crt" );
      j->setCACerts( ca );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );


      m_server = new SOCKS5BytestreamServer( j->logInstance(), 6666 );
      printf( "about to listen\n" );
      ConnectionError le = ConnNoError;
      if( ( le = m_server->listen() ) != ConnNoError )
        printf( "listen returned: %d\n", le );
      printf( "listening\n" );

      f = new SIProfileFT( j, this );
      f->registerSOCKS5BytestreamServer( m_server );
      f->addStreamHost( j->jid(), "192.168.100.20", 6666 );
      // you should obtain this using disco, really:
//       f->addStreamHost( JID( "reflector.amessage.eu" ), "reflector.amessage.eu", 6565 );
//       f->addStreamHost( JID( "proxy.jabber.org" ), "208.245.212.98", 7777 );

      if( j->connect( false ) )
      {
        char input[200024];
        ConnectionError ce = ConnNoError;
        ConnectionError se = ConnNoError;
        while( ce == ConnNoError )
        {
          if( m_quit )
            j->disconnect();

          ce = j->recv( 1 );
          if( m_server )
          {
            se = m_server->recv( 1 );
            if( se != ConnNoError )
            {
              printf( "SOCKS5BytestreamServer returned: %d\n", se );
              delete m_server;
              m_server = 0;
              m_quit = true;
            }
          }
          if( m_s5b && !ifile.eof() )
          {
            if( m_s5b->isOpen() )
            {
              ifile.read( input, 200024 );
              std::string t( input, ifile.gcount() );
              if( !m_s5b->send( t ) )
                m_quit = true;
            }
            m_s5b->recv( 1 );
          }
          else if( m_s5b )
            m_s5b->close();
        }
        printf( "ce: %d\n", ce );
      }

      f->dispose( m_s5b );
      delete f;
      delete m_server;
      delete j;
    }

    virtual void onConnect()
    {
      printf( "connected!!!\n" );
      f->requestFT( m_to, m_file, m_size );
    }

    virtual void onDisconnect( ConnectionError e )
    {
      printf( "ft_send: disconnected: %d\n", e );
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
      m_quit = true;
    }

    virtual void handleFTSOCKS5Bytestream( SOCKS5Bytestream* s5b )
    {
      printf( "received socks5 bytestream\n" );
      m_s5b = s5b;
      m_s5b->registerSOCKS5BytestreamDataHandler( this );
      if( m_s5b->connect() )
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
    SOCKS5Bytestream* m_s5b;
    SOCKS5BytestreamServer* m_server;
    JID m_to;
    std::string m_file;
    bool m_quit;
    int m_size;
};

int main( int argc, char** argv )
{
  if( argc == 3 )
  {
    FTTest *r = new FTTest( JID( argv[1] ), argv[2] );
    r->start();
    delete( r );
  }
  else
  {
    printf( "error: need jid + file\n" );
  }
  return 0;
}
