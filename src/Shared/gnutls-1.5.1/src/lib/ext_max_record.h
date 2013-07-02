/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005 Free Software Foundation
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

/* Maps record size to numbers according to the
 * extensions draft.
 */
int _gnutls_mre_num2record (int num);
int _gnutls_mre_record2num (uint16_t record_size);
int _gnutls_max_record_recv_params (gnutls_session_t session,
				    const opaque * data, size_t data_size);
int _gnutls_max_record_send_params (gnutls_session_t session, opaque * data,
				    size_t);
