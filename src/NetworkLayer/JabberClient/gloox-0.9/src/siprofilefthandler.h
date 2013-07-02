/*
  Copyright (c) 2007 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SIPROFILEFTHANDLER_H__
#define SIPROFILEFTHANDLER_H__

#include "jid.h"

#include <string>

namespace gloox
{

  class JID;

  /**
   * @brief An abstract base class to handle file transfer (FT) requests.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API SIProfileFTHandler
  {

    public:
      /**
       * Virtual destructor.
       */
      virtual ~SIProfileFTHandler() {}

      /**
       * This function is called to handle incoming file transfer requests, i.e. a remote entity requested
       * to send a file to you. You should use either SIProfileFT::acceptFT() or
       * SIProfileFT::declineFT() to accept or reject the request, respectively.
       * @param from The file transfer requestor.
       * @param id The request's id. This id MUST be supplied to either SIProfileFT::acceptFT() or
       * SIProfileFT::declineFT().
       * @param name The file name.
       * @param size The file size.
       * @param hash The file content's MD5 sum.
       * @param date The file's last modification time.
       * @param mimetype The file's mime-type.
       * @param desc The file's description.
       * @param stypes An ORed list of @link gloox::SIProfileFT::StreamType SIProfileFT::StreamType @endlink
       * indicating the StreamTypes the initiator supports.
       * @param offset The offset in bytes from which the file should be transmitted.
       * @param length The number of bytes to send, starting from the given offset. A value of -1
       * indicates that the entire file is to be transmitted (taking the offset into account).
       */
      virtual void handleFTRequest( const JID& from, const std::string& id, const std::string& name,
                                    long size, const std::string& hash,
                                    const std::string& date, const std::string& mimetype,
                                    const std::string& desc, int stypes, long offset, long length ) = 0;

      /**
       * This function is called to handle results of outgoing file transfer requests,
       * i.e. you requested a stream (using SIProfileFT::requestFT()) to send a file
       * to a remote entity.
       * @param from The file transfer receiver.
       * @param sid The stream ID.
       */
//       virtual void handleFTRequestResult( const JID& from, const std::string& sid ) = 0;

      /**
       * This function is called to handle a request error or decline.
       * @param stanza The complete error stanza.
       */
      virtual void handleFTRequestError( Stanza* stanza ) = 0;

      /**
       * This function is called to pass a negotiated SOCKS5 bytestream.
       * The bytestream is not yet open and not ready to send/receive data.
       * @note To initialize the bytestream and to prepare it for data transfer
       * do the following, preferable in that order:
       * @li register a SOCKS5BytestreamDataHandler with the SOCKS5Bytestream,
       * @li set up a separate thread for the bytestream or integrate it into
       * your main loop,
       * @li call its connect() method and check the return value.
       * To not block your application while the data transfer and/or the connection
       * attempts last, you most likely want to put  the bytestream into its own
       * thread or process (before calling connect() on it). It is safe to do so
       * without additional synchronization.
       * @param s5b The SOCKS5 bytestream.
       */
      virtual void handleFTSOCKS5Bytestream( SOCKS5Bytestream* s5b ) = 0;
  };

}

#endif // SIPROFILEFTHANDLER_H__
