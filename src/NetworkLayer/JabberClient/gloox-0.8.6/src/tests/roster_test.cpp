#include "../client.h"
#include "../connectionlistener.h"
#include "../discohandler.h"
#include "../disco.h"
#include "../rostermanager.h"
#include "../loghandler.h"
#include "../logsink.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

class RosterTest : public RosterListener, ConnectionListener, LogHandler
{
  public:
    RosterTest() {};
    virtual ~RosterTest() {};

    void start()
    {
      setlocale( LC_ALL, "" );

      JID jid( "hurkhurk@example.org/gloox" );
      j = new Client( jid, "hurkhurks" );
      j->setAutoPresence( true );
      j->setInitialPriority( 3 );
      j->registerConnectionListener( this );
      j->rosterManager()->registerRosterListener( this );
      j->disco()->setVersion( "rosterTest", GLOOX_VERSION );
      j->disco()->setIdentity( "client", "bot" );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

      j->connect();

      delete( j );
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

    virtual void itemSubscribed( const std::string& jid )
    {
      printf( "subscribed %s\n", jid.c_str() );
    }

    virtual void itemAdded( const std::string& jid )
    {
      printf( "added %s\n", jid.c_str() );
    }

    virtual void itemUnsubscribed( const std::string& jid )
    {
      printf( "unsubscribed %s\n", jid.c_str() );
    }

    virtual void itemRemoved( const std::string& jid )
    {
      printf( "removed %s\n", jid.c_str() );
    }

    virtual void itemUpdated( const std::string& jid )
    {
      printf( "updated %s\n", jid.c_str() );
    }

    virtual void roster( const Roster& roster )
    {
      printf( "roster arriving\nitems:\n" );
      RosterListener::Roster::const_iterator it = roster.begin();
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

    virtual void presenceUpdated( const RosterItem& item, int /*status*/, const std::string& /*msg*/ )
    {
      printf( "item changed: %s\n", item.jid().c_str() );
    }

    virtual void itemAvailable( const RosterItem& item, const std::string& /*msg*/ )
    {
      printf( "item online: %s\n", item.jid().c_str() );
    }

    virtual void itemUnavailable( const RosterItem& item, const std::string& /*msg*/ )
    {
      printf( "item offline: %s\n", item.jid().c_str() );
    };

    virtual bool subscriptionRequest( const std::string& jid, const std::string& /*msg*/ )
    {
      printf( "subscription: %s\n", jid.c_str() );
      StringList groups;
      j->rosterManager()->subscribe( jid, "", groups, "" );
      return true;
    }

    virtual bool unsubscriptionRequest( const std::string& jid, const std::string& /*msg*/ )
    {
      printf( "unsubscription: %s\n", jid.c_str() );
      return true;
    }

    virtual void nonrosterPresenceReceived( const JID& jid )
    {
      printf( "received presence from entity not in the roster: %s\n", jid.full().c_str() );
    }

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    };

  private:
    Client *j;
};

int main( int /*argc*/, char* /*argv[]*/ )
{
  RosterTest *r = new RosterTest();
  r->start();
  delete( r );
  return 0;
}
