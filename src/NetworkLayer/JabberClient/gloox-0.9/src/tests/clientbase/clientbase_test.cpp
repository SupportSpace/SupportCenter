#include "../../clientbase.h"
#include "../../connectionbase.h"
// #include "../../logsink.h"
// #include "../../loghandler.h"
#include "../../connectionlistener.h"
#include "../../gloox.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

class ClientBaseTest : public ClientBase, /*LogHandler,*/ ConnectionListener
{
  public:
    ClientBaseTest( const std::string& ns, const std::string& server, int port = -1 )
      : ClientBase( ns, server, port ), m_handleStartNodeCalled( false ),
        m_versionOK( false )
    {
      m_jid.setUsername( "test" );
      m_jid.setServer( server );
      m_jid.setResource( "gloox" );
//       logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );
      registerConnectionListener( this );
    }
    virtual void handleStartNode() { m_handleStartNodeCalled = true; }
    virtual bool handleNormalNode(gloox::Stanza*) { return true; }
    virtual void rosterFilled() {}
/*    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }*/
    virtual void onConnect() { /*printf( "connect\n" );*/ }
    virtual void onDisconnect( ConnectionError /*e*/ ) { /*printf( "disconnected: %d\n", e );*/ }
    virtual void onResourceBindError( ResourceBindError /*error*/ ) { /*printf( "res bind err: %d\n", error );*/ }
    virtual void onSessionCreateError( SessionCreateError /*error*/ ) { /*printf( "ses err: %d\n", error );*/ }
    virtual bool onTLSConnect( const CertInfo& /*info*/ ) { return false; }
    bool handleStartNodeCalled() const { return m_handleStartNodeCalled; }
    bool sidOK() const { return ( m_sid == "testsid" ); }
    bool versionOK() const { return m_versionOK; }

  protected:
      virtual bool checkStreamVersion( const std::string& version )
      {
        m_versionOK = ClientBase::checkStreamVersion( version );
        return m_versionOK;
      }

  private:
    bool m_handleStartNodeCalled;
    bool m_versionOK;
};

class ConnectionImpl : public ConnectionBase
{
  public:
    ConnectionImpl( ConnectionDataHandler *cdh )
      : ConnectionBase( cdh ), m_pos( 0 ) {}
    virtual ~ConnectionImpl() {}
    virtual ConnectionError connect() { m_state = StateConnected; return ConnNoError; }
    virtual ConnectionError recv( int timeout = -1 ) { return ConnNoError; }
    virtual bool send( const std::string& data ) { return true; }
    virtual ConnectionError receive()
    {
      ConnectionError ce = ConnNoError;
      while( ce == ConnNoError && m_pos <= 8 )
        ce = recv( 0 );
      return ConnNotConnected;
    }
    virtual void disconnect() {}

  private:
    int m_pos;

};

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  ClientBaseTest *c = 0;
  Tag *t = 0;


  // -------
  name = "disconnected: recv()";
  c = new ClientBaseTest( "a", "b", 1 );
  if( c->recv() != ConnNotConnected )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  // -------
  name = "disconnected: handleTag(): start node: handleStartNode";
  c = new ClientBaseTest( "a", "b", 1 );
  t = new Tag( "stream:stream" );
  t->addAttribute( "id", "testsid" );
  c->handleTag( t );
  if( !c->handleStartNodeCalled() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  delete t;
  c = 0;
  t = 0;

  // -------
  name = "disconnected: handleTag(): start node: version";
  c = new ClientBaseTest( "a", "b", 1 );
  t = new Tag( "stream:stream" );
  t->addAttribute( "version", "1.0" );
  c->handleTag( t );
  if( !c->versionOK() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  delete t;
  c = 0;
  t = 0;

  // -------
  name = "disconnected: handleTag(): start node: version (fail 1)";
  c = new ClientBaseTest( "a", "b", 1 );
  t = new Tag( "stream:stream" );
  t->addAttribute( "version", "3.0" );
  c->handleTag( t );
  if( c->versionOK() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  delete t;
  c = 0;
  t = 0;

  // -------
  name = "disconnected: handleTag(): start node: version (fail 2)";
  c = new ClientBaseTest( "a", "b", 1 );
  t = new Tag( "stream:stream" );
  c->handleTag( t );
  if( c->versionOK() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  delete t;
  c = 0;
  t = 0;

  // -------
  name = "disconnected: handleTag(): start node: session id";
  c = new ClientBaseTest( "a", "b", 1 );
  t = new Tag( "stream:stream" );
  t->addAttribute( "version", "1.0" );
  t->addAttribute( "id", "testsid" );
  c->handleTag( t );
  if( !c->sidOK() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  delete t;
  c = 0;
  t = 0;











  if( fail == 0 )
  {
    printf( "ClientBase: all tests passed\n" );
    return 0;
  }
  else
  {
    printf( "ClientBase: %d test(s) failed\n", fail );
    return 1;
  }

}
