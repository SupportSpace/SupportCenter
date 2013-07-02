/*
  Copyright (c) 2005-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "dataformfield.h"
#include "tag.h"

namespace gloox
{

  DataFormField::DataFormField( DataFormFieldType type )
    : m_type( type ), m_required( false )
  {
  }

  DataFormField::DataFormField( Tag *tag )
    : m_type( FIELD_TYPE_INVALID ), m_required( false )
  {
    if( !tag )
      return;

    if( tag->hasAttribute( "type", "boolean" ) )
      m_type = FIELD_TYPE_BOOLEAN;
    else if( tag->hasAttribute( "type", "fixed" ) )
      m_type = FIELD_TYPE_FIXED;
    else if( tag->hasAttribute( "type", "hidden" ) )
      m_type = FIELD_TYPE_HIDDEN;
    else if( tag->hasAttribute( "type", "jid-multi" ) )
      m_type = FIELD_TYPE_JID_MULTI;
    else if( tag->hasAttribute( "type", "jid-single" ) )
      m_type = FIELD_TYPE_JID_SINGLE;
    else if( tag->hasAttribute( "type", "list-multi" ) )
      m_type = FIELD_TYPE_LIST_MULTI;
    else if( tag->hasAttribute( "type", "list-single" ) )
      m_type = FIELD_TYPE_LIST_SINGLE;
    else if( tag->hasAttribute( "type", "text-multi" ) )
      m_type = FIELD_TYPE_TEXT_MULTI;
    else if( tag->hasAttribute( "type", "text-private" ) )
      m_type = FIELD_TYPE_TEXT_PRIVATE;
    else if( tag->hasAttribute( "type", "text-single" ) )
      m_type = FIELD_TYPE_TEXT_SINGLE;

    if( tag->hasAttribute( "var" ) )
      m_name = tag->findAttribute( "var" );

    if( tag->hasAttribute( "label" ) )
      m_label = tag->findAttribute( "label" );

    Tag::TagList l = tag->children();
    Tag::TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() == "desc" )
        m_desc = (*it)->cdata();
      else if( (*it)->name() == "required" )
        m_required = true;
      else if( (*it)->name() == "value" )
      {
        if( m_type == FIELD_TYPE_TEXT_MULTI || m_type == FIELD_TYPE_LIST_MULTI )
          m_values.push_back( (*it)->cdata() );
        else
          m_value = (*it)->cdata();
      }
      else if( (*it)->name() == "option" )
      {
        Tag *v = (*it)->findChild( "value" );
        if( v )
          m_options[(*it)->findAttribute( "label" )] = v->cdata();
      }
    }

  }

  DataFormField::~DataFormField()
  {
  }

  Tag* DataFormField::tag() const
  {
    if( m_type == FIELD_TYPE_INVALID )
      return 0;

    Tag *field = new Tag( "field" );
    field->addAttribute( "var", m_name );
    field->addAttribute( "label", m_label );
    if( m_required )
      new Tag( field, "required" );

    if( !m_desc.empty() )
      new Tag( field, "desc", m_desc );

    switch( m_type )
    {
      case FIELD_TYPE_BOOLEAN:
        field->addAttribute( "type", "boolean" );
        break;
      case FIELD_TYPE_FIXED:
        field->addAttribute( "type", "fixed" );
        break;
      case FIELD_TYPE_HIDDEN:
        field->addAttribute( "type", "hidden" );
        break;
      case FIELD_TYPE_JID_MULTI:
        field->addAttribute( "type", "jid-multi" );
        break;
      case FIELD_TYPE_JID_SINGLE:
        field->addAttribute( "type", "jid-single" );
        break;
      case FIELD_TYPE_LIST_MULTI:
        field->addAttribute( "type", "list-multi" );
        break;
      case FIELD_TYPE_LIST_SINGLE:
        field->addAttribute( "type", "list-single" );
        break;
      case FIELD_TYPE_TEXT_MULTI:
        field->addAttribute( "type", "text-multi" );
        break;
      case FIELD_TYPE_TEXT_PRIVATE:
        field->addAttribute( "type", "text-private" );
        break;
      case FIELD_TYPE_TEXT_SINGLE:
        field->addAttribute( "type", "text-single" );
        break;
      default:
        break;
    }

    if( m_type == FIELD_TYPE_LIST_SINGLE || m_type == FIELD_TYPE_LIST_MULTI )
    {
      StringMap::const_iterator it = m_options.begin();
      for( ; it != m_options.end(); ++it )
      {
        Tag *option = new Tag( field, "option" );
        option->addAttribute( "label", (*it).first );
        new Tag( option, "value", (*it).second );
      }
    }
    else if( m_type == FIELD_TYPE_BOOLEAN )
    {
      if( m_value.empty() || m_value == "false" || m_value == "0" )
        new Tag( field, "value", "0" );
      else
        new Tag( field, "value", "1" );
    }
    
    if( m_type == FIELD_TYPE_TEXT_MULTI || m_type == FIELD_TYPE_LIST_MULTI )
    {
      StringList::const_iterator it = m_values.begin();
      for( ; it != m_values.end() ; ++it )
        new Tag( field, "value", (*it) );
    }

    if( !m_value.empty() )
      new Tag( field, "value", m_value );

    return field;
  }

}
