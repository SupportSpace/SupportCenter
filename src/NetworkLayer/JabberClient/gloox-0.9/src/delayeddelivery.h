/*
  Copyright (c) 2006-2007 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef DELAYEDDELIVERY_H__
#define DELAYEDDELIVERY_H__

#include "stanzaextension.h"
#include "jid.h"

#include <string>

namespace gloox
{

  class Tag;

  /**
   * @brief This is an implementation of XEP-0203 (Delayed Delivery).
   *
   * XEP Version: 0.1
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class DelayedDelivery : public StanzaExtension
  {

    public:
      /**
       * Constructs a new object and fills it according to the parameters.
       * @param from The JID of the original sender or the entity that delayed the sending.
       * @param stamp The datetime stamp of the original send.
       * @param reason A natural language reason for the delay.
       */
      DelayedDelivery( const JID& from, const std::string stamp, const std::string& reason );

      /**
       * Constructs a new object from the given Tag.
       * @param tag The Tag to parse.
       */
      DelayedDelivery( Tag *tag );

      /**
       * Virtual Destructor.
       */
      virtual ~DelayedDelivery();

      /**
       * Returns the datetime when the stanza was originally sent.
       * The format MUST adhere to the dateTime format specified in XEP-0082 and MUST
       * be expressed in UTC.
       * @return The original datetime.
       */
      const std::string& stamp() const { return m_stamp; }

      /**
       * Sets the original datetime.
       * @param stamp The original datetime.
       */
      void setStamp( const std::string& stamp ) { m_stamp = stamp; }

      /**
       * Returns the JID of the original sender of the stanza or the entity that
       * delayed the sending.
       * The format MUST adhere to the dateTime format specified in XEP-0082 and MUST
       * be expressed in UTC.
       * @return The JID.
       */
      const JID& from() const { return m_from; }

      /**
       * Sets the JID of the origianl sender or the entity that delayed the sending.
       * @param from The JID.
       */
      void setFrom( const JID& from ) { m_from = from; }

      /**
       * Returns a natural language reason fro the delay.
       * @return A natural language reason fro the delay.
       */
      const std::string& reason() const { return m_reason; }

      /**
       * Sets the reason for the delay.
       * @param reason The reason for the delay.
       */
      void setBody( const std::string& reason ) { m_reason = reason; }

      // reimplemented from StanzaExtension
      virtual Tag* tag() const;

    private:
      JID m_from;
      std::string m_stamp;
      std::string m_reason;
      bool m_valid;
  };

}

#endif // DELAYEDDELIVERY_H__
