/*
 * Copyright (C) 2004, 2005, 2006 Free Software Foundation
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

#ifdef ENABLE_PKI

#include <stdio.h>
#include <stdlib.h>
#include <certtool-cfg.h>
#include <cfg+.h>
#include <gnutls/x509.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>

/* Gnulib portability files. */
#include <getpass.h>
#include "readline.h"

extern int batch;

typedef struct _cfg_ctx
{
  char *organization;
  char *unit;
  char *locality;
  char *state;
  char *cn;
  char *uid;
  char *challenge_password;
  char *pkcs9_email;
  char *country;
  char *dns_name;
  char *ip_addr;
  char *email;
  char **dn_oid;
  char *crl_dist_points;
  char *password;
  char *pkcs12_key_name;
  int serial;
  int expiration_days;
  int ca;
  int tls_www_client;
  int tls_www_server;
  int signing_key;
  int encryption_key;
  int cert_sign_key;
  int crl_sign_key;
  int code_sign_key;
  int ocsp_sign_key;
  int time_stamping_key;
  int crl_next_update;
} cfg_ctx;

cfg_ctx cfg;

void
cfg_init (void)
{
  memset (&cfg, 0, sizeof (cfg));
}

int
template_parse (const char *template)
{
  /* libcfg+ parsing context */
  CFG_CONTEXT con;

  /* Parsing return code */
  register int ret;

  /* Option variables */

  /* Option set */
  struct cfg_option options[] = {
    {NULL, '\0', "organization", CFG_STR, (void *) &cfg.organization,
     0},
    {NULL, '\0', "unit", CFG_STR, (void *) &cfg.unit, 0},
    {NULL, '\0', "locality", CFG_STR, (void *) &cfg.locality, 0},
    {NULL, '\0', "state", CFG_STR, (void *) &cfg.state, 0},
    {NULL, '\0', "cn", CFG_STR, (void *) &cfg.cn, 0},
    {NULL, '\0', "uid", CFG_STR, (void *) &cfg.uid, 0},
    {NULL, '\0', "challenge_password", CFG_STR,
     (void *) &cfg.challenge_password, 0},
    {NULL, '\0', "password", CFG_STR, (void *) &cfg.password, 0},
    {NULL, '\0', "pkcs9_email", CFG_STR, (void *) &cfg.pkcs9_email, 0},
    {NULL, '\0', "country", CFG_STR, (void *) &cfg.country, 0},
    {NULL, '\0', "dns_name", CFG_STR, (void *) &cfg.dns_name, 0},
    {NULL, '\0', "ip_address", CFG_STR, (void *) &cfg.ip_addr, 0},
    {NULL, '\0', "email", CFG_STR, (void *) &cfg.email, 0},

    {NULL, '\0', "dn_oid", CFG_STR + CFG_MULTI_SEPARATED,
     (void *) &cfg.dn_oid, 0},

    {NULL, '\0', "crl_dist_points", CFG_STR,
     (void *) &cfg.crl_dist_points, 0},
    {NULL, '\0', "pkcs12_key_name", CFG_STR,
     (void *) &cfg.pkcs12_key_name, 0},

    {NULL, '\0', "serial", CFG_INT, (void *) &cfg.serial, 0},
    {NULL, '\0', "expiration_days", CFG_INT,
     (void *) &cfg.expiration_days, 0},

    {NULL, '\0', "crl_next_update", CFG_INT,
     (void *) &cfg.crl_next_update, 0},

    {NULL, '\0', "ca", CFG_BOOL, (void *) &cfg.ca, 0},
    {NULL, '\0', "tls_www_client", CFG_BOOL,
     (void *) &cfg.tls_www_client, 0},
    {NULL, '\0', "tls_www_server", CFG_BOOL,
     (void *) &cfg.tls_www_server, 0},
    {NULL, '\0', "signing_key", CFG_BOOL, (void *) &cfg.signing_key,
     0},
    {NULL, '\0', "encryption_key", CFG_BOOL,
     (void *) &cfg.encryption_key, 0},
    {NULL, '\0', "cert_signing_key", CFG_BOOL,
     (void *) &cfg.cert_sign_key, 0},
    {NULL, '\0', "crl_signing_key", CFG_BOOL,
     (void *) &cfg.crl_sign_key, 0},
    {NULL, '\0', "code_signing_key", CFG_BOOL,
     (void *) &cfg.code_sign_key, 0},
    {NULL, '\0', "ocsp_signing_key", CFG_BOOL,
     (void *) &cfg.ocsp_sign_key, 0},
    {NULL, '\0', "time_stamping_key", CFG_BOOL,
     (void *) &cfg.time_stamping_key, 0},
    CFG_END_OF_LIST
  };

  /* Creating context */
  con = cfg_get_context (options);
  if (con == NULL)
    {
      puts ("Not enough memory");
      exit (1);
    }

  cfg_set_cfgfile_context (con, 0, -1, (char *) template);

  /* Parsing command line */
  ret = cfg_parse (con);

  if (ret != CFG_OK)
    {
      printf ("error parsing command line: %s: ", template);
      cfg_fprint_error (con, stdout);
      putchar ('\n');
      exit (ret < 0 ? -ret : ret);
    }

  return 0;
}

void
read_crt_set (gnutls_x509_crt crt, const char *input_str, const char *oid)
{
  char input[128];
  int ret;

  fputs (input_str, stderr);
  fgets (input, sizeof (input), stdin);

  if (strlen (input) == 1)	/* only newline */
    return;

  ret =
    gnutls_x509_crt_set_dn_by_oid (crt, oid, 0, input, strlen (input) - 1);
  if (ret < 0)
    {
      fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
      exit (1);
    }
}

void
read_crq_set (gnutls_x509_crq crq, const char *input_str, const char *oid)
{
  char input[128];
  int ret;

  fputs (input_str, stderr);
  fgets (input, sizeof (input), stdin);

  if (strlen (input) == 1)	/* only newline */
    return;

  ret =
    gnutls_x509_crq_set_dn_by_oid (crq, oid, 0, input, strlen (input) - 1);
  if (ret < 0)
    {
      fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
      exit (1);
    }
}

int
read_int (const char *input_str)
{
  char *in;
  char *endptr;
  long l;

  in = readline (input_str);

  l = strtol (in, &endptr, 0);

  if (*endptr != '\0')
    {
      fprintf (stderr, "Trailing garbage ignored: `%s'\n", endptr);
      free (in);
      return 0;
    }

  if (l <= INT_MIN || l >= INT_MAX)
    {
      fprintf (stderr, "Integer out of range: `%s'\n", in);
      free (in);
      return 0;
    }

  free (in);

  return (int) l;
}

const char *
read_str (const char *input_str)
{
  static char input[128];
  int len;

  fputs (input_str, stderr);
  if (fgets (input, sizeof (input), stdin) == NULL)
    return NULL;

  len = strlen (input);
  if ((len > 0) && (input[len - 1] == '\n'))
    input[len - 1] = 0;
  if (input[0] == 0)
    return NULL;

  return input;
}

int
read_yesno (const char *input_str)
{
  char input[128];

  fputs (input_str, stderr);
  fgets (input, sizeof (input), stdin);

  if (strlen (input) == 1)	/* only newline */
    return 0;

  if (input[0] == 'y' || input[0] == 'Y')
    return 1;

  return 0;
}


/* Wrapper functions for non-interactive mode.
 */
const char *
get_pass (void)
{
  if (batch)
    return cfg.password;
  else
    return getpass ("Enter password: ");
}

const char *
get_challenge_pass (void)
{
  if (batch)
    return cfg.challenge_password;
  else
    return getpass ("Enter a challenge password: ");
}

const char *
get_crl_dist_point_url (void)
{
  if (batch)
    return cfg.crl_dist_points;
  else
    return read_str ("Enter the URI of the CRL distribution point: ");
}

void
get_country_crt_set (gnutls_x509_crt crt)
{
  int ret;

  if (batch)
    {
      if (!cfg.country)
	return;
      ret =
	gnutls_x509_crt_set_dn_by_oid (crt,
				       GNUTLS_OID_X520_COUNTRY_NAME, 0,
				       cfg.country, strlen (cfg.country));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crt_set (crt, "Country name (2 chars): ",
		    GNUTLS_OID_X520_COUNTRY_NAME);
    }

}

void
get_organization_crt_set (gnutls_x509_crt crt)
{
  int ret;

  if (batch)
    {
      if (!cfg.organization)
	return;

      ret =
	gnutls_x509_crt_set_dn_by_oid (crt,
				       GNUTLS_OID_X520_ORGANIZATION_NAME,
				       0, cfg.organization,
				       strlen (cfg.organization));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crt_set (crt, "Organization name: ",
		    GNUTLS_OID_X520_ORGANIZATION_NAME);
    }

}

void
get_unit_crt_set (gnutls_x509_crt crt)
{
  int ret;

  if (batch)
    {
      if (!cfg.unit)
	return;

      ret =
	gnutls_x509_crt_set_dn_by_oid (crt,
				       GNUTLS_OID_X520_ORGANIZATIONAL_UNIT_NAME,
				       0, cfg.unit, strlen (cfg.unit));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crt_set (crt, "Organizational unit name: ",
		    GNUTLS_OID_X520_ORGANIZATIONAL_UNIT_NAME);
    }

}

void
get_state_crt_set (gnutls_x509_crt crt)
{
  int ret;

  if (batch)
    {
      if (!cfg.state)
	return;
      ret =
	gnutls_x509_crt_set_dn_by_oid (crt,
				       GNUTLS_OID_X520_STATE_OR_PROVINCE_NAME,
				       0, cfg.state, strlen (cfg.state));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crt_set (crt, "State or province name: ",
		    GNUTLS_OID_X520_STATE_OR_PROVINCE_NAME);
    }

}

void
get_locality_crt_set (gnutls_x509_crt crt)
{
  int ret;

  if (batch)
    {
      if (!cfg.locality)
	return;
      ret =
	gnutls_x509_crt_set_dn_by_oid (crt,
				       GNUTLS_OID_X520_LOCALITY_NAME, 0,
				       cfg.locality, strlen (cfg.locality));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crt_set (crt, "Locality name: ", GNUTLS_OID_X520_LOCALITY_NAME);
    }

}

void
get_cn_crt_set (gnutls_x509_crt crt)
{
  int ret;

  if (batch)
    {
      if (!cfg.cn)
	return;
      ret =
	gnutls_x509_crt_set_dn_by_oid (crt, GNUTLS_OID_X520_COMMON_NAME,
				       0, cfg.cn, strlen (cfg.cn));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crt_set (crt, "Common name: ", GNUTLS_OID_X520_COMMON_NAME);
    }

}

void
get_uid_crt_set (gnutls_x509_crt crt)
{
  int ret;

  if (batch)
    {
      if (!cfg.uid)
	return;
      ret = gnutls_x509_crt_set_dn_by_oid (crt, GNUTLS_OID_LDAP_UID, 0,
					   cfg.uid, strlen (cfg.uid));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crt_set (crt, "UID: ", GNUTLS_OID_LDAP_UID);
    }

}

void
get_oid_crt_set (gnutls_x509_crt crt)
{
  int ret, i;

  if (batch)
    {
      if (!cfg.dn_oid)
	return;
      for (i = 0; cfg.dn_oid[i] != NULL; i += 2)
	{
	  if (cfg.dn_oid[i + 1] == NULL)
	    {
	      fprintf (stderr, "dn_oid: %s does not have an argument.\n",
		       cfg.dn_oid[i]);
	      exit (1);
	    }
	  ret = gnutls_x509_crt_set_dn_by_oid (crt, cfg.dn_oid[i], 0,
					       cfg.dn_oid[i + 1],
					       strlen (cfg.dn_oid[i + 1]));

	  if (ret < 0)
	    {
	      fprintf (stderr, "set_dn_oid: %s\n", gnutls_strerror (ret));
	      exit (1);
	    }
	}
    }

}


void
get_pkcs9_email_crt_set (gnutls_x509_crt crt)
{
  int ret;

  if (batch)
    {
      if (!cfg.pkcs9_email)
	return;
      ret = gnutls_x509_crt_set_dn_by_oid (crt, GNUTLS_OID_PKCS9_EMAIL, 0,
					   cfg.pkcs9_email,
					   strlen (cfg.pkcs9_email));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crt_set (crt, "E-mail: ", GNUTLS_OID_PKCS9_EMAIL);
    }

}

int
get_serial (void)
{
  if (batch)
    {
      if (cfg.serial < 0)
	return 0;
      return cfg.serial;
    }
  else
    {
      return read_int ("Enter the certificate's serial number (decimal): ");
    }
}

int
get_days (void)
{
  int days;

  if (batch)
    {
      if (cfg.expiration_days <= 0)
	return 365;
      else
	return cfg.expiration_days;
    }
  else
    {
      do
	{
	  days = read_int ("The certificate will expire in (days): ");
	}
      while (days == 0);
      return days;
    }
}

int
get_ca_status (void)
{
  if (batch)
    {
      return cfg.ca;
    }
  else
    {
      return
	read_yesno ("Does the certificate belong to an authority? (Y/N): ");
    }
}

const char *
get_pkcs12_key_name (void)
{
  const char *name;

  if (batch)
    {
      if (!cfg.pkcs12_key_name)
	return "Anonymous";
      return cfg.pkcs12_key_name;
    }
  else
    {
      do
	{
	  name = read_str ("Enter a name for the key: ");
	}
      while (name == NULL);
    }
  return name;
}

int
get_tls_client_status (void)
{
  if (batch)
    {
      return cfg.tls_www_client;
    }
  else
    {
      return read_yesno ("Is this a TLS web client certificate? (Y/N): ");
    }
}

int
get_tls_server_status (void)
{
  if (batch)
    {
      return cfg.tls_www_server;
    }
  else
    {
      return
	read_yesno ("Is this also a TLS web server certificate? (Y/N): ");
    }
}

const char *
get_dns_name (void)
{
  if (batch)
    {
      return cfg.dns_name;
    }
  else
    {
      return
	read_str ("Enter the dnsName of the subject of the certificate: ");
    }
}

const char *
get_ip_addr (void)
{
  if (batch)
    {
      return cfg.ip_addr;
    }
  else
    {
      return
	read_str ("Enter the IP address of the subject of the certificate: ");
    }
}

const char *
get_email (void)
{
  if (batch)
    {
      return cfg.email;
    }
  else
    {
      return
	read_str ("Enter the e-mail of the subject of the certificate: ");
    }
}

int
get_sign_status (int server)
{
  const char *msg;

  if (batch)
    {
      return cfg.signing_key;
    }
  else
    {
      if (server)
	msg =
	  "Will the certificate be used for signing (DHE and RSA-EXPORT ciphersuites)? (Y/N): ";
      else
	msg =
	  "Will the certificate be used for signing (required for TLS)? (Y/N): ";
      return read_yesno (msg);
    }
}

int
get_encrypt_status (int server)
{
  const char *msg;

  if (batch)
    {
      return cfg.encryption_key;
    }
  else
    {
      if (server)
	msg =
	  "Will the certificate be used for encryption (RSA ciphersuites)? (Y/N): ";
      else
	msg =
	  "Will the certificate be used for encryption (not required for TLS)? (Y/N): ";
      return read_yesno (msg);
    }
}

int
get_cert_sign_status (void)
{
  if (batch)
    {
      return cfg.cert_sign_key;
    }
  else
    {
      return
	read_yesno
	("Will the certificate be used to sign other certificates? (Y/N): ");
    }
}

int
get_crl_sign_status (void)
{
  if (batch)
    {
      return cfg.crl_sign_key;
    }
  else
    {
      return
	read_yesno ("Will the certificate be used to sign CRLs? (Y/N): ");
    }
}

int
get_code_sign_status (void)
{
  if (batch)
    {
      return cfg.code_sign_key;
    }
  else
    {
      return
	read_yesno ("Will the certificate be used to sign code? (Y/N): ");
    }
}

int
get_ocsp_sign_status (void)
{
  if (batch)
    {
      return cfg.ocsp_sign_key;
    }
  else
    {
      return
	read_yesno
	("Will the certificate be used to sign OCSP requests? (Y/N): ");
    }
}

int
get_time_stamp_status (void)
{
  if (batch)
    {
      return cfg.time_stamping_key;
    }
  else
    {
      return
	read_yesno
	("Will the certificate be used for time stamping? (Y/N): ");
    }
}

int
get_crl_next_update (void)
{
  int days;

  if (batch)
    {
      if (cfg.crl_next_update <= 0)
	return 365;
      else
	return cfg.crl_next_update;
    }
  else
    {
      do
	{
	  days = read_int ("The next CRL will be issued in (days): ");
	}
      while (days == 0);
      return days;
    }
}

/* CRQ stuff.
 */
void
get_country_crq_set (gnutls_x509_crq crq)
{
  int ret;

  if (batch)
    {
      if (!cfg.country)
	return;
      ret =
	gnutls_x509_crq_set_dn_by_oid (crq,
				       GNUTLS_OID_X520_COUNTRY_NAME, 0,
				       cfg.country, strlen (cfg.country));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crq_set (crq, "Country name (2 chars): ",
		    GNUTLS_OID_X520_COUNTRY_NAME);
    }

}

void
get_organization_crq_set (gnutls_x509_crq crq)
{
  int ret;

  if (batch)
    {
      if (!cfg.organization)
	return;

      ret =
	gnutls_x509_crq_set_dn_by_oid (crq,
				       GNUTLS_OID_X520_ORGANIZATION_NAME,
				       0, cfg.organization,
				       strlen (cfg.organization));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crq_set (crq, "Organization name: ",
		    GNUTLS_OID_X520_ORGANIZATION_NAME);
    }

}

void
get_unit_crq_set (gnutls_x509_crq crq)
{
  int ret;

  if (batch)
    {
      if (!cfg.unit)
	return;

      ret =
	gnutls_x509_crq_set_dn_by_oid (crq,
				       GNUTLS_OID_X520_ORGANIZATIONAL_UNIT_NAME,
				       0, cfg.unit, strlen (cfg.unit));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crq_set (crq, "Organizational unit name: ",
		    GNUTLS_OID_X520_ORGANIZATIONAL_UNIT_NAME);
    }

}

void
get_state_crq_set (gnutls_x509_crq crq)
{
  int ret;

  if (batch)
    {
      if (!cfg.state)
	return;
      ret =
	gnutls_x509_crq_set_dn_by_oid (crq,
				       GNUTLS_OID_X520_STATE_OR_PROVINCE_NAME,
				       0, cfg.state, strlen (cfg.state));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crq_set (crq, "State or province name: ",
		    GNUTLS_OID_X520_STATE_OR_PROVINCE_NAME);
    }

}

void
get_locality_crq_set (gnutls_x509_crq crq)
{
  int ret;

  if (batch)
    {
      if (!cfg.locality)
	return;
      ret =
	gnutls_x509_crq_set_dn_by_oid (crq,
				       GNUTLS_OID_X520_LOCALITY_NAME, 0,
				       cfg.locality, strlen (cfg.locality));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crq_set (crq, "Locality name: ", GNUTLS_OID_X520_LOCALITY_NAME);
    }

}

void
get_cn_crq_set (gnutls_x509_crq crq)
{
  int ret;

  if (batch)
    {
      if (!cfg.cn)
	return;
      ret =
	gnutls_x509_crq_set_dn_by_oid (crq, GNUTLS_OID_X520_COMMON_NAME,
				       0, cfg.cn, strlen (cfg.cn));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crq_set (crq, "Common name: ", GNUTLS_OID_X520_COMMON_NAME);
    }

}

void
get_uid_crq_set (gnutls_x509_crq crq)
{
  int ret;

  if (batch)
    {
      if (!cfg.uid)
	return;
      ret = gnutls_x509_crq_set_dn_by_oid (crq, GNUTLS_OID_LDAP_UID, 0,
					   cfg.uid, strlen (cfg.uid));
      if (ret < 0)
	{
	  fprintf (stderr, "set_dn: %s\n", gnutls_strerror (ret));
	  exit (1);
	}
    }
  else
    {
      read_crq_set (crq, "UID: ", GNUTLS_OID_LDAP_UID);
    }

}

void
get_oid_crq_set (gnutls_x509_crq crq)
{
  int ret, i;

  if (batch)
    {
      if (!cfg.dn_oid)
	return;
      for (i = 0; cfg.dn_oid[i] != NULL; i += 2)
	{
	  if (cfg.dn_oid[i + 1] == NULL)
	    {
	      fprintf (stderr, "dn_oid: %s does not have an argument.\n",
		       cfg.dn_oid[i]);
	      exit (1);
	    }
	  ret = gnutls_x509_crq_set_dn_by_oid (crq, cfg.dn_oid[i], 0,
					       cfg.dn_oid[i + 1],
					       strlen (cfg.dn_oid[i + 1]));

	  if (ret < 0)
	    {
	      fprintf (stderr, "set_dn_oid: %s\n", gnutls_strerror (ret));
	      exit (1);
	    }
	}
    }

}


#endif
