/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation
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

int gnutls_cipher_set_priority (gnutls_session_t session, const int *);
int gnutls_kx_set_priority (gnutls_session_t session, const int *);
int gnutls_mac_set_priority (gnutls_session_t session, const int *);
int gnutls_compression_set_priority (gnutls_session_t session, const int *);
int gnutls_protocol_set_priority (gnutls_session_t session, const int *);
int gnutls_certificate_type_set_priority (gnutls_session_t session,
					  const int *);
