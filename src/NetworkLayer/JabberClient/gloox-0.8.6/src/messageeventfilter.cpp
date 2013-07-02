/*
  Copyright (c) 2005-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "messageeventfilter.h"
#include "messageeventhandler.h"
#include "messagesession.h"
#include "stanza.h"

namespace gloox
{

  MessageEventFilter::MessageEventFilter( MessageSession *parent, int defaultEvents )
    : MessageFilter( parent ), m_messageEventHandler( 0 ), m_requestedEvents( 0 ),
      m_defaultEvents( defaultEvents ), m_lastSent( MessageEventCancel )
  {
  }

  MessageEventFilter::~MessageEventFilter()
  {
  }

  void MessageEventFilter::filter( Stanza *stanza )
  {
    if( ( m_messageEventHandler ) && stanza->hasChild( "x", "xmlns", XMLNS_X_EVENT ) )
    {
      if( stanza->body().empty() )
      {
        Tag *x = stanza->findChild( "x" );
        if( x->hasChild( "offline" ) )
          m_messageEventHandler->handleMessageEvent( stanza->from(), MessageEventOffline );
        else if( x->hasChild( "delivered" ) )
          m_messageEventHandler->handleMessageEvent( stanza->from(), MessageEventDelivered );
        else if( x->hasChild( "displayed" ) )
          m_messageEventHandler->handleMessageEvent( stanza->from(), MessageEventDisplayed );
        else if( x->hasChild( "composing" ) )
          m_messageEventHandler->handleMessageEvent( stanza->from(), MessageEventComposing );
        else
          m_messageEventHandler->handleMessageEvent( stanza->from(), MessageEventCancel );
      }
      else
      {
        m_requestedEvents = 0;
        Tag *x = stanza->findChild( "x" );
        if( x->hasChild( "offline" ) )
          m_requestedEvents |= MessageEventOffline;
        if( x->hasChild( "delivered" ) )
          m_requestedEvents |= MessageEventDelivered;
        if( x->hasChild( "displayed" ) )
          m_requestedEvents |= MessageEventDisplayed;
        if( x->hasChild( "composing" ) )
          m_requestedEvents |= MessageEventComposing;
      }
    }
    else if( stanza->body().empty() )
    {
      m_requestedEvents = 0;
      m_lastID = "";
    }
  }

  void MessageEventFilter::raiseMessageEvent( MessageEventType event )
  {
    if( ( m_requestedEvents & event ) || ( event == MessageEventCancel ) )
    {
      Tag *m = new Tag( "message" );
      m->addAttribute( "to", m_parent->target().full() );
      Tag *x = new Tag( m, "x" );
      x->addAttribute( "xmlns", XMLNS_X_EVENT );
      new Tag( x, "id", m_lastID );

      bool used = false;
      switch( event )
      {
        case MessageEventOffline:
          new Tag( x, "offline" );
          m_requestedEvents ^= event;
          used = true;
          break;
        case MessageEventDelivered:
          new Tag( x, "delivered" );
          m_requestedEvents ^= event;
          used = true;
          break;
        case MessageEventDisplayed:
          new Tag( x, "displayed" );
          m_requestedEvents ^= event;
          used = true;
          break;
        case MessageEventComposing:
          if( m_lastSent != MessageEventComposing )
          {
            new Tag( x, "composing" );
            used = true;
          }
          break;
        case MessageEventCancel:
          break;
      }

      m_lastSent = event;

      if( used )
        m_parent->send( m );
      else
        delete m;
    }
  }

  void MessageEventFilter::decorate( Tag *tag )
  {
    if( m_defaultEvents != 0 )
    {
      Tag *x = new Tag( tag, "x" );
      x->addAttribute( "xmlns", XMLNS_X_EVENT );

      if( m_defaultEvents & MessageEventOffline )
        new Tag( x, "offline" );
      if( m_defaultEvents & MessageEventDelivered )
        new Tag( x, "delivered" );
      if( m_defaultEvents & MessageEventDisplayed )
        new Tag( x, "displayed" );
      if( m_defaultEvents & MessageEventComposing )
        new Tag( x, "composing" );
    }
    m_lastSent = MessageEventCancel;
  }

  void MessageEventFilter::registerMessageEventHandler( MessageEventHandler *meh )
  {
    m_messageEventHandler = meh;
  }

  void MessageEventFilter::removeMessageEventHandler()
  {
    m_messageEventHandler = 0;
  }

}
