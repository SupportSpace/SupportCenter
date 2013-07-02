/*
  Copyright (c) 2005-2007 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef MESSAGESESSION_H__
#define MESSAGESESSION_H__

#include "jid.h"

#include <string>
#include <list>

namespace gloox
{

  class ClientBase;
  class Tag;
  class MessageFilter;
  class MessageHandler;
  class Stanza;

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
   * @li automatic handling of threading (i.e., XMPP message threads)
   * @li simpler sending of messages
   * @li support for MessageFilters.
   *
   * @b Usage:<br>
   * Derive an object from MessageSessionHandler and reimplement handleMessageSession() to store your
   * shiny new sessions somewhere, or to create a new chat window, or whatever. Register your
   * object with a ClientBase instance using registerMessageSessionHandler(). In code:
   * @code
   * void MyClass::myFunc()
   * {
   *   JID jid( "abc@example.org/gloox" );
   *   j = new Client( jid, "password" );
   *   [...]
   *   j->registerMessageSessionHandler( this, 0 );
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
   *   // for this example only, we delete any earlier session
   *   if( m_session )
   *     delete m_session;
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
   * m_messageEventFilter->raiseMessageEvent( MessageEventComposing );
   *
   * // acknowledge receiving of a message
   * m_messageEventFilter->raiseMessageEvent( MessageEventDelivered );
   *
   * // user is not actively paying attention to the chat
   * m_chatStateFilter->setChatState( ChatStateInactive );
   *
   * // user has closed the chat window
   * m_chatStateFilter->setChatState( ChatStateGone );
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
   * When done using a MessageSession, you can either delete it or pass it to
   * @link gloox::ClientBase::disposeMessageSession ClientBase::disposeMessageSession @endlink.
   * Use @link gloox::ClientBase::removeMessageSession ClientBase::removeMessageSession @endlink
   * to detach a MessageSession from its ClientBase.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API MessageSession
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
       * @param jid The remote contact's full JID. If you don't know the full JID (this is probably the
       * most common case) but still want replies from the full JID to be handled by this MessageSession,
       * set the @b wantUpgrade parameter to true (or leave it untouched).
       * @param wantUpgrade This flag indicates whether gloox should try to match an incoming message
       * from a full JID to this MessageSession. If unsure, use the default. You probably only want to use
       * a non-default value if this MessageSession is supposed to talk directly to a server or component
       * JID that has no resource. This 'upgrade' will only happen once.
       * @param types ORed list of StanzaSubType values this MessageSession shall receive. Only the
       * StanzaMessage* types are valid. Defaults to 0 which means any type is received.
       */
      MessageSession( ClientBase *parent, const JID& jid, bool wantUpgrade = true, int types = 0 );

      /**
       * Virtual destructor.
       *
       * @note This destructor de-registers with the ClientBase provided to the constructor. So make
       * sure you have it still around when you delete your last MessageSession.
       */
      virtual ~MessageSession();

      /**
       * Use this function to find out where this session points at.
       * @return The receipient's JID.
       */
      const JID& target() const { return m_target; }

      /**
       * By default, a thread ID is sent with every message to identify
       * messages belonging together.
       * @returns The thread ID for this session.
       */
      const std::string& threadID() const { return m_thread; }

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
      virtual void send( const std::string& message, const std::string& subject = "" );

      /**
       * Use this function to hook a new MessageFilter into a MessageSession.
       * The filter will be able to read and/or modify a message stanza's content.
       * @note The MessageSession will become the owner of the filter, it will be
       * deleted by MessageSession's destructor. To get rid of the filter before that,
       * use disposeMessageFilter().
       * @param mf The MessageFilter to add.
       */
      void registerMessageFilter( MessageFilter *mf );

      /**
       * Use this function to remove a MessageFilter from the MessageSession.
       * @param mf The MessageFilter to remove.
       * @note To remove and delete the MessageFilter in one step use disposeMessageFilter().
       */
      void removeMessageFilter( MessageFilter *mf );

      /**
       * Use this function to remove and delete a MessageFilter from the MessageSession.
       * @param mf The MessageFilter to remove and delete.
       * @note To just remove (and not delete) the MessageFilter use removeMessageFilter().
       */
      void disposeMessageFilter( MessageFilter *mf );

      /**
       * Returns the message type this MessageSession wants to receive.
       * @return ORed list of StanzaSubType values this MessageSession wants to receive. Only the
       * StanzaMessage* types are valid.
       */
      int types() const { return m_types; }

      /**
       * Receives messages from ClientBase.
       * @param stanza The message Stanza.
       */
      virtual void handleMessage( Stanza *stanza );

    protected:
      /**
      * A wrapper around ClientBase::send(). You should @b not use this function to send a
      * chat message because the Tag is not prepared accordingly (neither Thread ID nor Message
      * Event requests are added).
      * @param tag A Tag to send.
      */
      virtual void send( Tag *tag );

    protected:
      void decorate( Tag *tag );

      ClientBase *m_parent;
      JID m_target;
      MessageHandler *m_messageHandler;

    private:
      void setResource( const std::string& resource );

      typedef std::list<MessageFilter*> MessageFilterList;

      MessageFilterList m_messageFilterList;
      std::string m_thread;
      int m_types;
      bool m_wantUpgrade;
      bool m_hadMessages;

  };

}

#endif // MESSAGESESSION_H__
