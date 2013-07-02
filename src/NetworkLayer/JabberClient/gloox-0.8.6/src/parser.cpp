/*
  Copyright (c) 2004-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "gloox.h"

#include "parser.h"
#include "clientbase.h"
#include "stanza.h"

namespace gloox
{

  Parser::Parser( ClientBase *parent )
    : m_parent( parent ), m_current( 0 ), m_root( 0 )
  {
    m_parser = iks_sax_new( this, (iksTagHook*)tagHook, (iksCDataHook*)cdataHook );
  }

  Parser::~Parser()
  {
    iks_parser_delete( m_parser );
    delete m_root;
  }

  Parser::ParserState Parser::feed( const std::string& data )
  {
    int res = iks_parse( m_parser, data.c_str(), data.length(), 0 );
    switch( res )
    {
      case IKS_NOMEM:
        return PARSER_NOMEM;
        break;
      case IKS_BADXML:
        return PARSER_BADXML;
        break;
      case IKS_OK:
      default:
        return PARSER_OK;
        break;
    }
  }

  void Parser::streamEvent( Stanza *stanza )
  {
    if( m_parent && stanza )
    {
      ClientBase::NodeType type = ClientBase::NODE_STREAM_CHILD;
      if( m_root->name() == "stream:stream" )
        type = ClientBase::NODE_STREAM_START;
      else if( m_root->name() == "stream:error" )
        type = ClientBase::NODE_STREAM_ERROR;

      m_parent->filter( type, stanza );
    }
    else if( !stanza )
      m_parent->filter( ClientBase::NODE_STREAM_CLOSE, 0 );
  }

  int tagHook( Parser *parser, char *name, char **atts, int type )
  {
    if( !name )
      return IKS_OK;

    switch( type )
    {
      case IKS_OPEN:
      case IKS_SINGLE:
      {
        Stanza *tag = new Stanza( name, "", "default", true );
        for(int i=0; atts && atts[i]; )
        {
          tag->addAttribute( atts[i], atts[i+1] );
          i+=2;
        }
        if( !parser->m_root )
        {
          parser->m_root = tag;
          parser->m_current = parser->m_root;
        }
        else
        {
          parser->m_current->addChild( tag );
          parser->m_current = tag;
        }
        if( tag->name() == "stream:stream" )
        {
          parser->streamEvent( parser->m_root );
          delete parser->m_root;
          parser->m_root = 0;
          parser->m_current = 0;
        }
        if( type == IKS_OPEN )
        break;
      }
      case IKS_CLOSE:
        if( iks_strncmp( name, "stream:stream", 13 ) == 0 )
        {
          parser->streamEvent( 0 );
          break;
        }
        parser->m_current = dynamic_cast<Stanza*>( parser->m_current->parent() );
        if( !parser->m_current )
        {
          parser->m_root->finalize();
          parser->streamEvent( parser->m_root );
          delete parser->m_root;
          parser->m_root = 0;
          parser->m_current = 0;
        }
        break;
    }
    return IKS_OK;
  }

  int cdataHook( Parser *parser, char *data, size_t len )
  {
    if( parser->m_current && data && len )
    {
      std::string tmp;
      tmp.assign( data, len );
      parser->m_current->addCData( tmp );
    }

    return IKS_OK;
  }

}
