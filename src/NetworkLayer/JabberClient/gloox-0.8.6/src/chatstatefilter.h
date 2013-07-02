/*
  Copyright (c) 2005-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef CHATSTATEFILTER_H__
#define CHATSTATEFILTER_H__

#include "messagefilter.h"
#include "gloox.h"

namespace gloox
{

  class Tag;
  class ChatStateHandler;
  class MessageSession;
  class Stanza;

  /**
   * @brief This class adds Chat State Notifications (JEP-0085) support to a MessageSession.
   *
   * This implementation of Chat States is fully transparent to the user of the class.
   * If the remote entity does not request chat states, ChatStateFilter will not send
   * any, even if the user requests it. (This is required by the protocol specification.)
   * You should annouce this capability by use of Disco (associated namespace is XMLNS_CHAT_STATES).
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API ChatStateFilter : public MessageFilter
  {
    public:
      /**
       * Contstructs a new Chat State filter for a MessageSession.
       * @param parent The MessageSession to decorate.
       */
      ChatStateFilter( MessageSession *parent );

      /**
       * Virtual destructor.
       */
      virtual ~ChatStateFilter();

      /**
       * Use this function to set a chat state as defined in JEP-0085.
       * @note The Spec states that Chat States shall not be sent to an entity
       * which did not request them. Reasonable effort is taken in this function to
       * avoid spurious state sending. You should be safe to call this even if Message
       * Events were not requested by the remote entity. However,
       * calling setChatState( CHAT_STATE_COMPOSING ) for every keystroke still is
       * discouraged. ;)
       * @param state The state to set.
       */
      void setChatState( ChatStateType state );

      /**
       * The ChatStateHandler registered here will receive Chat States according
       * to JEP-0085.
       * @param csh The ChatStateHandler to register.
       */
      void registerChatStateHandler( ChatStateHandler *csh );

      /**
       * This function clears the internal pointer to the ChatStateHandler.
       * Chat States will not be delivered anymore after calling this function until another
       * ChatStateHandler is registered.
       */
      void removeChatStateHandler();

      // reimplemented from MessageFilter
      virtual void decorate( Tag *tag );

      // reimplemented from MessageFilter
      virtual void filter( Stanza *stanza );

    private:
      ChatStateHandler *m_chatStateHandler;
      ChatStateType m_lastSent;
      bool m_enableChatStates;

  };

}

#endif // CHATSTATEFILTER_H__
