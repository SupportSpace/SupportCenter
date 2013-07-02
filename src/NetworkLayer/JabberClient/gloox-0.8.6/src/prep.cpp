/*
  Copyright (c) 2004-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifdef WIN32
#include "../config.h.win"
#else
#include "config.h"
#endif

#include "prep.h"

#include <string>

#ifdef HAVE_LIBIDN
#include <stringprep.h>
#include <idna.h>
#endif

#define JID_PORTION_SIZE 1023

namespace gloox
{

  std::string Prep::nodeprep( const std::string& node )
  {
    if( node.empty() )
      return node;

    if( node.length() > JID_PORTION_SIZE )
      return "";

#ifdef HAVE_LIBIDN
  char* p;
  char buf[JID_PORTION_SIZE + 1];
  memset( &buf, '\0', JID_PORTION_SIZE + 1 );
  strncpy( buf, node.c_str(), node.length() );
  p = stringprep_locale_to_utf8( buf );
    if ( p )
    {
      strncpy( buf, p, JID_PORTION_SIZE + 1 );
      free( p );
    }

    int rc = stringprep( (char*)&buf, JID_PORTION_SIZE,
                         (Stringprep_profile_flags)0, stringprep_xmpp_nodeprep );
    if ( rc != STRINGPREP_OK )
    {
      return "";
    }
    return buf;
#else
    return node;
#endif
  }

  std::string Prep::nameprep( const std::string& domain )
  {
    if( domain.empty() )
      return domain;

    if( domain.length() > JID_PORTION_SIZE )
      return "";

#ifdef HAVE_LIBIDN
    char* p;
    char buf[JID_PORTION_SIZE + 1];
    memset( &buf, '\0', JID_PORTION_SIZE + 1 );
    strncpy( buf, domain.c_str(), domain.length() );
    p = stringprep_locale_to_utf8( buf );
    if ( p )
    {
      strncpy( buf, p, JID_PORTION_SIZE + 1 );
      free( p );
    }

    int rc = stringprep( (char*)&buf, JID_PORTION_SIZE,
                         (Stringprep_profile_flags)0, stringprep_nameprep );
    if ( rc != STRINGPREP_OK )
    {
      return "";
    }
    return buf;
#else
    return domain;
#endif
  }

  std::string Prep::resourceprep( const std::string& resource )
  {
    if( resource.empty() )
      return resource;

    if( resource.length() > JID_PORTION_SIZE )
      return "";

#ifdef HAVE_LIBIDN
    char* p;
    char buf[JID_PORTION_SIZE + 1];
    memset( &buf, '\0', JID_PORTION_SIZE + 1 );
    strncpy( buf, resource.c_str(), resource.length() );
    p = stringprep_locale_to_utf8( buf );
    if ( p )
    {
      strncpy( buf, p, JID_PORTION_SIZE + 1 );
      free( p );
    }

    int rc = stringprep( (char*)&buf, JID_PORTION_SIZE,
                          (Stringprep_profile_flags)0, stringprep_xmpp_resourceprep );
    if ( rc != STRINGPREP_OK )
    {
      return "";
    }
    return buf;
#else
    return resource;
#endif
  }

  std::string Prep::idna( const std::string& domain )
  {
    if( domain.empty() )
      return domain;

    if( domain.length() > JID_PORTION_SIZE )
      return "";

#ifdef HAVE_LIBIDN
    char* p;
    char buf[JID_PORTION_SIZE + 1];
    memset( &buf, '\0', JID_PORTION_SIZE + 1 );
    strncpy( buf, domain.c_str(), domain.length() );
    p = stringprep_locale_to_utf8( buf );
    if ( p )
    {
      strncpy( buf, p, JID_PORTION_SIZE + 1 );
      free( p );
    }

    int rc = idna_to_ascii_8z( (char*)&buf, &p, (Idna_flags)0 );
    if ( rc != IDNA_SUCCESS )
    {
      return "";
    }
    return p;
#else
    return domain;
#endif
  }

}
