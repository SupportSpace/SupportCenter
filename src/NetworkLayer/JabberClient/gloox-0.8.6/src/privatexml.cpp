/*
  Copyright (c) 2004-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "privatexml.h"
#include "clientbase.h"
#include "stanza.h"

namespace gloox
{

  PrivateXML::PrivateXML( ClientBase *parent )
    : m_parent( parent )
  {
    if( m_parent )
      m_parent->registerIqHandler( this, XMLNS_PRIVATE_XML );
  }

  PrivateXML::~PrivateXML()
  {
    if( m_parent )
      m_parent->removeIqHandler( XMLNS_PRIVATE_XML );

  }

  std::string PrivateXML::requestXML( const std::string& tag, const std::string& xmlns,
                                      PrivateXMLHandler *pxh )
  {
    const std::string id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "id", id );
    iq->addAttribute( "type", "get" );
    Tag *query = new Tag( iq, "query" );
    query->addAttribute( "xmlns", XMLNS_PRIVATE_XML );
    Tag *x = new Tag( query, tag );
    x->addAttribute( "xmlns", xmlns );

    m_track[id] = pxh;
    m_parent->trackID( this, id, REQUEST_XML );
    m_parent->send( iq );

    return id;
  }

  std::string PrivateXML::storeXML( Tag *tag, PrivateXMLHandler *pxh )
  {
    const std::string id = m_parent->getID();

    Tag *iq = new Tag( "iq" );
    iq->addAttribute( "id", id );
    iq->addAttribute( "type", "set" );
    Tag *query = new Tag( iq, "query" );
    query->addAttribute( "xmlns", XMLNS_PRIVATE_XML );
    query->addChild( tag );

    m_track[id] = pxh;
    m_parent->trackID( this, id, STORE_XML );
    m_parent->send( iq );

    return id;
  }

  bool PrivateXML::handleIqID( Stanza *stanza, int context )
  {
    TrackMap::iterator t = m_track.find( stanza->id() );
    if( t != m_track.end() )
    {
      switch( stanza->subtype() )
      {
        case StanzaIqResult:
        {
          switch( context )
          {
            case REQUEST_XML:
            {
              Tag *q = stanza->findChild( "query" );
              if( q )
              {
                Tag::TagList l = q->children();
                Tag::TagList::const_iterator it = l.begin();
                if( it != l.end() )
                {
                  (*t).second->handlePrivateXML( (*it)->name(), (*it) );
                }
              }
              break;
            }

            case STORE_XML:
            {
              (*t).second->handlePrivateXMLResult( stanza->id(), PrivateXMLHandler::PXML_STORE_OK );
              break;
            }
          }
          return true;
          break;
        }
        case StanzaIqError:
        {
          switch( context )
          {
            case REQUEST_XML:
            {
              (*t).second->handlePrivateXMLResult( stanza->id(), PrivateXMLHandler::PXML_REQUEST_ERROR );
              break;
            }

            case STORE_XML:
            {
              (*t).second->handlePrivateXMLResult( stanza->id(), PrivateXMLHandler::PXML_STORE_ERROR );
              break;
            }
          }
          break;
        }
        default:
          break;
      }

      m_track.erase( t );
    }
    return false;
  }

  bool PrivateXML::handleIq( Stanza * /*stanza*/ )
  {
    return false;
  }

}
