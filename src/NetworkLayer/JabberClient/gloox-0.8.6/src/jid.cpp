/*
  Copyright (c) 2005-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "jid.h"

#include "prep.h"

namespace gloox
{

  JID::JID()
  {
  }

  JID::JID( const std::string& jid )
  {
    setJID( jid );
  }

  JID::~JID()
  {
  }

  void JID::setJID( const std::string& jid )
  {
    if( jid.empty() )
      return;

    size_t at = jid.find( "@", 0 );
    size_t slash = jid.find( "/", 0 );

    if( ( at == std::string::npos ) && ( slash == std::string::npos ) )
    {
      m_serverRaw = jid;
    }

    if( ( at != std::string::npos ) && ( slash != std::string::npos ) )
    {
      m_username = Prep::nodeprep( jid.substr( 0, at ) );
      m_serverRaw = jid.substr( at + 1, slash - at - 1 );
      m_resource = Prep::resourceprep( jid.substr( slash + 1 ) );
    }

    if( ( at == std::string::npos ) && ( slash != std::string::npos ) )
    {
      m_serverRaw = jid.substr( 0, slash );
      m_resource = Prep::resourceprep( jid.substr( slash + 1 ) );
    }

    if( ( at != std::string::npos ) && ( slash == std::string::npos ) )
    {
      m_username = Prep::nodeprep( jid.substr( 0, at ) );
      m_serverRaw = jid.substr( at + 1 );
    }

    m_server = Prep::nameprep( m_serverRaw );
  }

  void JID::setUsername( const std::string& username )
  {
    m_username = Prep::nodeprep( username );
  }

  void JID::setServer( const std::string& server )
  {
    m_serverRaw = server;
    m_server = Prep::nameprep( m_serverRaw );
  }

  void JID::setResource( const std::string& resource )
  {
    m_resource = Prep::resourceprep( resource );
  }

  std::string JID::full() const
  {
    if( m_server.empty() )
      return "";
    else if( m_username.empty() )
      if( m_resource.empty() )
        return m_server;
      else
        return ( m_server + "/" + m_resource );
    else
      if( m_resource.empty() )
        return ( m_username + "@" + m_server );
      else
        return ( m_username + "@" + m_server + "/" + m_resource );
  }

  std::string JID::bare() const
  {
    if( m_server.empty() )
      return "";
    else if( m_username.empty() )
      return m_server;
    else
      return m_username + "@" + m_server;
  }

  int JID::operator==( const JID& right ) const
  {
    return ( ( m_resource == right.m_resource )
        && ( m_server == right.m_server )
        && ( m_username == right.m_username ) );
  }

  int JID::operator!=( const JID& right ) const
  {
    return !( *this == right );
  }

}
