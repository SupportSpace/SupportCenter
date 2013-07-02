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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA
 *
 */

/* Functions on X.509 Certificate parsing
 */

#include <gnutls_int.h>
#include <gnutls_datum.h>
#include <gnutls_global.h>
#include <gnutls_errors.h>
#include <common.h>
#include <gnutls_x509.h>
#include <x509_b64.h>
#include <x509.h>
#include <dn.h>
#include <extensions.h>
#include <libtasn1.h>
#include <mpi.h>
#include <privkey.h>
#include <verify.h>

/**
  * gnutls_x509_crt_init - This function initializes a gnutls_x509_crt_t structure
  * @cert: The structure to be initialized
  *
  * This function will initialize an X.509 certificate structure. 
  *
  * Returns 0 on success.
  *
  **/
int
gnutls_x509_crt_init (gnutls_x509_crt_t * cert)
{
  *cert = gnutls_calloc (1, sizeof (gnutls_x509_crt_int));

  if (*cert)
    {
      int result = asn1_create_element (_gnutls_get_pkix (),
					"PKIX1.Certificate",
					&(*cert)->cert);
      if (result != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  gnutls_free (*cert);
	  return _gnutls_asn2err (result);
	}
      return 0;			/* success */
    }
  return GNUTLS_E_MEMORY_ERROR;
}

/*-
  * _gnutls_x509_crt_cpy - This function copies a gnutls_x509_crt_t structure
  * @dest: The structure where to copy
  * @src: The structure to be copied
  *
  * This function will copy an X.509 certificate structure. 
  *
  * Returns 0 on success.
  *
  -*/
int
_gnutls_x509_crt_cpy (gnutls_x509_crt_t dest, gnutls_x509_crt_t src)
{
  int ret;
  size_t der_size;
  opaque *der;
  gnutls_datum_t tmp;

  ret = gnutls_x509_crt_export (src, GNUTLS_X509_FMT_DER, NULL, &der_size);
  if (ret != GNUTLS_E_SHORT_MEMORY_BUFFER)
    {
      gnutls_assert ();
      return ret;
    }

  der = gnutls_alloca (der_size);
  if (der == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_MEMORY_ERROR;
    }

  ret = gnutls_x509_crt_export (src, GNUTLS_X509_FMT_DER, der, &der_size);
  if (ret < 0)
    {
      gnutls_assert ();
      gnutls_afree (der);
      return ret;
    }

  tmp.data = der;
  tmp.size = der_size;
  ret = gnutls_x509_crt_import (dest, &tmp, GNUTLS_X509_FMT_DER);

  gnutls_afree (der);

  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  return 0;

}

/**
  * gnutls_x509_crt_deinit - This function deinitializes memory used by a gnutls_x509_crt_t structure
  * @cert: The structure to be initialized
  *
  * This function will deinitialize a CRL structure. 
  *
  **/
void
gnutls_x509_crt_deinit (gnutls_x509_crt_t cert)
{
  if (!cert)
    return;

  if (cert->cert)
    asn1_delete_structure (&cert->cert);

  gnutls_free (cert);
}

/**
  * gnutls_x509_crt_import - This function will import a DER or PEM encoded Certificate
  * @cert: The structure to store the parsed certificate.
  * @data: The DER or PEM encoded certificate.
  * @format: One of DER or PEM
  *
  * This function will convert the given DER or PEM encoded Certificate
  * to the native gnutls_x509_crt_t format. The output will be stored in @cert.
  *
  * If the Certificate is PEM encoded it should have a header of "X509 CERTIFICATE", or
  * "CERTIFICATE".
  *
  * Returns 0 on success.
  *
  **/
int
gnutls_x509_crt_import (gnutls_x509_crt_t cert,
			const gnutls_datum_t * data,
			gnutls_x509_crt_fmt_t format)
{
  int result = 0, need_free = 0;
  gnutls_datum_t _data;
  opaque *signature = NULL;

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  _data.data = data->data;
  _data.size = data->size;

  /* If the Certificate is in PEM format then decode it
   */
  if (format == GNUTLS_X509_FMT_PEM)
    {
      opaque *out;

      /* Try the first header */
      result =
	_gnutls_fbase64_decode (PEM_X509_CERT2, data->data, data->size, &out);

      if (result <= 0)
	{
	  /* try for the second header */
	  result =
	    _gnutls_fbase64_decode (PEM_X509_CERT, data->data,
				    data->size, &out);

	  if (result <= 0)
	    {
	      if (result == 0)
		result = GNUTLS_E_INTERNAL_ERROR;
	      gnutls_assert ();
	      return result;
	    }
	}

      _data.data = out;
      _data.size = result;

      need_free = 1;
    }

  result = asn1_der_decoding (&cert->cert, _data.data, _data.size, NULL);
  if (result != ASN1_SUCCESS)
    {
      result = _gnutls_asn2err (result);
      gnutls_assert ();
      goto cleanup;
    }

  /* Since we do not want to disable any extension
   */
  cert->use_extensions = 1;
  if (need_free)
    _gnutls_free_datum (&_data);

  return 0;

cleanup:
  gnutls_free (signature);
  if (need_free)
    _gnutls_free_datum (&_data);
  return result;
}


/**
  * gnutls_x509_crt_get_issuer_dn - This function returns the Certificate's issuer distinguished name
  * @cert: should contain a gnutls_x509_crt_t structure
  * @buf: a pointer to a structure to hold the name (may be null)
  * @sizeof_buf: initially holds the size of @buf
  *
  * This function will copy the name of the Certificate issuer in the
  * provided buffer. The name will be in the form
  * "C=xxxx,O=yyyy,CN=zzzz" as described in RFC2253. The output string
  * will be ASCII or UTF-8 encoded, depending on the certificate data.
  *
  * If @buf is null then only the size will be filled.
  *
  * Returns GNUTLS_E_SHORT_MEMORY_BUFFER if the provided buffer is not
  * long enough, and in that case the *sizeof_buf will be updated with
  * the required size.  On success 0 is returned.
  *
  **/
int
gnutls_x509_crt_get_issuer_dn (gnutls_x509_crt_t cert, char *buf,
			       size_t * sizeof_buf)
{
  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  return _gnutls_x509_parse_dn (cert->cert,
				"tbsCertificate.issuer.rdnSequence", buf,
				sizeof_buf);
}

/**
  * gnutls_x509_crt_get_issuer_dn_by_oid - This function returns the Certificate's issuer distinguished name
  * @cert: should contain a gnutls_x509_crt_t structure
  * @oid: holds an Object Identified in null terminated string
  * @indx: In case multiple same OIDs exist in the RDN, this specifies which to send. Use zero to get the first one.
  * @raw_flag: If non zero returns the raw DER data of the DN part.
  * @buf: a pointer to a structure to hold the name (may be null)
  * @sizeof_buf: initially holds the size of @buf
  *
  * This function will extract the part of the name of the Certificate
  * issuer specified by the given OID. The output, if the raw flag is not
  * used, will be encoded as described in RFC2253. Thus a string that is
  * ASCII or UTF-8 encoded, depending on the certificate data.
  *
  * Some helper macros with popular OIDs can be found in gnutls/x509.h
  * If raw flag is zero, this function will only return known OIDs as
  * text. Other OIDs will be DER encoded, as described in RFC2253 --
  * in hex format with a '\#' prefix.  You can check about known OIDs
  * using gnutls_x509_dn_oid_known().
  *
  * If @buf is null then only the size will be filled.
  *
  * Returns GNUTLS_E_SHORT_MEMORY_BUFFER if the provided buffer is not
  * long enough, and in that case the *sizeof_buf will be updated with
  * the required size.  On success 0 is returned.
  *
  **/
int
gnutls_x509_crt_get_issuer_dn_by_oid (gnutls_x509_crt_t cert,
				      const char *oid, int indx,
				      unsigned int raw_flag, void *buf,
				      size_t * sizeof_buf)
{
  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  return _gnutls_x509_parse_dn_oid (cert->cert,
				    "tbsCertificate.issuer.rdnSequence",
				    oid, indx, raw_flag, buf, sizeof_buf);
}

/**
  * gnutls_x509_crt_get_issuer_dn_oid - This function returns the Certificate's issuer distinguished name OIDs
  * @cert: should contain a gnutls_x509_crt_t structure
  * @indx: This specifies which OID to return. Use zero to get the first one.
  * @oid: a pointer to a buffer to hold the OID (may be null)
  * @sizeof_oid: initially holds the size of @oid
  *
  * This function will extract the OIDs of the name of the Certificate
  * issuer specified by the given index.
  *
  * If @oid is null then only the size will be filled.
  *
  * Returns GNUTLS_E_SHORT_MEMORY_BUFFER if the provided buffer is not
  * long enough, and in that case the *sizeof_oid will be updated with
  * the required size.  On success 0 is returned.
  *
  **/
int
gnutls_x509_crt_get_issuer_dn_oid (gnutls_x509_crt_t cert,
				   int indx, void *oid, size_t * sizeof_oid)
{
  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  return _gnutls_x509_get_dn_oid (cert->cert,
				  "tbsCertificate.issuer.rdnSequence",
				  indx, oid, sizeof_oid);
}

/**
  * gnutls_x509_crt_get_dn - This function returns the Certificate's distinguished name
  * @cert: should contain a gnutls_x509_crt_t structure
  * @buf: a pointer to a structure to hold the name (may be null)
  * @sizeof_buf: initially holds the size of @buf
  *
  * This function will copy the name of the Certificate in the
  * provided buffer. The name will be in the form
  * "C=xxxx,O=yyyy,CN=zzzz" as described in RFC2253. The output string
  * will be ASCII or UTF-8 encoded, depending on the certificate data.
  *
  * If @buf is null then only the size will be filled.
  *
  * Returns GNUTLS_E_SHORT_MEMORY_BUFFER if the provided buffer is not
  * long enough, and in that case the *sizeof_buf will be updated with
  * the required size.  On success 0 is returned.
  *
  **/
int
gnutls_x509_crt_get_dn (gnutls_x509_crt_t cert, char *buf,
			size_t * sizeof_buf)
{
  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  return _gnutls_x509_parse_dn (cert->cert,
				"tbsCertificate.subject.rdnSequence", buf,
				sizeof_buf);
}

/**
  * gnutls_x509_crt_get_dn_by_oid - This function returns the Certificate's distinguished name
  * @cert: should contain a gnutls_x509_crt_t structure
  * @oid: holds an Object Identified in null terminated string
  * @indx: In case multiple same OIDs exist in the RDN, this specifies which to send. Use zero to get the first one.
  * @raw_flag: If non zero returns the raw DER data of the DN part.
  * @buf: a pointer where the DN part will be copied (may be null).
  * @sizeof_buf: initially holds the size of @buf
  *
  * This function will extract the part of the name of the Certificate
  * subject specified by the given OID. The output, if the raw flag is not
  * used, will be encoded as described in RFC2253. Thus a string that is
  * ASCII or UTF-8 encoded, depending on the certificate data.
  *
  * Some helper macros with popular OIDs can be found in gnutls/x509.h
  * If raw flag is zero, this function will only return known OIDs as
  * text. Other OIDs will be DER encoded, as described in RFC2253 --
  * in hex format with a '\#' prefix.  You can check about known OIDs
  * using gnutls_x509_dn_oid_known().
  *
  * If @buf is null then only the size will be filled.
  *
  * Returns GNUTLS_E_SHORT_MEMORY_BUFFER if the provided buffer is not
  * long enough, and in that case the *sizeof_buf will be updated with
  * the required size.  On success 0 is returned.
  *
  **/
int
gnutls_x509_crt_get_dn_by_oid (gnutls_x509_crt_t cert, const char *oid,
			       int indx, unsigned int raw_flag,
			       void *buf, size_t * sizeof_buf)
{
  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  return _gnutls_x509_parse_dn_oid (cert->cert,
				    "tbsCertificate.subject.rdnSequence",
				    oid, indx, raw_flag, buf, sizeof_buf);
}

/**
  * gnutls_x509_crt_get_dn_oid - This function returns the Certificate's subject distinguished name OIDs
  * @cert: should contain a gnutls_x509_crt_t structure
  * @indx: This specifies which OID to return. Use zero to get the first one.
  * @oid: a pointer to a buffer to hold the OID (may be null)
  * @sizeof_oid: initially holds the size of @oid
  *
  * This function will extract the OIDs of the name of the Certificate
  * subject specified by the given index.
  *
  * If oid is null then only the size will be filled.
  *
  * Returns GNUTLS_E_SHORT_MEMORY_BUFFER if the provided buffer is not
  * long enough, and in that case the *sizeof_oid will be updated with
  * the required size.  On success 0 is returned.
  *
  **/
int
gnutls_x509_crt_get_dn_oid (gnutls_x509_crt_t cert,
			    int indx, void *oid, size_t * sizeof_oid)
{
  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  return _gnutls_x509_get_dn_oid (cert->cert,
				  "tbsCertificate.subject.rdnSequence",
				  indx, oid, sizeof_oid);
}

/**
  * gnutls_x509_crt_get_signature_algorithm - This function returns the Certificate's signature algorithm
  * @cert: should contain a gnutls_x509_crt_t structure
  *
  * This function will return a value of the gnutls_sign_algorithm_t enumeration that 
  * is the signature algorithm. 
  *
  * Returns a negative value on error.
  *
  **/
int
gnutls_x509_crt_get_signature_algorithm (gnutls_x509_crt_t cert)
{
  int result;
  gnutls_datum_t sa;

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  /* Read the signature algorithm. Note that parameters are not
   * read. They will be read from the issuer's certificate if needed.
   */
  result =
    _gnutls_x509_read_value (cert->cert, "signatureAlgorithm.algorithm",
			     &sa, 0);

  if (result < 0)
    {
      gnutls_assert ();
      return result;
    }

  result = _gnutls_x509_oid2sign_algorithm (sa.data);

  _gnutls_free_datum (&sa);

  return result;
}

/**
  * gnutls_x509_crt_get_version - This function returns the Certificate's version number
  * @cert: should contain a gnutls_x509_crt_t structure
  *
  * This function will return the version of the specified Certificate.
  *
  * Returns a negative value on error.
  *
  **/
int
gnutls_x509_crt_get_version (gnutls_x509_crt_t cert)
{
  opaque version[5];
  int len, result;

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  len = sizeof (version);
  if ((result =
       asn1_read_value (cert->cert, "tbsCertificate.version", version,
			&len)) != ASN1_SUCCESS)
    {

      if (result == ASN1_ELEMENT_NOT_FOUND)
	return 1;		/* the DEFAULT version */
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  return (int) version[0] + 1;
}

/**
  * gnutls_x509_crt_get_activation_time - This function returns the Certificate's activation time
  * @cert: should contain a gnutls_x509_crt_t structure
  *
  * This function will return the time this Certificate was or will be activated.
  *
  * Returns (time_t)-1 on error.
  *
  **/
time_t
gnutls_x509_crt_get_activation_time (gnutls_x509_crt_t cert)
{
  if (cert == NULL)
    {
      gnutls_assert ();
      return (time_t) - 1;
    }

  return _gnutls_x509_get_time (cert->cert,
				"tbsCertificate.validity.notBefore");
}

/**
  * gnutls_x509_crt_get_expiration_time - This function returns the Certificate's expiration time
  * @cert: should contain a gnutls_x509_crt_t structure
  *
  * This function will return the time this Certificate was or will be expired.
  *
  * Returns (time_t)-1 on error.
  *
  **/
time_t
gnutls_x509_crt_get_expiration_time (gnutls_x509_crt_t cert)
{
  if (cert == NULL)
    {
      gnutls_assert ();
      return (time_t) - 1;
    }

  return _gnutls_x509_get_time (cert->cert,
				"tbsCertificate.validity.notAfter");
}

/**
  * gnutls_x509_crt_get_serial - This function returns the certificate's serial number
  * @cert: should contain a gnutls_x509_crt_t structure
  * @result: The place where the serial number will be copied
  * @result_size: Holds the size of the result field.
  *
  * This function will return the X.509 certificate's serial number. 
  * This is obtained by the X509 Certificate serialNumber
  * field. Serial is not always a 32 or 64bit number. Some CAs use
  * large serial numbers, thus it may be wise to handle it as something
  * opaque. 
  *
  * Returns 0 on success and a negative value in case of an error.
  *
  **/
int
gnutls_x509_crt_get_serial (gnutls_x509_crt_t cert, void *result,
			    size_t * result_size)
{
  int ret, len;

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  len = *result_size;
  ret =
    asn1_read_value (cert->cert, "tbsCertificate.serialNumber", result, &len);
  *result_size = len;

  if (ret != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (ret);
    }

  return 0;
}

/**
  * gnutls_x509_crt_get_subject_key_id - This function returns the certificate's key identifier
  * @cert: should contain a gnutls_x509_crt_t structure
  * @result: The place where the identifier will be copied
  * @result_size: Holds the size of the result field.
  * @critical: will be non zero if the extension is marked as critical (may be null)
  *
  * This function will return the X.509v3 certificate's subject key identifier.
  * This is obtained by the X.509 Subject Key identifier extension
  * field (2.5.29.14). 
  *
  * Returns 0 on success and a negative value in case of an error.
  *
  **/
int
gnutls_x509_crt_get_subject_key_id (gnutls_x509_crt_t cert, void *ret,
				    size_t * ret_size, unsigned int *critical)
{
  int result, len;
  gnutls_datum_t id;
  ASN1_TYPE c2 = ASN1_TYPE_EMPTY;

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }


  if (ret)
    memset (ret, 0, *ret_size);
  else
    *ret_size = 0;

  if ((result =
       _gnutls_x509_crt_get_extension (cert, "2.5.29.14", 0, &id,
				       critical)) < 0)
    {
      return result;
    }

  if (id.size == 0 || id.data == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
    }

  result = asn1_create_element
    (_gnutls_get_pkix (), "PKIX1.SubjectKeyIdentifier", &c2);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      _gnutls_free_datum (&id);
      return _gnutls_asn2err (result);
    }

  result = asn1_der_decoding (&c2, id.data, id.size, NULL);
  _gnutls_free_datum (&id);

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      asn1_delete_structure (&c2);
      return _gnutls_asn2err (result);
    }

  len = *ret_size;
  result = asn1_read_value (c2, "", ret, &len);

  *ret_size = len;
  asn1_delete_structure (&c2);

  if (result == ASN1_VALUE_NOT_FOUND || result == ASN1_ELEMENT_NOT_FOUND)
    {
      return GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
    }

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  return 0;
}

/**
  * gnutls_x509_crt_get_authority_key_id - This function returns the certificate authority's identifier
  * @cert: should contain a gnutls_x509_crt_t structure
  * @result: The place where the identifier will be copied
  * @result_size: Holds the size of the result field.
  * @critical: will be non zero if the extension is marked as critical (may be null)
  *
  * This function will return the X.509v3 certificate authority's key identifier.
  * This is obtained by the X.509 Authority Key identifier extension
  * field (2.5.29.35). Note that this function only returns the keyIdentifier
  * field of the extension.
  *
  * Returns 0 on success and a negative value in case of an error.
  *
  **/
int
gnutls_x509_crt_get_authority_key_id (gnutls_x509_crt_t cert, void *ret,
				      size_t * ret_size,
				      unsigned int *critical)
{
  int result, len;
  gnutls_datum_t id;
  ASN1_TYPE c2 = ASN1_TYPE_EMPTY;

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }


  if (ret)
    memset (ret, 0, *ret_size);
  else
    *ret_size = 0;

  if ((result =
       _gnutls_x509_crt_get_extension (cert, "2.5.29.35", 0, &id,
				       critical)) < 0)
    {
      return result;
    }

  if (id.size == 0 || id.data == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
    }

  result = asn1_create_element
    (_gnutls_get_pkix (), "PKIX1.AuthorityKeyIdentifier", &c2);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      _gnutls_free_datum (&id);
      return _gnutls_asn2err (result);
    }

  result = asn1_der_decoding (&c2, id.data, id.size, NULL);
  _gnutls_free_datum (&id);

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      asn1_delete_structure (&c2);
      return _gnutls_asn2err (result);
    }

  len = *ret_size;
  result = asn1_read_value (c2, "keyIdentifier", ret, &len);

  *ret_size = len;
  asn1_delete_structure (&c2);

  if (result == ASN1_VALUE_NOT_FOUND || result == ASN1_ELEMENT_NOT_FOUND)
    {
      return GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
    }

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  return 0;
}

/**
  * gnutls_x509_crt_get_pk_algorithm - This function returns the certificate's PublicKey algorithm
  * @cert: should contain a gnutls_x509_crt_t structure
  * @bits: if bits is non null it will hold the size of the parameters' in bits
  *
  * This function will return the public key algorithm of an X.509 
  * certificate.
  *
  * If bits is non null, it should have enough size to hold the parameters
  * size in bits. For RSA the bits returned is the modulus. 
  * For DSA the bits returned are of the public
  * exponent.
  *
  * Returns a member of the gnutls_pk_algorithm_t enumeration on success,
  * or a negative value on error.
  *
  **/
int
gnutls_x509_crt_get_pk_algorithm (gnutls_x509_crt_t cert, unsigned int *bits)
{
  int result;

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  result =
    _gnutls_x509_get_pk_algorithm (cert->cert,
				   "tbsCertificate.subjectPublicKeyInfo",
				   bits);

  if (result < 0)
    {
      gnutls_assert ();
      return result;
    }

  return result;

}

/* returns the type and the name.
 */
static int
parse_general_name (ASN1_TYPE src, const char *src_name,
		    int seq, void *name, size_t * name_size)
{
  int len;
  char nptr[MAX_NAME_SIZE];
  int result;
  opaque choice_type[128];
  gnutls_x509_subject_alt_name_t type;

  seq++;			/* 0->1, 1->2 etc */

  if ( src_name[0] != 0)
    snprintf( nptr, sizeof(nptr), "%s.?%u", src_name, seq);
  else
    snprintf( nptr, sizeof(nptr), "?%u", seq);
  
  len = sizeof (choice_type);
  result = asn1_read_value (src, nptr, choice_type, &len);

  if (result == ASN1_VALUE_NOT_FOUND || result == ASN1_ELEMENT_NOT_FOUND)
    {
      return GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
    }

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }


  type = _gnutls_x509_san_find_type (choice_type);
  if (type == (gnutls_x509_subject_alt_name_t) - 1)
    {
      gnutls_assert ();
      return GNUTLS_E_X509_UNKNOWN_SAN;
    }

  _gnutls_str_cat (nptr, sizeof (nptr), ".");
  _gnutls_str_cat (nptr, sizeof (nptr), choice_type);

  len = *name_size;
  result = asn1_read_value (src, nptr, name, &len);
  *name_size = len;

  if (result == ASN1_MEM_ERROR)
    return GNUTLS_E_SHORT_MEMORY_BUFFER;

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  return type;
}

/**
  * gnutls_x509_crt_get_subject_alt_name - This function returns the certificate's alternative name, if any
  * @cert: should contain a gnutls_x509_crt_t structure
  * @seq: specifies the sequence number of the alt name (0 for the first one, 1 for the second etc.)
  * @ret: is the place where the alternative name will be copied to
  * @ret_size: holds the size of ret.
  * @critical: will be non zero if the extension is marked as critical (may be null)
  *
  * This function will return the alternative names, contained in the
  * given certificate.
  * 
  * This is specified in X509v3 Certificate Extensions. 
  * GNUTLS will return the Alternative name (2.5.29.17), or a negative
  * error code.
  *
  * Returns GNUTLS_E_SHORT_MEMORY_BUFFER if &ret_size is not enough to hold the alternative 
  * name. In that case &ret_size will be updated. If everything was ok the type of alternative 
  * name is returned. The type is one of the enumerated gnutls_x509_subject_alt_name_t.
  *
  * If the certificate does not have an Alternative name with the specified 
  * sequence number then returns GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
  *
  **/
int
gnutls_x509_crt_get_subject_alt_name (gnutls_x509_crt_t cert,
				      unsigned int seq, void *ret,
				      size_t * ret_size,
				      unsigned int *critical)
{
  int result;
  gnutls_datum_t dnsname;
  ASN1_TYPE c2 = ASN1_TYPE_EMPTY;
  gnutls_x509_subject_alt_name_t type;

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  if (ret)
    memset (ret, 0, *ret_size);
  else
    *ret_size = 0;

  if ((result =
       _gnutls_x509_crt_get_extension (cert, "2.5.29.17", 0, &dnsname,
				       critical)) < 0)
    {
      return result;
    }

  if (dnsname.size == 0 || dnsname.data == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
    }

  result = asn1_create_element
    (_gnutls_get_pkix (), "PKIX1.SubjectAltName", &c2);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      _gnutls_free_datum (&dnsname);
      return _gnutls_asn2err (result);
    }

  result = asn1_der_decoding (&c2, dnsname.data, dnsname.size, NULL);
  _gnutls_free_datum (&dnsname);

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      asn1_delete_structure (&c2);
      return _gnutls_asn2err (result);
    }

  result = parse_general_name (c2, "", seq, ret, ret_size);

  asn1_delete_structure (&c2);

  if (result < 0)
    {
      return result;
    }

  type = result;

  return type;
}

/**
  * gnutls_x509_crt_get_ca_status - This function returns the certificate CA status
  * @cert: should contain a gnutls_x509_crt_t structure
  * @critical: will be non zero if the extension is marked as critical
  *
  * This function will return certificates CA status, by reading the
  * basicConstraints X.509 extension (2.5.29.19). If the certificate is a CA a positive
  * value will be returned, or zero if the certificate does not have
  * CA flag set. 
  *
  * A negative value may be returned in case of parsing error.
  * If the certificate does not contain the basicConstraints extension
  * GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE will be returned.
  *
  **/
int
gnutls_x509_crt_get_ca_status (gnutls_x509_crt_t cert, unsigned int *critical)
{
  int result;
  gnutls_datum_t basicConstraints;
  int ca;

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  if ((result =
       _gnutls_x509_crt_get_extension (cert, "2.5.29.19", 0,
				       &basicConstraints, critical)) < 0)
    {
      return result;
    }

  if (basicConstraints.size == 0 || basicConstraints.data == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
    }

  result =
    _gnutls_x509_ext_extract_basicConstraints (&ca,
					       basicConstraints.data,
					       basicConstraints.size);
  _gnutls_free_datum (&basicConstraints);

  if (result < 0)
    {
      gnutls_assert ();
      return result;
    }

  return ca;
}

/**
  * gnutls_x509_crt_get_key_usage - This function returns the certificate's key usage
  * @cert: should contain a gnutls_x509_crt_t structure
  * @key_usage: where the key usage bits will be stored
  * @critical: will be non zero if the extension is marked as critical
  *
  * This function will return certificate's key usage, by reading the 
  * keyUsage X.509 extension (2.5.29.15). The key usage value will ORed values of the:
  * GNUTLS_KEY_DIGITAL_SIGNATURE, GNUTLS_KEY_NON_REPUDIATION,
  * GNUTLS_KEY_KEY_ENCIPHERMENT, GNUTLS_KEY_DATA_ENCIPHERMENT,
  * GNUTLS_KEY_KEY_AGREEMENT, GNUTLS_KEY_KEY_CERT_SIGN,
  * GNUTLS_KEY_CRL_SIGN, GNUTLS_KEY_ENCIPHER_ONLY,
  * GNUTLS_KEY_DECIPHER_ONLY.
  *
  * A negative value may be returned in case of parsing error.
  * If the certificate does not contain the keyUsage extension
  * GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE will be returned.
  *
  **/
int
gnutls_x509_crt_get_key_usage (gnutls_x509_crt_t cert,
			       unsigned int *key_usage,
			       unsigned int *critical)
{
  int result;
  gnutls_datum_t keyUsage;
  uint16_t _usage;

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  if ((result =
       _gnutls_x509_crt_get_extension (cert, "2.5.29.15", 0, &keyUsage,
				       critical)) < 0)
    {
      return result;
    }

  if (keyUsage.size == 0 || keyUsage.data == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
    }

  result = _gnutls_x509_ext_extract_keyUsage (&_usage, keyUsage.data,
					      keyUsage.size);
  _gnutls_free_datum (&keyUsage);

  *key_usage = _usage;

  if (result < 0)
    {
      gnutls_assert ();
      return result;
    }

  return 0;
}

/**
  * gnutls_x509_crt_get_extension_by_oid - This function returns the specified extension
  * @cert: should contain a gnutls_x509_crt_t structure
  * @oid: holds an Object Identified in null terminated string
  * @indx: In case multiple same OIDs exist in the extensions, this specifies which to send. Use zero to get the first one.
  * @buf: a pointer to a structure to hold the name (may be null)
  * @sizeof_buf: initially holds the size of @buf
  * @critical: will be non zero if the extension is marked as critical
  *
  * This function will return the extension specified by the OID in the certificate.
  * The extensions will be returned as binary data DER encoded, in the provided
  * buffer.
  *
  * A negative value may be returned in case of parsing error.
  * If the certificate does not contain the specified extension
  * GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE will be returned.
  *
  **/
int
gnutls_x509_crt_get_extension_by_oid (gnutls_x509_crt_t cert,
				      const char *oid, int indx,
				      void *buf, size_t * sizeof_buf,
				      unsigned int *critical)
{
  int result;
  gnutls_datum_t output;

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  if ((result =
       _gnutls_x509_crt_get_extension (cert, oid, indx, &output,
				       critical)) < 0)
    {
      gnutls_assert ();
      return result;
    }

  if (output.size == 0 || output.data == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
    }

  if (output.size > (unsigned int) *sizeof_buf)
    {
      *sizeof_buf = output.size;
      _gnutls_free_datum (&output);
      return GNUTLS_E_SHORT_MEMORY_BUFFER;
    }

  *sizeof_buf = output.size;

  if (buf)
    memcpy (buf, output.data, output.size);

  _gnutls_free_datum (&output);

  return 0;

}

/**
  * gnutls_x509_crt_get_extension_oid - This function returns the specified extension OID
  * @cert: should contain a gnutls_x509_crt_t structure
  * @indx: Specifies which extension OID to send. Use zero to get the first one.
  * @oid: a pointer to a structure to hold the OID (may be null)
  * @sizeof_oid: initially holds the size of @oid
  *
  * This function will return the requested extension OID in the certificate.
  * The extension OID will be stored as a string in the provided buffer.
  *
  * A negative value may be returned in case of parsing error.
  * If your have reached the last extension available 
  * GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE will be returned.
  *
  **/
int
gnutls_x509_crt_get_extension_oid (gnutls_x509_crt_t cert, int indx,
				   void *oid, size_t * sizeof_oid)
{
  int result;

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  result = _gnutls_x509_crt_get_extension_oid (cert, indx, oid, sizeof_oid);
  if (result < 0)
    {
      return result;
    }

  return 0;

}


static int
_gnutls_x509_crt_get_raw_dn2 (gnutls_x509_crt_t cert,
			      const char *whom, gnutls_datum_t * start)
{
  ASN1_TYPE c2 = ASN1_TYPE_EMPTY;
  int result, len1;
  int start1, end1;
  gnutls_datum_t signed_data = { NULL, 0 };

  /* get the issuer of 'cert'
   */
  if ((result =
       asn1_create_element (_gnutls_get_pkix (), "PKIX1.TBSCertificate",
			    &c2)) != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  result =
    _gnutls_x509_get_signed_data (cert->cert, "tbsCertificate", &signed_data);
  if (result < 0)
    {
      gnutls_assert ();
      goto cleanup;
    }

  result = asn1_der_decoding (&c2, signed_data.data, signed_data.size, NULL);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      asn1_delete_structure (&c2);
      result = _gnutls_asn2err (result);
      goto cleanup;
    }

  result =
    asn1_der_decoding_startEnd (c2, signed_data.data, signed_data.size,
				whom, &start1, &end1);

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto cleanup;
    }

  len1 = end1 - start1 + 1;

  _gnutls_set_datum (start, &signed_data.data[start1], len1);

  result = 0;

cleanup:
  asn1_delete_structure (&c2);
  _gnutls_free_datum (&signed_data);
  return result;
}

/*-
  * _gnutls_x509_crt_get_raw_issuer_dn - This function returns the issuer's DN DER encoded
  * @cert: should contain a gnutls_x509_crt_t structure
  * @start: will hold the starting point of the DN
  *
  * This function will return a pointer to the DER encoded DN structure and
  * the length.
  *
  * Returns 0 on success or a negative value on error.
  *
  -*/
int
_gnutls_x509_crt_get_raw_issuer_dn (gnutls_x509_crt_t cert,
				    gnutls_datum_t * start)
{
  return _gnutls_x509_crt_get_raw_dn2 (cert, "issuer", start);
}

/*-
  * _gnutls_x509_crt_get_raw_dn - This function returns the subject's DN DER encoded
  * @cert: should contain a gnutls_x509_crt_t structure
  * @start: will hold the starting point of the DN
  *
  * This function will return a pointer to the DER encoded DN structure and
  * the length.
  *
  * Returns 0 on success, or a negative value on error.
  *
  -*/
int
_gnutls_x509_crt_get_raw_dn (gnutls_x509_crt_t cert, gnutls_datum_t * start)
{
  return _gnutls_x509_crt_get_raw_dn2 (cert, "subject", start);
}


/**
  * gnutls_x509_crt_get_fingerprint - This function returns the Certificate's fingerprint
  * @cert: should contain a gnutls_x509_crt_t structure
  * @algo: is a digest algorithm
  * @buf: a pointer to a structure to hold the fingerprint (may be null)
  * @sizeof_buf: initially holds the size of @buf
  *
  * This function will calculate and copy the certificate's fingerprint
  * in the provided buffer.
  *
  * If the buffer is null then only the size will be filled.
  *
  * Returns GNUTLS_E_SHORT_MEMORY_BUFFER if the provided buffer is not
  * long enough, and in that case the *sizeof_buf will be updated with
  * the required size.  On success 0 is returned.
  *
  **/
int
gnutls_x509_crt_get_fingerprint (gnutls_x509_crt_t cert,
				 gnutls_digest_algorithm_t algo,
				 void *buf, size_t * sizeof_buf)
{
  opaque *cert_buf;
  int cert_buf_size;
  int result;
  gnutls_datum_t tmp;

  if (sizeof_buf == 0 || cert == NULL)
    {
      return GNUTLS_E_INVALID_REQUEST;
    }

  cert_buf_size = 0;
  asn1_der_coding (cert->cert, "", NULL, &cert_buf_size, NULL);

  cert_buf = gnutls_alloca (cert_buf_size);
  if (cert_buf == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_MEMORY_ERROR;
    }

  result = asn1_der_coding (cert->cert, "", cert_buf, &cert_buf_size, NULL);

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      gnutls_afree (cert_buf);
      return _gnutls_asn2err (result);
    }

  tmp.data = cert_buf;
  tmp.size = cert_buf_size;

  result = gnutls_fingerprint (algo, &tmp, buf, sizeof_buf);
  gnutls_afree (cert_buf);

  return result;
}

/**
  * gnutls_x509_crt_export - This function will export the certificate
  * @cert: Holds the certificate
  * @format: the format of output params. One of PEM or DER.
  * @output_data: will contain a certificate PEM or DER encoded
  * @output_data_size: holds the size of output_data (and will be
  *   replaced by the actual size of parameters)
  *
  * This function will export the certificate to DER or PEM format.
  *
  * If the buffer provided is not long enough to hold the output, then
  * *output_data_size is updated and GNUTLS_E_SHORT_MEMORY_BUFFER will
  * be returned.
  *
  * If the structure is PEM encoded, it will have a header
  * of "BEGIN CERTIFICATE".
  *
  * Return value: In case of failure a negative value will be
  *   returned, and 0 on success.
  *
  **/
int
gnutls_x509_crt_export (gnutls_x509_crt_t cert,
			gnutls_x509_crt_fmt_t format, void *output_data,
			size_t * output_data_size)
{
  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  return _gnutls_x509_export_int (cert->cert, format, "CERTIFICATE",
				  *output_data_size, output_data,
				  output_data_size);
}


/**
  * gnutls_x509_crt_get_key_id - Return unique ID of public key's parameters
  * @crt: Holds the certificate
  * @flags: should be 0 for now
  * @output_data: will contain the key ID
  * @output_data_size: holds the size of output_data (and will be
  *   replaced by the actual size of parameters)
  *
  * This function will return a unique ID the depends on the public key
  * parameters. This ID can be used in checking whether a certificate
  * corresponds to the given private key.
  *
  * If the buffer provided is not long enough to hold the output, then
  * *output_data_size is updated and GNUTLS_E_SHORT_MEMORY_BUFFER will
  * be returned.  The output will normally be a SHA-1 hash output,
  * which is 20 bytes.
  *
  * Return value: In case of failure a negative value will be
  *   returned, and 0 on success.
  *
  **/
int
gnutls_x509_crt_get_key_id (gnutls_x509_crt_t crt, unsigned int flags,
			    unsigned char *output_data,
			    size_t * output_data_size)
{
  mpi_t params[MAX_PUBLIC_PARAMS_SIZE];
  int params_size = MAX_PUBLIC_PARAMS_SIZE;
  int i, pk, result = 0;
  gnutls_datum_t der = { NULL, 0 };
  GNUTLS_HASH_HANDLE hd;

  if (crt == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  if (*output_data_size < 20)
    {
      gnutls_assert ();
      *output_data_size = 20;
      return GNUTLS_E_SHORT_MEMORY_BUFFER;
    }

  pk = gnutls_x509_crt_get_pk_algorithm (crt, NULL);

  if (pk < 0)
    {
      gnutls_assert ();
      return pk;
    }

  result = _gnutls_x509_crt_get_mpis (crt, params, &params_size);

  if (result < 0)
    {
      gnutls_assert ();
      return result;
    }

  if (pk == GNUTLS_PK_RSA)
    {
      result = _gnutls_x509_write_rsa_params (params, params_size, &der);
      if (result < 0)
	{
	  gnutls_assert ();
	  goto cleanup;
	}
    }
  else if (pk == GNUTLS_PK_DSA)
    {
      result = _gnutls_x509_write_dsa_public_key (params, params_size, &der);
      if (result < 0)
	{
	  gnutls_assert ();
	  goto cleanup;
	}
    }
  else
    return GNUTLS_E_INTERNAL_ERROR;

  hd = _gnutls_hash_init (GNUTLS_MAC_SHA1);
  if (hd == GNUTLS_HASH_FAILED)
    {
      gnutls_assert ();
      result = GNUTLS_E_INTERNAL_ERROR;
      goto cleanup;
    }

  _gnutls_hash (hd, der.data, der.size);

  _gnutls_hash_deinit (hd, output_data);
  *output_data_size = 20;

  result = 0;

cleanup:

  _gnutls_free_datum (&der);

  /* release all allocated MPIs
   */
  for (i = 0; i < params_size; i++)
    {
      _gnutls_mpi_release (&params[i]);
    }
  return result;
}


#ifdef ENABLE_PKI

/**
  * gnutls_x509_crt_check_revocation - This function checks if the given certificate is revoked
  * @cert: should contain a gnutls_x509_crt_t structure
  * @crl_list: should contain a list of gnutls_x509_crl_t structures
  * @crl_list_length: the length of the crl_list
  *
  * This function will return check if the given certificate is revoked.
  * It is assumed that the CRLs have been verified before.
  *
  * Returns 0 if the certificate is NOT revoked, and 1 if it is.
  * A negative value is returned on error. 
  *
  **/
int
gnutls_x509_crt_check_revocation (gnutls_x509_crt_t cert,
				  const gnutls_x509_crl_t * crl_list,
				  int crl_list_length)
{
  opaque serial[64];
  opaque cert_serial[64];
  size_t serial_size, cert_serial_size;
  int ncerts, ret, i, j;
  gnutls_datum_t dn1, dn2;

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  for (j = 0; j < crl_list_length; j++)
    {				/* do for all the crls */

      /* Step 1. check if issuer's DN match
       */
      ret = _gnutls_x509_crl_get_raw_issuer_dn (crl_list[j], &dn1);
      if (ret < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      ret = _gnutls_x509_crt_get_raw_issuer_dn (cert, &dn2);
      if (ret < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      ret = _gnutls_x509_compare_raw_dn (&dn1, &dn2);
      _gnutls_free_datum (&dn1);
      _gnutls_free_datum (&dn2);
      if (ret == 0)
	{
	  /* issuers do not match so don't even
	   * bother checking.
	   */
	  continue;
	}

      /* Step 2. Read the certificate's serial number
       */
      cert_serial_size = sizeof (cert_serial);
      ret = gnutls_x509_crt_get_serial (cert, cert_serial, &cert_serial_size);
      if (ret < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      /* Step 3. cycle through the CRL serials and compare with
       *   certificate serial we have.
       */

      ncerts = gnutls_x509_crl_get_crt_count (crl_list[j]);
      if (ncerts < 0)
	{
	  gnutls_assert ();
	  return ncerts;
	}

      for (i = 0; i < ncerts; i++)
	{
	  serial_size = sizeof (serial);
	  ret =
	    gnutls_x509_crl_get_crt_serial (crl_list[j], i, serial,
					    &serial_size, NULL);

	  if (ret < 0)
	    {
	      gnutls_assert ();
	      return ret;
	    }

	  if (serial_size == cert_serial_size)
	    {
	      if (memcmp (serial, cert_serial, serial_size) == 0)
		{
		  /* serials match */
		  return 1;	/* revoked! */
		}
	    }
	}

    }
  return 0;			/* not revoked. */
}

/**
  * gnutls_x509_crt_verify_data - This function will verify the given signed data.
  * @crt: Holds the certificate
  * @flags: should be 0 for now
  * @data: holds the data to be signed
  * @signature: contains the signature
  *
  * This function will verify the given signed data, using the parameters from the
  * certificate.
  *
  * In case of a verification failure 0 is returned, and
  * 1 on success.
  *
  **/
int
gnutls_x509_crt_verify_data (gnutls_x509_crt_t crt, unsigned int flags,
			     const gnutls_datum_t * data,
			     const gnutls_datum_t * signature)
{
  int result;

  if (crt == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  result = _gnutls_x509_verify_signature (data, signature, crt);
  if (result < 0)
    {
      gnutls_assert ();
      return 0;
    }

  return result;
}

/**
  * gnutls_x509_crt_get_crl_dist_points - This function returns the CRL distribution points
  * @cert: should contain a gnutls_x509_crt_t structure
  * @seq: specifies the sequence number of the distribution point (0 for the first one, 1 for the second etc.)
  * @ret: is the place where the distribution point will be copied to
  * @ret_size: holds the size of ret.
  * @reason_flags: Revocation reasons flags.
  * @critical: will be non zero if the extension is marked as critical (may be null)
  *
  * This function will return the CRL distribution points (2.5.29.31), contained in the
  * given certificate.
  *
  * @reason_flags should be an ORed sequence of GNUTLS_CRL_REASON_UNUSED,
  * GNUTLS_CRL_REASON_KEY_COMPROMISE, GNUTLS_CRL_REASON_CA_COMPROMISE,
  * GNUTLS_CRL_REASON_AFFILIATION_CHANGED, GNUTLS_CRL_REASON_SUPERSEEDED,
  * GNUTLS_CRL_REASON_CESSATION_OF_OPERATION, GNUTLS_CRL_REASON_CERTIFICATE_HOLD,
  * GNUTLS_CRL_REASON_PRIVILEGE_WITHDRAWN, GNUTLS_CRL_REASON_AA_COMPROMISE,
  * or zero for all possible reasons.
  * 
  * This is specified in X509v3 Certificate Extensions. GNUTLS will return the 
  * distribution point type, or a negative error code on error.
  *
  * Returns GNUTLS_E_SHORT_MEMORY_BUFFER and updates &ret_size if &ret_size is not enough to hold the distribution
  * point, or the type of the distribution point if everything was ok. The type is 
  * one of the enumerated gnutls_x509_subject_alt_name_t.
  *
  * If the certificate does not have an Alternative name with the specified 
  * sequence number then returns GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
  *
  **/
int
gnutls_x509_crt_get_crl_dist_points (gnutls_x509_crt_t cert,
				     unsigned int seq, void *ret,
				     size_t * ret_size,
				     unsigned int *reason_flags,
				     unsigned int *critical)
{
  int result;
  gnutls_datum_t dist_points = { NULL, 0 };
  ASN1_TYPE c2 = ASN1_TYPE_EMPTY;
  char name[MAX_NAME_SIZE];
  int len;
  gnutls_x509_subject_alt_name_t type;
  uint8_t reasons[2];

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  if (ret)
    memset (ret, 0, *ret_size);
  else
    *ret_size = 0;

  if (reason_flags)
    *reason_flags = 0;

  result =
    _gnutls_x509_crt_get_extension (cert, "2.5.29.31", 0, &dist_points,
				    critical);
  if (result < 0)
    {
      return result;
    }

  if (dist_points.size == 0 || dist_points.data == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
    }

  result = asn1_create_element
    (_gnutls_get_pkix (), "PKIX1.CRLDistributionPoints", &c2);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      _gnutls_free_datum (&dist_points);
      return _gnutls_asn2err (result);
    }

  result = asn1_der_decoding (&c2, dist_points.data, dist_points.size, NULL);
  _gnutls_free_datum (&dist_points);

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      asn1_delete_structure (&c2);
      return _gnutls_asn2err (result);
    }

  /* Return the different names from the first CRLDistr. point.
   * The whole thing is a mess.
   */
  _gnutls_str_cpy (name, sizeof (name), "?1.distributionPoint.fullName");

  result = parse_general_name (c2, name, seq, ret, ret_size);
  if (result < 0)
    {
      asn1_delete_structure (&c2);
      return result;
    }

  type = result;


  /* Read the CRL reasons.
   */
  if (reason_flags)
    {
      _gnutls_str_cpy( name, sizeof(name), "?1.reasons");

      reasons[0] = reasons[1] = 0;

      len = sizeof (reasons);
      result = asn1_read_value (c2, name, reasons, &len);

      if (result != ASN1_VALUE_NOT_FOUND && result != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  asn1_delete_structure (&c2);
	  return _gnutls_asn2err (result);
	}

      *reason_flags = reasons[0] | (reasons[1] << 8);
    }

  return type;
}

/**
  * gnutls_x509_crt_get_key_purpose_oid - This function returns the Certificate's key purpose OIDs
  * @cert: should contain a gnutls_x509_crt_t structure
  * @indx: This specifies which OID to return. Use zero to get the first one.
  * @oid: a pointer to a buffer to hold the OID (may be null)
  * @sizeof_oid: initially holds the size of @oid
  *
  * This function will extract the key purpose OIDs of the Certificate
  * specified by the given index. These are stored in the Extended Key
  * Usage extension (2.5.29.37) See the GNUTLS_KP_* definitions for
  * human readable names.
  *
  * If @oid is null then only the size will be filled.
  *
  * Returns GNUTLS_E_SHORT_MEMORY_BUFFER if the provided buffer is not
  * long enough, and in that case the *sizeof_oid will be updated with
  * the required size.  On success 0 is returned.
  *
  **/
int
gnutls_x509_crt_get_key_purpose_oid (gnutls_x509_crt_t cert,
				     int indx, void *oid, size_t * sizeof_oid,
				     unsigned int *critical)
{
  char tmpstr[MAX_NAME_SIZE];
  int result, len;
  gnutls_datum_t id;
  ASN1_TYPE c2 = ASN1_TYPE_EMPTY;

  if (cert == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  if (oid)
    memset (oid, 0, *sizeof_oid);
  else
    *sizeof_oid = 0;

  if ((result =
       _gnutls_x509_crt_get_extension (cert, "2.5.29.37", 0, &id,
				       critical)) < 0)
    {
      return result;
    }

  if (id.size == 0 || id.data == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
    }

  result = asn1_create_element
    (_gnutls_get_pkix (), "PKIX1.ExtKeyUsageSyntax", &c2);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      _gnutls_free_datum (&id);
      return _gnutls_asn2err (result);
    }

  result = asn1_der_decoding (&c2, id.data, id.size, NULL);
  _gnutls_free_datum (&id);

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      asn1_delete_structure (&c2);
      return _gnutls_asn2err (result);
    }

  indx++;
  /* create a string like "?1"
   */
  snprintf( tmpstr, sizeof(tmpstr), "?%u", indx);

  len = *sizeof_oid;
  result = asn1_read_value (c2, tmpstr, oid, &len);

  *sizeof_oid = len;
  asn1_delete_structure (&c2);

  if (result == ASN1_VALUE_NOT_FOUND || result == ASN1_ELEMENT_NOT_FOUND)
    {
      return GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE;
    }

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  return 0;

}

/**
  * gnutls_x509_crt_get_pk_rsa_raw - This function will export the RSA public key
  * @crt: Holds the certificate
  * @m: will hold the modulus
  * @e: will hold the public exponent
  *
  * This function will export the RSA private key's parameters found in the given
  * structure. The new parameters will be allocated using
  * gnutls_malloc() and will be stored in the appropriate datum.
  * 
  **/
int
gnutls_x509_crt_get_pk_rsa_raw (gnutls_x509_crt_t crt,
				gnutls_datum_t * m, gnutls_datum_t * e)
{
  int ret;
  mpi_t params[MAX_PUBLIC_PARAMS_SIZE];
  int params_size = MAX_PUBLIC_PARAMS_SIZE;
  int i;

  if (crt == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  ret = gnutls_x509_crt_get_pk_algorithm (crt, NULL);
  if (ret != GNUTLS_PK_RSA)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  ret = _gnutls_x509_crt_get_mpis (crt, params, &params_size);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  ret = _gnutls_mpi_dprint (m, params[0]);
  if (ret < 0)
    {
      gnutls_assert ();
      goto cleanup;
    }

  ret = _gnutls_mpi_dprint (e, params[1]);
  if (ret < 0)
    {
      gnutls_assert ();
      _gnutls_free_datum (m);
      goto cleanup;
    }

  ret = 0;

cleanup:
  for (i = 0; i < params_size; i++)
    {
      _gnutls_mpi_release (&params[i]);
    }
  return ret;
}

/**
  * gnutls_x509_crt_get_pk_dsa_raw - This function will export the DSA private key
  * @crt: Holds the certificate
  * @p: will hold the p
  * @q: will hold the q
  * @g: will hold the g
  * @y: will hold the y
  *
  * This function will export the DSA private key's parameters found in the given
  * certificate. The new parameters will be allocated using
  * gnutls_malloc() and will be stored in the appropriate datum.
  * 
  **/
int
gnutls_x509_crt_get_pk_dsa_raw (gnutls_x509_crt_t crt,
				gnutls_datum_t * p, gnutls_datum_t * q,
				gnutls_datum_t * g, gnutls_datum_t * y)
{
  int ret;
  mpi_t params[MAX_PUBLIC_PARAMS_SIZE];
  int params_size = MAX_PUBLIC_PARAMS_SIZE;
  int i;

  if (crt == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  ret = gnutls_x509_crt_get_pk_algorithm (crt, NULL);
  if (ret != GNUTLS_PK_DSA)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  ret = _gnutls_x509_crt_get_mpis (crt, params, &params_size);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }


  /* P */
  ret = _gnutls_mpi_dprint (p, params[0]);
  if (ret < 0)
    {
      gnutls_assert ();
      goto cleanup;
    }

  /* Q */
  ret = _gnutls_mpi_dprint (q, params[1]);
  if (ret < 0)
    {
      gnutls_assert ();
      _gnutls_free_datum (p);
      goto cleanup;
    }


  /* G */
  ret = _gnutls_mpi_dprint (g, params[2]);
  if (ret < 0)
    {
      gnutls_assert ();
      _gnutls_free_datum (p);
      _gnutls_free_datum (q);
      goto cleanup;
    }


  /* Y */
  ret = _gnutls_mpi_dprint (y, params[3]);
  if (ret < 0)
    {
      gnutls_assert ();
      _gnutls_free_datum (p);
      _gnutls_free_datum (g);
      _gnutls_free_datum (q);
      goto cleanup;
    }

  ret = 0;

cleanup:
  for (i = 0; i < params_size; i++)
    {
      _gnutls_mpi_release (&params[i]);
    }
  return ret;

}

#endif

#define CLEAR_CERTS \
    for(j=0;j<count;j++) gnutls_x509_crt_deinit( certs[j])
/**
  * gnutls_x509_crt_list_import - This function will import a PEM encoded certificate list
  * @certs: The structures to store the parsed certificate. Must not be initialized.
  * @cert_max: Initially must hold the maximum number of certs. It will be updated with the number of certs available.
  * @data: The PEM encoded certificate.
  * @format: One of DER or PEM.
  * @flags: must be zero or an OR'd sequence of gnutls_certificate_import_flags.
  *
  * This function will convert the given PEM encoded certificate list
  * to the native gnutls_x509_crt_t format. The output will be stored in @certs.
  * They will be automatically initialized.
  *
  * If the Certificate is PEM encoded it should have a header of "X509 CERTIFICATE", or
  * "CERTIFICATE".
  *
  * Returns the number of certificates read or a negative error value.
  *
  **/
int
gnutls_x509_crt_list_import (gnutls_x509_crt_t * certs,
			     unsigned int *cert_max,
			     const gnutls_datum_t * data,
			     gnutls_x509_crt_fmt_t format, unsigned int flags)
{
  int size;
  const char *ptr;
  gnutls_datum_t tmp;
  int ret, nocopy = 0;
  unsigned int count = 0, j;

  if (format == GNUTLS_X509_FMT_DER)
    {
      if (*cert_max < 1)
	{
	  *cert_max = 1;
	  return GNUTLS_E_SHORT_MEMORY_BUFFER;
	}

      count = 1;		/* import only the first one */

      ret = gnutls_x509_crt_init (&certs[0]);
      if (ret < 0)
	{
	  gnutls_assert ();
	  goto error;
	}

      ret = gnutls_x509_crt_import (certs[0], data, format);
      if (ret < 0)
	{
	  gnutls_assert ();
	  goto error;
	}

      *cert_max = 1;
      return 1;
    }

  /* move to the certificate
   */
  ptr = memmem (data->data, data->size,
		PEM_CERT_SEP, sizeof (PEM_CERT_SEP) - 1);
  if (ptr == NULL)
    ptr = memmem (data->data, data->size,
		  PEM_CERT_SEP2, sizeof (PEM_CERT_SEP2) - 1);

  if (ptr == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_BASE64_DECODING_ERROR;
    }
  size = data->size - (ptr - (char *) data->data);

  count = 0;

  do
    {
      if (count >= *cert_max)
	{
	  if (!(flags & GNUTLS_X509_CRT_LIST_IMPORT_FAIL_IF_EXCEED))
	    break;
	  else
	    nocopy = 1;
	}

      if (!nocopy)
	{
	  ret = gnutls_x509_crt_init (&certs[count]);
	  if (ret < 0)
	    {
	      gnutls_assert ();
	      goto error;
	    }

	  tmp.data = (void *) ptr;
	  tmp.size = size;

	  ret =
	    gnutls_x509_crt_import (certs[count], &tmp, GNUTLS_X509_FMT_PEM);
	  if (ret < 0)
	    {
	      gnutls_assert ();
	      goto error;
	    }
	}

      /* now we move ptr after the pem header 
       */
      ptr++;
      /* find the next certificate (if any)
       */
      size = data->size - (ptr - (char *) data->data);

      if (size > 0)
	{
	  char *ptr2;

	  ptr2 = memmem (ptr, size, PEM_CERT_SEP, sizeof (PEM_CERT_SEP) - 1);
	  if (ptr2 == NULL)
	    ptr2 = memmem (ptr, size, PEM_CERT_SEP2,
			   sizeof (PEM_CERT_SEP2) - 1);

	  ptr = ptr2;
	}
      else
	ptr = NULL;

      count++;
    }
  while (ptr != NULL);

  *cert_max = count;

  if (nocopy == 0)
    return count;
  else
    return GNUTLS_E_SHORT_MEMORY_BUFFER;

error:
  CLEAR_CERTS;
  return ret;
}
