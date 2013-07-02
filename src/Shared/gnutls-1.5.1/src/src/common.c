/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation
 * Author: Nikos Mavroyanopoulos
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
#include <stdlib.h>
#include <string.h>
#include <gnutls/gnutls.h>
#include <gnutls/extra.h>
#include <gnutls/x509.h>
#include <gnutls/openpgp.h>
#include <time.h>
#include <common.h>

#define TEST_STRING

#define SU(x) (x!=NULL?x:"Unknown")

int xml = 0;
int print_cert;
extern int verbose;

static char buffer[5 * 1024];

#define PRINTX(x,y) if (y[0]!=0) printf(" #   %s %s\n", x, y)
#define PRINT_PGP_NAME(X) PRINTX( "NAME:", name)

const char str_unknown[] = "(unknown)";

/* Hex encodes the given data.
 */
const char *
raw_to_string (const unsigned char *raw, size_t raw_size)
{
  static char buf[1024];
  size_t i;
  if (raw_size == 0)
    return NULL;

  if (raw_size * 3 + 1 >= sizeof (buf))
    return NULL;

  for (i = 0; i < raw_size; i++)
    {
      sprintf (&(buf[i * 3]), "%02X%s", raw[i],
	       (i == raw_size - 1) ? "" : ":");
    }
  buf[sizeof (buf) - 1] = '\0';

  return buf;
}

static const char *
my_ctime (const time_t * tv)
{
  static char buf[256];
  struct tm *tp;

  if (((tp = localtime (tv)) == NULL) ||
      (!strftime (buf, sizeof buf, "%a %b %e %H:%M:%S %Z %Y\n", tp)))
    strcpy (buf, str_unknown);	/* make sure buf text isn't garbage */

  return buf;

}


void
print_x509_info (gnutls_session session, const char *hostname)
{
  gnutls_x509_crt crt;
  const gnutls_datum *cert_list;
  size_t cert_list_size = 0;
  int ret;
  char digest[20];
  char serial[40];
  char dn[256];
  size_t dn_size;
  size_t digest_size = sizeof (digest);
  unsigned int j;
  size_t serial_size = sizeof (serial);
  const char *print;
  const char *cstr;
  unsigned int bits, algo;
  time_t expiret, activet;

  cert_list = gnutls_certificate_get_peers (session, &cert_list_size);


  if (cert_list_size == 0)
    {
      fprintf (stderr, "No certificates found!\n");
      return;
    }

  printf (" - Got a certificate list of %d certificates.\n\n",
	  cert_list_size);

  for (j = 0; j < (unsigned int) cert_list_size; j++)
    {

      gnutls_x509_crt_init (&crt);
      ret = gnutls_x509_crt_import (crt, &cert_list[j], GNUTLS_X509_FMT_DER);
      if (ret < 0)
	{
	  fprintf (stderr, "Decoding error: %s\n", gnutls_strerror (ret));
	  return;
	}

      printf (" - Certificate[%d] info:\n", j);

      if (print_cert)
	{
	  size_t size;

	  size = sizeof (buffer);

	  ret =
	    gnutls_x509_crt_export (crt, GNUTLS_X509_FMT_PEM, buffer, &size);
	  if (ret < 0)
	    {
	      fprintf (stderr, "Encoding error: %s\n", gnutls_strerror (ret));
	      return;
	    }
	  fputs ("\n", stdout);
	  fputs (buffer, stdout);
	  fputs ("\n", stdout);
	}

      if (j == 0 && hostname != NULL)
	{			/* Check the hostname of the first certificate
				 * if it matches the name of the host we
				 * connected to.
				 */
	  if (gnutls_x509_crt_check_hostname (crt, hostname) == 0)
	    {
	      printf
		(" # The hostname in the certificate does NOT match '%s'.\n",
		 hostname);
	    }
	  else
	    {
	      printf
		(" # The hostname in the certificate matches '%s'.\n",
		 hostname);
	    }
	}


      if (xml)
	{
#ifdef ENABLE_PKI
	  gnutls_datum xml_data;

	  ret = gnutls_x509_crt_to_xml (crt, &xml_data, 0);
	  if (ret < 0)
	    {
	      fprintf (stderr, "XML encoding error: %s\n",
		       gnutls_strerror (ret));
	      return;
	    }

	  printf ("%s", xml_data.data);
	  gnutls_free (xml_data.data);
#endif
	}
      else
	{

	  expiret = gnutls_x509_crt_get_expiration_time (crt);
	  activet = gnutls_x509_crt_get_activation_time (crt);

	  printf (" # valid since: %s", my_ctime (&activet));
	  printf (" # expires at: %s", my_ctime (&expiret));


	  /* Print the serial number of the certificate.
	   */
	  if (verbose
	      && gnutls_x509_crt_get_serial (crt, serial, &serial_size) >= 0)
	    {
	      print = raw_to_string (serial, serial_size);
	      if (print != NULL)
		printf (" # serial number: %s\n", print);
	    }

	  /* Print the fingerprint of the certificate
	   */
	  digest_size = sizeof (digest);
	  if ((ret =
	       gnutls_x509_crt_get_fingerprint (crt,
						GNUTLS_DIG_MD5,
						digest, &digest_size)) < 0)
	    {
	      fprintf (stderr,
		       "Error in fingerprint calculation: %s\n",
		       gnutls_strerror (ret));
	    }
	  else
	    {
	      print = raw_to_string (digest, digest_size);
	      if (print != NULL)
		printf (" # fingerprint: %s\n", print);
	    }

	  /* Print the version of the X.509 
	   * certificate.
	   */
	  if (verbose)
	    {
	      printf (" # version: #%d\n", gnutls_x509_crt_get_version (crt));

	      bits = 0;
	      algo = gnutls_x509_crt_get_pk_algorithm (crt, &bits);
	      printf (" # public key algorithm: ");

	      cstr = SU (gnutls_pk_algorithm_get_name (algo));
	      printf ("%s (%d bits)\n", cstr, bits);

#ifdef ENABLE_PKI
	      if (algo == GNUTLS_PK_RSA)
		{
		  gnutls_datum e, m;

		  ret = gnutls_x509_crt_get_pk_rsa_raw (crt, &m, &e);
		  if (ret >= 0)
		    {
		      print = SU (raw_to_string (e.data, e.size));
		      printf (" # e [%d bits]: %s\n", e.size * 8, print);

		      print = SU (raw_to_string (m.data, m.size));
		      printf (" # m [%d bits]: %s\n", m.size * 8, print);

		      gnutls_free (e.data);
		      gnutls_free (m.data);
		    }
		}
	      else if (algo == GNUTLS_PK_DSA)
		{
		  gnutls_datum p, q, g, y;

		  ret = gnutls_x509_crt_get_pk_dsa_raw (crt, &p, &q, &g, &y);
		  if (ret >= 0)
		    {
		      print = SU (raw_to_string (p.data, p.size));
		      printf (" # p [%d bits]: %s\n", p.size * 8, print);

		      print = SU (raw_to_string (q.data, q.size));
		      printf (" # q [%d bits]: %s\n", q.size * 8, print);

		      print = SU (raw_to_string (g.data, g.size));
		      printf (" # g [%d bits]: %s\n", g.size * 8, print);

		      print = SU (raw_to_string (y.data, y.size));
		      printf (" # y [%d bits]: %s\n", y.size * 8, print);

		      gnutls_free (p.data);
		      gnutls_free (q.data);
		      gnutls_free (g.data);
		      gnutls_free (y.data);
		    }
		}
#endif
	    }

	  dn_size = sizeof (dn);
	  ret = gnutls_x509_crt_get_dn (crt, dn, &dn_size);
	  if (ret >= 0)
	    printf (" # Subject's DN: %s\n", dn);

	  dn_size = sizeof (dn);
	  ret = gnutls_x509_crt_get_issuer_dn (crt, dn, &dn_size);
	  if (ret >= 0)
	    printf (" # Issuer's DN: %s\n", dn);
	}

      gnutls_x509_crt_deinit (crt);

      printf ("\n");

    }

}

#ifdef ENABLE_OPENPGP

void
print_openpgp_info (gnutls_session session, const char *hostname)
{

  char digest[20];
  size_t digest_size = sizeof (digest);
  int ret;
  const char *print;
  const char *cstr;
  char name[256];
  size_t name_len = sizeof (name);
  gnutls_openpgp_key crt;
  const gnutls_datum *cert_list;
  int cert_list_size = 0;
  time_t expiret;
  time_t activet;

  cert_list = gnutls_certificate_get_peers (session, &cert_list_size);

  if (cert_list_size > 0)
    {
      unsigned int algo, bits;

      gnutls_openpgp_key_init (&crt);
      ret =
	gnutls_openpgp_key_import (crt, &cert_list[0],
				   GNUTLS_OPENPGP_FMT_RAW);
      if (ret < 0)
	{
	  fprintf (stderr, "Decoding error: %s\n", gnutls_strerror (ret));
	  return;
	}

      if (print_cert)
	{
	  size_t size;

	  size = sizeof (buffer);

	  ret =
	    gnutls_openpgp_key_export (crt,
				       GNUTLS_OPENPGP_FMT_BASE64,
				       buffer, &size);
	  if (ret < 0)
	    {
	      fprintf (stderr, "Encoding error: %s\n", gnutls_strerror (ret));
	      return;
	    }
	  fputs ("\n", stdout);
	  fputs (buffer, stdout);
	  fputs ("\n", stdout);
	}

      if (hostname != NULL)
	{			/* Check the hostname of the first certificate
				 * if it matches the name of the host we
				 * connected to.
				 */
	  if (gnutls_openpgp_key_check_hostname (crt, hostname) == 0)
	    {
	      printf
		(" # The hostname in the key does NOT match '%s'.\n",
		 hostname);
	    }
	  else
	    {
	      printf (" # The hostname in the key matches '%s'.\n", hostname);
	    }
	}

      if (xml)
	{
	  gnutls_datum xml_data;

	  ret = gnutls_openpgp_key_to_xml (crt, &xml_data, 0);
	  if (ret < 0)
	    {
	      fprintf (stderr, "XML encoding error: %s\n",
		       gnutls_strerror (ret));
	      return;
	    }

	  printf ("%s", xml_data.data);
	  gnutls_free (xml_data.data);

	  return;
	}

      activet = gnutls_openpgp_key_get_creation_time (crt);
      expiret = gnutls_openpgp_key_get_expiration_time (crt);

      printf (" # Key was created at: %s", my_ctime (&activet));
      printf (" # Key expires: ");
      if (expiret != 0)
	printf ("%s", my_ctime (&expiret));
      else
	printf ("Never\n");

      if (gnutls_openpgp_key_get_fingerprint (crt, digest, &digest_size) >= 0)
	{
	  print = raw_to_string (digest, digest_size);

	  printf (" # PGP Key version: %d\n",
		  gnutls_openpgp_key_get_version (crt));

	  bits = 0;
	  algo = gnutls_openpgp_key_get_pk_algorithm (crt, &bits);

	  printf (" # PGP Key public key algorithm: ");
	  cstr = SU (gnutls_pk_algorithm_get_name (algo));
	  printf ("%s (%d bits)\n", cstr, bits);

	  if (print != NULL)
	    printf (" # PGP Key fingerprint: %s\n", print);

	  name_len = sizeof (name);
	  if (gnutls_openpgp_key_get_name (crt, 0, name, &name_len) < 0)
	    {
	      fprintf (stderr, "Could not extract name\n");
	    }
	  else
	    {
	      PRINT_PGP_NAME (name);
	    }

	}

      gnutls_openpgp_key_deinit (crt);

    }
}

#endif

void
print_cert_vrfy (gnutls_session session)
{
  int rc;
  unsigned int status;

  rc = gnutls_certificate_verify_peers2 (session, &status);
  printf ("\n");

  if (rc == GNUTLS_E_NO_CERTIFICATE_FOUND)
    {
      printf ("- Peer did not send any certificate.\n");
      return;
    }

  if (rc < 0)
    {
      printf ("- Could not verify certificate (err: %s)\n",
	      gnutls_strerror (rc));
      return;
    }

  if (gnutls_certificate_type_get (session) == GNUTLS_CRT_X509)
    {
      if (status & GNUTLS_CERT_SIGNER_NOT_FOUND)
	printf ("- Peer's certificate issuer is unknown\n");
      if (status & GNUTLS_CERT_INVALID)
	printf ("- Peer's certificate is NOT trusted\n");
      else
	printf ("- Peer's certificate is trusted\n");
    }
  else
    {
      if (status & GNUTLS_CERT_INVALID)
	printf ("- Peer's key is invalid\n");
      else
	printf ("- Peer's key is valid\n");
      if (status & GNUTLS_CERT_SIGNER_NOT_FOUND)
	printf ("- Could not find a signer of the peer's key\n");
    }
}

int
print_info (gnutls_session session, const char *hostname)
{
  const char *tmp;
  gnutls_credentials_type cred;
  gnutls_kx_algorithm kx;


  /* print the key exchange's algorithm name
   */
  kx = gnutls_kx_get (session);

  cred = gnutls_auth_get_type (session);
  switch (cred)
    {
#ifdef ENABLE_ANON
    case GNUTLS_CRD_ANON:
      printf ("- Anonymous DH using prime of %d bits, secret key "
	      "of %d bits, and peer's public key is %d bits.\n",
	      gnutls_dh_get_prime_bits (session),
	      gnutls_dh_get_secret_bits (session),
	      gnutls_dh_get_peers_public_bits (session));
      break;
#endif
#ifdef ENABLE_SRP
    case GNUTLS_CRD_SRP:
      /* This should be only called in server
       * side.
       */
      if (gnutls_srp_server_get_username (session) != NULL)
	printf ("- SRP authentication. Connected as '%s'\n",
		gnutls_srp_server_get_username (session));
      break;
#endif
#ifdef ENABLE_PSK
    case GNUTLS_CRD_PSK:
      /* This should be only called in server
       * side.
       */
      if (gnutls_psk_server_get_username (session) != NULL)
	printf ("- PSK authentication. Connected as '%s'\n",
		gnutls_psk_server_get_username (session));
      if (kx == GNUTLS_KX_DHE_PSK)
	{
	  printf ("- DH using prime of %d bits, secret key "
		  "of %d bits, and peer's public key is %d bits.\n",
		  gnutls_dh_get_prime_bits (session),
		  gnutls_dh_get_secret_bits (session),
		  gnutls_dh_get_peers_public_bits (session));
	}
      break;
#endif
    case GNUTLS_CRD_CERTIFICATE:
      {
	char dns[256];
	size_t dns_size = sizeof (dns);
	unsigned int type;

	/* This fails in client side */
	if (gnutls_server_name_get (session, dns, &dns_size, &type, 0) == 0)
	  {
	    printf ("- Given server name[%d]: %s\n", type, dns);
	  }
      }

      print_cert_info (session, hostname);

      print_cert_vrfy (session);

    }

  tmp = SU (gnutls_protocol_get_name (gnutls_protocol_get_version (session)));
  printf ("- Version: %s\n", tmp);

  tmp = SU (gnutls_kx_get_name (kx));
  printf ("- Key Exchange: %s\n", tmp);

  tmp = SU (gnutls_cipher_get_name (gnutls_cipher_get (session)));
  printf ("- Cipher: %s\n", tmp);

  tmp = SU (gnutls_mac_get_name (gnutls_mac_get (session)));
  printf ("- MAC: %s\n", tmp);

  tmp = SU (gnutls_compression_get_name (gnutls_compression_get (session)));
  printf ("- Compression: %s\n", tmp);

  fflush (stdout);

  return 0;
}

void
print_cert_info (gnutls_session session, const char *hostname)
{

  if (gnutls_certificate_client_get_request_status( session) != 0)
    printf("- Server has requested a certificate.\n");

  printf ("- Certificate type: ");
  switch (gnutls_certificate_type_get (session))
    {
    case GNUTLS_CRT_X509:
      printf ("X.509\n");
      print_x509_info (session, hostname);
      break;
#ifdef ENABLE_OPENPGP
    case GNUTLS_CRT_OPENPGP:
      printf ("OpenPGP\n");
      print_openpgp_info (session, hostname);
      break;
#endif
    }

}

void
print_list (void)
{
  /* FIXME: This is hard coded. Make it print all the supported
   * algorithms.
   */
  printf ("\n");
  printf ("Certificate types:");
  printf (" X.509");
  printf (", OPENPGP\n");

  printf ("Protocols:");
  printf (" TLS1.0");
  printf (", TLS1.1");
  printf (", SSL3.0\n");

  printf ("Ciphers:");
  printf (" AES-256-CBC");
  printf (", AES-128-CBC");
  printf (", 3DES-CBC");
  printf (", ARCFOUR");
  printf (", ARCFOUR-40\n");

  printf ("MACs:");
  printf (" MD5");
  printf (", RMD160");
  printf (", SHA1\n");

  printf ("Key exchange algorithms:");
  printf (" RSA");
  printf (", RSA-EXPORT");
  printf (", DHE-DSS");
  printf (", DHE-RSA");
  printf (", DHE-PSK");
  printf (", PSK");
  printf (", SRP");
  printf (", SRP-RSA");
  printf (", SRP-DSS");
  printf (", ANON-DH\n");

  printf ("Compression methods:");
  printf (" DEFLATE");
  printf (", LZO");
  printf (", NULL\n");
}

void
print_license (void)
{
  fputs ("\nCopyright (C) 2004 Free Software Foundation\n"
	 "This program is free software; you can redistribute it and/or modify \n"
	 "it under the terms of the GNU General Public License as published by \n"
	 "the Free Software Foundation; either version 2 of the License, or \n"
	 "(at your option) any later version. \n" "\n"
	 "This program is distributed in the hope that it will be useful, \n"
	 "but WITHOUT ANY WARRANTY; without even the implied warranty of \n"
	 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \n"
	 "GNU General Public License for more details. \n" "\n"
	 "You should have received a copy of the GNU General Public License \n"
	 "along with this program; if not, write to the Free Software \n"
	 "Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\n",
	 stdout);
}

void
parse_protocols (char **protocols, int protocols_size, int *protocol_priority)
{
  int i, j;

  if (protocols != NULL && protocols_size > 0)
    {
      for (j = i = 0; i < protocols_size; i++)
	{
	  if (strncasecmp (protocols[i], "SSL", 3) == 0)
	    protocol_priority[j++] = GNUTLS_SSL3;
	  else if (strncasecmp (protocols[i], "TLS1.1", 6) == 0)
	    protocol_priority[j++] = GNUTLS_TLS1_1;
	  else if (strncasecmp (protocols[i], "TLS", 3) == 0)
	    protocol_priority[j++] = GNUTLS_TLS1_0;
	  else
	    fprintf (stderr, "Unknown protocol: '%s'\n", protocols[i]);
	}
      protocol_priority[j] = 0;
    }
}

void
parse_ciphers (char **ciphers, int nciphers, int *cipher_priority)
{
  int j, i;

  if (ciphers != NULL && nciphers > 0)
    {
      for (j = i = 0; i < nciphers; i++)
	{
	  if (strncasecmp (ciphers[i], "AES-2", 5) == 0)
	    cipher_priority[j++] = GNUTLS_CIPHER_AES_256_CBC;
	  else if (strncasecmp (ciphers[i], "AES", 3) == 0)
	    cipher_priority[j++] = GNUTLS_CIPHER_AES_128_CBC;
	  else if (strncasecmp (ciphers[i], "3DE", 3) == 0)
	    cipher_priority[j++] = GNUTLS_CIPHER_3DES_CBC;
	  else if (strcasecmp (ciphers[i], "ARCFOUR-40") == 0)
	    cipher_priority[j++] = GNUTLS_CIPHER_ARCFOUR_40;
	  else if (strcasecmp (ciphers[i], "ARCFOUR") == 0)
	    cipher_priority[j++] = GNUTLS_CIPHER_ARCFOUR_128;
	  else if (strncasecmp (ciphers[i], "NUL", 3) == 0)
	    cipher_priority[j++] = GNUTLS_CIPHER_NULL;
	  else
	    fprintf (stderr, "Unknown cipher: '%s'\n", ciphers[i]);
	}
      cipher_priority[j] = 0;
    }
}

void
parse_macs (char **macs, int nmacs, int *mac_priority)
{
  int i, j;
  if (macs != NULL && nmacs > 0)
    {
      for (j = i = 0; i < nmacs; i++)
	{
	  if (strncasecmp (macs[i], "MD5", 3) == 0)
	    mac_priority[j++] = GNUTLS_MAC_MD5;
	  else if (strncasecmp (macs[i], "RMD", 3) == 0)
	    mac_priority[j++] = GNUTLS_MAC_RMD160;
	  else if (strncasecmp (macs[i], "SHA", 3) == 0)
	    mac_priority[j++] = GNUTLS_MAC_SHA1;
	  else
	    fprintf (stderr, "Unknown MAC: '%s'\n", macs[i]);
	}
      mac_priority[j] = 0;
    }
}

void
parse_ctypes (char **ctype, int nctype, int *cert_type_priority)
{
  int i, j;
  if (ctype != NULL && nctype > 0)
    {
      for (j = i = 0; i < nctype; i++)
	{
	  if (strncasecmp (ctype[i], "OPE", 3) == 0)
	    cert_type_priority[j++] = GNUTLS_CRT_OPENPGP;
	  else if (strncasecmp (ctype[i], "X", 1) == 0)
	    cert_type_priority[j++] = GNUTLS_CRT_X509;
	  else
	    fprintf (stderr, "Unknown certificate type: '%s'\n", ctype[i]);
	}
      cert_type_priority[j] = 0;
    }
}

void
parse_kx (char **kx, int nkx, int *kx_priority)
{
  int i, j;
  if (kx != NULL && nkx > 0)
    {
      for (j = i = 0; i < nkx; i++)
	{
	  if (strcasecmp (kx[i], "SRP") == 0)
	    kx_priority[j++] = GNUTLS_KX_SRP;
	  else if (strcasecmp (kx[i], "SRP-RSA") == 0)
	    kx_priority[j++] = GNUTLS_KX_SRP_RSA;
	  else if (strcasecmp (kx[i], "SRP-DSS") == 0)
	    kx_priority[j++] = GNUTLS_KX_SRP_DSS;
	  else if (strcasecmp (kx[i], "RSA") == 0)
	    kx_priority[j++] = GNUTLS_KX_RSA;
	  else if (strcasecmp (kx[i], "PSK") == 0)
	    kx_priority[j++] = GNUTLS_KX_PSK;
	  else if (strcasecmp (kx[i], "DHE-PSK") == 0)
	    kx_priority[j++] = GNUTLS_KX_DHE_PSK;
	  else if (strcasecmp (kx[i], "RSA-EXPORT") == 0)
	    kx_priority[j++] = GNUTLS_KX_RSA_EXPORT;
	  else if (strncasecmp (kx[i], "DHE-RSA", 7) == 0)
	    kx_priority[j++] = GNUTLS_KX_DHE_RSA;
	  else if (strncasecmp (kx[i], "DHE-DSS", 7) == 0)
	    kx_priority[j++] = GNUTLS_KX_DHE_DSS;
	  else if (strncasecmp (kx[i], "ANON", 4) == 0)
	    kx_priority[j++] = GNUTLS_KX_ANON_DH;
	  else
	    fprintf (stderr, "Unknown key exchange: '%s'\n", kx[i]);
	}
      kx_priority[j] = 0;
    }
}

void
parse_comp (char **comp, int ncomp, int *comp_priority)
{
  int i, j;
  if (comp != NULL && ncomp > 0)
    {
      for (j = i = 0; i < ncomp; i++)
	{
	  if (strncasecmp (comp[i], "NUL", 3) == 0)
	    comp_priority[j++] = GNUTLS_COMP_NULL;
	  else if (strncasecmp (comp[i], "ZLI", 3) == 0)
	    comp_priority[j++] = GNUTLS_COMP_DEFLATE;
	  else if (strncasecmp (comp[i], "DEF", 3) == 0)
	    comp_priority[j++] = GNUTLS_COMP_DEFLATE;
	  else if (strncasecmp (comp[i], "LZO", 3) == 0)
	    comp_priority[j++] = GNUTLS_COMP_LZO;
	  else
	    fprintf (stderr, "Unknown compression: '%s'\n", comp[i]);
	}
      comp_priority[j] = 0;
    }

}

void
sockets_init (void)
{
#ifdef _WIN32
  WORD wVersionRequested;
  WSADATA wsaData;

  wVersionRequested = MAKEWORD (1, 1);
  if (WSAStartup (wVersionRequested, &wsaData) != 0)
    {
      perror ("WSA_STARTUP_ERROR");
    }
#endif
}

/* converts a service name or a port (in string) to a
 * port number. The protocol is assumed to be TCP.
 *
 * returns -1 on error;
 */
int
service_to_port (const char *service)
{
  int port;
  struct servent *server_port;

  port = atoi (service);
  if (port != 0)
    return port;

  server_port = getservbyname (service, "tcp");
  if (server_port == NULL)
    {
      perror ("getservbyname()");
      return (-1);
    }

  return ntohs (server_port->s_port);

}
