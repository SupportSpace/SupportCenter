#include "../../stanza.h"
#include "../../tag.h"
#include "../../prep.h"
#include "../../gloox.h"
#include "../../chatstatehandler.h"

#include <stdio.h>
#include <string>

namespace gloox
{
  class MessageSession : public ChatStateHandler
  {
    public:
      MessageSession() : m_jid( "abc@example.net/foo" ), m_test( 0 ), m_result( false ) {}
      virtual ~MessageSession() {}
      const JID& target() const { return m_jid; }
      void send( Tag* tag )
      {
        if( !tag )
          return;
        if( tag->name() != "message" || !tag->hasAttribute( "to", m_jid.full() ) )
        {
          delete tag;
          return;
        }
        switch( m_test )
        {
          case 0:
            if( tag->hasChild( "gone", "xmlns", XMLNS_CHAT_STATES ) )
              m_result = true;
            break;
          case 1:
            if( tag->hasChild( "inactive", "xmlns", XMLNS_CHAT_STATES ) )
              m_result = true;
            break;
          case 2:
            if( tag->hasChild( "active", "xmlns", XMLNS_CHAT_STATES ) )
              m_result = true;
            break;
          case 3:
            if( tag->hasChild( "composing", "xmlns", XMLNS_CHAT_STATES ) )
              m_result = true;
            break;
          case 4:
            if( tag->hasChild( "paused", "xmlns", XMLNS_CHAT_STATES ) )
              m_result = true;
            break;
          default:
            break;
        }
        delete tag;
      }
      void setTest( int test ) { m_test = test; }
      bool ok() { bool ok = m_result; m_result = false; return ok; }
      virtual void handleChatState( const JID& from, ChatStateType state )
      {
        switch( m_test )
        {
          case 0:
            if( state == ChatStateGone )
              m_result = true;
            break;
          case 1:
            if( state == ChatStateInactive )
              m_result = true;
            break;
          case 2:
            if( state == ChatStateActive )
              m_result = true;
            break;
          case 3:
            if( state == ChatStateComposing )
              m_result = true;
            break;
          case 4:
            if( state == ChatStatePaused )
              m_result = true;
            break;
          default:
            break;
        }
      }
    private:
      JID m_jid;
      int m_test;
      bool m_result;
  };

  class MessageFilter
  {
    public:
      MessageFilter( MessageSession *parent );
      virtual ~MessageFilter();
      void attachTo( MessageSession *session );
      virtual void decorate( Tag *tag );
    protected:
      MessageSession *m_parent;
  };

  MessageFilter::MessageFilter( MessageSession *parent ) : m_parent( parent ) {}
  MessageFilter::~MessageFilter() { delete m_parent; }
  void MessageFilter::attachTo( MessageSession *session ) {}
  void MessageFilter::decorate( Tag *tag ) {}
}

#define MESSAGEFILTER_H__
#define MESSAGESESSION_H__
#include "../../chatstatefilter.h"
#include "../../chatstatefilter.cpp"

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  gloox::ChatStateFilter *f;
  gloox::MessageSession *ms;
  gloox::Tag *t = 0;
  gloox::Tag *x = 0;
  gloox::Tag *m = 0;
  gloox::Stanza *s = 0;

  // -------
  name = "simple decorate";
  f = new gloox::ChatStateFilter( new gloox::MessageSession() );
  t = new gloox::Tag( "dummy" );
  f->decorate( t );
  if( !t->hasChild( "active", "xmlns", gloox::XMLNS_CHAT_STATES ) )
  {
    ++fail;
    printf( "test '%s' failed:s %s\n", name.c_str(), t->xml().c_str() );
  }
  delete f;
  delete t;
  f = 0;
  t = 0;

  // -------
  ms = new gloox::MessageSession();
  f = new gloox::ChatStateFilter( ms );
  f->registerChatStateHandler( ms );

  name = "filter gone";
  m = new gloox::Stanza( "message" );
  m->addAttribute( "type", "chat" );
  t = new gloox::Tag( m, "gone" ); t->addAttribute( "xmlns", gloox::XMLNS_CHAT_STATES );
  s = new gloox::Stanza( m );
  delete m;
  ms->setTest( 0 );
  f->filter( s );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete s;
  s = 0;

  // -------
  name = "filter inactive";
  m = new gloox::Stanza( "message" );
  m->addAttribute( "type", "chat" );
  t = new gloox::Tag( m, "inactive" ); t->addAttribute( "xmlns", gloox::XMLNS_CHAT_STATES );
  s = new gloox::Stanza( m );
  delete m;
  ms->setTest( 1 );
  f->filter( s );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete s;
  s = 0;

  // -------
  name = "filter active";
  m = new gloox::Stanza( "message" );
  m->addAttribute( "type", "chat" );
  t = new gloox::Tag( m, "active" ); t->addAttribute( "xmlns", gloox::XMLNS_CHAT_STATES );
  s = new gloox::Stanza( m );
  delete m;
  ms->setTest( 2 );
  f->filter( s );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete s;
  s = 0;

  // -------
  name = "filter composing";
  m = new gloox::Stanza( "message" );
  m->addAttribute( "type", "chat" );
  t = new gloox::Tag( m, "composing" ); t->addAttribute( "xmlns", gloox::XMLNS_CHAT_STATES );
  s = new gloox::Stanza( m );
  delete m;
  ms->setTest( 3 );
  f->filter( s );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete s;
  s = 0;

  // -------
  name = "filter paused";
  m = new gloox::Stanza( "message" );
  m->addAttribute( "type", "chat" );
  t = new gloox::Tag( m, "paused" ); t->addAttribute( "xmlns", gloox::XMLNS_CHAT_STATES );
  s = new gloox::Stanza( m );
  delete m;
  ms->setTest( 4 );
  f->filter( s );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete s;
  s = 0;


  // -------
  name = "set gone state";
  ms->setTest( 0 );
  f->setChatState( gloox::ChatStateGone );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "set inactive state";
  ms->setTest( 1 );
  f->setChatState( gloox::ChatStateInactive );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "set active state";
  ms->setTest( 2 );
  f->setChatState( gloox::ChatStateActive );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "set composing state";
  ms->setTest( 3 );
  f->setChatState( gloox::ChatStateComposing );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "set paused state";
  ms->setTest( 4 );
  f->setChatState( gloox::ChatStatePaused );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  delete f;
//   delete s;
  f = 0;
//   s = 0;











  if( fail == 0 )
  {
    printf( "ChatStateFilter: all tests passed\n" );
    return 0;
  }
  else
  {
    printf( "ChatStateFilter: %d test(s) failed\n", fail );
    return 1;
  }

}
