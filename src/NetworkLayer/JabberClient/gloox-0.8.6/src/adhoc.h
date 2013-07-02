/*
  Copyright (c) 2004-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef ADHOC_H__
#define ADHOC_H__

#include "adhoccommandprovider.h"
#include "disconodehandler.h"
#include "iqhandler.h"

#include <string>
#include <list>
#include <map>

namespace gloox
{

  class ClientBase;
  class Disco;
  class Stanza;

  /**
   * @brief This class implements a provider for JEP-0050 (Ad-hoc Commands).
   *
   * The current, not complete, implementation is probably best suited for fire-and-forget
   * type of commands. Any additional feature, like multiple stages, etc., would have to be
   * added separately.
   *
   * Use this class as follows:
   * Create a class that will handle command execution requests and derive it from
   * AdhocCommandProvider. Instantiate an Adhoc object and register your
   * AdhocCommandProvider-derived object with the Adhoc object using
   * registerAdhocCommandProvider(). The additional parameters to that method are the internal
   * name of the command as used in the code, and the public name of the command as it
   * will be shown to an end user:
   * @code
   * MyClass::someFunc()
   * {
   *   Adhoc *m_adhoc = new Adhoc( m_client, m_client->disco() );
   *
   *   // this might be a bot monitoring a weather station, for example
   *   m_adhoc->registerAdhocCommandProvider( this, "getTemp", "Retrieve current temperature" );
   *   m_adhoc->registerAdhocCommandProvider( this, "getPressure", "Retrieve current air pressure" );
   *   [...]
   * }
   * @endcode
   * In this example, MyClass is AdhocCommandProvider-derived so it is obviously the command handler, too.
   *
   * And that's about it you can do with the Adhoc class. Of course you can have a AdhocCommandProvider
   * handle more than one command, just register it with the Adhoc object for every desired command,
   * like shown above.
   *
   * What the Adhoc object does when you install a new command is tell the supplied Disco object
   * to advertise these commands to clients using the 'Service Discovery' protocol to learn about
   * this implementation's features. These clients can then call and execute the command. Of course you
   * are free to implement access restrictions to not let anyone mess with your bot, for example.
   * However, the commands offered using Service Discovery are publically visible in any case.
   *
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API Adhoc : public DiscoNodeHandler, IqHandler
  {
    public:
      /**
       * Constructor.
       * Creates a new Adhoc client that registers as IqHandler with a ClientBase.
       * @param parent The ClientBase used for XMPP communication.
       * @param disco The Disco object used to announce available commands.
       */
      Adhoc( ClientBase *parent, Disco *disco );

      /**
       * Virtual destructor.
       */
      virtual ~Adhoc();

      // reimplemented from DiscoNodeHandler
      virtual StringList handleDiscoNodeFeatures( const std::string& node );

      // reimplemented from DiscoNodeHandler
      virtual StringMap handleDiscoNodeIdentities( const std::string& node, std::string& name );

      // reimplemented from DiscoNodeHandler
      virtual StringMap handleDiscoNodeItems( const std::string& node );

      // reimplemented from IqHandler
      virtual bool handleIq( Stanza *stanza );

      // reimplemented from IqHandler
      virtual bool handleIqID( Stanza *stanza, int context );

      /**
       * Using this function, you can register a AdhocCommandProvider -derived object as
       * handler for a specific Ad-hoc Command as defined in JEP-0050.
       * @param acp The obejct to register as handler for the specified command.
       * @param command The node name of the command. Will be announced in disco#items.
       * @param name The natural-language name of the command. Will be announced in disco#items.
       */
      void registerAdhocCommandProvider( AdhocCommandProvider *acp, const std::string& command,
                                         const std::string& name );

      /**
       * Use this function to unregister an adhoc command previously registered using
       * registerAdhocCommandProvider().
       * @param command The command to unregister.
       */
      void removeAdhocCommandProvider( const std::string& command );

    private:
      typedef std::map<const std::string, AdhocCommandProvider*> AdhocCommandProviderMap;

      ClientBase *m_parent;
      Disco *m_disco;

      AdhocCommandProviderMap m_adhocCommandProviders;
      StringMap m_items;

  };

}

#endif // ADHOC_H__
