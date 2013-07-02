/*
 * Copyright (C) 2002, 2003, 2004, 2005 Free Software Foundation
 *
 * Author: Timo Schulz, Nikos Mavroyanopoulos
 *
 * This file is part of GNUTLS-EXTRA.
 *
 * GNUTLS-EXTRA is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * GNUTLS-EXTRA is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNUTLS-EXTRA; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

/* Compatibility functions on OpenPGP key parsing.
 */

#include <gnutls_int.h>
#include <gnutls_errors.h>
#include <gnutls_openpgp.h>
#include <openpgp.h>

/*-
 * gnutls_openpgp_verify_key - Verify all signatures on the key
 * @cert_list: the structure that holds the certificates.
 * @cert_list_lenght: the items in the cert_list.
 * @status: the output of the verification function
 *
 * Verify all signatures in the certificate list. When the key
 * is not available, the signature is skipped.
 *
 * When the trustdb parameter is used, the function checks the
 * ownertrust of the key before the signatures are checked. It
 * is possible that the key was disabled or the owner is not trusted
 * at all. Then we don't check the signatures because it makes no sense.
 *
 * The return value is one of the CertificateStatus entries.
 *
 * NOTE: this function does not verify using any "web of trust". You
 * may use GnuPG for that purpose, or any other external PGP application.
 -*/
int
_gnutls_openpgp_verify_key (const gnutls_certificate_credentials_t cred,
			    const gnutls_datum_t * cert_list,
			    int cert_list_length, unsigned int *status)
{
  int ret = 0;
  gnutls_openpgp_key_t key = NULL;
  gnutls_openpgp_keyring_t ring = NULL;
  gnutls_openpgp_trustdb_t tdb = NULL;
  unsigned int verify_ring = 0, verify_db = 0, verify_self = 0;

  if (!cert_list || cert_list_length != 1)
    {
      gnutls_assert ();
      return GNUTLS_E_NO_CERTIFICATE_FOUND;
    }

  ret = gnutls_openpgp_key_init (&key);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  ret = gnutls_openpgp_key_import (key, &cert_list[0], 0);
  if (ret < 0)
    {
      gnutls_assert ();
      goto leave;
    }

  if (cred->keyring.data && cred->keyring.size != 0)
    {

      /* use the keyring
       */
      ret = gnutls_openpgp_keyring_init (&ring);
      if (ret < 0)
	{
	  gnutls_assert ();
	  goto leave;
	}

      ret = gnutls_openpgp_keyring_import (ring, &cred->keyring, 0);
      if (ret < 0)
	{
	  gnutls_assert ();
	  goto leave;
	}

      ret = gnutls_openpgp_key_verify_ring (key, ring, 0, &verify_ring);
      if (ret < 0)
	{
	  gnutls_assert ();
	  goto leave;
	}
    }

  if (cred->pgp_trustdb)
    {				/* Use the trustDB */
      ret = gnutls_openpgp_trustdb_init (&tdb);
      if (ret < 0)
	{
	  gnutls_assert ();
	  goto leave;
	}

      ret = gnutls_openpgp_trustdb_import_file (tdb, cred->pgp_trustdb);
      if (ret < 0)
	{
	  gnutls_assert ();
	  goto leave;
	}

      ret = gnutls_openpgp_key_verify_trustdb (key, tdb, 0, &verify_db);
    }

  /* now try the self signature.
   */
  ret = gnutls_openpgp_key_verify_self (key, 0, &verify_self);
  if (ret < 0)
    {
      gnutls_assert ();
      goto leave;
    }

  *status = verify_self | verify_ring | verify_db;

  /* If we only checked the self signature.
   */
  if (!cred->pgp_trustdb && !cred->keyring.data)
    *status |= GNUTLS_CERT_SIGNER_NOT_FOUND;

  ret = 0;

leave:
  gnutls_openpgp_key_deinit (key);
  gnutls_openpgp_trustdb_deinit (tdb);
  gnutls_openpgp_keyring_deinit (ring);

  return ret;
}

/*-
 * gnutls_openpgp_fingerprint - Gets the fingerprint
 * @cert: the raw data that contains the OpenPGP public key.
 * @fpr: the buffer to save the fingerprint.
 * @fprlen: the integer to save the length of the fingerprint.
 *
 * Returns the fingerprint of the OpenPGP key. Depence on the algorithm,
 * the fingerprint can be 16 or 20 bytes.
 -*/
int
_gnutls_openpgp_fingerprint (const gnutls_datum_t * cert,
			     unsigned char *fpr, size_t * fprlen)
{
  gnutls_openpgp_key_t key;
  int ret;

  ret = gnutls_openpgp_key_init (&key);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  ret = gnutls_openpgp_key_import (key, cert, 0);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  ret = gnutls_openpgp_key_get_fingerprint (key, fpr, fprlen);

  gnutls_openpgp_key_deinit (key);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  return 0;
}

/*-
 * gnutls_openpgp_get_raw_key_creation_time - Extract the timestamp
 * @cert: the raw data that contains the OpenPGP public key.
 *
 * Returns the timestamp when the OpenPGP key was created.
 -*/
time_t
_gnutls_openpgp_get_raw_key_creation_time (const gnutls_datum_t * cert)
{
  gnutls_openpgp_key_t key;
  int ret;
  time_t tim;

  ret = gnutls_openpgp_key_init (&key);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  ret = gnutls_openpgp_key_import (key, cert, 0);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  tim = gnutls_openpgp_key_get_creation_time (key);

  gnutls_openpgp_key_deinit (key);

  return tim;
}


/*-
 * gnutls_openpgp_get_raw_key_expiration_time - Extract the expire date
 * @cert: the raw data that contains the OpenPGP public key.
 *
 * Returns the time when the OpenPGP key expires. A value of '0' means
 * that the key doesn't expire at all.
 -*/
time_t
_gnutls_openpgp_get_raw_key_expiration_time (const gnutls_datum_t * cert)
{
  gnutls_openpgp_key_t key;
  int ret;
  time_t tim;

  ret = gnutls_openpgp_key_init (&key);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  ret = gnutls_openpgp_key_import (key, cert, 0);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  tim = gnutls_openpgp_key_get_expiration_time (key);

  gnutls_openpgp_key_deinit (key);

  return tim;
}
