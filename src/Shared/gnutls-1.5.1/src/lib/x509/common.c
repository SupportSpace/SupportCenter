/*
 * Copyright (C) 2003, 2004, 2005, 2006 Free Software Foundation
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
#include <libtasn1.h>
#include <gnutls_datum.h>
#include <gnutls_global.h>
#include <gnutls_errors.h>
#include <gnutls_str.h>
#include <gnutls_x509.h>
#include <gnutls_num.h>
#include <x509_b64.h>
#include <common.h>
#include <mpi.h>
#include <time.h>

typedef struct _oid2string
{
  const char *oid;
  const char *ldap_desc;
  int choice;			/* of type DirectoryString */
  int printable;
} oid2string;

/* This list contains all the OIDs that may be
 * contained in a rdnSequence and are printable.
 */
static const oid2string _oid2str[] = {
  /* PKIX
   */
  {"1.3.6.1.5.5.7.9.1", "dateOfBirth", 0, 1},
  {"1.3.6.1.5.5.7.9.2", "placeOfBirth", 0, 1},
  {"1.3.6.1.5.5.7.9.3", "gender", 0, 1},
  {"1.3.6.1.5.5.7.9.4", "countryOfCitizenship", 0, 1},
  {"1.3.6.1.5.5.7.9.5", "countryOfResidence", 0, 1},

  {"2.5.4.6", "C", 0, 1},
  {"2.5.4.9", "STREET", 1, 1},
  {"2.5.4.12", "T", 1, 1},
  {"2.5.4.10", "O", 1, 1},
  {"2.5.4.11", "OU", 1, 1},
  {"2.5.4.3", "CN", 1, 1},
  {"2.5.4.7", "L", 1, 1},
  {"2.5.4.8", "ST", 1, 1},

  {"2.5.4.5", "serialNumber", 0, 1},
  {"2.5.4.20", "telephoneNumber", 0, 1},
  {"2.5.4.4", "surName", 1, 1},
  {"2.5.4.43", "initials", 1, 1},
  {"2.5.4.44", "generationQualifier", 1, 1},
  {"2.5.4.42", "givenName", 1, 1},
  {"2.5.4.65", "pseudonym", 1, 1},
  {"2.5.4.46", "dnQualifier", 0, 1},

  {"0.9.2342.19200300.100.1.25", "DC", 0, 1},
  {"0.9.2342.19200300.100.1.1", "UID", 0, 1},

  /* PKCS #9
   */
  {"1.2.840.113549.1.9.1", "EMAIL", 0, 1},
  {"1.2.840.113549.1.9.7", NULL, 1, 1},

  /* friendly name */
  {"1.2.840.113549.1.9.20", NULL, 0, 1},
  {NULL, NULL, 0, 0}
};

/* Returns 1 if the data defined by the OID are printable.
 */
int
_gnutls_x509_oid_data_printable (const char *oid)
{
  int i = 0;

  do
    {
      if (strcmp (_oid2str[i].oid, oid) == 0)
	return _oid2str[i].printable;
      i++;
    }
  while (_oid2str[i].oid != NULL);

  return 0;
}

/**
  * gnutls_x509_dn_oid_known - This function will return true if the given OID is known
  * @oid: holds an Object Identifier in a null terminated string
  *
  * This function will inform about known DN OIDs. This is useful since functions
  * like gnutls_x509_crt_set_dn_by_oid() use the information on known
  * OIDs to properly encode their input. Object Identifiers that are not
  * known are not encoded by these functions, and their input is stored directly
  * into the ASN.1 structure. In that case of unknown OIDs, you have
  * the responsibility of DER encoding your data.
  *
  * Returns 1 on known OIDs and 0 otherwise.
  *
  **/
int
gnutls_x509_dn_oid_known (const char *oid)
{
  int i = 0;

  do
    {
      if (strcmp (_oid2str[i].oid, oid) == 0)
	return 1;
      i++;
    }
  while (_oid2str[i].oid != NULL);

  return 0;
}

/* Returns 1 if the data defined by the OID are of a choice
 * type.
 */
int
_gnutls_x509_oid_data_choice (const char *oid)
{
  int i = 0;

  do
    {
      if (strcmp (_oid2str[i].oid, oid) == 0)
	return _oid2str[i].choice;
      i++;
    }
  while (_oid2str[i].oid != NULL);

  return 0;
}

const char *
_gnutls_x509_oid2ldap_string (const char *oid)
{
  int i = 0;

  do
    {
      if (strcmp (_oid2str[i].oid, oid) == 0)
	return _oid2str[i].ldap_desc;
      i++;
    }
  while (_oid2str[i].oid != NULL);

  return NULL;
}

/* This function will convert an attribute value, specified by the OID,
 * to a string. The result will be a null terminated string.
 *
 * res may be null. This will just return the res_size, needed to
 * hold the string.
 */
int
_gnutls_x509_oid_data2string (const char *oid, void *value,
			      int value_size, char *res, size_t * res_size)
{
  char str[MAX_STRING_LEN], tmpname[128];
  const char *ANAME = NULL;
  int CHOICE = -1, len = -1, result;
  ASN1_TYPE tmpasn = ASN1_TYPE_EMPTY;

  if (value == NULL || value_size <= 0 || res_size == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  if (_gnutls_x509_oid_data_printable (oid) == 0)
    {
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
    }

  ANAME = asn1_find_structure_from_oid (_gnutls_get_pkix (), oid);
  CHOICE = _gnutls_x509_oid_data_choice (oid);

  if (ANAME == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
    }

  _gnutls_str_cpy (str, sizeof (str), "PKIX1.");
  _gnutls_str_cat (str, sizeof (str), ANAME);

  if ((result =
       asn1_create_element (_gnutls_get_pkix (), str,
			    &tmpasn)) != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  if ((result =
       asn1_der_decoding (&tmpasn, value, value_size, NULL)) != ASN1_SUCCESS)
    {
      gnutls_assert ();
      asn1_delete_structure (&tmpasn);
      return _gnutls_asn2err (result);
    }

  /* If this is a choice then we read the choice. Otherwise it
   * is the value;
   */
  len = sizeof (str) - 1;
  if ((result = asn1_read_value (tmpasn, "", str, &len)) != ASN1_SUCCESS)
    {				/* CHOICE */
      gnutls_assert ();
      asn1_delete_structure (&tmpasn);
      return _gnutls_asn2err (result);
    }

  if (CHOICE == 0)
    {
      str[len] = 0;

      if (res)
	_gnutls_str_cpy (res, *res_size, str);
      *res_size = len;

      asn1_delete_structure (&tmpasn);
    }
  else
    {				/* CHOICE */
      int non_printable = 0, teletex = 0;
      str[len] = 0;

      /* Note that we do not support strings other than
       * UTF-8 (thus ASCII as well).
       */
      if (strcmp (str, "printableString") != 0 &&
	  strcmp (str, "utf8String") != 0)
	{
	  non_printable = 1;
	}
      if (strcmp (str, "teletexString") == 0)
	teletex = 1;


      _gnutls_str_cpy (tmpname, sizeof (tmpname), str);

      len = sizeof (str) - 1;
      if ((result =
	   asn1_read_value (tmpasn, tmpname, str, &len)) != ASN1_SUCCESS)
	{
	  asn1_delete_structure (&tmpasn);
	  return _gnutls_asn2err (result);
	}

      asn1_delete_structure (&tmpasn);

      if (teletex != 0)
	{
	  int ascii = 0, i;
	  /* HACK: if the teletex string contains only ascii
	   * characters then treat it as printable.
	   */
	  for (i = 0; i < len; i++)
	    if (!isascii (str[i]))
	      ascii = 1;

	  if (ascii == 0)
	    non_printable = 0;
	}

      if (res)
	{
	  if (non_printable == 0)
	    {
	      str[len] = 0;
	      _gnutls_str_cpy (res, *res_size, str);
	      *res_size = len;
	    }
	  else
	    {
	      result = _gnutls_x509_data2hex (str, len, res, res_size);
	      if (result < 0)
		{
		  gnutls_assert ();
		  return result;
		}
	    }
	}

    }

  return 0;
}


/* Converts a data string to an LDAP rfc2253 hex string
 * something like '#01020304'
 */
int
_gnutls_x509_data2hex (const opaque * data, size_t data_size,
		       opaque * out, size_t * sizeof_out)
{
  char *res;
  char escaped[MAX_STRING_LEN];

  if (2 * data_size + 1 > MAX_STRING_LEN)
    {
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
    }

  res = _gnutls_bin2hex (data, data_size, escaped, sizeof (escaped));

  if (res)
    {
      unsigned int size = strlen (res) + 1;
      if (size + 1 > *sizeof_out)
	{
	  *sizeof_out = size;
	  return GNUTLS_E_SHORT_MEMORY_BUFFER;
	}
      *sizeof_out = size;	/* -1 for the null +1 for the '#' */

      if (out)
	{
	  strcpy (out, "#");
	  strcat (out, res);
	}

      return 0;
    }
  else
    {
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
    }

  return 0;
}


/* TIME functions 
 * Convertions between generalized or UTC time to time_t
 *
 */

/* This is an emulations of the struct tm.
 * Since we do not use libc's functions, we don't need to
 * depend on the libc structure.
 */
typedef struct fake_tm
{
  int tm_mon;
  int tm_year;			/* FULL year - ie 1971 */
  int tm_mday;
  int tm_hour;
  int tm_min;
  int tm_sec;
} fake_tm;

/* The mktime_utc function is due to Russ Allbery (rra@stanford.edu),
 * who placed it under public domain:
 */

/* The number of days in each month. 
 */
static const int MONTHDAYS[] = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

    /* Whether a given year is a leap year. */
#define ISLEAP(year) \
        (((year) % 4) == 0 && (((year) % 100) != 0 || ((year) % 400) == 0))

/*
 **  Given a struct tm representing a calendar time in UTC, convert it to
 **  seconds since epoch.  Returns (time_t) -1 if the time is not
 **  convertable.  Note that this function does not canonicalize the provided
 **  struct tm, nor does it allow out of range values or years before 1970.
 */
static time_t
mktime_utc (const struct fake_tm *tm)
{
  time_t result = 0;
  int i;

/* We do allow some ill-formed dates, but we don't do anything special
 * with them and our callers really shouldn't pass them to us.  Do
 * explicitly disallow the ones that would cause invalid array accesses
 * or other algorithm problems. 
 */
  if (tm->tm_mon < 0 || tm->tm_mon > 11 || tm->tm_year < 1970)
    return (time_t) - 1;

/* Convert to a time_t. 
 */
  for (i = 1970; i < tm->tm_year; i++)
    result += 365 + ISLEAP (i);
  for (i = 0; i < tm->tm_mon; i++)
    result += MONTHDAYS[i];
  if (tm->tm_mon > 1 && ISLEAP (tm->tm_year))
    result++;
  result = 24 * (result + tm->tm_mday - 1) + tm->tm_hour;
  result = 60 * result + tm->tm_min;
  result = 60 * result + tm->tm_sec;
  return result;
}


/* this one will parse dates of the form:
 * month|day|hour|minute|sec* (2 chars each)
 * and year is given. Returns a time_t date.
 */
time_t
_gnutls_x509_time2gtime (const char *ttime, int year)
{
  char xx[3];
  struct fake_tm etime;
  time_t ret;

  if (strlen (ttime) < 8)
    {
      gnutls_assert ();
      return (time_t) - 1;
    }

  etime.tm_year = year;

  /* In order to work with 32 bit
   * time_t.
   */
  if (sizeof (time_t) <= 4 && etime.tm_year >= 2038)
    return (time_t) 2145914603;	/* 2037-12-31 23:23:23 */

  xx[2] = 0;

/* get the month
 */
  memcpy (xx, ttime, 2);	/* month */
  etime.tm_mon = atoi (xx) - 1;
  ttime += 2;

/* get the day
 */
  memcpy (xx, ttime, 2);	/* day */
  etime.tm_mday = atoi (xx);
  ttime += 2;

/* get the hour
 */
  memcpy (xx, ttime, 2);	/* hour */
  etime.tm_hour = atoi (xx);
  ttime += 2;

/* get the minutes
 */
  memcpy (xx, ttime, 2);	/* minutes */
  etime.tm_min = atoi (xx);
  ttime += 2;

  if (strlen (ttime) >= 2)
    {
      memcpy (xx, ttime, 2);
      etime.tm_sec = atoi (xx);
      ttime += 2;
    }
  else
    etime.tm_sec = 0;

  ret = mktime_utc (&etime);

  return ret;
}


/* returns a time_t value that contains the given time.
 * The given time is expressed as:
 * YEAR(2)|MONTH(2)|DAY(2)|HOUR(2)|MIN(2)|SEC(2)*
 *
 * (seconds are optional)
 */
time_t
_gnutls_x509_utcTime2gtime (const char *ttime)
{
  char xx[3];
  int year;

  if (strlen (ttime) < 10)
    {
      gnutls_assert ();
      return (time_t) - 1;
    }
  xx[2] = 0;
/* get the year
 */
  memcpy (xx, ttime, 2);	/* year */
  year = atoi (xx);
  ttime += 2;

  if (year > 49)
    year += 1900;
  else
    year += 2000;

  return _gnutls_x509_time2gtime (ttime, year);
}

/* returns a time value that contains the given time.
 * The given time is expressed as:
 * YEAR(2)|MONTH(2)|DAY(2)|HOUR(2)|MIN(2)|SEC(2)
 */
int
_gnutls_x509_gtime2utcTime (time_t gtime, char *str_time, int str_time_size)
{
  size_t ret;

#ifdef HAVE_GMTIME_R
  struct tm _tm;

  gmtime_r (&gtime, &_tm);

  ret = strftime (str_time, str_time_size, "%y%m%d%H%M%SZ", &_tm);
#else
  struct tm *_tm;

  _tm = gmtime (&gtime);

  ret = strftime (str_time, str_time_size, "%y%m%d%H%M%SZ", _tm);
#endif

  if (!ret)
    {
      gnutls_assert ();
      return GNUTLS_E_SHORT_MEMORY_BUFFER;
    }

  return 0;

}

/* returns a time_t value that contains the given time.
 * The given time is expressed as:
 * YEAR(4)|MONTH(2)|DAY(2)|HOUR(2)|MIN(2)|SEC(2)*
 */
time_t
_gnutls_x509_generalTime2gtime (const char *ttime)
{
  char xx[5];
  int year;

  if (strlen (ttime) < 12)
    {
      gnutls_assert ();
      return (time_t) - 1;
    }

  if (strchr (ttime, 'Z') == 0)
    {
      gnutls_assert ();
      /* sorry we don't support it yet
       */
      return (time_t) - 1;
    }
  xx[4] = 0;

/* get the year
 */
  memcpy (xx, ttime, 4);	/* year */
  year = atoi (xx);
  ttime += 4;

  return _gnutls_x509_time2gtime (ttime, year);

}

/* Extracts the time in time_t from the ASN1_TYPE given. When should
 * be something like "tbsCertList.thisUpdate".
 */
#define MAX_TIME 1024
time_t
_gnutls_x509_get_time (ASN1_TYPE c2, const char *when)
{
  char ttime[MAX_TIME];
  char name[1024];
  time_t c_time = (time_t) - 1;
  int len, result;

  _gnutls_str_cpy (name, sizeof (name), when);

  len = sizeof (ttime) - 1;
  if ((result = asn1_read_value (c2, name, ttime, &len)) < 0)
    {
      gnutls_assert ();
      return (time_t) (-1);
    }

  /* CHOICE */
  if (strcmp (ttime, "generalTime") == 0)
    {

      _gnutls_str_cat (name, sizeof (name), ".generalTime");
      len = sizeof (ttime) - 1;
      result = asn1_read_value (c2, name, ttime, &len);
      if (result == ASN1_SUCCESS)
	c_time = _gnutls_x509_generalTime2gtime (ttime);
    }
  else
    {				/* UTCTIME */

      _gnutls_str_cat (name, sizeof (name), ".utcTime");
      len = sizeof (ttime) - 1;
      result = asn1_read_value (c2, name, ttime, &len);
      if (result == ASN1_SUCCESS)
	c_time = _gnutls_x509_utcTime2gtime (ttime);
    }

  /* We cannot handle dates after 2031 in 32 bit machines.
   * a time_t of 64bits has to be used.
   */

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return (time_t) (-1);
    }
  return c_time;
}

/* Sets the time in time_t in the ASN1_TYPE given. Where should
 * be something like "tbsCertList.thisUpdate".
 */
int
_gnutls_x509_set_time (ASN1_TYPE c2, const char *where, time_t tim)
{
  char str_time[MAX_TIME];
  char name[1024];
  int result, len;

  _gnutls_str_cpy (name, sizeof (name), where);

  if ((result = asn1_write_value (c2, name, "utcTime", 1)) < 0)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  result = _gnutls_x509_gtime2utcTime (tim, str_time, sizeof (str_time));
  if (result < 0)
    {
      gnutls_assert ();
      return result;
    }

  _gnutls_str_cat (name, sizeof (name), ".utcTime");

  len = strlen (str_time);
  result = asn1_write_value (c2, name, str_time, len);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  return 0;
}


gnutls_x509_subject_alt_name_t
_gnutls_x509_san_find_type (char *str_type)
{
  if (strcmp (str_type, "dNSName") == 0)
    return GNUTLS_SAN_DNSNAME;
  if (strcmp (str_type, "rfc822Name") == 0)
    return GNUTLS_SAN_RFC822NAME;
  if (strcmp (str_type, "uniformResourceIdentifier") == 0)
    return GNUTLS_SAN_URI;
  if (strcmp (str_type, "iPAddress") == 0)
    return GNUTLS_SAN_IPADDRESS;
  return (gnutls_x509_subject_alt_name_t) - 1;
}

/* A generic export function. Will export the given ASN.1 encoded data
 * to PEM or DER raw data.
 */
int
_gnutls_x509_export_int (ASN1_TYPE asn1_data,
			 gnutls_x509_crt_fmt_t format, char *pem_header,
			 int tmp_buf_size, unsigned char *output_data,
			 size_t * output_data_size)
{
  int result, len;
  if (tmp_buf_size == 0)
    tmp_buf_size = 16 * 1024;

  if (format == GNUTLS_X509_FMT_DER)
    {

      if (output_data == NULL)
	*output_data_size = 0;

      len = *output_data_size;

      if ((result =
	   asn1_der_coding (asn1_data, "", output_data, &len,
			    NULL)) != ASN1_SUCCESS)
	{
	  *output_data_size = len;
	  if (result == ASN1_MEM_ERROR)
	    {
	      return GNUTLS_E_SHORT_MEMORY_BUFFER;
	    }
	  gnutls_assert ();
	  return _gnutls_asn2err (result);
	}

      *output_data_size = len;

    }
  else
    {				/* PEM */
      opaque *tmp;
      opaque *out;

      len = tmp_buf_size;

      tmp = gnutls_alloca (len);
      if (tmp == NULL)
	{
	  gnutls_assert ();
	  return GNUTLS_E_MEMORY_ERROR;
	}

      if ((result =
	   asn1_der_coding (asn1_data, "", tmp, &len, NULL)) != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  if (result == ASN1_MEM_ERROR)
	    {
	      *output_data_size = B64FSIZE (strlen (pem_header), len) + 1;
	    }
	  gnutls_afree (tmp);
	  return _gnutls_asn2err (result);
	}

      result = _gnutls_fbase64_encode (pem_header, tmp, len, &out);

      gnutls_afree (tmp);

      if (result < 0)
	{
	  gnutls_assert ();
	  return result;
	}

      if (result == 0)
	{			/* oooops */
	  gnutls_assert ();
	  return GNUTLS_E_INTERNAL_ERROR;
	}

      if ((unsigned) result > *output_data_size)
	{
	  gnutls_assert ();
	  gnutls_free (out);
	  *output_data_size = result;
	  return GNUTLS_E_SHORT_MEMORY_BUFFER;
	}

      *output_data_size = result;

      if (output_data)
	{
	  memcpy (output_data, out, result);

	  /* do not include the null character into output size.
	   */
	  *output_data_size = result - 1;
	}
      gnutls_free (out);

    }

  return 0;
}

/* Decodes an octet string. Leave string_type null for a normal
 * octet string. Otherwise put something like BMPString, PrintableString
 * etc.
 */
int
_gnutls_x509_decode_octet_string (const char *string_type,
				  const opaque * der, size_t der_size,
				  opaque * output, size_t * output_size)
{
  ASN1_TYPE c2 = ASN1_TYPE_EMPTY;
  int result, tmp_output_size;
  char strname[64];

  if (string_type == NULL)
    _gnutls_str_cpy (strname, sizeof (strname), "PKIX1.pkcs-7-Data");
  else
    {
      _gnutls_str_cpy (strname, sizeof (strname), "PKIX1.");
      _gnutls_str_cat (strname, sizeof (strname), string_type);
    }

  if ((result = asn1_create_element
       (_gnutls_get_pkix (), strname, &c2)) != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto cleanup;
    }

  result = asn1_der_decoding (&c2, der, der_size, NULL);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto cleanup;
    }

  tmp_output_size = *output_size;
  result = asn1_read_value (c2, "", output, &tmp_output_size);
  *output_size = tmp_output_size;

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto cleanup;
    }

  return 0;

cleanup:
  if (c2)
    asn1_delete_structure (&c2);

  return result;
}


/* Reads a value from an ASN1 tree, and puts the output
 * in an allocated variable in the given datum.
 * If str is non zero, then the output will be treated as
 * an octet string.
 */
int
_gnutls_x509_read_value (ASN1_TYPE c, const char *root,
			 gnutls_datum_t * ret, int str)
{
  int len = 0, result;
  size_t slen;
  opaque *tmp = NULL;

  result = asn1_read_value (c, root, NULL, &len);
  if (result != ASN1_MEM_ERROR)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      return result;
    }

  tmp = gnutls_malloc (len);
  if (tmp == NULL)
    {
      gnutls_assert ();
      result = GNUTLS_E_MEMORY_ERROR;
      goto cleanup;
    }

  result = asn1_read_value (c, root, tmp, &len);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto cleanup;
    }

  /* Extract the OCTET STRING.
   */

  if (str)
    {
      slen = len;
      result = _gnutls_x509_decode_octet_string (NULL, tmp, slen, tmp, &slen);
      if (result < 0)
	{
	  gnutls_assert ();
	  goto cleanup;
	}
      len = slen;
    }

  ret->data = tmp;
  ret->size = len;

  return 0;

cleanup:
  gnutls_free (tmp);
  return result;

}

/* DER Encodes the src ASN1_TYPE and stores it to
 * the given datum. If str is non null then the data are encoded as
 * an OCTET STRING.
 */
int
_gnutls_x509_der_encode (ASN1_TYPE src, const char *src_name,
			 gnutls_datum_t * res, int str)
{
  int size, result;
  int asize;
  opaque *data = NULL;
  ASN1_TYPE c2 = ASN1_TYPE_EMPTY;

  size = 0;
  result = asn1_der_coding (src, src_name, NULL, &size, NULL);
  if (result != ASN1_MEM_ERROR)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto cleanup;
    }

  /* allocate data for the der
   */

  if (str)
    size += 16;			/* for later to include the octet tags */
  asize = size;

  data = gnutls_malloc (size);
  if (data == NULL)
    {
      gnutls_assert ();
      result = GNUTLS_E_MEMORY_ERROR;
      goto cleanup;
    }

  result = asn1_der_coding (src, src_name, data, &size, NULL);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto cleanup;
    }

  if (str)
    {
      if ((result = asn1_create_element
	   (_gnutls_get_pkix (), "PKIX1.pkcs-7-Data", &c2)) != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  result = _gnutls_asn2err (result);
	  goto cleanup;
	}

      result = asn1_write_value (c2, "", data, size);
      if (result != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  result = _gnutls_asn2err (result);
	  goto cleanup;
	}

      result = asn1_der_coding (c2, "", data, &asize, NULL);
      if (result != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  result = _gnutls_asn2err (result);
	  goto cleanup;
	}

      size = asize;

      asn1_delete_structure (&c2);
    }

  res->data = data;
  res->size = size;
  return 0;

cleanup:
  gnutls_free (data);
  asn1_delete_structure (&c2);
  return result;

}

/* DER Encodes the src ASN1_TYPE and stores it to
 * dest in dest_name. Useful to encode something and store it
 * as OCTET. If str is non null then the data are encoded as
 * an OCTET STRING.
 */
int
_gnutls_x509_der_encode_and_copy (ASN1_TYPE src, const char *src_name,
				  ASN1_TYPE dest, const char *dest_name,
				  int str)
{
  int result;
  gnutls_datum_t encoded;

  result = _gnutls_x509_der_encode (src, src_name, &encoded, str);

  if (result < 0)
    {
      gnutls_assert ();
      return result;
    }

  /* Write the data.
   */
  result = asn1_write_value (dest, dest_name, encoded.data, encoded.size);

  _gnutls_free_datum (&encoded);

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  return 0;
}

/* Writes the value of the datum in the given ASN1_TYPE. If str is non
 * zero it encodes it as OCTET STRING.
 */
int
_gnutls_x509_write_value (ASN1_TYPE c, const char *root,
			  const gnutls_datum_t * data, int str)
{
  int result;
  int asize;
  ASN1_TYPE c2 = ASN1_TYPE_EMPTY;
  gnutls_datum_t val;

  asize = data->size + 16;

  val.data = gnutls_malloc (asize);
  if (val.data == NULL)
    {
      gnutls_assert ();
      result = GNUTLS_E_MEMORY_ERROR;
      goto cleanup;
    }

  if (str)
    {
      /* Convert it to OCTET STRING
       */
      if ((result = asn1_create_element
	   (_gnutls_get_pkix (), "PKIX1.pkcs-7-Data", &c2)) != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  result = _gnutls_asn2err (result);
	  goto cleanup;
	}

      result = asn1_write_value (c2, "", data->data, data->size);
      if (result != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  result = _gnutls_asn2err (result);
	  goto cleanup;
	}

      result = _gnutls_x509_der_encode (c2, "", &val, 0);
      if (result < 0)
	{
	  gnutls_assert ();
	  goto cleanup;
	}

    }
  else
    {
      val.data = data->data;
      val.size = data->size;
    }

  /* Write the data.
   */
  result = asn1_write_value (c, root, val.data, val.size);

  if (val.data != data->data)
    _gnutls_free_datum (&val);

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  return 0;

cleanup:
  if (val.data != data->data)
    _gnutls_free_datum (&val);
  return result;
}

/* Encodes and copies the private key parameters into a
 * subjectPublicKeyInfo structure.
 *
 */
int
_gnutls_x509_encode_and_copy_PKI_params (ASN1_TYPE dst,
					 const char *dst_name,
					 gnutls_pk_algorithm_t
					 pk_algorithm, mpi_t * params,
					 int params_size)
{
  const char *pk;
  gnutls_datum_t der = { NULL, 0 };
  int result;
  char name[128];

  pk = _gnutls_x509_pk_to_oid (pk_algorithm);
  if (pk == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_UNKNOWN_PK_ALGORITHM;
    }

  /* write the OID
   */
  _gnutls_str_cpy (name, sizeof (name), dst_name);
  _gnutls_str_cat (name, sizeof (name), ".algorithm.algorithm");
  result = asn1_write_value (dst, name, pk, 1);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  if (pk_algorithm == GNUTLS_PK_RSA)
    {
      /* disable parameters, which are not used in RSA.
       */
      _gnutls_str_cpy (name, sizeof (name), dst_name);
      _gnutls_str_cat (name, sizeof (name), ".algorithm.parameters");
      result = asn1_write_value (dst, name, NULL, 0);
      if (result != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  return _gnutls_asn2err (result);
	}

      result = _gnutls_x509_write_rsa_params (params, params_size, &der);
      if (result < 0)
	{
	  gnutls_assert ();
	  return result;
	}

      /* Write the DER parameters. (in bits)
       */
      _gnutls_str_cpy (name, sizeof (name), dst_name);
      _gnutls_str_cat (name, sizeof (name), ".subjectPublicKey");
      result = asn1_write_value (dst, name, der.data, der.size * 8);

      _gnutls_free_datum (&der);

      if (result != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  return _gnutls_asn2err (result);
	}
    }
  else if (pk_algorithm == GNUTLS_PK_DSA)
    {

      result = _gnutls_x509_write_dsa_params (params, params_size, &der);
      if (result < 0)
	{
	  gnutls_assert ();
	  return result;
	}

      /* Write the DER parameters.
       */
      _gnutls_str_cpy (name, sizeof (name), dst_name);
      _gnutls_str_cat (name, sizeof (name), ".algorithm.parameters");
      result = asn1_write_value (dst, name, der.data, der.size);

      _gnutls_free_datum (&der);

      if (result != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  return _gnutls_asn2err (result);
	}

      result = _gnutls_x509_write_dsa_public_key (params, params_size, &der);
      if (result < 0)
	{
	  gnutls_assert ();
	  return result;
	}

      _gnutls_str_cpy (name, sizeof (name), dst_name);
      _gnutls_str_cat (name, sizeof (name), ".subjectPublicKey");
      result = asn1_write_value (dst, name, der.data, der.size * 8);

      _gnutls_free_datum (&der);

      if (result != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  return _gnutls_asn2err (result);
	}

    }
  else
    return GNUTLS_E_UNIMPLEMENTED_FEATURE;

  return 0;
}

/* Reads and returns the PK algorithm of the given certificate-like
 * ASN.1 structure. src_name should be something like "tbsCertificate.subjectPublicKeyInfo".
 */
int
_gnutls_x509_get_pk_algorithm (ASN1_TYPE src, const char *src_name,
			       unsigned int *bits)
{
  int result;
  opaque *str = NULL;
  int algo;
  char oid[64];
  int len;
  mpi_t params[MAX_PUBLIC_PARAMS_SIZE];
  char name[128];

  _gnutls_str_cpy (name, sizeof (name), src_name);
  _gnutls_str_cat (name, sizeof (name), ".algorithm.algorithm");

  len = sizeof (oid);
  result = asn1_read_value (src, name, oid, &len);

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  algo = _gnutls_x509_oid2pk_algorithm (oid);

  if (bits == NULL)
    {
      gnutls_free (str);
      return algo;
    }

  /* Now read the parameters' bits 
   */
  _gnutls_str_cpy (name, sizeof (name), src_name);
  _gnutls_str_cat (name, sizeof (name), ".subjectPublicKey");

  len = 0;
  result = asn1_read_value (src, name, NULL, &len);
  if (result != ASN1_MEM_ERROR)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  if (len % 8 != 0)
    {
      gnutls_assert ();
      return GNUTLS_E_CERTIFICATE_ERROR;
    }

  len /= 8;

  str = gnutls_malloc (len);
  if (str == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_MEMORY_ERROR;
    }

  _gnutls_str_cpy (name, sizeof (name), src_name);
  _gnutls_str_cat (name, sizeof (name), ".subjectPublicKey");

  result = asn1_read_value (src, name, str, &len);

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      gnutls_free (str);
      return _gnutls_asn2err (result);
    }

  len /= 8;

  if (algo == GNUTLS_PK_RSA)
    {
      if ((result = _gnutls_x509_read_rsa_params (str, len, params)) < 0)
	{
	  gnutls_assert ();
	  return result;
	}

      bits[0] = _gnutls_mpi_get_nbits (params[0]);

      _gnutls_mpi_release (&params[0]);
      _gnutls_mpi_release (&params[1]);
    }

  if (algo == GNUTLS_PK_DSA)
    {

      if ((result = _gnutls_x509_read_dsa_pubkey (str, len, params)) < 0)
	{
	  gnutls_assert ();
	  return result;
	}

      bits[0] = _gnutls_mpi_get_nbits (params[3]);

      _gnutls_mpi_release (&params[3]);
    }

  gnutls_free (str);
  return algo;
}

/* Reads the DER signed data from the certificate and allocates space and
 * returns them into signed_data.
 */
int
_gnutls_x509_get_signed_data (ASN1_TYPE src, const char *src_name,
			      gnutls_datum_t * signed_data)
{
  gnutls_datum_t der;
  int start, end, result;

  result = _gnutls_x509_der_encode (src, "", &der, 0);
  if (result < 0)
    {
      gnutls_assert ();
      return result;
    }

  /* Get the signed data
   */
  result = asn1_der_decoding_startEnd (src, der.data, der.size,
				       src_name, &start, &end);
  if (result != ASN1_SUCCESS)
    {
      result = _gnutls_asn2err (result);
      gnutls_assert ();
      goto cleanup;
    }

  result = _gnutls_set_datum (signed_data, &der.data[start], end - start + 1);

  if (result < 0)
    {
      gnutls_assert ();
      goto cleanup;
    }

  result = 0;

cleanup:
  _gnutls_free_datum (&der);

  return result;
}

/* Reads the DER signature from the certificate and allocates space and
 * returns them into signed_data.
 */
int
_gnutls_x509_get_signature (ASN1_TYPE src, const char *src_name,
			    gnutls_datum_t * signature)
{
  int bits, result, len;

  signature->data = NULL;
  signature->size = 0;

  /* Read the signature 
   */
  bits = 0;
  result = asn1_read_value (src, src_name, NULL, &bits);

  if (result != ASN1_MEM_ERROR)
    {
      result = _gnutls_asn2err (result);
      gnutls_assert ();
      goto cleanup;
    }

  if (bits % 8 != 0)
    {
      gnutls_assert ();
      result = GNUTLS_E_CERTIFICATE_ERROR;
      goto cleanup;
    }

  len = bits / 8;

  signature->data = gnutls_malloc (len);
  if (signature->data == NULL)
    {
      gnutls_assert ();
      result = GNUTLS_E_MEMORY_ERROR;
      return result;
    }

  /* read the bit string of the signature
   */
  bits = len;
  result = asn1_read_value (src, src_name, signature->data, &bits);

  if (result != ASN1_SUCCESS)
    {
      result = _gnutls_asn2err (result);
      gnutls_assert ();
      goto cleanup;
    }

  signature->size = len;

  return 0;

cleanup:
  return result;
}
