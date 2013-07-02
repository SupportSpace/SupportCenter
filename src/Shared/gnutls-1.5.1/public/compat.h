/* Typedefs to be fully compatible with the types of
 * GnuTLS 1.0.x.
 */

#ifndef GCOMPAT_H
# define GCOMPAT_H

#define gnutls_cipher_algorithm gnutls_cipher_algorithm_t
#define gnutls_kx_algorithm gnutls_kx_algorithm_t
#define gnutls_paramsype gnutls_paramsype_t
#define gnutls_mac_algorithm gnutls_mac_algorithm_t
#define gnutls_digest_algorithm gnutls_digest_algorithm_t
#define gnutls_compression_method gnutls_compression_method_t
#define gnutls_connection_end gnutls_connection_end_t
#define gnutls_credentialsype gnutls_credentialsype_t
#define gnutls_certificateype gnutls_certificateype_t
#define gnutls_x509_crt_fmt gnutls_x509_crt_fmt_t
#define gnutls_openpgp_key_fmt gnutls_openpgp_key_fmt_t
#define gnutls_pk_algorithm gnutls_pk_algorithm_t
#define gnutls_sign_algorithm gnutls_sign_algorithm_t
#define gnutls_server_name gnutls_server_nameype_t
#define gnutls_protocol gnutls_protocol_version_t
#define gnutls_close_request gnutls_close_request_t
#define gnutls_openpgp_key_status gnutls_openpgp_key_status_t
#define gnutls_certificate_request gnutls_certificate_request_t
#define gnutls_certificate_status gnutls_certificate_status_t
#define gnutls_session gnutls_session_t
#define gnutls_alert_level gnutls_alert_level_t
#define gnutls_alert_description gnutls_alert_description_t
#define gnutls_x509_subject_alt_name gnutls_x509_subject_alt_name_t
#define gnutls_openpgp_key gnutls_openpgp_key_t
#define gnutls_openpgp_privkey gnutls_openpgp_privkey_t
#define gnutls_openpgp_keyring gnutls_openpgp_keyring_t
#define gnutls_openpgp_trustdb gnutls_openpgp_trustdb_t
#define gnutls_x509_crt gnutls_x509_crt_t
#define gnutls_x509_privkey gnutls_x509_privkey_t
#define gnutls_x509_crl gnutls_x509_crl_t
#define gnutls_pkcs7 gnutls_pkcs7_t
#define gnutls_x509_crq gnutls_x509_crq_t
#define gnutls_pkcs_encrypt_flags gnutls_pkcs_encrypt_flags_t
#define gnutls_pkcs12_bag_type gnutls_pkcs12_bag_type_t
#define gnutls_pkcs12_bag gnutls_pkcs12_bag_t
#define gnutls_pkcs12 gnutls_pkcs12_t
#define gnutls_certificate_credentials gnutls_certificate_credentials_t
#define gnutls_anon_server_credentials gnutls_anon_server_credentials_t
#define gnutls_anon_client_credentials gnutls_anon_client_credentials_t
#define gnutls_srp_client_credentials gnutls_srp_client_credentials_t
#define gnutls_srp_server_credentials gnutls_srp_server_credentials_t
#define gnutls_dh_params gnutls_dh_params_t
#define gnutls_rsa_params gnutls_rsa_params_t
#define gnutls_params_type gnutls_params_type_t
#define gnutls_credentials_type gnutls_credentials_type_t
#define gnutls_certificate_type gnutls_certificate_type_t
#define gnutls_datum gnutls_datum_t
#define gnutls_transport_ptr gnutls_transport_ptr_t

#endif /* GCOMPAT_H */
