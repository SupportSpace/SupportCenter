/*
  Copyright (c) 2005-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "privacymanager.h"
#include "clientbase.h"

#include <sstream>

namespace gloox
{

  PrivacyManager::PrivacyManager( ClientBase *parent )
    : m_parent( parent ), m_privacyListHandler( 0 )
  {
    if( m_parent )
      m_parent->registerIqHandler( this, XMLNS_PRIVACY );
  }

  PrivacyManager::~PrivacyManager()
  {
    if( m_parent )
      m_parent->removeIqHandler( XMLNS_PRIVACY );
  }

  std::string PrivacyManager::requestListNames()
  {
    std::string id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "get" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );

    m_parent->trackID( this, id, PL_REQUEST_NAMES );
    m_parent->send( iq );
    return id;
  }

  std::string PrivacyManager::requestList( const std::string& name )
  {
    std::string id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "get" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );
    Tag *l = new Tag( q, "list" );
    l->addAttribute( "name", name );

    m_parent->trackID( this, id, PL_REQUEST_LIST );
    m_parent->send( iq );
    return id;
  }

  std::string PrivacyManager::removeList( const std::string& name )
  {
    std::string id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );
    Tag *l = new Tag( q, "list" );
    l->addAttribute( "name", name );

    m_parent->trackID( this, id, PL_REMOVE );
    m_parent->send( iq );
    return id;
  }

  std::string PrivacyManager::setDefault( const std::string& name )
  {
    std::string id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );
    Tag *d = new Tag( q, "default" );
    d->addAttribute( "name", name );

    m_parent->trackID( this, id, PL_DEFAULT );
    m_parent->send( iq );
    return id;
  }

  std::string PrivacyManager::unsetDefault()
  {
    std::string id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );
    new Tag( q, "default" );

    m_parent->trackID( this, id, PL_UNSET_DEFAULT );
    m_parent->send( iq );
    return id;
  }

  std::string PrivacyManager::setActive( const std::string& name )
  {
    std::string id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );
    Tag *a = new Tag( q, "active" );
    a->addAttribute( "name", name );

    m_parent->trackID( this, id, PL_ACTIVATE );
    m_parent->send( iq );
    return id;
  }

  std::string PrivacyManager::unsetActive()
  {
    std::string id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );
    new Tag( q, "active" );

    m_parent->trackID( this, id, PL_UNSET_ACTIVATE );
    m_parent->send( iq );
    return id;
  }

  std::string PrivacyManager::store( const std::string& name, PrivacyListHandler::PrivacyList& list )
  {
    std::string id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_PRIVACY );
    Tag *l = new Tag( q, "list" );
    l->addAttribute( "name", name );

    int count = 0;
    PrivacyListHandler::PrivacyList::iterator it = list.begin();
    for( ; it != list.end(); ++it )
    {
      Tag *i = new Tag( l, "item" );

      switch( (*it).type() )
      {
        case PrivacyItem::TYPE_JID:
          i->addAttribute( "type", "jid" );
          break;
        case PrivacyItem::TYPE_GROUP:
          i->addAttribute( "type", "group" );
          break;
        case PrivacyItem::TYPE_SUBSCRIPTION:
          i->addAttribute( "type", "subscription" );
          break;
        default:
          break;
      }

      switch( (*it).action() )
      {
        case PrivacyItem::ACTION_ALLOW:
          i->addAttribute( "action", "allow" );
          break;
        case PrivacyItem::ACTION_DENY:
          i->addAttribute( "action", "deny" );
          break;
      }

      int pType = (*it).packetType();
      if( pType != 15 )
      {
        if( pType & PrivacyItem::PACKET_MESSAGE )
          new Tag( i, "message" );
        if( pType & PrivacyItem::PACKET_PRESENCE_IN )
          new Tag( i, "presence-in" );
        if( pType & PrivacyItem::PACKET_PRESENCE_OUT )
          new Tag( i, "presence-out" );
        if( pType & PrivacyItem::PACKET_IQ )
          new Tag( i, "iq" );
      }

      i->addAttribute( "value", (*it).value() );

      std::ostringstream oss;
      oss << ++count;
      i->addAttribute( "order", oss.str() );
    }

    m_parent->trackID( this, id, PL_STORE );
    m_parent->send( iq );
    return id;
  }

  bool PrivacyManager::handleIq( Stanza *stanza )
  {
    if( stanza->subtype() != StanzaIqSet || !m_privacyListHandler )
      return false;

    Tag *l = stanza->findChild( "query" )->findChild( "list" );
    if( l->hasAttribute( "name" ) )
    {
      std::string name = l->findAttribute( "name" );
      m_privacyListHandler->handlePrivacyListChanged( name );

      Tag *iq = new Tag( "iq" );
      iq->addAttribute( "type", "result" );
      iq->addAttribute( "id", stanza->id() );
      m_parent->send( iq );
      return true;
    }

    return false;
  }

  bool PrivacyManager::handleIqID( Stanza *stanza, int context )
  {
    if( stanza->subtype() != StanzaIqResult || !m_privacyListHandler )
      return false;

    switch( stanza->subtype() )
    {
      case StanzaIqResult:
        switch( context )
        {
          case PL_STORE:
            m_privacyListHandler->handlePrivacyListResult( stanza->id(),
                PrivacyListHandler::RESULT_STORE_SUCCESS );
            break;
          case PL_ACTIVATE:
            m_privacyListHandler->handlePrivacyListResult( stanza->id(),
                PrivacyListHandler::RESULT_ACTIVATE_SUCCESS );
            break;
          case PL_DEFAULT:
            m_privacyListHandler->handlePrivacyListResult( stanza->id(),
                PrivacyListHandler::RESULT_DEFAULT_SUCCESS );
            break;
          case PL_REMOVE:
            m_privacyListHandler->handlePrivacyListResult( stanza->id(),
                PrivacyListHandler::RESULT_REMOVE_SUCCESS );
            break;
          case PL_REQUEST_NAMES:
          {
            StringList lists;
            std::string def;
            std::string active;
            Tag *q = stanza->findChild( "query" );
            Tag::TagList l = q->children();
            Tag::TagList::const_iterator it = l.begin();
            for( ; it != l.end(); ++it )
            {
              if( (*it)->name() == "default" )
                def = (*it)->findAttribute( "name" );
              if( (*it)->name() == "active" )
                def = (*it)->findAttribute( "name" );
              if( (*it)->name() == "list" )
              {
                const std::string name = (*it)->findAttribute( "name" );
                lists.push_back( name );
              }
            }

            m_privacyListHandler->handlePrivacyListNames( def, active, lists );
            break;
          }
          case PL_REQUEST_LIST:
          {
            PrivacyListHandler::PrivacyList items;

            Tag *list = stanza->findChild( "query" )->findChild( "list" );
            const std::string name = list->findAttribute( "name" );
            Tag::TagList l = list->children();
            Tag::TagList::iterator it = l.begin();
            for( ; it != l.end(); ++it )
            {
              PrivacyItem::ItemType type;
              PrivacyItem::ItemAction action;
              int packetType = 0;

              const std::string t = (*it)->findAttribute( "type" );
              if( t == "jid" )
                type = PrivacyItem::TYPE_JID;
              else if( t == "group" )
                type = PrivacyItem::TYPE_GROUP;
              else if( t == "subscription" )
                type = PrivacyItem::TYPE_SUBSCRIPTION;
              else
                type = PrivacyItem::TYPE_UNDEFINED;

              const std::string a = (*it)->findAttribute( "action" );
              if( a == "allow" )
                action = PrivacyItem::ACTION_ALLOW;
              else if( a == "deny" )
                action = PrivacyItem::ACTION_DENY;
              else
                action = PrivacyItem::ACTION_ALLOW;

              std::string value = (*it)->findAttribute( "value" );

              Tag::TagList c = (*it)->children();
              Tag::TagList::const_iterator it_c = c.begin();
              for( ; it_c != c.end(); ++it_c )
              {
                if( (*it_c)->name() == "iq" )
                  packetType |= PrivacyItem::PACKET_IQ;
                else if( (*it_c)->name() == "presence-out" )
                  packetType |= PrivacyItem::PACKET_PRESENCE_OUT;
                else if( (*it_c)->name() == "presence-in" )
                  packetType |= PrivacyItem::PACKET_PRESENCE_IN;
                else if( (*it_c)->name() == "message" )
                  packetType |= PrivacyItem::PACKET_MESSAGE;
              }

              PrivacyItem item( type, action, packetType, value );
              items.push_back( item );
            }
            m_privacyListHandler->handlePrivacyList( name, items );
            break;
          }
        }
        break;

      case StanzaIqError:
      {
        Tag *e = stanza->findChild( "error" );
        if( e->hasChild( "conflict" ) )
          m_privacyListHandler->handlePrivacyListResult( stanza->id(),
            PrivacyListHandler::RESULT_CONFLICT );
        else if( e->hasChild( "item-not-found" ) )
          m_privacyListHandler->handlePrivacyListResult( stanza->id(),
            PrivacyListHandler::RESULT_ITEM_NOT_FOUND );
        else if( e->hasChild( "bad-request" ) )
          m_privacyListHandler->handlePrivacyListResult( stanza->id(),
            PrivacyListHandler::RESULT_BAD_REQUEST );
        break;
      }

      default:
        break;
    }
    return false;
  }

  void PrivacyManager::registerPrivacyListHandler( PrivacyListHandler *plh )
  {
    m_privacyListHandler = plh;
  }

  void PrivacyManager::removePrivacyListHandler()
  {
    m_privacyListHandler = 0;
  }

}
