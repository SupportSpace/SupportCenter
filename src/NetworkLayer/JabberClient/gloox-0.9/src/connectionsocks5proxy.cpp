/*
  Copyright (c) 2007 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "gloox.h"

#include "connectionsocks5proxy.h"
#include "dns.h"
#include "logsink.h"
#include "prep.h"
#include "base64.h"

#include <string>

#include <string.h>

#if !defined( WIN32 ) && !defined( _WIN32_WCE )
# include <netinet/in.h>
#endif

#ifdef WIN32
# include <winsock.h>
#elif defined( _WIN32_WCE )
# include <winsock2.h>
#endif

#ifndef _WIN32_WCE
# include <sstream>
#endif

namespace gloox
{

  ConnectionSOCKS5Proxy::ConnectionSOCKS5Proxy( ConnectionBase *connection, const LogSink& logInstance,
                                                const std::string& server, int port, bool ip )
    : ConnectionBase( 0 ), m_connection( connection ),
      m_logInstance( logInstance ), m_s5state( S5StateDisconnected ), m_ip( ip )
  {
    m_server = prep::idna( server );
    m_port = port;

    if( m_connection )
      m_connection->registerConnectionDataHandler( this );
  }

  ConnectionSOCKS5Proxy::ConnectionSOCKS5Proxy( ConnectionDataHandler *cdh, ConnectionBase *connection,
                                                const LogSink& logInstance,
                                                const std::string& server, int port, bool ip )
    : ConnectionBase( cdh ), m_connection( connection ),
      m_logInstance( logInstance ), m_s5state( S5StateDisconnected ), m_ip( ip )
  {
    m_server = prep::idna( server );
    m_port = port;

    if( m_connection )
      m_connection->registerConnectionDataHandler( this );
  }

  ConnectionSOCKS5Proxy::~ConnectionSOCKS5Proxy()
  {
    if( m_connection )
      delete m_connection;
  }

  ConnectionBase* ConnectionSOCKS5Proxy::newInstance() const
  {
    ConnectionBase* conn = m_connection ? m_connection->newInstance() : 0;
    return new ConnectionSOCKS5Proxy( m_handler, conn, m_logInstance, m_server, m_port, m_ip );
  }

  void ConnectionSOCKS5Proxy::setConnectionImpl( ConnectionBase* connection )
  {
    if( m_connection )
      delete m_connection;

    m_connection = connection;
  }

  ConnectionError ConnectionSOCKS5Proxy::connect()
  {
    if( m_connection && m_handler )
    {
      m_state = StateConnecting;
      m_s5state = S5StateConnecting;
      return m_connection->connect();
    }

    return ConnNotConnected;
  }

  void ConnectionSOCKS5Proxy::disconnect()
  {
    if( m_connection )
      m_connection->disconnect();
    cleanup();
  }

  ConnectionError ConnectionSOCKS5Proxy::recv( int timeout )
  {
    if( m_connection )
      return m_connection->recv( timeout );
    else
      return ConnNotConnected;
  }

  ConnectionError ConnectionSOCKS5Proxy::receive()
  {
    if( m_connection )
      return m_connection->receive();
    else
      return ConnNotConnected;
  }

  bool ConnectionSOCKS5Proxy::send( const std::string& data )
  {
//     if( m_s5state != S5StateConnected )
//     {
//       printf( "p data sent: " );
//       const char* x = data.c_str();
//       for( unsigned int i = 0; i < data.length(); ++i )
//         printf( "%02X ", (const char)x[i] );
//       printf( "\n" );
//     }

    if( m_connection )
      return m_connection->send( data );

    return false;
  }

  void ConnectionSOCKS5Proxy::cleanup()
  {
    m_state = StateDisconnected;
    m_s5state = S5StateDisconnected;

    if( m_connection )
      m_connection->cleanup();
  }

  void ConnectionSOCKS5Proxy::getStatistics( int &totalIn, int &totalOut )
  {
    if( m_connection )
      m_connection->getStatistics( totalIn, totalOut );
    else
    {
      totalIn = 0;
      totalOut = 0;
    }
  }

  void ConnectionSOCKS5Proxy::handleReceivedData( const ConnectionBase* /*connection*/,
                                                  const std::string& data )
  {
//     if( m_s5state != S5StateConnected )
//     {
//       printf( "data recv: " );
//       const char* x = data.c_str();
//       for( unsigned int i = 0; i < data.length(); ++i )
//         printf( "%02X ", (const char)x[i] );
//       printf( "\n" );
//     }

    if( !m_connection || !m_handler )
      return;

    switch( m_s5state  )
    {
      case S5StateConnecting:
        if( data.length() != 2 || data[0] != 0x05 )
        {
          m_connection->disconnect();
          m_handler->handleDisconnect( this, ConnIoError );
        }
        if( data[1] == 0x00 ) // no auth
        {
          negotiate();
        }
        else if( data[1] == 0x02 && !m_proxyUser.empty() && !m_proxyPassword.empty() ) // user/password auth
        {
          m_logInstance.log( LogLevelDebug, LogAreaClassConnectionSOCKS5Proxy,
                             "authenticating to socks5 proxy as user " + m_proxyUser );
          m_s5state = S5StateAuthenticating;
          char* d = new char[3 + m_proxyUser.length() + m_proxyPassword.length()];
          int pos = 0;
          d[pos++] = 0x01;
          d[pos++] = m_proxyUser.length();
          strncpy( d + pos, m_proxyUser.c_str(), m_proxyUser.length() );
          pos += m_proxyUser.length();
          d[pos++] = m_proxyPassword.length();
          strncpy( d + pos, m_proxyPassword.c_str(), m_proxyPassword.length() );
          pos += m_proxyPassword.length();

          if( !send( std::string( d, pos ) ) )
          {
            cleanup();
            m_handler->handleDisconnect( this, ConnIoError );
          }
          delete[] d;
        }
        else if( data[1] == (char)0xFF && !m_proxyUser.empty() && !m_proxyPassword.empty() )
        {
          m_connection->disconnect();
          m_handler->handleDisconnect( this, ConnProxyNoSupportedAuth );
        }
        else if( data[1] == (char)0xFF && ( m_proxyUser.empty() || m_proxyPassword.empty() ) )
        {
          m_connection->disconnect();
          m_handler->handleDisconnect( this, ConnProxyAuthRequired );
        }
        else
        {
          m_connection->disconnect();
          m_handler->handleDisconnect( this, ConnProxyAuthRequired );
        }
        break;
      case S5StateNegotiating:
        if( data.length() >= 6 && data[0] == 0x05 )
        {
          if( data[1] == 0x00 )
          {
            m_state = StateConnected;
            m_s5state = S5StateConnected;
            m_handler->handleConnect( this );
          }
          else // connection refused
          {
            m_connection->disconnect();
            m_handler->handleDisconnect( this, ConnConnectionRefused );
          }
        }
        else
        {
          m_connection->disconnect();
          m_handler->handleDisconnect( this, ConnIoError );
        }
        break;
      case S5StateAuthenticating:
        if( data.length() == 2 && data[0] == 0x01 && data[1] == 0x00 )
        {
          negotiate();
        }
        else
        {
          m_connection->disconnect();
          m_handler->handleDisconnect( this, ConnProxyAuthFailed );
        }
        break;
      case S5StateConnected:
        m_handler->handleReceivedData( this, data );
        break;
      default:
        break;
    }
  }

  void ConnectionSOCKS5Proxy::negotiate()
  {
    m_s5state = S5StateNegotiating;
    char *d = new char[m_ip ? 10 : 6 + m_server.length() + 1];
    int pos = 0;
    d[pos++] = 0x05; // SOCKS version 5
    d[pos++] = 0x01; // command CONNECT
    d[pos++] = 0x00; // reserved
    int port = m_port;
    std::string server = m_server;
    if( m_ip ) // IP address
    {
      d[pos++] = 0x01; // IPv4 address
      std::string s;
      int j = server.length();
      int l = 0;
      for( int k = 0; k < j && l < 4; ++k )
      {
        if( server[k] != '.' )
          s += server[k];

        if( server[k] == '.' || k == j-1 )
        {
          d[pos++] = atoi( s.c_str() ) & 0x0FF;
          s = "";
          ++l;
        }
      }
    }
    else // hostname
    {
      if( port == -1 )
      {
        DNS::HostMap servers = DNS::resolve( m_server, m_logInstance );
        if( servers.size() )
        {
          server = (*(servers.begin())).first;
          port = (*(servers.begin())).second;
        }
      }
      d[pos++] = 0x03; // hostname
      d[pos++] = m_server.length();
      strncpy( d + pos, m_server.c_str(), m_server.length() );
      pos += m_server.length();
    }
    int nport = htons( port );
    d[pos++] = nport;
    d[pos++] = nport >> 8;

#ifndef _WIN32_WCE
    std::ostringstream oss;
    oss << "requesting socks5 proxy connection to " << server << ":" << port;
    m_logInstance.log( LogLevelDebug, LogAreaClassConnectionSOCKS5Proxy, oss.str() );
#endif

    if( !send( std::string( d, pos ) ) )
    {
      cleanup();
      m_handler->handleDisconnect( this, ConnIoError );
    }
    delete[] d;
  }

  void ConnectionSOCKS5Proxy::handleConnect( const ConnectionBase* /*connection*/ )
  {
    if( m_connection )
    {
      std::string server = m_server;
      int port = m_port;
      if( port == -1 )
      {
        DNS::HostMap servers = DNS::resolve( m_server, m_logInstance );
        if( servers.size() )
        {
          server = (*(servers.begin())).first;
          port = (*(servers.begin())).second;
        }
      }
      m_logInstance.log( LogLevelDebug, LogAreaClassConnectionSOCKS5Proxy,
                         "attempting to negotiate socks5 proxy connection" );

      bool auth = !m_proxyUser.empty() && !m_proxyPassword.empty();
      char *d = new char[auth ? 4 : 3];
      d[0] = 0x05; // SOCKS version 5
      if( auth )
      {
        d[1] = 0x02; // two methods
        d[3] = 0x02; // method: username/password auth
      }
      else
        d[1] = 0x01; // one method
      d[2] = 0x00; // method: no auth

      if( !send( std::string( d, auth ? 4 : 3 ) ) )
      {
        cleanup();
        if( m_handler )
          m_handler->handleDisconnect( this, ConnIoError );
      }
      delete[] d;
    }
  }

  void ConnectionSOCKS5Proxy::handleDisconnect( const ConnectionBase* /*connection*/,
                                                ConnectionError reason )
  {
    cleanup();
    m_logInstance.log( LogLevelDebug, LogAreaClassConnectionSOCKS5Proxy, "socks5 proxy connection closed" );

    if( m_handler )
      m_handler->handleDisconnect( this, reason );
  }

}
