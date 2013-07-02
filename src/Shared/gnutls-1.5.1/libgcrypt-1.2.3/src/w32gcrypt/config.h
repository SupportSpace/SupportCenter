#ifndef CONFIG_H
#define CONFIG_H

#define HAVE_DOSISH_SYSTEM
#define HAVE_MEMMOVE
#define HAVE_RAND
#define VERSION "1.2.2"

typedef unsigned __int64 uint64_t;
#define UINT64_C(C) (C)

#define SIZEOF_UINT64_T 8

#define USE_AES 1
#define USE_ARCFOUR 1
#define USE_BLOWFISH 1
/* #undef USE_CAPABILITIES */
#define USE_CAST5 1
#define USE_CRC 1
#define USE_DES 1
#define USE_DSA 1
#define USE_ELGAMAL 1
#define USE_MD4 1
#define USE_MD5 1
#define USE_ONLY_8DOT3 1
#define USE_RFC2268 1
#define USE_RMD160 1
#define USE_RNDEGD 0
#define USE_RNDLINUX 0
#define USE_RNDUNIX 0
#define USE_RNDW32 1
#define USE_RSA 1
#define USE_SERPENT 1
#define USE_SHA1 1
#define USE_SHA256 1
#define USE_SHA512 1
//#define USE_TIGER 1
#define USE_TWOFISH 1

#define _GCRYPT_IN_LIBGCRYPT
#define S_IWUSR _S_IWRITE
#define S_IRUSR _S_IREAD

#define S_ISREG(A) (1)

#define pid_t int
#define ssize_t int
#define socklen_t int

#endif
