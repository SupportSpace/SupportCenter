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

#ifndef GNUTLS_CIPHER_INT
# define GNUTLS_CIPHER_INT

#define cipher_hd_t gc_cipher_handle
#define GNUTLS_CIPHER_FAILED NULL

cipher_hd_t _gnutls_cipher_init (gnutls_cipher_algorithm_t cipher,
				 const gnutls_datum_t * key,
				 const gnutls_datum_t * iv);
int _gnutls_cipher_encrypt (cipher_hd_t handle, void *text, int textlen);
int _gnutls_cipher_decrypt (cipher_hd_t handle, void *ciphertext,
			    int ciphertextlen);
void _gnutls_cipher_deinit (cipher_hd_t handle);

#endif /* GNUTLS_CIPHER_INT */
