/*
  Copyright (c) 2005-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef MESSAGESESSION_H__
#define MESSAGESESSION_H__

#include "messagehandler.h"
#include "jid.h"

#include <string>

namespace gloox
{

  class ClientBase;
  class Tag;
  class MessageFilter;

  /**
   * @brief An abstraction of a message session between any two entities.
   *
   * This is an alternative interface to raw, old-style messaging. The original interface, using the simple
   * MessageHandler-derived interface, is based on an all-or-nothing approach. Once registered with
   * ClientBase, a handler receives all message stanzas sent to this client and has to do any filtering
   * on its own.
   *
   * MessageSession adds an abstraction to a chat conversation. A MessageSession is responsible for
   * communicating with exactly one (full) JID. It is extensible with so-called MessageFilters, which can
   * provide additional features such as Message Events, Chat State Notifications or In-Band Bytestreams.
   *
   * You can still use the old MessageHandler in parallel, but messages will not be relayed to both
   * the generic MessageHandler and a MessageSession established for the sender's JID. The MessageSession
   * takes precedence.
   *
   * Using MessageSessions has the following advantages over the plain old MessageHandler:
   * @li automatic creation of MessageSessions
   * @li filtering by JID
   * @li automatic handling of threading (XMPP message threads, that is)
   * @li simpler sending of messages
   * @li support for MessageFilters.
   *
   * @b Usage:<br>
   * Derive an object from MessageSessionHandler and reimplement handleMessageSession() to store your
   * shiny new sessions somewhere, or to create a new chat window, or whatever. Register your
   * object with a ClientBase instance using setAutoMessageSession(). In code:
   * @code
   * void MyClass::myFunc()
   * {
   *   JID jid( "abc@example.org/gloox" );
   *   j = new Client( jid, "password" );
   *   [...]
   *   j->setAutoMessageSession( true, this );
   * }
   * @endcode
   * MyClass is a MessageSessionHandler here.
   *
   * In this example, MyClass needs to be MessageHandler, MessageEventHandler and
   * ChatStateHandler, too. The handlers are registered with the session to receive the
   * respective events.
   * @code
   * virtual void MyClass::handleMessageSession( MessageSession *session )
   * {
   *   // this leaks heavily if there was an earlier session
   *   m_session = session;
   *   m_session->registerMessageHandler( this );
   *
   *   // the following is optional
   *   m_messageEventFilter = new MessageEventFilter( m_session );
   *   m_messageEventFilter->registerMessageEventHandler( this );
   *   m_chatStateFilter = new ChatStateFilter( m_session );
   *   m_chatStateFilter->registerChatStateHandler( this );
   * }
   * @endcode
   *
   * MessageEventHandler::handleMessageEvent() and ChatStateHandler::handleChatState() are called
   * for incoming Message Events and Chat States, respectively.
   * @code
   * virtual void MyClass::handleMessageEvent( const JID& from, MessageEventType event )
   * {
   *   // display contact's Message Event
   * }
   *
   * virtual void MyClass::handleChatState( const JID& from, ChatStateType state )
   * {
   *   // display contact's Chat State
   * }
   * @endcode
   *
   * To let the chat partner now that the user is typing a message or has closed the chat window, use
   * raiseMessageEvent() and setChatState(), respectively. For example:
   * @code
   * // user is typing a message
   * m_messageEventFilter->raiseMessageEvent( MESSAGE_EVENT_COMPOSING );
   *
   * // acknowledge receiving of a message
   * m_messageEventFilter->raiseMessageEvent( MESSAGE_EVENT_DELIVERED );
   *
   * // user is not actively paying attention to the chat
   * m_chatStateFilter->setChatState( CHAT_STATE_INACTIVE );
   *
   * // user has closed the chat window
   * m_chatStateFilter->setChatState( CHAT_STATE_GONE );
   * @endcode
   *
   * To send a message to the chat partner of the session, use
   * @ref send( const std::string& message, const std::string& subject ). You don't have to care about
   * receipient, thread id, etc., they are added automatically.
   *
   * @code
   * m_session->send( "Hello World!", "No Subject" );
   * @endcode
   *
   * To initiate a new chat session, all you have to do is create a new MessageSession and register
   * a MessageHandler with it:
   * @code
   * MessageSession* MyClass::newSession( const JID& to )
   * {
   *   MessageSession *session = new MessageSession( m_client, to );
   *   session->registerMessageHandler( this );
   *   return session;
   * }
   * @endcode
   *
   * See InBandBytestreamManager for a detailed description on how to implement In-Band Bytestreams.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API MessageSession : public MessageHandler
  {

    friend class MessageEventFilter;
    friend class ChatStateFilter;
    friend class InBandBytestream;

    public:
      /**
       * Constructs a new MessageSession for the given JID.
       * It is recommended to supply a full JID, in other words, it should have a resource set.
       * No resource can lead to unexpected behavior. A thread ID is generated and sent along
       * with every message sent through this session.
       * @param parent The ClientBase to use for communication.
       * @param jid The remote contact's full JID.
       */
      MessageSession( ClientBase *parent, const JID& jid );

      /**
       * Virtual destructor.
       *
       * @note This destructor de-registers with the ClientBase provided to the constructor. So make
       * sure you have it still around when you delete your last MessageSession.
       */
      virtual ~MessageSession();

      /**
       * Use this function to find out where is this session points at.
       * @return The receipient's JID.
       */
      const JID& target() const { return m_target; };

      /**
       * By default, a thread ID is sent with every message to identify
       * messages belonging together.
       * @returns The thread ID for this session.
       */
      const std::string& threadID() const { return m_thread; };

      /**
       * Use this function to associate a MessageHandler with this MessageSession.
       * The MessageHandler will receive all messages sent from this MessageSession's
       * remote contact.
       * @param mh The MessageHandler to register.
       */
      void registerMessageHandler( MessageHandler *mh );

      /**
       * This function clears the internal pointer to the MessageHandler and therefore
       * disables message delivery.
       */
      void removeMessageHandler();

      /**
       * A convenience function to quickly send a message (optionally with subject). This is
       * the preferred way to send a message from a MessageSession.
       * @param message The message to send.
       * @param subject The optional subject to send.
       */
      void send( const std::string& message, const std::string& subject );

      /**
       * Use this function to hook a new MessageFilter into a MessageSession.
       * The filter will be able to read and/or modify a message stanza's content.
       * @param mf The MessageFilter to add.
       */
      void registerMessageFilter( MessageFilter *mf );

      /**
       * Use this function to remove a MessageFilter from the MessageSession.
       * @param mf The MessageFilter to remove.
       */
      void removeMessageFilter( MessageFilter *mf );

      // reimplemented from MessageHandler
      virtual void handleMessage( Stanza *stanza );

    protected:
      /**
      * A wrapper around ClientBase::send(). You should @b not use this function to send a
      * chat message because the Tag is not prepared accordingly (neither Thread ID nor Message
      * Event requests are added).
      * @param tag A Tag to send.
      */
      virtual void send( Tag *tag );

    private:
      typedef std::list<MessageFilter*> MessageFilterList;

      MessageFilterList m_messageFilterList;
      ClientBase *m_parent;
      JID m_target;
      MessageHandler *m_messageHandler;
      std::string m_thread;

  };

}

#endif // MESSAGESESSION_H__
