#include "../../stanza.h"
#include "../../tag.h"
#include "../../prep.h"
#include "../../gloox.h"
#include "../../messageeventhandler.h"

#include <stdio.h>
#include <string>

namespace gloox
{
  class MessageSession : public MessageEventHandler
  {
    public:
      MessageSession() : m_jid( "abc@example.net/foo" ), m_test( 0 ), m_result( false ) {}
      virtual ~MessageSession() {}
      const JID& target() const { return m_jid; }
      void send( Tag* tag )
      {
        if( !tag )
          return;
        Tag *x = tag->findChild( "x", "xmlns", XMLNS_X_EVENT );
        if( !x || tag->name() != "message" || !tag->hasAttribute( "to", m_jid.full() )
             || !x->hasChild( "id" ) )
        {
          delete tag;
          return;
        }
        switch( m_test )
        {
          case 0:
            if( x->hasChild( "offline" ) )
              m_result = true;
            break;
          case 1:
            if( x->hasChild( "delivered" ) )
              m_result = true;
            break;
          case 2:
            if( x->hasChild( "displayed" ) )
              m_result = true;
            break;
          case 3:
            if( x->hasChild( "composing" ) )
              m_result = true;
            break;
          case 4:
            if( x->children().size() == 1 )
              m_result = true;
            break;
          default:
            break;
        }
        delete tag;
      }
      void setTest( int test ) { m_test = test; }
      bool ok() { bool ok = m_result; m_result = false; return ok; }
      virtual void handleMessageEvent( const JID& from, MessageEventType event )
      {
        printf( "recved event %d\n", event );
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
#include "../../messageeventfilter.h"
#include "../../messageeventfilter.cpp"

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  gloox::MessageEventFilter *f;
  gloox::MessageSession *ms;
  gloox::Tag *t = 0;
  gloox::Tag *x = 0;

  // -------
  name = "simple decorate";
  f = new gloox::MessageEventFilter( new gloox::MessageSession() );
  t = new gloox::Tag( "dummy" );
  f->decorate( t );
  x = t->findChild( "x", "xmlns", gloox::XMLNS_X_EVENT );
  if( !x || !x->hasChild( "offline" ) || !x->hasChild( "delivered" )
      || !x->hasChild( "displayed" ) || !x->hasChild( "composing" ) )
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
  f = new gloox::MessageEventFilter( ms );
  f->registerMessageEventHandler( ms );

  gloox::Tag *m = new gloox::Stanza( "message" );
  m->addAttribute( "type", "chat" );
  new gloox::Tag( m, "body", "my message" );
  x = new gloox::Tag( m, "x" );
  x->addAttribute( "xmlns", gloox::XMLNS_X_EVENT );
  new gloox::Tag( x, "offline" );
  new gloox::Tag( x, "delivered" );
  new gloox::Tag( x, "displayed" );
  new gloox::Tag( x, "composing" );
  gloox::Stanza *s = new gloox::Stanza( m );
  delete m;
  f->filter( s );

  name = "raise offline event 1";
  ms->setTest( 0 );
  f->raiseMessageEvent( gloox::MessageEventOffline );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  name = "raise offline event 2";
  ms->setTest( 0 );
  f->raiseMessageEvent( gloox::MessageEventOffline );
  if( ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  name = "raise delivered event 1";
  ms->setTest( 1 );
  f->raiseMessageEvent( gloox::MessageEventDelivered );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  name = "raise delivered event 2";
  ms->setTest( 1 );
  f->raiseMessageEvent( gloox::MessageEventDelivered );
  if( ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  name = "raise displayed event 1";
  ms->setTest( 2 );
  f->raiseMessageEvent( gloox::MessageEventDisplayed );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  name = "raise displayed event 2";
  ms->setTest( 2 );
  f->raiseMessageEvent( gloox::MessageEventDisplayed );
  if( ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  name = "raise composing event 1";
  ms->setTest( 3 );
  f->raiseMessageEvent( gloox::MessageEventComposing );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  name = "raise composing event 2";
  ms->setTest( 3 );
  f->raiseMessageEvent( gloox::MessageEventComposing );
  if( ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  name = "raise cancel event 1";
  ms->setTest( 4 );
  f->raiseMessageEvent( gloox::MessageEventCancel );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  name = "raise cancel event 2";
  ms->setTest( 4 );
  f->raiseMessageEvent( gloox::MessageEventCancel );
  if( !ms->ok() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  delete f;
  delete s;
  f = 0;
  s = 0;











  if( fail == 0 )
  {
    printf( "MessageEventFilter: all tests passed\n" );
    return 0;
  }
  else
  {
    printf( "MessageEventFilter: %d test(s) failed\n", fail );
    return 1;
  }

}
