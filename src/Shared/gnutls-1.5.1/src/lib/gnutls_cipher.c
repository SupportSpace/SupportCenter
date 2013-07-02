/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation
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

/* Some high level functions to be used in the record encryption are
 * included here.
 */

#include "gnutls_int.h"
#include "gnutls_errors.h"
#include "gnutls_compress.h"
#include "gnutls_cipher.h"
#include "gnutls_algorithms.h"
#include "gnutls_hash_int.h"
#include "gnutls_cipher_int.h"
#include "debug.h"
#include "gnutls_num.h"
#include "gnutls_datum.h"
#include "gnutls_kx.h"
#include "gnutls_record.h"
#include "gnutls_constate.h"
#include <gc.h>

inline static int
is_write_comp_null (gnutls_session_t session)
{
  if (session->security_parameters.write_compression_algorithm ==
      GNUTLS_COMP_NULL)
    return 0;

  return 1;
}

inline static int
is_read_comp_null (gnutls_session_t session)
{
  if (session->security_parameters.read_compression_algorithm ==
      GNUTLS_COMP_NULL)
    return 0;

  return 1;
}


/* returns ciphertext which contains the headers too. This also
 * calculates the size in the header field.
 * 
 * If random pad != 0 then the random pad data will be appended.
 */
int
_gnutls_encrypt (gnutls_session_t session, const opaque * headers,
		 size_t headers_size, const opaque * data,
		 size_t data_size, opaque * ciphertext,
		 size_t ciphertext_size, content_type_t type, int random_pad)
{
  gnutls_datum_t plain;
  gnutls_datum_t comp;
  int ret;
  int free_comp = 1;

  plain.data = (opaque *) data;
  plain.size = data_size;

  if (plain.size == 0 || is_write_comp_null (session) == 0)
    {
      comp = plain;
      free_comp = 0;
    }
  else
    {
      /* Here comp is allocated and must be 
       * freed.
       */
      ret = _gnutls_m_plaintext2compressed (session, &comp, plain);
      if (ret < 0)
	{
	  gnutls_assert ();
	  return ret;
	}
    }

  ret = _gnutls_compressed2ciphertext (session, &ciphertext[headers_size],
				       ciphertext_size - headers_size,
				       comp, type, random_pad);

  if (free_comp)
    _gnutls_free_datum (&comp);

  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }


  /* copy the headers */
  memcpy (ciphertext, headers, headers_size);
  _gnutls_write_uint16 (ret, &ciphertext[3]);

  return ret + headers_size;
}

/* Decrypts the given data.
 * Returns the decrypted data length.
 */
int
_gnutls_decrypt (gnutls_session_t session, opaque * ciphertext,
		 size_t ciphertext_size, uint8_t * data,
		 size_t max_data_size, content_type_t type)
{
  gnutls_datum_t gtxt;
  gnutls_datum_t gcipher;
  int ret;

  if (ciphertext_size == 0)
    return 0;

  gcipher.size = ciphertext_size;
  gcipher.data = ciphertext;

  ret =
    _gnutls_ciphertext2compressed (session, data, max_data_size,
				   gcipher, type);
  if (ret < 0)
    {
      return ret;
    }

  if (ret == 0 || is_read_comp_null (session) == 0)
    {
      /* ret == ret */

    }
  else
    {
      gnutls_datum_t gcomp;

      /* compression has this malloc overhead.
       */

      gcomp.data = data;
      gcomp.size = ret;
      ret = _gnutls_m_compressed2plaintext (session, &gtxt, gcomp);
      if (ret < 0)
	{
	  return ret;
	}

      if (gtxt.size > max_data_size)
	{
	  gnutls_assert ();
	  _gnutls_free_datum (&gtxt);
	  /* This shouldn't have happen and
	   * is a TLS fatal error.
	   */
	  return GNUTLS_E_INTERNAL_ERROR;
	}

      memcpy (data, gtxt.data, gtxt.size);
      ret = gtxt.size;

      _gnutls_free_datum (&gtxt);
    }

  return ret;
}

inline static mac_hd_t
mac_init (gnutls_mac_algorithm_t mac, opaque * secret, int secret_size,
	  int ver)
{
  mac_hd_t td;

  if (mac == GNUTLS_MAC_NULL)
    return GNUTLS_MAC_FAILED;

  if (ver == GNUTLS_SSL3)
    {				/* SSL 3.0 */
      td = _gnutls_mac_init_ssl3 (mac, secret, secret_size);
    }
  else
    {				/* TLS 1.x */
      td = _gnutls_hmac_init (mac, secret, secret_size);
    }

  return td;
}

inline static void
mac_deinit (mac_hd_t td, opaque * res, int ver)
{
  if (ver == GNUTLS_SSL3)
    {				/* SSL 3.0 */
      _gnutls_mac_deinit_ssl3 (td, res);
    }
  else
    {
      _gnutls_hmac_deinit (td, res);
    }
}

inline static int
calc_enc_length (gnutls_session_t session, int data_size,
		 int hash_size, uint8_t * pad, int random_pad,
		 cipher_type_t block_algo, uint16_t blocksize)
{
  uint8_t rnd;
  int length;

  *pad = 0;

  switch (block_algo)
    {
    case CIPHER_STREAM:
      length = data_size + hash_size;

      break;
    case CIPHER_BLOCK:
      if (gc_nonce (&rnd, 1) != GC_OK)
	{
	  gnutls_assert ();
	  return GNUTLS_E_RANDOM_FAILED;
	}

      /* make rnd a multiple of blocksize */
      if (session->security_parameters.version == GNUTLS_SSL3 ||
	  random_pad == 0)
	{
	  rnd = 0;
	}
      else
	{
	  rnd = (rnd / blocksize) * blocksize;
	  /* added to avoid the case of pad calculated 0
	   * seen below for pad calculation.
	   */
	  if (rnd > blocksize)
	    rnd -= blocksize;
	}

      length = data_size + hash_size;

      *pad = (uint8_t) (blocksize - (length % blocksize)) + rnd;

      length += *pad;
      if (session->security_parameters.version >= GNUTLS_TLS1_1)
	length += blocksize;	/* for the IV */

      break;
    default:
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
    }

  return length;
}

/* This is the actual encryption 
 * Encrypts the given compressed datum, and puts the result to cipher_data,
 * which has cipher_size size.
 * return the actual encrypted data length.
 */
int
_gnutls_compressed2ciphertext (gnutls_session_t session,
			       opaque * cipher_data, int cipher_size,
			       gnutls_datum_t compressed,
			       content_type_t _type, int random_pad)
{
  uint8_t MAC[MAX_HASH_SIZE];
  uint16_t c_length;
  uint8_t pad;
  int length, ret;
  mac_hd_t td;
  uint8_t type = _type;
  uint8_t major, minor;
  int hash_size =
    _gnutls_hash_get_algo_len (session->security_parameters.
			       write_mac_algorithm);
  gnutls_protocol_t ver;
  int blocksize =
    _gnutls_cipher_get_block_size (session->security_parameters.
				   write_bulk_cipher_algorithm);
  cipher_type_t block_algo =
    _gnutls_cipher_is_block (session->security_parameters.
			     write_bulk_cipher_algorithm);
  opaque *data_ptr;


  ver = gnutls_protocol_get_version (session);
  minor = _gnutls_version_get_minor (ver);
  major = _gnutls_version_get_major (ver);


  /* Initialize MAC */
  td = mac_init (session->security_parameters.write_mac_algorithm,
		 session->connection_state.write_mac_secret.data,
		 session->connection_state.write_mac_secret.size, ver);

  if (td == GNUTLS_MAC_FAILED
      && session->security_parameters.write_mac_algorithm != GNUTLS_MAC_NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
    }

  c_length = _gnutls_conv_uint16 (compressed.size);

  if (td != GNUTLS_MAC_FAILED)
    {				/* actually when the algorithm in not the NULL one */
      _gnutls_hmac (td,
		    UINT64DATA (session->connection_state.
				write_sequence_number), 8);

      _gnutls_hmac (td, &type, 1);
      if (ver >= GNUTLS_TLS1)
	{			/* TLS 1.0 or higher */
	  _gnutls_hmac (td, &major, 1);
	  _gnutls_hmac (td, &minor, 1);
	}
      _gnutls_hmac (td, &c_length, 2);
      _gnutls_hmac (td, compressed.data, compressed.size);
      mac_deinit (td, MAC, ver);
    }


  /* Calculate the encrypted length (padding etc.)
   */
  length =
    calc_enc_length (session, compressed.size, hash_size, &pad,
		     random_pad, block_algo, blocksize);
  if (length < 0)
    {
      gnutls_assert ();
      return length;
    }

  /* copy the encrypted data to cipher_data.
   */
  if (cipher_size < length)
    {
      gnutls_assert ();
      return GNUTLS_E_MEMORY_ERROR;
    }

  data_ptr = cipher_data;
  if (block_algo == CIPHER_BLOCK &&
      session->security_parameters.version >= GNUTLS_TLS1_1)
    {
      /* copy the random IV.
       */
      if (gc_nonce (data_ptr, blocksize) != GC_OK)
	{
	  gnutls_assert ();
	  return GNUTLS_E_RANDOM_FAILED;
	}
      data_ptr += blocksize;
    }

  memcpy (data_ptr, compressed.data, compressed.size);
  data_ptr += compressed.size;

  if (hash_size > 0)
    {
      memcpy (data_ptr, MAC, hash_size);
      data_ptr += hash_size;
    }
  if (block_algo == CIPHER_BLOCK && pad > 0)
    {
      memset (data_ptr, pad - 1, pad);
    }


  /* Actual encryption (inplace).
   */
  ret = _gnutls_cipher_encrypt (session->connection_state.
				write_cipher_state, cipher_data, length);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  return length;
}

/* Deciphers the ciphertext packet, and puts the result to compress_data, of compress_size.
 * Returns the actual compressed packet size.
 */
int
_gnutls_ciphertext2compressed (gnutls_session_t session,
			       opaque * compress_data,
			       int compress_size,
			       gnutls_datum_t ciphertext, uint8_t type)
{
  uint8_t MAC[MAX_HASH_SIZE];
  uint16_t c_length;
  uint8_t pad;
  int length;
  mac_hd_t td;
  uint16_t blocksize;
  int ret, i, pad_failed = 0;
  uint8_t major, minor;
  gnutls_protocol_t ver;
  int hash_size =
    _gnutls_hash_get_algo_len (session->security_parameters.
			       read_mac_algorithm);

  ver = gnutls_protocol_get_version (session);
  minor = _gnutls_version_get_minor (ver);
  major = _gnutls_version_get_major (ver);

  blocksize = _gnutls_cipher_get_block_size (session->security_parameters.
					     read_bulk_cipher_algorithm);

  /* initialize MAC 
   */
  td = mac_init (session->security_parameters.read_mac_algorithm,
		 session->connection_state.read_mac_secret.data,
		 session->connection_state.read_mac_secret.size, ver);

  if (td == GNUTLS_MAC_FAILED
      && session->security_parameters.read_mac_algorithm != GNUTLS_MAC_NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
    }


  /* actual decryption (inplace)
   */
  switch (_gnutls_cipher_is_block
	  (session->security_parameters.read_bulk_cipher_algorithm))
    {
    case CIPHER_STREAM:
      if ((ret = _gnutls_cipher_decrypt (session->connection_state.
					 read_cipher_state,
					 ciphertext.data,
					 ciphertext.size)) < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      length = ciphertext.size - hash_size;

      break;
    case CIPHER_BLOCK:
      if ((ciphertext.size < blocksize) || (ciphertext.size % blocksize != 0))
	{
	  gnutls_assert ();
	  return GNUTLS_E_DECRYPTION_FAILED;
	}

      if ((ret = _gnutls_cipher_decrypt (session->connection_state.
					 read_cipher_state,
					 ciphertext.data,
					 ciphertext.size)) < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      /* ignore the IV in TLS 1.1.
       */
      if (session->security_parameters.version >= GNUTLS_TLS1_1)
	{
	  ciphertext.size -= blocksize;
	  ciphertext.data += blocksize;

	  if (ciphertext.size == 0)
	    {
	      gnutls_assert ();
	      return GNUTLS_E_DECRYPTION_FAILED;
	    }
	}

      pad = ciphertext.data[ciphertext.size - 1] + 1;	/* pad */

      length = ciphertext.size - hash_size - pad;

      if (pad > ciphertext.size - hash_size)
	{
	  gnutls_assert ();
	  /* We do not fail here. We check below for the
	   * the pad_failed. If zero means success.
	   */
	  pad_failed = GNUTLS_E_DECRYPTION_FAILED;
	}

      /* Check the pading bytes (TLS 1.x)
       */
      if (ver >= GNUTLS_TLS1 && pad_failed == 0)
	for (i = 2; i < pad; i++)
	  {
	    if (ciphertext.data[ciphertext.size - i] !=
		ciphertext.data[ciphertext.size - 1])
	      pad_failed = GNUTLS_E_DECRYPTION_FAILED;
	  }
      break;
    default:
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
    }

  if (length < 0)
    length = 0;
  c_length = _gnutls_conv_uint16 ((uint16_t) length);

  /* Pass the type, version, length and compressed through
   * MAC.
   */
  if (td != GNUTLS_MAC_FAILED)
    {
      _gnutls_hmac (td,
		    UINT64DATA (session->connection_state.
				read_sequence_number), 8);

      _gnutls_hmac (td, &type, 1);
      if (ver >= GNUTLS_TLS1)
	{			/* TLS 1.x */
	  _gnutls_hmac (td, &major, 1);
	  _gnutls_hmac (td, &minor, 1);
	}
      _gnutls_hmac (td, &c_length, 2);

      if (length > 0)
	_gnutls_hmac (td, ciphertext.data, length);

      mac_deinit (td, MAC, ver);
    }

  /* This one was introduced to avoid a timing attack against the TLS
   * 1.0 protocol.
   */
  if (pad_failed != 0)
    return pad_failed;

  /* HMAC was not the same. 
   */
  if (memcmp (MAC, &ciphertext.data[length], hash_size) != 0)
    {
      gnutls_assert ();
      return GNUTLS_E_DECRYPTION_FAILED;
    }

  /* copy the decrypted stuff to compress_data.
   */
  if (compress_size < length)
    {
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
    }
  memcpy (compress_data, ciphertext.data, length);

  return length;
}
