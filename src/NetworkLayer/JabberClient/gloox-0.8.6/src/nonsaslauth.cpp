/*
  Copyright (c) 2005-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "nonsaslauth.h"
#include "client.h"

#include <string>

#include <iksemel\iksemel.h>

namespace gloox
{

  NonSaslAuth::NonSaslAuth( Client *parent, const std::string& sid )
    : m_parent( parent ), m_sid( sid )
  {
    if( m_parent )
      m_parent->registerIqHandler( this, XMLNS_AUTH );
  }

  NonSaslAuth::~NonSaslAuth()
  {
    if( m_parent )
      m_parent->removeIqHandler( XMLNS_AUTH );
  }

  void NonSaslAuth::doAuth()
  {
    std::string id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "to", m_parent->jid().server() );
    iq->addAttribute( "id", id );
    iq->addAttribute( "type", "get" );
    Tag *q = new Tag( iq, "query" );
    q->addAttribute( "xmlns", XMLNS_AUTH );
    new Tag( q, "username", m_parent->username() );

    m_parent->trackID( this, id, TRACK_REQUEST_AUTH_FIELDS );
    m_parent->send( iq );
  }

  bool NonSaslAuth::handleIqID( Stanza *stanza, int context )
  {
    switch( stanza->subtype() )
    {
      case StanzaIqError:
      {
        m_parent->setAuthed( false );
        m_parent->disconnect( ConnAuthenticationFailed );

        Tag *t = stanza->findChild( "error" );
        if( t )
        {
          if( t->hasChild( "conflict" ) || t->hasAttribute( "code", "409" ) )
            m_parent->setAuthFailure( NonSaslConflict );
          else if( t->hasChild( "not-acceptable" ) || t->hasAttribute( "code", "406" ) )
            m_parent->setAuthFailure( NonSaslNotAcceptable );
          else if( t->hasChild( "not-authorized" ) || t->hasAttribute( "code", "401" ) )
            m_parent->setAuthFailure( NonSaslNotAuthorized );
        }
        break;
      }
      case StanzaIqResult:
        switch( context )
        {
          case TRACK_REQUEST_AUTH_FIELDS:
          {
            std::string id = m_parent->getID();

            Tag *iq = new Tag( "iq" );
            iq->addAttribute( "id", id );
            iq->addAttribute( "type", "set" );
            Tag *query = new Tag( iq, "query" );
            query->addAttribute( "xmlns", XMLNS_AUTH );
            new Tag( query, "username", m_parent->jid().username() );
            new Tag( query, "resource", m_parent->jid().resource() );

            Tag *q = stanza->findChild( "query" );
            if( ( q->hasChild( "digest" ) ) && !m_sid.empty() )
            {
              const std::string pwd = m_parent->password();
              char buf[41];
              iksha *sha;
              sha = iks_sha_new();
              iks_sha_hash( sha, (const unsigned char*)m_sid.c_str(), m_sid.length(), 0 );
              iks_sha_hash( sha, (const unsigned char*)pwd.c_str(), pwd.length(), 1 );
              iks_sha_print( sha, buf );
              iks_sha_delete( sha );
              new Tag( query, "digest", buf );
            }
            else
            {
              new Tag( query, "password", m_parent->password() );
            }

            m_parent->trackID( this, id, TRACK_SEND_AUTH );
            m_parent->send( iq );
            break;
          }
          case TRACK_SEND_AUTH:
            m_parent->setAuthed( true );
            m_parent->connected();
            break;
        }
        break;

      default:
        break;
    }
    return false;
  }

  bool NonSaslAuth::handleIq( Stanza * /*stanza*/ )
  {
    return false;
  }

}
