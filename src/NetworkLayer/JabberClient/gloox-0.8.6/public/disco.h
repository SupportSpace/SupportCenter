/*
  Copyright (c) 2004-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef DISCO_H__
#define DISCO_H__

#include "gloox.h"

#include "iqhandler.h"
#include "disconodehandler.h"

#include <string>
#include <list>
#include <map>

namespace gloox
{

  class ClientBase;
  class DiscoHandler;
  class DiscoItem;
  class Stanza;

  /**
   * @brief This class implements JEP-0030 (Service Discovery) and JEP-0092 (Software Version).
   *
   * Client and Component will automatically instantiate a Disco obejct. It can be used to
   * announce special features of your client, or its version, or...
   *
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API Disco : public IqHandler
  {
    friend class Client;
    friend class Component;

    public:
      /**
       * Adds a feature to the list of supported Jabber features.
       * The list will be posted as an answer to IQ queries in the
       * "http://jabber.org/protocol/disco#info" namespace.
       * These IQ packets will also be forwarded to the
       * application's IqHandler, if it listens to the disco\#info namespace.
       * You can call Client::disableDisco() to disable automatic disco
       * entirely. By default, disco(very) queries are handled by the library.
       * By default, all supported, not disabled features are announced.
       * @param feature A feature (namespace) the host app supports.
       * @note Use this function for non-queryable features. For nodes that shall
       * answer to disco\#info queries, use registerNodeHandler().
       */
      void addFeature( const std::string& feature );

      /**
       * Lets you retrieve the features this Disco instance supports.
       * @return A list of disco items.
       */
      const StringList& features() const { return m_features; };

      /**
       * Queries the given JID for general infomation according to
       * JEP-0030 (Service Discovery).
       * To receive the results inherit from DiscoHandler and register with the Disco object.
       * @param to The destination-JID of the query.
       * @param node An optional node to query. Not inserted if empty.
       * @param dh The DiscoHandler to notify about results.
       * @param context A context identifier.
       */
      void getDiscoInfo( const std::string& to, const std::string& node, DiscoHandler *dh, int context );

      /**
       * Queries the given JID for its items according to
       * JEP-0030 (Service Discovery).
       * To receive the results inherit from DiscoHandler and register with the Disco object.
       * @param to The destination-JID of the query.
       * @param node An optional node to query. Not inserted if empty.
       * @param dh The DiscoHandler to notify about results.
       * @param context A context identifier.
       */
      void getDiscoItems( const std::string& to, const std::string& node, DiscoHandler *dh, int context );

      /**
       * Sets the version of the host application using this library.
       * The library takes care of jabber:iq:version requests. These
       * IQ packets will not be forwarded to the IqHandlers.
       * @param name The name to be returned to inquireing clients.
       * @param version The version to be returned to inquireing clients.
       * @param os The operating system to announce. Default: don't include.
       */
      void setVersion( const std::string& name, const std::string& version, const std::string& os = "" );

      /**
       * Sets the identity of this entity.
       * The library uses this information to answer disco#info requests
       * with a correct identity.
       * JEP-0030 requires an entity to have at least one identity. See JEP-0030
       * for more information on categories and types.
       * @param category The entity category of this client. Default: client
       * @param type The type of this entity. Default: bot
       */
      void setIdentity( const std::string& category, const std::string& type );

      /**
       * Use this function to register an @ref DiscoHandler with the Disco
       * object. This is only necessary if you want to receive Disco-set requests. Else
       * a one-time registration happens when calling getDiscoInfo() and getDiscoItems(), respectively.
       * @param dh The DiscoHandler-derived object to register.
       */
      void registerDiscoHandler( DiscoHandler *dh );

      /**
       * Unregisters the given DiscoHandler.
       * @param dh The DiscoHandler to unregister.
       */
      void removeDiscoHandler( DiscoHandler *dh );

      /**
       * Use this function to register an @ref DiscoNodeHandler with the Disco
       * object. The DiscoNodeHandler will receive disco#items queries which are
       * directed to the corresponding node registered for the handler.
       * Only one handler per node is possible.
       * @param nh The NodeHandler-derived object to register.
       * @param node The node name to associate with this handler.
       */
      void registerNodeHandler( DiscoNodeHandler *nh, const std::string& node );

      /**
       * Removes the node handler.
       * @param node The node for which the handler shall be removed.
       */
      void removeNodeHandler( const std::string& node );

      // reimplemented from IqHandler.
      virtual bool handleIq( Stanza *stanza );

      // reimplemented from IqHandler.
      virtual bool handleIqID( Stanza *stanza, int context );

    private:
      Disco( ClientBase *parent );
      virtual ~Disco();

      enum IdType
      {
        GET_DISCO_INFO,
        GET_DISCO_ITEMS
      };

      struct DiscoHandlerContext
      {
        DiscoHandler *dh;
        int context;
      };

      ClientBase *m_parent;

      typedef std::list<DiscoHandler*>                   DiscoHandlerList;
      typedef std::map<std::string, DiscoNodeHandler*>   DiscoNodeHandlerMap;
      typedef std::map<std::string, DiscoHandlerContext> DiscoHandlerMap;
      typedef std::list<DiscoItem*>                      ItemList;

      DiscoHandlerList m_discoHandlers;
      DiscoNodeHandlerMap m_nodeHandlers;
      DiscoHandlerMap m_track;
      ItemList m_items;
      StringList m_features;
      StringMap  m_queryIDs;

      std::string m_versionName;
      std::string m_versionVersion;
      std::string m_versionOs;
      std::string m_identityCategory;
      std::string m_identityType;

  };

}

#endif // DISCO_H__
