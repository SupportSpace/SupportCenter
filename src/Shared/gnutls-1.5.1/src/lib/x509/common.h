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

#ifndef COMMON_H
# define COMMON_H

#include <gnutls_algorithms.h>

#define MAX_STRING_LEN 512

#define GNUTLS_XML_SHOW_ALL 1

#define PEM_CRL "X509 CRL"
#define PEM_X509_CERT "X509 CERTIFICATE"
#define PEM_X509_CERT2 "CERTIFICATE"
#define PEM_PKCS7 "PKCS7"
#define PEM_PKCS12 "PKCS12"

/* public key algorithm's OIDs
 */
#define PK_PKIX1_RSA_OID "1.2.840.113549.1.1.1"
#define PK_DSA_OID "1.2.840.10040.4.1"
#define PK_GOST_R3410_94_OID "1.2.643.2.2.20"
#define PK_GOST_R3410_2001_OID "1.2.643.2.2.19"

/* signature OIDs
 */
#define SIG_DSA_SHA1_OID "1.2.840.10040.4.3"
#define SIG_RSA_MD5_OID "1.2.840.113549.1.1.4"
#define SIG_RSA_MD2_OID "1.2.840.113549.1.1.2"
#define SIG_RSA_SHA1_OID "1.2.840.113549.1.1.5"
#define SIG_RSA_RMD160_OID "1.3.36.3.3.1.2"
#define SIG_GOST_R3410_94_OID "1.2.643.2.2.4"
#define SIG_GOST_R3410_2001_OID "1.2.643.2.2.3"

time_t _gnutls_x509_utcTime2gtime (const char *ttime);
time_t _gnutls_x509_generalTime2gtime (const char *ttime);
int _gnutls_x509_set_time (ASN1_TYPE c2, const char *where, time_t tim);

int _gnutls_x509_decode_octet_string (const char *string_type,
				      const opaque * der, size_t der_size,
				      opaque * output, size_t * output_size);
int _gnutls_x509_oid_data2string (const char *OID, void *value,
				  int value_size, char *res,
				  size_t * res_size);
int _gnutls_x509_data2hex (const opaque * data, size_t data_size,
			   opaque * out, size_t * sizeof_out);

const char *_gnutls_x509_oid2ldap_string (const char *OID);

int _gnutls_x509_oid_data_choice (const char *OID);
int _gnutls_x509_oid_data_printable (const char *OID);

time_t _gnutls_x509_get_time (ASN1_TYPE c2, const char *when);

gnutls_x509_subject_alt_name_t _gnutls_x509_san_find_type (char *str_type);

int _gnutls_x509_der_encode_and_copy (ASN1_TYPE src, const char *src_name,
				      ASN1_TYPE dest, const char *dest_name,
				      int str);
int _gnutls_x509_der_encode (ASN1_TYPE src, const char *src_name,
			     gnutls_datum_t * res, int str);

int _gnutls_x509_export_int (ASN1_TYPE asn1_data,
			     gnutls_x509_crt_fmt_t format, char *pem_header,
			     int tmp_buf_size, unsigned char *output_data,
			     size_t * output_data_size);

int _gnutls_x509_read_value (ASN1_TYPE c, const char *root,
			     gnutls_datum_t * ret, int str);
int _gnutls_x509_write_value (ASN1_TYPE c, const char *root,
			      const gnutls_datum_t * data, int str);

int _gnutls_x509_encode_and_write_attribute (const char *given_oid,
					     ASN1_TYPE asn1_struct,
					     const char *where,
					     const void *data,
					     int sizeof_data, int multi);
int _gnutls_x509_decode_and_read_attribute (ASN1_TYPE asn1_struct,
					    const char *where, char *oid,
					    int oid_size,
					    gnutls_datum_t * value, int multi,
					    int octet);

int _gnutls_x509_get_pk_algorithm (ASN1_TYPE src, const char *src_name,
				   unsigned int *bits);

int _gnutls_x509_encode_and_copy_PKI_params (ASN1_TYPE dst,
					     const char *dst_name,
					     gnutls_pk_algorithm_t
					     pk_algorithm, mpi_t * params,
					     int params_size);
int _gnutls_asn1_copy_node (ASN1_TYPE * dst, const char *dst_name,
			    ASN1_TYPE src, const char *src_name);

int _gnutls_x509_get_signed_data (ASN1_TYPE src, const char *src_name,
				  gnutls_datum_t * signed_data);
int _gnutls_x509_get_signature (ASN1_TYPE src, const char *src_name,
				gnutls_datum_t * signature);

#endif
