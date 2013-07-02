/*
 * Copyright (C) 2004, 2005 Free Software Foundation
 *
 * Author: Simon Josefsson
 *
 * This file is part of GNUTLS.
 *
 * GNUTLS is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNUTLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNUTLS; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* Parts copied from GnuTLS example programs. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <gnutls/gnutls.h>
#include <gnutls/extra.h>

#include "utils.h"

#include <readline.h>

/* A very basic TLS client, with anonymous authentication.
 */

#define MAX_BUF 1024
#define MSG "Hello TLS"

/* Connects to the peer and returns a socket
 * descriptor.
 */
int
tcp_connect (void)
{
  const char *PORT = "5556";
  const char *SERVER = "127.0.0.1";
  int err, sd;
  struct sockaddr_in sa;

  /* connects to server
   */
  sd = socket (AF_INET, SOCK_STREAM, 0);

  memset (&sa, '\0', sizeof (sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons (atoi (PORT));
  inet_pton (AF_INET, SERVER, &sa.sin_addr);

  err = connect (sd, (struct sockaddr *) &sa, sizeof (sa));
  if (err < 0)
    {
      fprintf (stderr, "Connect error\n");
      exit (1);
    }

  return sd;
}

/* closes the given socket descriptor.
 */
void
tcp_close (int sd)
{
  shutdown (sd, SHUT_RDWR);	/* no more receptions */
  close (sd);
}

int
client_avp (gnutls_session_t session, void *ptr,
	    const char *last, size_t lastlen, char **new, size_t * newlen)
{
  static int iter = 0;
  char *p;

  if (last)
    printf ("client: received %d bytes AVP: `%.*s'\n",
	    lastlen, lastlen, last);
  else
    printf ("client: new application phase\n");

  switch (iter)
    {
    case 0:
      p = "client's first AVP, next will be empty";
      break;

    case 1:
      p = "";
      break;

    case 2:
      p = "client avp";
      break;

    default:
      p = "final client AVP, we'll restart next";
      iter = -1;
      break;
    }

  iter++;

  if (debug)
    p = readline ("Client TLS/IA AVP: ");

  *new = gnutls_strdup (p);
  if (!*new)
    return -1;
  *newlen = strlen (*new);

  printf ("client: sending %d bytes AVP: `%s'\n", *newlen, *new);

  gnutls_ia_permute_inner_secret (session, 3, "foo");

  return 0;
}

void
client (void)
{
  int ret, sd, ii;
  gnutls_session_t session;
  char buffer[MAX_BUF + 1];
  gnutls_anon_client_credentials_t anoncred;
  gnutls_ia_client_credentials_t iacred;
  /* Need to enable anonymous KX specifically. */
  const int kx_prio[] = { GNUTLS_KX_ANON_DH, 0 };

  gnutls_global_init ();
  gnutls_global_init_extra ();

  gnutls_anon_allocate_client_credentials (&anoncred);
  gnutls_ia_allocate_client_credentials (&iacred);

  /* Initialize TLS session
   */
  gnutls_init (&session, GNUTLS_CLIENT);

  /* Use default priorities */
  gnutls_set_default_priority (session);
  gnutls_kx_set_priority (session, kx_prio);

  /* put the anonymous credentials to the current session
   */
  gnutls_credentials_set (session, GNUTLS_CRD_ANON, anoncred);
  gnutls_credentials_set (session, GNUTLS_CRD_IA, iacred);

  /* connect to the peer
   */
  sd = tcp_connect ();

  gnutls_transport_set_ptr (session, (gnutls_transport_ptr_t) sd);

  /* Enable TLS/IA. */
  gnutls_ia_set_client_avp_function (iacred, client_avp);

  /* Perform the TLS handshake
   */
  ret = gnutls_handshake (session);

  if (ret < 0)
    {
      fail ("client: Handshake failed\n");
      gnutls_perror (ret);
      goto end;
    }
  else
    {
      success ("client: Handshake was completed\n");
    }

  /*
     To test TLS/IA alert's (the server will print that a fatal alert
     was received):
     gnutls_alert_send(session, GNUTLS_AL_FATAL,
     GNUTLS_A_INNER_APPLICATION_FAILURE);
   */

  if (!gnutls_ia_handshake_p (session))
    fail ("client: No TLS/IA negotiation\n");
  else
    {
      success ("client: TLS/IA handshake\n");

      ret = gnutls_ia_handshake (session);

      if (ret < 0)
	{
	  fail ("client: TLS/IA handshake failed\n");
	  gnutls_perror (ret);
	  goto end;
	}
      else
	{
	  success ("client: TLS/IA Handshake was completed\n");
	}
    }

  gnutls_record_send (session, MSG, strlen (MSG));

  ret = gnutls_record_recv (session, buffer, MAX_BUF);
  if (ret == 0)
    {
      success ("client: Peer has closed the TLS connection\n");
      goto end;
    }
  else if (ret < 0)
    {
      fail ("client: Error: %s\n", gnutls_strerror (ret));
      goto end;
    }

  if (debug)
    {
      printf ("- Received %d bytes: ", ret);
      for (ii = 0; ii < ret; ii++)
	{
	  fputc (buffer[ii], stdout);
	}
      fputs ("\n", stdout);
    }

  gnutls_bye (session, GNUTLS_SHUT_RDWR);

end:

  tcp_close (sd);

  gnutls_deinit (session);

  gnutls_ia_free_client_credentials (iacred);

  gnutls_anon_free_client_credentials (anoncred);

  gnutls_global_deinit ();
}

/* This is a sample TLS 1.0 echo server, for anonymous authentication only.
 */

#define SA struct sockaddr
#define MAX_BUF 1024
#define PORT 5556		/* listen to 5556 port */
#define DH_BITS 1024

/* These are global */
gnutls_anon_server_credentials_t anoncred;
gnutls_ia_server_credentials_t iacred;

gnutls_session_t
initialize_tls_session (void)
{
  gnutls_session_t session;
  const int kx_prio[] = { GNUTLS_KX_ANON_DH, 0 };

  gnutls_init (&session, GNUTLS_SERVER);

  /* avoid calling all the priority functions, since the defaults
   * are adequate.
   */
  gnutls_set_default_priority (session);
  gnutls_kx_set_priority (session, kx_prio);

  gnutls_credentials_set (session, GNUTLS_CRD_ANON, anoncred);

  gnutls_dh_set_prime_bits (session, DH_BITS);

  return session;
}

static gnutls_dh_params_t dh_params;

static int
generate_dh_params (void)
{

  /* Generate Diffie Hellman parameters - for use with DHE
   * kx algorithms. These should be discarded and regenerated
   * once a day, once a week or once a month. Depending on the
   * security requirements.
   */
  gnutls_dh_params_init (&dh_params);
  gnutls_dh_params_generate2 (dh_params, DH_BITS);

  return 0;
}

int err, listen_sd, i;
int sd, ret;
struct sockaddr_in sa_serv;
struct sockaddr_in sa_cli;
socklen_t client_len;
char topbuf[512];
gnutls_session_t session;
char buffer[MAX_BUF + 1];
int optval = 1;

int
server_avp (gnutls_session_t session, void *ptr,
	    const char *last, size_t lastlen, char **new, size_t * newlen)
{
  static int iter = 0;
  char *p;

  if (last)
    printf ("server: received %d bytes AVP: `%.*s'\n",
	    lastlen, lastlen, last);

  gnutls_ia_permute_inner_secret (session, 3, "foo");

  switch (iter)
    {
    case 0:
      p = "first server AVP";
      break;

    case 1:
      p = "second server AVP, next will be empty, then a intermediate finish";
      break;

    case 2:
      p = "";
      break;

    case 3:
      p = "1";
      break;

    case 4:
      p = "server avp, after intermediate finish, next another intermediate";
      break;

    case 5:
      p = "1";
      break;

    case 6:
      p = "server avp, next will be the finish phase";
      break;

    default:
      p = "2";
      break;
    }

  iter++;

  if (debug)
    p = readline ("Server TLS/IA AVP (type '1' to sync, '2' to finish): ");

  if (!p)
    return -1;

  if (strcmp (p, "1") == 0)
    {
      success ("server: Sending IntermediatePhaseFinished...\n");
      return 1;
    }

  if (strcmp (p, "2") == 0)
    {
      success ("server: Sending FinalPhaseFinished...\n");
      return 2;
    }

  *new = gnutls_strdup (p);
  if (!*new)
    return -1;
  *newlen = strlen (*new);

  printf ("server: sending %d bytes AVP: `%s'\n", *newlen, *new);

  return 0;
}

void
server_start (void)
{
  /* this must be called once in the program
   */
  gnutls_global_init ();

  gnutls_anon_allocate_server_credentials (&anoncred);
  gnutls_ia_allocate_server_credentials (&iacred);

  success ("Launched, generating DH parameters...\n");

  generate_dh_params ();

  gnutls_anon_set_server_dh_params (anoncred, dh_params);

  /* Socket operations
   */
  listen_sd = socket (AF_INET, SOCK_STREAM, 0);
  if (err == -1)
    {
      perror ("socket");
      fail ("server: socket failed\n");
      return;
    }

  memset (&sa_serv, '\0', sizeof (sa_serv));
  sa_serv.sin_family = AF_INET;
  sa_serv.sin_addr.s_addr = INADDR_ANY;
  sa_serv.sin_port = htons (PORT);	/* Server Port number */

  setsockopt (listen_sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (int));

  err = bind (listen_sd, (SA *) & sa_serv, sizeof (sa_serv));
  if (err == -1)
    {
      perror ("bind");
      fail ("server: bind failed\n");
      return;
    }

  err = listen (listen_sd, 1024);
  if (err == -1)
    {
      perror ("listen");
      fail ("server: listen failed\n");
      return;
    }

  success ("server: ready. Listening to port '%d'\n", PORT);
}

void
server (void)
{
  client_len = sizeof (sa_cli);

  session = initialize_tls_session ();

  sd = accept (listen_sd, (SA *) & sa_cli, &client_len);

  success ("server: connection from %s, port %d\n",
	   inet_ntop (AF_INET, &sa_cli.sin_addr, topbuf,
		      sizeof (topbuf)), ntohs (sa_cli.sin_port));

  gnutls_transport_set_ptr (session, (gnutls_transport_ptr_t) sd);

  /* Enable TLS/IA. */
  gnutls_credentials_set (session, GNUTLS_CRD_IA, iacred);
  gnutls_ia_set_server_avp_function (iacred, server_avp);

  ret = gnutls_handshake (session);
  if (ret < 0)
    {
      close (sd);
      gnutls_deinit (session);
      fail ("server: Handshake has failed (%s)\n\n", gnutls_strerror (ret));
      return;
    }
  success ("server: Handshake was completed\n");

  if (!gnutls_ia_handshake_p (session))
    fail ("server: No TLS/IA negotiation\n");
  else
    {
      success ("server: TLS/IA handshake\n");

      ret = gnutls_ia_handshake (session);

      if (ret < 0)
	{
	  fail ("server: TLS/IA handshake failed\n");
	  gnutls_perror (ret);
	  return;
	}
      else
	{
	  success ("server: TLS/IA Handshake was completed\n");
	}
    }

  /* see the Getting peer's information example */
  /* print_info(session); */

  i = 0;
  for (;;)
    {
      bzero (buffer, MAX_BUF + 1);
      ret = gnutls_record_recv (session, buffer, MAX_BUF);

      if (ret == 0)
	{
	  success ("server: Peer has closed the GNUTLS connection\n");
	  break;
	}
      else if (ret < 0)
	{
	  if (ret == GNUTLS_E_FATAL_ALERT_RECEIVED)
	    {
	      gnutls_alert_description_t alert;
	      const char *err;
	      alert = gnutls_alert_get (session);
	      err = gnutls_alert_get_name (alert);
	      if (err)
		printf ("Fatal alert: %s\n", err);
	    }

	  fail ("server: Received corrupted data(%d). Closing...\n", ret);
	  break;
	}
      else if (ret > 0)
	{
	  /* echo data back to the client
	   */
	  gnutls_record_send (session, buffer, strlen (buffer));
	}
    }
  /* do not wait for the peer to close the connection.
   */
  gnutls_bye (session, GNUTLS_SHUT_WR);

  close (sd);
  gnutls_deinit (session);

  close (listen_sd);

  gnutls_ia_free_server_credentials (iacred);

  gnutls_anon_free_server_credentials (anoncred);

  gnutls_global_deinit ();

  success ("server: finished\n");
}

void
doit (void)
{
  pid_t child;

  server_start ();
  if (error_count)
    return;

  child = fork ();
  if (child < 0)
    {
      perror ("fork");
      fail ("fork");
      return;
    }

  if (child)
    {
      int status;
      /* parent */
      server ();
      wait (&status);
    }
  else
    client ();
}
