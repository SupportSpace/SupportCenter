/*
 * Copyright (C) 2001, 2004, 2005 Free Software Foundation
 *
 * Author: Nikos Mavroyanopoulos
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

#include <gnutls_int.h>
#include <gnutls_errors.h>
#include <gnutls_extensions.h>
#include <gnutls_openpgp.h>
#include <gnutls_extra.h>
#include <gnutls_algorithms.h>
#ifdef USE_LZO
# ifdef USE_MINILZO
#  include "minilzo/minilzo.h"
# elif HAVE_LZO_LZO1X_H
#  include <lzo/lzo1x.h>
# elif HAVE_LZO1X_H
#  include <lzo1x.h>
# endif
#endif


/* the number of the compression algorithms available in the compression
 * structure.
 */
extern int _gnutls_comp_algorithms_size;

/* Functions in gnutls that have not been initialized.
 */
#ifdef USE_LZO
typedef int (*LZO_FUNC) ();
extern LZO_FUNC _gnutls_lzo1x_decompress_safe;
extern LZO_FUNC _gnutls_lzo1x_1_compress;

extern gnutls_compression_entry _gnutls_compression_algorithms[];

static int
_gnutls_add_lzo_comp (void)
{
  int i;

  /* find the last element */
  for (i = 0; i < _gnutls_comp_algorithms_size; i++)
    {
      if (_gnutls_compression_algorithms[i].name == NULL)
	break;
    }

  if (_gnutls_compression_algorithms[i].name == NULL
      && (i < _gnutls_comp_algorithms_size - 1))
    {
      _gnutls_compression_algorithms[i].name = "GNUTLS_COMP_LZO";
      _gnutls_compression_algorithms[i].id = GNUTLS_COMP_LZO;
      _gnutls_compression_algorithms[i].num = 0xf2;

      _gnutls_compression_algorithms[i + 1].name = 0;

      /* Now enable the lzo functions: */
      _gnutls_lzo1x_decompress_safe = lzo1x_decompress_safe;
      _gnutls_lzo1x_1_compress = lzo1x_1_compress;

      return 0;			/* ok */
    }


  return GNUTLS_E_MEMORY_ERROR;
}
#endif

extern OPENPGP_KEY_CREATION_TIME_FUNC
  _E_gnutls_openpgp_get_raw_key_creation_time;
extern OPENPGP_KEY_EXPIRATION_TIME_FUNC
  _E_gnutls_openpgp_get_raw_key_expiration_time;
extern OPENPGP_VERIFY_KEY_FUNC _E_gnutls_openpgp_verify_key;
extern OPENPGP_FINGERPRINT _E_gnutls_openpgp_fingerprint;
extern OPENPGP_KEY_REQUEST _E_gnutls_openpgp_request_key;

extern OPENPGP_RAW_KEY_TO_GCERT _E_gnutls_openpgp_raw_key_to_gcert;
extern OPENPGP_RAW_PRIVKEY_TO_GKEY _E_gnutls_openpgp_raw_privkey_to_gkey;

extern OPENPGP_KEY_TO_GCERT _E_gnutls_openpgp_key_to_gcert;
extern OPENPGP_PRIVKEY_TO_GKEY _E_gnutls_openpgp_privkey_to_gkey;
extern OPENPGP_KEY_DEINIT _E_gnutls_openpgp_key_deinit;
extern OPENPGP_PRIVKEY_DEINIT _E_gnutls_openpgp_privkey_deinit;

static void
_gnutls_add_openpgp_functions (void)
{
#ifdef ENABLE_OPENPGP
  _E_gnutls_openpgp_verify_key = _gnutls_openpgp_verify_key;
  _E_gnutls_openpgp_get_raw_key_expiration_time =
    _gnutls_openpgp_get_raw_key_expiration_time;
  _E_gnutls_openpgp_get_raw_key_creation_time =
    _gnutls_openpgp_get_raw_key_creation_time;
  _E_gnutls_openpgp_fingerprint = _gnutls_openpgp_fingerprint;
  _E_gnutls_openpgp_request_key = _gnutls_openpgp_request_key;

  _E_gnutls_openpgp_raw_key_to_gcert = _gnutls_openpgp_raw_key_to_gcert;
  _E_gnutls_openpgp_raw_privkey_to_gkey = _gnutls_openpgp_raw_privkey_to_gkey;

  _E_gnutls_openpgp_key_to_gcert = _gnutls_openpgp_key_to_gcert;
  _E_gnutls_openpgp_privkey_to_gkey = _gnutls_openpgp_privkey_to_gkey;
  _E_gnutls_openpgp_key_deinit = gnutls_openpgp_key_deinit;
  _E_gnutls_openpgp_privkey_deinit = gnutls_openpgp_privkey_deinit;
#endif
}

extern const char *gnutls_check_version (const char *);
static int _gnutls_init_extra = 0;

/**
  * gnutls_global_init_extra - This function initializes the global state of gnutls-extra 
  *
  * This function initializes the global state of gnutls-extra library to defaults.
  * Returns zero on success.
  *
  * Note that gnutls_global_init() has to be called before this function.
  * If this function is not called then the gnutls-extra library will not
  * be usable.
  *
  **/
int
gnutls_global_init_extra (void)
{
  int ret;

  /* If the version of libgnutls != version of
   * libextra, then do not initialize the library.
   * This is because it may break things.
   */
  if (strcmp (gnutls_check_version (NULL), VERSION) != 0)
    {
      return GNUTLS_E_LIBRARY_VERSION_MISMATCH;
    }

  _gnutls_init_extra++;

  if (_gnutls_init_extra != 1)
    {
      return 0;
    }

  /* Initialize the LZO library
   */
#ifdef USE_LZO
  if (lzo_init () != LZO_E_OK)
    {
      return GNUTLS_E_LZO_INIT_FAILED;
    }

  /* Add the LZO compression method in the list of compression
   * methods.
   */
  ret = _gnutls_add_lzo_comp ();
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }
#endif

  /* Register the openpgp functions. This is because some
   * of them are defined to be NULL in the main library.
   */
  _gnutls_add_openpgp_functions ();

  return 0;
}

/* Taken from libgcrypt. Needed to configure scripts.
 */

static const char *
parse_version_number (const char *s, int *number)
{
  int val = 0;

  if (*s == '0' && isdigit (s[1]))
    return NULL;		/* leading zeros are not allowed */
  for (; isdigit (*s); s++)
    {
      val *= 10;
      val += *s - '0';
    }
  *number = val;
  return val < 0 ? NULL : s;
}

/* The parse version functions were copied from libgcrypt.
 */
static const char *
parse_version_string (const char *s, int *major, int *minor, int *micro)
{
  s = parse_version_number (s, major);
  if (!s || *s != '.')
    return NULL;
  s++;
  s = parse_version_number (s, minor);
  if (!s || *s != '.')
    return NULL;
  s++;
  s = parse_version_number (s, micro);
  if (!s)
    return NULL;
  return s;			/* patchlevel */
}

/**
 * gnutls_extra_check_version - This function checks the library's version
 * @req_version: the version to check
 *
 * Check that the version of the gnutls-extra library is at minimum
 * the requested one and return the version string; return NULL if the
 * condition is not satisfied.  If a NULL is passed to this function,
 * no check is done, but the version string is simply returned.
 *
 **/
const char *
gnutls_extra_check_version (const char *req_version)
{
  const char *ver = VERSION;
  int my_major, my_minor, my_micro;
  int rq_major, rq_minor, rq_micro;
  const char *my_plvl, *rq_plvl;

  if (!req_version)
    return ver;

  my_plvl = parse_version_string (ver, &my_major, &my_minor, &my_micro);
  if (!my_plvl)
    return NULL;		/* very strange our own version is bogus */
  rq_plvl = parse_version_string (req_version, &rq_major, &rq_minor,
				  &rq_micro);
  if (!rq_plvl)
    return NULL;		/* req version string is invalid */

  if (my_major > rq_major
      || (my_major == rq_major && my_minor > rq_minor)
      || (my_major == rq_major && my_minor == rq_minor
	  && my_micro > rq_micro)
      || (my_major == rq_major && my_minor == rq_minor
	  && my_micro == rq_micro && strcmp (my_plvl, rq_plvl) >= 0))
    {
      return ver;
    }
  return NULL;
}
