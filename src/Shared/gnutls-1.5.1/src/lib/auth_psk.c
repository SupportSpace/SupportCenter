/*
 * Copyright (C) 2005 Free Software Foundation
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

#ifdef ENABLE_PSK

#include "gnutls_errors.h"
#include "gnutls_auth.h"
#include "gnutls_auth_int.h"
#include "debug.h"
#include "gnutls_num.h"
#include <auth_psk.h>
#include <auth_psk_passwd.h>
#include <gnutls_str.h>
#include <gnutls_datum.h>

int _gnutls_gen_psk_client_kx (gnutls_session_t, opaque **);

int _gnutls_proc_psk_client_kx (gnutls_session_t, opaque *, size_t);

const mod_auth_st psk_auth_struct = {
  "PSK",
  NULL,
  NULL,
  NULL,
  _gnutls_gen_psk_client_kx,
  NULL,
  NULL,

  NULL,
  NULL,				/* certificate */
  NULL,
  _gnutls_proc_psk_client_kx,
  NULL,
  NULL
};

/* Set the PSK premaster secret.
 */
int
_gnutls_set_psk_session_key (gnutls_session_t session,
			     gnutls_datum * dh_secret)
{
  gnutls_datum pwd_psk = { NULL, 0 };
  gnutls_datum *ppsk;
  size_t dh_secret_size;
  int ret;

  if (session->security_parameters.entity == GNUTLS_CLIENT)
    {
      gnutls_psk_client_credentials_t cred;

      cred = (gnutls_psk_client_credentials_t)
	_gnutls_get_cred (session->key, GNUTLS_CRD_PSK, NULL);

      if (cred == NULL)
	{
	  gnutls_assert ();
	  return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
	}

      ppsk = &cred->key;

    }
  else
    {				/* SERVER side */
      psk_auth_info_t info;

      info = _gnutls_get_auth_info (session);

      /* find the key of this username
       */
      ret = _gnutls_psk_pwd_find_entry (session, info->username, &pwd_psk);
      if (ret < 0)
	{
	  gnutls_assert ();
	  return ret;
	}
      ppsk = &pwd_psk;
    }


  if (dh_secret == NULL)
    dh_secret_size = ppsk->size;
  else
    dh_secret_size = dh_secret->size;

  /* set the session key
   */
  session->key->key.size = 4 + dh_secret_size + ppsk->size;
  session->key->key.data = gnutls_malloc (session->key->key.size);
  if (session->key->key.data == NULL)
    {
      gnutls_assert ();
      ret = GNUTLS_E_MEMORY_ERROR;
      goto error;
    }

  /* format of the premaster secret:
   * (uint16_t) psk_size
   * psk_size bytes of zeros
   * (uint16_t) psk_size
   * the psk
   */
  _gnutls_write_uint16 (dh_secret_size, session->key->key.data);
  if (dh_secret == NULL)
    memset (&session->key->key.data[2], 0, dh_secret_size);
  else
    memcpy (&session->key->key.data[2], dh_secret->data, dh_secret->size);
  _gnutls_write_datum16 (&session->key->key.data[dh_secret_size + 2], *ppsk);

  ret = 0;

error:
  _gnutls_free_datum (&pwd_psk);
  return ret;
}


/* Generates the PSK client key exchange
 *
 * 
 * struct {
 *    select (KeyExchangeAlgorithm) {
 *       opaque psk_identity<0..2^16-1>;
 *    } exchange_keys;
 * } ClientKeyExchange;
 *
 */
int
_gnutls_gen_psk_client_kx (gnutls_session_t session, opaque ** data)
{
  int ret;
  gnutls_psk_client_credentials_t cred;

  cred = (gnutls_psk_client_credentials_t)
    _gnutls_get_cred (session->key, GNUTLS_CRD_PSK, NULL);

  if (cred == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
    }

  if (cred->username.data == NULL || cred->key.data == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
    }

  ret = _gnutls_set_psk_session_key (session, NULL);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  (*data) = gnutls_malloc (2 + cred->username.size);
  if ((*data) == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_MEMORY_ERROR;
    }

  _gnutls_write_datum16 (*data, cred->username);

  return (cred->username.size + 2);
}


/* just read the username from the client key exchange.
 */
int
_gnutls_proc_psk_client_kx (gnutls_session_t session, opaque * data,
			    size_t _data_size)
{
  ssize_t data_size = _data_size;
  int ret;
  gnutls_datum username;
  gnutls_psk_server_credentials_t cred;
  psk_auth_info_t info;

  cred = (gnutls_psk_server_credentials_t)
    _gnutls_get_cred (session->key, GNUTLS_CRD_PSK, NULL);

  if (cred == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INSUFFICIENT_CREDENTIALS;
    }

  if ((ret =
       _gnutls_auth_info_set (session, GNUTLS_CRD_PSK,
			      sizeof (psk_auth_info_st), 1)) < 0)
    {
      gnutls_assert ();
      return ret;
    }

  DECR_LEN (data_size, 2);
  username.size = _gnutls_read_uint16 (&data[0]);

  DECR_LEN (data_size, username.size);

  username.data = &data[2];


  /* copy the username to the auth info structures
   */
  info = _gnutls_get_auth_info (session);

  if (username.size > MAX_SRP_USERNAME)
    {
      gnutls_assert ();
      return GNUTLS_E_ILLEGAL_SRP_USERNAME;
    }

  memcpy (info->username, username.data, username.size);
  info->username[username.size] = 0;

  ret = _gnutls_set_psk_session_key (session, NULL);
  if (ret < 0)
    {
      gnutls_assert ();
      goto error;
    }

  ret = 0;

error:
  return ret;
}


#endif /* ENABLE_SRP */
