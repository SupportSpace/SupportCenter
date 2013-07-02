#include "../../gloox.h"
#include "../../jid.h"
#include "../../tag.h"
#include "../../iqhandler.h"
#include "../../sihandler.h"
#include "../../siprofilehandler.h"

#include <stdio.h>
#include <locale.h>
#include <string>

const std::string& g_dir = "test.dir";
const std::string& g_inst = "the instructions";
const std::string& g_profile = "test-prof";

gloox::Tag* t1 = 0;
gloox::Tag* t2 = 0;
const gloox::JID to( "abc@def.gh/ijk" );

namespace gloox
{

  class Disco
  {
    public:
      Disco();
      ~Disco();
      void addFeature( const std::string& feature );
      void removeFeature( const std::string& feature );
  };
  Disco::Disco() {}
  Disco::~Disco() {}
  void Disco::addFeature( const std::string& /*feature*/ ) {}
  void Disco::removeFeature( const std::string& /*feature*/ ) {}

  class ClientBase : public SIHandler, public SIProfileHandler
  {
    public:
      ClientBase();
      ~ClientBase();
      const std::string getID();
      Disco* disco();
      void send( Tag *tag );
      void trackID( IqHandler *ih, const std::string& id, int context );
      void registerIqHandler( IqHandler *ih, const std::string& xmlns );
      void removeIqHandler( const std::string& xmlns );
      virtual void handleSIRequestResult( const JID& from, const std::string& sid,
                                          Tag* si, Tag* ptag, Tag* fneg );
      virtual void handleSIRequestError( Stanza* stanza );
      virtual void handleSIRequest( const JID& from, const std::string& id, const std::string& profile,
                                    Tag* si, Tag* ptag, Tag* fneg );
      void setTest( int test );
      bool ok();
    private:
      Disco* m_disco;
      int m_test;
      bool m_ok;
  };
  ClientBase::ClientBase() : m_disco( new Disco() ), m_test( 0 ), m_ok( false ) {}
  ClientBase::~ClientBase() { delete m_disco; }
  const std::string ClientBase::getID() { return "id"; }
  Disco* ClientBase::disco() { return m_disco; }
  void ClientBase::send( Tag *tag )
  {
    switch( m_test )
    {
      case 1:
      {
        Tag* si = tag->findChild( "si", "xmlns", XMLNS_SI );
        if( tag->findAttribute( "to" ) == to.full() && si && *(si->findChild( "file" )) == *t1
            && *(si->findChild( "feature" )) == *t2 && si->findAttribute( "mime-type" ) == "binary/octet-stream"
            && si->findAttribute( "profile" ) == g_profile )
          m_ok = true;
        break;
      }
    }
    delete tag;
  }
  void ClientBase::trackID( IqHandler* /*ih*/, const std::string& /*id*/, int /*context*/ ) {}
  void ClientBase::registerIqHandler( IqHandler* /*ih*/, const std::string& /*xmlns*/ ) {}
  void ClientBase::removeIqHandler( const std::string& /*xmlns*/ ) {}
  void ClientBase::handleSIRequestResult( const JID& /*from*/, const std::string& /*sid*/,
                                          Tag* /*si*/, Tag* /*ptag*/, Tag* /*fneg*/ ) {}
  void ClientBase::handleSIRequestError( Stanza* /*stanza*/ ) {}
  void ClientBase::handleSIRequest( const JID& /*from*/, const std::string& /*id*/,
                                    const std::string& /*profile*/,
                                    Tag* /*si*/, Tag* /*ptag*/, Tag* /*fneg*/ ) {}
  void ClientBase::setTest( int test ) { m_test = test; }
  bool ClientBase::ok() { bool t = m_ok; m_ok = false; return t; }
}

#define CLIENTBASE_H__
#define DISCO_H__
#include "../../simanager.h"
#include "../../simanager.cpp"
int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  t1 = new gloox::Tag( "file", "xmlns", gloox::XMLNS_SI_FT );
  t1->addAttribute( "name", "filename" );
  t1->addAttribute( "size", "1022" );

  t2 = new gloox::Tag( "feature", "xmlns", gloox::XMLNS_FEATURE_NEG );
  t2->addAttribute( "abc", "def" );
  t2->addAttribute( "ghi", "jkl" );

  gloox::SIManager* sim = 0;

  gloox::ClientBase* cb = new gloox::ClientBase();
  sim = new gloox::SIManager( cb, true );


  // -------
  name = "request si";
  cb->setTest( 1 );
  sim->requestSI( cb, to, g_profile, t1->clone(), t2->clone() );
  if( !cb->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }









  delete t1;
  delete t2;
  delete sim;
  delete cb;

  if( fail == 0 )
  {
    printf( "SIManager: all tests passed\n" );
    return 0;
  }
  else
  {
    printf( "SIManager: %d test(s) failed\n", fail );
    return 1;
  }

}
