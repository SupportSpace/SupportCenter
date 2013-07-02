/*
  Copyright (c) 2005-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef JID_H__
#define JID_H__

#include "macros.h"

#include <string>

namespace gloox
{
  /**
   * @brief An abstraction of a JID.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.4
   */
  class GLOOX_API JID
  {
    public:

      /**
       * Constructs an empty JID.
       */
      JID();

      /**
       * Constructs a new JID from a string.
       * @param jid The string containing the JID.
       */
      JID( const std::string& jid );

      /**
       * Virtual destructor.
       */
      virtual ~JID();

      /**
       * Sets the JID from a string.
       * @param jid The string containing the JID.
       */
      void setJID( const std::string& jid );

      /**
       * Returns the full (prepped) JID (user\@host/resource).
       * @return The full JID.
       */
      std::string full() const;

      /**
       * Returns the bare (prepped) JID (user\@host).
       * @return The bare JID.
       */
      std::string bare() const;

      /**
       * Sets the username.
       * @param username The new username.
       */
      void setUsername( const std::string& username );

      /**
       * Sets the server.
       * @param server The new server.
       */
      void setServer( const std::string& server );

      /**
       * Sets the resource.
       * @param resource The new resource.
       */
      void setResource( const std::string& resource );

      /**
       * Returns the prepped username.
       * @return The current username.
       */
      std::string username() const { return m_username; };

      /**
       * Returns the prepped server name.
       * @return The current server.
       */
      std::string server() const { return m_server; };

      /**
       * Returns the raw (unprepped) server name.
       * @return The raw server name.
       */
      std::string serverRaw() const { return m_serverRaw; };

      /**
       * Returns the prepped resource.
       * @return The current resource.
       */
      std::string resource() const { return m_resource; };

      /**
       * A JID is empty as long as no server is set.
       * @return @b True if the JID is empty, @b false otherwise.
       */
      bool empty() const { return m_server.empty(); };

      /**
       * Compares two JIDs.
       * @param right The second JID.
       */
      int operator==( const JID& right ) const;

      /**
       * Compares two JIDs.
       * @param right The second JID.
       */
      int operator!=( const JID& right ) const;

    private:
      std::string m_resource;
      std::string m_username;
      std::string m_server;
      std::string m_serverRaw;
  };

}

#endif // JID_H__
