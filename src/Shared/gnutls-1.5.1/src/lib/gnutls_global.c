/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Free Software Foundation
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
#include <libtasn1.h>
#include <gnutls_dh.h>

#ifdef HAVE_WINSOCK
# include <winsock2.h>
#endif

#include "gettext.h"

#define gnutls_log_func LOG_FUNC

/* created by asn1c */
extern const ASN1_ARRAY_TYPE gnutls_asn1_tab[];
extern const ASN1_ARRAY_TYPE pkix_asn1_tab[];

LOG_FUNC _gnutls_log_func;
int _gnutls_log_level = 0;	/* default log level */

ASN1_TYPE _gnutls_pkix1_asn;
ASN1_TYPE _gnutls_gnutls_asn;

/**
  * gnutls_global_set_log_function - This function sets the logging function
  * @log_func: it's a log function
  *
  * This is the function where you set the logging function gnutls
  * is going to use. This function only accepts a character array.
  * Normally you may not use this function since it is only used 
  * for debugging purposes.
  *
  * gnutls_log_func is of the form, 
  * void (*gnutls_log_func)( int level, const char*);
  **/
void
gnutls_global_set_log_function (gnutls_log_func log_func)
{
  _gnutls_log_func = log_func;
}

/**
  * gnutls_global_set_log_level - This function sets the logging level
  * @level: it's an integer from 0 to 9. 
  *
  * This is the function that allows you to set the log level.
  * The level is an integer between 0 and 9. Higher values mean
  * more verbosity. The default value is 0. Larger values should
  * only be used with care, since they may reveal sensitive information.
  *
  * Use a log level over 10 to enable all debugging options.
  *
  **/
void
gnutls_global_set_log_level (int level)
{
  _gnutls_log_level = level;
}


#ifdef DEBUG
/* default logging function */
static void
dlog (int level, const char *str)
{
  fputs (str, stderr);
}
#endif

extern gnutls_alloc_function gnutls_secure_malloc;
extern gnutls_alloc_function gnutls_malloc;
extern gnutls_free_function gnutls_free;
extern int (*_gnutls_is_secure_memory) (const void *);
extern gnutls_realloc_function gnutls_realloc;
extern char *(*gnutls_strdup) (const char *);
extern void *(*gnutls_calloc) (size_t, size_t);

int _gnutls_is_secure_mem_null (const void *);

/**
  * gnutls_global_set_mem_functions - This function sets the memory allocation functions
  * @alloc_func: it's the default memory allocation function. Like malloc().
  * @secure_alloc_func: This is the memory allocation function that will be used for sensitive data.
  * @is_secure_func: a function that returns 0 if the memory given is not secure. May be NULL.
  * @realloc_func: A realloc function
  * @free_func: The function that frees allocated data. Must accept a NULL pointer.
  *
  * This is the function were you set the memory allocation functions gnutls
  * is going to use. By default the libc's allocation functions (malloc(), free()),
  * are used by gnutls, to allocate both sensitive and not sensitive data.
  * This function is provided to set the memory allocation functions to
  * something other than the defaults (ie the gcrypt allocation functions). 
  *
  * This function must be called before gnutls_global_init() is called.
  *
  **/
void
gnutls_global_set_mem_functions (gnutls_alloc_function alloc_func,
				 gnutls_alloc_function
				 secure_alloc_func,
				 gnutls_is_secure_function
				 is_secure_func,
				 gnutls_realloc_function realloc_func,
				 gnutls_free_function free_func)
{
  gnutls_secure_malloc = secure_alloc_func;
  gnutls_malloc = alloc_func;
  gnutls_realloc = realloc_func;
  gnutls_free = free_func;

  if (is_secure_func != NULL)
    _gnutls_is_secure_memory = is_secure_func;
  else
    _gnutls_is_secure_memory = _gnutls_is_secure_mem_null;

  /* if using the libc's default malloc
   * use libc's calloc as well.
   */
  if (gnutls_malloc == malloc)
    {
      gnutls_calloc = calloc;
    }
  else
    {				/* use the included ones */
      gnutls_calloc = _gnutls_calloc;
    }
  gnutls_strdup = _gnutls_strdup;

}

#ifdef DEBUG
static void
_gnutls_gcry_log_handler (void *dummy, int level,
			  const char *fmt, va_list list)
{
  _gnutls_log (fmt, list);
}
#endif

static int _gnutls_init = 0;

/**
  * gnutls_global_init - This function initializes the global data to defaults.
  *
  * This function initializes the global data to defaults.
  * Every gnutls application has a global data which holds common parameters
  * shared by gnutls session structures.
  * You must call gnutls_global_deinit() when gnutls usage is no longer needed
  * Returns zero on success.
  *
  * Note that this function will also initialize libgcrypt, if it has not
  * been initialized before. Thus if you want to manually initialize libgcrypt
  * you must do it before calling this function. This is useful in cases you 
  * want to disable libgcrypt's internal lockings etc.
  *
  * This function increment a global counter, so that
  * gnutls_global_deinit() only releases resources when it has been
  * called as many times as gnutls_global_init().  This is useful when
  * GnuTLS is used by more than one library in an application.  This
  * function can be called many times, but will only do something the
  * first time.
  *
  * Note!  This function is not thread safe.  If two threads call this
  * function simultaneously, they can cause a race between checking
  * the global counter and incrementing it, causing both threads to
  * execute the library initialization code.  That would lead to a
  * memory leak.  To handle this, your application could invoke this
  * function after aquiring a thread mutex.  To ignore the potential
  * memory leak is also an option.
  *
  **/
int
gnutls_global_init (void)
{

  int result = 0;
  int res;
  char c;

  if (_gnutls_init++)
    goto out;

#if HAVE_WINSOCK
  {
    WORD requested;
    WSADATA data;
    int err;

    requested = MAKEWORD (1, 1);
    err = WSAStartup (requested, &data);
    if (err != 0)
      {
	_gnutls_debug_log ("WSAStartup failed: %d.\n", err);
	return GNUTLS_E_LIBRARY_VERSION_MISMATCH;
      }

    if (data.wVersion < requested)
      {
	_gnutls_debug_log ("WSAStartup version check failed (%d < %d).\n",
			   data.wVersion, requested);
	WSACleanup ();
	return GNUTLS_E_LIBRARY_VERSION_MISMATCH;
      }
  }
#endif

  bindtextdomain (PACKAGE, LOCALEDIR);

  if (gcry_control (GCRYCTL_ANY_INITIALIZATION_P) == 0)
    {
      const char *p;
      p = strchr (GNUTLS_GCRYPT_VERSION, ':');
      if (p == NULL)
	p = GNUTLS_GCRYPT_VERSION;
      else
	p++;

      if (gcry_check_version (p) == NULL)
	{
	  gnutls_assert ();
	  _gnutls_debug_log ("Checking for libgcrypt failed '%s'\n", p);
	  return GNUTLS_E_INCOMPATIBLE_GCRYPT_LIBRARY;
	}

      /* for gcrypt in order to be able to allocate memory */
      gcry_set_allocation_handler (gnutls_malloc, gnutls_secure_malloc,
				   _gnutls_is_secure_memory, gnutls_realloc,
				   gnutls_free);

      /* gcry_control (GCRYCTL_DISABLE_INTERNAL_LOCKING, NULL, 0); */

      gcry_control (GCRYCTL_INITIALIZATION_FINISHED, NULL, 0);

#ifdef DEBUG
      /* applications may want to override that, so we only use
       * it in debugging mode.
       */
      gcry_set_log_handler (_gnutls_gcry_log_handler, NULL);
#endif
    }

  if (gc_init () != GC_OK)
    {
      gnutls_assert ();
      _gnutls_debug_log ("Initializing crypto backend failed\n");
      return GNUTLS_E_INCOMPATIBLE_CRYPTO_LIBRARY;
    }

  /* for gcrypt in order to be able to allocate memory */
  gc_set_allocators (gnutls_malloc, gnutls_secure_malloc,
		     _gnutls_is_secure_memory, gnutls_realloc, gnutls_free);

#ifdef DEBUG
  gnutls_global_set_log_function (dlog);
#endif

  /* initialize parser 
   * This should not deal with files in the final
   * version.
   */

  if (asn1_check_version (GNUTLS_LIBTASN1_VERSION) == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INCOMPATIBLE_LIBTASN1_LIBRARY;
    }

  res = asn1_array2tree (pkix_asn1_tab, &_gnutls_pkix1_asn, NULL);
  if (res != ASN1_SUCCESS)
    {
      result = _gnutls_asn2err (res);
      goto out;
    }

  res = asn1_array2tree (gnutls_asn1_tab, &_gnutls_gnutls_asn, NULL);
  if (res != ASN1_SUCCESS)
    {
      asn1_delete_structure (&_gnutls_pkix1_asn);
      result = _gnutls_asn2err (res);
      goto out;
    }
    
  /* Initialize the gcrypt (if used random generator) */
  gc_pseudo_random (&c, 1);

out:
  return result;
}

/**
  * gnutls_global_deinit - This function deinitializes the global data 
  *
  * This function deinitializes the global data, that were initialized
  * using gnutls_global_init().
  *
  * Note!  This function is not thread safe.  See the discussion for
  * gnutls_global_init() for more information.
  *
  **/
void
gnutls_global_deinit (void)
{
  if (_gnutls_init == 1)
    {
#if HAVE_WINSOCK
      WSACleanup ();
#endif
      asn1_delete_structure (&_gnutls_gnutls_asn);
      asn1_delete_structure (&_gnutls_pkix1_asn);
      gc_done ();
    }
  _gnutls_init--;
}


/* These functions should be elsewere. Kept here for 
 * historical reasons.
 */

/**
  * gnutls_transport_set_pull_function - This function sets a read like function
  * @pull_func: a callback function similar to read()
  * @session: gnutls session
  *
  * This is the function where you set a function for gnutls 
  * to receive data. Normally, if you use berkeley style sockets,
  * do not need to use this function since the default (recv(2)) will 
  * probably be ok.
  *
  * PULL_FUNC is of the form, 
  * ssize_t (*gnutls_pull_func)(gnutls_transport_ptr_t, void*, size_t);
  **/
void
gnutls_transport_set_pull_function (gnutls_session_t session,
				    gnutls_pull_func pull_func)
{
  session->internals._gnutls_pull_func = pull_func;
}

/**
  * gnutls_transport_set_push_function - This function sets the function to send data
  * @push_func: a callback function similar to write()
  * @session: gnutls session
  *
  * This is the function where you set a push function for gnutls
  * to use in order to send data. If you are going to use berkeley style
  * sockets, you do not need to use this function since
  * the default (send(2)) will probably be ok. Otherwise you should
  * specify this function for gnutls to be able to send data.
  *  
  * PUSH_FUNC is of the form, 
  * ssize_t (*gnutls_push_func)(gnutls_transport_ptr_t, const void*, size_t);
  **/
void
gnutls_transport_set_push_function (gnutls_session_t session,
				    gnutls_push_func push_func)
{
  session->internals._gnutls_push_func = push_func;
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
  * gnutls_check_version - This function checks the library's version
  * @req_version: the version to check
  *
  * Check that the version of the library is at minimum the requested one
  * and return the version string; return NULL if the condition is not
  * satisfied.  If a NULL is passed to this function, no check is done,
  * but the version string is simply returned.
  *
  **/
const char *
gnutls_check_version (const char *req_version)
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
