/* iksemel (XML parser for Jabber)
** Copyright (C) 2000-2004 Gurer Ozen <madcat@e-kolay.net>
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#include "common.h"
#include "iksemel.h"

#ifdef HAVE_GNUTLS
#include <gnutls/gnutls.h>
#endif

#define SF_FOREIGN 1
#define SF_TRY_SECURE 2
#define SF_SECURE 4

struct stream_data {
	iksparser *prs;
	ikstack *s;
	ikstransport *trans;
	char *name_space;
	void *user_data;
	const char *server;
	iksStreamHook *streamHook;
	iksLogHook *logHook;
	iks *current;
	char *buf;
	void *sock;
	unsigned int flags;
	char *auth_username;
	char *auth_pass;
#ifdef HAVE_GNUTLS
	gnutls_session sess;
	gnutls_certificate_credentials cred;
#endif
};

#ifdef HAVE_GNUTLS

static size_t
tls_push (iksparser *prs, const char *buffer, size_t len)
{
	struct stream_data *data = iks_user_data (prs);
	int ret;

	ret = data->trans->send (data->sock, buffer, len);
	if (ret) return (size_t) -1;
	return len;
}

static size_t
tls_pull (iksparser *prs, char *buffer, size_t len)
{
	struct stream_data *data = iks_user_data (prs);
	int ret;

	ret = data->trans->recv (data->sock, buffer, len, -1);
	if (ret == -1) return (size_t) -1;
	return ret;
}

static int
handshake (struct stream_data *data)
{
	const int protocol_priority[] = { GNUTLS_TLS1, GNUTLS_SSL3, 0 };
	const int kx_priority[] = { GNUTLS_KX_RSA, 0 };
	const int cipher_priority[] = { GNUTLS_CIPHER_3DES_CBC, GNUTLS_CIPHER_ARCFOUR, 0};
	const int comp_priority[] = { GNUTLS_COMP_ZLIB, GNUTLS_COMP_NULL, 0 };
	const int mac_priority[] = { GNUTLS_MAC_SHA, GNUTLS_MAC_MD5, 0 };
	int ret;

	if (gnutls_global_init () != 0)
		return IKS_NOMEM;

	if (gnutls_certificate_allocate_credentials (&data->cred) < 0)
		return IKS_NOMEM;

	if (gnutls_init (&data->sess, GNUTLS_CLIENT) != 0) {
		gnutls_certificate_free_credentials (data->cred);
		return IKS_NOMEM;
	}
	gnutls_protocol_set_priority (data->sess, protocol_priority);
	gnutls_cipher_set_priority(data->sess, cipher_priority);
	gnutls_compression_set_priority(data->sess, comp_priority);
	gnutls_kx_set_priority(data->sess, kx_priority);
	gnutls_mac_set_priority(data->sess, mac_priority);
	gnutls_credentials_set (data->sess, GNUTLS_CRD_CERTIFICATE, data->cred);

	gnutls_transport_set_push_function (data->sess, (gnutls_push_func) tls_push);
	gnutls_transport_set_pull_function (data->sess, (gnutls_pull_func) tls_pull);
	gnutls_transport_set_ptr (data->sess, data->prs);

	ret = gnutls_handshake (data->sess);
	if (ret != 0) {
		gnutls_deinit (data->sess);
		gnutls_certificate_free_credentials (data->cred);
		return IKS_NET_TLSFAIL;
	}

	data->flags &= (~SF_TRY_SECURE);
	data->flags |= SF_SECURE;

	iks_send_header (data->prs, data->server);

	return IKS_OK;
}
#endif

static void
insert_attribs (iks *x, char **atts)
{
	if (atts) {
		int i = 0;
		while (atts[i]) {
			iks_insert_attrib (x, atts[i], atts[i+1]);
			i += 2;
		}
	}
}

#define CNONCE_LEN 4

static void
iks_sasl_challenge (struct stream_data *data, iks *challenge)
{
	char *b;
	iks *x;

	b = iks_cdata (iks_child (challenge));
	if (!b) return;

	b = iks_base64_decode (b);

	if (strstr (b, "rspauth")) {
		x = iks_new("response");
	} else {
		char *realm, *nonce, *charset = NULL, *algorithm = NULL;
		char *realm_end, *nonce_end, cnonce[CNONCE_LEN*8 + 1];
		unsigned char a1_h[16], a1[33], a2[33], response_value[33];
		char *response, *response_coded;
		iksmd5 *md5;
		int i;

		realm = strstr (b, "realm");
		if (realm) {
			realm += 7;
			do {
				realm_end = strchr(realm, '"');
			} while (*(realm_end - 1) == '\\');
		} else {
			realm_end = 0;
			realm = (char *) data->server;
		}
		nonce = strstr (b, "nonce");
		if (nonce) {
			nonce += 7;
			do {
				nonce_end = strchr (nonce, '"');
			} while (*(nonce - 1) == '\\');
		} else {
			iks_free (b);
			return;
		}
		charset = strstr (b, "charset");
		if (charset) charset += 8;
		algorithm = strstr (b, "algorithm");
		if (algorithm) algorithm += 10;
		if (realm_end) *realm_end = 0;
		*nonce_end = 0;

		for (i = 0; i < CNONCE_LEN; ++i)
			sprintf (cnonce + i*8, "%08x", rand());

		md5 = iks_md5_new();
		iks_md5_hash (md5, data->auth_username, iks_strlen (data->auth_username), 0);
		iks_md5_hash (md5, ":", 1, 0);
		iks_md5_hash (md5, realm, iks_strlen (realm), 0);
		iks_md5_hash (md5, ":", 1, 0);
		iks_md5_hash (md5, data->auth_pass, iks_strlen (data->auth_pass), 1);
		iks_md5_digest (md5, a1_h);
		iks_md5_reset (md5);
		iks_md5_hash (md5, a1_h, 16, 0);
		iks_md5_hash (md5, ":", 1, 0);
		iks_md5_hash (md5, nonce, iks_strlen (nonce), 0);
		iks_md5_hash (md5, ":", 1, 0);
		iks_md5_hash (md5, cnonce, iks_strlen (cnonce), 1);
		iks_md5_print (md5, a1);
		iks_md5_reset (md5);
		iks_md5_hash (md5, "AUTHENTICATE:xmpp/", 18, 0);
		iks_md5_hash (md5, data->server, iks_strlen (data->server), 1);
		iks_md5_print (md5, a2);
		iks_md5_reset (md5);
		iks_md5_hash (md5, a1, 32, 0);
		iks_md5_hash (md5, ":", 1, 0);
		iks_md5_hash (md5, nonce, iks_strlen (nonce), 0);
		iks_md5_hash (md5, ":00000001:", 10, 0);
		iks_md5_hash (md5, cnonce, iks_strlen (cnonce), 0);
		iks_md5_hash (md5, ":auth:", 6, 0);
		iks_md5_hash (md5, a2, 32, 1);
		iks_md5_print (md5, response_value);
		iks_md5_delete (md5);

		i = iks_strlen (data->auth_username) + iks_strlen (realm) +
			iks_strlen (nonce) + iks_strlen (data->server) +
			CNONCE_LEN*8 + 136;
		response = iks_malloc (i);
		sprintf (response, "username=\"%s\",realm=\"%s\",nonce=\"%s\""
			",cnonce=\"%s\",nc=00000001,qop=auth,digest-uri=\""
			"xmpp/%s\",response=%s,charset=utf-8",
			data->auth_username, realm, nonce, cnonce,
			data->server, response_value);
		response_coded = iks_base64_encode (response, 0);

		x = iks_new ("response");
		iks_insert_cdata (x, response_coded, 0);

		iks_free (response);
		iks_free (response_coded);
	}
	iks_insert_attrib (x, "xmlns", IKS_NS_XMPP_SASL);
	iks_send (data->prs, x);
	iks_delete (x);
	iks_free (b);
}

static int
tagHook (struct stream_data *data, char *name, char **atts, int type)
{
	iks *x;
	int err;

	switch (type) {
		case IKS_OPEN:
		case IKS_SINGLE:
#ifdef HAVE_GNUTLS
			if (data->flags & SF_TRY_SECURE) {
				if (strcmp (name, "proceed") == 0) {
					err = handshake (data);
					return err;
				} else if (strcmp (name, "failure") == 0){
					return IKS_NET_TLSFAIL;
				}
			}
#endif
			if (data->current) {
				x = iks_insert (data->current, name);
				insert_attribs (x, atts);
			} else {
				x = iks_new (name);
				insert_attribs (x, atts);
				if (iks_strcmp (name, "stream:stream") == 0) {
					err = data->streamHook (data->user_data, IKS_NODE_START, x);
					if (err != IKS_OK) return err;
					break;
				}
			}
			data->current = x;
			if (IKS_OPEN == type) break;
		case IKS_CLOSE:
			x = data->current;
			if (NULL == x) {
				err = data->streamHook (data->user_data, IKS_NODE_STOP, NULL);
				if (err != IKS_OK) return err;
				break;
			}
			if (NULL == iks_parent (x)) {
				data->current = NULL;
				if (iks_strcmp (name, "challenge") == 0)
					iks_sasl_challenge(data, x);
				else if (iks_strcmp (name, "stream:error") == 0) {
					err = data->streamHook (data->user_data, IKS_NODE_ERROR, x);
					if (err != IKS_OK) return err;
				} else {
					err = data->streamHook (data->user_data, IKS_NODE_NORMAL, x);
					if (err != IKS_OK) return err;
				}
				break;
			}
			data->current = iks_parent (x);
	}
	return IKS_OK;
}

static int
cdataHook (struct stream_data *data, char *cdata, size_t len)
{
	if (data->current) iks_insert_cdata (data->current, cdata, len);
	return IKS_OK;
}

static void
deleteHook (struct stream_data *data)
{
#ifdef HAVE_GNUTLS
	if (data->flags & SF_SECURE) {
		gnutls_bye (data->sess, GNUTLS_SHUT_WR);
		gnutls_deinit (data->sess);
		gnutls_certificate_free_credentials (data->cred);
	}
#endif
	if (data->trans) data->trans->close (data->sock);
	data->trans = NULL;
	if (data->current) iks_delete (data->current);
	data->current = NULL;
	data->flags = 0;
}

iksparser *
iks_stream_new (char *name_space, void *user_data, iksStreamHook *streamHook)
{
	ikstack *s;
	struct stream_data *data;

	s = iks_stack_new (DEFAULT_STREAM_CHUNK_SIZE, 0);
	if (NULL == s) return NULL;
	data = iks_stack_alloc (s, sizeof (struct stream_data));
	memset (data, 0, sizeof (struct stream_data));
	data->s = s;
	data->prs = iks_sax_extend (s, data, (iksTagHook *)tagHook, (iksCDataHook *)cdataHook, (iksDeleteHook *)deleteHook);
	data->name_space = name_space;
	data->user_data = user_data;
	data->streamHook = streamHook;
	return data->prs;
}

void *
iks_stream_user_data (iksparser *prs)
{
	struct stream_data *data = iks_user_data (prs);

	return data->user_data;
}

void
iks_set_log_hook (iksparser *prs, iksLogHook *logHook)
{
	struct stream_data *data = iks_user_data (prs);

	data->logHook = logHook;
}

int
iks_connect_tcp (iksparser *prs, const char *server, int port)
{
#ifdef USE_DEFAULT_IO
	return iks_connect_with (prs, server, port, server, &iks_default_transport);
#else
	return IKS_NET_NOTSUPP;
#endif
}

int
iks_connect_via (iksparser *prs, const char *server, int port, const char *server_name)
{
#ifdef USE_DEFAULT_IO
	return iks_connect_with (prs, server, port, server_name, &iks_default_transport);
#else
	return IKS_NET_NOTSUPP;
#endif
}

int
iks_connect_with (iksparser *prs, const char *server, int port, const char *server_name, ikstransport *trans)
{
	struct stream_data *data = iks_user_data (prs);
	int ret;

	if (!trans->connect) return IKS_NET_NOTSUPP;

	if (!data->buf) {
		data->buf = iks_stack_alloc (data->s, NET_IO_BUF_SIZE);
		if (NULL == data->buf) return IKS_NOMEM;
	}

	ret = trans->connect (prs, &data->sock, server, port);
	if (ret) return ret;

	data->trans = trans;

	return iks_send_header (prs, server_name);
}

int
iks_connect_async (iksparser *prs, const char *server, int port, void *notify_data, iksAsyncNotify *notify_func)
{
#ifdef USE_DEFAULT_IO
	return iks_connect_async_with (prs, server, port, server, &iks_default_transport, notify_data, notify_func);
#else
	return IKS_NET_NOTSUPP;
#endif
}

int
iks_connect_async_with (iksparser *prs, const char *server, int port, const char *server_name, ikstransport *trans, void *notify_data, iksAsyncNotify *notify_func)
{
	struct stream_data *data = iks_user_data (prs);
	int ret;

	if (NULL == trans->connect_async)
		return IKS_NET_NOTSUPP;

	if (!data->buf) {
		data->buf = iks_stack_alloc (data->s, NET_IO_BUF_SIZE);
		if (NULL == data->buf) return IKS_NOMEM;
	}

	ret = trans->connect_async (prs, &data->sock, server, server_name, port, notify_data, notify_func);
	if (ret) return ret;

	data->trans = trans;
	data->server = server_name;

	return IKS_OK;
}

int
iks_connect_fd (iksparser *prs, int fd)
{
#ifdef USE_DEFAULT_IO
	struct stream_data *data = iks_user_data (prs);

	if (!data->buf) {
		data->buf = iks_stack_alloc (data->s, NET_IO_BUF_SIZE);
		if (NULL == data->buf) return IKS_NOMEM;
	}

	data->sock = (void *) fd;
	data->flags |= SF_FOREIGN;
	data->trans = &iks_default_transport;

	return IKS_OK;
#else
	return IKS_NET_NOTSUPP;
#endif
}

int
iks_fd (iksparser *prs)
{
	struct stream_data *data = iks_user_data (prs);

	return (int) data->sock;
}

int
iks_recv (iksparser *prs, int timeout)
{
	struct stream_data *data = iks_user_data (prs);
	int len, ret;

	while (1) {
#ifdef HAVE_GNUTLS
		if (data->flags & SF_SECURE) {
			len = gnutls_record_recv (data->sess, data->buf, NET_IO_BUF_SIZE - 1);
		} else
#endif
		{
			len = data->trans->recv (data->sock, data->buf, NET_IO_BUF_SIZE - 1, timeout);
		}
		if (len < 0) return IKS_NET_RWERR;
		if (len == 0) break;
		data->buf[len] = '\0';
		if (data->logHook) data->logHook (data->user_data, data->buf, len, 1);
		ret = iks_parse (prs, data->buf, len, 0);
		if (ret != IKS_OK) return ret;
		if (!data->trans) {
			/* stream hook called iks_disconnect */
			return IKS_NET_NOCONN;
		}
		timeout = 0;
	}
	return IKS_OK;
}

int
iks_send_header (iksparser *prs, const char *to)
{
	struct stream_data *data = iks_user_data (prs);
	char *msg;
	int len, err;

	len = 91 + strlen (data->name_space) + 6 + strlen (to) + 16 + 1;
	msg = iks_malloc (len);
	if (!msg) return IKS_NOMEM;
	sprintf (msg, "<?xml version='1.0'?>"
		"<stream:stream xmlns:stream='http://etherx.jabber.org/streams' xmlns='"
		"%s' to='%s' version='1.0'>", data->name_space, to);
	err = iks_send_raw (prs, msg);
	iks_free (msg);
	if (err) return err;
	data->server = to;
	return IKS_OK;
}

int
iks_send (iksparser *prs, iks *x)
{
	return iks_send_raw (prs, iks_string (iks_stack (x), x));
}

int
iks_send_raw (iksparser *prs, const char *xmlstr)
{
	struct stream_data *data = iks_user_data (prs);
	int ret;

#ifdef HAVE_GNUTLS
	if (data->flags & SF_SECURE) {
		if (gnutls_record_send (data->sess, xmlstr, strlen (xmlstr)) < 0) return IKS_NET_RWERR;
	} else
#endif
	{
		ret = data->trans->send (data->sock, xmlstr, strlen (xmlstr));
		if (ret) return ret;
	}
	if (data->logHook) data->logHook (data->user_data, xmlstr, strlen (xmlstr), 0);
	return IKS_OK;
}

void
iks_disconnect (iksparser *prs)
{
	iks_parser_reset (prs);
}

/*****  tls api  *****/

int
iks_has_tls (void)
{
#ifdef HAVE_GNUTLS
	return 1;
#else
	return 0;
#endif
}

int
iks_is_secure (iksparser *prs)
{
#ifdef HAVE_GNUTLS
	struct stream_data *data = iks_user_data (prs);

	return data->flags & SF_SECURE;
#else
	return 0;
#endif
}

int
iks_start_tls (iksparser *prs)
{
#ifdef HAVE_GNUTLS
	int ret;
	struct stream_data *data = iks_user_data (prs);

	ret = iks_send_raw (prs, "<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
	if (ret) return ret;
	data->flags |= SF_TRY_SECURE;
	return IKS_OK;
#else
	return IKS_NET_NOTSUPP;
#endif
}

/*****  sasl  *****/

int
iks_start_sasl (iksparser *prs, enum ikssasltype type, char *username, char *pass)
{
	iks *x;

	x = iks_new ("auth");
	iks_insert_attrib (x, "xmlns", IKS_NS_XMPP_SASL);
	switch (type) {
		case IKS_SASL_PLAIN: {
			int len = iks_strlen (username) + iks_strlen (pass) + 2;
			char *s = iks_malloc (80+len);
			char *base64;

			iks_insert_attrib (x, "mechanism", "PLAIN");
			sprintf (s, "%c%s%c%s", 0, username, 0, pass);
			base64 = iks_base64_encode (s, len);
			iks_insert_cdata (x, base64, 0);
			iks_free (base64);
			iks_free (s);
			break;
		}
		case IKS_SASL_DIGEST_MD5: {
			struct stream_data *data = iks_user_data (prs);

			iks_insert_attrib (x, "mechanism", "DIGEST-MD5");
			data->auth_username = username;
			data->auth_pass = pass;
			break;
		}
		default:
			iks_delete (x);
			return IKS_NET_NOTSUPP;
	}
	iks_send (prs, x);
	iks_delete (x);
	return IKS_OK;
}
