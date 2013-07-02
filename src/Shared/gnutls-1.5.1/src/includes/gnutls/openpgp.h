/*
 * Copyright (C) 2003, 2004, 2005 Free Software Foundation
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA
 *
 */

/* This file contains the types and prototypes for the OpenPGP
 * key and private key parsing functions.
 */

#ifndef GNUTLS_OPENPGP_H
# define GNUTLS_OPENPGP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <gnutls/gnutls.h>
#include <gnutls/extra.h>

/* gnutls_openpgp_key_t should be defined in gnutls.h
 */

  typedef enum gnutls_openpgp_key_fmt
  { GNUTLS_OPENPGP_FMT_RAW,
    GNUTLS_OPENPGP_FMT_BASE64
  } gnutls_openpgp_key_fmt;

  int gnutls_openpgp_key_init (gnutls_openpgp_key * key);	/* initializes the memory for gnutls_openpgp_key_t struct */
  void gnutls_openpgp_key_deinit (gnutls_openpgp_key_t key);	/* frees all memory */

  int gnutls_openpgp_key_import (gnutls_openpgp_key_t key,
				 const gnutls_datum_t * data,
				 gnutls_openpgp_key_fmt format);
  int gnutls_openpgp_key_export (gnutls_openpgp_key_t key,
				 gnutls_openpgp_key_fmt_t format,
				 void *output_data,
				 size_t * output_data_size);

/* The key_usage flags are defined in gnutls.h. They are
 * the GNUTLS_KEY_* definitions.
 */
  int gnutls_openpgp_key_get_key_usage (gnutls_openpgp_key_t cert,
					unsigned int *key_usage);
  int gnutls_openpgp_key_get_fingerprint (gnutls_openpgp_key_t key, void *fpr,
					  size_t * fprlen);

  int gnutls_openpgp_key_get_name (gnutls_openpgp_key_t key,
				   int idx, char *buf, size_t * sizeof_buf);

  int gnutls_openpgp_key_get_pk_algorithm (gnutls_openpgp_key_t key,
					   unsigned int *bits);

  int gnutls_openpgp_key_get_version (gnutls_openpgp_key_t key);

  time_t gnutls_openpgp_key_get_creation_time (gnutls_openpgp_key_t key);
  time_t gnutls_openpgp_key_get_expiration_time (gnutls_openpgp_key_t key);

  int gnutls_openpgp_key_get_id (gnutls_openpgp_key_t key,
				 unsigned char keyid[8]);

  int gnutls_openpgp_key_check_hostname (gnutls_openpgp_key_t key,
					 const char *hostname);

  int gnutls_openpgp_key_to_xml (gnutls_openpgp_key_t key,
				 gnutls_datum_t * xmlkey, int ext);

/* privkey stuff.
 */
  int gnutls_openpgp_privkey_init (gnutls_openpgp_privkey_t * key);
  void gnutls_openpgp_privkey_deinit (gnutls_openpgp_privkey_t key);
  int gnutls_openpgp_privkey_get_pk_algorithm (gnutls_openpgp_privkey_t key,
					       unsigned int *bits);
  int gnutls_openpgp_privkey_import (gnutls_openpgp_privkey_t key,
				     const gnutls_datum_t * data,
				     gnutls_openpgp_key_fmt format,
				     const char *pass, unsigned int flags);

/* Keyring stuff.
 */
  struct gnutls_openpgp_keyring_int;	/* object to hold (parsed) openpgp keyrings */
  typedef struct gnutls_openpgp_keyring_int *gnutls_openpgp_keyring;

  int gnutls_openpgp_keyring_init (gnutls_openpgp_keyring * keyring);
  void gnutls_openpgp_keyring_deinit (gnutls_openpgp_keyring_t keyring);

  int gnutls_openpgp_keyring_import (gnutls_openpgp_keyring_t keyring,
				     const gnutls_datum_t * data,
				     gnutls_openpgp_key_fmt format);

  int gnutls_openpgp_keyring_check_id (gnutls_openpgp_keyring_t ring,
				       const unsigned char keyid[8],
				       unsigned int flags);

/* Trustdb functions.
 */
  struct gnutls_openpgp_trustdb_int;	/* object to hold (parsed) openpgp trustdbs */
  typedef struct gnutls_openpgp_trustdb_int *gnutls_openpgp_trustdb;

  int gnutls_openpgp_trustdb_init (gnutls_openpgp_trustdb * trustdb);
  void gnutls_openpgp_trustdb_deinit (gnutls_openpgp_trustdb_t trustdb);

  int gnutls_openpgp_trustdb_import (gnutls_openpgp_trustdb_t trustdb,
				     const char *file);


  int gnutls_openpgp_key_verify_ring (gnutls_openpgp_key_t key,
				      gnutls_openpgp_keyring_t keyring,
				      unsigned int flags, unsigned int *verify
				      /* the output of the verification */ );

  int gnutls_openpgp_key_verify_self (gnutls_openpgp_key_t key,
				      unsigned int flags,
				      unsigned int *verify);

  int gnutls_openpgp_key_verify_trustdb (gnutls_openpgp_key_t key,
					 gnutls_openpgp_trustdb_t trustdb,
					 unsigned int flags,
					 unsigned int *verify
					 /* the output of the verification */
    );


/* certificate authentication stuff.
 */
  int gnutls_certificate_set_openpgp_key (gnutls_certificate_credentials_t
					  res, gnutls_openpgp_key_t key,
					  gnutls_openpgp_privkey_t pkey);

#ifdef __cplusplus
}
#endif
#endif				/* GNUTLS_OPENPGP_H */
