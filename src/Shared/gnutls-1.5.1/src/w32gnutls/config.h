#ifndef CONFIG_H
#define CONFIG_H

#define ENABLE_ANON
#define ENABLE_PKI

#define ssize_t int
#define pid_t int

#define SIZEOF_UNSIGNED_LONG 4
#define SIZEOF_UNSIGNED_INT 4
#define SIZEOF_UNSIGNED_SHORT 2
#define SIZEOF_UNSIGNED_CHAR 1

#define STDC_HEADERS
#define HAVE_MEMMOVE
#define HAVE_ISASCII
#define inline __inline
#include <errno.h>

#define HAVE_DECL_MEMMEM 0
#include <io.h>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define GNUTLS_LIBTASN1_VERSION "0.3.4"
#define GNUTLS_GCRYPT_VERSION "1:1.2.2"
#define VERSION "1.4.0"
#undef WORDS_BIGENDIAN
#define LOCALEDIR ""

#define vsnprintf _vsnprintf
#define snprintf _snprintf

#define strcasecmp stricmp
#define HAVE_LIBZ
#define USE_LZO
#define USE_MINILZO

#define GC_USE_RANDOM
#define GC_USE_ARCFOUR
#define GC_USE_ARCTWO
#define GC_USE_DES
#define GC_USE_HMAC_MD5
#define GC_USE_HMAC_SHA1
#define GC_USE_MD2
#define GC_USE_MD4
#define GC_USE_MD5
#define GC_USE_RANDOM
#define GC_USE_RIJNDAEL
#define GC_USE_SHA1

#define GNUTLS_POINTER_TO_INT_CAST (long)

#define ENABLE_PSK

#endif
