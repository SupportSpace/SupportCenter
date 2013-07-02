/*
  Copyright (c) 2004-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "adhoc.h"
#include "disco.h"
#include "discohandler.h"
#include "client.h"


namespace gloox
{

  Adhoc::Adhoc( ClientBase *parent, Disco *disco )
    : m_parent( parent ), m_disco( disco )
  {
    if( m_parent && m_disco )
    {
      m_parent->registerIqHandler( this, XMLNS_ADHOC_COMMANDS );
      m_disco->addFeature( XMLNS_ADHOC_COMMANDS );
      m_disco->registerNodeHandler( this, XMLNS_ADHOC_COMMANDS );
    }
  }

  Adhoc::~Adhoc()
  {
    if( m_parent )
    {
      m_parent->removeIqHandler( XMLNS_ADHOC_COMMANDS );
    }
    if( m_disco )
    {
      m_disco->removeNodeHandler( XMLNS_ADHOC_COMMANDS );
    }
  }

  StringList Adhoc::handleDiscoNodeFeatures( const std::string& /*node*/ )
  {
    StringList features;
    features.push_back( XMLNS_ADHOC_COMMANDS );
    return features;
  }

  StringMap Adhoc::handleDiscoNodeItems( const std::string& node )
  {
    if( node.empty() )
    {
      StringMap item;
      item[XMLNS_ADHOC_COMMANDS] = "Ad-Hoc Commands";
      return item;
    }
    else if( node == XMLNS_ADHOC_COMMANDS )
    {
      return m_items;
    }
    else
    {
      StringMap item;
      return item;
    }
  }

  StringMap Adhoc::handleDiscoNodeIdentities( const std::string& node, std::string& name )
  {
    StringMap::const_iterator it = m_items.find( node );
    if( it != m_items.end() )
      name = (*it).second;
    else
      name = "Ad-Hoc Commands";

    StringMap ident;
    if( node == XMLNS_ADHOC_COMMANDS )
      ident["automation"] = "command-list";
    else
      ident["automation"] = "command-node";
    return ident;
  }

  bool Adhoc::handleIq( Stanza *stanza )
  {
    if( stanza->hasChild( "command" ) )
    {
      Tag *c = stanza->findChild( "command" );
      const std::string node = c->findAttribute( "node" );
      AdhocCommandProviderMap::const_iterator it = m_adhocCommandProviders.find( node );
      if( !node.empty() && ( it != m_adhocCommandProviders.end() ) )
      {
        (*it).second->handleAdhocCommand( node, c );
        return true;
      }
    }
    return false;
  }

  bool Adhoc::handleIqID( Stanza * /*stanza*/, int /*context*/ )
  {
    return false;
  }

  void Adhoc::registerAdhocCommandProvider( AdhocCommandProvider *acp, const std::string& command,
                                            const std::string& name )
  {
    m_disco->registerNodeHandler( this, command );
    m_adhocCommandProviders[command] = acp;
    m_items[command] = name;
  }

  void Adhoc::removeAdhocCommandProvider( const std::string& command )
  {
    m_disco->removeNodeHandler( command );
    m_adhocCommandProviders.erase( command );
    m_items.erase( command );
  }
}
