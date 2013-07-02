/* -*- Mode: C; c-file-style: "bsd" -*-
 * misc.c
 *        Copyright (C) 2002, 2003 Timo Schulz
 *        Copyright (C) 1998-2002 Free Software Foundation, Inc.
 *
 * This file is part of OpenCDK.
 *
 * OpenCDK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * OpenCDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenCDK; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "opencdk.h"
#include "main.h"


/* return 0 if the file exists. otherwise 1 */
int
_cdk_check_file( const char * file )
{
    FILE * fp;
    int check;

    if( !file )
        return 1;
    fp = fopen( file, "r" );
    check = fp? 0 : 1;
    if( fp )
        fclose( fp );
    return check;
}


u32
_cdk_timestamp (void)
{
    return (u32)time (NULL);
}


u32
_cdk_buftou32 (const byte * buf)
{
    u32 u = 0;
  
    if (buf) {
        u  = buf[0] << 24;
        u |= buf[1] << 16;
        u |= buf[2] <<  8;
        u |= buf[3];
    }
    return u;
}


void
_cdk_u32tobuf (u32 u, byte * buf)
{
    if (buf) {
        buf[0] = u >> 24;
        buf[1] = u >> 16;
        buf[2] = u >>  8;
        buf[3] = u      ;
    }
}


int
_cdk_strcmp (const char * a, const char * b)
{
    int alen, blen;
  
    alen = strlen (a);
    blen = strlen (b);
    if (alen != blen)
        return alen > blen? 1 : -1;
    return strcmp (a, b);
}
    

static const char *
parse_version_number (const char *s, int *number)
{
    int val = 0;

    if (*s == '0' && isdigit(s[1]))
        return NULL;
    /* leading zeros are not allowed */
    for (; isdigit(*s); s++) {
        val *= 10;
        val += *s - '0';     
    }
    *number = val;
    return val < 0? NULL : s;      
}


static const char *
parse_version_string (const char * s, int * major, int * minor, int * micro)
{
    s = parse_version_number (s, major);
    if (!s || *s != '.')
        return NULL;
    s++;
    s = parse_version_number (s, minor);
    if (!s || *s != '.')
        return NULL;
    s++;
    s = parse_version_number(s, micro);
    if (!s)
        return NULL;
    return s; /* patchlevel */
}


/**
 * cdk_check_version - Version control handling.
 * @req_version: The requested version
 *
 * Check that the the version of the library is at minimum the requested
 * one and return the version string; return NULL if the condition is
 * not satisfied.  If a NULL is passed to this function, no check is done,
 *but the version string is simply returned.
 **/
const char *
cdk_check_version (const char *req_version)
{
    const char *ver = VERSION;
    int my_major, my_minor, my_micro;
    int rq_major, rq_minor, rq_micro;
    const char *my_plvl, *rq_plvl;
  
    if (!req_version)
        return ver;
    my_plvl = parse_version_string (ver, &my_major, &my_minor, &my_micro);
    if (!my_plvl)
        return NULL;
    /* very strange our own version is bogus */
    rq_plvl = parse_version_string (req_version, &rq_major, &rq_minor,
                                    &rq_micro);
    if (!rq_plvl)
        return NULL;  /* req version string is invalid */
    if (my_major > rq_major
        || (my_major == rq_major && my_minor > rq_minor)
        || (my_major == rq_major && my_minor == rq_minor
            && my_micro > rq_micro)
        || (my_major == rq_major && my_minor == rq_minor
            && my_micro == rq_micro
            && strcmp (my_plvl, rq_plvl) >= 0)) {
        return ver;
    }
    return NULL;
}


void
cdk_strlist_free (cdk_strlist_t sl)
{
    cdk_strlist_t sl2;

    for(; sl; sl = sl2 ) {
        sl2 = sl->next;
        cdk_free (sl);
    }
}


cdk_strlist_t
cdk_strlist_add (cdk_strlist_t *list, const char *string)
{
    cdk_strlist_t sl;

    if (!string)
        return NULL;
  
    sl = cdk_calloc (1, sizeof *sl + strlen (string) + 1);
    if (!sl)
        return NULL;
    strcpy (sl->d, string);
    sl->next = *list;
    *list = sl;
    return sl;
}


const char *
cdk_strlist_walk (cdk_strlist_t root, cdk_strlist_t * context)
{
    cdk_strlist_t n;
  
    if( ! *context ) {
        *context = root;
        n = root;
    }
    else {
        n = (*context)->next;
        *context = n;
    }

    return n? n->d : NULL;
}


const char *
_cdk_memistr (const char *buf, size_t buflen, const char *sub)
{
    const byte *t, *s;
    size_t n;

    for (t = buf, n = buflen, s = sub ; n ; t++, n--) {
        if (toupper (*t) == toupper (*s)) {
            for (buf = t++, buflen = n--, s++;
                 n && toupper (*t) == toupper (*s); t++, s++, n--)
                ;
            if (!*s)
                return buf;
            t = buf;
            n = buflen;
            s = sub ;                        
        }
    }

    return NULL;
}


char *
cdk_utf8_encode (const char * string)
{
    const byte * s;
    char * buffer;
    byte * p;
    size_t length = 0;

    for (s = string; *s; s++) {
        length++;
        if (*s & 0x80)
            length++;
    }

    buffer = cdk_calloc (1, length + 1);
    for (p = buffer, s = string; *s; s++) {
        if (*s & 0x80) {
            *p++ = 0xc0 | ((*s >> 6) & 3);
            *p++ = 0x80 | (*s & 0x3f);
        }
        else
            *p++ = *s;
    }
    *p = 0;
    return buffer;
}


char *
cdk_utf8_decode (const char * string, size_t length, int delim)
{
  int nleft;
  int i;
  byte encbuf[8];
  int encidx;
  const byte *s;
  size_t n;
  byte *buffer = NULL, *p = NULL;
  unsigned long val = 0;
  size_t slen;
  int resync = 0;

  /* 1. pass (p==NULL): count the extended utf-8 characters */
  /* 2. pass (p!=NULL): create string */
  for (;;)
    {
      for (slen = length, nleft = encidx = 0, n = 0, s = string; slen;
           s++, slen--)
	{
          if (resync)
	    {
              if (!(*s < 128 || (*s >= 0xc0 && *s <= 0xfd)))
		{
                  /* still invalid */
                  if (p)
		    {
                      sprintf (p, "\\x%02x", *s);
                      p += 4;
		    }
                  n += 4;
                  continue;
		}
              resync = 0;
	    }
          if (!nleft)
	    {
              if (!(*s & 0x80))
		{		/* plain ascii */
                  if (*s < 0x20 || *s == 0x7f || *s == delim ||
                      (delim && *s == '\\'))
		    {
                      n++;
                      if (p)
                        *p++ = '\\';
                      switch (*s)
			{
			case '\n':
                          n++;
                          if (p)
                            *p++ = 'n';
                          break;
			case '\r':
                          n++;
                          if (p)
                            *p++ = 'r';
                          break;
			case '\f':
                          n++;
                          if (p)
                            *p++ = 'f';
                          break;
			case '\v':
                          n++;
                          if (p)
                            *p++ = 'v';
                          break;
			case '\b':
                          n++;
                          if (p)
                            *p++ = 'b';
                          break;
			case 0:
                          n++;
                          if (p)
                            *p++ = '0';
                          break;
			default:
                          n += 3;
                          if (p)
			    {
                              sprintf (p, "x%02x", *s);
                              p += 3;
			    }
                          break;
			}
		    }
                  else
		    {
                      if (p)
                        *p++ = *s;
                      n++;
		    }
		}
              else if ((*s & 0xe0) == 0xc0)
		{		/* 110x xxxx */
                  val = *s & 0x1f;
                  nleft = 1;
                  encidx = 0;
                  encbuf[encidx++] = *s;
		}
              else if ((*s & 0xf0) == 0xe0)
		{		/* 1110 xxxx */
                  val = *s & 0x0f;
                  nleft = 2;
                  encidx = 0;
                  encbuf[encidx++] = *s;
		}
              else if ((*s & 0xf8) == 0xf0)
		{		/* 1111 0xxx */
                  val = *s & 0x07;
                  nleft = 3;
                  encidx = 0;
                  encbuf[encidx++] = *s;
		}
              else if ((*s & 0xfc) == 0xf8)
		{		/* 1111 10xx */
                  val = *s & 0x03;
                  nleft = 4;
                  encidx = 0;
                  encbuf[encidx++] = *s;
		}
              else if ((*s & 0xfe) == 0xfc)
		{		/* 1111 110x */
                  val = *s & 0x01;
                  nleft = 5;
                  encidx = 0;
                  encbuf[encidx++] = *s;
		}
              else
		{		/* invalid encoding: print as \xnn */
                  if (p)
		    {
                      sprintf (p, "\\x%02x", *s);
                      p += 4;
		    }
                  n += 4;
                  resync = 1;
		}
	    }
          else if (*s < 0x80 || *s >= 0xc0)
	    {			/* invalid */
              if (p)
		{
                  for (i = 0; i < encidx; i++)
		    {
                      sprintf (p, "\\x%02x", encbuf[i]);
                      p += 4;
		    }
                  sprintf (p, "\\x%02x", *s);
                  p += 4;
		}
              n += 4 + 4 * encidx;
              nleft = 0;
              encidx = 0;
              resync = 1;
	    }
          else
	    {
              encbuf[encidx++] = *s;
              val <<= 6;
              val |= *s & 0x3f;
              if (!--nleft)
		{ /* ready native set */
                  if (val >= 0x80 && val < 256)
                    {
                      n++;	/* we can simply print this character */
                      if (p)
                        *p++ = val;
                    }
                  else
                    {	/* we do not have a translation: print utf8 */
                      if (p)
                        {
                          for (i = 0; i < encidx; i++)
                            {
                              sprintf (p, "\\x%02x", encbuf[i]);
                              p += 4;
                            }
                        }
                      n += encidx * 4;
                      encidx = 0;
                    }
                }
            }

        }
      if (!buffer) /* allocate the buffer after the first pass */
        buffer = p = cdk_malloc (n + 1);
      else
        {
          *p = 0; /* make a string */
          return buffer;
        }
    }
}


#ifndef HAVE_VASPRINTF
/* 
 * Like vsprintf but provides a pointer to malloc'd storage, which
 * must be freed by the caller (gcry_free).  Taken from libiberty as
 * found in gcc-2.95.2 and a little bit modernized.
 */
int
_cdk_vasprintf ( char **result, const char *format, va_list args)
{
    const char *p = format;
    /* Add one to make sure that it is never zero, which might cause malloc
       to return NULL.  */
    int total_width = strlen (format) + 1;
    va_list ap;

    /* this is not really portable but works under Windows */
    memcpy ( &ap, &args, sizeof (va_list));

    while (*p != '\0') {
        if (*p++ == '%') {
            while (strchr ("-+ #0", *p))
                ++p;
            if (*p == '*') {
                ++p;
                total_width += abs (va_arg (ap, int));
	    }
            else {
                char *endp;  
                total_width += strtoul (p, &endp, 10);
                p = endp;
            }
            if (*p == '.') {
                ++p;
                if (*p == '*') {
                    ++p;
                    total_width += abs (va_arg (ap, int));
		}
                else {
                    char *endp;
                    total_width += strtoul (p, &endp, 10);
                    p = endp;
                }
	    }
            while (strchr ("hlL", *p))
                ++p;
            /* Should be big enough for any format specifier except %s
               and floats.  */
            total_width += 30;
            switch (*p) {
	    case 'd':
	    case 'i':
	    case 'o':
	    case 'u':
	    case 'x':
	    case 'X':
	    case 'c':
                (void) va_arg (ap, int);
                break;
	    case 'f':
	    case 'e':
	    case 'E':
	    case 'g':
	    case 'G':
                (void) va_arg (ap, double);
                /* Since an ieee double can have an exponent of 307, we'll
                   make the buffer wide enough to cover the gross case. */
                total_width += 307;
	    
	    case 's':
                total_width += strlen (va_arg (ap, char *));
                break;
	    case 'p':
	    case 'n':
                (void) va_arg (ap, char *);
                break;
	    }
	}
    }
    *result = gcry_malloc (total_width);
    if (*result != NULL)
        return vsprintf (*result, format, args);
    else
        return 0;
}

void
_cdk_vasprintf_free (void * p)
{
    cdk_free (p);
}
#else /*!__MINGW32__*/
void
_cdk_vasprintf_free (void * p)
{
    if (p)
        free (p);
}
#endif
