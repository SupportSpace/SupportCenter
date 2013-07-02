/*
 * Copyright (C) 2005 Free Software Foundation
 *
 * This file is part of GNUTLS.
 *
 * GNUTLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNUTLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <config.h>

#define _MAX(x,y) (x>y?x:y)

#ifndef ENABLE_PSK

#include <stdio.h>


int
main (int argc, char **argv)
{
  printf ("\nPSK not supported. This program is a dummy.\n\n");
  return 1;
};

void
psktool_version (void)
{
  fprintf (stderr, "GNU TLS dummy psktool.\n");
}

#else

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gnutls/gnutls.h>
#include <gnutls/extra.h>
#include <psk-gaa.h>

#include <gc.h>			/* for randomize */

#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WIN32
# include <pwd.h>
# include <unistd.h>
#else
# include <windows.h>
#endif

static int write_key (const char *username, const char *key, int key_size,
		      char *passwd_file);

void
psktool_version (void)
{
  const char *v = gnutls_check_version (NULL);

  printf ("psktool (GnuTLS) %s\n", LIBGNUTLS_VERSION);
  if (strcmp (v, LIBGNUTLS_VERSION) != 0)
    printf ("libgnutls %s\n", v);
}


#define KPASSWD "/etc/passwd.psk"
#define MAX_KEY_SIZE 64
int
main (int argc, char **argv)
{
  gaainfo info;
  int ret;
  struct passwd *pwd;
  unsigned char key[MAX_KEY_SIZE];
  char hex_key[MAX_KEY_SIZE * 2 + 1];
  gnutls_datum dkey;
  size_t hex_key_size = sizeof (hex_key);

  if ((ret = gnutls_global_init ()) < 0)
    {
      fprintf (stderr, "global_init: %s\n", gnutls_strerror (ret));
      exit (1);
    }

#ifdef HAVE_UMASK
  umask (066);
#endif

  if (gaa (argc, argv, &info) != -1)
    {
      fprintf (stderr, "Error in the arguments.\n");
      return -1;
    }

  if (info.passwd == NULL)
    info.passwd = KPASSWD;

  if (info.username == NULL)
    {
#ifndef _WIN32
      pwd = getpwuid (getuid ());

      if (pwd == NULL)
	{
	  fprintf (stderr, "No such user\n");
	  return -1;
	}

      info.username = pwd->pw_name;
#else
      fprintf (stderr, "Please specify a user\n");
      return -1;
#endif
    }

  if (info.key_size > MAX_KEY_SIZE)
    {
      fprintf (stderr, "Key size is too long\n");
      exit (1);
    }

  if (info.key_size < 1)
    info.key_size = 16;

  ret = gc_pseudo_random ((char *) key, info.key_size);
  if (ret != GC_OK)
    {
      fprintf (stderr, "Not enough randomness\n");
      exit (1);
    }

  printf ("Generating a random key for user '%s'\n", info.username);

  dkey.data = key;
  dkey.size = info.key_size;
  ret = gnutls_hex_encode (&dkey, hex_key, &hex_key_size);
  if (ret < 0)
    {
      fprintf (stderr, "HEX encoding error\n");
      exit (1);
    }

  ret = write_key (info.username, hex_key, hex_key_size, info.passwd);
  if (ret == 0)
    printf ("Key stored to %s\n", info.passwd);

  return ret;
}

static int
filecopy (char *src, char *dst)
{
  FILE *fd, *fd2;
  char line[5 * 1024];
  char *p;

  fd = fopen (dst, "w");
  if (fd == NULL)
    {
      fprintf (stderr, "Cannot open '%s' for write\n", dst);
      return -1;
    }

  fd2 = fopen (src, "r");
  if (fd2 == NULL)
    {
      /* empty file */
      fclose (fd);
      return 0;
    }

  line[sizeof (line) - 1] = 0;
  do
    {
      p = fgets (line, sizeof (line) - 1, fd2);
      if (p == NULL)
	break;

      fputs (line, fd);
    }
  while (1);

  fclose (fd);
  fclose (fd2);

  return 0;
}

static int
write_key (const char *username, const char *key, int key_size,
	   char *passwd_file)
{
  FILE *fd;
  char line[5 * 1024];
  char *p, *pp;
  char tmpname[1024];


  /* delete previous entry */
  struct stat st;
  FILE *fd2;
  int put;

  if (strlen (passwd_file) > sizeof (tmpname) + 5)
    {
      fprintf (stderr, "file '%s' is tooooo long\n", passwd_file);
      return -1;
    }
  strcpy (tmpname, passwd_file);
  strcat (tmpname, ".tmp");

  if (stat (tmpname, &st) != -1)
    {
      fprintf (stderr, "file '%s' is locked\n", tmpname);
      return -1;
    }

  if (filecopy (passwd_file, tmpname) != 0)
    {
      fprintf (stderr, "Cannot copy '%s' to '%s'\n", passwd_file, tmpname);
      return -1;
    }

  fd = fopen (passwd_file, "w");
  if (fd == NULL)
    {
      fprintf (stderr, "Cannot open '%s' for write\n", passwd_file);
      remove (tmpname);
      return -1;
    }

  fd2 = fopen (tmpname, "r");
  if (fd2 == NULL)
    {
      fprintf (stderr, "Cannot open '%s' for read\n", tmpname);
      remove (tmpname);
      return -1;
    }

  put = 0;
  do
    {
      p = fgets (line, sizeof (line) - 1, fd2);
      if (p == NULL)
	break;

      pp = strchr (line, ':');
      if (pp == NULL)
	continue;

      if (strncmp
	  (p, username,
	   _MAX (strlen (username), (unsigned int) (pp - p))) == 0)
	{
	  put = 1;
	  fprintf (fd, "%s:%s\n", username, key);
	}
      else
	{
	  fputs (line, fd);
	}
    }
  while (1);

  if (put == 0)
    {
      fprintf (fd, "%s:%s\n", username, key);
    }

  fclose (fd);
  fclose (fd2);

  remove (tmpname);


  return 0;
}

#endif /* ENABLE_PSK */
