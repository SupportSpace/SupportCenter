/*
 * Copyright (C) 2001, 2004, 2005 Free Software Foundation
 *
 * Author: Nikos Mavroyanopoulos
 *
 * This file is part of GNUTLS.
 *
 * The GNUTLS library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA
 *
 */

#include <gnutls_int.h>
#include <gnutls_errors.h>
#include <x509_b64.h>
#include <auth_cert.h>
#include <gnutls_cert.h>
#include <libtasn1.h>
#include <gnutls_datum.h>
#include <gnutls_mpi.h>
#include <gnutls_global.h>
#include <gnutls_pk.h>
#include <debug.h>
#include <gnutls_buffers.h>
#include <gnutls_sig.h>
#include <gnutls_kx.h>

static
  int _gnutls_tls_sign (gnutls_cert * cert, gnutls_privkey * pkey,
			const gnutls_datum_t * hash_concat,
			gnutls_datum_t * signature);


/* Generates a signature of all the previous sent packets in the 
 * handshake procedure. (20040227: now it works for SSL 3.0 as well)
 */
int
_gnutls_tls_sign_hdata (gnutls_session_t session,
			gnutls_cert * cert, gnutls_privkey * pkey,
			gnutls_datum_t * signature)
{
  gnutls_datum_t dconcat;
  int ret;
  opaque concat[36];
  mac_hd_t td_md5;
  mac_hd_t td_sha;
  gnutls_protocol_t ver = gnutls_protocol_get_version (session);

  td_sha = _gnutls_hash_copy (session->internals.handshake_mac_handle_sha);
  if (td_sha == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_HASH_FAILED;
    }

  if (ver == GNUTLS_SSL3)
    {
      ret = _gnutls_generate_master (session, 1);
      if (ret < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      _gnutls_mac_deinit_ssl3_handshake (td_sha, &concat[16],
					 session->security_parameters.
					 master_secret, TLS_MASTER_SIZE);
    }
  else
    _gnutls_hash_deinit (td_sha, &concat[16]);

  switch (cert->subject_pk_algorithm)
    {
    case GNUTLS_PK_RSA:
      td_md5 =
	_gnutls_hash_copy (session->internals.handshake_mac_handle_md5);
      if (td_md5 == NULL)
	{
	  gnutls_assert ();
	  return GNUTLS_E_HASH_FAILED;
	}

      if (ver == GNUTLS_SSL3)
	_gnutls_mac_deinit_ssl3_handshake (td_md5, concat,
					   session->security_parameters.
					   master_secret, TLS_MASTER_SIZE);
      else
	_gnutls_hash_deinit (td_md5, concat);

      dconcat.data = concat;
      dconcat.size = 36;
      break;
    case GNUTLS_PK_DSA:
      dconcat.data = &concat[16];
      dconcat.size = 20;
      break;

    default:
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
    }
  ret = _gnutls_tls_sign (cert, pkey, &dconcat, signature);
  if (ret < 0)
    {
      gnutls_assert ();
    }

  return ret;
}


/* Generates a signature of all the random data and the parameters.
 * Used in DHE_* ciphersuites.
 */
int
_gnutls_tls_sign_params (gnutls_session_t session, gnutls_cert * cert,
			 gnutls_privkey * pkey, gnutls_datum_t * params,
			 gnutls_datum_t * signature)
{
  gnutls_datum_t dconcat;
  int ret;
  mac_hd_t td_md5;
  mac_hd_t td_sha;
  opaque concat[36];


  td_sha = _gnutls_hash_init (GNUTLS_MAC_SHA1);
  if (td_sha == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_HASH_FAILED;
    }

  _gnutls_hash (td_sha, session->security_parameters.client_random,
		TLS_RANDOM_SIZE);
  _gnutls_hash (td_sha, session->security_parameters.server_random,
		TLS_RANDOM_SIZE);
  _gnutls_hash (td_sha, params->data, params->size);

  _gnutls_hash_deinit (td_sha, &concat[16]);

  switch (cert->subject_pk_algorithm)
    {
    case GNUTLS_PK_RSA:
      td_md5 = _gnutls_hash_init (GNUTLS_MAC_MD5);
      if (td_md5 == NULL)
	{
	  gnutls_assert ();
	  return GNUTLS_E_HASH_FAILED;
	}

      _gnutls_hash (td_md5, session->security_parameters.client_random,
		    TLS_RANDOM_SIZE);
      _gnutls_hash (td_md5, session->security_parameters.server_random,
		    TLS_RANDOM_SIZE);
      _gnutls_hash (td_md5, params->data, params->size);

      _gnutls_hash_deinit (td_md5, concat);

      dconcat.data = concat;
      dconcat.size = 36;
      break;
    case GNUTLS_PK_DSA:
      dconcat.data = &concat[16];
      dconcat.size = 20;
      break;

    default:
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
    }
  ret = _gnutls_tls_sign (cert, pkey, &dconcat, signature);
  if (ret < 0)
    {
      gnutls_assert ();
    }

  return ret;

}


/* This will create a PKCS1 or DSA signature, using the given parameters, and the
 * given data. The output will be allocated and be put in signature.
 */
int
_gnutls_sign (gnutls_pk_algorithm_t algo, mpi_t * params,
	      int params_size, const gnutls_datum_t * data,
	      gnutls_datum_t * signature)
{
  int ret;

  switch (algo)
    {
    case GNUTLS_PK_RSA:
      /* encrypt */
      if ((ret = _gnutls_pkcs1_rsa_encrypt (signature, data, params,
					    params_size, 1)) < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      break;
    case GNUTLS_PK_DSA:
      /* sign */
      if ((ret = _gnutls_dsa_sign (signature, data, params, params_size)) < 0)
	{
	  gnutls_assert ();
	  return ret;
	}
      break;
    default:
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
      break;
    }

  return 0;
}

/* This will create a PKCS1 or DSA signature, as defined in the TLS protocol.
 * Cert is the certificate of the corresponding private key. It is only checked if
 * it supports signing.
 */
static int
_gnutls_tls_sign (gnutls_cert * cert, gnutls_privkey * pkey,
		  const gnutls_datum_t * hash_concat,
		  gnutls_datum_t * signature)
{

  /* If our certificate supports signing
   */

  if (cert != NULL)
    if (cert->key_usage != 0)
      if (!(cert->key_usage & KEY_DIGITAL_SIGNATURE))
	{
	  gnutls_assert ();
	  return GNUTLS_E_KEY_USAGE_VIOLATION;
	}

  return _gnutls_sign (pkey->pk_algorithm, pkey->params,
		       pkey->params_size, hash_concat, signature);

}


static int
_gnutls_pkcs1_rsa_verify_sig (gnutls_cert * cert,
			      const gnutls_datum_t * hash_concat,
			      gnutls_datum_t * signature)
{
  int ret;
  gnutls_datum_t vdata;

  if (cert->version == 0 || cert == NULL)
    {				/* this is the only way to check
				 * if it is initialized
				 */
      gnutls_assert ();
      return GNUTLS_E_CERTIFICATE_ERROR;
    }

  /* If the certificate supports signing continue.
   */
  if (cert != NULL)
    if (cert->key_usage != 0)
      if (!(cert->key_usage & KEY_DIGITAL_SIGNATURE))
	{
	  gnutls_assert ();
	  return GNUTLS_E_KEY_USAGE_VIOLATION;
	}

  switch (cert->subject_pk_algorithm)
    {
    case GNUTLS_PK_RSA:

      vdata.data = hash_concat->data;
      vdata.size = hash_concat->size;

      /* verify signature */
      if ((ret = _gnutls_rsa_verify (&vdata, signature, cert->params,
				     cert->params_size, 1)) < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      break;
    case GNUTLS_PK_DSA:

      vdata.data = &hash_concat->data[16];
      vdata.size = 20;		/* sha1 */

      /* verify signature */
      if ((ret = _gnutls_dsa_verify (&vdata, signature, cert->params,
				     cert->params_size)) < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      break;

    default:
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
    }



  return 0;
}


/* Verifies a TLS signature (like the one in the client certificate
 * verify message). 
 */
int
_gnutls_verify_sig_hdata (gnutls_session_t session, gnutls_cert * cert,
			  gnutls_datum_t * signature)
{
  int ret;
  opaque concat[36];
  mac_hd_t td_md5;
  mac_hd_t td_sha;
  gnutls_datum_t dconcat;
  gnutls_protocol_t ver = gnutls_protocol_get_version (session);

  td_md5 = _gnutls_hash_copy (session->internals.handshake_mac_handle_md5);
  if (td_md5 == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_HASH_FAILED;
    }

  td_sha = _gnutls_hash_copy (session->internals.handshake_mac_handle_sha);
  if (td_sha == NULL)
    {
      gnutls_assert ();
      _gnutls_hash_deinit (td_md5, NULL);
      return GNUTLS_E_HASH_FAILED;
    }

  if (ver == GNUTLS_SSL3)
    {
      ret = _gnutls_generate_master (session, 1);
      if (ret < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      _gnutls_mac_deinit_ssl3_handshake (td_md5, concat,
					 session->security_parameters.
					 master_secret, TLS_MASTER_SIZE);
      _gnutls_mac_deinit_ssl3_handshake (td_sha, &concat[16],
					 session->security_parameters.
					 master_secret, TLS_MASTER_SIZE);
    }
  else
    {
      _gnutls_hash_deinit (td_md5, concat);
      _gnutls_hash_deinit (td_sha, &concat[16]);
    }

  dconcat.data = concat;
  dconcat.size = 20 + 16;	/* md5+ sha */

  ret = _gnutls_pkcs1_rsa_verify_sig (cert, &dconcat, signature);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  return ret;

}

/* Generates a signature of all the random data and the parameters.
 * Used in DHE_* ciphersuites.
 */
int
_gnutls_verify_sig_params (gnutls_session_t session, gnutls_cert * cert,
			   const gnutls_datum_t * params,
			   gnutls_datum_t * signature)
{
  gnutls_datum_t dconcat;
  int ret;
  mac_hd_t td_md5;
  mac_hd_t td_sha;
  opaque concat[36];

  td_md5 = _gnutls_hash_init (GNUTLS_MAC_MD5);
  if (td_md5 == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_HASH_FAILED;
    }

  td_sha = _gnutls_hash_init (GNUTLS_MAC_SHA1);
  if (td_sha == NULL)
    {
      gnutls_assert ();
      _gnutls_hash_deinit (td_md5, NULL);
      return GNUTLS_E_HASH_FAILED;
    }

  _gnutls_hash (td_md5, session->security_parameters.client_random,
		TLS_RANDOM_SIZE);
  _gnutls_hash (td_md5, session->security_parameters.server_random,
		TLS_RANDOM_SIZE);
  _gnutls_hash (td_md5, params->data, params->size);

  _gnutls_hash (td_sha, session->security_parameters.client_random,
		TLS_RANDOM_SIZE);
  _gnutls_hash (td_sha, session->security_parameters.server_random,
		TLS_RANDOM_SIZE);
  _gnutls_hash (td_sha, params->data, params->size);

  _gnutls_hash_deinit (td_md5, concat);
  _gnutls_hash_deinit (td_sha, &concat[16]);

  dconcat.data = concat;
  dconcat.size = 36;

  ret = _gnutls_pkcs1_rsa_verify_sig (cert, &dconcat, signature);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  return ret;

}
