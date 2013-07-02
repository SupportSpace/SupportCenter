/*
  Copyright (c) 2004-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef CONNECTION_H__
#define CONNECTION_H__

#ifdef WIN32
# include "../config.h.win"
#else
# include "config.h"
#endif

#include "gloox.h"
#include "logsink.h"

#include <string>

#if defined( HAVE_OPENSSL )
# define USE_OPENSSL
# include <openssl/ssl.h>
# define HAVE_TLS
#elif defined( HAVE_GNUTLS )
# define USE_GNUTLS
# include <gnutls/gnutls.h>
# include <gnutls/x509.h>
# define HAVE_TLS
#elif defined( HAVE_WINTLS )
# define USE_WINTLS
# define SECURITY_WIN32
# include <windows.h>
# include <security.h>
# include <sspi.h>
# define HAVE_TLS
#endif

namespace gloox
{

  class Compression;
  class Packet;
  class Parser;

  /**
   * @brief This is an implementation of a TLS- and Stream Compression-aware connection handler.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.4
   */
  class GLOOX_API Connection
  {
    public:
      /**
       * Constructs a new Connection object.
       * You should not need to use this function directly.
       * @param parser A parser to feed with incoming data.
       * @param logInstance A LogSink to use for logging.
       * @param server A server to connect to.
       * @param port The port to connect to. The default of -1 means that SRV records will be used
       * to find out about the actual host:port.
       */
      Connection( Parser *parser, const LogSink& logInstance, const std::string& server,
                  unsigned short port = -1 );

      /**
       * Virtual destructor
       */
      virtual ~Connection();

      /**
       * Used to initiate the connection.
       * @return Returns the connection state.
       */
      ConnectionState connect();

      /**
       * Use this periodically to receive data from the socket and to feed the parser.
       * @param timeout The timeout to use for select in microseconds. Default of -1 means blocking.
       * @return The state of the connection.
       */
      ConnectionError recv( int timeout = -1 );

      /**
       * Use this function to send a string of data over the wire. The function returns only after
       * all data has been sent.
       * @param data The data to send.
       */
      bool send( const std::string& data );

      /**
       * Use this function to put the connection into 'receive mode'.
       * @return Returns a value indicating the disconnection reason.
       */
      ConnectionError receive();

      /**
       * Disconnects an established connection. NOOP if no active connection exists.
       * @param e A ConnectionError decribing why the connection is terminated. Well, its not really an
       * error here, but...
       */
      void disconnect( ConnectionError e );

      /**
       * Use this function to determine whether an esatblished connection is encrypted.
       * @return @b True if the connection is encrypted, @b false otherwise.
       */
      bool isSecure() const { return m_secure; };

      /**
       * Returns the current connection state.
       * @return The state of the connection.
       */
      ConnectionState state() const { return m_state; };

      /**
       * Gives access to the raw file descriptor of a connection. Use it wisely. Especially, you should not
       * ::recv() any data from it. There is no way to feed that back into the parser. You can
       * select()/poll() it and use Connection::recv( -1 ) to fetch the data.
       * @return The file descriptor of the active connection, or -1 if no connection is established.
       */
      int fileDescriptor();

#ifdef HAVE_ZLIB
      /**
       * This function is used to init or de-init stream compression. You must
       * call this before enabling compression using setCompression().
       * @param method The desired stream compression method (e.g. zlib, lzw, ...)
       * @return Returns @b true if compression was successfully initialized/de-initialized,
       * @b false otherwise.
       */
      bool initCompression( StreamFeature method );

      /**
       * This function is used to enable stream compression as defined in JEP-0138.
       * It is necessary because when compression is negotiated it is not enabled instantly.
       */
      void enableCompression();
#endif

#ifdef HAVE_TLS
      /**
       * Call this function to start a TLS handshake over an established connection.
       */
      bool tlsHandshake();

      /**
       * Use this function to set a number of trusted root CA certificates which shall be
       * used to verify a servers certificate.
       * @param cacerts A list of absolute paths to CA root certificate files in PEM format.
       */
      void setCACerts( const StringList& cacerts ) { m_cacerts = cacerts; };

      /**
       * This function is used to retrieve certificate and connection info of a encrypted connection.
       * @return Certificate information.
       */
      const CertInfo& fetchTLSInfo() const { return m_certInfo; };

      /**
       * Use this function to set the user's certificate and private key. The certificate will
       * be presented to the server upon request and can be used for SASL EXTERNAL authentication.
       * The user's certificate file should be a bundle of more than one certificate in PEM format.
       * The first one in the file should be the user's certificate, each cert following that one
       * should have signed the previous one.
       * @note These certificates are not necessarily the same as those used to verify the server's
       * certificate.
       * @param clientKey The absolute path to the user's private key in PEM format.
       * @param clientCerts A path to a certificate bundle in PEM format.
       */
      void setClientCert( const std::string& clientKey, const std::string& clientCerts );
#endif

    private:
      Connection &operator = ( const Connection & );
      bool dataAvailable( int timeout = -1 );

      void cancel();
      void cleanup();

#ifdef HAVE_TLS
      bool tls_send( const void *data, size_t len );
      int tls_recv( void *data, size_t len );
      bool tls_dataAvailable();
      void tls_cleanup();
#endif

#if defined( USE_GNUTLS )
      bool verifyAgainstCAs( gnutls_x509_crt_t cert, gnutls_x509_crt_t *CAList, int CAListSize );
      bool verifyAgainst( gnutls_x509_crt_t cert, gnutls_x509_crt_t issuer );

      gnutls_session_t m_session;
      gnutls_certificate_credentials m_credentials;

#elif defined( USE_OPENSSL )
      SSL *m_ssl;
#elif defined( USE_WINTLS )
      bool handshakeLoop();

      SecurityFunctionTableA *m_securityFunc;
      CredHandle m_credentials;
      CtxtHandle m_context;
      SecBufferDesc m_imessage;
      SecBufferDesc m_omessage;
      SecBuffer m_ibuffers[4];
      SecBuffer m_obuffers[4];
      SecPkgContext_StreamSizes m_streamSizes;
      HMODULE m_lib;

      char *m_messageOffset;
      char *m_iBuffer;
      char *m_oBuffer;
      int m_bufferSize;
      int m_bufferOffset;
      int m_sspiFlags;
#endif

      StringList m_cacerts;
      std::string m_clientKey;
      std::string m_clientCerts;

      Parser *m_parser;
      ConnectionState m_state;
      CertInfo m_certInfo;
      ConnectionError m_disconnect;
      const LogSink& m_logInstance;
      Compression *m_compression;

      char *m_buf;
      std::string m_server;
      unsigned short m_port;
      int m_socket;
      const int m_bufsize;
      bool m_cancel;
      bool m_secure;
      bool m_fdRequested;
      bool m_enableCompression;
  };

}

#endif // CONNECTION_H__
