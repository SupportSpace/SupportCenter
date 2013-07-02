/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005 Free Software Foundation
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

#include "gnutls_int.h"
#include "gnutls_errors.h"
#include "gnutls_auth.h"
#include "gnutls_auth_int.h"
#include "gnutls_algorithms.h"
#include "auth_cert.h"
#include <gnutls_datum.h>

#include "auth_anon.h"
/* The functions here are used in order for authentication algorithms
 * to be able to retrieve the needed credentials eg public and private
 * key etc.
 */

/**
  * gnutls_credentials_clear - Clears all the credentials previously set
  * @session: is a #gnutls_session_t structure.
  *
  * Clears all the credentials previously set in this session.
  *
  **/
void
gnutls_credentials_clear (gnutls_session_t session)
{
  if (session->key && session->key->cred)
    {				/* beginning of the list */
      auth_cred_st *ccred, *ncred;
      ccred = session->key->cred;
      while (ccred != NULL)
	{
	  ncred = ccred->next;
	  gnutls_free (ccred);
	  ccred = ncred;
	}
      session->key->cred = NULL;
    }
}

/* 
 * This creates a linked list of the form:
 * { algorithm, credentials, pointer to next }
 */
/**
  * gnutls_credentials_set - Sets the needed credentials for the specified authentication algorithm.
  * @session: is a #gnutls_session_t structure.
  * @type: is the type of the credentials
  * @cred: is a pointer to a structure.
  *
  * Sets the needed credentials for the specified type.
  * Eg username, password - or public and private keys etc.  
  * The (void* cred) parameter is a structure that depends on the
  * specified type and on the current session (client or server).
  * [ In order to minimize memory usage, and share credentials between 
  * several threads gnutls keeps a pointer to cred, and not the whole cred
  * structure. Thus you will have to keep the structure allocated until   
  * you call gnutls_deinit(). ]
  *
  * For GNUTLS_CRD_ANON cred should be gnutls_anon_client_credentials_t in case of a client.
  * In case of a server it should be gnutls_anon_server_credentials_t.
  * 
  * For GNUTLS_CRD_SRP cred should be gnutls_srp_client_credentials_t
  * in case of a client, and gnutls_srp_server_credentials_t, in case
  * of a server.
  *
  * For GNUTLS_CRD_CERTIFICATE cred should be gnutls_certificate_credentials_t.
  *
  **/
int
gnutls_credentials_set (gnutls_session_t session,
			gnutls_credentials_type_t type, void *cred)
{
  auth_cred_st *ccred = NULL, *pcred = NULL;
  int exists = 0;

  if (session->key->cred == NULL)
    {				/* beginning of the list */

      session->key->cred = gnutls_malloc (sizeof (auth_cred_st));
      if (session->key->cred == NULL)
	return GNUTLS_E_MEMORY_ERROR;

      /* copy credentials locally */
      session->key->cred->credentials = cred;

      session->key->cred->next = NULL;
      session->key->cred->algorithm = type;
    }
  else
    {
      ccred = session->key->cred;
      while (ccred != NULL)
	{
	  if (ccred->algorithm == type)
	    {
	      exists = 1;
	      break;
	    }
	  pcred = ccred;
	  ccred = ccred->next;
	}
      /* After this, pcred is not null.
       */

      if (exists == 0)
	{			/* new entry */
	  pcred->next = gnutls_malloc (sizeof (auth_cred_st));
	  if (pcred->next == NULL)
	    return GNUTLS_E_MEMORY_ERROR;

	  ccred = pcred->next;

	  /* copy credentials locally */
	  ccred->credentials = cred;

	  ccred->next = NULL;
	  ccred->algorithm = type;
	}
      else
	{			/* modify existing entry */
	  gnutls_free (ccred->credentials);
	  ccred->credentials = cred;
	}
    }

  return 0;
}

/**
  * gnutls_auth_get_type - Returns the type of credentials for the current authentication schema.
  * @session: is a #gnutls_session_t structure.
  *
  * Returns type of credentials for the current authentication schema.
  * The returned information is to be used to distinguish the function used
  * to access authentication data.
  * 
  * Eg. for CERTIFICATE ciphersuites (key exchange algorithms: KX_RSA, KX_DHE_RSA),
  * the same function are to be used to access the authentication data.
  **/
gnutls_credentials_type_t
gnutls_auth_get_type (gnutls_session_t session)
{
/* This is not the credentials we must set, but the authentication data
 * we get by the peer, so it should be reversed.
 */
  int server = session->security_parameters.entity == GNUTLS_SERVER ? 0 : 1;

  return
    _gnutls_map_kx_get_cred (_gnutls_cipher_suite_get_kx_algo
			     (&session->security_parameters.
			      current_cipher_suite), server);
}

/**
  * gnutls_auth_server_get_type - Returns the type of credentials for the server authentication schema.
  * @session: is a #gnutls_session_t structure.
  *
  * Returns the type of credentials that were used for server authentication.
  * The returned information is to be used to distinguish the function used
  * to access authentication data.
  * 
  **/
gnutls_credentials_type_t
gnutls_auth_server_get_type (gnutls_session_t session)
{
  return
    _gnutls_map_kx_get_cred (_gnutls_cipher_suite_get_kx_algo
			     (&session->security_parameters.
			      current_cipher_suite), 1);
}

/**
  * gnutls_auth_client_get_type - Returns the type of credentials for the client authentication schema.
  * @session: is a #gnutls_session_t structure.
  *
  * Returns the type of credentials that were used for client authentication.
  * The returned information is to be used to distinguish the function used
  * to access authentication data.
  * 
  **/
gnutls_credentials_type_t
gnutls_auth_client_get_type (gnutls_session_t session)
{
  return
    _gnutls_map_kx_get_cred (_gnutls_cipher_suite_get_kx_algo
			     (&session->security_parameters.
			      current_cipher_suite), 0);
}


/* 
 * This returns a pointer to the linked list. Don't
 * free that!!!
 */
const void *
_gnutls_get_kx_cred (gnutls_session_t session,
		     gnutls_kx_algorithm_t algo, int *err)
{
  int server = session->security_parameters.entity == GNUTLS_SERVER ? 1 : 0;

  return _gnutls_get_cred (session->key,
			   _gnutls_map_kx_get_cred (algo, server), err);
}

const void *
_gnutls_get_cred (gnutls_key_st key, gnutls_credentials_type_t type, int *err)
{
  const void *retval = NULL;
  int _err = -1;
  auth_cred_st *ccred;

  if (key == NULL)
    goto out;

  ccred = key->cred;
  while (ccred != NULL)
    {
      if (ccred->algorithm == type)
	{
	  break;
	}
      ccred = ccred->next;
    }
  if (ccred == NULL)
    goto out;

  _err = 0;
  retval = ccred->credentials;

out:
  if (err != NULL)
    *err = _err;
  return retval;
}

/*-
  * _gnutls_get_auth_info - Returns a pointer to authentication information.
  * @session: is a #gnutls_session_t structure.
  *
  * This function must be called after a succesful gnutls_handshake().
  * Returns a pointer to authentication information. That information
  * is data obtained by the handshake protocol, the key exchange algorithm,
  * and the TLS extensions messages.
  *
  * In case of GNUTLS_CRD_ANON returns a type of &anon_(server/client)_auth_info_t;
  * In case of GNUTLS_CRD_CERTIFICATE returns a type of &cert_auth_info_t;
  * In case of GNUTLS_CRD_SRP returns a type of &srp_(server/client)_auth_info_t;
  -*/
void *
_gnutls_get_auth_info (gnutls_session_t session)
{
  return session->key->auth_info;
}

/*-
  * _gnutls_free_auth_info - Frees the auth info structure
  * @session: is a #gnutls_session_t structure.
  *
  * This function frees the auth info structure and sets it to
  * null. It must be called since some structures contain malloced
  * elements.
  -*/
void
_gnutls_free_auth_info (gnutls_session_t session)
{
  dh_info_st *dh_info;
  rsa_info_st *rsa_info;

  if (session == NULL || session->key == NULL)
    {
      gnutls_assert ();
      return;
    }

  switch (session->key->auth_info_type)
    {
    case GNUTLS_CRD_SRP:
      break;
    case GNUTLS_CRD_ANON:
      {
	anon_auth_info_t info = _gnutls_get_auth_info (session);

	if (info == NULL)
	  break;

	dh_info = &info->dh;
	_gnutls_free_dh_info (dh_info);
      }
      break;
    case GNUTLS_CRD_CERTIFICATE:
      {
	unsigned int i;
	cert_auth_info_t info = _gnutls_get_auth_info (session);

	if (info == NULL)
	  break;

	dh_info = &info->dh;
	rsa_info = &info->rsa_export;
	for (i = 0; i < info->ncerts; i++)
	  {
	    _gnutls_free_datum (&info->raw_certificate_list[i]);
	  }

	gnutls_free (info->raw_certificate_list);
	info->raw_certificate_list = NULL;
	info->ncerts = 0;

	_gnutls_free_dh_info (dh_info);
	_gnutls_free_rsa_info (rsa_info);
      }


      break;
    default:
      return;

    }

  gnutls_free (session->key->auth_info);
  session->key->auth_info = NULL;
  session->key->auth_info_size = 0;
  session->key->auth_info_type = 0;

}

/* This function will set the auth info structure in the key
 * structure.
 * If allow change is !=0 then this will allow changing the auth
 * info structure to a different type.
 */
int
_gnutls_auth_info_set (gnutls_session_t session,
		       gnutls_credentials_type_t type, int size,
		       int allow_change)
{
  if (session->key->auth_info == NULL)
    {
      session->key->auth_info = gnutls_calloc (1, size);
      if (session->key->auth_info == NULL)
	{
	  gnutls_assert ();
	  return GNUTLS_E_MEMORY_ERROR;
	}
      session->key->auth_info_type = type;
      session->key->auth_info_size = size;
    }
  else
    {
      if (allow_change == 0)
	{
	  /* If the credentials for the current authentication scheme,
	   * are not the one we want to set, then it's an error.
	   * This may happen if a rehandshake is performed an the
	   * ciphersuite which is negotiated has different authentication
	   * schema.
	   */
	  if (gnutls_auth_get_type (session) != session->key->auth_info_type)
	    {
	      gnutls_assert ();
	      return GNUTLS_E_INVALID_REQUEST;
	    }
	}
      else
	{
	  /* The new behaviour: Here we reallocate the auth info structure
	   * in order to be able to negotiate different authentication
	   * types. Ie. perform an auth_anon and then authenticate again using a
	   * certificate (in order to prevent revealing the certificate's contents,
	   * to passive eavesdropers.
	   */
	  if (gnutls_auth_get_type (session) != session->key->auth_info_type)
	    {
	      session->key->auth_info =
		gnutls_realloc_fast (session->key->auth_info, size);
	      if (session->key->auth_info == NULL)
		{
		  gnutls_assert ();
		  return GNUTLS_E_MEMORY_ERROR;
		}
	      memset (session->key->auth_info, 0, size);
	      session->key->auth_info_type = type;
	      session->key->auth_info_size = size;
	    }
	}
    }
  return 0;
}
