/*
 * Copyright (C) 2004, 2005, 2006 Free Software Foundation
 * Copyright (C) 2000,2001,2002,2003 Nikos Mavroyanopoulos
 *
 * This file is part of GNUTLS.
 *
 * GNUTLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNUTLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <config.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <gnutls/gnutls.h>
#include <gnutls/extra.h>
#include <gnutls/x509.h>
#include <gnutls/openpgp.h>

#include "common.h"
#include "cli-gaa.h"

#if defined _WIN32 || defined __WIN32__
#define select _win_select
#endif

#ifndef SHUT_WR
# define SHUT_WR 1
#endif

#ifndef SHUT_RDWR
# define SHUT_RDWR 2
#endif

#define SA struct sockaddr
#define ERR(err,s) do { if (err==-1) {perror(s);return(1);} } while (0)
#define MAX_BUF 4096

/* global stuff here */
int resume, starttls, insecure;
char *hostname = NULL;
char *service;
int record_max_size;
int fingerprint;
int crlf;
int verbose = 0;
extern int xml;
extern int print_cert;

char *srp_passwd = NULL;
char *srp_username;
char *pgp_keyfile;
char *pgp_certfile;
char *pgp_keyring;
char *pgp_trustdb;
char *x509_keyfile;
char *x509_certfile;
char *x509_cafile;
char *x509_crlfile = NULL;
static int x509ctype;
static int disable_extensions;
static int debug;

char *psk_username = NULL;
gnutls_datum psk_key = { NULL, 0 };

static gnutls_srp_client_credentials_t srp_cred;
static gnutls_psk_client_credentials_t psk_cred;
static gnutls_anon_client_credentials_t anon_cred;
static gnutls_certificate_credentials_t xcred;

int protocol_priority[PRI_MAX] =
  { GNUTLS_TLS1_1, GNUTLS_TLS1_0, GNUTLS_SSL3, 0 };
int kx_priority[PRI_MAX] =
  { GNUTLS_KX_DHE_RSA, GNUTLS_KX_DHE_DSS, GNUTLS_KX_RSA,
  GNUTLS_KX_SRP_RSA, GNUTLS_KX_SRP_DSS, GNUTLS_KX_SRP, GNUTLS_KX_PSK,
  /* Do not use anonymous authentication, unless you know what that means */
  GNUTLS_KX_RSA_EXPORT, GNUTLS_KX_ANON_DH, 0
};
int cipher_priority[PRI_MAX] =
  { GNUTLS_CIPHER_AES_256_CBC, GNUTLS_CIPHER_AES_128_CBC,
  GNUTLS_CIPHER_3DES_CBC, GNUTLS_CIPHER_ARCFOUR_128,
  GNUTLS_CIPHER_ARCFOUR_40, 0
};
int comp_priority[PRI_MAX] = { GNUTLS_COMP_ZLIB, GNUTLS_COMP_NULL, 0 };
int mac_priority[PRI_MAX] =
  { GNUTLS_MAC_SHA1, GNUTLS_MAC_MD5, GNUTLS_MAC_RMD160, 0 };
int cert_type_priority[PRI_MAX] = { GNUTLS_CRT_X509, GNUTLS_CRT_OPENPGP, 0 };

/* end of global stuff */

/* prototypes */
typedef struct
{
  int fd;
  gnutls_session session;
  int secure;
  char *hostname;
  char *ip;
  char* service;
  struct addrinfo *ptr;
  struct addrinfo *addr_info;
} socket_st;

ssize_t socket_recv (const socket_st *socket, void *buffer, int buffer_size);
ssize_t socket_send (const socket_st *socket, const void *buffer, int buffer_size);
void socket_open( socket_st* hd, const char* hostname, const char* service);
void socket_connect( const socket_st* hd);
void socket_bye (socket_st * socket);

static void check_rehandshake (socket_st *socket, int ret);
static int do_handshake (socket_st * socket);
static void init_global_tls_stuff (void);


#undef MAX
#define MAX(X,Y) (X >= Y ? X : Y);


/* Helper functions to load a certificate and key
 * files into memory.
 */
static gnutls_datum
load_file (const char *file)
{
  FILE *f;
  gnutls_datum loaded_file = { NULL, 0 };
  long filelen;
  void *ptr;

  if (!(f = fopen (file, "r"))
      || fseek (f, 0, SEEK_END) != 0
      || (filelen = ftell (f)) < 0
      || fseek (f, 0, SEEK_SET) != 0
      || !(ptr = malloc ((size_t) filelen))
      || fread (ptr, 1, (size_t) filelen, f) < (size_t) filelen)
    {
      return loaded_file;
    }

  loaded_file.data = ptr;
  loaded_file.size = (unsigned int) filelen;
  return loaded_file;
}

static void
unload_file (gnutls_datum data)
{
  free (data.data);
}

#define MAX_CRT 6
static unsigned int x509_crt_size;
static gnutls_x509_crt x509_crt[MAX_CRT];
static gnutls_x509_privkey x509_key = NULL;

static gnutls_openpgp_key pgp_crt = NULL;
static gnutls_openpgp_privkey pgp_key = NULL;

/* Load the certificate and the private key.
 */
static void
load_keys (void)
{
  unsigned int crt_num;
  int ret;
  gnutls_datum data;

  if (x509_certfile != NULL && x509_keyfile != NULL)
    {
      data = load_file (x509_certfile);
      if (data.data == NULL)
	{
	  fprintf (stderr, "*** Error loading cert file.\n");
	  exit (1);
	}

      crt_num = MAX_CRT;
      ret =
	gnutls_x509_crt_list_import (x509_crt, &crt_num, &data,
				     GNUTLS_X509_FMT_PEM,
				     GNUTLS_X509_CRT_LIST_IMPORT_FAIL_IF_EXCEED);
      if (ret < 0)
	{
	  if (ret == GNUTLS_E_SHORT_MEMORY_BUFFER)
	    {
	      fprintf (stderr,
		       "*** Error loading cert file: Too many certs %d\n",
		       crt_num);

	    }
	  else
	    {
	      fprintf (stderr,
		       "*** Error loading cert file: %s\n",
		       gnutls_strerror (ret));
	    }
	  exit (1);
	}
      x509_crt_size = ret;
      fprintf (stderr, "Processed %d client certificates...\n", ret);

      unload_file (data);

      data = load_file (x509_keyfile);
      if (data.data == NULL)
	{
	  fprintf (stderr, "*** Error loading key file.\n");
	  exit (1);
	}

      gnutls_x509_privkey_init (&x509_key);

      ret = gnutls_x509_privkey_import (x509_key, &data, GNUTLS_X509_FMT_PEM);
      if (ret < 0)
	{
	  fprintf (stderr, "*** Error loading key file: %s\n",
		   gnutls_strerror (ret));
	  exit (1);
	}

      unload_file (data);

      fprintf (stderr, "Processed %d client X.509 certificates...\n",
	       x509_crt_size);
    }
#ifdef ENABLE_OPENPGP
  if (pgp_certfile != NULL && pgp_keyfile != NULL)
    {
      data = load_file (pgp_certfile);
      if (data.data == NULL)
	{
	  fprintf (stderr, "*** Error loading PGP cert file.\n");
	  exit (1);
	}
      gnutls_openpgp_key_init (&pgp_crt);

      ret =
	gnutls_openpgp_key_import (pgp_crt, &data, GNUTLS_OPENPGP_FMT_BASE64);
      if (ret < 0)
	{
	  fprintf (stderr,
		   "*** Error loading PGP cert file: %s\n",
		   gnutls_strerror (ret));
	  exit (1);
	}

      unload_file (data);

      data = load_file (pgp_keyfile);
      if (data.data == NULL)
	{
	  fprintf (stderr, "*** Error loading PGP key file.\n");
	  exit (1);
	}

      gnutls_openpgp_privkey_init (&pgp_key);

      ret =
	gnutls_openpgp_privkey_import (pgp_key, &data,
				       GNUTLS_OPENPGP_FMT_BASE64, NULL, 0);
      if (ret < 0)
	{
	  fprintf (stderr,
		   "*** Error loading PGP key file: %s\n",
		   gnutls_strerror (ret));
	  exit (1);
	}

      unload_file (data);
      fprintf (stderr, "Processed 1 client PGP certificate...\n");
    }
#endif

}



/* This callback should be associated with a session by calling
 * gnutls_certificate_client_set_retrieve_function( session, cert_callback),
 * before a handshake.
 */

static int
cert_callback (gnutls_session session,
	       const gnutls_datum * req_ca_rdn, int nreqs,
	       const gnutls_pk_algorithm * sign_algos,
	       int sign_algos_length, gnutls_retr_st * st)
{
  char issuer_dn[256];
  int i, ret;
  size_t len;

  if (verbose)
    {

      /* Print the server's trusted CAs
       */
      if (nreqs > 0)
	printf ("- Server's trusted authorities:\n");
      else
	printf ("- Server did not send us any trusted authorities names.\n");

      /* print the names (if any) */
      for (i = 0; i < nreqs; i++)
	{
	  len = sizeof (issuer_dn);
	  ret = gnutls_x509_rdn_get (&req_ca_rdn[i], issuer_dn, &len);
	  if (ret >= 0)
	    {
	      printf ("   [%d]: ", i);
	      printf ("%s\n", issuer_dn);
	    }
	}
    }

  /* Select a certificate and return it.
   * The certificate must be of any of the "sign algorithms"
   * supported by the server.
   */

  st->type = gnutls_certificate_type_get (session);

  st->ncerts = 0;

  if (st->type == GNUTLS_CRT_X509)
    {
      if (x509_crt != NULL && x509_key != NULL)
	{
	  st->ncerts = x509_crt_size;

	  st->cert.x509 = x509_crt;
	  st->key.x509 = x509_key;

	  st->deinit_all = 0;

	  return 0;
	}
    }
  else if (st->type == GNUTLS_CRT_OPENPGP)
    {
      if (pgp_key != NULL && pgp_crt != NULL)
	{
	  st->ncerts = 1;

	  st->cert.pgp = pgp_crt;
	  st->key.pgp = pgp_key;

	  st->deinit_all = 0;

	  return 0;
	}
    }

  printf ("- Successfully sent %d certificate(s) to server.\n", st->ncerts);
  return 0;

}



/* initializes a gnutls_session with some defaults.
 */
static gnutls_session
init_tls_session (const char *hostname)
{
  gnutls_session session;

  gnutls_init (&session, GNUTLS_CLIENT);

  /* allow the use of private ciphersuites.
   */
  if (disable_extensions == 0)
    {
      gnutls_handshake_set_private_extensions (session, 1);
      gnutls_server_name_set (session, GNUTLS_NAME_DNS, hostname,
			      strlen (hostname));
      gnutls_certificate_type_set_priority (session, cert_type_priority);
    }

  gnutls_cipher_set_priority (session, cipher_priority);
  gnutls_compression_set_priority (session, comp_priority);
  gnutls_kx_set_priority (session, kx_priority);
  gnutls_protocol_set_priority (session, protocol_priority);
  gnutls_mac_set_priority (session, mac_priority);


  gnutls_dh_set_prime_bits (session, 512);

  gnutls_credentials_set (session, GNUTLS_CRD_ANON, anon_cred);
  gnutls_credentials_set (session, GNUTLS_CRD_SRP, srp_cred);
  gnutls_credentials_set (session, GNUTLS_CRD_PSK, psk_cred);
  gnutls_credentials_set (session, GNUTLS_CRD_CERTIFICATE, xcred);

  gnutls_certificate_client_set_retrieve_function (xcred, cert_callback);

  /* send the fingerprint */
  if (fingerprint != 0)
    gnutls_openpgp_send_key (session, GNUTLS_OPENPGP_KEY_FINGERPRINT);

  /* use the max record size extension */
  if (record_max_size > 0 && disable_extensions == 0)
    {
      if (gnutls_record_set_max_size (session, record_max_size) < 0)
	{
	  fprintf (stderr,
		   "Cannot set the maximum record size to %d.\n",
		   record_max_size);
	  fprintf (stderr, "Possible values: 512, 1024, 2048, 4096.\n");
	  exit (1);
	}
    }

  return session;
}

static void gaa_parser (int argc, char **argv);

/* Returns zero if the error code was successfully handled.
 */
static int
handle_error (socket_st *hd, int err)
{
  int alert, ret;
  const char *err_type, *str;

  if (err >= 0)
    return 0;

  if (gnutls_error_is_fatal (err) == 0)
    {
      ret = 0;
      err_type = "Non fatal";
    }
  else
    {
      ret = err;
      err_type = "Fatal";
    }

  str = gnutls_strerror (err);
  if (str == NULL)
    str = str_unknown;
  fprintf (stderr, "*** %s error: %s\n", err_type, str);

  if (err == GNUTLS_E_WARNING_ALERT_RECEIVED
      || err == GNUTLS_E_FATAL_ALERT_RECEIVED)
    {
      alert = gnutls_alert_get (hd->session);
      str = gnutls_alert_get_name (alert);
      if (str == NULL)
	str = str_unknown;
      printf ("*** Received alert [%d]: %s\n", alert, str);

      /* In SRP if the alert is MISSING_SRP_USERNAME,
       * we should read the username/password and
       * call gnutls_handshake(). This is not implemented
       * here.
       */
    }

  check_rehandshake (hd, ret);

  return ret;
}

int starttls_alarmed = 0;

void
starttls_alarm (int signum)
{
  starttls_alarmed = 1;
}


int
main (int argc, char **argv)
{
  int err, ret;
  int ii, i;
  char buffer[MAX_BUF + 1];
  char *session_data = NULL;
  char *session_id = NULL;
  size_t session_data_size;
  size_t session_id_size;
  fd_set rset;
  int maxfd;
  struct timeval tv;
  int user_term = 0;
  socket_st hd;

  gaa_parser (argc, argv);
  if (hostname == NULL)
    {
      fprintf (stderr, "No hostname given\n");
      exit (1);
    }

  sockets_init ();

#ifndef _WIN32
  signal (SIGPIPE, SIG_IGN);
#endif

  init_global_tls_stuff ();

  socket_open( &hd, hostname, service);
  socket_connect( &hd);

  hd.session = init_tls_session (hostname);
  if (starttls)
    goto after_handshake;

  for (i = 0; i < 2; i++)
    {


      if (i == 1)
	{
	  hd.session = init_tls_session (hostname);
	  gnutls_session_set_data (hd.session, session_data,
				   session_data_size);
	  free (session_data);
	}

      ret = do_handshake (&hd);

      if (ret < 0)
	{
	  fprintf (stderr, "*** Handshake has failed\n");
	  gnutls_perror (ret);
	  gnutls_deinit (hd.session);
	  return 1;
	}
      else
	{
	  printf ("- Handshake was completed\n");
	  if (gnutls_session_is_resumed (hd.session) != 0)
	    printf ("*** This is a resumed session\n");
	}



      if (resume != 0 && i == 0)
	{

	  gnutls_session_get_data (hd.session, NULL, &session_data_size);
	  session_data = malloc (session_data_size);

	  gnutls_session_get_data (hd.session, session_data,
				   &session_data_size);

	  gnutls_session_get_id (hd.session, NULL, &session_id_size);
	  session_id = malloc (session_id_size);
	  gnutls_session_get_id (hd.session, session_id, &session_id_size);

	  /* print some information */
	  print_info (hd.session, hostname);

	  printf ("- Disconnecting\n");
	  socket_bye (&hd);

	  printf
	    ("\n\n- Connecting again- trying to resume previous session\n");
          socket_open( &hd, hostname, service);
          socket_connect(&hd);
	}
      else
	{
	  break;
	}
    }

after_handshake:

  printf ("\n- Simple Client Mode:\n\n");

#ifndef _WIN32
  signal (SIGALRM, &starttls_alarm);
#endif

  /* do not buffer */
#if !(defined _WIN32 || defined __WIN32__)
  setbuf (stdin, NULL);
#endif
  setbuf (stdout, NULL);
  setbuf (stderr, NULL);

  for (;;)
    {
      if (starttls_alarmed && !hd.secure)
	{
	  fprintf (stderr, "*** Starting TLS handshake\n");
	  ret = do_handshake (&hd);
	  if (ret < 0)
	    {
	      fprintf (stderr, "*** Handshake has failed\n");
	      socket_bye (&hd);
	      user_term = 1;
	      break;
	    }
	}

      FD_ZERO (&rset);
      FD_SET (fileno (stdin), &rset);
      FD_SET (hd.fd, &rset);

      maxfd = MAX (fileno (stdin), hd.fd);
      tv.tv_sec = 3;
      tv.tv_usec = 0;

      err = select (maxfd + 1, &rset, NULL, NULL, &tv);
      if (err < 0)
	continue;

      if (FD_ISSET (hd.fd, &rset))
	{
	  memset (buffer, 0, MAX_BUF + 1);
	  ret = socket_recv (&hd, buffer, MAX_BUF);

	  if (ret == 0)
	    {
	      printf ("- Peer has closed the GNUTLS connection\n");
	      break;
	    }
	  else if (handle_error (&hd, ret) < 0 && user_term == 0)
	    {
	      fprintf (stderr,
		       "*** Server has terminated the connection abnormally.\n");
	      break;
	    }
	  else if (ret > 0)
	    {
	      if (verbose != 0)
		printf ("- Received[%d]: ", ret);
	      for (ii = 0; ii < ret; ii++)
		{
		  fputc (buffer[ii], stdout);
		}
	      fflush (stdout);
	    }

	  if (user_term != 0)
	    break;
	}

      if (FD_ISSET (fileno (stdin), &rset))
	{
	  if (fgets (buffer, MAX_BUF, stdin) == NULL)
	    {
	      if (hd.secure == 0)
		{
		  fprintf (stderr, "*** Starting TLS handshake\n");
		  ret = do_handshake (&hd);
		  if (ret < 0)
		    {
		      fprintf (stderr, "*** Handshake has failed\n");
		      socket_bye (&hd);
		      user_term = 1;
		    }
		}
	      else
		{
		  user_term = 1;
		  break;
		}
	      continue;
	    }

	  if (crlf != 0)
	    {
	      char *b = strchr (buffer, '\n');
	      if (b != NULL)
		strcpy (b, "\r\n");
	    }

	  ret = socket_send (&hd, buffer, strlen (buffer));

	  if (ret > 0)
	    {
	      if (verbose != 0)
		printf ("- Sent: %d bytes\n", ret);
	    }
	  else
	    handle_error (&hd, ret);

	}
    }

  if (user_term != 0)
    socket_bye (&hd);
  else
    gnutls_deinit (hd.session);

#ifdef ENABLE_SRP
  gnutls_srp_free_client_credentials (srp_cred);
#endif
#ifdef ENABLE_PSK
  gnutls_psk_free_client_credentials (psk_cred);
#endif

  gnutls_certificate_free_credentials (xcred);

#ifdef ENABLE_ANON
  gnutls_anon_free_client_credentials (anon_cred);
#endif

  gnutls_global_deinit ();

  return 0;
}

static gaainfo info;
void
gaa_parser (int argc, char **argv)
{
  if (gaa (argc, argv, &info) != -1)
    {
      fprintf (stderr,
	       "Error in the arguments. Use the --help or -h parameters to get more information.\n");
      exit (1);
    }

  debug = info.debug;
  verbose = info.verbose;
  disable_extensions = info.disable_extensions;
  xml = info.xml;
  print_cert = info.print_cert;
  starttls = info.starttls;
  resume = info.resume;
  insecure = info.insecure;
  service = info.port;
  record_max_size = info.record_size;
  fingerprint = info.fingerprint;

  if (info.fmtder == 0)
    x509ctype = GNUTLS_X509_FMT_PEM;
  else
    x509ctype = GNUTLS_X509_FMT_DER;

  srp_username = info.srp_username;
  srp_passwd = info.srp_passwd;
  x509_cafile = info.x509_cafile;
  x509_crlfile = info.x509_crlfile;
  x509_keyfile = info.x509_keyfile;
  x509_certfile = info.x509_certfile;
  pgp_keyfile = info.pgp_keyfile;
  pgp_certfile = info.pgp_certfile;

  psk_username = info.psk_username;
  psk_key.data = (unsigned char *) info.psk_key;
  if (info.psk_key != NULL)
    psk_key.size = strlen (info.psk_key);
  else
    psk_key.size = 0;

  pgp_keyring = info.pgp_keyring;
  pgp_trustdb = info.pgp_trustdb;

  crlf = info.crlf;

  if (info.rest_args == NULL)
    hostname = "localhost";
  else
    hostname = info.rest_args;

  parse_protocols (info.proto, info.nproto, protocol_priority);
  parse_ciphers (info.ciphers, info.nciphers, cipher_priority);
  parse_macs (info.macs, info.nmacs, mac_priority);
  parse_ctypes (info.ctype, info.nctype, cert_type_priority);
  parse_kx (info.kx, info.nkx, kx_priority);
  parse_comp (info.comp, info.ncomp, comp_priority);


}

void
cli_version (void)
{
  const char *v = gnutls_check_version (NULL);

  printf ("gnutls-cli (GnuTLS) %s\n", LIBGNUTLS_VERSION);
  if (strcmp (v, LIBGNUTLS_VERSION) != 0)
    printf ("libgnutls %s\n", v);
}


static void
check_rehandshake (socket_st *socket, int ret)
{
  if (socket->secure && ret == GNUTLS_E_REHANDSHAKE)
    {
      /* There is a race condition here. If application
       * data is sent after the rehandshake request,
       * the server thinks we ignored his request.
       * This is a bad design of this client.
       */
      printf ("*** Received rehandshake request\n");
      /* gnutls_alert_send( session, GNUTLS_AL_WARNING, GNUTLS_A_NO_RENEGOTIATION); */

      ret = do_handshake (socket);

      if (ret == 0)
	{
	  printf ("*** Rehandshake was performed.\n");
	}
      else
	{
	  printf ("*** Rehandshake Failed.\n");
	}
    }
}


static int
do_handshake (socket_st * socket)
{
  int ret;
  gnutls_transport_set_ptr (socket->session,
			    (gnutls_transport_ptr) socket->fd);
  do
    {
      ret = gnutls_handshake (socket->session);

      if (ret < 0)
	{
	  handle_error (socket, ret);
	}
    }
  while (ret < 0 && gnutls_error_is_fatal (ret) == 0);

  if (ret == 0)
    {
      /* print some information */
      print_info (socket->session, socket->hostname);

      if ((x509_cafile || pgp_trustdb) && !insecure)
	{
	  int rc;
	  unsigned int status;

	  /* abort if verification fail  */
	  rc = gnutls_certificate_verify_peers2 (socket->session, &status);
	  if (rc != 0 || status != 0)
	    {
	      printf ("*** Verifying server certificate failed...\n");
	      exit (1);
	    }
	}

      socket->secure = 1;

    }
  return ret;
}

static int
srp_username_callback (gnutls_session session,
		       unsigned int times, char **username, char **password)
{
  if (srp_username == NULL || srp_passwd == NULL)
    {
      return -1;
    }

  /* We should ask here the user for his SRP username
   * and password.
   */
  if (times == 1)
    {
      *username = gnutls_strdup (srp_username);
      *password = gnutls_strdup (srp_passwd);

      return 0;
    }
  else
    /* At the first time return username and password, if
     * the kx_priority[0] is an SRP method.
     */
  if (times == 0 && (kx_priority[0] == GNUTLS_KX_SRP ||
		       kx_priority[0] ==
		       GNUTLS_KX_SRP_RSA
		       || kx_priority[0] == GNUTLS_KX_SRP_DSS))
    {

      *username = gnutls_strdup (srp_username);
      *password = gnutls_strdup (srp_passwd);

      return 0;
    }

  return -1;
}

static void
tls_log_func (int level, const char *str)
{
  fprintf (stderr, "|<%d>| %s", level, str);
}

static void
init_global_tls_stuff (void)
{
  int ret;

  if ((ret = gnutls_global_init ()) < 0)
    {
      fprintf (stderr, "global_init: %s\n", gnutls_strerror (ret));
      exit (1);
    }

  gnutls_global_set_log_function (tls_log_func);
  gnutls_global_set_log_level (debug);

  if ((ret = gnutls_global_init_extra ()) < 0)
    {
      fprintf (stderr, "global_init_extra: %s\n", gnutls_strerror (ret));
//      exit (1);
    }

  /* X509 stuff */
  if (gnutls_certificate_allocate_credentials (&xcred) < 0)
    {
      fprintf (stderr, "Certificate allocation memory error\n");
      exit (1);
    }

  /* there are some CAs that have a v1 certificate *%&@#*%&
   */
  gnutls_certificate_set_verify_flags (xcred,
				       GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT);

  if (x509_cafile != NULL)
    {
      ret =
	gnutls_certificate_set_x509_trust_file (xcred,
						x509_cafile, x509ctype);
      if (ret < 0)
	{
	  fprintf (stderr, "Error setting the x509 trust file\n");
	}
      else
	{
	  printf ("Processed %d CA certificate(s).\n", ret);
	}
    }
#ifdef ENABLE_PKI
  if (x509_crlfile != NULL)
    {
      ret =
	gnutls_certificate_set_x509_crl_file (xcred, x509_crlfile, x509ctype);
      if (ret < 0)
	{
	  fprintf (stderr, "Error setting the x509 CRL file\n");
	}
      else
	{
	  printf ("Processed %d CRL(s).\n", ret);
	}
    }
#endif

  load_keys ();

#ifdef ENABLE_OPENPGP
  if (pgp_keyring != NULL)
    {
      ret = gnutls_certificate_set_openpgp_keyring_file (xcred, pgp_keyring);
      if (ret < 0)
	{
	  fprintf (stderr, "Error setting the OpenPGP keyring file\n");
	}
    }

  if (pgp_trustdb != NULL)
    {
      ret = gnutls_certificate_set_openpgp_trustdb (xcred, pgp_trustdb);
      if (ret < 0)
	{
	  fprintf (stderr, "Error setting the OpenPGP trustdb file\n");
	}
    }
#endif

#ifdef ENABLE_SRP
  /* SRP stuff */
  if (gnutls_srp_allocate_client_credentials (&srp_cred) < 0)
    {
      fprintf (stderr, "SRP authentication error\n");
    }


  gnutls_srp_set_client_credentials_function (srp_cred,
					      srp_username_callback);
#endif

#ifdef ENABLE_PSK
  /* SRP stuff */
  if (gnutls_psk_allocate_client_credentials (&psk_cred) < 0)
    {
      fprintf (stderr, "PSK authentication error\n");
    }

  gnutls_psk_set_client_credentials (psk_cred,
				     psk_username, &psk_key,
				     GNUTLS_PSK_KEY_HEX);
#endif


#ifdef ENABLE_ANON
  /* ANON stuff */
  if (gnutls_anon_allocate_client_credentials (&anon_cred) < 0)
    {
      fprintf (stderr, "Anonymous authentication error\n");
    }
#endif

}

/* Functions to manipulate sockets
 */

ssize_t
        socket_recv (const socket_st* socket, void *buffer, int buffer_size)
{
    int ret;

    if (socket->secure)
        do
    {
        ret = gnutls_record_recv (socket->session, buffer, buffer_size);
    }
    while (ret == GNUTLS_E_INTERRUPTED || ret == GNUTLS_E_AGAIN);
    else
        do
    {
        ret = recv (socket->fd, buffer, buffer_size, 0);
    }
    while (ret == -1 && errno == EINTR);

    return ret;
}

ssize_t
        socket_send (const socket_st *socket, const void *buffer, int buffer_size)
{
    int ret;

    if (socket->secure)
        do
    {
        ret = gnutls_record_send (socket->session, buffer, buffer_size);
    }
    while (ret == GNUTLS_E_AGAIN || ret == GNUTLS_E_INTERRUPTED);
    else
        do
    {
        ret = send (socket->fd, buffer, buffer_size, 0);
    }
    while (ret == -1 && errno == EINTR);

    if (ret > 0 && ret != buffer_size && verbose)
        fprintf (stderr,
                 "*** Only sent %d bytes instead of %d.\n", ret, buffer_size);

    return ret;
}

void
        socket_bye (socket_st * socket)
{
    int ret;
    if (socket->secure)
    {
        do
            ret = gnutls_bye (socket->session, GNUTLS_SHUT_RDWR);
        while (ret == GNUTLS_E_INTERRUPTED || ret == GNUTLS_E_AGAIN);
        if (ret < 0)
            fprintf (stderr, "*** gnutls_bye() error: %s\n",
                     gnutls_strerror (ret));
        gnutls_deinit (socket->session);
        socket->session = NULL;
    }

    freeaddrinfo( socket->addr_info);
    socket->addr_info = socket->ptr = NULL;
    
    free( socket->ip);
    free( socket->hostname);
    free( socket->service);
    
    shutdown (socket->fd, SHUT_RDWR);     /* no more receptions */
    close (socket->fd);

    socket->fd = -1;
    socket->secure = 0;
}

void socket_connect( const socket_st* hd)
{
    int err;

    printf ("Connecting to '%s:%s'...\n", hd->ip, hd->service);

    err = connect (hd->fd, hd->ptr->ai_addr, hd->ptr->ai_addrlen);
    if (err < 0)
    {
        fprintf (stderr, "Cannot connect to %s:%s: %s\n", hd->hostname, hd->service,
                 strerror (errno));
        exit (1);
    }
}

void socket_open( socket_st* hd, const char* hostname, const char* service)
{
    struct addrinfo hints, *res, *ptr;
    int sd, err;
    char buffer[MAX_BUF + 1];
    char portname[16] = { 0 };

    printf ("Resolving '%s'...\n", hostname);
    /* get server name */
    memset (&hints, 0, sizeof (hints));
    hints.ai_socktype = SOCK_STREAM;
    if ((err = getaddrinfo (hostname, service, &hints, &res)))
    {
        fprintf (stderr, "Cannot resolve %s:%s: %s\n", hostname, service,
                 gai_strerror (err));
        exit (1);
    }

    sd = -1;
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        sd = socket (ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sd == -1) continue;

        if ((err = getnameinfo (ptr->ai_addr, ptr->ai_addrlen, buffer, MAX_BUF,
             portname, sizeof (portname), NI_NUMERICHOST|NI_NUMERICSERV)) != 0)
        {
            fprintf (stderr, "getnameinfo(): %s\n", gai_strerror (err));
            freeaddrinfo (res);
            exit (1);
        }

        break;
    }

    if (sd==-1) {
        fprintf (stderr, "socket(): %s\n", strerror (errno));
        exit (1);
    }
    
    hd->secure = 0;
    hd->fd = sd;
    hd->hostname = strdup(hostname);
    hd->ip = strdup(buffer);
    hd->service = strdup(portname);
    hd->ptr = ptr;
    hd->addr_info = res;

    return;
}
