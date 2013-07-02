#include <gnutls/gnutlsxx.h>

using namespace gnutls;

inline int RETWRAP_NET(int ret) 
{
    if (gnutls_error_is_fatal(ret)) throw(exception(ret));
    else return ret;
}

inline int RETWRAP(int ret) 
{
    if (ret < 0) throw(exception(ret));
    return ret;
}

session::session( gnutls_connection_end_t end)
{
    RETWRAP(gnutls_init( &this->s, end));
}

session::session( session& s)
{
    this->s = s.s;
}

session::~session()
{
    gnutls_deinit( this->s);
}

int session::bye( gnutls_close_request_t how)
{
    return RETWRAP_NET( gnutls_bye( this->s, how));
}

int session::handshake ()
{
    return RETWRAP_NET( gnutls_handshake( this->s));
}


server_session::server_session() : session( GNUTLS_SERVER)
{
}

int server_session::rehandshake()
{
    return RETWRAP_NET( gnutls_rehandshake( this->s));
}

gnutls_alert_description_t session::get_alert() const
{
    return gnutls_alert_get( this->s);
}

int session::send_alert ( gnutls_alert_level_t level,
                          gnutls_alert_description_t desc)
{
    return RETWRAP_NET(gnutls_alert_send( this->s, level, desc));
}

int session::send_appropriate_alert (int err)
{
    return RETWRAP_NET(gnutls_alert_send_appropriate( this->s, err));
}

gnutls_cipher_algorithm_t session::get_cipher() const
{
    return gnutls_cipher_get( this->s);
}

gnutls_kx_algorithm_t session::get_kx () const
{
    return gnutls_kx_get( this->s);
}

gnutls_mac_algorithm_t session::get_mac () const
{
    return gnutls_mac_get( this->s);
}

gnutls_compression_method_t session::get_compression() const
{
    return gnutls_compression_get( this->s);
}

gnutls_certificate_type_t session::get_certificate_type() const
{
    return gnutls_certificate_type_get( this->s);
}

void session::set_private_extensions ( bool allow)
{
    gnutls_handshake_set_private_extensions( this->s, (int)allow);
}

gnutls_handshake_description_t session::get_handshake_last_out() const
{
    return gnutls_handshake_get_last_out( this->s);
}

gnutls_handshake_description_t session::get_handshake_last_in() const
{
    return gnutls_handshake_get_last_in( this->s);
}

ssize_t session::send (const void *data, size_t sizeofdata)
{
    return RETWRAP_NET(gnutls_record_send( this->s, data, sizeofdata));
}

ssize_t session::recv (void *data, size_t sizeofdata)
{
    return RETWRAP_NET(gnutls_record_recv( this->s, data, sizeofdata));
}

bool session::get_record_direction() const
{
    return gnutls_record_get_direction(this->s);
}

        // maximum packet size
size_t session::get_max_size () const
{
    return gnutls_record_get_max_size( this->s);
}

void session::set_max_size(size_t size)
{
    RETWRAP( gnutls_record_set_max_size( this->s, size));
}

size_t session::check_pending () const
{
    return gnutls_record_check_pending( this->s);
}


void session::prf (size_t label_size, const char *label,
                 int server_random_first,
                 size_t extra_size, const char *extra,
                 size_t outsize, char *out)
{
    RETWRAP(gnutls_prf( this->s, label_size, label, server_random_first,
            extra_size, extra, outsize, out));
}

void session::prf_raw ( size_t label_size, const char *label,
                      size_t seed_size, const char *seed,
                      size_t outsize, char *out)
{
    RETWRAP( gnutls_prf_raw( this->s, label_size, label, seed_size, seed, outsize, out));
}


void session::set_cipher_priority (const int *list)
{
    RETWRAP( gnutls_cipher_set_priority( this->s, list));
}

void session::set_mac_priority (const int *list)
{
    RETWRAP( gnutls_mac_set_priority( this->s, list));
}

void session::set_compression_priority (const int *list)
{
    RETWRAP( gnutls_compression_set_priority( this->s, list));
}

void session::set_kx_priority (const int *list)
{
    RETWRAP( gnutls_kx_set_priority( this->s, list));
}

void session::set_protocol_priority (const int *list)
{
    RETWRAP( gnutls_protocol_set_priority( this->s, list));
}

void session::set_certificate_type_priority (const int *list)
{
    RETWRAP( gnutls_certificate_type_set_priority( this->s, list));
}


/* if you just want some defaults, use the following.
 */
void session::set_default_priority(priority_flag flag)
{
    if (flag == EXPORT_CIPHERS)
        RETWRAP( gnutls_set_default_export_priority( this->s));
    else
        RETWRAP( gnutls_set_default_priority( this->s));
}

gnutls_protocol_t session::get_protocol_version() const
{
    return gnutls_protocol_get_version( this->s);
}

void session::set_data ( const void *session_data,
                        size_t session_data_size)
{
    RETWRAP(gnutls_session_set_data( this->s, session_data, session_data_size));
}

void session::get_data (void *session_data,
                        size_t * session_data_size) const
{
    RETWRAP(gnutls_session_get_data( this->s, session_data, session_data_size));
}

void session::get_data(gnutls_session_t session,
                       gnutls_datum_t & data) const
{
    RETWRAP(gnutls_session_get_data2( this->s, &data));

}

void session::get_id ( void *session_id,
                       size_t * session_id_size) const
{
    RETWRAP( gnutls_session_get_id( this->s, session_id, session_id_size));
}

bool session::is_resumed() const
{
    int ret = gnutls_session_is_resumed( this->s);
    
    if (ret != 0) return true;
    return false;
}


bool session::get_peers_certificate(std::vector<gnutls_datum_t> &out_certs) const
{
    const gnutls_datum_t *certs;
    unsigned int certs_size;
    
    certs = gnutls_certificate_get_peers (this->s, &certs_size);
    
    if (certs==NULL) return false;
    
    for(unsigned int i=0;i<certs_size;i++)
        out_certs.push_back( certs[i]);
    
    return true;
}

bool session::get_peers_certificate(const gnutls_datum_t** certs, unsigned int *certs_size) const
{
    *certs = gnutls_certificate_get_peers (this->s, certs_size);
    
    if (*certs==NULL) return false;
    return true;
}

void session::get_our_certificate(gnutls_datum& cert) const
{
const gnutls_datum_t *d;
    
    d = gnutls_certificate_get_ours(this->s);
    if (d==NULL)
        throw(exception( GNUTLS_E_INVALID_REQUEST));
    cert = *d;
}
          
time_t session::get_peers_certificate_activation_time() const
{
    return gnutls_certificate_activation_time_peers( this->s);
}

time_t session::get_peers_certificate_expiration_time() const
{
    return gnutls_certificate_expiration_time_peers( this->s);
}
void session::verify_peers_certificate( unsigned int& status) const
{
    RETWRAP( gnutls_certificate_verify_peers2( this->s, &status));
}


client_session::client_session() : session( GNUTLS_CLIENT)
{
}

// client session
void client_session::set_server_name (gnutls_server_name_type_t type,
                                      const void *name, size_t name_length)
{
    RETWRAP( gnutls_server_name_set( this->s, type, name, name_length));
}

bool client_session::get_request_status()
{
    return RETWRAP(gnutls_certificate_client_get_request_status (this->s));
}

// server_session
void server_session::get_server_name (void *data, size_t * data_length,
                                      unsigned int *type, unsigned int indx) const
{
    RETWRAP( gnutls_server_name_get( this->s, data, data_length, type, indx));
}

// internal DB stuff
static int store_function(void *_db, gnutls_datum_t key, gnutls_datum_t data)
{
    try {
        DB* db = static_cast<DB*>(_db);
    
        if (db->store( key, data)==false) return -1;
    } catch(...) {
        return -1;
    }

    return 0;
}

const static gnutls_datum_t null_datum = { NULL, 0 };

static gnutls_datum_t retrieve_function(void *_db, gnutls_datum_t key)
{
    gnutls_datum_t data;
    
    try {
        DB* db = static_cast<DB*>(_db);
    
        if (db->retrieve( key, data)==false) return null_datum;
    
    } catch(...) {
        return null_datum;
    }

    return data;
}

static int remove_function(void *_db, gnutls_datum_t key)
{
    try {
        DB* db = static_cast<DB*>(_db);
    
        if (db->remove( key)==false) return -1;
    } catch(...) {
        return -1;
    }

    return 0;
}

void server_session::set_db( const DB& db)
{
    gnutls_db_set_ptr( this->s, const_cast<DB*>(&db));
    gnutls_db_set_store_function( this->s, store_function);
    gnutls_db_set_retrieve_function( this->s, retrieve_function);
    gnutls_db_set_remove_function( this->s, remove_function);
}

void server_session::set_db_cache_expiration (unsigned int seconds)
{
    gnutls_db_set_cache_expiration( this->s, seconds);
}

void server_session::db_remove () const
{
    gnutls_db_remove_session( this->s);
}

bool server_session::db_check_entry ( gnutls_datum_t &session_data) const
{
    int ret = gnutls_db_check_entry( this->s, session_data);

    if (ret != 0) return true;
    return false;
}

void session::set_max_handshake_packet_length ( size_t max)
{
    gnutls_handshake_set_max_packet_length( this->s, max);
}

void session::clear_credentials()
{
    gnutls_credentials_clear( this->s);
}

void session::set_credentials( credentials &cred)
{
    RETWRAP(gnutls_credentials_set( this->s, cred.get_type(), cred.ptr()));
}

const char* server_session::get_srp_username() const
{
    return gnutls_srp_server_get_username( this->s);
}

const char* server_session::get_psk_username() const
{
    return gnutls_psk_server_get_username( this->s);
}


void session::set_transport_ptr( gnutls_transport_ptr_t ptr)
{
    gnutls_transport_set_ptr( this->s, ptr);
}
  
void session::set_transport_ptr( gnutls_transport_ptr_t recv_ptr, gnutls_transport_ptr_t send_ptr)
{
    gnutls_transport_set_ptr2( this->s, recv_ptr, send_ptr);
}


gnutls_transport_ptr_t session::get_transport_ptr () const
{
    return gnutls_transport_get_ptr (this->s);
}
  
void session::get_transport_ptr( gnutls_transport_ptr_t & recv_ptr,
                                 gnutls_transport_ptr_t & send_ptr) const
{
    gnutls_transport_get_ptr2 (this->s, &recv_ptr, &send_ptr);
}

void session::set_transport_lowat( size_t num)
{
    gnutls_transport_set_lowat (this->s, num);
}

void session::set_transport_push_function( gnutls_push_func push_func)
{
    gnutls_transport_set_push_function ( this->s,  push_func);
}
  
void session::set_transport_pull_function( gnutls_pull_func pull_func)
{
    gnutls_transport_set_pull_function ( this->s,  pull_func);
}

void session::set_user_ptr( void* ptr)
{
    gnutls_session_set_ptr( this->s, ptr);
}

void* session::get_user_ptr( ) const
{
    return gnutls_session_get_ptr(this->s);
}
  
void session::send_openpgp_key( gnutls_openpgp_key_status_t status)
{
    gnutls_openpgp_send_key(this->s, status);
}


void session::set_dh_prime_bits( unsigned int bits)
{
    gnutls_dh_set_prime_bits( this->s, bits);
}

unsigned int session::get_dh_secret_bits() const
{
    return RETWRAP( gnutls_dh_get_secret_bits( this->s));
}

unsigned int session::get_dh_peers_public_bits() const
{
    return RETWRAP(gnutls_dh_get_peers_public_bits( this->s));
}

unsigned int session::get_dh_prime_bits() const
{
    return RETWRAP( gnutls_dh_get_prime_bits( this->s));
}

void session::get_dh_group( gnutls_datum_t & gen, gnutls_datum_t & prime) const
{
    RETWRAP( gnutls_dh_get_group( this->s, &gen, &prime));
}

void session::get_dh_pubkey( gnutls_datum_t & raw_key) const
{
    RETWRAP(gnutls_dh_get_pubkey( this->s, &raw_key));
}

void session::get_rsa_export_pubkey( gnutls_datum& exponent, gnutls_datum& modulus) const
{
    RETWRAP( gnutls_rsa_export_get_pubkey( this->s, &exponent, &modulus));
}

unsigned int session::get_rsa_export_modulus_bits() const
{
    return RETWRAP(gnutls_rsa_export_get_modulus_bits( this->s));
}

void server_session::set_certificate_request( gnutls_certificate_request_t req)
{
    gnutls_certificate_server_set_request (this->s, req);
}




gnutls_credentials_type_t session::get_auth_type() const
{
    return gnutls_auth_get_type( this->s);
}

gnutls_credentials_type_t session::get_server_auth_type() const
{
    return gnutls_auth_server_get_type( this->s);
}

gnutls_credentials_type_t session::get_client_auth_type() const
{
    return gnutls_auth_client_get_type( this->s);
}


void* certificate_credentials::ptr() const
{
    return this->cred;
}

void certificate_credentials::set_ptr(void* p)
{
    this->cred = static_cast<gnutls_certificate_credentials_t> (p);
}

certificate_credentials::~certificate_credentials()
{
    gnutls_certificate_free_credentials (this->cred); 
}

certificate_credentials::certificate_credentials() : credentials(GNUTLS_CRD_CERTIFICATE)
{
    RETWRAP(gnutls_certificate_allocate_credentials ( &this->cred));
}

void certificate_server_credentials::set_params_function( gnutls_params_function* func)
{
    gnutls_certificate_set_params_function( this->cred, func);
}

anon_server_credentials::anon_server_credentials() : credentials(GNUTLS_CRD_ANON)
{ 
    RETWRAP(gnutls_anon_allocate_server_credentials( &this->cred));
}
        
anon_server_credentials::~anon_server_credentials() 
{ 
    gnutls_anon_free_server_credentials( this->cred); 
}

void anon_server_credentials::set_dh_params( const dh_params& params) 
{
    gnutls_anon_set_server_dh_params (this->cred, params.get_params_t());
}

void anon_server_credentials::set_params_function ( gnutls_params_function * func)
{
    gnutls_anon_set_server_params_function ( this->cred, func); 
}

anon_client_credentials::anon_client_credentials() : credentials(GNUTLS_CRD_ANON)
{ 
    RETWRAP(gnutls_anon_allocate_client_credentials( &this->cred));
}
        
anon_client_credentials::~anon_client_credentials() 
{ 
    gnutls_anon_free_client_credentials( this->cred); 
}

void certificate_credentials::free_keys ()
{
    gnutls_certificate_free_keys( this->cred);
}

void certificate_credentials::free_cas ()
{
    gnutls_certificate_free_cas( this->cred);
}

void certificate_credentials::free_ca_names ()
{
    gnutls_certificate_free_ca_names( this->cred);
}

void certificate_credentials::free_crls ()
{
    gnutls_certificate_free_crls( this->cred);
}


void certificate_credentials::set_dh_params ( const dh_params& params)
{
    gnutls_certificate_set_dh_params( this->cred, params.get_params_t());
}

void certificate_credentials::set_rsa_export_params ( const rsa_params & params)
{
    gnutls_certificate_set_rsa_export_params( this->cred, params.get_params_t());
}

void certificate_credentials::set_verify_flags ( unsigned int flags)
{
    gnutls_certificate_set_verify_flags( this->cred, flags);
}

void certificate_credentials::set_verify_limits ( unsigned int max_bits, unsigned int max_depth)
{
    gnutls_certificate_set_verify_limits( this->cred, max_bits, max_depth);
}

void certificate_credentials::set_x509_trust_file(const char *cafile, gnutls_x509_crt_fmt_t type)
{
    RETWRAP( gnutls_certificate_set_x509_trust_file( this->cred, cafile, type));
}

void certificate_credentials::set_x509_trust(const gnutls_datum_t & CA, gnutls_x509_crt_fmt_t type)
{
    RETWRAP( gnutls_certificate_set_x509_trust_mem( this->cred, &CA, type));
}


void certificate_credentials::set_x509_crl_file( const char *crlfile, gnutls_x509_crt_fmt_t type)
{
    RETWRAP( gnutls_certificate_set_x509_crl_file( this->cred, crlfile, type));
}

void certificate_credentials::set_x509_crl(const gnutls_datum_t & CRL, gnutls_x509_crt_fmt_t type)
{
    RETWRAP( gnutls_certificate_set_x509_crl_mem( this->cred, &CRL, type));
}

void certificate_credentials::set_x509_key_file(const char *certfile, const char *keyfile, gnutls_x509_crt_fmt_t type)
{
    RETWRAP( gnutls_certificate_set_x509_key_file( this->cred, certfile, keyfile, type));
}

void certificate_credentials::set_x509_key(const gnutls_datum_t & CERT, const gnutls_datum_t & KEY, gnutls_x509_crt_fmt_t type)
{
    RETWRAP( gnutls_certificate_set_x509_key_mem( this->cred, &CERT, &KEY, type));
}

void certificate_credentials::set_simple_pkcs12_file( const char *pkcs12file,
                                     gnutls_x509_crt_fmt_t type, const char *password)
{
    RETWRAP( gnutls_certificate_set_x509_simple_pkcs12_file( this->cred, pkcs12file, type, password));
}

void certificate_credentials::set_x509_key ( gnutls_x509_crt_t * cert_list, int cert_list_size,
                            gnutls_x509_privkey_t key)
{
    RETWRAP( gnutls_certificate_set_x509_key( this->cred, cert_list, cert_list_size, key));
}

void certificate_credentials::set_x509_trust ( gnutls_x509_crt_t * ca_list, int ca_list_size)
{
    RETWRAP( gnutls_certificate_set_x509_trust( this->cred, ca_list, ca_list_size));
}

void certificate_credentials::set_x509_crl ( gnutls_x509_crl_t * crl_list, int crl_list_size)
{
    RETWRAP( gnutls_certificate_set_x509_crl( this->cred, crl_list, crl_list_size));
}

void certificate_server_credentials::set_retrieve_function( gnutls_certificate_server_retrieve_function* func)
{
    gnutls_certificate_server_set_retrieve_function( this->cred, func);
}

void certificate_client_credentials::set_retrieve_function( gnutls_certificate_client_retrieve_function* func)
{
    gnutls_certificate_client_set_retrieve_function( this->cred, func);
}

// SRP

srp_server_credentials::srp_server_credentials() : credentials(GNUTLS_CRD_SRP)
{ 
    RETWRAP(gnutls_srp_allocate_server_credentials( &this->cred));
}
        
srp_server_credentials::~srp_server_credentials() 
{ 
    gnutls_srp_free_server_credentials( this->cred); 
}

void* srp_server_credentials::ptr() const
{
    return this->cred;
}

void srp_server_credentials::set_ptr(void* p)
{
    this->cred = static_cast<gnutls_srp_server_credentials_t> (p);
}

srp_client_credentials::srp_client_credentials() : credentials(GNUTLS_CRD_SRP)
{ 
    RETWRAP(gnutls_srp_allocate_client_credentials( &this->cred));
}
        
srp_client_credentials::~srp_client_credentials() 
{ 
    gnutls_srp_free_client_credentials( this->cred); 
}

void* srp_client_credentials::ptr() const
{
    return this->cred;
}

void srp_client_credentials::set_ptr(void* p)
{
    this->cred = static_cast<gnutls_srp_client_credentials_t> (p);
}

void srp_client_credentials::set_credentials( const char* username, const char* password)
{
    RETWRAP(gnutls_srp_set_client_credentials (this->cred, username, password));
}

void srp_server_credentials::set_credentials_file (
    const char *password_file, const char *password_conf_file)
{
    RETWRAP( gnutls_srp_set_server_credentials_file( this->cred, password_file, password_conf_file));
}


void srp_server_credentials::set_credentials_function(gnutls_srp_server_credentials_function * func)
{
    gnutls_srp_set_server_credentials_function( this->cred, func);
}

void srp_client_credentials::set_credentials_function(gnutls_srp_client_credentials_function * func)
{
    gnutls_srp_set_client_credentials_function( this->cred, func);
}

// PSK

psk_server_credentials::psk_server_credentials() : credentials(GNUTLS_CRD_PSK)
{ 
    RETWRAP(gnutls_psk_allocate_server_credentials( &this->cred));
}
        
psk_server_credentials::~psk_server_credentials() 
{ 
    gnutls_psk_free_server_credentials( this->cred); 
}

void* psk_server_credentials::ptr() const
{
    return this->cred;
}

void psk_server_credentials::set_ptr(void* p)
{
    this->cred = static_cast<gnutls_psk_server_credentials_t> (p);
}

void psk_server_credentials::set_credentials_file(const char* password_file)
{ 
    RETWRAP(gnutls_psk_set_server_credentials_file( this->cred, password_file));
}

void psk_server_credentials::set_credentials_function(gnutls_psk_server_credentials_function * func)
{
    gnutls_psk_set_server_credentials_function( this->cred, func);
}

void psk_server_credentials::set_dh_params( const dh_params &params)
{
    gnutls_psk_set_server_dh_params( this->cred, params.get_params_t());
}

void psk_server_credentials::set_params_function(gnutls_params_function * func)
{
    gnutls_psk_set_server_params_function (this->cred, func);
}



psk_client_credentials::psk_client_credentials() : credentials(GNUTLS_CRD_PSK)
{ 
    RETWRAP(gnutls_psk_allocate_client_credentials( &this->cred));
}
        
psk_client_credentials::~psk_client_credentials() 
{ 
    gnutls_psk_free_client_credentials( this->cred); 
}

void* psk_client_credentials::ptr() const
{
    return this->cred;
}

void psk_client_credentials::set_ptr(void* p)
{
    this->cred = static_cast<gnutls_psk_client_credentials_t> (p);
}

void psk_client_credentials::set_credentials(const char* username,
        const gnutls_datum& key, gnutls_psk_key_flags flags) 
{ 
    RETWRAP(gnutls_psk_set_client_credentials( this->cred, username, &key, flags));
}

void psk_client_credentials::set_credentials_function(gnutls_psk_client_credentials_function * func)
{
    gnutls_psk_set_client_credentials_function( this->cred, func);
}


credentials::credentials(gnutls_credentials_type_t t) : type(t) 
{ 
}

credentials::credentials( credentials& c)
{
    this->type = c.type;
    this->set_ptr( c.ptr());
}

gnutls_credentials_type_t credentials::get_type() const
{ 
    return type; 
}
        


          

exception::exception( int x) 
{ 
    retcode = x; 
}

int exception::get_code()
{
    return retcode;
}

const char* exception::what() const throw()
{ 
    return gnutls_strerror(retcode); 
}




dh_params::dh_params()
{
    RETWRAP(gnutls_dh_params_init( &params));
}

dh_params::~dh_params()
{
    gnutls_dh_params_deinit(params);
}

void dh_params::import_raw( const gnutls_datum_t & prime,
                     const gnutls_datum_t & generator)
{
    RETWRAP( gnutls_dh_params_import_raw( params, &prime, &generator));
}

void dh_params::import_pkcs3( const gnutls_datum_t & pkcs3_params,
                              gnutls_x509_crt_fmt_t format)
{
    RETWRAP(gnutls_dh_params_import_pkcs3( params, &pkcs3_params, format));
}

void dh_params::generate( unsigned int bits)
{
    RETWRAP(gnutls_dh_params_generate2( params, bits));
}
        
void dh_params::export_pkcs3( gnutls_x509_crt_fmt_t format, unsigned char *params_data, size_t * params_data_size)
{
    RETWRAP( gnutls_dh_params_export_pkcs3( params, format, params_data, params_data_size));
}

void dh_params::export_raw( gnutls_datum_t& prime, gnutls_datum_t &generator)
{
    RETWRAP( gnutls_dh_params_export_raw( params, &prime, &generator, NULL));
}

gnutls_dh_params_t dh_params::get_params_t() const
{
    return params;
}

dh_params & dh_params::operator=(const dh_params& src)
{
    dh_params* dst = new dh_params;
    int ret;
    
    ret = gnutls_dh_params_cpy( dst->params, src.params);
    
    if (ret < 0) {
        delete dst;
        throw(ret);
    }
    
    return *dst;
}


// RSA

rsa_params::rsa_params()
{
    RETWRAP(gnutls_rsa_params_init( &params));
}

rsa_params::~rsa_params()
{
    gnutls_rsa_params_deinit(params);
}

void rsa_params::import_pkcs1( const gnutls_datum_t & pkcs1_params,
                              gnutls_x509_crt_fmt_t format)
{
    RETWRAP(gnutls_rsa_params_import_pkcs1( params, &pkcs1_params, format));
}

void rsa_params::generate( unsigned int bits)
{
    RETWRAP(gnutls_rsa_params_generate2( params, bits));
}
        
void rsa_params::export_pkcs1( gnutls_x509_crt_fmt_t format, unsigned char *params_data, size_t * params_data_size)
{
    RETWRAP( gnutls_rsa_params_export_pkcs1( params, format, params_data, params_data_size));
}

gnutls_rsa_params_t rsa_params::get_params_t() const
{
    return params;
}

rsa_params & rsa_params::operator=(const rsa_params& src)
{
    rsa_params* dst = new rsa_params;
    int ret;
    
    ret = gnutls_rsa_params_cpy( dst->params, src.params);
    
    if (ret < 0)
        delete dst;
    throw(ret);
    
    return *dst;
}
  
void rsa_params::import_raw( const gnutls_datum_t & m,
                         const gnutls_datum_t & e,
                         const gnutls_datum_t & d,
                         const gnutls_datum_t & p,
                         const gnutls_datum_t & q,
                         const gnutls_datum_t & u)
{
    
    RETWRAP(gnutls_rsa_params_import_raw ( params, &m, &e, &d, &p, &q, &u));
}
  

void rsa_params::export_raw( gnutls_datum_t & m, gnutls_datum_t & e,
                      gnutls_datum_t & d, gnutls_datum_t & p,
                      gnutls_datum_t & q, gnutls_datum_t & u)
{
    RETWRAP( gnutls_rsa_params_export_raw ( params, &m, &e, &d, &p, &q, &u, NULL));
}
