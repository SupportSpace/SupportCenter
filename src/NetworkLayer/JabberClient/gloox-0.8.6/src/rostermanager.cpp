/*
  Copyright (c) 2004-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "clientbase.h"
#include "rostermanager.h"
#include "disco.h"
#include "rosteritem.h"
#include "rosterlistener.h"
#include "privatexml.h"


namespace gloox
{

  RosterManager::RosterManager( ClientBase *parent, bool self )
    : m_rosterListener( 0 ), m_parent( parent ), m_privateXML( 0 ),
      m_syncSubscribeReq( false )
  {
    if( m_parent )
    {
      m_parent->registerIqHandler( this, XMLNS_ROSTER );
      m_parent->registerPresenceHandler( this );
      m_parent->registerSubscriptionHandler( this );

      if( self )
      {
        RosterItem *i = new RosterItem( m_parent->jid().bare() );
        i->setSynchronized();
        m_roster[m_parent->jid().bare()] = i;
      }

      m_privateXML = new PrivateXML( m_parent );
    }
  }

  RosterManager::~RosterManager()
  {
    if( m_parent )
    {
      m_parent->removeIqHandler( XMLNS_ROSTER );
      m_parent->removePresenceHandler( this );
      m_parent->removeSubscriptionHandler( this );
    }

    RosterListener::Roster::iterator it = m_roster.begin();
    for( ; it != m_roster.end(); ++it )
      delete (*it).second;
    m_roster.clear();

    if( m_privateXML )
      delete m_privateXML;
  }

  RosterListener::Roster* RosterManager::roster()
  {
    return &m_roster;
  }

  void RosterManager::fill()
  {
    m_privateXML->requestXML( "roster", XMLNS_ROSTER_DELIMITER, this );

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "get" );
    iq->addAttribute( "id", m_parent->getID() );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_ROSTER );
    m_parent->send( iq );
  }

  bool RosterManager::handleIq( Stanza *stanza )
  {
    if( stanza->subtype() == StanzaIqResult ) // initial roster
    {
      extractItems( stanza, false );

      if( m_rosterListener )
        m_rosterListener->roster( m_roster );

      m_parent->rosterFilled();

      return true;
    }
    else if( stanza->subtype() == StanzaIqSet ) // roster item push
    {
      extractItems( stanza, true );

      Tag *iq = new Tag( "iq" );
      iq->addAttribute( "id", stanza->id() );
      iq->addAttribute( "type", "result" );
      m_parent->send( iq );

      return true;
    }

    return false;
  }

  bool RosterManager::handleIqID( Stanza * /*stanza*/, int /*context*/ )
  {
    return false;
  }

  void RosterManager::handlePresence( Stanza *stanza )
  {
    if( stanza->subtype() == StanzaPresenceError )
      return;

    RosterListener::Roster::iterator it = m_roster.find( stanza->from().bare() );
    if( it != m_roster.end() )
    {
      bool online = (*it).second->online();

      (*it).second->setStatus( stanza->from().resource(), stanza->show() );
      (*it).second->setStatusMsg( stanza->from().resource(), stanza->status() );
      (*it).second->setPriority( stanza->from().resource(), stanza->priority() );

      if( m_rosterListener && stanza->show() == PresenceAvailable )
      {
        if( !online )
          m_rosterListener->itemAvailable( (*(*it).second), stanza->status() );
        else
          m_rosterListener->presenceUpdated( (*(*it).second), stanza->show(), stanza->status() );
      }
      else if( stanza->show() == PresenceUnavailable )
      {
        (*it).second->removeResource( stanza->from().resource() );
        if( m_rosterListener )
          m_rosterListener->itemUnavailable( (*(*it).second), stanza->status() );
      }
      else
        if( m_rosterListener )
          m_rosterListener->presenceUpdated( (*(*it).second), stanza->show(), stanza->status() );
    }
    else
    {
      StringList sl;
      add( stanza->from().bare(), "", sl, "none", false );
      m_roster[stanza->from().bare()]->setStatus( stanza->from().resource(), stanza->show() );
      m_roster[stanza->from().bare()]->setStatusMsg( stanza->from().resource(), stanza->status() );
      m_roster[stanza->from().bare()]->setPriority( stanza->from().resource(), stanza->priority() );
      if( m_rosterListener )
        m_rosterListener->nonrosterPresenceReceived( stanza->from() );
    }
  }

  void RosterManager::subscribe( const std::string& jid, const std::string& name,
                                 StringList& groups, const std::string& msg )
  {
    if( jid.empty() )
      return;

    add( jid, name, groups );

    Tag *s = new Tag( "presence" );
    s->addAttribute( "type", "subscribe" );
    s->addAttribute( "to", jid );
    s->addAttribute( "from", m_parent->jid().full() );
    if( !msg.empty() )
      new Tag( s, "status", msg );

    m_parent->send( s );
  }


  void RosterManager::add( const std::string& jid, const std::string& name, StringList& groups )
  {
    if( jid.empty() )
      return;

    std::string id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "type", "set" );
    iq->addAttribute( "id", id );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_ROSTER );
    Tag *i = new Tag( q, "item" );
    i->addAttribute( "jid", jid );
    if( !name.empty() )
      i->addAttribute( "name", name );

    if( groups.size() != 0 )
    {
      StringList::const_iterator it = groups.begin();
      for( ; it != groups.end(); ++it )
        new Tag( i, "group", (*it) );
    }

    m_parent->send( iq );
  }

  void RosterManager::unsubscribe( const std::string& jid, const std::string& msg, bool remove )
  {
    Tag *s = new Tag( "presence" );
    s->addAttribute( "type", "unsubscribe" );
    s->addAttribute( "from", m_parent->jid().bare() );
    s->addAttribute( "to", jid );
    if( !msg.empty() )
      new Tag( s, "status", msg );

    m_parent->send( s );

    if( remove )
    {
      std::string id = m_parent->getID();

      Tag *iq = new Tag( "iq" );
      iq->addAttribute( "type", "set" );
      iq->addAttribute( "id", id );
      Tag *q = new Tag( iq, "query" );
      q->addAttribute( "xmlns", XMLNS_ROSTER );
      Tag *i = new Tag( q, "item" );
      i->addAttribute( "jid", jid );
      i->addAttribute( "subscription", "remove" );

      m_parent->send( iq );
    }
  }

  void RosterManager::synchronize()
  {
    RosterListener::Roster::const_iterator it = m_roster.begin();
    for( ; it != m_roster.end(); ++it )
    {
      if( (*it).second->changed() )
      {
        std::string id = m_parent->getID();

        Tag *iq = new Tag( "iq" );
        iq->addAttribute( "type", "set" );
        iq->addAttribute( "id", id );
        Tag *q = new Tag( iq, "query" );
        q->addAttribute( "xmlns", XMLNS_ROSTER );
        Tag *i = new Tag( q, "item" );
        i->addAttribute( "jid", (*it).second->jid() );
        if( !(*it).second->name().empty() )
          i->addAttribute( "name", (*it).second->name() );

        if( (*it).second->groups().size() != 0 )
        {
          StringList::const_iterator g_it = (*it).second->groups().begin();
          for( ; g_it != (*it).second->groups().end(); ++g_it )
            new Tag( i, "group", (*g_it) );
        }

        m_parent->send( iq );
      }
    }
  }

  void RosterManager::ackSubscriptionRequest( const JID& to, bool ack )
  {
    Tag *p = new Tag( "presence" );
    if( ack )
      p->addAttribute( "type", "subscribed" );
    else
      p->addAttribute( "type", "unsubscribed" );

    p->addAttribute( "from", m_parent->jid().bare() );
    p->addAttribute( "to", to.bare() );
    m_parent->send( p );
  }

  void RosterManager::handleSubscription( Stanza *stanza )
  {
    if( !m_rosterListener )
      return;

    switch( stanza->subtype() )
    {
      case StanzaS10nSubscribe:
      {
        bool answer = m_rosterListener->subscriptionRequest( stanza->from().bare(), stanza->status() );
        if( m_syncSubscribeReq )
        {
          ackSubscriptionRequest( stanza->from(), answer );
        }
        break;
      }
      case StanzaS10nSubscribed:
      {
//         Tag *p = new Tag( "presence" );
//         p->addAttribute( "type", "subscribe" );
//         p->addAttribute( "from", m_parent->jid().bare() );
//         p->addAttribute( "to", stanza->from().bare() );
//         m_parent->send( p );

        m_rosterListener->itemSubscribed( stanza->from().bare() );
        break;
      }

      case StanzaS10nUnsubscribe:
      {
        Tag *p = new Tag( "presence" );
        p->addAttribute( "type", "unsubscribed" );
        p->addAttribute( "from", m_parent->jid().bare() );
        p->addAttribute( "to", stanza->from().bare() );
        m_parent->send( p );

        bool answer = m_rosterListener->unsubscriptionRequest( stanza->from().bare(), stanza->status() );
        if( m_syncSubscribeReq && answer )
          unsubscribe( stanza->from().bare(), "", true );
        break;
      }

      case StanzaS10nUnsubscribed:
      {
//         Tag *p = new Tag( "presence" );
//         p->addAttribute( "type", "unsubscribe" );
//         p->addAttribute( "from", m_parent->jid().bare() );
//         p->addAttribute( "to", stanza->from().bare() );
//         m_parent->send( p );

        m_rosterListener->itemUnsubscribed( stanza->from().bare() );
        break;
      }

      default:
        break;
    }
  }

  void RosterManager::registerRosterListener( RosterListener *rl, bool syncSubscribeReq )
  {
    m_syncSubscribeReq = syncSubscribeReq;
    m_rosterListener = rl;
  }

  void RosterManager::removeRosterListener()
  {
    m_syncSubscribeReq = false;
    m_rosterListener = 0;
  }

  void RosterManager::extractItems( Tag *tag, bool isPush )
  {
    Tag *t = tag->findChild( "query" );
    Tag::TagList l = t->children();
    Tag::TagList::iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() == "item" )
      {
        StringList gl;
        if( (*it)->hasChild( "group" ) )
        {
          Tag::TagList g = (*it)->children();
          Tag::TagList::const_iterator it_g = g.begin();
          for( ; it_g != g.end(); ++it_g )
          {
            gl.push_back( (*it_g)->cdata() );
          }
        }

        const std::string jid = (*it)->findAttribute( "jid" );
        RosterListener::Roster::iterator it_d = m_roster.find( jid );
        if( it_d != m_roster.end() )
        {
          (*it_d).second->setName( (*it)->findAttribute( "name" ) );
          const std::string sub = (*it)->findAttribute( "subscription" );
          if( sub == "remove" )
          {
            delete (*it_d).second;
            m_roster.erase( it_d );
            if( m_rosterListener )
              m_rosterListener->itemRemoved( jid );
            continue;
          }
          const std::string ask = (*it)->findAttribute( "ask" );
          bool a = false;
          if( !ask.empty() )
            a = true;
          (*it_d).second->setSubscription( sub, a );
          (*it_d).second->setGroups( gl );
          (*it_d).second->setSynchronized();

          if( isPush && m_rosterListener )
            m_rosterListener->itemUpdated( jid );
        }
        else
        {
          const std::string sub = (*it)->findAttribute( "subscription" );
          if( sub == "remove" )
            continue;
          const std::string name = (*it)->findAttribute( "name" );
          const std::string ask = (*it)->findAttribute( "ask" );
          bool a = false;
          if( !ask.empty() )
            a = true;

          add( jid, name, gl, sub, a );
          if( isPush && m_rosterListener )
            m_rosterListener->itemAdded( jid );
        }
      }
    }
  }

  void RosterManager::add( const std::string& jid, const std::string& name,
                           StringList& groups, const std::string& sub, bool ask )
  {
    if( m_roster.find( jid ) == m_roster.end() )
      m_roster[jid] = new RosterItem( jid, name );

    m_roster[jid]->setSubscription( sub, ask );
    m_roster[jid]->setGroups( groups );
    m_roster[jid]->setSynchronized();
  }

  void RosterManager::setDelimiter( const std::string& delimiter )
  {
    m_delimiter = delimiter;
    Tag *t = new Tag( "roster", m_delimiter );
    t->addAttribute( "xmlns", XMLNS_ROSTER_DELIMITER );
    m_privateXML->storeXML( t, this );
  }

  void RosterManager::handlePrivateXML( const std::string& /*tag*/, Tag *xml )
  {
    m_delimiter = xml->cdata();
  }

  void RosterManager::handlePrivateXMLResult( const std::string& /*uid*/, PrivateXMLResult /*result*/ )
  {
  }

  RosterItem* RosterManager::getRosterItem( const JID& jid )
  {
    RosterListener::Roster::const_iterator it = m_roster.find( jid.bare() );
    if( it != m_roster.end() )
      return (*it).second;
    else
      return 0;
  }

}
