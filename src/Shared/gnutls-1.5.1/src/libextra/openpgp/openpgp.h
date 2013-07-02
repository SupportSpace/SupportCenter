#ifndef OPENPGP_H
# define OPENPGP_H

#if HAVE_CONFIG_H
# include <config.h>
#endif

typedef enum gnutls_openpgp_key_fmt_t
{
  GNUTLS_OPENPGP_FMT_RAW, GNUTLS_OPENPGP_FMT_BASE64
} gnutls_openpgp_key_fmt_t;

#ifdef ENABLE_OPENPGP

# include <opencdk.h>

typedef struct gnutls_openpgp_key_int
{
  cdk_kbnode_t knode;
  cdk_stream_t inp;
} gnutls_openpgp_key_int;

typedef struct gnutls_openpgp_privkey_int
{
  gnutls_privkey pkey;
} gnutls_openpgp_privkey_int;

typedef struct gnutls_openpgp_keyring_int
{
  cdk_keydb_hd_t hd;
} gnutls_openpgp_keyring_int;

typedef struct gnutls_openpgp_trustdb_int
{
  cdk_stream_t st;
} gnutls_openpgp_trustdb_int;

typedef struct gnutls_openpgp_keyring_int *gnutls_openpgp_keyring_t;
typedef struct gnutls_openpgp_trustdb_int *gnutls_openpgp_trustdb_t;

int _gnutls_map_cdk_rc (int rc);
int
gnutls_openpgp_key_get_name (gnutls_openpgp_key_t key,
			     int idx, char *buf, size_t * sizeof_buf);
int gnutls_openpgp_key_get_fingerprint (gnutls_openpgp_key_t key,
					void *fpr, size_t * fprlen);
int gnutls_openpgp_key_get_pk_algorithm (gnutls_openpgp_key_t key,
					 unsigned int *bits);
int gnutls_openpgp_key_get_version (gnutls_openpgp_key_t key);
time_t gnutls_openpgp_key_get_creation_time (gnutls_openpgp_key_t key);
time_t gnutls_openpgp_key_get_expiration_time (gnutls_openpgp_key_t key);
int gnutls_openpgp_key_get_id (gnutls_openpgp_key_t key,
			       unsigned char keyid[8]);

int gnutls_openpgp_key_init (gnutls_openpgp_key_t * key);
void gnutls_openpgp_key_deinit (gnutls_openpgp_key_t key);
int gnutls_openpgp_key_import (gnutls_openpgp_key_t key,
			       const gnutls_datum_t * data,
			       gnutls_openpgp_key_fmt_t format);
int gnutls_openpgp_key_export (gnutls_openpgp_key_t key,
			       gnutls_openpgp_key_fmt_t format,
			       void *output_data, size_t * output_data_size);

void gnutls_openpgp_keyring_deinit (gnutls_openpgp_keyring_t keyring);
int gnutls_openpgp_keyring_init (gnutls_openpgp_keyring_t * keyring);
int gnutls_openpgp_keyring_import (gnutls_openpgp_keyring_t keyring,
				   const gnutls_datum_t * data,
				   gnutls_openpgp_key_fmt_t format);
int gnutls_openpgp_keyring_check_id (gnutls_openpgp_keyring_t ring,
				     const unsigned char keyid[8],
				     unsigned int flags);

void gnutls_openpgp_trustdb_deinit (gnutls_openpgp_trustdb_t trustdb);
int gnutls_openpgp_trustdb_init (gnutls_openpgp_trustdb_t * trustdb);
int gnutls_openpgp_trustdb_import_file (gnutls_openpgp_trustdb_t trustdb,
					const char *file);

int gnutls_openpgp_key_verify_ring (gnutls_openpgp_key_t key,
				    gnutls_openpgp_keyring_t keyring,
				    unsigned int flags, unsigned int *verify);

int gnutls_openpgp_key_verify_trustdb (gnutls_openpgp_key_t key,
				       gnutls_openpgp_trustdb_t trustdb,
				       unsigned int flags,
				       unsigned int *verify);

int gnutls_openpgp_key_verify_self (gnutls_openpgp_key_t key,
				    unsigned int flags, unsigned int *verify);

int _gnutls_openpgp_key_to_gcert (gnutls_cert * gcert,
				  gnutls_openpgp_key_t cert);
int _gnutls_openpgp_privkey_to_gkey (gnutls_privkey * dest,
				     gnutls_openpgp_privkey_t src);

void gnutls_openpgp_privkey_deinit (gnutls_openpgp_privkey_t key);

#else /* no opencdk */

typedef void *gnutls_openpgp_keyring_t;
typedef void *gnutls_openpgp_trustdb_t;

#endif /* ENABLE_OPENPGP */

#endif /* OPENPGP_H */
