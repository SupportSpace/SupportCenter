/*
  Copyright (c) 2005-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "flexoff.h"
#include "dataform.h"

namespace gloox
{

  FlexibleOffline::FlexibleOffline( ClientBase *parent, Disco *disco )
    : m_parent( parent ), m_disco( disco ), m_flexibleOfflineHandler( 0 )
  {
  }

  FlexibleOffline::~FlexibleOffline()
  {
  }

  void FlexibleOffline::checkSupport()
  {
    m_disco->getDiscoInfo( m_parent->jid().server(), "", this, FO_CHECK_SUPPORT );
  }

  void FlexibleOffline::getMsgCount()
  {
    m_disco->getDiscoInfo( m_parent->jid().server(), XMLNS_OFFLINE, this, FO_REQUEST_NUM );
  }

  void FlexibleOffline::fetchHeaders()
  {
    m_disco->getDiscoItems( m_parent->jid().server(), XMLNS_OFFLINE, this, FO_REQUEST_HEADERS );
  }

  void FlexibleOffline::fetchMessages( const StringList& msgs )
  {
    const std::string id = m_parent->getID();
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "get" );
    iq->addAttribute( "id", id );
    Tag *o = new Tag( iq, "offline" );
    o->addAttribute( "xmlns", XMLNS_OFFLINE );

    if( msgs.size() == 0 )
      new Tag( o, "fetch" );
    else
    {
      StringList::const_iterator it = msgs.begin();
      for( ; it != msgs.end(); ++it )
      {
        Tag *i = new Tag( o, "item" );
        i->addAttribute( "action", "view" );
        i->addAttribute( "node", (*it) );
      }
    }

    m_parent->trackID( this, id, FO_REQUEST_MSGS );
    m_parent->send( iq );
  }

  void FlexibleOffline::removeMessages( const StringList& msgs )
  {
    const std::string id = m_parent->getID();
    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "get" );
    iq->addAttribute( "id", id );
    Tag *o = new Tag( iq, "offline" );
    o->addAttribute( "xmlns", XMLNS_OFFLINE );

    if( msgs.size() == 0 )
      new Tag( o, "purge" );
    else
    {
      StringList::const_iterator it = msgs.begin();
      for( ; it != msgs.end(); ++it )
      {
        Tag *i = new Tag( o, "item" );
        i->addAttribute( "action", "remove" );
        i->addAttribute( "node", (*it) );
      }
    }

    m_parent->trackID( this, id, FO_REMOVE_MSGS );
    m_parent->send( iq );
  }

  void FlexibleOffline::registerFlexibleOfflineHandler( FlexibleOfflineHandler *foh )
  {
    m_flexibleOfflineHandler = foh;
  }

  void FlexibleOffline::removeFlexibleOfflineHandler()
  {
    m_flexibleOfflineHandler = 0;
  }

  void FlexibleOffline::handleDiscoInfoResult( Stanza *stanza, int context )
  {
    if( !m_flexibleOfflineHandler )
      return;

    switch( context )
    {
      case FO_CHECK_SUPPORT:
        m_flexibleOfflineHandler->handleFlexibleOfflineSupport(
            stanza->findChild( "query" )->hasChild( "feature", "var", XMLNS_OFFLINE ) );
        break;

      case FO_REQUEST_NUM:
        int num = -1;
        DataForm f( stanza->findChild( "query" )->findChild( "x" ) );
        if( f.hasField( "number_of_messages" ) )
          num = atoi( f.field( "number_of_messages" ).value().c_str() );

        m_flexibleOfflineHandler->handleFlexibleOfflineMsgNum( num );
        break;
    }
  }

  void FlexibleOffline::handleDiscoItemsResult( Stanza *stanza, int context )
  {
    if( context == FO_REQUEST_HEADERS && m_flexibleOfflineHandler )
    {
      Tag *q = stanza->findChild( "query" );
      if( q && q->hasAttribute( "xmlns", XMLNS_DISCO_ITEMS ) && q->hasAttribute( "node", XMLNS_OFFLINE ) )
      {
        StringMap m;
        Tag::TagList l = q->children();
        Tag::TagList::const_iterator it = l.begin();
        for( ; it != l.end(); ++it )
        {
          m[(*it)->findAttribute( "node" )] = (*it)->findAttribute( "name" );
        }
        m_flexibleOfflineHandler->handleFlexibleOfflineMessageHeaders( m );
      }
    }
  }

  void FlexibleOffline::handleDiscoError( Stanza * /*stanza*/, int /*context*/ )
  {
  }

  bool FlexibleOffline::handleIqID( Stanza *stanza, int context )
  {
    if( !m_flexibleOfflineHandler )
      return false;

    switch( context )
    {
      case FO_REQUEST_MSGS:
        switch( stanza->subtype() )
        {
          case StanzaIqResult:
            m_flexibleOfflineHandler->handleFlexibleOfflineResult(
                FlexibleOfflineHandler::FOMR_REQUEST_SUCCESS );
            break;
          case StanzaIqError:
            switch( stanza->error() )
            {
              case StanzaErrorForbidden:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult(
                    FlexibleOfflineHandler::FOMR_FORBIDDEN );
                break;
              case StanzaErrorItemNotFound:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult(
                    FlexibleOfflineHandler::FOMR_ITEM_NOT_FOUND );
                break;
              default:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult(
                    FlexibleOfflineHandler::FOMR_UNKNOWN_ERROR );
                break;
            }
            break;
          default:
            break;
        }
        break;
      case FO_REMOVE_MSGS:
        switch( stanza->subtype() )
        {
          case StanzaIqResult:
            m_flexibleOfflineHandler->handleFlexibleOfflineResult(
                FlexibleOfflineHandler::FOMR_REMOVE_SUCCESS );
            break;
          case StanzaIqError:
            switch( stanza->error() )
            {
              case StanzaErrorForbidden:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult(
                    FlexibleOfflineHandler::FOMR_FORBIDDEN );
                break;
              case StanzaErrorItemNotFound:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult(
                    FlexibleOfflineHandler::FOMR_ITEM_NOT_FOUND );
                break;
              default:
                m_flexibleOfflineHandler->handleFlexibleOfflineResult(
                    FlexibleOfflineHandler::FOMR_UNKNOWN_ERROR );
                break;
            }
            break;
          default:
            break;
        }
        break;
    }

    return false;
  }

  bool FlexibleOffline::handleIq( Stanza * /*stanza*/ )
  {
    return false;
  }

}
