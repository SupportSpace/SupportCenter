/*
  Copyright (c) 2005-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "tag.h"

#include <sstream>

namespace gloox
{
  Tag::Tag()
    : m_parent( 0 ), m_type( StanzaUndefined ), m_incoming( false )
  {
  }

  Tag::Tag( const std::string& name, const std::string& cdata, bool incoming )
    : m_name( incoming ? relax( name ) : name ),
      m_cdata( incoming ? relax( cdata ) : cdata ),
      m_parent( 0 ), m_type( StanzaUndefined ), m_incoming( incoming )
  {
  }

  Tag::Tag( Tag *parent, const std::string& name, const std::string& cdata, bool incoming )
    : m_name( incoming ? relax( name ) : name ),
      m_cdata( incoming ? relax( cdata ) : cdata ),
      m_parent( parent ), m_type( StanzaUndefined ), m_incoming( incoming )
  {
    m_parent->addChild( this );
  }

  Tag::~Tag()
  {
    TagList::iterator it = m_children.begin();
    for( ; it != m_children.end(); ++it )
    {
      delete (*it);
      (*it) = 0;
    }
    m_children.clear();
    m_parent = 0;
  }

  void Tag::setCData( const std::string& cdata )
  {
    m_cdata = m_incoming ? relax( cdata ) : cdata;
  }

  void Tag::addCData( const std::string& cdata )
  {
    m_cdata += m_incoming ? relax( cdata ) : cdata;
  }

  const std::string Tag::xml() const
  {
    std::string xml;
    xml = "<" + escape( m_name );
    if( m_attribs.size() )
    {
      StringMap::const_iterator it_a = m_attribs.begin();
      for( ; it_a != m_attribs.end(); ++it_a )
      {
        xml += " " + escape( (*it_a).first ) + "='" + escape( (*it_a).second ) + "'";
      }
    }

    if( m_cdata.empty() && !m_children.size() )
      xml += "/>";
    else if( m_children.size() )
    {
      xml += ">";
      TagList::const_iterator it_c = m_children.begin();
      for( ; it_c != m_children.end(); ++it_c )
      {
        xml += (*it_c)->xml();
      }
      xml += "</" + escape( m_name ) + ">";
    }
    else if( !m_cdata.empty() )
      xml += ">" + escape( m_cdata ) + "</" + escape( m_name ) + ">";

    return xml;
  }

  void Tag::addAttribute( const std::string& name, const std::string& value )
  {
    if( !name.empty() && !value.empty() )
      m_attribs[m_incoming ? relax( name ) : name] = m_incoming ? relax( value ) : value;
  }

  void Tag::addAttribute( const std::string& name, int value )
  {
    if( !name.empty() )
    {
      std::ostringstream oss;
      oss << value;
      m_attribs[m_incoming ? relax( name ) : name] = oss.str();
    }
  }

  void Tag::addChild( Tag *child )
  {
    if( child )
    {
      m_children.push_back( child );
      child->m_parent = this;
    }
  }

  const std::string Tag::cdata() const
  {
    return m_cdata;
  }

  StringMap& Tag::attributes()
  {
    return m_attribs;
  }

  Tag::TagList& Tag::children()
  {
    return m_children;
  }

  const std::string Tag::findAttribute( const std::string& name ) const
  {
    StringMap::const_iterator it = m_attribs.find( name );
    if( it != m_attribs.end() )
      return (*it).second;
    else
      return "";
  }

  bool Tag::hasAttribute( const std::string& name, const std::string& value ) const
  {
    if( name.empty() )
      return true;

    StringMap::const_iterator it = m_attribs.find( name );
    if( it != m_attribs.end() )
      return ( ( value.empty() )?( true ):( (*it).second == value ) );
    else
      return false;
  }

  Tag* Tag::findChild( const std::string& name )
  {
    TagList::const_iterator it = m_children.begin();
    for( ; it != m_children.end(); ++it )
    {
      if( (*it)->name() == name )
        return (*it);
    }

    return 0;
  }

  Tag* Tag::findChild( const std::string& name, const std::string& attr,
                       const std::string& value )
  {
    TagList::const_iterator it = m_children.begin();
    for( ; it != m_children.end(); ++it )
    {
      if( ( (*it)->name() == name ) && (*it)->hasAttribute( attr, value ) )
        return (*it);
    }

    return 0;
  }

  bool Tag::hasChild( const std::string& name,
                      const std::string& attr, const std::string& value ) const
  {
    if( name.empty() )
      return false;

    TagList::const_iterator it = m_children.begin();
    for( ; it != m_children.end(); ++it )
    {
      if( ( (*it)->name() == name )
              && (*it)->hasAttribute( attr, value ) )
        return true;
    }

    return false;
  }

  bool Tag::hasChildWithCData( const std::string& name, const std::string& cdata ) const
  {
    TagList::const_iterator it = m_children.begin();
    for( ; it != m_children.end(); ++it )
    {
      if( ( (*it)->name() == name ) && !cdata.empty() && ( (*it)->cdata() == cdata ) )
        return true;
      else if( ( (*it)->name() == name ) && cdata.empty() )
        return true;
    }

    return false;
  }

  bool Tag::hasChildWithAttrib( const std::string& attr, const std::string& value ) const
  {
    TagList::const_iterator it = m_children.begin();
    for( ; it != m_children.end(); ++it )
    {
      if( (*it)->hasAttribute( attr, value ) )
        return true;
    }

    return false;
  }

  Tag* Tag::findChildWithAttrib( const std::string& attr, const std::string& value )
  {
    TagList::const_iterator it = m_children.begin();
    for( ; it != m_children.end(); ++it )
    {
      if( (*it)->hasAttribute( attr, value ) )
        return (*it);
    }

    return 0;
  }

  const std::string Tag::replace( const std::string& what, const Duo& duo ) const
  {
    std::string esc = what;
    Duo::const_iterator it = duo.begin();
    for( ; it != duo.end(); ++it )
    {
      size_t lookHere = 0;
      size_t foundHere = 0;
      while( ( foundHere = esc.find( (*it).first, lookHere ) ) != std::string::npos )
      {
        esc.replace( foundHere, (*it).first.size(), (*it).second );
        lookHere = foundHere + (*it).second.size();
      }
    }
    return esc;
  }

  const std::string Tag::escape( const std::string& what ) const
  {
    Duo d;
    d.push_back( duo( "&", "&amp;" ) );
    d.push_back( duo( "<", "&lt;" ) );
    d.push_back( duo( ">", "&gt;" ) );
    d.push_back( duo( "'", "&apos;" ) );
    d.push_back( duo( "\"", "&quot;" ) );

    return replace( what, d );
  }

  const std::string Tag::relax( const std::string& what ) const
  {
    Duo d;
    d.push_back( duo( "&#60;", "<" ) );
    d.push_back( duo( "&#62;", ">" ) );
    d.push_back( duo( "&#39;", "'" ) );
    d.push_back( duo( "&#34;", "\"" ) );
    d.push_back( duo( "&#x3c;", "<" ) );
    d.push_back( duo( "&#x3e;", ">" ) );
    d.push_back( duo( "&#x3C;", "<" ) );
    d.push_back( duo( "&#x3E;", ">" ) );
    d.push_back( duo( "&#x27;", "'" ) );
    d.push_back( duo( "&#x22;", "\"" ) );
    d.push_back( duo( "&#X3c;", "<" ) );
    d.push_back( duo( "&#X3e;", ">" ) );
    d.push_back( duo( "&#X3C;", "<" ) );
    d.push_back( duo( "&#X3E;", ">" ) );
    d.push_back( duo( "&#X27;", "'" ) );
    d.push_back( duo( "&#X22;", "\"" ) );
    d.push_back( duo( "&lt;", "<" ) );
    d.push_back( duo( "&gt;", ">" ) );
    d.push_back( duo( "&apos;", "'" ) );
    d.push_back( duo( "&quot;", "\"" ) );
    d.push_back( duo( "&amp;", "&" ) );

    return replace( what, d );
  }

  Tag* Tag::clone() const
  {
    Tag *t = new Tag( name(), cdata() );
    t->m_attribs = m_attribs;

    Tag::TagList::const_iterator it = m_children.begin();
    for( ; it != m_children.end(); ++it )
    {
      t->addChild( (*it)->clone() );
    }

    return t;
  }

}
