/*
  Copyright (c) 2005-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef REGISTRATIONHANDLER_H__
#define REGISTRATIONHANDLER_H__

#include <string>

namespace gloox
{

  class DataForm;

  /**
   * @brief A virtual interface that receives events from an @ref Registration object.
   *
   * Derived classes can be registered as RegistrationHandlers with an
   * Registration object. Incoming results for operations initiated through
   * the Registration object are forwarded to this handler.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.2
   */
  class GLOOX_API RegistrationHandler
  {
    public:
      /**
       * Possible results of a JEP-0077 operation.
       */
      enum resultEnum
      {
        REGISTRATION_SUCCESS = 0,             /*!< The last operation (account registration, account
                                               * deletion or password change) was successful. */
        REGISTRATION_NOT_ACCEPTABLE,          /*!< 406: Not all necessary information provided */
        REGISTRATION_CONFLICT,                /*!< 409: Username alreday exists. */
        REGISTRATION_NOT_AUTHORIZED,          /*!< Account removal: Unregistered entity waits too long
                                               * before authentication or performs tasks other than
                                               * authentication after registration.<br>
                                               * Password change: The server or service does not consider
                                               * the channel safe enough to enable a password change. */
        REGISTRATION_BAD_REQUEST,             /*!< Account removal: The &lt;remove/&gt; element was not the
                                               * only child element of the &lt;query/&gt; element. Should not
                                               * happen when only gloox functions are being used.<br>
                                               * Password change: The password change request does not
                                               * contain complete information (both &lt;username/&gt; and
                                               * &lt;password/&gt; are required). */
        REGISTRATION_FORBIDDEN,               /*!< Account removal: The sender does not have sufficient
                                               * permissions to cancel the registration. */
        REGISTRATION_REGISTRATION_REQUIRED,   /*!< Account removal: The entity sending the remove request was
                                               * not previously registered. */
        REGISTRATION_UNEXPECTED_REQUEST,      /*!< Account removal: The host is an instant messaging server
                                               * and the IQ get does not contain a 'from' address because the
                                               * entity is not registered with the server.<br>
                                               * Password change: The host is an instant messaging server and
                                               * the IQ set does not contain a 'from' address because the
                                               * entity is not registered with the server. */
        REGISTRATION_NOT_ALLOWED,             /*!< Password change: The server or service does not allow
                                               * password changes. */
        UNKNOWN_ERROR                         /**< An unknown error condition occured. */
      };

      /**
       * Virtual Destructor.
       */
      virtual ~RegistrationHandler() {};

      /**
       * Reimplement this function to receive results of the @ref Registration::fetchRegistrationFields()
       * function.
       * @param fields The OR'ed fields the server requires. From @ref Registration::fieldEnum.
       * @param instructions Any additional information the server sends along.
       */
      virtual void handleRegistrationFields( int fields, std::string instructions ) = 0;

      /**
       * This function is called if @ref Registration::createAccount() was called on an authenticated
       * stream and the server lets us know about this.
       */
      virtual void handleAlreadyRegistered() = 0;

      /**
       * This funtion is called to notify about the result of an operation.
       * @param result The result of the last operation.
       */
      virtual void handleRegistrationResult( resultEnum result ) = 0;

      /**
       * This function is called additionally to @ref handleRegistrationFields() if the server
       * supplied a data form together with legacy registration fields.
       * @param form The DataForm conataining registration information.
       */
      virtual void handleDataForm( const DataForm &form ) = 0;

      /**
       * This function is called if the server does not offer in-band registration
       * but wants to refer the user to an external URL.
       * @param url The external URL where registration is possible (or where more information
       * can be found).
       * @param desc Some descriptive text.
       */
      virtual void handleOOB( const std::string& url, const std::string& desc ) = 0;

  };

}

#endif // REGISTRATIONHANDLER_H__
