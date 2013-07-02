/*
  Copyright (c) 2005-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef FLEXOFFHANDLER_H__
#define FLEXOFFHANDLER_H__

namespace gloox
{

  /**
   * @brief Implementation of this virtual interface allows for retrieval of offline messages following
   * JEP-0030.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.7
   */
  class GLOOX_API FlexibleOfflineHandler
  {
    public:
      /**
       * Describes the possible results of a message retrieval or deletion request.
       */
      enum FlexibleOfflineResult
      {
        FOMR_REMOVE_SUCCESS,        /**< Message(s) were removed successfully. */
        FOMR_REQUEST_SUCCESS,       /**< Message(s) were fetched successfully. */
        FOMR_FORBIDDEN,             /**< The requester is a JID other than an authorized resource of the
                                     * user. Something wnet serieously wrong */
        FOMR_ITEM_NOT_FOUND,        /**< The requested node (message ID) does not exist. */
        FOMR_UNKNOWN_ERROR          /**< An error occurred which is not specified in JEP-0013. */
      };

      /**
       * Virtual Destructor.
       */
      virtual ~FlexibleOfflineHandler() {};

      /**
       * This function is called to indicate whether the server supports JEP-0013 or not.
       * Call @ref FlexibleOffline::checkSupport() to trigger the check.
       * @param support Whether the server support JEP-0013 or not.
       */
      virtual void handleFlexibleOfflineSupport( bool support ) = 0;

      /**
       * This function is called to announce the number of available offline messages.
       * Call @ref FlexibleOffline::getMsgCount() to trigger the check.
       * @param num The number of stored offline messages.
       */
      virtual void handleFlexibleOfflineMsgNum( int num ) = 0;

      /**
       * This function is called when the offline message headers arrive.
       * Call @ref FlexibleOffline::fetchHeaders() to trigger the check.
       * @param headers A map of ID/sender pairs describing the offline messages.
       */
      virtual void handleFlexibleOfflineMessageHeaders( StringMap& headers ) = 0;

      /**
       * This function is called to indicate the result of a fetch or delete instruction.
       * @param result The result of the operation.
       */
      virtual void handleFlexibleOfflineResult( FlexibleOfflineResult result ) = 0;
  };

}

#endif // FLEXOFFHANDLER_H__
