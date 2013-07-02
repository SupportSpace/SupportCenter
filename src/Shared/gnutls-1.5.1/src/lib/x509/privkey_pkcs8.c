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

#include <gnutls_int.h>

#ifdef ENABLE_PKI

#include <gnutls_datum.h>
#include <gnutls_global.h>
#include <gnutls_errors.h>
#include <gnutls_rsa_export.h>
#include <common.h>
#include <gnutls_x509.h>
#include <x509_b64.h>
#include <x509.h>
#include <dn.h>
#include <pkcs12.h>
#include <privkey.h>
#include <extensions.h>
#include <mpi.h>
#include <gnutls_algorithms.h>
#include <gnutls_num.h>
#include <gc.h>


#define PBES2_OID "1.2.840.113549.1.5.13"
#define PBKDF2_OID "1.2.840.113549.1.5.12"
#define DES_EDE3_CBC_OID "1.2.840.113549.3.7"
#define DES_CBC_OID "1.3.14.3.2.7"

/* oid_pbeWithSHAAnd3_KeyTripleDES_CBC */
#define PKCS12_PBE_3DES_SHA1_OID "1.2.840.113549.1.12.1.3"
#define PKCS12_PBE_ARCFOUR_SHA1_OID "1.2.840.113549.1.12.1.1"
#define PKCS12_PBE_RC2_40_SHA1_OID "1.2.840.113549.1.12.1.6"

struct pbkdf2_params
{
  opaque salt[32];
  int salt_size;
  unsigned int iter_count;
  unsigned int key_size;
};

struct pbe_enc_params
{
  gnutls_cipher_algorithm_t cipher;
  opaque iv[8];
  int iv_size;
};

static int generate_key (schema_id schema, const char *password,
			 struct pbkdf2_params *kdf_params,
			 struct pbe_enc_params *enc_params,
			 gnutls_datum_t * key);
static int read_pbkdf2_params (ASN1_TYPE pbes2_asn,
			       const gnutls_datum_t * der,
			       struct pbkdf2_params *params);
static int read_pbe_enc_params (ASN1_TYPE pbes2_asn,
				const gnutls_datum_t * der,
				struct pbe_enc_params *params);
static int decrypt_data (schema_id, ASN1_TYPE pkcs8_asn, const char *root,
			 const char *password,
			 const struct pbkdf2_params *kdf_params,
			 const struct pbe_enc_params *enc_params,
			 gnutls_datum_t * decrypted_data);
static int decode_private_key_info (const gnutls_datum_t * der,
				    gnutls_x509_privkey_t pkey,
				    ASN1_TYPE * out);
static int write_schema_params (schema_id schema, ASN1_TYPE pkcs8_asn,
				const char *where,
				const struct pbkdf2_params *kdf_params,
				const struct pbe_enc_params *enc_params);
static int encrypt_data (const gnutls_datum_t * plain,
			 const struct pbe_enc_params *enc_params,
			 gnutls_datum_t * key, gnutls_datum_t * encrypted);

static int read_pkcs12_kdf_params (ASN1_TYPE pbes2_asn,
				   struct pbkdf2_params *params);
static int write_pkcs12_kdf_params (ASN1_TYPE pbes2_asn,
				    const struct pbkdf2_params *params);

#define PEM_PKCS8 "ENCRYPTED PRIVATE KEY"
#define PEM_UNENCRYPTED_PKCS8 "PRIVATE KEY"

/* Returns a negative error code if the encryption schema in
 * the OID is not supported. The schema ID is returned.
 */
inline static int
check_schema (const char *oid)
{

  if (strcmp (oid, PBES2_OID) == 0)
    return PBES2;

  if (strcmp (oid, PKCS12_PBE_3DES_SHA1_OID) == 0)
    return PKCS12_3DES_SHA1;

  if (strcmp (oid, PKCS12_PBE_ARCFOUR_SHA1_OID) == 0)
    return PKCS12_ARCFOUR_SHA1;

  if (strcmp (oid, PKCS12_PBE_RC2_40_SHA1_OID) == 0)
    return PKCS12_RC2_40_SHA1;

  _gnutls_x509_log ("PKCS encryption schema OID '%s' is unsupported.\n", oid);

  return GNUTLS_E_UNKNOWN_CIPHER_TYPE;
}

/* 
 * Encodes a PKCS #1 private key to a PKCS #8 private key
 * info. The output will be allocated and stored into der. Also
 * the ASN1_TYPE of private key info will be returned.
 */
static int
encode_to_private_key_info (gnutls_x509_privkey_t pkey,
			    gnutls_datum_t * der, ASN1_TYPE * pkey_info)
{
  int result, len;
  size_t size;
  opaque *data = NULL;
  opaque null = 0;

  if (pkey->pk_algorithm != GNUTLS_PK_RSA)
    {
      gnutls_assert ();
      return GNUTLS_E_UNIMPLEMENTED_FEATURE;
    }

  if ((result =
       asn1_create_element (_gnutls_get_pkix (),
			    "PKIX1.pkcs-8-PrivateKeyInfo",
			    pkey_info)) != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* Write the version.
   */
  result = asn1_write_value (*pkey_info, "version", &null, 1);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* write the privateKeyAlgorithm
   * fields. (OID+NULL data)
   */
  result =
    asn1_write_value (*pkey_info, "privateKeyAlgorithm.algorithm",
		      PK_PKIX1_RSA_OID, 1);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  result =
    asn1_write_value (*pkey_info, "privateKeyAlgorithm.parameters", NULL, 0);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* Write the raw private key
   */
  size = 0;
  result =
    gnutls_x509_privkey_export (pkey, GNUTLS_X509_FMT_DER, NULL, &size);
  if (result != GNUTLS_E_SHORT_MEMORY_BUFFER)
    {
      gnutls_assert ();
      goto error;
    }

  data = gnutls_alloca (size);
  if (data == NULL)
    {
      gnutls_assert ();
      result = GNUTLS_E_MEMORY_ERROR;
      goto error;
    }


  result =
    gnutls_x509_privkey_export (pkey, GNUTLS_X509_FMT_DER, data, &size);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  result = asn1_write_value (*pkey_info, "privateKey", data, size);

  gnutls_afree (data);
  data = NULL;

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* Append an empty Attributes field.
   */
  result = asn1_write_value (*pkey_info, "attributes", NULL, 0);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* DER Encode the generated private key info.
   */
  len = 0;
  result = asn1_der_coding (*pkey_info, "", NULL, &len, NULL);
  if (result != ASN1_MEM_ERROR)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* allocate data for the der
   */
  der->size = len;
  der->data = gnutls_malloc (len);
  if (der->data == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_MEMORY_ERROR;
    }

  result = asn1_der_coding (*pkey_info, "", der->data, &len, NULL);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  return 0;

error:
  asn1_delete_structure (pkey_info);
  if (data != NULL)
    {
      gnutls_afree (data);
    }
  return result;

}

/* Converts a PKCS #8 private key info to
 * a PKCS #8 EncryptedPrivateKeyInfo.
 */
static int
encode_to_pkcs8_key (schema_id schema, const gnutls_datum_t * der_key,
		     const char *password, ASN1_TYPE * out)
{
  int result;
  gnutls_datum_t key = { NULL, 0 };
  gnutls_datum_t tmp = { NULL, 0 };
  ASN1_TYPE pkcs8_asn = ASN1_TYPE_EMPTY;
  struct pbkdf2_params kdf_params;
  struct pbe_enc_params enc_params;


  if ((result =
       asn1_create_element (_gnutls_get_pkix (),
			    "PKIX1.pkcs-8-EncryptedPrivateKeyInfo",
			    &pkcs8_asn)) != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* Write the encryption schema OID
   */
  switch (schema)
    {
    case PBES2:
      result =
	asn1_write_value (pkcs8_asn, "encryptionAlgorithm.algorithm",
			  PBES2_OID, 1);
      break;
    case PKCS12_3DES_SHA1:
      result =
	asn1_write_value (pkcs8_asn, "encryptionAlgorithm.algorithm",
			  PKCS12_PBE_3DES_SHA1_OID, 1);
      break;
    case PKCS12_ARCFOUR_SHA1:
      result =
	asn1_write_value (pkcs8_asn, "encryptionAlgorithm.algorithm",
			  PKCS12_PBE_ARCFOUR_SHA1_OID, 1);
      break;
    case PKCS12_RC2_40_SHA1:
      result =
	asn1_write_value (pkcs8_asn, "encryptionAlgorithm.algorithm",
			  PKCS12_PBE_RC2_40_SHA1_OID, 1);
      break;

    }

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* Generate a symmetric key.
   */

  result = generate_key (schema, password, &kdf_params, &enc_params, &key);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  result =
    write_schema_params (schema, pkcs8_asn,
			 "encryptionAlgorithm.parameters", &kdf_params,
			 &enc_params);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  /* Parameters have been encoded. Now
   * encrypt the Data.
   */
  result = encrypt_data (der_key, &enc_params, &key, &tmp);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  /* write the encrypted data.
   */
  result = asn1_write_value (pkcs8_asn, "encryptedData", tmp.data, tmp.size);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  _gnutls_free_datum (&tmp);
  _gnutls_free_datum (&key);

  *out = pkcs8_asn;

  return 0;

error:
  _gnutls_free_datum (&key);
  _gnutls_free_datum (&tmp);
  asn1_delete_structure (&pkcs8_asn);
  return result;
}


/**
  * gnutls_x509_privkey_export_pkcs8 - This function will export the private key to PKCS8 format
  * @key: Holds the key
  * @format: the format of output params. One of PEM or DER.
  * @password: the password that will be used to encrypt the key. 
  * @flags: an ORed sequence of gnutls_pkcs_encrypt_flags_t
  * @output_data: will contain a private key PEM or DER encoded
  * @output_data_size: holds the size of output_data (and will be
  *   replaced by the actual size of parameters)
  *
  * This function will export the private key to a PKCS8 structure.
  * Currently only RSA keys can be exported since there is no documented
  * standard for other keys. If the flags do not
  * specify the encryption cipher, then the default 3DES (PBES2) will
  * be used.
  *
  * The @password can be either ASCII or UTF-8 in the default PBES2
  * encryption schemas, or ASCII for the PKCS12 schemas.
  *
  * If the buffer provided is not long enough to hold the output, then
  * *output_data_size is updated and GNUTLS_E_SHORT_MEMORY_BUFFER will
  * be returned.
  *
  * If the structure is PEM encoded, it will have a header
  * of "BEGIN ENCRYPTED PRIVATE KEY" or "BEGIN PRIVATE KEY" if
  * encryption is not used.
  *
  * Return value: In case of failure a negative value will be
  *   returned, and 0 on success.
  *
  **/
int
gnutls_x509_privkey_export_pkcs8 (gnutls_x509_privkey_t key,
				  gnutls_x509_crt_fmt_t format,
				  const char *password,
				  unsigned int flags,
				  void *output_data,
				  size_t * output_data_size)
{
  ASN1_TYPE pkcs8_asn, pkey_info;
  int ret;
  gnutls_datum_t tmp;
  schema_id schema;

  if (key == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  /* Get the private key info
   * tmp holds the DER encoding.
   */
  ret = encode_to_private_key_info (key, &tmp, &pkey_info);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  if (flags & GNUTLS_PKCS_USE_PKCS12_3DES)
    schema = PKCS12_3DES_SHA1;
  else if (flags & GNUTLS_PKCS_USE_PKCS12_ARCFOUR)
    schema = PKCS12_ARCFOUR_SHA1;
  else if (flags & GNUTLS_PKCS_USE_PKCS12_RC2_40)
    schema = PKCS12_RC2_40_SHA1;
  else
    schema = PBES2;


  if ((flags & GNUTLS_PKCS_PLAIN) || password == NULL)
    {
      _gnutls_free_datum (&tmp);

      ret =
	_gnutls_x509_export_int (pkey_info, format,
				 PEM_UNENCRYPTED_PKCS8,
				 *output_data_size, output_data,
				 output_data_size);

      asn1_delete_structure (&pkey_info);
    }
  else
    {
      asn1_delete_structure (&pkey_info);	/* we don't need it */

      ret = encode_to_pkcs8_key (schema, &tmp, password, &pkcs8_asn);
      _gnutls_free_datum (&tmp);

      if (ret < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      ret =
	_gnutls_x509_export_int (pkcs8_asn, format, PEM_PKCS8,
				 *output_data_size, output_data,
				 output_data_size);

      asn1_delete_structure (&pkcs8_asn);
    }

  return ret;
}


/* Read the parameters cipher, IV, salt etc using the given
 * schema ID.
 */
static int
read_pkcs_schema_params (schema_id schema, const char *password,
			 const opaque * data, int data_size,
			 struct pbkdf2_params *kdf_params,
			 struct pbe_enc_params *enc_params)
{
  ASN1_TYPE pbes2_asn = ASN1_TYPE_EMPTY;
  int result;
  gnutls_datum_t tmp;

  switch (schema)
    {

    case PBES2:

      /* Now check the key derivation and the encryption
       * functions.
       */
      if ((result =
	   asn1_create_element (_gnutls_get_pkix (),
				"PKIX1.pkcs-5-PBES2-params",
				&pbes2_asn)) != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  result = _gnutls_asn2err (result);
	  goto error;
	}

      /* Decode the parameters.
       */
      result = asn1_der_decoding (&pbes2_asn, data, data_size, NULL);
      if (result != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  result = _gnutls_asn2err (result);
	  goto error;
	}

      tmp.data = (opaque *) data;
      tmp.size = data_size;

      result = read_pbkdf2_params (pbes2_asn, &tmp, kdf_params);
      if (result < 0)
	{
	  gnutls_assert ();
	  result = _gnutls_asn2err (result);
	  goto error;
	}

      result = read_pbe_enc_params (pbes2_asn, &tmp, enc_params);
      if (result < 0)
	{
	  gnutls_assert ();
	  result = _gnutls_asn2err (result);
	  goto error;
	}

      asn1_delete_structure (&pbes2_asn);
      return 0;
      break;

    case PKCS12_3DES_SHA1:
    case PKCS12_ARCFOUR_SHA1:
    case PKCS12_RC2_40_SHA1:

      if ((schema) == PKCS12_3DES_SHA1)
	{
	  enc_params->cipher = GNUTLS_CIPHER_3DES_CBC;
	  enc_params->iv_size = 8;
	}
      else if ((schema) == PKCS12_ARCFOUR_SHA1)
	{
	  enc_params->cipher = GNUTLS_CIPHER_ARCFOUR_128;
	  enc_params->iv_size = 0;
	}
      else if ((schema) == PKCS12_RC2_40_SHA1)
	{
	  enc_params->cipher = GNUTLS_CIPHER_RC2_40_CBC;
	  enc_params->iv_size = 8;
	}

      if ((result =
	   asn1_create_element (_gnutls_get_pkix (),
				"PKIX1.pkcs-12-PbeParams",
				&pbes2_asn)) != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  result = _gnutls_asn2err (result);
	  goto error;
	}

      /* Decode the parameters.
       */
      result = asn1_der_decoding (&pbes2_asn, data, data_size, NULL);
      if (result != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  result = _gnutls_asn2err (result);
	  goto error;
	}

      result = read_pkcs12_kdf_params (pbes2_asn, kdf_params);
      if (result < 0)
	{
	  gnutls_assert ();
	  goto error;
	}

      if (enc_params->iv_size)
	{
	  result =
	    _pkcs12_string_to_key (2 /*IV*/, kdf_params->salt,
				   kdf_params->salt_size,
				   kdf_params->iter_count, password,
				   enc_params->iv_size, enc_params->iv);
	  if (result < 0)
	    {
	      gnutls_assert ();
	      goto error;
	    }

	}

      asn1_delete_structure (&pbes2_asn);

      return 0;
      break;

    }				/* switch */

  return GNUTLS_E_UNKNOWN_CIPHER_TYPE;

error:
  asn1_delete_structure (&pbes2_asn);
  return result;
}


/* Converts a PKCS #8 key to
 * an internal structure (gnutls_private_key)
 * (normally a PKCS #1 encoded RSA key)
 */
static int
decode_pkcs8_key (const gnutls_datum_t * raw_key,
		  const char *password,
		  gnutls_x509_privkey_t pkey, ASN1_TYPE * out)
{
  int result, len;
  char enc_oid[64];
  gnutls_datum_t tmp;
  ASN1_TYPE pbes2_asn = ASN1_TYPE_EMPTY, pkcs8_asn = ASN1_TYPE_EMPTY;
  ASN1_TYPE ret_asn;
  int params_start, params_end, params_len;
  struct pbkdf2_params kdf_params;
  struct pbe_enc_params enc_params;
  schema_id schema;

  if ((result =
       asn1_create_element (_gnutls_get_pkix (),
			    "PKIX1.pkcs-8-EncryptedPrivateKeyInfo",
			    &pkcs8_asn)) != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  result = asn1_der_decoding (&pkcs8_asn, raw_key->data, raw_key->size, NULL);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* Check the encryption schema OID
   */
  len = sizeof (enc_oid);
  result =
    asn1_read_value (pkcs8_asn, "encryptionAlgorithm.algorithm",
		     enc_oid, &len);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      goto error;
    }

  if ((result = check_schema (enc_oid)) < 0)
    {
      gnutls_assert ();
      goto error;
    }

  schema = result;

  /* Get the DER encoding of the parameters.
   */
  result =
    asn1_der_decoding_startEnd (pkcs8_asn, raw_key->data,
				raw_key->size,
				"encryptionAlgorithm.parameters",
				&params_start, &params_end);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }
  params_len = params_end - params_start + 1;

  result =
    read_pkcs_schema_params (schema, password,
			     &raw_key->data[params_start],
			     params_len, &kdf_params, &enc_params);


  /* Parameters have been decoded. Now
   * decrypt the EncryptedData.
   */
  result =
    decrypt_data (schema, pkcs8_asn, "encryptedData", password,
		  &kdf_params, &enc_params, &tmp);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  asn1_delete_structure (&pkcs8_asn);

  result = decode_private_key_info (&tmp, pkey, &ret_asn);
  _gnutls_free_datum (&tmp);

  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  *out = ret_asn;

  return 0;

error:
  asn1_delete_structure (&pbes2_asn);
  asn1_delete_structure (&pkcs8_asn);
  return result;
}

static int
decode_private_key_info (const gnutls_datum_t * der,
			 gnutls_x509_privkey_t pkey, ASN1_TYPE * out)
{
  int result, len;
  opaque oid[64], *data = NULL;
  gnutls_datum_t tmp;
  ASN1_TYPE pkcs8_asn = ASN1_TYPE_EMPTY;
  ASN1_TYPE ret_asn;
  int data_size;


  if ((result =
       asn1_create_element (_gnutls_get_pkix (),
			    "PKIX1.pkcs-8-PrivateKeyInfo",
			    &pkcs8_asn)) != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  result = asn1_der_decoding (&pkcs8_asn, der->data, der->size, NULL);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      goto error;
    }

  /* Check the private key algorithm OID
   */
  len = sizeof (oid);
  result =
    asn1_read_value (pkcs8_asn, "privateKeyAlgorithm.algorithm", oid, &len);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* we only support RSA private keys.
   */
  if (strcmp (oid, PK_PKIX1_RSA_OID) != 0)
    {
      gnutls_assert ();
      _gnutls_x509_log
	("PKCS #8 private key OID '%s' is unsupported.\n", oid);
      result = GNUTLS_E_UNKNOWN_PK_ALGORITHM;
      goto error;
    }

  /* Get the DER encoding of the actual private key.
   */
  data_size = 0;
  result = asn1_read_value (pkcs8_asn, "privateKey", NULL, &data_size);
  if (result != ASN1_MEM_ERROR)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  data = gnutls_alloca (data_size);
  if (data == NULL)
    {
      gnutls_assert ();
      result = GNUTLS_E_MEMORY_ERROR;
      goto error;
    }

  result = asn1_read_value (pkcs8_asn, "privateKey", data, &data_size);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  asn1_delete_structure (&pkcs8_asn);

  tmp.data = data;
  tmp.size = data_size;

  pkey->pk_algorithm = GNUTLS_PK_RSA;

  ret_asn = _gnutls_privkey_decode_pkcs1_rsa_key (&tmp, pkey);
  if (ret_asn == NULL)
    {
      gnutls_assert ();
    }

  *out = ret_asn;

  return 0;

error:
  asn1_delete_structure (&pkcs8_asn);
  if (data != NULL)
    {
      gnutls_afree (data);
    }
  return result;

}


/**
  * gnutls_x509_privkey_import_pkcs8 - This function will import a DER or PEM PKCS8 encoded key
  * @key: The structure to store the parsed key
  * @data: The DER or PEM encoded key.
  * @format: One of DER or PEM
  * @password: the password to decrypt the key (if it is encrypted).
  * @flags: 0 if encrypted or GNUTLS_PKCS_PLAIN if not encrypted.
  *
  * This function will convert the given DER or PEM encoded PKCS8 2.0 encrypted key
  * to the native gnutls_x509_privkey_t format. The output will be stored in @key.
  * Currently only RSA keys can be imported, and flags can only be used to indicate
  * an unencrypted key.
  *
  * The @password can be either ASCII or UTF-8 in the default PBES2
  * encryption schemas, or ASCII for the PKCS12 schemas.
  *
  * If the Certificate is PEM encoded it should have a header of "ENCRYPTED PRIVATE KEY",
  * or "PRIVATE KEY". You only need to specify the flags if the key is DER encoded, since
  * in that case the encryption status cannot be auto-detected.
  *
  * Returns 0 on success.
  *
  **/
int
gnutls_x509_privkey_import_pkcs8 (gnutls_x509_privkey_t key,
				  const gnutls_datum_t * data,
				  gnutls_x509_crt_fmt_t format,
				  const char *password, unsigned int flags)
{
  int result = 0, need_free = 0;
  gnutls_datum_t _data;

  if (key == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_INVALID_REQUEST;
    }

  _data.data = data->data;
  _data.size = data->size;

  key->pk_algorithm = GNUTLS_PK_UNKNOWN;

  /* If the Certificate is in PEM format then decode it
   */
  if (format == GNUTLS_X509_FMT_PEM)
    {
      opaque *out;

      /* Try the first header 
       */
      result =
	_gnutls_fbase64_decode (PEM_UNENCRYPTED_PKCS8,
				data->data, data->size, &out);

      if (result < 0)
	{			/* Try the encrypted header 
				 */
	  result =
	    _gnutls_fbase64_decode (PEM_PKCS8, data->data, data->size, &out);

	  if (result <= 0)
	    {
	      if (result == 0)
		result = GNUTLS_E_INTERNAL_ERROR;
	      gnutls_assert ();
	      return result;
	    }
	}
      else if (flags == 0)
	flags |= GNUTLS_PKCS_PLAIN;

      _data.data = out;
      _data.size = result;

      need_free = 1;
    }

  if (flags & GNUTLS_PKCS_PLAIN)
    {
      result = decode_private_key_info (&_data, key, &key->key);
    }
  else
    {				/* encrypted. */
      result = decode_pkcs8_key (&_data, password, key, &key->key);
    }

  if (result < 0)
    {
      gnutls_assert ();
      goto cleanup;
    }

  if (need_free)
    _gnutls_free_datum (&_data);

  /* The key has now been decoded.
   */

  return 0;

cleanup:
  key->pk_algorithm = GNUTLS_PK_UNKNOWN;
  if (need_free)
    _gnutls_free_datum (&_data);
  return result;
}

/* Reads the PBKDF2 parameters.
 */
static int
read_pbkdf2_params (ASN1_TYPE pbes2_asn,
		    const gnutls_datum_t * der, struct pbkdf2_params *params)
{
  int params_start, params_end;
  int params_len, len, result;
  ASN1_TYPE pbkdf2_asn = ASN1_TYPE_EMPTY;
  char oid[64];

  memset (params, 0, sizeof (params));

  /* Check the key derivation algorithm
   */
  len = sizeof (oid);
  result =
    asn1_read_value (pbes2_asn, "keyDerivationFunc.algorithm", oid, &len);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }
  _gnutls_hard_log ("keyDerivationFunc.algorithm: %s\n", oid);

  if (strcmp (oid, PBKDF2_OID) != 0)
    {
      gnutls_assert ();
      _gnutls_x509_log
	("PKCS #8 key derivation OID '%s' is unsupported.\n", oid);
      return _gnutls_asn2err (result);
    }

  result =
    asn1_der_decoding_startEnd (pbes2_asn, der->data, der->size,
				"keyDerivationFunc.parameters",
				&params_start, &params_end);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }
  params_len = params_end - params_start + 1;

  /* Now check the key derivation and the encryption
   * functions.
   */
  if ((result =
       asn1_create_element (_gnutls_get_pkix (),
			    "PKIX1.pkcs-5-PBKDF2-params",
			    &pbkdf2_asn)) != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  result =
    asn1_der_decoding (&pbkdf2_asn, &der->data[params_start],
		       params_len, NULL);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* read the salt */
  params->salt_size = sizeof (params->salt);
  result =
    asn1_read_value (pbkdf2_asn, "salt.specified", params->salt,
		     &params->salt_size);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }
  _gnutls_hard_log ("salt.specified.size: %d\n", params->salt_size);

  /* read the iteration count 
   */
  result =
    _gnutls_x509_read_uint (pbkdf2_asn, "iterationCount",
			    &params->iter_count);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      goto error;
    }
  _gnutls_hard_log ("iterationCount: %d\n", params->iter_count);

  /* read the keylength, if it is set.
   */
  result =
    _gnutls_x509_read_uint (pbkdf2_asn, "keyLength", &params->key_size);
  if (result < 0)
    {
      params->key_size = 0;
    }
  _gnutls_hard_log ("keyLength: %d\n", params->key_size);

  /* We don't read the PRF. We only use the default.
   */

  return 0;

error:
  asn1_delete_structure (&pbkdf2_asn);
  return result;

}

/* Reads the PBE parameters from PKCS-12 schemas (*&#%*&#% RSA).
 */
static int
read_pkcs12_kdf_params (ASN1_TYPE pbes2_asn, struct pbkdf2_params *params)
{
  int result;

  memset (params, 0, sizeof (params));

  /* read the salt */
  params->salt_size = sizeof (params->salt);
  result =
    asn1_read_value (pbes2_asn, "salt", params->salt, &params->salt_size);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }
  _gnutls_hard_log ("salt.size: %d\n", params->salt_size);

  /* read the iteration count 
   */
  result =
    _gnutls_x509_read_uint (pbes2_asn, "iterations", &params->iter_count);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      goto error;
    }
  _gnutls_hard_log ("iterationCount: %d\n", params->iter_count);

  params->key_size = 0;

  return 0;

error:
  return result;

}

/* Writes the PBE parameters for PKCS-12 schemas.
 */
static int
write_pkcs12_kdf_params (ASN1_TYPE pbes2_asn,
			 const struct pbkdf2_params *kdf_params)
{
  int result;

  /* write the salt 
   */
  result =
    asn1_write_value (pbes2_asn, "salt",
		      kdf_params->salt, kdf_params->salt_size);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }
  _gnutls_hard_log ("salt.size: %d\n", kdf_params->salt_size);

  /* write the iteration count 
   */
  result =
    _gnutls_x509_write_uint32 (pbes2_asn, "iterations",
			       kdf_params->iter_count);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }
  _gnutls_hard_log ("iterationCount: %d\n", kdf_params->iter_count);

  return 0;

error:
  return result;

}


/* Converts an OID to a gnutls cipher type.
 */
inline static int
oid2cipher (const char *oid, gnutls_cipher_algorithm_t * algo)
{

  *algo = 0;

  if (strcmp (oid, DES_EDE3_CBC_OID) == 0)
    {
      *algo = GNUTLS_CIPHER_3DES_CBC;
      return 0;
    }

  if (strcmp (oid, DES_CBC_OID) == 0)
    {
      *algo = GNUTLS_CIPHER_DES_CBC;
      return 0;
    }

  _gnutls_x509_log ("PKCS #8 encryption OID '%s' is unsupported.\n", oid);
  return GNUTLS_E_UNKNOWN_CIPHER_TYPE;
}


static int
read_pbe_enc_params (ASN1_TYPE pbes2_asn,
		     const gnutls_datum_t * der,
		     struct pbe_enc_params *params)
{
  int params_start, params_end;
  int params_len, len, result;
  ASN1_TYPE pbe_asn = ASN1_TYPE_EMPTY;
  char oid[64];

  memset (params, 0, sizeof (params));

  /* Check the encryption algorithm
   */
  len = sizeof (oid);
  result =
    asn1_read_value (pbes2_asn, "encryptionScheme.algorithm", oid, &len);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      goto error;
    }
  _gnutls_hard_log ("encryptionScheme.algorithm: %s\n", oid);

  if ((result = oid2cipher (oid, &params->cipher)) < 0)
    {
      gnutls_assert ();
      goto error;
    }

  result =
    asn1_der_decoding_startEnd (pbes2_asn, der->data, der->size,
				"encryptionScheme.parameters",
				&params_start, &params_end);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }
  params_len = params_end - params_start + 1;

  /* Now check the encryption parameters.
   */
  if ((result =
       asn1_create_element (_gnutls_get_pkix (),
			    "PKIX1.pkcs-5-des-EDE3-CBC-params",
			    &pbe_asn)) != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  result =
    asn1_der_decoding (&pbe_asn, &der->data[params_start], params_len, NULL);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* read the IV */
  params->iv_size = sizeof (params->iv);
  result = asn1_read_value (pbe_asn, "", params->iv, &params->iv_size);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }
  _gnutls_hard_log ("IV.size: %d\n", params->iv_size);

  return 0;

error:
  asn1_delete_structure (&pbe_asn);
  return result;

}

static int
decrypt_data (schema_id schema, ASN1_TYPE pkcs8_asn,
	      const char *root, const char *password,
	      const struct pbkdf2_params *kdf_params,
	      const struct pbe_enc_params *enc_params,
	      gnutls_datum_t * decrypted_data)
{
  int result;
  int data_size;
  opaque *data = NULL, *key = NULL;
  gnutls_datum_t dkey, d_iv;
  cipher_hd_t ch = NULL;
  int key_size;

  data_size = 0;
  result = asn1_read_value (pkcs8_asn, root, NULL, &data_size);
  if (result != ASN1_MEM_ERROR)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  data = gnutls_malloc (data_size);
  if (data == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_MEMORY_ERROR;
    }

  result = asn1_read_value (pkcs8_asn, root, data, &data_size);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  if (kdf_params->key_size == 0)
    {
      key_size = gnutls_cipher_get_key_size (enc_params->cipher);
    }
  else
    key_size = kdf_params->key_size;

  key = gnutls_alloca (key_size);
  if (key == NULL)
    {
      gnutls_assert ();
      result = GNUTLS_E_MEMORY_ERROR;
      goto error;
    }

  /* generate the key
   */
  if (schema == PBES2)
    {
      result = gc_pbkdf2_sha1 (password, strlen (password),
			       kdf_params->salt, kdf_params->salt_size,
			       kdf_params->iter_count, key, key_size);

      if (result != GC_OK)
	{
	  gnutls_assert ();
	  result = GNUTLS_E_DECRYPTION_FAILED;
	  goto error;
	}
    }
  else
    {
      result =
	_pkcs12_string_to_key (1 /*KEY*/, kdf_params->salt,
			       kdf_params->salt_size,
			       kdf_params->iter_count, password,
			       key_size, key);

      if (result < 0)
	{
	  gnutls_assert ();
	  goto error;
	}
    }

  /* do the decryption.
   */
  dkey.data = key;
  dkey.size = key_size;

  d_iv.data = (opaque *) enc_params->iv;
  d_iv.size = enc_params->iv_size;
  ch = _gnutls_cipher_init (enc_params->cipher, &dkey, &d_iv);

  gnutls_afree (key);
  key = NULL;

  if (ch == NULL)
    {
      gnutls_assert ();
      result = GNUTLS_E_DECRYPTION_FAILED;
      goto error;
    }

  result = _gnutls_cipher_decrypt (ch, data, data_size);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  decrypted_data->data = data;

  if (_gnutls_cipher_get_block_size (enc_params->cipher) != 1)
    decrypted_data->size = data_size - data[data_size - 1];
  else
    decrypted_data->size = data_size;

  _gnutls_cipher_deinit (ch);

  return 0;

error:
  gnutls_free (data);
  gnutls_afree (key);
  if (ch != NULL)
    _gnutls_cipher_deinit (ch);
  return result;
}


/* Writes the PBKDF2 parameters.
 */
static int
write_pbkdf2_params (ASN1_TYPE pbes2_asn,
		     const struct pbkdf2_params *kdf_params)
{
  int result;
  ASN1_TYPE pbkdf2_asn = ASN1_TYPE_EMPTY;
  opaque tmp[64];

  /* Write the key derivation algorithm
   */
  result =
    asn1_write_value (pbes2_asn, "keyDerivationFunc.algorithm",
		      PBKDF2_OID, 1);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  /* Now write the key derivation and the encryption
   * functions.
   */
  if ((result =
       asn1_create_element (_gnutls_get_pkix (),
			    "PKIX1.pkcs-5-PBKDF2-params",
			    &pbkdf2_asn)) != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  result = asn1_write_value (pbkdf2_asn, "salt", "specified", 1);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* write the salt 
   */
  result =
    asn1_write_value (pbkdf2_asn, "salt.specified",
		      kdf_params->salt, kdf_params->salt_size);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }
  _gnutls_hard_log ("salt.specified.size: %d\n", kdf_params->salt_size);

  /* write the iteration count 
   */
  _gnutls_write_uint32 (kdf_params->iter_count, tmp);

  result = asn1_write_value (pbkdf2_asn, "iterationCount", tmp, 4);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }
  _gnutls_hard_log ("iterationCount: %d\n", kdf_params->iter_count);

  /* write the keylength, if it is set.
   */
  result = asn1_write_value (pbkdf2_asn, "keyLength", NULL, 0);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* We write an emptry prf.
   */
  result = asn1_write_value (pbkdf2_asn, "prf", NULL, 0);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* now encode them an put the DER output
   * in the keyDerivationFunc.parameters
   */
  result = _gnutls_x509_der_encode_and_copy (pbkdf2_asn, "",
					     pbes2_asn,
					     "keyDerivationFunc.parameters",
					     0);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  return 0;

error:
  asn1_delete_structure (&pbkdf2_asn);
  return result;

}

static int
write_pbe_enc_params (ASN1_TYPE pbes2_asn,
		      const struct pbe_enc_params *params)
{
  int result;
  ASN1_TYPE pbe_asn = ASN1_TYPE_EMPTY;

  /* Write the encryption algorithm
   */
  result =
    asn1_write_value (pbes2_asn, "encryptionScheme.algorithm",
		      DES_EDE3_CBC_OID, 1);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      goto error;
    }
  _gnutls_hard_log ("encryptionScheme.algorithm: %s\n", DES_EDE3_CBC_OID);

  /* Now check the encryption parameters.
   */
  if ((result =
       asn1_create_element (_gnutls_get_pkix (),
			    "PKIX1.pkcs-5-des-EDE3-CBC-params",
			    &pbe_asn)) != ASN1_SUCCESS)
    {
      gnutls_assert ();
      return _gnutls_asn2err (result);
    }

  /* read the salt */
  result = asn1_write_value (pbe_asn, "", params->iv, params->iv_size);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }
  _gnutls_hard_log ("IV.size: %d\n", params->iv_size);

  /* now encode them an put the DER output
   * in the encryptionScheme.parameters
   */
  result = _gnutls_x509_der_encode_and_copy (pbe_asn, "",
					     pbes2_asn,
					     "encryptionScheme.parameters",
					     0);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  return 0;

error:
  asn1_delete_structure (&pbe_asn);
  return result;

}

/* Generates a key and also stores the key parameters.
 */
static int
generate_key (schema_id schema,
	      const char *password,
	      struct pbkdf2_params *kdf_params,
	      struct pbe_enc_params *enc_params, gnutls_datum_t * key)
{
  opaque rnd[2];
  int ret;

  /* We should use the flags here to use different
   * encryption algorithms etc. 
   */

  if (schema == PKCS12_ARCFOUR_SHA1)
    enc_params->cipher = GNUTLS_CIPHER_ARCFOUR_128;
  else if (schema == PKCS12_3DES_SHA1)
    enc_params->cipher = GNUTLS_CIPHER_3DES_CBC;
  else if (schema == PKCS12_RC2_40_SHA1)
    enc_params->cipher = GNUTLS_CIPHER_RC2_40_CBC;

  if (gc_pseudo_random (rnd, 2) != GC_OK)
    {
      gnutls_assert ();
      return GNUTLS_E_RANDOM_FAILED;
    }

  /* generate salt */

  if (schema == PBES2)
    kdf_params->salt_size =
      MIN (sizeof (kdf_params->salt), (unsigned) (10 + (rnd[1] % 10)));
  else
    kdf_params->salt_size = 8;

  if (gc_pseudo_random (kdf_params->salt, kdf_params->salt_size) != GC_OK)
    {
      gnutls_assert ();
      return GNUTLS_E_RANDOM_FAILED;
    }

  kdf_params->iter_count = 256 + rnd[0];
  key->size = kdf_params->key_size =
    gnutls_cipher_get_key_size (enc_params->cipher);

  enc_params->iv_size = _gnutls_cipher_get_iv_size (enc_params->cipher);

  key->data = gnutls_secure_malloc (key->size);
  if (key->data == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_MEMORY_ERROR;
    }

  /* now generate the key. 
   */

  if (schema == PBES2)
    {

      ret = gc_pbkdf2_sha1 (password, strlen (password),
			    kdf_params->salt, kdf_params->salt_size,
			    kdf_params->iter_count,
			    key->data, kdf_params->key_size);
      if (ret != GC_OK)
	{
	  gnutls_assert ();
	  return GNUTLS_E_ENCRYPTION_FAILED;
	}

      if (enc_params->iv_size &&
	  gc_nonce (enc_params->iv, enc_params->iv_size) != GC_OK)
	{
	  gnutls_assert ();
	  return GNUTLS_E_RANDOM_FAILED;
	}
    }
  else
    {				/* PKCS12 schemas */
      ret =
	_pkcs12_string_to_key (1 /*KEY*/, kdf_params->salt,
			       kdf_params->salt_size,
			       kdf_params->iter_count, password,
			       kdf_params->key_size, key->data);
      if (ret < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      /* Now generate the IV
       */
      if (enc_params->iv_size)
	{
	  ret =
	    _pkcs12_string_to_key (2 /*IV*/, kdf_params->salt,
				   kdf_params->salt_size,
				   kdf_params->iter_count, password,
				   enc_params->iv_size, enc_params->iv);
	  if (ret < 0)
	    {
	      gnutls_assert ();
	      return ret;
	    }
	}
    }


  return 0;
}


/* Encodes the parameters to be written in the encryptionAlgorithm.parameters
 * part.
 */
static int
write_schema_params (schema_id schema, ASN1_TYPE pkcs8_asn,
		     const char *where,
		     const struct pbkdf2_params *kdf_params,
		     const struct pbe_enc_params *enc_params)
{
  int result;
  ASN1_TYPE pbes2_asn = ASN1_TYPE_EMPTY;

  if (schema == PBES2)
    {
      if ((result =
	   asn1_create_element (_gnutls_get_pkix (),
				"PKIX1.pkcs-5-PBES2-params",
				&pbes2_asn)) != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  return _gnutls_asn2err (result);
	}

      result = write_pbkdf2_params (pbes2_asn, kdf_params);
      if (result < 0)
	{
	  gnutls_assert ();
	  goto error;
	}

      result = write_pbe_enc_params (pbes2_asn, enc_params);
      if (result < 0)
	{
	  gnutls_assert ();
	  goto error;
	}

      result = _gnutls_x509_der_encode_and_copy (pbes2_asn, "",
						 pkcs8_asn, where, 0);
      if (result < 0)
	{
	  gnutls_assert ();
	  goto error;
	}

      asn1_delete_structure (&pbes2_asn);
    }
  else
    {				/* PKCS12 schemas */

      if ((result =
	   asn1_create_element (_gnutls_get_pkix (),
				"PKIX1.pkcs-12-PbeParams",
				&pbes2_asn)) != ASN1_SUCCESS)
	{
	  gnutls_assert ();
	  result = _gnutls_asn2err (result);
	  goto error;
	}

      result = write_pkcs12_kdf_params (pbes2_asn, kdf_params);
      if (result < 0)
	{
	  gnutls_assert ();
	  goto error;
	}

      result = _gnutls_x509_der_encode_and_copy (pbes2_asn, "",
						 pkcs8_asn, where, 0);
      if (result < 0)
	{
	  gnutls_assert ();
	  goto error;
	}

      asn1_delete_structure (&pbes2_asn);

    }

  return 0;

error:
  asn1_delete_structure (&pbes2_asn);
  return result;

}

static int
encrypt_data (const gnutls_datum_t * plain,
	      const struct pbe_enc_params *enc_params,
	      gnutls_datum_t * key, gnutls_datum_t * encrypted)
{
  int result;
  int data_size;
  opaque *data = NULL;
  gnutls_datum_t d_iv;
  cipher_hd_t ch = NULL;
  opaque pad, pad_size;

  pad_size = _gnutls_cipher_get_block_size (enc_params->cipher);

  if (pad_size == 1)		/* stream */
    pad_size = 0;

  data = gnutls_malloc (plain->size + pad_size);
  if (data == NULL)
    {
      gnutls_assert ();
      return GNUTLS_E_MEMORY_ERROR;
    }

  memcpy (data, plain->data, plain->size);

  if (pad_size > 0)
    {
      pad = pad_size - (plain->size % pad_size);
      if (pad == 0)
	pad = pad_size;
      memset (&data[plain->size], pad, pad);
    }
  else
    pad = 0;

  data_size = plain->size + pad;

  d_iv.data = (opaque *) enc_params->iv;
  d_iv.size = enc_params->iv_size;
  ch = _gnutls_cipher_init (enc_params->cipher, key, &d_iv);

  if (ch == GNUTLS_CIPHER_FAILED)
    {
      gnutls_assert ();
      result = GNUTLS_E_ENCRYPTION_FAILED;
      goto error;
    }

  result = _gnutls_cipher_encrypt (ch, data, data_size);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  encrypted->data = data;
  encrypted->size = data_size;

  _gnutls_cipher_deinit (ch);

  return 0;

error:
  gnutls_free (data);
  if (ch != NULL)
    _gnutls_cipher_deinit (ch);
  return result;
}

/* Decrypts a PKCS #7 encryptedData. The output is allocated
 * and stored in dec.
 */
int
_gnutls_pkcs7_decrypt_data (const gnutls_datum_t * data,
			    const char *password, gnutls_datum_t * dec)
{
  int result, len;
  char enc_oid[64];
  gnutls_datum_t tmp;
  ASN1_TYPE pbes2_asn = ASN1_TYPE_EMPTY, pkcs7_asn = ASN1_TYPE_EMPTY;
  int params_start, params_end, params_len;
  struct pbkdf2_params kdf_params;
  struct pbe_enc_params enc_params;
  schema_id schema;

  if ((result =
       asn1_create_element (_gnutls_get_pkix (),
			    "PKIX1.pkcs-7-EncryptedData",
			    &pkcs7_asn)) != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  result = asn1_der_decoding (&pkcs7_asn, data->data, data->size, NULL);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* Check the encryption schema OID
   */
  len = sizeof (enc_oid);
  result =
    asn1_read_value (pkcs7_asn,
		     "encryptedContentInfo.contentEncryptionAlgorithm.algorithm",
		     enc_oid, &len);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  if ((result = check_schema (enc_oid)) < 0)
    {
      gnutls_assert ();
      goto error;
    }
  schema = result;

  /* Get the DER encoding of the parameters.
   */
  result =
    asn1_der_decoding_startEnd (pkcs7_asn, data->data, data->size,
				"encryptedContentInfo.contentEncryptionAlgorithm.parameters",
				&params_start, &params_end);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }
  params_len = params_end - params_start + 1;

  result =
    read_pkcs_schema_params (schema, password,
			     &data->data[params_start],
			     params_len, &kdf_params, &enc_params);
  if (result < ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* Parameters have been decoded. Now
   * decrypt the EncryptedData.
   */

  result =
    decrypt_data (schema, pkcs7_asn,
		  "encryptedContentInfo.encryptedContent", password,
		  &kdf_params, &enc_params, &tmp);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  asn1_delete_structure (&pkcs7_asn);

  *dec = tmp;

  return 0;

error:
  asn1_delete_structure (&pbes2_asn);
  asn1_delete_structure (&pkcs7_asn);
  return result;
}

/* Encrypts to a PKCS #7 encryptedData. The output is allocated
 * and stored in enc.
 */
int
_gnutls_pkcs7_encrypt_data (schema_id schema,
			    const gnutls_datum_t * data,
			    const char *password, gnutls_datum_t * enc)
{
  int result;
  gnutls_datum_t key = { NULL, 0 };
  gnutls_datum_t tmp = { NULL, 0 };
  ASN1_TYPE pkcs7_asn = ASN1_TYPE_EMPTY;
  struct pbkdf2_params kdf_params;
  struct pbe_enc_params enc_params;


  if ((result =
       asn1_create_element (_gnutls_get_pkix (),
			    "PKIX1.pkcs-7-EncryptedData",
			    &pkcs7_asn)) != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* Write the encryption schema OID
   */
  switch (schema)
    {
    case PBES2:
      result =
	asn1_write_value (pkcs7_asn,
			  "encryptedContentInfo.contentEncryptionAlgorithm.algorithm",
			  PBES2_OID, 1);
      break;
    case PKCS12_3DES_SHA1:
      result =
	asn1_write_value (pkcs7_asn,
			  "encryptedContentInfo.contentEncryptionAlgorithm.algorithm",
			  PKCS12_PBE_3DES_SHA1_OID, 1);
      break;
    case PKCS12_ARCFOUR_SHA1:
      result =
	asn1_write_value (pkcs7_asn,
			  "encryptedContentInfo.contentEncryptionAlgorithm.algorithm",
			  PKCS12_PBE_ARCFOUR_SHA1_OID, 1);
      break;
    case PKCS12_RC2_40_SHA1:
      result =
	asn1_write_value (pkcs7_asn,
			  "encryptedContentInfo.contentEncryptionAlgorithm.algorithm",
			  PKCS12_PBE_RC2_40_SHA1_OID, 1);
      break;

    }

  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* Generate a symmetric key.
   */

  result = generate_key (schema, password, &kdf_params, &enc_params, &key);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  result = write_schema_params (schema, pkcs7_asn,
				"encryptedContentInfo.contentEncryptionAlgorithm.parameters",
				&kdf_params, &enc_params);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  /* Parameters have been encoded. Now
   * encrypt the Data.
   */
  result = encrypt_data (data, &enc_params, &key, &tmp);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  /* write the encrypted data.
   */
  result =
    asn1_write_value (pkcs7_asn,
		      "encryptedContentInfo.encryptedContent", tmp.data,
		      tmp.size);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  _gnutls_free_datum (&tmp);
  _gnutls_free_datum (&key);

  /* Now write the rest of the pkcs-7 stuff.
   */

  result = _gnutls_x509_write_uint32 (pkcs7_asn, "version", 0);
  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }

  result =
    asn1_write_value (pkcs7_asn, "encryptedContentInfo.contentType",
		      DATA_OID, 1);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  result = asn1_write_value (pkcs7_asn, "unprotectedAttrs", NULL, 0);
  if (result != ASN1_SUCCESS)
    {
      gnutls_assert ();
      result = _gnutls_asn2err (result);
      goto error;
    }

  /* Now encode and copy the DER stuff.
   */
  result = _gnutls_x509_der_encode (pkcs7_asn, "", enc, 0);

  asn1_delete_structure (&pkcs7_asn);

  if (result < 0)
    {
      gnutls_assert ();
      goto error;
    }


error:
  _gnutls_free_datum (&key);
  _gnutls_free_datum (&tmp);
  asn1_delete_structure (&pkcs7_asn);
  return result;
}


#endif
