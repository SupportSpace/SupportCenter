/*
 * Copyright (C) 2004, 2005 Free Software Foundation
 *
 * Author: Simon Josefsson
 *
 * This file is part of GNUTLS.
 *
 * GNUTLS is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNUTLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNUTLS; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include "utils.h"

void
doit (void)
{
  if (debug)
    {
      printf ("GNUTLS header version %s.\n", LIBGNUTLS_VERSION);
      printf ("GNUTLS library version %s.\n", gnutls_check_version (NULL));
    }

  if (gnutls_check_version (LIBGNUTLS_VERSION))
    success ("gnutls_check_version OK\n");
  else
    fail ("gnutls_check_version ERROR\n");
}
